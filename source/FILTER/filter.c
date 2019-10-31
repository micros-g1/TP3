/*
 * filter.c
 *
 *  Created on: 29 oct. 2019
 *      Author: Tomas
 */
#include "filter.h"
#include <stdbool.h>
//File generated using filtro.py

#define FILTER_SIZE	25

static const float filter_coeffs[FILTER_SIZE] = { 7.53867824e-04,  9.88292031e-04, -8.38362670e-19, -3.66728802e-03,
 -1.00267989e-02, -1.60457165e-02, -1.52722375e-02,  4.78683290e-18,
  3.43659422e-02,  8.49846569e-02,  1.40313090e-01,  1.83602856e-01,
  2.00006674e-01,  1.83602856e-01,  1.40313090e-01,  8.49846569e-02,
  3.43659422e-02,  4.78683290e-18, -1.52722375e-02, -1.60457165e-02,
 -1.00267989e-02, -3.66728802e-03, -8.38362670e-19,  9.88292031e-04,
  7.53867824e-04 };

/*input_x[0] is the last received input*/
static float input_x[FILTER_SIZE];

void filter_init(){
	static bool initialized = false;
	if(initialized) return;


	initialized = true;
}

float apply_filter(float input){
	//starts updating input vector
	//should use a circular buffer instead for optimizing speed!
	for(int i = 0; i < FILTER_SIZE - 1; i++)
		input_x[i+1] = input_x[i];
	input_x[0] = input;
	//finished updating input vector

	double result = 0;
//	for (i = 0; i < FILTER_SIZE; ++i)
//		result += input_x[i]*filter_coeffs[i];

	return result;
}



