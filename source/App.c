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

/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){
	fskInit();
	uart_cfg_t config;
	config.baudrate = UART_BAUD_RATE;
	config.parity = true;
	config.odd_parity = true;
	uartInit(UART_ID, config);
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
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
