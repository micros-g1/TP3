/*
 * flex_timer.h
 *
 *  Created on: 18 oct. 2019
 *      Author: Tomas
 */

#ifndef ftm_FLEX_TIMER_H_
#define ftm_FLEX_TIMER_H_
#include "MK64F12.h"
#include <stdbool.h>
#include <stdint.h>

typedef void (*ftm_irq_callback_t)(void);
typedef enum{FTM_0, FTM_1, FTM_2, FTM_3, ftm_AMOUNT_MODULES} ftm_modules_t;

typedef enum{ FTM_PSC_x1= 0x00, FTM_PSC_x2= 0x01, FTM_PSC_x4= 0x02,
				FTM_PSC_x8= 0x03, FTM_PSC_x16= 0x04,FTM_PSC_x32= 0x05,
					FTM_PSC_x64= 0x06, FTM_PSC_x128= 0x07 } ftm_prescaler_t;

typedef enum{FTM_PWM_CENTER_ALIGNED, FTM_PWM_EDGE_ALIGNED, FTM_PWM_COMBINED }ftm_pwm_mode_t;

typedef enum{ ftm_CHNL_0, ftm_CHNL_1, ftm_CHNL_2, FTM_CHNL_3, FTM_CHNL_4, FTM_CHNL_5, FTM_CHNL_6, FTM_CHNL_7}ftm_channel_t;

typedef enum{ FTM_IC_RISING_EDGE=1, FTM_IC_FALLING_EDGE, FTM_IC_BOTH_EDGES}ftm_input_capture_mode;

typedef struct{
	ftm_channel_t channel;
	ftm_pwm_mode_t mode;
	bool enableDMA;
	uint16_t mod;
	uint8_t CnV;
}ftm_pwm_config_t;

typedef struct {
	ftm_channel_t channel;
	ftmCaptureFun_t callback;
	uint16_t mod;
	bool enableDMA;
	ftm_input_capture_mode mode;
	uint32_t filterValue;		//Only channels 0,1,2 and 3 can have filter
}ftm_input_capture_config_t;

void ftm_init(ftm_modules_t module, ftm_irq_callback_t callback, ftm_prescaler_t prescaler_config);
void ftm_start_stop_clock(ftm_modules_t module, bool start_stop);
void ftm_enable_overflow_interrupts(ftm_modules_t module, bool enable_disable);
void ftm_set_prescaler(ftm_modules_t module , ftm_prescaler_t prescaler_config);
bool ftm_has_overflowed(ftm_modules_t module);

void ftm_reset_counter_value(ftm_modules_t module);
uint16_t ftm_read_counter_value(ftm_modules_t module);

void ftm_set_pwm_conf(ftm_modules_t module, ftm_pwm_config_t * config);
bool ftm_set_input_capture_conf(ftm_modules_t module, ftm_input_capture_config_t *config);

/**
 * @brief Gives a pointer to the address of the CnV register for the specified instance
 * and channel.
 * @param instance ftm instance used
 * @param channel
 *  */
uint32_t ftm_GetCnVAddress(ftm_modules_t module, ftm_channel_t channel);
uint16_t ftm_GetModValue(ftm_modules_t module);


#endif /* ftm_FLEX_TIMER_H_ */
