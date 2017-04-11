/*
 * ADC_DMA_DAC.h
 *
 * Created: 2017-03-24 1:04:57 AM
 *  Author: Test
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

