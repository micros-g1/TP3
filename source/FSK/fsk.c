/*
 * fsk.c
 *
 *  Created on: 30 Oct 2019
 *      Author: grein
 */

#include <FSK/fsk.h>
#include <stdbool.h>
#include <stdint.h>

#if FSK_VERSION == 1
#include <FSK/fsk_rx.h>
#include <FSK/fsk_tx.h>
#else
#include <FSK/fsk_rx_v2.h>
#include <FSK/fsk_tx_v2.h>
#endif


#define USING_ODD_PARITY true

//FSK Queues
static uint8_t fsk_rx_queue[FSK_RX_QUEUE_SIZE];
static uint8_t fsk_tx_queue[FSK_TX_QUEUE_SIZE];
//FSK indexes
static size_t rx_in_index, rx_out_index;
static size_t tx_in_index, tx_out_index;
static size_t tx_total_elements, rx_total_elements;

static void __fsk_byte_received_callback(uint8_t byte);
static bool __fsk_next_bit_callback();

#define __MIN(x,y) ( (x) < (y) ? (x) : (y) )

void __fsk_byte_received_callback(uint8_t byte);

void fskInit ()
{
	rx_in_index = rx_out_index = 0;
	tx_in_index = tx_out_index = 0;
	tx_total_elements = rx_total_elements = 0;

	fsk_rx_init(__fsk_byte_received_callback);
	fsk_rx_enable_interrupts();

	fsk_tx_init(__fsk_next_bit_callback);
	fsk_tx_interrupt_enable(true);
	fsk_rx_init(__fsk_byte_received_callback);
}

bool fskIsRxMsg()
{
	bool got_message = false;
	fsk_rx_disable_interrupts();
	got_message = rx_total_elements != 0;
	fsk_rx_enable_interrupts();
	return got_message;
}

size_t fskGetRxMsgLength()
{
	size_t message_length;
	fsk_rx_disable_interrupts();
	message_length = rx_total_elements;
	fsk_rx_enable_interrupts();
	return message_length;
}

size_t fskReadMsg(uint8_t * msg, size_t cant)
{
	fsk_rx_disable_interrupts();
	size_t bytes_to_copy = __MIN(rx_total_elements,cant);
	for(size_t i = 0 ; i < bytes_to_copy ; i++)
	{
		msg[i] = fsk_rx_queue[rx_out_index++];
		rx_total_elements--;
		if(rx_out_index == FSK_RX_QUEUE_SIZE)
			rx_out_index = 0;
	}
	fsk_rx_enable_interrupts();
	return bytes_to_copy;
}

size_t fskWriteMsg(const uint8_t* msg, size_t cant)
{
	fsk_tx_interrupt_enable(false);
	size_t bytes_to_copy = __MIN(FSK_TX_QUEUE_SIZE-rx_total_elements,cant);
	for(size_t i = 0 ; i < bytes_to_copy ; i++)
	{
		fsk_tx_queue[tx_in_index++] = msg[i];
		tx_total_elements++;
		if(tx_in_index == FSK_TX_QUEUE_SIZE)
			tx_in_index = 0;
	}
	fsk_tx_interrupt_enable(true);
	return bytes_to_copy;
}


bool fskIsTxMsgComplete()
{
	bool transfer_finished = false;
	fsk_tx_interrupt_enable(false);
	transfer_finished = tx_total_elements == 0;
	fsk_tx_interrupt_enable(true);
	return transfer_finished;
}

static bool __fsk_next_bit_callback()
{
	//Default values
	static bool started = false;
	static uint8_t bit_mask;
	static bool parity_sent;
	static bool parity;
	//Assume IDLE
	bool bit = true;	//If idle, send TRUE

	//Sending byte?
	if(started)
	{
		if(bit_mask) //Something to send?
		{
			bit = !!(fsk_tx_queue[tx_out_index] & bit_mask);
			bit_mask <<= 1;
			parity ^= bit;
		}
		else if(!parity_sent)
		{
			bit = parity;
			parity_sent = true;
		}
		else
		{
			//No data and parity sent. Send stop and finish
			bit = true;	//STOP
			tx_out_index++;
			tx_total_elements--;
			if(tx_out_index == FSK_TX_QUEUE_SIZE)
				tx_out_index = 0;
			//DEFAULT VALUES
			parity = true;
			started = parity_sent = false;
			bit_mask = 1;
		}
	}
	else if(tx_total_elements)	//Byte in queue?
	{
		//More data to send. Send start bit
		started = true;
		bit = false;
		bit_mask = 1;
		parity_sent = false;
		parity = USING_ODD_PARITY;
	}
	return bit;
}

static void __fsk_byte_received_callback(uint8_t byte)
{
	if(rx_total_elements != FSK_RX_QUEUE_SIZE)
	{
		fsk_rx_queue[rx_in_index++] = byte;
		rx_total_elements++;
		if(rx_in_index == FSK_RX_QUEUE_SIZE)
			rx_in_index = 0;
	}
}
