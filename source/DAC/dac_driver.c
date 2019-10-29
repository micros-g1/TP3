#include <stdlib.h>

#include "dac_driver.h"

#define DAC_DATL_DATA0_WIDTH	8


dac_watermark_callback_t watermark_callback;
dac_read_ptr_bottom_callback_t bottom_callback;
dac_read_ptr_top_callback_t top_callback;

void dac_init(){
	/* Click Gating */
	SIM->SCGC2 |= SIM_SCGC2_DAC0_MASK;

	/* Enable module interrupts */
	NVIC_EnableIRQ(DAC0_IRQn);

	watermark_callback = NULL;
	bottom_callback = NULL;
	top_callback = NULL;
}

void dac_enable(bool enable){
	if(enable)
		DAC0->C0 |= DAC_C0_DACEN_MASK;
	else
		DAC0->C0 &= ~DAC_C0_DACEN_MASK;
}

void dac_select_reference(dac_vref_enum vref){
	if (vref >= DAC_VREF1 && vref <= DAC_VREF2)
			DAC0->C0 |= DAC_C0_DACRFS(vref);
}

void dac_setup_buffer(dac_buffer_config_enum buffer_config){
	switch(buffer_config){
	case DAC_BUFFER_DISABLED:
		DAC0->C1 &= ~DAC_C1_DACBFEN_MASK;
		break;
	case DAC_BUFFER_NORMAL:
		DAC0->C1 |= DAC_C1_DACBFEN_MASK;
		DAC0->C1 &= ~DAC_C1_DACBFMD_MASK;
		DAC0->C1 |= DAC_C1_DACBFMD(0b00);
		break;
	case DAC_BUFFER_SWING:
		DAC0->C1 |= DAC_C1_DACBFEN_MASK;
		DAC0->C1 &= ~DAC_C1_DACBFMD_MASK;
		DAC0->C1 |= DAC_C1_DACBFMD(0b01);
		break;
	case DAC_BUFFER_OTSCAN:
		DAC0->C1 |= DAC_C1_DACBFEN_MASK;
		DAC0->C1 &= ~DAC_C1_DACBFMD_MASK;
		DAC0->C1 |= DAC_C1_DACBFMD(0b10);
		break;
	default:
		break;
	}
}

void dac_write_to_buffer(uint8_t pos, uint16_t data){
	uint8_t upper_limit = DAC0->C2 & DAC_C2_DACBFUP_MASK;
	if(pos >= 0 && pos <= upper_limit){
		DAC0->DAT[pos].DATL = DAC_DATL_DATA0(data);
		DAC0->DAT[pos].DATH = DAC_DATH_DATA1(data >> DAC_DATL_DATA0_WIDTH);
	}
}

void dac_trigger_select(dac_trigger_enum trigger_type){
	if(trigger_type == DAC_HARDWARE_TRIGGER)
		DAC0->C0 &= ~DAC_C0_DACTRGSEL_MASK;
	else
		DAC0->C0 |= DAC_C0_DACTRGSEL_MASK;
}

void dac_trigger(){
	DAC0->C0 |= DAC_C0_DACSWTRG_MASK;
}

void dac_enable_dma(bool enable){
	if(enable)
		DAC0->C1 |= DAC_C1_DMAEN_MASK;
	else
		DAC0->C1 &= ~DAC_C1_DMAEN_MASK;
}

void dac_set_watermark_irq_handler(dac_watermark_callback_t callback){
	watermark_callback = callback;
}

void dac_set_read_pointer_top_irq_handler(dac_read_ptr_top_callback_t callback){
	top_callback = callback;
}

void dac_set_read_pointer_botom_irq_handler(dac_read_ptr_bottom_callback_t callback){
	bottom_callback = callback;
}

void dac_setup_irq(dac_irq_config_t irq_config){
	if (irq_config.watermark_irq)
		DAC0->C0 |= DAC_C0_DACBWIEN_MASK;
	else
		DAC0->C0 &= ~DAC_C0_DACBWIEN_MASK;
	if (irq_config.red_pointer_bottom_flag_irq)
		DAC0->C0 |= DAC_C0_DACBBIEN_MASK;
	else
		DAC0->C0 &= ~DAC_C0_DACBBIEN_MASK;
	if (irq_config.read_pointer_top_flag_irq)
		DAC0->C0 |= DAC_C0_DACBTIEN_MASK;
	else
		DAC0->SR &= ~DAC_C0_DACBTIEN_MASK;
}

void DAC0_IRQHandler(){
	/* Disable interrupts */
	NVIC_DisableIRQ(DAC0_IRQn);

	/* Call handlers */
	if (DAC0->SR & DAC_SR_DACBFWMF_MASK){
		watermark_callback();
		/* Clear flag */
		DAC0->SR &= ~DAC_SR_DACBFWMF_MASK;
	}
	if (DAC0->SR & DAC_SR_DACBFRPTF_MASK){
		top_callback();
		/* Clear flag */
		DAC0->SR &= ~DAC_SR_DACBFRPTF_MASK;
	}
	if (DAC0->SR & DAC_SR_DACBFRPBF_MASK){
		bottom_callback();
		DAC0->SR &= ~DAC_SR_DACBFRPBF_MASK;
	}

	/* Enable interrupts */
	NVIC_EnableIRQ(DAC0_IRQn);
}

uint32_t dac_data0_address(){
	return (uint32_t)&(DAC0->DAT[0].DATL);
}
