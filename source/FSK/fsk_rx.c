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
#include "ADC/adc_driver.h"
#include "PIT/pit.h"
#include "MK64F12.h"
#include <stdbool.h>
#include <stdint.h>
#include "util/clock.h"
#include "gpio.h"

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

#define THRESHOLD   0.0
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

static bool idle;
static float y_sum;
static int32_t curr_sample; // this is allowed to be negative to discard some samples
static uint32_t curr_bit;
static uint8_t word_received;
static bool curr_parity;

double avg_time;
static fsk_callback_t callback = NULL;

static void fsk_rx_process_sample(void);
static void start_convertion_callback();

void fsk_rx_init(fsk_callback_t cb)
{
	static bool isinit = false;
	    if (isinit)
	        return;
	    isinit = true;
	    idle = true;
	    y_sum = 0;
	    callback = cb;

	    flist_t * lists[N_LISTS] = {&samples, &prod_delay, &y};
	    float * buffers[N_LISTS] = {samples_buffer, prod_delay_buffer, y_buffer};
	    uint32_t sizes[N_LISTS] = {D_SAMPLES, FILTER_N, WAVE_LEN};
	    float init_value[N_LISTS] = {0, 0, -1};

	    for (unsigned int i = 0; i < N_LISTS; i++) {
	        fl_init(lists[i], buffers[i], sizes[i]);
	        for (unsigned int j = 0; j < sizes[i]; j++) {
	            fl_pushback(lists[i], init_value[i]);
	        }
	    }

	    y_sum = -1*WAVE_LEN;

		vref_init();

		adc_init();
		adc_trigger_select(ADC_SOFTWARE_TRIGGER);
		adc_set_conversion_completed_handler(fsk_rx_process_sample);

		/* pit */
		pit_init();
		pit_conf_t pit_conf = {
			.callback=start_convertion_callback,
			.chain_mode=false,
			.channel=PIT_CH1,
			.timer_count=SAMPLING_COUNT_VALUE,
			.timer_enable=true,
			.timer_interrupt_enable=true
		};
		pit_set_channel_conf(pit_conf);
		gpioMode(PORTNUM2PIN(PA, 1), OUTPUT);
		gpioMode(PORTNUM2PIN(PB, 23), OUTPUT);
}


void fsk_rx_process_sample(void)
{
	gpioWrite(PORTNUM2PIN(PB, 23),true);
	uint16_t sample = adc_get_data();
    float newest_sample = ((float)2.3*sample)/(((float)UINT16_MAX)/2.0)-1;
    float oldest_sample = fl_popfront(&samples);
    fl_pushback(&samples, newest_sample);

    fl_popfront(&prod_delay);
    fl_pushback(&prod_delay, oldest_sample*newest_sample);

    float newest_y = 0;

    for (unsigned int i = 0; i < FILTER_N; i++) {
        newest_y += taps[i]*fl_read(&prod_delay, FILTER_N-i-1);
    }
    newest_y *= 2;
    //float oldest_y =
    fl_popfront(&y);
    fl_pushback(&y, newest_y);

    if (idle) {
    	y_sum = 0;
    	for (unsigned int i = 0; i < y.len; i++) {
    		y_sum += fl_read(&y, i);
    	}
        if (y_sum > THRESHOLD*WAVE_LEN && newest_y > 0) {
            idle = false;
            curr_bit = 0;
            curr_sample = -((int32_t)WAVE_LEN)/2;
            word_received = 0;
            curr_parity = true;
        }
    }
    else if (++curr_sample >= ((int32_t)WAVE_LEN)) {
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
    gpioWrite(PORTNUM2PIN(PB, 23),false);
}

void fsk_rx_disable_interrupts()
{
	adc_set_interrupts_enabled(false);
}

void fsk_rx_enable_interrupts()
{
	adc_set_interrupts_enabled(true);
}


static void start_convertion_callback()
{
	gpioWrite(PORTNUM2PIN(PA, 1),true);
	adc_trigger_conversion();
	gpioWrite(PORTNUM2PIN(PA, 1),false);
}


