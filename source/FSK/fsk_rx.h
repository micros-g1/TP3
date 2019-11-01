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


//extern uint16_t adc_result;


typedef void (* fsk_callback_t)(uint8_t);


/**
 * @brief fsk_init: initialize fsk rx module. Interruptions are enabled by default
 * @param callback: to be called with each new received data byte
 */
void fsk_rx_init(fsk_callback_t callback);

///**
// * @brief fsk_rx_process_sample: to be called with each new sample
// * @param sample: new sample
// */
//void fsk_rx_process_sample(uint16_t sample);

/**
 * @brief disable interrupts. This effectively pauses the module and may cause ADC samples to be lost
 */
void fsk_rx_disable_interrupts();

/**
 * @brief enable interrupts. Interrupts are enabled by default
 */
void fsk_rx_enable_interrupts();

#endif /* FSK_FSK_RX_H_ */
