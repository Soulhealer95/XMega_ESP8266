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

 * Serial_AT_Functions.c
 * Created: 2017-03-14 1:36:03 AM
 *  Author: Shivam S
 */ 
#include "Serial_AT_Functions.h"


//****************************USART CONFIG FUNCTIONS ***************************


void Serial_conf(void)
{
	// Baud rate selection
	//For 115200 @ 32 MHz Clock
	

	//Set clock config accordingly to 32MHz
		OSC_CTRL |= OSC_RC32MEN_bm;
				
		while(!(OSC_STATUS & OSC_RC32MRDY_bm));

		//Trigger protection mechanism				
		CCP = CCP_IOREG_gc; 

		CLK_CTRL = CLK_SCLKSEL_RC32M_gc;
	


	//BSCALE is  -1(0b0111)   -3 (two's complement form 0b1101)
		AT_USART.BAUDCTRLB = 0b11010000;
	
	//BSEL is 131
		AT_USART.BAUDCTRLA = 0x83; //33- 21 | 131- 0x83
	
	//disable interrupts
		AT_USART.CTRLA |= (USART_RXCINTLVL_OFF_gc | USART_TXCINTLVL_OFF_gc | USART_DREINTLVL_OFF_gc);
	
	//8 data bits, no parity and 1 stop bit
		AT_USART.CTRLC = USART_CHSIZE_8BIT_gc;

	//Enable receive and transmit
		AT_USART.CTRLB = USART_TXEN_bm | USART_RXEN_bm; // And enable high speed mode
}




//Wait for Pending Transfers to be complete
void Tx_Wait(void)
{
	// Wait for the transmit buffer to be empty
		while (  !(AT_USART.STATUS & USART_DREIF_bm) );
		AT_USART.STATUS |= PIN6_bm;
		
}

//Wait for data in read buffer to be read
void Rx_Wait(void)
{
	while( (AT_USART.STATUS & USART_RXCIF_bm) == 0 ); 

}

//Send a char and wait for the transfer to be complete
int sendChar(char c)
{
	//send data
	AT_USART.DATA = c;	
	
	//wait for pending transfers to complete
	Tx_Wait();
		
	return 1;	
}

//Send an int over serial
void sendInt(int x)
{
	//get the lower byte
	//configure DMA source address to ADCA CH2's result
	char x_l =  ((x) & 0x00FF);
	sendChar(x_l);
	char x_h = (((x) & 0xFF00) >> 8); 
	sendChar(x_h);
	
}



//Send a string over serial using sendChar
void sendString(char* string)
{
	int i = 0;
	while(string[i] != '\0')
	{
		sendChar(string[i]);
		
		i++;
	}
}

//Wait for DMA Transfers to be complete

//enable DMA channel 2 for RX
void Rx_DMA_Conf(void)
{
	//reset DMA (before enabling)
	DMA.CTRL |= DMA_RESET_bm;

	//wait DMA to complete pending transfers --> reset
	while((DMA.STATUS & DMA_CH2BUSY_bm)!= 0);

	//enable the DMA controller
	DMA.CTRL |= DMA_ENABLE_bm;//0x80;
	
	//set burst transfer length to 1-Bytes (bit 1 and 0 set to: 01) and enable single shot transfer
	AT_DMACH.CTRLA |=  (DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm);
	
	AT_DMACH.ADDRCTRL |=	 (DMA_CH_SRCRELOAD_NONE_gc | DMA_CH_SRCDIR_FIXED_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_INC_gc);	
	
	//setting up interrupts
	
	//Enable interrupts
	PMIC_CTRL |= PMIC_HILVLEN_bm; 
	
	//enable and configure to trigger at high priority by setting the bits at 0x03
	AT_DMACH.CTRLB |=  0x03;

	//select Trigger Source as AT_USART Receive complete (0x8B + 0x00) S
	AT_DMACH.TRIGSRC |=   (0x8B + 0x00);
	
	//set the number of bytes
	AT_DMACH.TRFCNT = BUFF_LEN/2;
	
	//configure DMA source address to USART DATA and destination to buffer array
	Byte AT_USART_Lowernib_bm =  (((uint16_t) &AT_USART.DATA >> 0) & 0xFF);
	Byte AT_USART_Highernib_bm = ((((uint16_t) &AT_USART.DATA >> 8) & 0xFF));
	Byte DACB_Lowernib_bm =  (((uint16_t) &BUFFER[0] >> 0) & 0xFF);
	Byte DACB_Highernib_bm = (((uint16_t) &BUFFER[0] >> 8) & 0xFF);

	AT_DMACH.SRCADDR0 = AT_USART_Lowernib_bm; 
	AT_DMACH.SRCADDR1 =  AT_USART_Highernib_bm; 
	AT_DMACH.SRCADDR2  = 0x00;
	
	//Configure DMA destination address to DACB CH1's input
	AT_DMACH.DESTADDR0 = DACB_Lowernib_bm;
	AT_DMACH.DESTADDR1 = DACB_Highernib_bm;
	AT_DMACH.DESTADDR0 = 0x00;
	
	
	//enable DMA channel
	AT_DMACH.CTRLA |=  DMA_ENABLE_bm;
}


