/*
 * fsk_rx_v2.c
 *
 *  Created on: 1 nov. 2019
 *      Author: Tomas
 */
#include <FSK/fsk_rx_v2.h>
#include "CMP/cmp.h"
#include "FTM/flex_timer.h"
#include <FSK/fsk.h>
#include "MK64F12.h"
#include "gpio.h"


#define FREQ_1      1200.0
#define FREQ_0      2200.0
#define T_BIT       (1.0/FREQ_1)

#define FSK_START   0
#define FSK_STOP    1
#define T0 			20833		//1/f_0 * f_50mhz - 1
#define SYMBOL_TOL	12

fsk_v2_callback_t my_fsk_v2_callback;
void hola();
void fsk_rx_process_sample_v2(uint16_t elapsed_time);


void fsk_rx_init(fsk_v2_callback_t fsk_callback){
	my_fsk_v2_callback = fsk_callback;

	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	//Pin CMP0_OUT	out
	PORTC->PCR[5] = 0x00;
	PORTC->PCR[5] |= PORT_PCR_MUX(0x06);

	//Pin CMP0_IN1	in
	PORTC->PCR[7] = 0x00;
	PORTC->PCR[7] |= PORT_PCR_MUX(0x00);

	cmp_init(CMP_MOD0);

	cmp_dac_conf_t conf_dac = {.dac_enable=true, .module=CMP_MOD0, .digital_input=0x01, .reference_voltage_source=31};

	cmp_mux_conf_t mux_conf = {.minus_input_mux_control=CMP_IN7, .plus_input_mux_control=CMP_IN1};

	cmp_conf_t conf = {.high_power=true, .enable_output_pin=true, .hysteresis=CMP_HYST3, .invert_comparison=false,
						.comparator_output_unfiltered=false, .mux_conf=mux_conf, .dma_enable=false,
						.sample_enable=true, .filter_sample_count=200, .filter_sample_period=200};

	cmp_set_mod_conf(conf, conf_dac);

	cmp_enable_interrupt_type(CMP_MOD0, true, hola, CMP_FALLING);

	//pin PTC1
	ftm_init(FTM_0, FTM_PSC_x1);
	ftm_input_capture_config_t input_conf = {.channel=FTM_CHNL_0, .mod=((1<<16)-1),.mode=FTM_IC_RISING_EDGE, .filter_value=0x00, .callback=fsk_rx_process_sample_v2};
	ftm_set_input_capture_conf(FTM_0, input_conf);
	ftm_conf_port(FTM_0, FTM_CHNL_0);
	ftm_enable_clock(FTM_0, true);


	gpioMode(PORTNUM2PIN(PC, 16), OUTPUT); // debug
}

void fsk_rx_process_sample_v2(uint16_t elapsed_time){
    static bool idle = true;
    static uint32_t curr_bit = 0;
    static uint8_t word_received = 0;
    static bool curr_parity = true;
    static bool ignore_next = false;


    gpioWrite(PORTNUM2PIN(PC, 16), HIGH);
    ftm_reset_counter_value(FTM_0);
    if (idle) {
    	if(elapsed_time <= T0 + SYMBOL_TOL){	//check if counter_value <= T0 + tolerance
			idle = false;
			curr_bit = 0;
			word_received = 0;
			curr_parity = true;
			ignore_next = true;
    	}
    }
    else {
    	if( !ignore_next ){
        	bool newest_bit = (elapsed_time > T0 + SYMBOL_TOL);
    		ignore_next = !newest_bit;
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
					my_fsk_v2_callback(temp);
				}
				idle = true;
			}
			else {
				word_received <<= 1U;
				word_received |= newest_bit;
				curr_parity ^= newest_bit;
			}
    	}
    	else {
    		ignore_next = false;
    	}
    }

    gpioWrite(PORTNUM2PIN(PC,16), LOW);
//	ftm_reset_counter_value(FTM_0);
}

void hola(){

}

