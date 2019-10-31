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
#define TIMER_FREQ 50e6UL
#define BIT_1_FREQ	1200
#define BIT_0_FREQ  2200
#define BAUD_RATE   1200

#if (TOTAL_TABLE_ELEMENTS & (TOTAL_TABLE_ELEMENTS - 1)) != 0
#error Table total elements must be power of two
#endif
uint16_t sin_table[TOTAL_TABLE_ELEMENTS] __attribute__ ((aligned(TOTAL_TABLE_ELEMENTS)));


static float map_to_range(float a, float b, float c, float d, float x);
static void next_bit_request_handler(void);

void fsk_tx_init(fsk_tx_next_bit_callback next_bit_callback)
{
	static bool initialized = false;
	if(initialized)
		return;
	vref_init();

	float h = 2*M_PI/TOTAL_TABLE_ELEMENTS;
	for (int i = 0; i < TOTAL_TABLE_ELEMENTS; i++){
		float value = map_to_range(-1, 1, 0, DAC_MAX_DIGITAL_VALUE, sin(h*i));
		sin_table[i] = (uint16_t)value;
	}
	dac_init();
	dac_enable(true);
	dac_setup_buffer(DAC_BUFFER_DISABLED);

	dma_init();
	/* Configure DMA to write sin values to dest_array */
	dma_mux_conf_t mux_conf1 = {.channel_number=0, .dma_enable=true, .source=58, .trigger_enable=true};
	dma_conf_t conf1 = {.citer=1, .destination_address=dac_data0_address(), .destination_address_adjustment=0, .destination_data_transfer_size=DMA_16BIT,
						.destination_offset=0, .dma_mux_conf=mux_conf1, .nbytes=sizeof(uint16_t), .source_address=(uint32_t)sin_table, .source_data_transfer_size=DMA_16BIT,
						.source_offset=sizeof(uint16_t), .source_address_adjustment=0, .smod=log2(TOTAL_TABLE_ELEMENTS)+1, .dmod=0};
	dma_set_config_channel(conf1);

	/* pit */
	pit_init();
	pit_conf_t pit_conf = {.callback=NULL, .chain_mode=false, .channel=PIT_CH0, .timer_count=0x00FF , .timer_enable=true, .timer_interrupt_enable=false};
	//Interrupt disabled by default
	pit_conf_t pit_conf1 = {.callback=next_bit_request_handler, .chain_mode=false, .channel=PIT_CH1, .timer_count=41667, .timer_enable=true, .timer_interrupt_enable=false};
	pit_set_channel_conf(pit_conf);
	pit_set_channel_conf(pit_conf1);
}
void fsk_tx_interrupt_enable(bool ie)
{
	pit_set_timer_interrupt_enabled(PIT_CH1, ie);
}


static float map_to_range(float a, float b, float c, float d, float x){
	float ret = c + (d-c)/(b-a) * (x - a);
	return ret;
}

static bool bit = false;
static void next_bit_request_handler(void)
{
	bit = !bit;
	if(bit)
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(89);
	else
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(163);
}
