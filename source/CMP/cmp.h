/*
 * cmp.h
 *
 *  Created on: 31 oct. 2019
 *      Author: Tomas
 */

#ifndef CMP_CMP_H_
#define CMP_CMP_H_
#include <stdint.h>
#include <stdbool.h>

typedef enum {CMP_MOD0, CMP_MOD1, CMP_MOD2, CMP_AMOUNT_MODS} cmp_modules_t;

//number of consecutive samples that must agree in order for the CMP module to determine that its output value should change.
typedef enum {CMP_NO_SAMPLES, CMP_1_SAMPLE, CMP_2_SAMPLES, CMP_3_SAMPLES, CMP_4_SAMPLES,
				CMP_5_SAMPLES, CMP_6_SAMPLES, CMP_7_SAMPLES} cmp_filter_sample_count_t;

//Defines the programmable hysteresis level. The hysteresis values associated with each level are devicespecific.
//See the Data Sheet of the device for the exact values.
typedef enum{CMP_HYST0, CMP_HYST1, CMP_HYST2, CMP_HYST3} cmp_hysteresis_control_level_t;

typedef enum {CMP_IN0, CMP_IN1, CMP_IN2, CMP_IN3, CMP_IN4, CMP_IN5, CMP_IN6, CMP_IN7} cmp_input_mux_control_t;

typedef enum {CMP_FALLING, CMP_RISING, CMP_AMOUNT_INT_TYPES} cmp_interrupt_types;

typedef void (*cmp_callback_t)(void);

typedef struct{
	bool pass_through_mode_enabled;
	cmp_input_mux_control_t plus_input_mux_control;
	cmp_input_mux_control_t minus_input_mux_control;
}cmp_mux_conf_t;

typedef struct{
	cmp_modules_t module;
	cmp_filter_sample_count_t filter_sample_count;
	cmp_hysteresis_control_level_t hysteresis;
	bool sample_enable;
	bool window_enable;
	bool high_power;
	bool invert_comparison;
	bool comparator_output_unfiltered;
	uint8_t filter_sample_period;
	bool dma_enable;
	cmp_mux_conf_t mux_conf;
}cmp_conf_t;

typedef struct{
	cmp_modules_t module;
	bool dac_enable;
	uint8_t reference_voltage_source;
	uint8_t digital_input;
}cmp_dac_conf_t;

void cmp_init(cmp_modules_t module);
// Obsoleta
//void cmp_set_mod_conf(cmp_conf_t conf);
void cmp_set_dac_conf(cmp_dac_conf_t conf);

void cmp_enable_module(cmp_modules_t module, bool enable_disable);
bool cmp_get_falling_flag(cmp_modules_t module);
bool cmp_get_rising_flag(cmp_modules_t module);
void cmp_enable_interrupt_type(cmp_modules_t module, bool enable_disable, cmp_callback_t callback, cmp_interrupt_types int_type);

bool cmp_get_output(cmp_modules_t module);
#endif /* CMP_CMP_H_ */
