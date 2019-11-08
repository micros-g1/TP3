/*
 * fsk_rx.c
 *
 *  Created on: Oct 31, 2019
 *      Author: Rocío Parra
 */


#include "fsk_rx.h"
#include "util/f_list.h"
#include <stdlib.h>
#include "VREF/vref_driver.h"
#include "DMA/dma.h"
#include "ADC/adc_driver.h"
#include "PIT/pit.h"
#include "MK64F12.h"

#define FREQ_1      1200.0
#define FREQ_0      2200.0
#define T_BIT       (1.0/FREQ_1)

#define FSK_START   0
#define FSK_STOP    1


#define FREQ_S      12000.0
#define OPT_DELAY   0.000446
#define D_SAMPLES   ((uint32_t)(OPT_DELAY*FREQ_S))
#define D_SAMPLES_INT	5 // redo the math to avoid warnings

#define FILTER_N    25
#define WAVE_LEN    ((uint32_t)(T_BIT*FREQ_S))
#define WAVE_LEN_INT	10	// redo the math to avoid warnings

#define SAMPLING_COUNT_VALUE PIT_CLOCK_FREQUENCY/FREQ_S


#define N_LISTS     3
static float samples_buffer[D_SAMPLES_INT];
static float prod_delay_buffer[FILTER_N];
static float y_buffer[WAVE_LEN_INT];

static flist_t samples;
static flist_t prod_delay;
static flist_t y;

volatile uint16_t adc_result;

#define THRESHOLD   0.5
// 1.0 is logic 0, -1.0 is logic 1
// so i consider that if the average is >THRESHOLD i've detected a 0
// used only to sync (detect start bit)

static const float taps[FILTER_N] = {
        7.538678242239056767268201575405e-04,
        9.882920310659427769184315337725e-04,
        -8.383626700726674786968700597563e-19,
        -3.667288016803320338521388066511e-03,
        -1.002679893057338220141438966948e-02,
        -1.604571650584507569448078356800e-02,
        -1.527223745500639294492284392391e-02,
        4.786832900832612893620485891445e-18,
        3.436594215694192128163209076774e-02,
        8.498465691047664416490903249723e-02,
        1.403130895612941519967620251919e-01,
        1.836028556180376858364411418734e-01,
        2.000066736123758959209339991503e-01,
        1.836028556180376858364411418734e-01,
        1.403130895612941519967620251919e-01,
        8.498465691047664416490903249723e-02,
        3.436594215694192128163209076774e-02,
        4.786832900832612893620485891445e-18,
        -1.527223745500639294492284392391e-02,
        -1.604571650584507569448078356800e-02,
        -1.002679893057338220141438966948e-02,
        -3.667288016803320338521388066511e-03,
        -8.383626700726674786968700597563e-19,
        9.882920310659427769184315337725e-04,
        7.538678242239056767268201575405e-04
};



static fsk_callback_t callback = NULL;


void fsk_rx_process_sample(void);


