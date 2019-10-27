#include "vref_driver.h"
#include "MK64F12.h"

void vref_init(){
	/* Clock gating */
	SIM->SCGC4 |= SIM_SCGC4_VREF(1);

	/* VREF_OUT available for
	 * internal and external use. 100
	 * nF capacitor is required
	 * */
	VREF->SC |= VREF_SC_VREFEN_MASK;
	VREF->SC |= VREF_SC_MODE_LV(0b01);
}
