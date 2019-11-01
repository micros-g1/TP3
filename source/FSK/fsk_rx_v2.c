/*
 * fsk_rx_v2.c
 *
 *  Created on: 1 nov. 2019
 *      Author: Tomas
 */
#include <FSK/fsk_rx.h>

#define FREQ_1      1200.0
#define FREQ_0      2200.0
#define T_BIT       (1.0/FREQ_1)

#define FSK_START   0
#define FSK_STOP    1
#define T0 			127
#define SYMBOL_TOL	12

void fsk_rx_process_sample_v2(uint16_t elapsed_time){
    static bool idle = true;
    static uint32_t curr_bit = 0;
    static uint8_t word_received = 0;
    static bool curr_parity = true;
    static bool first_zero_trigger = false;

    if (idle) {
    	if(elapsed_time <= T0 + SYMBOL_TOL){	//check if counter_value <= T0 + tolerance
			idle = false;
			curr_bit = 0;
			word_received = 0;
			first_zero_trigger = true;
    	}
    }
    else {
    	bool newest_bit;
    	if( (first_zero_trigger = !first_zero_trigger) ){
    		if( ( newest_bit = (elapsed_time <= T0 + SYMBOL_TOL) ))
    			first_zero_trigger = true;

			curr_bit++;
			if (curr_bit == 9) {
				if (curr_parity != newest_bit) { // error!
					idle = true;
				}
			}
			else if (curr_bit > 9) {
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
			}
			else {
				word_received <<= 1U;
				word_received |= newest_bit;
				curr_parity ^= newest_bit;
			}
    	}
    }
}

