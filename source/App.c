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
#include <string.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_ID			0
#define UART_BAUD_RATE	9600

#define BUFFER_SIZE		255


/******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


//static uint8_t buffer[BUFFER_SIZE];

// extern uint16_t adc_result;

void reset(uint16_t count_value);
//static clock_t clk;
/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){
//	uart_cfg_t config;
//	config.baudrate = UART_BAUD_RATE;
//	config.parity = true;
//	config.odd_parity = true;
//	uartInit(UART_ID, config);
//	clock_init();
//	clk = get_clock();
	ftm_init(FTM_0, FTM_PSC_x1);

//	int mod = (50000000 / (2*98400*(1<<FTM_PSC_x1)) );
//	ftm_pwm_config_t pwm_conf = {.channel = FTM_CHNL_0, .enable_dma=false, .mod=mod, .CnV=(uint16_t)(mod/2), .mode=FTM_PWM_CENTER_ALIGNED};
//	ftm_set_pwm_conf(FTM_0, pwm_conf);
//	ftm_conf_port(FTM_0, pwm_conf.channel);

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

/*******************************************************************************
 ******************************************************************************/
