#include "adc_driver.h"

void adc_init(){
	/* Clock Gating */
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

	/* Input channel select
	 *  - DADP0
	 * */
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;

	/* 16 bit conversion */
	ADC0->CFG1 |= ADC_CFG1_MODE_MASK;

	/* Calibration */
	ADC0->SC3 |= ADC_SC3_CAL_MASK;
	while(ADC0->SC3 & ADC_SC3_CALF_MASK);

	/* Continous Conversion */
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;

	/* Harware average enable */
	ADC0->SC3 |= ADC_SC3_AVGE_MASK;

	/* 8 samples average */
	ADC0->SC3 |= ADC_SC3_AVGS(0b01);
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
	/* Input channel select
	 *  - DADP0
	 * */
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;
}

uint32_t adc_data_result_address(){
	return (uint32_t)&(ADC0->R[0]);
}

void adc_enable_continous_conversion(bool enable){
	if (enable)
		ADC0->SC3 |= ADC_SC3_ADCO_MASK;
	else
		ADC0->SC3 &= ~ADC_SC3_ADCO_MASK;
}

