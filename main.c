//************************* XMega **************************

//
// Configures an ESP8266 Module (USARTD0) as PHANTOM_KING!
// Master Needs to be On
// Allows Communication through MIC -> ADCA 
// and Receives Data via Stream and plays it through DACB over a speaker output
// Takes about 15secs to Establish a Stream and is recommended to not keep it open 
// for longer than 10 seconds for best use
// Uses On board Switch S1 and LEDs 0/1
//

// S. Saxena											2017

//

//********************** INCLUDES (libraries) *********************
#include "AT_Based_Library.h"

//****************************** DEFINITIONS **********************/


extern int ADC_buff[BUFF_LEN];

//****************Function Prototypes*********************/
//Overall_Functions
void Peripherals_Config(void);
void IO_config(void);

//variables to store the results

//Without the volatile keyword, the compiler may optimize out accesses to a variable you update in an ISR
// as the C language itself has no concept of different execution threads
extern char packet[4*SAMPLES];
extern volatile int result[SAMPLES];
volatile int count = 0;
//volatile int rx_count = 0;
volatile bool packet_condition = false;

//interrupt routine
/*ISR(DMA_CH0_vect)
{
	rx_count++;

}
*/

ISR(DMA_CH2_vect)
{
	//DACX.CH1DATA = ADC_buff[0];
	result[count] = ADC_buff[0];
	count++;
	
	//every 250th sample wait for transfer to finish
	if(count == 250)
	{
		count = 0;
	}

}	

//**************************** MAIN ********************************

int main(void)

{
	
	//Enable Global interrupts
	sei();

	//set i/o
	IO_config();
	
	//enable ADC and DAC
	Peripherals_Config();
	
	//wait for the module to turn on 
	_delay_ms(RES_DLY);
	//configure the module as master (King needs to be on)
	PORTR.OUTCLR |= PIN0_bm;
	AT_init_Config_King();


	while(1)
	{
		//Check for DATA stream start condition 	
		Start_DataS_from_King();
		PORTR.OUTSET |= PIN1_bm;
		
		//Check buffer for data to decode	
		AT_DecodeStream();
		PORTR.OUTSET |= PIN0_bm; 
	}

}






/************************   Function Implementations ************************/

void IO_config(void)
{
		
		//configure I/O PA2 clearing means input. setting means output
		PORTA.IN |= PIN2_bm;
		PORTA.DIRCLR = PIN2_bm;
		
		
		//PB3 as output for DACB CH1
		PORTB.DIR |= PIN3_bm;
		PORTA.DIR |= PIN3_bm;
				
		//Serial I/O Config
				
		//PE3 is output since it's TXD
		PORTE.DIR |= PIN3_bm;
				
		//PE2 is input since it's RXD which is default
		//button is PF1
		//led is Pr1 for data send start
		//led pr0 for data receive
		PORTR.DIR |= (PIN1_bm| PIN0_bm);
		
		
}




void Peripherals_Config(void)
{
	
	
	//configure the DAC
	DAC_init();
	
	//Configure the DMA
	DMA_Conf();
	
	//configure the ADC with events
	ADCA_EVENT_enable();
	
	Serial_conf();

}



//****************************** END ******************************************
