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
#include "board.h"
#include "gpio.h"

/*****************************************************
 * *********************DEFINES************************
 *****************************************************/

/*****************************************************
 * *****************STATIC VARIABLES******************
 *****************************************************/
static FTM_Type * const ftms[FTM_AMOUNT_MODULES] = FTM_BASE_PTRS;
static ftm_irq_callback_t irq_callbacks[FTM_AMOUNT_MODULES][FTM_AMOUNT_CHANNELS];
static int const ftm_irqs[FTM_AMOUNT_MODULES] = {FTM0_IRQn, FTM1_IRQn, FTM2_IRQn, FTM3_IRQn};

//for pwm configurations
static uint32_t const pwm_combine_masks[4]= {FTM_COMBINE_COMBINE0_MASK, FTM_COMBINE_COMBINE1_MASK,
							FTM_COMBINE_COMBINE2_MASK, FTM_COMBINE_COMBINE3_MASK};
static uint32_t const pwm_decapen_masks[4] = {FTM_COMBINE_DECAPEN0_MASK, FTM_COMBINE_DECAPEN1_MASK,
		FTM_COMBINE_DECAPEN2_MASK, FTM_COMBINE_DECAPEN3_MASK};
/*****************************************************
 * *****************STATIC FUNCTIONS******************
 *****************************************************/
void write_mod_value(ftm_modules_t module, uint16_t mod_value);
/*****************************************************
 * *****************FUNCTION IMPLEMENTATION***********
 *****************************************************/
void ftm_init(ftm_modules_t module, ftm_prescaler_t prescaler_config){
	static bool initiliazed[FTM_AMOUNT_MODULES];
	if(initiliazed[module]) return;

	if (module == FTM_0)
		SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;	//Clock Gating
	else if(module == FTM_1)
		SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;	//Clock Gating

	else if (module == FTM_2){
		SIM->SCGC6 |= SIM_SCGC6_FTM2_MASK;	//Clock Gating
		SIM->SCGC3 |= SIM_SCGC3_FTM2_MASK;	//Clock Gating
	}
	else if( module == FTM_3)
		SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;	//Clock Gating


	NVIC_ClearPendingIRQ(ftm_irqs[module]);
	NVIC_EnableIRQ(ftm_irqs[module]);

	ftm_set_prescaler(module, prescaler_config);
//	conf_port(module);

	initiliazed[module] = true;
}


