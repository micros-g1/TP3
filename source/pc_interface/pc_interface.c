//
// Created by Rocío Parra on 10/5/2019.
//

#include "pc_interface.h"

#ifndef ROCHI_DEBUG
#include "UART/uart.h"
#else
#include <stdio.h>
#endif

#include "../util/msg_queue.h"
#include "../util/clock.h"
#include <string.h>

#define PC_UART 0

#define PC_MIN_MS (1000.0/PC_MAX_FREQ) // it will round to a slightly faster frequency, but this is not an issue

#if Q_MSG_LEN != PC_MSG_LEN
#error "queue msg size does not match pc msg len"
#endif

static msg_queue_t uart_q;
static clock_t last;


void pc_init()
{
    static bool is_init = false;
    if (is_init)
        return;
    is_init = true;

    mq_init(&uart_q);

#ifndef ROCHI_DEBUG
    uart_cfg_t uart_config;
    uart_config.baudrate = 9600;
    uart_config.parity = true;
    uart_config.odd_parity = true;
    uartInit(PC_UART, uart_config);
#endif

    clock_init();
    last = get_clock();
}

void pc_send(uint8_t * msg)
{
    mq_pushback(&uart_q, msg);
    pc_periodic(); // see if i can send it right now
}

void pc_periodic()
{
    if (uart_q.len) {
        float time_diff = 1000 * (float)(get_clock() - last) / CLOCKS_PER_SECOND;
        while (time_diff >= PC_MIN_MS && uart_q.len) {
            uint8_t msg[PC_MSG_LEN + 1]; // leave one byte for terminator
            mq_popfront(&uart_q, msg);
#ifndef ROCHI_DEBUG
            uint8_t msglen = strlen(msg);
            uint8_t n_transmitted = uartWriteMsg(PC_UART, msg, msglen);
            msg[n_transmitted] = '\0';
            if (n_transmitted < msglen) {
            	mq_pushfront(&uart_q, &(msg[n_transmitted]));
            }
#else
            printf("PC: %s \n", msg);
#endif
            last = get_clock();
        }
    }
}
