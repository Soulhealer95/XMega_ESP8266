/*
MIT License

Copyright (c) 2017 Shivam Saxena

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 *										ADC_DMA_DAC
 *	This Library contains functions for configurations of ADC, DMA and DAC for XMega256A3BU. 
 *	The configurations are intended for use in a specific project but may be modified for any project. 
 *
 *
 *		Ways to include in your file:
 *			#include "ADC_DMA_DAC.h"
 *
 *	Language: C

 * Created: 2017-03-24 1:04:57 AM
 * Author: Shivam S.
 */ 


#ifndef ADC_DMA_DAC_H_
#define ADC_DMA_DAC_H_





#endif /* ADC_DMA_DAC_H_ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define F_CPU 32000000
#include <util/delay.h>
#include "Serial_AT_Functions.h"
//****** DEFINITIONS ********/
#define ADCX ADCA
#define DACX DACB
#define ADC_CH ADCX.CH0

//lowest channel gets priority and we need the adc to complete before pushing to dac
#define DAC_DMACH DMA.CH3
//#define DAC_STRCH DMA.CH3
#define ADC_DMACH DMA.CH2
#define Byte char
#define BUFF_LEN 2
#define SAMPLES 250



int ADC_buff[BUFF_LEN];
int DAC_buff[SAMPLES];

#define BUFFER ADC_buff
#define DAC_OFFSET 97
#define DBUFFER DAC_buff
#define DVAR DAC_Stream_Var


//Clock Functions
void clock_config(int type);

//DAC functions
void DAC_init(void);
void DAC_datain (int x);
void DAC_wait(void);

//ADC Functions
void adc_Wait(void);
void ADCA_EVENT_enable(void);

//DMA Functions
void DMA_wait(void);
void DMA_Conf(void);
void DAC_DMA_Conf(void);
void ADC_DMA_Conf(void);
void DAC_TransferStart(void);
void DAC_DMAStream_Conf(void);

