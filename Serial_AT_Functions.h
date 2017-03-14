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

 *										Serial_AT_Functions
 *	This Library contains all the functions pertaining to AT Command set for ESP-01 AND ESP-12 Firmware (per ver 1.3.0.0)
 *	on ESP8266. 
 *	
 *	Serial Configuration is included provided you change the Serial Port used defined in AT_USART 
 *	Buffers str_res and AT_output are used to return result as a string, you need to include them as external variables to utilize them
 *		
 *		Ways to include in your file:
 *			#include "Serial_AT_Functions.h"
 *
 *			extern char str_res[BUFF_LEN];
 *			extern char AT_output[BUFF_LEN/4];
 *
 *	Language: C																
 *	Tested with XMega256A3BU
 *  Created: 2017-03-14
 *
 *  Author: Shivam S.
 *	
 *
 */ 


#ifndef SERIAL_AT_FUNCTIONS_H_
#define SERIAL_AT_FUNCTIONS_H_





#endif /* SERIAL_AT_FUNCTIONS_H_ */

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#define F_CPU 32000000
#include <util/delay.h>

#define BUFF_LEN 4096
#define LIMIT 1024
#define AT_USART USARTE0
#define AT_DMACH DMA.CH2
#define Byte char

char str_res[BUFF_LEN];
char AT_output[BUFF_LEN/4];

// Definitions based on the variables
#define BUFFER str_res
#define CURR_BUFFER AT_output


//Serial Functions
void Serial_conf(void);
void Tx_Wait(void);
void Rx_Wait(void);
void Rx_DMA_Conf(void);

int sendChar(char c);
void sendString(char* string);



// AT Functions
char* AT_set_ssid(char* ssid, char* pwd, char* ch, char* ecn);
char* AT_get_ssid(void);

char* AT_AP_mode(int mode);

char* AT_APConnect(char* ssid, char* pwd);
char* AT_APDisconnect(void);

char* AT_ListAP(void);
char* AT_ListClients(void);

//get ip or set ips
char* AT_ips(char st_ap_loc, char* ip);

//start a tcp/udp connection
char* AT_IP_Start(int id, char* type, char* addr, int port);

//send data
char* AT_sendData(int id, char* data);

char* AT_IP_Close(int id);

//config server
char* AT_Server_Conf(int mode, int port);

//set server timeout
char* AT_STimeout(int timeout);


char* AT_Reset(void);
char* AT_MUX(int mode);
char* AT_ifconfig(void);

