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
#include "math.h"

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
	adc_init();
	adc_enable_continous_conversion(true);
	adc_trigger_select(ADC_SOFTWARE_TRIGGER);
	adc_trigger_conversion();
	dac_init();
	dac_setup_buffer(DAC_BUFFER_DISABLED);
	dac_enable(true);
}


void App_Run (void)
{
	if(adc_conversion_completed()){
		uint16_t adc_data = adc_get_data();
		uint16_t dac_data = map_to_range(0, ADC_MAX_DIGITAL_VALUE, 0, DAC_MAX_DIGITAL_VALUE, adc_data);
		dac_write_to_buffer(0, dac_data);
	}
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

/*******************************************************************************
 ******************************************************************************/