void fsk_rx_init(fsk_callback_t cb)
{
    static bool isinit = false;
    if (isinit)
        return;
    isinit = true;

    callback = cb;

    flist_t * lists[N_LISTS] = {&samples, &prod_delay, &y};
    float * buffers[N_LISTS] = {samples_buffer, prod_delay_buffer, y_buffer};
    uint32_t sizes[N_LISTS] = {D_SAMPLES, FILTER_N, WAVE_LEN};

    for (unsigned int i = 0; i < N_LISTS; i++) {
        fl_init(lists[i], buffers[i], sizes[i]);
        for (unsigned int j = 0; j < sizes[i]; j++) {
            fl_pushback(lists[i], 0.0);
        }
    }




	vref_init();

	adc_init();
	adc_trigger_select(ADC_SOFTWARE_TRIGGER);
	adc_enable_dma(true);

	dma_init();

	/* Configure DMA to trigger ADC conversion */

	dma_mux_conf_t mux_conf1 = {
		.channel_number=2,
		.dma_enable=true,
		.source=60, //kDmaRequestMux0AlwaysOn59,
		.trigger_enable=true
	};
	dma_conf_t conf1 = {
		.citer=1,
		.destination_address= (uint32_t)&ADC0->SC1[0],
		.destination_address_adjustment=0,
		.destination_data_transfer_size=DMA_32BIT,
		.destination_offset=0,
		.dma_mux_conf=mux_conf1,
		.nbytes=sizeof(uint32_t),
		.source_address=(uint32_t)&ADC0->SC1[0],
		.source_data_transfer_size=DMA_32BIT,
		.source_offset=0,
		.major_loop_int_enable = false,
		.callback = NULL
	};

	dma_set_config_channel(conf1);


	/* Configure DMA to retrieve ADC value and store it in DAC */
	dma_mux_conf_t mux_conf2 = {
		.channel_number=1,
		.dma_enable=true,
		.source=40, //ADC0
		.trigger_enable=true
	};
	dma_conf_t conf2 = {
		.citer=1,
		.destination_address=(uint32_t)&adc_result,
		.destination_address_adjustment=0,
		.destination_data_transfer_size=DMA_16BIT,
		.destination_offset=0,
		.dma_mux_conf=mux_conf2,
		.nbytes=sizeof(uint16_t),
		.source_address=(uint32_t)&ADC0->R[0],
		.source_address_adjustment=0,
		.source_data_transfer_size=DMA_16BIT,
		.source_offset=0,
		.major_loop_int_enable = false,
		.callback = fsk_rx_process_sample
	};
	dma_set_config_channel(conf2);


	/* pit */
	pit_init();
	pit_conf_t pit_conf = {
		.callback=NULL,
		.chain_mode=false,
		.channel=PIT_CH2,
		.timer_count=SAMPLING_COUNT_VALUE,
		.timer_enable=true,
		.timer_interrupt_enable=false
	};
	pit_set_channel_conf(pit_conf);
}


void fsk_rx_process_sample(void)
{
    static float y_sum = 0;
    static bool idle = true;
    static int32_t curr_sample = 0; // this is allowed to be negative to discard some samples
    static uint32_t curr_bit = 0;
    static uint8_t word_received = 0;
    static bool curr_parity = true;

    uint16_t sample = adc_result;

    float newest_sample = ((float)sample)/(((float)UINT16_MAX)/2.0)-1;
    float oldest_sample = fl_popfront(&samples);
    fl_pushback(&samples, newest_sample);

    fl_popfront(&prod_delay);
    fl_pushback(&prod_delay, oldest_sample*newest_sample);

    float newest_y = 0;
    for (unsigned int i = 0; i < FILTER_N; i++) {
        newest_y += taps[i]*fl_read(&prod_delay, FILTER_N-i-1);
    }
    newest_y *= 2;
    float oldest_y = fl_popfront(&y);
    fl_pushback(&y, newest_y);

    if (idle) {
        y_sum -= oldest_y;
        y_sum += newest_y;
        if (y_sum > THRESHOLD*WAVE_LEN) {
            idle = false;
            curr_bit = 0;
            curr_sample = 0;
            word_received = 0;
        }
    }
    else if (++curr_sample >= WAVE_LEN) {
        uint8_t sum = 0;
        for (unsigned int i = WAVE_LEN/2-1; i<= WAVE_LEN/2+1; i++) {
            if (fl_read(&y, i) < 0) { // -1 for 1, +1 for 0
                sum++;
            }
        }
        bool newest_bit = sum >= 2;

        curr_bit++;
        if (curr_bit == 9) {
            if (curr_parity != newest_bit) { // error!
                idle = true;
            }
        } else if (curr_bit > 9) {
            if (newest_bit == FSK_STOP) {
                uint8_t temp = 0;
                for (unsigned int i = 0; i < 8; i++) {
                    temp <<= 0x01U;
                    temp |= (word_received & 0x01U);
                    word_received >>= 0x01U;
                }
                callback(temp);
            }
            idle = true;
        } else {
            word_received <<= 1U;
            word_received |= newest_bit;
            curr_parity ^= newest_bit;
        }
        curr_sample = 0;
    }
}

void fsk_rx_disable_interrupts()
{
	;
}

void fsk_rx_enable_interrupts()
{
	;
}

