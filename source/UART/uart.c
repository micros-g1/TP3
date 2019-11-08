/*
 * uart_test.c
 *
 *  Created on: Sep 25, 2019
 *      Author: Rocío Parra
 */

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "uart.h"

#include "MK64F12.h"
#include "MK64F12_features.h"
#include "hardware.h"
#include "gpio.h"

#include "util/queue.h"


typedef void(*uart_tx_irq_t)();

#define FIFO_SIZE	8

#if !(FIFO_SIZE == 1 || FIFO_SIZE == 2 || FIFO_SIZE == 4 || FIFO_SIZE == 8 || FIFO_SIZE == 16 || FIFO_SIZE == 32 || FIFO_SIZE == 64 || FIFO_SIZE == 128)
#error "valid uart fifo sizes are 1, 2, 4, 8, 16, 32, 64 and 128"
#endif

#define WATERMARK 1
#if WATERMARK >= FIFO_SIZE
#error "uart fifo watermark must be smaller than uart fifo size"
#endif
/*******************************************************************************
 * VARIABLES WITH FILE SCOPE
 ******************************************************************************/

static UART_Type * const uarts[UART_N_IDS] = {UART0, UART1, UART2, UART3, UART4}; // pointers to UART structures
static bool uart_active[UART_N_IDS];	// true if a given UART has been initialized

static const pin_t pins [UART_N_IDS][2] = {	// default pins for each UART
				/* RX					TX 					*/
/* UART_0 */	{ PORTNUM2PIN(PB, 16),	PORTNUM2PIN(PB, 17)},
/* UART_1 */	{ PORTNUM2PIN(PC, 03),	PORTNUM2PIN(PC, 04)},
/* UART_2 */	{ PORTNUM2PIN(PD, 02),	PORTNUM2PIN(PD, 03)},
/* UART_3 */	{ PORTNUM2PIN(PC, 16),	PORTNUM2PIN(PC, 17)},
/* UART_4 */	{ PORTNUM2PIN(PC, 14),	PORTNUM2PIN(PC, 15)}
};

static const uint32_t clock_gating_masks[UART_N_IDS] = { SIM_SCGC4_UART0_MASK, SIM_SCGC4_UART1_MASK,
		SIM_SCGC4_UART2_MASK, SIM_SCGC4_UART3_MASK, SIM_SCGC1_UART4_MASK };

static queue_t tx_q[UART_N_IDS];	// pending trasmissions
static queue_t rx_q[UART_N_IDS];	// pending messages

uart_tx_irq_t tx_handler;

/*******************************************************************************
 * FUNCTION DECLARATIONS, FILE SCOPE
 ******************************************************************************/

void uart_irq_handler(uint8_t id); // all interrupts call this handler to avoid copy-pasting code

void uart_periodic(void);	// called by systick, received and transmits



/*******************************************************************************
 * FUNCTION IMPLEMENTATIONS, GLOBAL SCOPE
 ******************************************************************************/

void uartInit (uint8_t id, uart_cfg_t config){
	if (id >= UART_N_IDS || uart_active[id] == true)
		return;

	//////////////////////
	// initialize buffers
	//////////////////////
	q_init(&tx_q[id]);
	q_init(&rx_q[id]);

	UART_Type * uart = uarts[id];
	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	pin_t rx_pin = pins[id][0];
	PORT_Type * port = addr_arrays[PIN2PORT(rx_pin)];
	pin_t tx_pin = pins[id][1];

	////////////////
	// CLOCK GATING
	////////////////
	if (id < 4) {
		SIM->SCGC4 |= clock_gating_masks[id];
	}
	else {
		SIM->SCGC1 |= clock_gating_masks[id];
	}

	//////////////
	// PCR CONFIG
	//////////////
	port->PCR[PIN2NUM(rx_pin)] = 0x0;
	port->PCR[PIN2NUM(tx_pin)] = 0x0;

	port->PCR[PIN2NUM(rx_pin)] |= PORT_PCR_MUX(0x3);
	port->PCR[PIN2NUM(tx_pin)] |= PORT_PCR_MUX(0x3);

	port->PCR[PIN2NUM(rx_pin)] &= ~PORT_PCR_IRQC(0x0); // disable interrupts
	port->PCR[PIN2NUM(tx_pin)] &= ~PORT_PCR_IRQC(0x0); // disable interrupts


	uart->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK); // disable uart for configuration
	uart->C1 = 0x0; // default config

	////////////////////
	// BAUDRATE CONFIG
	///////////////////
	uint16_t sbr, brfa;
	uint32_t clock, baudrate;
	clock = (id <= 1 ? __CORE_CLOCK__ : __CORE_CLOCK__ >> 1);
	baudrate = config.baudrate;

	sbr = clock / (baudrate<<4);
	brfa = (clock << 1) / baudrate - (sbr << 5);

	uart->BDH = UART_BDH_SBR(sbr >> 8);
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);

	/////////////////
	// DATA CONFIG
	/////////////////
	if (config.parity) {
		uart->C1 |= UART_C1_M_MASK;
		uart->C4 &= ~UART_C4_M10_MASK;
		uart->C1 |= UART_C1_PE_MASK;
		uart->C1 = config.odd_parity? uart->C1 | UART_C1_PT_MASK : uart->C1 & ~UART_C1_PT_MASK;
	}
	else {
		uart->C1 &= ~UART_C1_PE_MASK;
		uart->C1 &= ~UART_C1_M_MASK;
	}

	/////////////////
	// FIFO CONFIG
	/////////////////
	uart->PFIFO = 0;
	uart->PFIFO |= UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK;

	uint8_t mask = 0; // for size = 1 -> mask = 0
	if (FIFO_SIZE >= 4) {
		// for size>=4 -> mask = log2(size) - 1 (1 for 4, 2 for 8, 3 for 16...)
		uint8_t size = FIFO_SIZE >> 2;
		mask = 1;
		while (!(size & 0x01)) {
			size >>= 1;
			mask++;
		}
	}
	uart->PFIFO |= UART_PFIFO_RXFIFOSIZE(mask);
	uart->PFIFO |= UART_PFIFO_TXFIFOSIZE(mask);
	uart->TWFIFO = WATERMARK;
	uart->RWFIFO = WATERMARK;

	////////////////
	// ENABLE UART
	////////////////
	uart->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK);
	//UART0->S2 &= ~(0x06); // MSBF = 0, BRK13 = 0
	switch(id) { // enable interrupts!
	case 0: NVIC_EnableIRQ(UART0_RX_TX_IRQn); break;
	case 1: NVIC_EnableIRQ(UART1_RX_TX_IRQn); break;
	case 2: NVIC_EnableIRQ(UART2_RX_TX_IRQn); break;
	case 3: NVIC_EnableIRQ(UART3_RX_TX_IRQn); break;
	case 4: NVIC_EnableIRQ(UART4_RX_TX_IRQn); break;
	default: break;
	}

	uart_active[id] = true;
}



