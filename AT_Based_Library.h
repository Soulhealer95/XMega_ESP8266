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

 *										AT_Based_Library
 *	This Library utilizes the Serial_AT_Functions library to implement configurations for ESP8266 through XMega256A3BU
 *
 *
 *		Ways to include in your file:
 *			#include "AT_Based_Library.h"
 *
 *	Language: C
 *	Tested with XMega256A3BU
 * Created: 2017-03-24 1:16:25 AM
 *  Author: Shivam Saxena
 */ 


#ifndef AT_BASED_LIBRARY_H_
#define AT_BASED_LIBRARY_H_





#endif /* AT_BASED_LIBRARY_H_ */
#include "ADC_DMA_DAC.h"
#include <math.h>


char packet[4*SAMPLES];
volatile int result[SAMPLES];


//ESP Config using AT Functions
void AT_init_Config_King(void);
void AT_init_Config_Master(void);
void Send_Data_From_Master(char* data);

//packet creation
char* AT_buildPacket(void);
//void AT_DecodePacket(char* data);
void AT_DecodeStream(void);

void Start_DataS_from_Master(void);
void Start_DataS_from_King(void);

void sendEvent_DAC(uint8_t data);
void sendDatatoDAC(char* packet_data);
void AT_DecodePacket(void);
void ATEnter_Raw_data_mode(int id, int source);
void ATExit_Raw_data_mode(void);
bool CheckArrayForSubStrStart(char* arr, char* sub, int start_at);
