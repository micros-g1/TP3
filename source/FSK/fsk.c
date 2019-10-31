/*
 * fsk.c
 *
 *  Created on: 30 Oct 2019
 *      Author: grein
 */

#include <FSK/fsk.h>
#include <FSK/fsk_tx.h>
#include <FSK/fsk_rx.h>

#define USING_ODD_PARITY true

//FSK Queues
static uint8_t fsk_rx_queue[FSK_RX_QUEUE_SIZE];
static uint8_t fsk_tx_queue[FSK_TX_QUEUE_SIZE];
//FSK indexes
static size_t rx_in_index, rx_out_index;
static size_t tx_in_index, tx_out_index;
static size_t tx_total_elements, rx_total_elements;

#define __MIN(x,y) ( (x) < (y) ? (x) : (y) )

void fskInit ()
{
	rx_in_index = rx_out_index = 0;
	tx_in_index = rx_out_index = 0;
	tx_total_elements = rx_total_elements = 0;
	//TODO: Enable everything else
	fsk_tx_init(NULL);
	fsk_tx_interrupt_enable(true);
	fsk_rx_init(NULL);
}

bool fskIsRxMsg()
{
	bool got_message = false;
	//TODO: Disable interrupts
	got_message = rx_total_elements != 0;
	//TODO: Enable interrupts
	return got_message;
}

size_t fskGetRxMsgLength()
{
	size_t message_length;
	//TODO: Disable interrupts
	message_length = rx_total_elements;
	//TODO: Enable interrupts
	return message_length;
}

size_t fskReadMsg(uint8_t * msg, size_t cant)
{

	//TODO: Disable interrupts
	size_t bytes_to_copy = __MIN(rx_total_elements,cant);
	for(size_t i = 0 ; i < bytes_to_copy ; i++)
	{
		msg[i] = fsk_rx_queue[rx_out_index++];
		rx_total_elements--;
		if(rx_out_index == FSK_RX_QUEUE_SIZE)
			rx_out_index = 0;
	}
	//TODO: Enable interrupts
	return bytes_to_copy;
}

size_t fskWriteMsg(const uint8_t* msg, size_t cant)
{
	//TODO: Disable interrupts
	size_t bytes_to_copy = __MIN(FSK_TX_QUEUE_SIZE-rx_total_elements,cant);
	for(size_t i = 0 ; i < bytes_to_copy ; i++)
	{
		fsk_tx_queue[tx_in_index++] = msg[i];
		tx_total_elements++;
		if(tx_in_index == FSK_TX_QUEUE_SIZE)
			tx_in_index = 0;
	}
	//TODO: Enable interrupts
	return bytes_to_copy;
}

bool fskIsTxMsgComplete()
{
	bool transfer_finished = false;
	//TODO: Disable interrupts
	transfer_finished = tx_total_elements == 0;
	//TODO: Enable interrupts
	return transfer_finished;
}


bool __fsk_next_bit_callback()
{
	//Default values
	static uint8_t bit_mask = 1;
	static bool started = false;
	static bool parity_sent = false;
	static bool parity = USING_ODD_PARITY;

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
			parity = USING_ODD_PARITY;
			started = parity_sent = false;
			bit_mask = 1;
		}
	}
	else if(tx_total_elements)	//Byte in queue?
	{
		//More data to send. Send start bit
		started = true;
		bit = false;
	}
	return bit;
}

void __fsk_byte_received_callback(uint8_t byte)
{
	if(rx_total_elements != FSK_RX_QUEUE_SIZE)
	{
		fsk_rx_queue[rx_in_index++] = byte;
		rx_total_elements++;
		if(rx_in_index == FSK_RX_QUEUE_SIZE)
			rx_in_index = 0;
	}
}
