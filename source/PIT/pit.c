/*
 * pit.c
 *
 *  Created on: 27 oct. 2019
 *      Author: Tomas
 */

#include <PIT/pit.h>
#include "MK64F12.h"
#include <stddef.h>

static pit_callback irq_callbacks[AMOUNT_PIT_CH] = {NULL, NULL, NULL, NULL};

void pit_init(){
	static bool initialized = false;
	if(!initialized) return;

	SIM->SCGC6 |= SIM_SCGC6_PIT(1);		//clock gating

	/*	Module Disable - (PIT section)
		Disables the standard timers. This field must be enabled before any other setup is done.
		0 Clock for standard PIT timers is enabled.
		1 Clock for standard PIT timers is disabled.*/
	PIT->MCR = PIT_MCR_MDIS(1);

	initialized = true;
}

void pit_set_channel_conf(pit_conf_t conf){

		PIT->CHANNEL[conf.channel].TCTRL = 0x00;
		PIT->CHANNEL[conf.channel].TCTRL = PIT_TCTRL_CHN(conf.chain_mode)| PIT_TCTRL_TIE(conf.timer_interrupt_enable) |
											PIT_TCTRL_TEN(conf.timer_enable);

		/*		Timer Start Value
				Sets the timer start value. The timer will count down until it reaches 0, then it will generate an interrupt and
				load this register value again. Writing a new value to this register will not restart the timer; instead the
				value will be loaded after the timer expires. To abort the current cycle and start a timer period with the new
				value, the timer must be disabled and enabled again.*/
		PIT->CHANNEL[conf.channel].LDVAL = PIT_LDVAL_TSV(conf.timer_count);
}

inline uint32_t pit_get_curr_timer_value(pit_channels_t channel){
	return PIT->CHANNEL[channel].CVAL;
}

inline bool pit_get_timer_interrupt_flag(pit_channels_t channel){
	return PIT->CHANNEL[channel].TFLG & PIT_TFLG_TIF_MASK;
}
inline void pit_clear_timer_interrupt_flag(pit_channels_t channel){
	PIT->CHANNEL[channel].TFLG |= PIT_TFLG_TIF(1);		//write one to clear
}

void PIT0_IRQHandler(void){
	if(irq_callbacks[PIT_CH0] != NULL)
		irq_callbacks[PIT_CH0]();
}
void PIT1_IRQHandler(void){
	if(irq_callbacks[PIT_CH1] != NULL)
		irq_callbacks[PIT_CH1]();

}
void PIT2_IRQHandler(void){
	if(irq_callbacks[PIT_CH2] != NULL)
		irq_callbacks[PIT_CH2]();
}
void PIT3_IRQHandler(void){
	if(irq_callbacks[PIT_CH3] != NULL)
		irq_callbacks[PIT_CH3]();
}

