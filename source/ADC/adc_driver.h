#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "MK64F12.h"
#include "stdbool.h"

/*
 * Modo de operacion:
 *  - Resolucion 16 bits unsigned single ended.
 *  - Reference Vrefh y Vrefl
 * */

#define ADC_MIN_DIGITAL_VALUE	0
#define ADC_MAX_DIGITAL_VALUE	65535

/**
 * @enum adc_trigger_enum
 * @brief ADC trigger source options
 */
typedef enum{
	ADC_HARDWARE_TRIGGER,	///< @brief Hardware trigger
	ADC_SOFTWARE_TRIGGER	///< @biref Software trigger
}adc_trigger_enum;

typedef void (*adc_conversion_completed_callback_t)(void);

/**
 * @brief ADC initialization
 * @details Initializates ADC module with default settings:
 *  - Interrupts disable
 *  - Single ended conversion
 *  - Input channel DADP0
 *  - 16 bit conversion
 *  - Max sample freq =~ 200kHz
 *  - Continous Conversion
 *  - Software trigger
 */
void adc_init();

/**
 * @brief Enable or disable the DMA interrupts
 * @param enable Boolean value
 *  - true: Enable DMA
 *  - false: Disable DMA
 */
void adc_enable_dma(bool enable);

/**
 * @brief Indicates if conversion is completed
 * @return true if conversion is completed
 */
bool adc_conversion_completed();

/**
 * @bried Indicates if a conversion is in active
 * @return true if conversion is active
 */
bool adc_is_converting();

/**
 * @brief Selects ADC trigger source
 * @param trigger_type Trigger source to be configured (soft. or hard.)
 */
void adc_trigger_select(adc_trigger_enum trigger_type);

/**
 * @brief Gets the conversion data
 * @return uint16_t data
 */
uint16_t adc_get_data();

/**
 * @brief Triggers a new conversion
 * @details If sofrware trigger is selected, beggins a new conversion
 */
void adc_trigger_conversion();

/**
 * @brief Enables continous conversion
 * @param enable Boolean variable, when true continous conversion is enabled
 */
void adc_enable_continous_conversion(bool enable);

/**
 * @brief ADC0 data register address
 * @return returns a uint32_t with the ADC0 Result address
 */
uint32_t adc_data_result_address();

/**
 * @brief Configures a callback for the conversion completed interrupt
 * @param callback Function pointer to the handler
 */
void adc_set_conversion_completed_handler(adc_conversion_completed_callback_t callback);

/**
 * @brief Method to get SC1 register address
 * @return Return the address of the config register.
 */
uint32_t adc_get_sc1_address();

/**
 * @brief Enable / Disable interrupts
 * @param ie enabled / disabled
 */
void adc_set_interrupts_enabled(bool ie);

#endif
