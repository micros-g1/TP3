/*
 * cmp.c
 *
 *  Created on: 31 oct. 2019
 *      Author: Tomas
 */

#include "cmp.h"
#include "MK64F12.h"
#include <stdbool.h>
#include <stddef.h>

static CMP_Type * modules[CMP_AMOUNT_MODS] = {CMP0, CMP1, CMP2};

static void cmp_enable_rising_interrupts(cmp_modules_t module, bool enable_disable);
static void cmp_enable_falling_interrupts(cmp_modules_t module, bool enable_disable);

typedef struct{
	bool callback_enabled;
	cmp_callback_t callback;
}edges_interrupts_t;

static edges_interrupts_t interrupts_info[CMP_AMOUNT_MODS][CMP_AMOUNT_INT_TYPES];
static void run_interrupt_callback(edges_interrupts_t interrupt);

void cmp_init(cmp_modules_t module){
	static bool initialized = false;

	if(!initialized) {

		SIM->SCGC4 |= SIM_SCGC4_CMP_MASK;

		edges_interrupts_t int_info = {.callback_enabled = false, .callback = NULL};
		for (int i =0; i < CMP_AMOUNT_MODS; i++)
			for (int j = 0; j < CMP_AMOUNT_INT_TYPES; ++j)
				interrupts_info[i][j] = int_info;

		modules[module]->CR1 |= CMP_CR1_EN_MASK;
//		SIM->SOPT4 |= SIM_SOPT4_FTM1CH0SRC(1);

		initialized = true;
	}

	switch (module) {
	case 0:	NVIC_EnableIRQ(CMP0_IRQn);	break;
	case 1: NVIC_EnableIRQ(CMP1_IRQn);	break;
	case 2: NVIC_EnableIRQ(CMP2_IRQn);	break;
	default:							break;
	}
	//NVIC_EnableIRQ(((uint32_t *) CMP_IRQS)[module]);

}

void cmp_set_mod_conf(cmp_conf_t conf, cmp_dac_conf_t dac_conf){
	CMP_Type* curr_cmp = modules[conf.module];

	//curr_cmp->CR1 |= CMP_CR1_PMODE_MASK;

	if(conf.enable_output_pin)		//TODO: permitir modificar. pone la salida en el pin
		curr_cmp->CR1 |= CMP_CR1_OPE_MASK;
	//		SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;	//Clock Gating

			/* send output to FTM1-CH0 */
	//		SIM->SOPT4 &= ~SIM_SOPT4_FTM1CH0SRC_MASK;
	if(conf.invert_comparison)
		curr_cmp->CR1 |= CMP_CR1_INV_MASK;

	if(conf.comparator_output_unfiltered)
		curr_cmp->CR1 |= CMP_CR1_COS_MASK;

	curr_cmp->CR0 |= CMP_CR0_HYSTCTR(conf.hysteresis);

	curr_cmp->MUXCR = CMP_MUXCR_PSEL(conf.mux_conf.plus_input_mux_control) | CMP_MUXCR_MSEL(conf.mux_conf.minus_input_mux_control);

	cmp_set_dac_conf(dac_conf);

	curr_cmp->CR0 &= ~CMP_CR0_FILTER_CNT_MASK;
	curr_cmp->CR0 |= CMP_CR0_FILTER_CNT(conf.filter_sample_count);
	curr_cmp->FPR = conf.filter_sample_period;
}


bool cmp_get_rising_flag(cmp_modules_t module){
	return modules[module]->SCR & CMP_SCR_CFR_MASK;
}
bool cmp_get_falling_flag(cmp_modules_t module){
	return modules[module]->SCR & CMP_SCR_CFF_MASK;
}

