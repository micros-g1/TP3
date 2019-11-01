//
// Created by Rocío Parra on 10/30/2019.
//

#include "f_list.h"

//
// Created by Rocío Parra on 9/9/2019.
//

#include "f_list.h"
#include "hardware.h"



void fl_init(flist_t * fl, float * buffer, uint32_t max_len)
{
    fl->len = fl->in = fl->out = 0;
    fl->max_len = max_len;
    fl->buffer = buffer;
}


//Wait for data
float fl_read_blocking(flist_t * fl)
{
    float ans;
    //Atomic operation (assembly)
    while(fl->len == 0) {;} // wait for data

    ans = fl->buffer[fl->out++];
    if(fl->out >= fl->max_len) {
        fl-> out = 0;
    }
    //Atomic operation (assembly)
    fl->len--;

    return ans;
}

//Flush flueue
void fl_flush(flist_t * fl)
{
    hw_DisableInterrupts();
    //Must set these three variables to zero before continuing...
    fl->in = fl->out = fl->len = 0;
    hw_EnableInterrupts();
}

//Add data to queue
bool fl_pushback(flist_t * fl, float data)
{
    bool ret_val = false;

    hw_DisableInterrupts();

    if (fl->len < fl->max_len) {
        fl->buffer[fl->in++] = data;
        if(fl->in >= fl->max_len) {
            fl->in = 0;
        }
        fl->len = fl->len <= fl->max_len? fl->len+1 : fl->max_len;
        ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Add data to queue
bool fl_pushfront(flist_t * fl, float data)
{
    bool ret_val = false;
    hw_DisableInterrupts();
    if(fl->len != fl->max_len) {
        if(fl->out == 0) {
            fl->out = fl->max_len;
        }
        fl->out--;
        fl->buffer[fl->out] = data;
        fl->len++;
        ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Get current queue length
unsigned int fl_length(flist_t * fl)
{
    //atomic read operation (assembly)
    return fl->len;
}

bool fl_isfull(flist_t * fl)
{
    return fl->len == fl->max_len;
}



float fl_popfront(flist_t * fl)
{
    float data = 0;

    hw_DisableInterrupts();
    if (fl->len) {
        fl->len--;

        data = fl->buffer[fl->out++];
        if(fl->out == fl->max_len) {
            fl->out = 0;
        }
        //DEBUG
//        if(fl.len == fl->max_len)
//        	fl.len = fl->max_len+1;
    }
    hw_EnableInterrupts();
    return data;
}

float fl_read(flist_t *fl, uint32_t i) {
    if (i >= fl->len)
        return 0;
    if (fl->in + i >= fl->len)
        return fl->buffer[fl->in + i - fl->len];
    else
        return fl->buffer[fl->in + i];
}
