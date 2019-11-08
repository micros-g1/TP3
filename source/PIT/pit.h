/*
 * pit.h
 *
 *  Created on: 27 oct. 2019
 *      Author: Tomas
 */

#ifndef PIT_PIT_H_
#define PIT_PIT_H_

#include <stdbool.h>
#include <stdint.h>

#define PIT_CLOCK_FREQUENCY 50000000UL

typedef void(*pit_callback)(void);
typedef enum {PIT_CH0, PIT_CH1, PIT_CH2, PIT_CH3, AMOUNT_PIT_CH} pit_channels_t;
typedef struct{
	bool chain_mode;
	pit_channels_t channel;
	bool timer_interrupt_enable;
	bool timer_enable;
	uint32_t timer_count;
	pit_callback callback;
}pit_conf_t;

void pit_init();
void pit_set_channel_conf(pit_conf_t conf);
void pit_set_new_timer_countdown(pit_channels_t channel, uint32_t countdown);
uint32_t pit_get_curr_timer_value(pit_channels_t channel);
bool pit_get_timer_interrupt_flag(pit_channels_t channel);
void pit_clear_timer_interrupt_flag(pit_channels_t channel);
void pit_set_timer_interrupt_enabled(pit_channels_t channel, bool enabled_disabled);

#endif /* PIT_PIT_H_ */
