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

typedef struct{
	int channel_number;
	bool dma_enable;
	bool trigger_enable;
	uint8_t source;
}dma_mux_conf_t;
typedef struct{
	dma_mux_conf_t dma_mux_conf;
	uint32_t source_address;
	uint32_t destination_address;
	uint32_t source_offset;
	uint32_t destination_offset;

}dma_conf_t;



void dma_init();
void dma_set_config_channel(dma_conf_t config);

#endif /* DMA_DMA_H_ */
