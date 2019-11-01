/*
 * fsk_tx.c
 *
 *  Created on: 31 Oct 2019
 *      Author: grein
 */

#include <FSK/fsk_tx.h>
#include <stdint.h>
#include "VREF/vref_driver.h"
#include "PIT/pit.h"
#include <stdlib.h>
#include <math.h>
#include "DAC/dac_driver.h"
#include "DMA/dma.h"

#define TOTAL_TABLE_ELEMENTS 256
#define BIT_1_FREQ	1200
#define BIT_0_FREQ  2200
#define BAUD_RATE   1200

#define BIT_1_COUNTER_VALUE (PIT_CLOCK_FREQUENCY/(BIT_1_FREQ*TOTAL_TABLE_ELEMENTS))
#define BIT_0_COUNTER_VALUE (PIT_CLOCK_FREQUENCY/(BIT_0_FREQ*TOTAL_TABLE_ELEMENTS))
#define BIT_TIME_COUNTER_VALUE (PIT_CLOCK_FREQUENCY/BAUD_RATE)

#if (TOTAL_TABLE_ELEMENTS & (TOTAL_TABLE_ELEMENTS - 1)) != 0
#error Table total elements must be power of two
#endif
uint16_t sin_table[TOTAL_TABLE_ELEMENTS] __attribute__ ((aligned(TOTAL_TABLE_ELEMENTS)));

fsk_tx_next_bit_callback next_bit_callback;

static float map_to_range(float a, float b, float c, float d, float x);
static void next_bit_request_handler(void);

void fsk_tx_init(fsk_tx_next_bit_callback callback)
{
	static bool initialized = false;
	if(initialized)
		return;
	vref_init();

	float h = 2*M_PI/TOTAL_TABLE_ELEMENTS;
	for (int i = 0; i < TOTAL_TABLE_ELEMENTS; i++){
		float value = map_to_range(-1, 1, 0, DAC_MAX_DIGITAL_VALUE, sin(h*i));
		//For debugging
		//value = (((float)i)/TOTAL_TABLE_ELEMENTS)*DAC_MAX_DIGITAL_VALUE;
		sin_table[i] = (uint16_t)value;
	}
	dac_init();
	dac_enable(true);
	dac_setup_buffer(DAC_BUFFER_DISABLED);

	dma_init();
	/* Configure DMA to write sin values to dest_array */
	dma_mux_conf_t mux_conf1 = {.channel_number=0, .dma_enable=true, .source=58, .trigger_enable=true};
	dma_conf_t conf1 = {
		.citer=1,
		.destination_address=dac_data0_address(),
		.destination_address_adjustment=0,
		.destination_data_transfer_size=DMA_16BIT,
		.destination_offset=0,
		.dma_mux_conf=mux_conf1,
		.nbytes=sizeof(uint16_t),
		.source_address=(uint32_t)sin_table,
		.source_data_transfer_size=DMA_16BIT,
		.source_offset=sizeof(uint16_t),
		.source_address_adjustment=0,
		.smod=log2(TOTAL_TABLE_ELEMENTS)+1,
		.dmod=0,
		.major_loop_int_enable = false,
		.callback = NULL
	};
	dma_set_config_channel(conf1);

	/* pit */
	pit_init();
	//Set default counter value to bit 1, IDLE
	pit_conf_t pit_conf = {
		.callback=NULL,
		.chain_mode=false,
		.channel=PIT_CH0,
		.timer_count=BIT_1_COUNTER_VALUE,
		.timer_enable=true,
		.timer_interrupt_enable=false
	};
	//Interrupt disabled by default
	pit_conf_t pit_conf1 = {
		.callback=next_bit_request_handler,
		.chain_mode=false,
		.channel=PIT_CH1,
		.timer_count=BIT_TIME_COUNTER_VALUE,
		.timer_enable=true,
		.timer_interrupt_enable=false
	};
	pit_set_channel_conf(pit_conf);
	pit_set_channel_conf(pit_conf1);
	next_bit_callback = callback;
}
void fsk_tx_interrupt_enable(bool ie)
{
	pit_set_timer_interrupt_enabled(PIT_CH1, ie);
}


static float map_to_range(float a, float b, float c, float d, float x){
	float ret = c + (d-c)/(b-a) * (x - a);
	return ret;
}

static void next_bit_request_handler(void)
{
	bool bit = true;
	if(next_bit_callback != NULL)
		bit = next_bit_callback();
	pit_set_new_timer_countdown(PIT_CH0,bit?BIT_1_COUNTER_VALUE:BIT_0_COUNTER_VALUE);
}
