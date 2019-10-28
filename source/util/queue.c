//
// Created by Rocío Parra on 9/9/2019.
//

#include "queue.h"
#include "hardware.h"



void q_init(queue_t * q)
{
	q->len = q->in = q->out = 0;
}


//Wait for data
uint8_t q_read_blocking(queue_t * q)
{
	uint8_t ans;
    //Atomic operation (assembly)
    while(q->len == 0) {;} // wait for data

    ans = q->buffer[q->out++];
    if(q->out >= Q_MAX_LENGTH) {
        q-> out = 0;
    }
    //Atomic operation (assembly)
    q->len--;

    return ans;
}

//Flush queue
void q_flush(queue_t * q)
{
    hw_DisableInterrupts();
    //Must set these three variables to zero before continuing...
    q_init(q);
    hw_EnableInterrupts();
}

//Add data to queue
bool q_pushback(queue_t * q, uint8_t data)
{
    bool ret_val = false;

    hw_DisableInterrupts();

    if (q->len < Q_MAX_LENGTH) {
    	q->buffer[q->in++] = data;
    	if(q->in >= Q_MAX_LENGTH) {
        	q->in = 0;
    	}
    	q->len = q->len <= Q_MAX_LENGTH? q->len+1 : Q_MAX_LENGTH;
    	ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Add data to queue
bool q_pushfront(queue_t * q, uint8_t data)
{
    bool ret_val = false;
    hw_DisableInterrupts();
    if(q->len != Q_MAX_LENGTH) {
    	if(q->out == 0) {
    		q->out = Q_MAX_LENGTH;
    	}
    	q->out--;
        q->buffer[q->out] = data;
        q->len++;
        ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Get current queue length
unsigned int q_length(queue_t * q)
{
    //atomic read operation (assembly)
    return q->len;
}

bool q_isfull(queue_t * q)
{
	return q->len == Q_MAX_LENGTH;
}



uint8_t q_popfront(queue_t * q)
{
	uint8_t data = 0;

    hw_DisableInterrupts();
    if (q->len) {
        q->len--;

        data = q->buffer[q->out++];
        if(q->out == Q_MAX_LENGTH) {
        	q->out = 0;
        }
        //DEBUG
//        if(q.len == Q_MAX_LENGTH)
//        	q.len = Q_MAX_LENGTH+1;
    }
    hw_EnableInterrupts();
    return data;
}
