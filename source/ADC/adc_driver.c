#include "adc_driver.h"
#include "stdlib.h"

adc_conversion_completed_callback_t conv_completed_callback = NULL;

void adc_init(){
	/* Clock Gating */
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

	/* Input channel select */
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;

	/* Enable COCO Interrupt*/
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;

	/* 16 bit conversion */
	ADC0->CFG1 |= ADC_CFG1_MODE_MASK;

	/* Calibration */
	ADC0->SC3 |= ADC_SC3_CAL_MASK;
	while(ADC0->SC3 & ADC_SC3_CALF_MASK);

	/* Harware average enable */
	ADC0->SC3 |= ADC_SC3_AVGE_MASK;

	/* 8 samples average */
	ADC0->SC3 |= ADC_SC3_AVGS(0b01);

	/* Enable Interrupts */
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);
}

void adc_enable_dma(bool enable){
	if(enable)
		ADC0->SC2 |= ADC_SC2_DMAEN_MASK;
	else
		ADC0->SC2 &= ~ADC_SC2_DMAEN_MASK;
}

bool adc_conversion_completed(){
	return (ADC0->SC1[0] & ADC_SC1_COCO_MASK);
}

bool adc_is_converting(){
	return (ADC0->SC2 & ADC_SC2_ADACT_MASK);
}

void adc_trigger_select(adc_trigger_enum trigger_type){
	if (trigger_type == ADC_HARDWARE_TRIGGER)
		ADC0->SC2 |= ADC_SC2_ADTRG_MASK;
	else if (trigger_type == ADC_HARDWARE_TRIGGER)
		ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;
}

uint16_t adc_get_data(){
	return (uint16_t)ADC0->R[0];
}

void adc_trigger_conversion(){
	/* Input channel select */
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;
}

uint32_t adc_data_result_address(){
	uint32_t ret = (uint32_t)&(ADC0->R[0]);
	return ret;
}

uint32_t adc_get_sc1_address(){
	return (uint32_t)&(ADC0->SC1[0]);
}

void adc_enable_continous_conversion(bool enable){
	if (enable)
		ADC0->SC3 |= ADC_SC3_ADCO_MASK;
	else
		ADC0->SC3 &= ~ADC_SC3_ADCO_MASK;
}

void adc_set_conversion_completed_handler(adc_conversion_completed_callback_t callback){
	conv_completed_callback = callback;
}

void ADC0_IRQHandler(void){
	if(conv_completed_callback != NULL)
		conv_completed_callback();
}

void adc_set_interrupts_enabled(bool ie)
{
	if(ie)
		NVIC_EnableIRQ(ADC0_IRQn);
	else
		NVIC_DisableIRQ(ADC0_IRQn);
}