void ftm_set_clk_src(ftm_modules_t module, ftm_clk_src_t source){
	ftms[module]->SC = ftms[module]->SC = (ftms[module]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(source);
}

// CONTROL STATUS REGISTER		SC
void ftm_enable_clock(ftm_modules_t module, bool enable){
	// by default enables clk in system clk source
	if(enable)
		ftms[module]->SC = ftms[module]->SC = (ftms[module]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(FTM_SYSTEM_CLK);
	else
		ftms[module]->SC = ftms[module]->SC = (ftms[module]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(0);
}

void ftm_set_prescaler(ftm_modules_t module , ftm_prescaler_t prescaler_config){
	ftms[module]->SC = (ftms[module]->SC & ~FTM_SC_PS_MASK) | FTM_SC_PS(prescaler_config);
}
void ftm_enable_overflow_interrupts(ftm_modules_t module, bool enable_disable){
	ftms[module]->SC = (ftms[module]->SC & ~FTM_SC_TOIE_MASK) | FTM_SC_TOIE(enable_disable);
}
void ftm_up_up_downcpwms(ftm_modules_t module, bool up_up_down){
	ftms[module]->SC = (ftms[module]->SC & ~FTM_SC_CPWMS_MASK) | FTM_SC_CPWMS(up_up_down);
}
bool ftm_has_overflowed(ftm_modules_t module){
	return ftms[module]->SC & FTM_SC_TOF_MASK;
}

// COUNTER REGISTER				CNT
void ftm_reset_counter_value(ftm_modules_t module){
	//Reset clears the CNT register. Writing any value to COUNT updates the counter with its initial value, CNTIN.
	ftms[module]->CNT |= FTM_CNT_COUNT(0x01);
}
uint16_t ftm_read_counter_value(ftm_modules_t module){
	return ftms[module]->CNT & FTM_CNT_COUNT_MASK;
}

//MOD REGISTER					MOD
/*
 The Modulo register contains the modulo value for the FTM counter. After the FTM
counter reaches the modulo value, the overflow flag (TOF) becomes set at the next clock,
and the next value of FTM counter depends on the selected counting method.

Initialize the FTM counter, by writing to CNT, before writing to the MOD register to
avoid confusion about when the first counter overflow will occur.
 */

void write_mod_value(ftm_modules_t module , uint16_t mod_value){
	ftms[module]->MOD = mod_value;
}
uint16_t ftm_get_mod_value(ftm_modules_t module){
	return ftms[module]->MOD;
}

//Channel (n) Status And Control		CnSC


void ftm_set_pwm_conf(ftm_modules_t module, ftm_pwm_config_t config){


	(ftms[module]->CONTROLS[config.channel].CnSC) &= ~FTM_CnSC_DMA_MASK;						//disables dma

	/*	SWSOC
	* Software output control synchronization is activated by the software trigger.
	*	0 The software trigger does not activate the SWOCTRL register synchronization.
	*	1 The software trigger activates the SWOCTRL register synchronization.
	*/
	ftms[module]->SYNCONF |= FTM_SYNCONF_SWWRBUF_MASK | FTM_SYNCONF_SWSOC_MASK;
	/*
	 * PWM Synchronization Software Trigger
	 * Selects the software trigger as the PWM synchronization trigger. The software trigger happens when a 1 is
	 * written to SWSYNC bit.
	 * 0 Software trigger is not selected.
	 * 1 Software trigger is selected.
	 */
	ftms[module]->SYNC |= FTM_SYNC_SWSYNC_MASK;

	// QUADEN = 0
	ftms[module]->QDCTRL &= ~FTM_QDCTRL_QUADEN_MASK;



	ftms[module]->COMBINE &= ~(pwm_decapen_masks[module]|pwm_combine_masks[module]);


	uint32_t SYNCEN_MASK[]= {FTM_COMBINE_SYNCEN0_MASK, FTM_COMBINE_SYNCEN1_MASK,
			FTM_COMBINE_SYNCEN2_MASK, FTM_COMBINE_SYNCEN3_MASK};
	ftms[module]->COMBINE|=SYNCEN_MASK[module];

	if(config.mode == FTM_PWM_CENTER_ALIGNED){
		ftms[module]->CONTROLS[config.channel].CnSC|=FTM_CnSC_ELSB_MASK;
		ftms[module]->SC |= FTM_SC_CPWMS_MASK;		//counting up and down
	}
	else if(config.mode == FTM_PWM_EDGE_ALIGNED){
		ftms[module]->SC &= ~FTM_SC_CPWMS_MASK;			//centered PWM disabled.
		// edge alligned, high true pulses
		ftms[module]->CONTROLS[config.channel].CnSC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	}

	write_mod_value(module , config.mod);
	ftms[module]->CONTROLS[config.channel].CnV=config.CnV;


	ftms[module]->CNTIN = 0;		//resets counter value.

	//set callbacks
//	irq_callbacks[module][config.channel] = config.callback;
}

void ftm_set_pwm_duty_cycle(ftm_modules_t module, ftm_channel_t channel,uint8_t duty_cycle){
	float new_cnv = ((float)duty_cycle / 100.0) * (float)(ftms[module]->MOD & FTM_MOD_MOD_MASK);
	ftms[module]->CONTROLS[channel].CnV = FTM_CnV_VAL((uint16_t)new_cnv);
}

void ftm_set_input_capture_conf(ftm_modules_t module, ftm_input_capture_config_t config){

	ftms[module]->CONTROLS[config.channel].CnSC = FTM_CnSC_ELSA(config.mode & 1) | FTM_CnSC_ELSB((config.mode << 1 )&1);

	if(config.channel <= 3){
		uint32_t filters_masks[4] = {FTM_FILTER_CH0FVAL(config.filter_value), FTM_FILTER_CH1FVAL(config.filter_value),
				FTM_FILTER_CH2FVAL(config.filter_value), FTM_FILTER_CH3FVAL(config.filter_value)};
		ftms[module]->FILTER |= filters_masks[config.channel];
	}

	//CALLBACK
	irq_callbacks[module][config.channel] = config.callback;

	ftms[module]->CONTROLS[config.channel].CnSC &= ~FTM_CnSC_DMA_MASK;
	ftms[module]->CONTROLS[config.channel].CnSC |= FTM_CnSC_CHIE_MASK;

	ftms[module]->CNTIN=0;		//resets the counter
	write_mod_value(module, config.mod);
}

void ftm_conf_port(ftm_modules_t module, ftm_channel_t channel){
	if(module == FTM_0){
		SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

		PORTC->PCR[1] = PORT_PCR_SRE(0) | PORT_PCR_PFE(0) |
				PORT_PCR_ODE(0) | PORT_PCR_DSE(0) | PORT_PCR_PS (2U) |
				PORT_PCR_MUX(4) | PORT_PCR_LK (0) | PORT_PCR_IRQC(0);

		// Enable or disable internal pull resistor
		PORTC->PCR[1] &= ~PORT_PCR_PE_MASK;
	}
	else if(module == FTM_2){
		SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
		PORTB->PCR[18] = PORT_PCR_SRE(0) | PORT_PCR_PFE(0) |
						PORT_PCR_ODE(0) | PORT_PCR_DSE(0) | PORT_PCR_PS (2U) |
						PORT_PCR_MUX(3) | PORT_PCR_LK (0) | PORT_PCR_IRQC(0);
	}
}

//IRQS
void FTM0_IRQHandler(void){
	ftms[FTM_0]->CONTROLS[0].CnSC &=  ~FTM_CnSC_CHF_MASK;
	irq_callbacks[FTM_0][0](ftms[FTM_0]->CONTROLS[0].CnV);
}

void FTM1_IRQHandler(void){
	ftms[FTM_1]->CONTROLS[1].CnSC &=  ~FTM_CnSC_CHF_MASK;
	irq_callbacks[FTM_1][0](ftms[FTM_1]->CONTROLS[0].CnV);
}

void FTM2_IRQHandler(void){
	ftms[FTM_2]->CONTROLS[1].CnSC &=  ~FTM_CnSC_CHF_MASK;
	irq_callbacks[FTM_2][0](ftms[FTM_2]->CONTROLS[0].CnV);
}
