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
#include "math.h"
#include "stdbool.h"

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
void value_toggle(void);
static void change_frequency();
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

#define N_POINTS 256

uint16_t value;
uint16_t source;
uint16_t sin_array[N_POINTS] __attribute__ ((aligned(N_POINTS)));
uint16_t dest_array[N_POINTS];


/* Función que se llama una vez, al comienzo del programa */
void App_Init (void){
	vref_init();

	float h = 2*M_PI/N_POINTS;
	for (int i = 0; i < N_POINTS; i++){
		float value = map_to_range(-1, 1, 0, DAC_MAX_DIGITAL_VALUE, sin(h*i));
		sin_array[i] = (uint16_t)value;
		dest_array[i]=0;
	}

	dac_init();
	dac_enable(true);
	dac_setup_buffer(DAC_BUFFER_DISABLED);

	adc_init();
	adc_trigger_select(ADC_SOFTWARE_TRIGGER);
	adc_enable_dma(true);

	dma_init();
//	/* Configure DMA to trigger ADC conversion */
//	dma_mux_conf_t mux_conf2 = {.channel_number=1, .dma_enable=true, .source=59, .trigger_enable=true};
//	dma_conf_t conf2 = {.citer=1, .destination_address=adc_get_sc1_address(), .destination_address_adjustment=0, .destination_data_transfer_size=DMA_32BIT,
//						.destination_offset=0, .dma_mux_conf=mux_conf2, .nbytes=sizeof(uint32_t), .source_address=adc_get_sc1_address(), .source_data_transfer_size=DMA_32BIT,
//						.source_offset=0, .source_address_adjustment=0};
//	dma_set_config_channel(conf2);
//
//	/* Configure DMA to write sin values to DAC */
//	dma_mux_conf_t mux_conf1 = {.channel_number=0, .dma_enable=true, .source=58, .trigger_enable=true};
//	dma_conf_t conf1 = {.citer=N_POINTS, .destination_address=dac_data0_address(), .destination_address_adjustment=0, .destination_data_transfer_size=DMA_16BIT,
//						.destination_offset=0, .dma_mux_conf=mux_conf1, .nbytes=sizeof(uint16_t), .source_address=sin_array, .source_data_transfer_size=DMA_16BIT,
//						.source_offset=0, .source_address_adjustment=-N_POINTS*sizeof(uint16_t)};
//	dma_set_config_channel(conf1);

	/* Configure DMA to write sin values to dest_array */
	dma_mux_conf_t mux_conf1 = {.channel_number=0, .dma_enable=true, .source=58, .trigger_enable=true};
	dma_conf_t conf1 = {.citer=1, .destination_address=dac_data0_address(), .destination_address_adjustment=0, .destination_data_transfer_size=DMA_16BIT,
						.destination_offset=0, .dma_mux_conf=mux_conf1, .nbytes=sizeof(uint16_t), .source_address=(uint32_t)sin_array, .source_data_transfer_size=DMA_16BIT,
						.source_offset=sizeof(uint16_t), .source_address_adjustment=0, .smod=log2(N_POINTS)+1, .dmod=0};
	dma_set_config_channel(conf1);

	/* pit */
	pit_init();
	pit_conf_t pit_conf = {.callback=NULL, .chain_mode=false, .channel=PIT_CH0, .timer_count=0x00FF , .timer_enable=true, .timer_interrupt_enable=false};
	pit_set_channel_conf(pit_conf);

	pit_conf_t pit_conf1 = {.callback=change_frequency, .chain_mode=false, .channel=PIT_CH1, .timer_count=41667, .timer_enable=true, .timer_interrupt_enable=true};
	pit_set_channel_conf(pit_conf1);

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

void value_toggle(void){
	gpioToggle(PIN_LED_BLUE);
	source = (source == 0xFFF) ? 0 : 0xFFF;
}

static bool bit = false;
void change_frequency()
{
	bit = !bit;
	if(bit)
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(89);
	else
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(163);
}
/*******************************************************************************
 ******************************************************************************/
