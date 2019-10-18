/*
 * flex_timer.c
 *
 *  Created on: 18 oct. 2019
 *      Author: Tomas
 */

/*****************************************************
 * *********************INCLUDES**********************
 *****************************************************/
#include "flex_timer.h"

/*****************************************************
 * *********************DEFINES************************
 *****************************************************/

/*****************************************************
 * *****************STATIC VARIABLES******************
 *****************************************************/
static FTM_Type * FTMs[FTM_AMOUNT_MODULES] = FTM_BASE_PTRS;
static ftm_irq_callback_t irq_callbacks[FTM_AMOUNT_MODULES];

/*****************************************************
 * *****************STATIC FUNCTIONS******************
 *****************************************************/

/*****************************************************
 * *****************FUNCTION IMPLEMENTATION***********
 *****************************************************/
void ftm_init(ftm_modules_t module, ftm_irq_callback_t callback, ftm_prescaler_t prescaler_config){
	static bool initiliazed = false;
	if(initiliazed) return;

	if (module == FTM_0){
		SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;	//Clock Gating
		NVIC_ClearPendingIRQ(FTM0_IRQn);	//IRQS
		NVIC_EnableIRQ(FTM0_IRQn);
	}
	else if(module == FTM_1){
		SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;	//Clock Gating
		NVIC_ClearPendingIRQ(FTM1_IRQn);	//IRQS
		NVIC_EnableIRQ(FTM1_IRQn);
	}
	else if (module == FTM_2){
		SIM->SCGC6 |= SIM_SCGC6_FTM2_MASK;	//Clock Gating
		SIM->SCGC3 |= SIM_SCGC3_FTM2_MASK;	//Clock Gating
		NVIC_ClearPendingIRQ(FTM2_IRQn);	//IRQS
		NVIC_EnableIRQ(FTM2_IRQn);
	}
	else if( module == FTM_3){
		SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;	//Clock Gating
		NVIC_ClearPendingIRQ(FTM3_IRQn);	//IRQS
		NVIC_EnableIRQ(FTM3_IRQn);
	}
	irq_callbacks[module] = callback;
	ftm_set_prescaler(module, prescaler_config);

	initiliazed = true;
}

// CONTROL STATUS REGISTER		SC
void ftm_start_stop_clock(ftm_modules_t module, bool start_stop){
	FTMs[module]->SC = (FTMs[module]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(start_stop);
}
void ftm_set_prescaler(ftm_modules_t module , ftm_prescaler_t prescaler_config){
	FTMs[module]->SC = (FTMs[module]->SC & ~FTM_SC_PS_MASK) | FTM_SC_PS(prescaler_config);
}
void ftm_enable_overflow_interrupts(ftm_modules_t module, bool enable_disable){
	FTMs[module]->SC = (FTMs[module]->SC & ~FTM_SC_TOIE_MASK) | FTM_SC_TOIE(enable_disable);
}
void ftm_up_up_downcpwms(ftm_modules_t module, bool up_up_down){
	FTMs[module]->SC = (FTMs[module]->SC & ~FTM_SC_CPWMS_MASK) | FTM_SC_CPWMS(up_up_down);
}
bool ftm_has_overflowed(ftm_modules_t module){
	return FTMs[module]->SC & FTM_SC_TOF_MASK;
}

// COUNTER REGISTER				CNT
void ftm_reset_counter_value(ftm_modules_t module){
	//Reset clears the CNT register. Writing any value to COUNT updates the counter with its initial value, CNTIN.
	FTMs[module]->CNT |= FTM_CNT_COUNT(0x01);
}
uint16_t ftm_read_counter_value(ftm_modules_t module){
	return FTMs[module]->CNT & FTM_CNT_COUNT_MASK;
}



//IRQS
void FTM0_IRQHandler(void){
	irq_callbacks[FTM_0]();
}
void FTM1_IRQHandler(void){
	irq_callbacks[FTM_1]();
}
void FTM2_IRQHandler(void){
	irq_callbacks[FTM_2]();
}

