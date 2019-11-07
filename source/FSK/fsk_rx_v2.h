/*
 * fsk_rx_v2.h
 *
 *  Created on: 1 nov. 2019
 *      Author: Tomas
 */

#ifndef FSK_FSK_RX_V2_H_
#define FSK_FSK_RX_V2_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (* fsk_v2_callback_t)(uint8_t);

void fsk_rx_init(fsk_v2_callback_t fsk_callback);

#endif /* FSK_FSK_RX_V2_H_ */