/*
	last_change_res
	parameters: none
	
	Returns the last updated index in the input buffer


*/
int last_change_res(void)
{
	int i = 0;
	while(i < 1023)
	{
		if(BUFFER[i] == '\0')
		{
			return i;
		}
		i++;
	}
	return 0;
}

/*
	return_AT_res
	parameters: 
	start_index: int	-index of the buffer at the end of last command
	
	Returns the result from the last command executed

*/
char* return_AT_res(int start_index)
{
		memset(CURR_BUFFER,0,(BUFF_LEN/4)*sizeof(char));

		int end_s = last_change_res();
		for(int i=start_index, j=0; i < end_s; i++,j++)
		{
			CURR_BUFFER[j] = BUFFER[i];
		}
		return CURR_BUFFER;
}



//****************************AT COMMAND FUNCTIONS ***************************


//reset the chip
char* AT_Reset(void)
{
	int begin_r = last_change_res();
	sendString("AT+RST\r\n");
	_delay_ms(7000);
	return return_AT_res(begin_r + strlen("AT+RST\r\n"));
}




/*
AT_set_ssid

Uses AT+CWSAP to change the AP ssid and password settings

Parameters:
ssid: String, ESP8266’s softAP SSID
pwd: String, Password, no longer than 64 characters
ch: channel id
ecn:
0 = OPEN
2 = WPA_PSK
3 = WPA2_PSK
4 = WPA_WPA2_PSK

*/

char* AT_set_ssid(char* ssid, char* pwd, char* ch, char* ecn)
{
	int begin_r = last_change_res();
	char command[50];
	sprintf(command, "AT+CWSAP=\"%s\",\"%s\",%s,%s\r\n",ssid,pwd,ch,ecn);
	sendString(command);
	_delay_ms(2);
	return return_AT_res(begin_r + strlen(command) +1);
		
}


/*
get_id

Uses AT+CWSAP to check for AP settings

Parameters:
None

*/
char* AT_get_ssid(void)
{
	int begin_r = last_change_res();
	sendString("AT+CWSAP?\r\n");

	return return_AT_res(begin_r + strlen("AT+CWSAP?\r\n") +1);
}


/*
AT_AP_mode

Parameters:

mode?A 0 or any negative integer to check for current mode status or An integer designating the mode of operation either 1, 2, or 3.
1 = Station mode (client)
2 = AP mode (host)
3 = AP + Station mode (Yes, ESP8266 has a dual mode!)
Notes:

ESP-12 came configured as host with ssid set to ESP_A0A3F2, no password, channel 1 You can use AT+CWSAP? to find the current settings.

This function does nothing for any other positive values
*/


char* AT_AP_mode(int mode)
{
	int begin_r = last_change_res();
	char command[30];
	if(mode <= 0)
	{
		sendString("AT+CWMODE?\r\n");
	}
	else if(mode < 4)
	{
		sprintf(command, "AT+CWMODE=%i\r\n",mode);
		sendString(command);			
	}
		_delay_ms(2);
		return return_AT_res(begin_r + strlen(command) +1);
}


