/*
 * dma.h
 *
 *  Created on: 25 oct. 2019
 *      Author: Tomas
 */

#ifndef DMA_DMA_H_
#define DMA_DMA_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum{DMA_8BIT = 0x00, DMA_16BIT = 0x01, DMA_32BIT = 0x02, DMA_16BYTE_BURST = 0x4, DMA_32BYTE_BURST = 0x05} dma_transfer_size_t;
typedef enum {DMA_FTM_} dma_sources_t;
typedef struct{
	uint8_t channel_number;
	bool dma_enable;
	bool trigger_enable;
	uint8_t source;
}dma_mux_conf_t;

typedef struct{
	dma_mux_conf_t dma_mux_conf;
	uint32_t source_address;
	uint32_t destination_address;
	int16_t source_offset;
	int16_t destination_offset;
	dma_transfer_size_t destination_data_transfer_size;
	dma_transfer_size_t	source_data_transfer_size;
	uint32_t nbytes;
	uint8_t citer;
	int32_t source_address_adjustment;
	int32_t destination_address_adjustment;
<<<<<<< HEAD
	uint8_t smod;
	uint8_t dmod;
=======
>>>>>>> 9161f0b686b9ae09572c91a78e6a134fad47f12d
}dma_conf_t;


typedef struct{
	uint8_t bandwidth_control;
	bool channel_to_channel_linking;
	bool enable_scatter_gather;
	bool enable_request;
}dma_control_and_status_conf_t;

void dma_init();
void dma_set_config_channel(dma_conf_t config);
bool dma_get_finished_transfer(int channel);
#endif /* DMA_DMA_H_ */
