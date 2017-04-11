/*
 * AT_Based_Library.h
 *
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
