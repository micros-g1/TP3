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


static uint8_t buffer[BUFFER_SIZE];

// extern uint16_t adc_result;


static clock_t clk;
/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){
	uart_cfg_t config;
	config.baudrate = UART_BAUD_RATE;
	config.parity = true;
	config.odd_parity = true;
	uartInit(UART_ID, config);
	clock_init();
	clk = get_clock();
//	ftm_init(FTM_0, FTM_PSC_x1);
//
//	int mod = (50000000 / (2*98400*(1<<FTM_PSC_x1)) );
//	ftm_pwm_config_t pwm_conf = {.channel = FTM_CHNL_0, .enable_dma=false, .mod=mod, .CnV=(uint16_t)(mod/2), .mode=FTM_PWM_CENTER_ALIGNED};
//	ftm_set_pwm_conf(FTM_0, pwm_conf);
//	ftm_conf_port(FTM_0, pwm_conf.channel);
//	ftm_start_stop_clock(FTM_0, true);

}


void App_Run (void)
{

	if (uartIsRxMsg(UART_ID)) {
		uint8_t n = uartReadMsg(UART_ID, buffer, BUFFER_SIZE);
		fskWriteMsg(buffer, n);
	}

	if (fskIsRxMsg()) {
		uint8_t n = fskReadMsg(buffer, BUFFER_SIZE);
		uartWriteMsg(UART_ID, buffer, n);
	}

//	clock_t now = get_clock();
//	if((now-clk)/CLOCKS_PER_SECOND >= 1) {
//		buffer[0] = '4';
//		buffer[1] = '\r';
//		buffer[2] = '\n';
//		itoa(adc_result, buffer, 10);
//		uint32_t n = strlen(buffer);
//		buffer[n] = '\r';
//		buffer[n+1] = '\n';
//		buffer[n+2] = '\0';
//		uartWriteMsg(UART_ID, buffer, n+2);
//		clk = now;
//	}

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
