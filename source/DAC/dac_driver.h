/**
 * @file dac_driver.h
 * @author Grupo 1 Labo de Micros
 * @date 25 Oct 2019
 * @brief DAC interface
 * @details
 * Dac interface header
 */
#ifndef DAC_DRIVER_H
#define DAC_DRIVER_H

#include "stdbool.h"
#include "MK64F12.h"

#define DAC_BUFFER_SIZE			16
#define DAC_MIN_DIGITAL_VALUE	0
#define DAC_MAX_DIGITAL_VALUE	4095

/**
 * @struct dac_irq_config_t
 * @brief struct containing configuration for irq sources.
 */
typedef struct{
	bool watermark_irq;					///< @brief Buffer watermark interrupt
	bool read_pointer_top_flag_irq;		///< @brief Buffer read pointer top flag interrupt
	bool red_pointer_bottom_flag_irq;	///< @brief Buffer read pointer bottom flag interrupt
}dac_irq_config_t;

/**
 * @enum dac_vref_enum
 * @brief DAC reference voltage options
 */
typedef enum{
	DAC_VREF1,	///< @brief Voltage reference 1
	DAC_VREF2	///< @brief Voltage reference 2
}dac_vref_enum;

/**
 * @enum dac_trigger_enum
 * @brief DAC trigger source options
 */
typedef enum{
	DAC_HARDWARE_TRIGGER,	///< @brief Hardware trigger
	DAC_SOFTWARE_TRIGGER	///< @brief Software trigger
}dac_trigger_enum;

/**
 * @enum dac_buffer_config_enum
 * @brief DAC buffer modes of operation
 */
typedef enum{
	DAC_BUFFER_DISABLED,	///< @brief Buffer disabled
	DAC_BUFFER_NORMAL,		///< @brief Normal Mode: Circular buffer.
	DAC_BUFFER_SWING,		///< @brief Swing Mode.
	DAC_BUFFER_OTSCAN		///< @brief One Time Scan
}dac_buffer_config_enum;

typedef void (*dac_watermark_callback_t)(void);
typedef void (*dac_read_ptr_top_callback_t)(void);
typedef void (*dac_read_ptr_bottom_callback_t)(void);

/**
 * @brief DAC initialization
 * @details initialize DAC0 with default config
 *  - Buffer disabled
 *  - Interrupts disabled
 */
void dac_init();

/**
 * @brief Starts or stops the Programmable Reference Generator operation
 * @param value Boolean value:
 *  - true: Enables PRG operation
 *  - false: Stops PRG operation
 */
void dac_enable(bool enable);

/**
 * @brief Selects the reference voltage source
 * @param vref Voltage reference.
 */
void dac_select_reference(dac_vref_enum vref);

/**
 * @brief Configures the buffer operating mode
 * @param buffer_config Configuration mode of he buffer
 */
void dac_setup_buffer(dac_buffer_config_enum buffer_config);

/**
 * @brief Sets the value of the specified buffer position
 * @param pos Indicates the index of the buffer to be set
 * @param enable The value to be loaded.
 */
void dac_write_to_buffer(uint8_t pos, uint16_t data);

/**
 * @brief Enable or disable the DMA interrupts
 * @param enable Boolean value
 *  - true: Enable DMA
 *  - false: Disable DMA
 */
void dac_enable_dma(bool enable);

/**
 * @brief Enables or disables interrupts sources
 * @param irq_config Structure containing the information to enable or disable each irq source
 *  - DAC Buffer Watermark Interrupt
 *  - DAC Buffer Read Pointer Top Flag Interrupt
 *  - DAC Buffer Read Pointer Bottom Flag Interrupt
 */
void dac_setup_irq(dac_irq_config_t irq_config);

/**
 * @brief Set the handler for the Buffer Watermark Interrupt
 * @param callback Function called when interrupt occurs
 */
void dac_set_watermark_irq_handler(dac_watermark_callback_t callback);

/**
 * @brief Set the handler for the Buffer Read Pointer Top Flag Interrupt
 * @param callback Function called when interrupt occurs
 */
void dac_set_read_pointer_top_irq_handler(dac_read_ptr_top_callback_t callback);

/**
 * @brief Set the handler for the Buffer Read Pointer Bottom Flag Interrupt
 * @param callback Function called when interrupt occurs
 */
void dac_set_read_pointer_botom_irq_handler(dac_read_ptr_bottom_callback_t callback);

/**
 * @brief Selects DAC trigger source
 * @param trigger_type Trigger source to be configured (soft. or hard.)
 */
void dac_trigger_select(dac_trigger_enum trigger_type);

/**
 * @brief Triggers the output
 * @details Sets the output to th word pointed by read poonter and advance the buffer (only if buffer enabled and software trigger selected)
 */
void dac_trigger_output();

/**
 * @brief DAC0 data register address
 * @return returns a uint32_t with the DAC0 Result address
 */
uint32_t dac_read_data_address();

#endif
