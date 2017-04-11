/*
 * AT_Based_Library.c
 *
 * Created: 2017-03-24 1:19:38 AM
 *  Author: Test
 */ 

#include "AT_Based_Library.h"

//**************************************		AT BASED FUNCTIONS			************************************************
void AT_init_Config_King(void)
{
	//Configure the WIFI Side
	//AP Config:
	
	//set mode as both station and AP
	AT_AP_mode(3);
	//disconnect any current connections
	AT_APDisconnect();
	//normal mode
	AT_IP_Mode(0);
	//enable multiple connections
	AT_MUX(1);
	//create an AP
	AT_set_ssid("PHANTOM_KING!","HeManNo1","1","2");

	//set AP ip
	AT_ips('A',"192.168.66.1");

	//STATION Config:
	AT_APConnect("Ph_Master","HeManNo1");
	
	//set station IP
	AT_ips('S',"192.168.5.2");

	//Configure TCP Side
	//Create Server:
	AT_Server_Conf(1,666);

	//set timeout as 180
	AT_STimeout(180);
	
		//set mode to unvarnished (note, to end this mode, you need to send a packet with just +++)
		//AT_IP_Mode(1);
	//Client Side:
	//Use the following to start a connection anytime :)
	//ip_Start(2,"TCP","192.168.5.1",666);

}

void AT_init_Config_Master(void)
{
	//Configure the WIFI Side
	//AP Config:
	
	//set mode as both station and AP
	AT_AP_mode(3);
	//disconnect any current connections
	AT_APDisconnect();
	//normal mode
	AT_IP_Mode(0);
	//enable multiple connections
	AT_MUX(1);
	//create an AP
	AT_set_ssid("Ph_Master","HeManNo1","2","2");

	//set AP ip
	AT_ips('A',"192.168.5.1");

	//STATION Config:
	AT_APConnect("PHANTOM_KING!","HeManNo1");
	
	//set station IP
	AT_ips('S',"192.168.66.2");

	//Configure TCP Side
	//Server:
	AT_Server_Conf(1,666);

	//set timeout as 180
	AT_STimeout(180);
	

	//Client Side:
	//Use the following to start a connection from this anytime :)
	//ip_Start(1,"TCP","192.168.66.1",666);

}


//source for Master = 0 for King = 1
void ATEnter_Raw_data_mode(int id, int source)
{
	if(source == 1 || source == 0)
	{
		//prepare for unvarnished mode
		//delete server
		AT_Server_Conf(0,-1);
	
		//disable multi-connection mode
		AT_MUX(0);
	
		//AT_Server_Conf(1);
		//set mode to unvarnished (note, to end this mode, you need to send a packet with just +++)
		//CIPMUX and CIPSERVER must be 0 before this can be set
		AT_IP_Mode(1);

		//start connection
		//from Master
		if(source == 0)
		{
			AT_IP_Start(-1,"TCP","192.168.66.1",666);	
		}
		//King
		else
		{
			AT_IP_Start(-1,"TCP","192.168.5.1",666);
		}
		_delay_ms(20);
		sendString("AT+CIPSEND\r\n");
		_delay_ms(20);	
	}

}

void ATExit_Raw_data_mode(void)
{
	//send a packet to indicate it's the end now
	sendString("ENDENDENDEND\r\n");
	//to exit a packet with just +++ needs to be sent and nothing must be sent for a while
	_delay_ms(4);
	sendString("+++");
	_delay_ms(4);
	
	//close connection
	AT_IP_Close(-1);
	
	//normal mode
	AT_IP_Mode(0);
	//enable multiple connections
	AT_MUX(1);
	//Configure TCP Side
	//Server:
	AT_Server_Conf(1,666);

	//set timeout as 180
	AT_STimeout(180);
}
/*
	Starts a data stream from Master to King
	Problem, it all happens in parallel, hence I won't know when it happened. 

*/
void Start_DataS_from_Master(void)
{
	if((PORTF.IN & PIN1_bm) == 0)
	{
		//turn on led0
		PORTR.OUTCLR |= PIN0_bm;
		
		//-2 -> close server for single connection; 0 -> source is master
		ATEnter_Raw_data_mode(-2,0);
		
		while((PORTF.IN & PIN2_bm) != 0)
		{
			//turn on led1
			PORTR.OUTCLR |= PIN1_bm;
			//turn off led0
			PORTR.OUTSET |= PIN0_bm;
			
			AT_buildPacket();
			sendString(packet);
			_delay_ms(1);
			/*debug purposes only.
			temp_count++;
			if(temp_count >= 4)
			{
				AT_DecodeStream();
				temp_count = 0;
			}*/
				
		}
		ATExit_Raw_data_mode();
		//turn off led1
		PORTR.OUTSET |= PIN1_bm;
	}
}