void cmp_enable_interrupt_type(cmp_modules_t module, bool enable_disable, cmp_callback_t callback, cmp_interrupt_types int_type){
	edges_interrupts_t interrupt_conf = {.callback_enabled = enable_disable, .callback = callback};
	interrupts_info[module][int_type] = interrupt_conf;

	if(int_type == CMP_FALLING)
		cmp_enable_falling_interrupts(module, enable_disable);
	else if(int_type == CMP_RISING)
		cmp_enable_rising_interrupts(module, enable_disable);

}
static void cmp_enable_rising_interrupts(cmp_modules_t module, bool enable_disable){
	if(enable_disable)
		modules[module]->SCR |= CMP_SCR_IER_MASK;
	else
		modules[module]->SCR &= ~CMP_SCR_IER_MASK;
}
static void cmp_enable_falling_interrupts(cmp_modules_t module, bool enable_disable){
	if(enable_disable)
		modules[module]->SCR |= CMP_SCR_IEF_MASK;
	else
		modules[module]->SCR &= ~CMP_SCR_IEF_MASK;
}
void cmp_enable_module(cmp_modules_t module, bool enable_disable){
	if(enable_disable)
		modules[module]->CR1 |= CMP_CR1_EN_MASK;
	else
		modules[module]->CR1 &= ~CMP_CR1_EN_MASK;
}

bool cmp_get_output(cmp_modules_t module){
	return modules[module]->SCR & CMP_SCR_COUT_MASK;
}
static void run_interrupt_callback(edges_interrupts_t interrupt){
	if(interrupt.callback_enabled && interrupt.callback != NULL)
		interrupt.callback();
}

void cmp_set_dac_conf(cmp_dac_conf_t conf){
	modules[conf.module]->DACCR = CMP_DACCR_DACEN(1) | CMP_DACCR_VRSEL(conf.digital_input) | CMP_DACCR_VOSEL(conf.reference_voltage_source);
}

void CMP0_IRQHandler(){
	if(modules[CMP_MOD0]->SCR & CMP_SCR_CFR_MASK){						//get flag value
		modules[CMP_MOD0]->SCR &= ~CMP_SCR_CFR_MASK;					//reset flag
		run_interrupt_callback(interrupts_info[CMP_MOD0][CMP_RISING]);		//execute interruption
	}
	else if(modules[CMP_MOD0]->SCR & CMP_SCR_CFF_MASK){					//get flag value
		modules[CMP_MOD0]->SCR &= ~CMP_SCR_CFF_MASK;						//reset flag
		run_interrupt_callback(interrupts_info[CMP_MOD0][CMP_FALLING]);		//execute interruption
	}
}
void CMP1_IRQHandler(){
	if(modules[CMP_MOD1]->SCR & CMP_SCR_CFR_MASK){					//get flag value
		modules[CMP_MOD1]->SCR &= ~CMP_SCR_CFR_MASK;				//reset flag
		run_interrupt_callback(interrupts_info[CMP_MOD1][CMP_RISING]); //execute interruption
	}
	else if(modules[CMP_MOD1]->SCR & CMP_SCR_CFF_MASK){				//get flag value
		modules[CMP_MOD1]->SCR &= ~CMP_SCR_CFF_MASK;					//reset flag
		run_interrupt_callback(interrupts_info[CMP_MOD1][CMP_FALLING]);	//execute interruption
	}
}
void CMP2_IRQHandler(){
	if(modules[CMP_MOD2]->SCR & CMP_SCR_CFR_MASK){		//get flag value
		modules[CMP_MOD2]->SCR &= ~CMP_SCR_CFR_MASK;	//reset flag
		run_interrupt_callback(interrupts_info[CMP_MOD2][CMP_RISING]);	//execute interruption
	}
	else if(modules[CMP_MOD2]->SCR & CMP_SCR_CFF_MASK){	//get flag value
		modules[CMP_MOD2]->SCR &= CMP_SCR_CFF_MASK;		//reset flag
		run_interrupt_callback(interrupts_info[CMP_MOD2][CMP_FALLING]);		//execute interruption
	}
}




