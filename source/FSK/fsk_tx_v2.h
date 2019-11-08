/*
 * fsk_tx_v2.h
 *
 *  Created on: Nov 7, 2019
 *      Author: Rocío Parra
 */

#ifndef FSK_FSK_TX_V2_H_
#define FSK_FSK_TX_V2_H_

#include <stdbool.h>

typedef bool (*fsk_tx_next_bit_callback)(void);

void fsk_tx_init(fsk_tx_next_bit_callback next_bit_callback);
void fsk_tx_interrupt_enable(bool ie);


#endif /* FSK_FSK_TX_V2_H_ */
