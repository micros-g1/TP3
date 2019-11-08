//
// Created by Rocío Parra on 10/30/2019.
//

#ifndef XTREME_F_LIST_H
#define XTREME_F_LIST_H


/*
* ms_evqueue.h
        *
        *  Created on: 30 Aug 2019
*      Author: grein
*/

//Idea based on
//https://github.com/ITBAALUMNOS/Tetris/blob/master/Sources/event_queue.h

//list

#include <stdbool.h>
#include <stdint.h>
//Total number of elements that event queue can hold

typedef struct {
    volatile float * buffer;
    volatile uint32_t len;				//Keeps track of queue length
    uint32_t max_len;
    volatile uint32_t in;				//Offset for adding next data
    volatile uint32_t out;				//Offset for reading next data
} flist_t;


void fl_init(flist_t * fl, float * buffer, uint32_t max_len);

//Wait for data. Can only be used by main loop.
float fl_read_blocking(flist_t * fl);
//Flush flist. Can only be used by main loop.
void fl_flush(flist_t * fl);
//Add data to flist. True if event flist was not full
bool fl_pushback(flist_t * fl, float data);
bool fl_pushfront(flist_t * fl, float data);

//Get current flist length.
float fl_popfront(flist_t * fl); // will return 0 if flist empty, but also if data is 0. check length first!
unsigned int fl_length(flist_t * fl);
bool fl_isfull(flist_t * fl);

float fl_read(flist_t * fl, uint32_t i);


#endif //XTREME_F_LIST_H
