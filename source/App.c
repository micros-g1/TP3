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
#include <string.h>
#include "math.h"
#include "MK64F12.h"
#include <FSK/fsk_rx_v2.h>
#include <FTM/flex_timer.h>

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
void reset(uint8_t count_value);

/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){
	fsk_rx_v2_init(reset);

}


void App_Run (void)
{

	while(1);

}
void reset(uint8_t count_value){
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
