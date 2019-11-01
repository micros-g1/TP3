/*
 * flex_timer.h
 *
 *  Created on: 18 oct. 2019
 *      Author: Tomas
 */

#ifndef ftm_FLEX_TIMER_H_
#define ftm_FLEX_TIMER_H_
#include "MK64F12.h"
#include "stdbool.h"
#include "stdlib.h"

typedef void (*ftm_irq_callback_t)(uint16_t capture_value);
typedef enum{FTM_0, FTM_1, FTM_2, FTM_3, FTM_AMOUNT_MODULES} ftm_modules_t;

typedef enum{ FTM_PSC_x1= 0x00, FTM_PSC_x2= 0x01, FTM_PSC_x4= 0x02,
				FTM_PSC_x8= 0x03, FTM_PSC_x16= 0x04,FTM_PSC_x32= 0x05,
					FTM_PSC_x64= 0x06, FTM_PSC_x128= 0x07 } ftm_prescaler_t;

typedef enum{FTM_PWM_CENTER_ALIGNED, FTM_PWM_EDGE_ALIGNED, FTM_PWM_COMBINED }ftm_pwm_mode_t;

typedef enum{ FTM_CHNL_0, FTM_CHNL_1, FTM_CHNL_2, FTM_CHNL_3, FTM_CHNL_4, FTM_CHNL_5, FTM_CHNL_6, FTM_CHNL_7, FTM_AMOUNT_CHANNELS}ftm_channel_t;

typedef enum{ FTM_IC_RISING_EDGE=1, FTM_IC_FALLING_EDGE, FTM_IC_BOTH_EDGES}ftm_input_capture_mode;

typedef enum{
	FTM_SYSTEM_CLK = 0x01,
	FTM_FIXED_FREQ_CLK = 0x02,
	FTM_EXTERN_CLK = 0x03
}ftm_clk_src_t;

typedef struct{
	ftm_channel_t channel;
	ftm_pwm_mode_t mode;
	uint16_t mod;
	uint8_t CnV;
	ftm_pwm_callback_t callback;
}ftm_pwm_config_t;

typedef struct {
	ftm_channel_t channel;
	ftm_irq_callback_t callback;
	uint16_t mod;
	ftm_input_capture_mode mode;
	uint32_t filter_value;		//Only channels 0,1,2 and 3 can have filter
}ftm_input_capture_config_t;



void ftm_init(ftm_modules_t module, ftm_prescaler_t prescaler_config);
void ftm_set_clk_src(ftm_modules_t module, ftm_clk_src_t source);

void ftm_enable_clock(ftm_modules_t module, bool enable);
void ftm_enable_overflow_interrupts(ftm_modules_t module, bool enable_disable);
void ftm_set_prescaler(ftm_modules_t module , ftm_prescaler_t prescaler_config);
bool ftm_has_overflowed(ftm_modules_t module);

void ftm_reset_counter_value(ftm_modules_t module);
uint16_t ftm_read_counter_value(ftm_modules_t module);
uint16_t ftm_get_mod_value(ftm_modules_t module);

void ftm_set_pwm_conf(ftm_modules_t module, ftm_pwm_config_t config);
void ftm_set_pwm_duty_cycle(ftm_modules_t module, ftm_channel_t channel,uint8_t duty_cycle);
void ftm_set_input_capture_conf(ftm_modules_t module, ftm_input_capture_config_t config);

void ftm_conf_port(ftm_modules_t module, ftm_channel_t channel);

#endif /* ftm_FLEX_TIMER_H_ */
