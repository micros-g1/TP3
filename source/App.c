/******************************************************************************
  @file     App.c
  @brief    Application functions
  @author	Grupo 1 - Labo de Micros 2019
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "board.h"
#include "FSK/fsk.h"
#include "UART/uart.h"
#include "util/clock.h"
#include <stdlib.h>
#include <FTM/flex_timer.h>
#include "FSK/fsk_rx.h"
#include "FTM/flex_timer.h"
#include <string.h>
#include "math.h"
#include "MK64F12.h"
#include "CMP/cmp.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_ID			0
#define UART_BAUD_RATE	9600

#define BUFFER_SIZE		255
#define N_POINTS		256
#define M_PI 			3.14159265358979323846

/******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

float map_to_range(float source_min, float source_max, float dest_min, float dest_max, float x);
void change_pwm_config();
void push_new_sample_to_pwm();

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


static uint16_t table[N_POINTS];

/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){

	/* PWM TESTS */

//	float h = 2*M_PI/N_POINTS;
//	for (int i = 0; i < N_POINTS; i++){
//		float x = sin(h*i);
//		float y = map_to_range(-1, 1, 0, 100, x);
//		table[i] = (uint16_t)y;
//	}
//
//
//	ftm_init(FTM_0, FTM_PSC_x1);
//
//	int mod = 0xFFF;
//	ftm_pwm_config_t pwm_conf = {.channel = FTM_CHNL_0, .enable_dma=false, .mod=mod, .CnV=0, .mode=FTM_PWM_EDGE_ALIGNED, .callback=push_new_sample_to_pwm};
//	ftm_set_pwm_conf(FTM_0, pwm_conf);
//	//ftm_conf_port(FTM_0, pwm_conf.channel);
//	ftm_enable_overflow_interrupts(FTM_0, true);
//	ftm_enable_clock(FTM_0, true);
//
//	PORTC->PCR[1] = PORT_PCR_MUX(4);

	/* CMP TESTS */
	PORTC->PCR[5] = PORT_PCR_MUX(6);

	cmp_init(CMP_MOD0);
	cmp_dac_conf_t dac_conf = {
			.dac_enable=true,
			.digital_input=0x20,
			.module=CMP_MOD0,
			.reference_voltage_source=0,
	};
	cmp_set_dac_conf(dac_conf);


	ftm_input_capture_config_t input_conf = {.channel=0x00, .mod=((1<<16)-1),.mode=FTM_IC_BOTH_EDGES, .filter_value=0x00, .callback=reset};
	ftm_set_input_capture_conf(FTM_0, input_conf);
	ftm_conf_port(FTM_0, FTM_CHNL_0);
	ftm_start_stop_clock(FTM_0, true);
}


void App_Run (void)
{


	while(1);

}
void reset(uint16_t count_value){
	ftm_reset_counter_value(FTM_0);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
int i = 0;
void push_new_sample_to_pwm(){
	ftm_set_pwm_duty_cycle(FTM_0, FTM_CHNL_0, table[i]);
	i = (i + 1) % N_POINTS;
}

void change_pwm_config(){

}

float map_to_range(float source_min, float source_max, float dest_min, float dest_max, float x){
	float ret = dest_min + (dest_max-dest_min)/(source_max-source_min) * (x - source_min);
	return ret;
}

/*******************************************************************************
 ******************************************************************************/
