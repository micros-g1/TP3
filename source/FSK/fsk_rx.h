/*
 * fsk_tx.h
 *
 *  Created on: Oct 31, 2019
 *      Author: Rocío Parra
 */

#ifndef FSK_FSK_RX_H_
#define FSK_FSK_RX_H_

#include <stdbool.h>
#include <stdint.h>

typedef void (* fsk_callback_t)(uint8_t);


/**
 * @brief fsk_init: initialize fsk rx module
 * @param callback: to be called with each new received data byte
 */
void fsk_rx_init(fsk_callback_t callback);

/**
 * @brief fsk_rx_process_sample: to be called with each new sample
 * @param sample: new sample
 */
void fsk_rx_process_sample(uint16_t sample);


#endif /* FSK_FSK_RX_H_ */
