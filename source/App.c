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
#include "pit.h"
#include "gpio.h"
#include "board.h"

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
void pin_toogle(void);
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

uint16_t value;

/* Función que se llama una vez, al comienzo del programa */
void App_Init (void)
{
	/* DAC TEST */
	vref_init();
	dac_init();
	dac_setup_buffer(DAC_BUFFER_DISABLED);
	dac_enable(true);
	gpioMode(PIN_LED_BLUE, OUTPUT);
	pit_init();
	pit_conf_t conf = {.callback=pin_toogle, .chain_mode=false, .channel=0,.timer_enable=true, .timer_count=0x01, .timer_interrupt_enable=true};
	pit_set_channel_conf(conf);
}


void App_Run (void)
{

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void pin_toogle(void){
	static bool initialized = false;
	if(initialized) return;

	gpioToggle(PIN_LED_BLUE);

	initialized = true;
}
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

/*******************************************************************************
 ******************************************************************************/
