/*
 * dma.c
 *
 *  Created on: 25 oct. 2019
 *      Author: Tomas
 */


#include <DMA/dma.h>
#include "MK64F12.h"
#define DMA_AMOUNT_CHANNELS	16

static void dma_mux_init(dma_mux_conf_t config);
inline static void change_erq_flag(int channel_number, bool value);

void dma_init(){

	static bool initialized = false;
	if(initialized) return;

	//clock gating
	SIM_Type* sim = SIM;
	sim->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
	sim->SCGC7 |= SIM_SCGC7_DMA_MASK;

	initialized = true;
}

static void dma_mux_init(dma_mux_conf_t config){
	DMAMUX_Type* dma_mux = DMAMUX;
	dma_mux->CHCFG[config.channel_number] = 0x00;
	dma_mux->CHCFG[config.channel_number] = DMAMUX_CHCFG_ENBL(config.dma_enable) | DMAMUX_CHCFG_TRIG(config.trigger_enable) | DMAMUX_CHCFG_SOURCE(config.source);
}
/*Prior to activating a channel, you must initialize its TCD with the appropriate transfer
profile.*/
void dma_set_config_channel(dma_conf_t config){
	//	dma_mux_conf_t mux_conf = {0, true, true, 0x00};	//channel, dma_enable, trigger_enable, source
	dma_mux_init(config.dma_mux_conf);

	DMA_Type * dma = DMA0;

	DMA0->SERQ |= DMA_SERQ_SERQ(config.dma_mux_conf.channel_number);
	//change_erq_flag(config.dma_mux_conf.channel_number, true);

	dma->TCD[config.dma_mux_conf.channel_number].SADDR = config.source_address;
	dma->TCD[config.dma_mux_conf.channel_number].DADDR = config.destination_address;

	dma->TCD[config.dma_mux_conf.channel_number].SOFF = config.source_offset;
	dma->TCD[config.dma_mux_conf.channel_number].DOFF = config.destination_offset;

	dma->TCD[config.dma_mux_conf.channel_number].ATTR = DMA_ATTR_SSIZE(config.source_data_transfer_size)| DMA_ATTR_DSIZE(config.destination_data_transfer_size);
	//number of bytes to be transferred in each service request

	dma->TCD[config.dma_mux_conf.channel_number].NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(config.nbytes);

	//	The current major iteration count (CITER) and the beginning iteration count (BITER)
	//	must be initialized to the same value.

	dma->TCD[config.dma_mux_conf.channel_number].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(config.citer);
	dma->TCD[config.dma_mux_conf.channel_number].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(config.citer);
	dma->TCD[config.dma_mux_conf.channel_number].SLAST = DMA_SLAST_SLAST(config.source_address_adjustment);
	dma->TCD[config.dma_mux_conf.channel_number].DLAST_SGA = DMA_DLAST_SGA_DLASTSGA(config.destination_address_adjustment);
		//sets bandwidth control to no engine stalls.
		/*1 The current channel’s TCD specifies a scatter gather format.
		The DLASTSGA field provides a memory pointer to the next TCD to be loaded
		into this channel after the major loop completes its execution.
		*/
		//Enable an interrupt when major counter is half complete.
		/*	Disable Request
			0 The channel’s ERQ bit is not affected
			1 The channel’s ERQ bit is cleared when the major loop is complete*/
		//	Enable an interrupt when major iteration count completes
		//1 starts the channel explicitly by software!
	dma->TCD[config.dma_mux_conf.channel_number].CSR = 0X00;
	dma->TCD[config.dma_mux_conf.channel_number].CSR = DMA_CSR_BWC(0x00) |
			DMA_CSR_MAJORLINKCH(config.dma_mux_conf.channel_number) | DMA_CSR_MAJORELINK(0x00) | DMA_CSR_ESG(0x00) |
			DMA_CSR_DREQ(0x00) | DMA_CSR_INTHALF(0) | DMA_CSR_INTMAJOR(1) | DMA_CSR_START(0);

	//TENDRIA QUE HACER EL NVIC!!!
	//DMA_CHN_IRQS { { DMA0_IRQn, DMA1_IRQn, DMA2_IRQn, DMA3_IRQn, DMA4_IRQn, DMA5_IRQn, DMA6_IRQn, DMA7_IRQn, DMA8_IRQn, DMA9_IRQn, DMA10_IRQn, DMA11_IRQn, DMA12_IRQn, DMA13_IRQn, DMA14_IRQn, DMA15_IRQn } }
}


inline static void change_erq_flag(int channel_number, bool value){
	/*In order to be able initiate a DMA transfer when a hardware service request is issued the enable request
	 * register (DMA_ERQ) must be set, it is important to notice that the state of the DMA enable request flag
	 * does not affect a channel service request made explicitly through software or a linked channel request.
	 */

	//https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
	unsigned long newbit = value;
	uint32_t dma_channel_shifts[DMA_AMOUNT_CHANNELS] = {DMA_ERQ_ERQ0_SHIFT, DMA_ERQ_ERQ1_SHIFT, DMA_ERQ_ERQ2_SHIFT,
														DMA_ERQ_ERQ3_SHIFT, DMA_ERQ_ERQ4_SHIFT, DMA_ERQ_ERQ5_SHIFT,
														DMA_ERQ_ERQ6_SHIFT, DMA_ERQ_ERQ7_SHIFT, DMA_ERQ_ERQ8_SHIFT,
														DMA_ERQ_ERQ9_SHIFT, DMA_ERQ_ERQ10_SHIFT, DMA_ERQ_ERQ11_SHIFT,
														DMA_ERQ_ERQ12_SHIFT, DMA_ERQ_ERQ13_SHIFT, DMA_ERQ_ERQ14_SHIFT,
														DMA_ERQ_ERQ15_SHIFT };

	if( (channel_number >= 0) && (channel_number < DMA_AMOUNT_CHANNELS) )
		DMA0->ERQ ^= (-newbit ^ DMA0->ERQ) & dma_channel_shifts[channel_number];
	else
		HardFault_Handler();


}

bool dma_get_finished_transfer(int channel){
	if((channel < 0) || (channel >= DMA_AMOUNT_CHANNELS))
		HardFault_Handler();

	return (DMA0->TCD[channel].CSR) & DMA_CSR_DONE_MASK;
}