bool uartIsRxMsg(uint8_t id)
{
	return uartGetRxMsgLength(id) > 0;
}

uint8_t uartGetRxMsgLength(uint8_t id)
{
	if (id >= UART_N_IDS)
		return 0;

	return rx_q[id].len + uarts[id]->RCFIFO;
}


uint8_t uartReadMsg(uint8_t id, uint8_t* msg, uint8_t cant)
{
	if (id >= UART_N_IDS)
		return 0;

	uint8_t i = 0;
	while (i < cant && rx_q[id].len) {
		msg[i++] = q_popfront(&rx_q[id]);
	}

	while (i < cant && !(uarts[id]->SFIFO & UART_SFIFO_RXEMPT_MASK)) {
		msg[i++] = uarts[id]->D;
	}

	while (!(uarts[id]->SFIFO & UART_SFIFO_RXEMPT_MASK) && !q_isfull(&rx_q[id])) {
		q_pushback(&rx_q[id], uarts[id]->D);
	}

	return i;
}

uint8_t uartWriteMsg(uint8_t id, const uint8_t * msg, uint8_t cant)
{
	if (id >= UART_N_IDS)
		return 0;
	uint8_t i = 0;
	if (!tx_q[id].len) {
		while (i < cant && uarts[id]->TCFIFO < FIFO_SIZE) {
			uarts[id]->D = msg[i++];
		}
	}

	while (i < cant && !q_isfull(&tx_q[id])) {
		q_pushback(&tx_q[id], msg[i++]);
	}

	if (tx_q[id].len) {
		uarts[id]->C2 |= UART_C2_TIE_MASK;
	}

	return i;
}


bool uartIsTxMsgComplete(uint8_t id)
{
	if (id >= UART_N_IDS)
		return false;

	return (tx_q[id].len == 0) && (uarts[id]->S1 & UART_S1_TC_MASK);
}



void uart_putchar (uint8_t id, uint8_t ch)
{
	if (id >= UART_N_IDS)
		return;

	/* Wait until space is available */
	while(uarts[id]->TCFIFO == FIFO_SIZE);
	/* Send the character */
	uarts[id]->D = (uint8_t)ch;
}

uint8_t uart_getchar (uint8_t id)
{
	if (id >= UART_N_IDS)
		return 0;
	/* Wait until character has been received */

	uint8_t data;
	while (!(rx_q[id].len || uarts[id]->RCFIFO));

	if (rx_q[id].len) {
		data = q_popfront(&rx_q[id]);
	}
	else {
		data = uarts[id]->D;
	}
	/* Return the 8-bit data from the receiver */
	return data;
}


/*******************************************************************************
 * FUNCTION IMPLEMENTATIONS, FILE SCOPE
 ******************************************************************************/

void uart_irq_handler(uint8_t id)
{
	if (uarts[id]->S1 & UART_S1_RDRF_MASK) {
		while (uarts[id]->RCFIFO && !q_isfull(&rx_q[id])) {
			q_pushback(&rx_q[id], uarts[id]->D);
		}
	}
	else {
		uarts[id]->D = q_popfront(&tx_q[id]);
	}

	if(!tx_q[id].len) {
		uarts[id]->C2 &= ~UART_C2_TIE_MASK; // message finished, disable transmission interrupts
	}
}



/*******************************************************************************
 * OVERRIDE OF INTERRUPT VECTORS
 ******************************************************************************/

__ISR__ UART0_RX_TX_IRQHandler (void)
{
	uart_irq_handler(0);
}

__ISR__ UART1_RX_TX_IRQHandler (void)
{
	uart_irq_handler(1);
}

__ISR__ UART2_RX_TX_IRQHandler (void)
{
	uart_irq_handler(2);
}

__ISR__ UART3_RX_TX_IRQHandler (void)
{
	uart_irq_handler(3);
}

__ISR__ UART4_RX_TX_IRQHandler (void)
{
	uart_irq_handler(4);
}
