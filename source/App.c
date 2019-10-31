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
#include "DMA/dma.h"
#include "gpio.h"
#include "board.h"
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
void value_togle(void);
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


uint16_t value;

/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){

	value = 0xFFFFFFFF;
	gpioMode(PIN_LED_BLUE, OUTPUT);

	vref_init();
	dac_init();
	dac_enable(true);
	dac_setup_buffer(DAC_BUFFER_DISABLED);

	adc_init();
	adc_trigger_select(ADC_SOFTWARE_TRIGGER);
	adc_enable_dma(true);

	dma_init();

	/* Configure DMA to trigger ADC conversion */
	dma_mux_conf_t mux_conf1 = {.channel_number=0, .dma_enable=true, .source=58, .trigger_enable=true};
	dma_conf_t conf1 = {.citer=1, .destination_address=0x4003B000, .destination_address_adjustment=0, .destination_data_transfer_size=DMA_32BIT,
						.destination_offset=0, .dma_mux_conf=mux_conf1, .nbytes=sizeof(uint32_t), .source_address=0x4003B000, .source_data_transfer_size=DMA_32BIT,
						.source_offset=0};
	dma_set_config_channel(conf1);


	/* Configure DMA to retrieve ADC value and store ir in DAC */
	dma_mux_conf_t mux_conf2 = {.channel_number=1, .dma_enable=true, .source=40, .trigger_enable=true};
	dma_conf_t conf2 = {.citer=1, .destination_address=0x400CC000, .destination_address_adjustment=0, .destination_data_transfer_size=DMA_16BIT,
						.destination_offset=0, .dma_mux_conf=mux_conf2, .nbytes=sizeof(uint16_t), .source_address=0x400BB010, .source_address_adjustment=0,
						.source_data_transfer_size=DMA_16BIT, .source_offset=0};
	dma_set_config_channel(conf2);


	/* pit */
	pit_init();
	pit_conf_t pit_conf = {.callback=value_toggle, .chain_mode=false, .channel=PIT_CH0, .timer_count=0xEE6B280, .timer_enable=true, .timer_interrupt_enable=true};
	pit_set_channel_conf(pit_conf);


}

void App_Run (void){

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

void value_togle(void){
	gpioToggle(PIN_LED_BLUE);
}

/*******************************************************************************
 ******************************************************************************/