//****************** CONNECT/DISCONNECT TO APS *******************

/*
Connect to AP
Parameters:

ssid?String, AP’s SSID
pwd?String, not longer than 64 characters

Note:
to check for connected APs leave ssid ""

*/

char* AT_APConnect(char* ssid, char* pwd)
{
		int begin_r = last_change_res();
		char command[30];
		if(ssid[0] == '\0')
		{
			sendString("AT+CWJAP?\r\n");
		}
		else
		{
			sprintf(command, "AT+CWJAP=\"%s\",\"%s\"\r\n",ssid, pwd);
			sendString(command);
		}
			_delay_ms(2);
			return return_AT_res(begin_r);

}

//Disconnect from AP
char* AT_APDisconnect(void)
{
	int begin_r = last_change_res();
	sendString("AT+CWQAP\r\n");
	_delay_ms(2);
	return return_AT_res(begin_r);
}


//***************** LIST APs ***********************

//AT+CWLAP - Lists available APs
char* AT_ListAP(void)
{
	int begin_r = last_change_res();
	sendString("AT+CWLAP\r\n");
	_delay_ms(4000);
	return return_AT_res(begin_r+1+ strlen("AT+CWLAP\r\n"));
}


//AT+CWLIF - List clients connected to ESP8266 softAP
char* AT_ListClients(void)
{
	int begin_r = last_change_res();
	sendString("AT+CWLIF\r\n");
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen("AT+CWLIF\r\n"));
}

//************** IP Addresses ********************
/*
AT_ips
Description: Get or Set IP of station or AP mode

Parameters:

char st_ap: either 'S' or 'A' for Station or AP mode respectively
char* ip: string of ipv4 address

Note:
To get the current values of a mode just leave the ip "" 
*/


char* AT_ips(char st_ap, char* ip)
{
	int begin_r = last_change_res();
	
	char command[30];
	//only do something if st_ap is valid
	if(st_ap == 'A' || st_ap == 'S')
	{
		char* rest;
		if(ip[0] == '\0')
		{
			rest = "?";
		}
		else
		{
			rest= "=\"";
			strcat(rest,ip);
			strcat(rest,"\"");
		}
	
		if(st_ap == 'S')
		{
			sprintf(command,"AT+CIPSTA%s\r\n",rest);
		}
		else if (st_ap == 'A')
		{
			sprintf(command,"AT+CIPAP%s\r\n",rest);
		}
		//send command
		sendString(command);

	}
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen(command));	
	
	
}



/*
AT_IP_Close

Uses AT+CIPCLOSE - Close TCP or UDP connection

Parameters:

id? ID no. of connection to close, when id=5, all connections will be closed.
Note:
if id == 6 tests command

In server mode, id = 5 has no effect!
negative ids have no effect!
*/
char* AT_IP_Close(int id)
{
	int begin_r = last_change_res();
	char command[30];
	if(id ==6)
	{
		sendString("AT+CIPCLOSE=?\r\n");
	}
	else if (id < 6 && id >= 0)
	{
		//single mode
		if(id < 0)
		{
			sendString("AT+CIPCLOSE\r\n");
		}
		//multimode
		else
		{
			
			sprintf(command,"AT+CIPCLOSE=%i\r\n",id);
			sendString(command);
		}
	}
		_delay_ms(2);
		return return_AT_res(begin_r+1+ strlen(command));
}




//Change connections mode
/*
AT_MUX

Uses AT+CIPMUX - Enable multiple connections or not
Parameters:

mode:
-1: Current mode
0: Single connection
1: Multiple connections (MAX 4)
NOTE:

This mode can only be changed after all connections are disconnected. If server is started, reboot is required.


*/
char* AT_MUX(int mode)
{
	int begin_r = last_change_res();
	char command[20];
	if(mode == -1)
	{
		sendString("AT+CIPMUX?\r\n");	
	}
	else if (mode < 2)
	{
		//stop all connections
		AT_IP_Close(5);
		
		//change mode
		
		sprintf(command,"AT+CIPMUX=%i\r\n",mode);
		sendString(command);	
	}
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen(command));
}