//LED configurations
//led0 on-> setting up initial config. 
//led1 on -> stream open
//both off -> stream close
void Start_DataS_from_King(void)
{
	//Press SW1 to start sending data
	if((PORTF.IN & PIN1_bm) == 0)
	{

		
		//turn on led0
		PORTR.OUTCLR |= PIN0_bm;
		//turn off led1
		PORTR.OUTSET |= PIN1_bm;
		
		//-2 -> close server for single connection; 1 -> source is king
		ATEnter_Raw_data_mode(-2,1);

		//enable ADC DMA channel
		ADC_DMACH.CTRLA |= DMA_ENABLE_bm;
		ADC_DMACH.CTRLB |= 0x03; //enable high level interrupts

		
		//Press SW2 to stop sending data
		while((PORTF.IN & PIN2_bm) != 0)
		{
			//turn on led1
			PORTR.OUTCLR |= PIN1_bm;
			//turn off led0
			PORTR.OUTSET |= PIN0_bm;
			
			AT_buildPacket();
			sendString(packet);
			_delay_ms(1);
			
			/*debug purposes only.
			temp_count++;
			if(temp_count >= 4)
			{
				AT_DecodeStream();
				temp_count = 0;
			}*/
			
		}
		//disable ADC DMA channel
		ADC_DMACH.CTRLA &= ~DMA_ENABLE_bm;
		ADC_DMACH.CTRLB &= 0b11111100; //disable interrupts

		ATExit_Raw_data_mode();
		
		//turn off led1
		PORTR.OUTSET |= PIN1_bm;
	}
	
}


void Send_Data_From_Master(char* data)
{
	AT_IP_Start(1,"TCP","192.168.66.1",666);
	_delay_ms(50);

	AT_sendData(1,data);

	_delay_ms(20);
	AT_IP_Close(1);
}

/*
AT_buildPacket

Builds a packet from the last SAMPLES samples from ADC stored into global array result during ISR
converts the 12 bit number to two chars and stores into the another global array packet.

returns packet

*/
char* AT_buildPacket(void)
{
	//reset packet buffer
	memset(packet,0,4*SAMPLES*sizeof(char));
	
	
	//create comma separated chars from the array of 2 byte ints transferred
	int j=0;
	for(int i=0; i < SAMPLES; i++)
	{
		//msb
		packet[j] = ((result[i] >> 8)& 0xFF);
		//strikethrough--check if this msb is visible or not if not, then add 97 to it--no offset
		//packet[j] = (uint8_t) packet[j] > 32?packet[j]:(char)((uint8_t) packet[j] + DAC_OFFSET);	
		j++;
		//lsb
		packet[j] = (result[i] >> 0)& 0xFF;
		j++;
		//note: no comma separated delimiters anymore

	}
	packet[j] = '\r';
	packet[j+1] = '\n';
	//strcat(packet,"\r\n");
	
	return packet;
}


void sendEvent_DAC(uint8_t data)
{

	//write data to event channel's data
	EVSYS.DATA = data;
	//send the data to channel 2 (DAC)
	EVSYS.STROBE |= PIN2_bm;
	
}

/*
SendDatatoDAC

	Takes the comma seperated ints in form of two chars (first with an offset [a sort of low level encryption])
	Converts it to an int and writes it to the DAC via CPU (highest priority) 
	
*/
void sendDatatoDAC(char* packet_data)
{
	const char delimiter[2] = ",";
	char *token;
	int count_i = 0;
	//reset buffer
	memset(DBUFFER,0,SAMPLES*sizeof(int));

	/* get the first token */
	token = strtok(packet_data, delimiter);
	
	/* walk through other tokens */
	while( token != NULL )
	{
		//convert the token to an int and get it
		uint8_t DACtemp1 = (uint8_t) token[0] - DAC_OFFSET;

		int	DACtemp_total = (int) ((DACtemp1 << 8) + token[1]);	
		if(DACtemp_total > 0)
		{
			DBUFFER[count_i] = DACtemp_total;
			//DACX.CH1DATA = DACtemp_total;
			//wait for conversion to be complete
			//DAC_wait();
		}

		
		token = strtok(NULL, delimiter);
		count_i++;
	}
	//RESET DAC
	//DACX.CH1DATA = 0;
	
	/*
		In case this is too unstable for the DAC
		use the DACDMA Function. put these token in an array and then execute that DMA :3 
		
		P.S. don't forget to change the buffer name per the new buffer you make
	
	*/
	//It was too slow! Let's try using DMA
	DAC_TransferStart();
	
}

