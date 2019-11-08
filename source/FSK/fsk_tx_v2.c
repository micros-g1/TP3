/*
 * fsk_tx_v2.c
 *
 *  Created on: Nov 7, 2019
 *      Author: Rocío Parra
 */


#include <stdbool.h>
#include <math.h>
#include "FTM/flex_timer.h"
#include "PIT/pit.h"
#include "FSK/fsk_tx_v2.h"
#include "hardware.h"

fsk_tx_next_bit_callback callback = NULL;

#define BAUDRATE	1200
#define F_S			24000
#define F_PWM		220000

#define N_SAMPLES	F_S/BAUDRATE
static uint16_t waveform[N_SAMPLES];



void fsk_tx_handler(void);
float map_to_range(float source_min, float source_max, float dest_min, float dest_max, float x);


void fsk_tx_init(fsk_tx_next_bit_callback next_bit_callback)
{
	static bool isinit = false;
	if (isinit)
		return;
	isinit = true;

	callback = next_bit_callback;
	for (unsigned int i = 0; i < N_SAMPLES; i++) {
		float w = sin(((float)2*M_PI*BAUDRATE*i)/(float)F_S);
		w = map_to_range(-1, 1, 0, 100, w);
		waveform[i] = (uint16_t)w;
	}

	ftm_init(FTM_2, FTM_PSC_x1);
	ftm_pwm_config_t pwm_conf = {
		.channel = FTM_CHNL_0,
		.mod=(__CORE_CLOCK__/2)/F_PWM -1,
		.CnV=0,
		.mode=FTM_PWM_EDGE_ALIGNED
	};

	ftm_enable_clock(FTM_2, true);
	ftm_conf_port(FTM_2, FTM_CHNL_0);
	ftm_set_pwm_conf(FTM_2, pwm_conf);

	pit_init();
	pit_conf_t pit_conf = {
		.chain_mode = false,
		.channel = PIT_CH0,
		.timer_interrupt_enable = true,
		.timer_enable = true,
		.timer_count = (__CORE_CLOCK__/2)/F_S -1,
		.callback = fsk_tx_handler
	};
	pit_set_channel_conf(pit_conf);
}

void fsk_tx_interrupt_enable(bool ie)
{
	;
}

void fsk_tx_handler(void)
{
	static uint32_t count = 0;
	static bool bit = true; // send idle by default
	static uint32_t step = 1;
	static uint32_t i = 0;

	if (++count == N_SAMPLES) {
		bit = callback();
		step = bit ? 1 : 2;
		count = 0;
	}

	ftm_set_pwm_duty_cycle(FTM_2, FTM_CHNL_0, waveform[i]);
	i += step;
	i = i < N_SAMPLES ? i : i - N_SAMPLES;
}

float map_to_range(float source_min, float source_max, float dest_min, float dest_max, float x)
{
	float ret = dest_min + (dest_max-dest_min)/(source_max-source_min) * (x - source_min);
	return ret;
}