//AT+CIPSTART - Establish TCP connection or register UDP port and start a connection
//only need single connection
/*
AT_IP_Start

Parameters:

id: 0-4, id of connection
type: String, “TCP” or “UDP”
addr: String, remote IP
port: String, remote port

*/
char* AT_IP_Start(int id, char* type, char* addr, int port)
{
	int start_s = last_change_res();
	char command[50];
	//if type is empty, it is a query
	if(type[0] == '\0')
	{
		sendString("AT+CIPSTART=?\r\n");
	}
	else if (id <5)
	{
		
			//check the connection type if id < 0 it is single
		if(id < 0)
		{
			sprintf(command,"AT+CIPSTART=\"%s\",\"%s\",%i\r\n",type,addr,port);	
		}
		else
		{
			sprintf(command,"AT+CIPSTART=%i,\"%s\",\"%s\",%i\r\n",id,type,addr,port);
		}
		
		sendString(command);	
	}

	return return_AT_res(start_s +1 + strlen(command));
}


//AT+CIPSEND - Send data
/*

Normal Mode

Parameters:

id: ID no. of transmit connection
data: String, data to be sent; data length, MAX 2048 bytes
Unvarnished Transmission Mode

Wrap return “>” after execute command. Enters unvarnished transmission, 20ms interval between each packet, maximum 2048 bytes per packet. When single packet containing “+++” is received, it returns to command mode.


*/


char* AT_sendData(int id, char* data)
{
	int begin_r = last_change_res();
	char command[50];
	//test data 
	if(data[0] == '\0')
	{
		sendString("AT+CIPSEND=?\r\n");
	}
	else if (id < 5)
	{
		
		//calculate the length of data to be sent
		int length = strlen(data);
		 
		//check the connection type if id < 0 it is single
		if(id < 0)
		{
			sprintf(command,"AT+CIPSEND=%i\r\n",length);	
		}
		else
		{
			sprintf(command,"AT+CIPSEND=%i,%i\r\n",id,length);
		}
		
		//send command to expect data
		sendString(command);
		
		//wait for command to be registered
		_delay_ms(1);
		
		//send the data
		sendString(data);
	}
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen(command));
}



//returns the IP configurations of AP and Station mode
char* AT_ifconfig(void)
{
	int begin_r = last_change_res();
	sendString("AT+CIFSR\r\n");
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen("AT+CIFSR\r\n"));
}

/*
AT_Server_Conf
Parameters:

mode:
0: Delete server (need to follow by restart)
1:	Create server
port: port number, default is 333 
NOTE:
setting port number to 0 or negative value will set it to default
Server can only be created when AT+CIPMUX=1
Server monitor will automatically be created when Server is created.
When a client is connected to the server, it will take up one connection?be gave an id.


*/


char* AT_Server_Conf(int mode, int port)
{
	int begin_r = last_change_res();
	char command[40];
	if(mode <2)
	{
		
		//check for the port number, if not a natural number, set to default
		if(port <= 0)
		{
			port = 333;
		}
		sprintf(command,"AT+CIPSERVER=%i,%i\r\n",mode, port);
		sendString(command);
		//if deleted server, needs to follow by reset
		if(mode == 0)
		{
			_delay_ms(1);
			AT_Reset();
		}
	}
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen(command));
}

/*
AT_STimeout

Uses AT+CIPSTO command to change the timeout
 
Parameters:

timeout: int, the time in seconds


*/

char* AT_STimeout(int timeout)
{
	int begin_r = last_change_res();
	char command[30];
	if(timeout <= 0)
	{
		sendString("AT+CIPSTO?\r\n");
	}
	else if (timeout <= 7200)
	{
		sprintf(command,"AT+CIPSTO=%i\r\n",timeout);
		sendString(command);
	}
	_delay_ms(2);
	return return_AT_res(begin_r+1+ strlen(command));
}