/*
	AT_DecodePacket
	
	Searches the RX buffer for key words indicating start and end of data packet
	If found, extracts the data and sends it to the DAC using sendDatatoDAC function

*/
void AT_DecodePacket(void)
{
	//find the first occurance of the string
	char * RXresult = strstr(str_res, "IPD");
	char* res_end = strstr(str_res, ",CLOSED");
	
	//only do something if found
	if(RXresult != NULL && res_end != NULL)
	{

		//get the index of the data
		int start_position = RXresult - str_res;
		int end_position = res_end - str_res;
		int new_length = end_position - start_position;
		char new_buf_voice[new_length+1];
		
		for(int i = 0; i < new_length; i++ )
		{
			new_buf_voice[i] = str_res[i+5+ start_position];
		}
		
		//clear the buffer
		memset(str_res,0,(BUFF_LEN * sizeof(char)));
				
		//reset the address for Rx
		RX_Address_Reset();
		//send the data to DAC
		sendDatatoDAC(new_buf_voice);
		//sendDatatoDAC(new_buf_voice,SAMPLES);
		
	}
	//if it's hung
	else if(res_end == NULL && ((strstr(str_res,"0,CON") - str_res) == 0))
	{
		strcat(str_res,",CLOSED");
	}

}


bool CheckArrayForSubStrStart(char* arr, char* sub, int start_at)
{
	if(arr[start_at] == sub[0])
	{
		for(int j=0; j < strlen(sub); j++)
		{
			if(arr[j] != sub[j])
			{
				return false;
			}
		}
		return true;
	}
	return false;
}
/*

	AT_DecodeStream
	
	Looks for first occurrence of IPD in the main rx buffer. once found it assumes it's a stream and registers the start position
	decode_ct is the counter that iterates over the rx buffer. once it finds the first ':' the next bytes are assumed to be ints in 2 
	one byte chars. they are written to the DAC channel by assembling two chars at a time as ints. 
	
	This continues until either the packet ends (denoted by another occurance of "IPD" till ':' and repeats until the keyword "CLOSED" 
	is registered hence closing the stream and going back to normal cycle.
	
	Note: Since this all happens in the CPU, no data can be sent while receive is in progress (singlex communication) 
	

*/
int decode_ct = 0;
void AT_DecodeStream(void)
{
	//reset DAC buffer
	memset(DBUFFER,0,SAMPLES*sizeof(int));
	int count_i = 0;
	//find the first occurrance of +IPD
	char * RXresult = strstr(RXBUFFER, "IPD");
	if(RXresult != NULL)
	{
			//turn on the speaker
			PORTA.OUTSET |= PIN3_bm;
			
			//thats the start
			int start_position = RXresult - RXBUFFER;
			decode_ct =start_position;
			//iterate starting at that position till you find the first ':'
			while(RXBUFFER[decode_ct] != ':')
			{
				decode_ct++;
			}
			//clean everything uptil this point
			//memset(str_res,0, start_position * sizeof(int));
			
			//start loading the numbers in array till you a) hit the IPD b) hit the CLOSED or end of array (avoid going overboard
			//if you hit b) break the loop.
			int toggle = 0;
			while((RXBUFFER[decode_ct] != 'E' || RXBUFFER[decode_ct+1] != 'N'))
			{
				
				//in a) skip over the IPD packet start till you hit ':' again and repeat
				if((RXBUFFER[decode_ct] == 'I' && RXBUFFER[decode_ct+1] == 'P' && RXBUFFER[decode_ct +2] == 'D') || (RXBUFFER[decode_ct] == '\r' && RXBUFFER[decode_ct+1] == '\n') )
				{
					//iterate starting at that position till you find the first ':'
					while(RXBUFFER[decode_ct] != ':')
					{
						decode_ct++;
					}
				}
				//write to the dac
				//convert the token to an int and get it
				//uint8_t DACtemp1 = (uint8_t)  - DAC_OFFSET;
				//error checking
				if(RXBUFFER[decode_ct] > RXBUFFER[decode_ct+1])
				{
					decode_ct++;
				}
				int	DACtemp_total = (int) ((RXBUFFER[decode_ct] << 8) + RXBUFFER[decode_ct+1]);
				if(DACtemp_total >= 0)
				{
					DBUFFER[count_i] = DACtemp_total;
					//DACX.CH1DATA = DACtemp_total;
				    //DAC_wait();
				}				
				
				//skip two bytes that have been written
				decode_ct+=2;
				
				//if decode ct goes above limit set in serial functions, switch channels and reset the current one
				if(decode_ct >= RXBUFF_LEN - 10 || decode_ct < 0)
				{
					RX_Toggle_DMACH(&toggle);
					decode_ct = 0;
				}
								
				count_i++;
				
				//every SAMPLES samples are transferred to DAC via DMA
				//check if the buffer is full
				if(count_i == SAMPLES)
				{
					//reset the count
					count_i = 0;
					//send the data to dac
					DAC_TransferStart();
				}
			}/*
			if(count_i > 0 && count_i < SAMPLES)
			{
				DAC_TransferStart();
			}*/
			
			//clear the buffer
			memset(RXBUFFER,'?',(RXBUFF_LEN * sizeof(char)));
					
			//turn off the speaker
			PORTA.OUTCLR |= PIN3_bm;		
			//reset the address for Rx <- this loses data
			RX_Address_Reset();
			
	}

}


