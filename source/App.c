/******************************************************************************
  @file     App.c
  @brief    Application functions
  @author	Grupo 1 - Labo de Micros 2019
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "DAC/dac_driver.h"
#include "ADC/adc_driver.h"
#include "VREF/vref_driver.h"
#include "PIT/pit.h"
#include "board.h"
#include "gpio.h"
#include "stdlib.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define M_PI 3.14159265358979323846
/******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void delayLoop(uint32_t veces);
int min(int x, int y);
float map_to_range(float a, float b, float c, float d, float x);
void toggle_pin();
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

uint16_t value;
bool state;
/* Función que se llama una vez, al comienzo del programa */
void App_Init (void)
{
	state = true;
	gpioMode(PIN_LED_BLUE, OUTPUT);
	pit_init();
	pit_conf_t config = {.callback=toggle_pin, .chain_mode=false, .channel=PIT_CH0, .timer_count=0xFFFF, .timer_enable=true, .timer_interrupt_enable=true};
	pit_set_channel_conf(config);
}


void App_Run (void)
{

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static void delayLoop(uint32_t veces)
{
    while (veces--);
}

int min(int x, int y){
  return (x < y) ? x : y;
}

float map_to_range(float a, float b, float c, float d, float x){
	float ret = c + (d-c)/(b-a) * (x - a);
	return ret;
}

void toggle_pin(){
	static bool initialized = false;
	if(initialized) return;
	gpioToggle(PIN_LED_BLUE);
	initialized = true;
}
/*******************************************************************************
 ******************************************************************************/
