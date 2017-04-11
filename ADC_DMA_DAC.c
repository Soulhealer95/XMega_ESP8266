/*
 * ADC_DMA_DAC.c
 *
 * Created: 2017-03-24 1:08:39 AM
 *  Author: Test
 */ 
#include "ADC_DMA_DAC.h"


void clock_config(int type)
{
	switch(type)
	{
		
		//2MHz
		case 0:
		CLK.CTRL &= 0x00;
		break;
		//32Mhz
		case 1:
		OSC_CTRL |= OSC_RC32MEN_bm; //Setup 32Mhz crystal
		
		while(!(OSC_STATUS & OSC_RC32MRDY_bm));
		
		CCP = CCP_IOREG_gc; //Trigger protection mechanism
		CLK_CTRL = CLK_SCLKSEL_RC32M_gc; //Enable internal  32Mhz crystal
		break;
		//32.7kHz
		case 2:
		CLK.CTRL &= 0x02;
		break;
		//External
		case 3:
		CLK.CTRL &= 0x03;
		break;
		//PLL
		default:
		CLK.CTRL &= 0x04;
		break;
	}
}

void DAC_datain (int x)
{
	DACX.CH1DATA = x;
}


void DAC_wait(void)
{
	//check if the data to be converted may be written on channel 1 (0x02)
	while (!(DACX.STATUS & DAC_CH1DRE_bm)){};
}


void DAC_init()
{
	//setting PB3 == DACB1 as output
	//PORTB_DIR |= 0x04; //3rd bit
	PORTB_DIR |= PIN3_bm; //0x08 -- 4th Bit
	
	//enable the entire DAC enable (bit0)
	DACX.CTRLA |= DAC_ENABLE_bm;//  0x01;
	
	//enable the IOEN (internal output enable) bit 4 (5th bit)
	//DACX.CTRLA |= 0x10;
	//enable the channel 1 as output
	DACX.CTRLA |= DAC_CH1EN_bm; //0x08;
	
	//enable the channel 1 on single mode (bit 5:6 in 01)
	DACX.CTRLB |= 0x20;
	
	//set the reference of DAC to aVcc (bit 3:4 in 01)
	DACX.CTRLC |= DAC_REFSEL_AREFB_gc;  //0x00;// 1V try?   0x08;
	
	
	//Enabling Events
	
	//enable event system channel 2
	EVSYS.CH2CTRL |= 0x01;
	
	//set the event to be triggered by software
	EVSYS.CH2MUX |= EVSYS_CHMUX_OFF_gc;//EVSYS_CHMUX_ADCA_CH0_gc;
	
	//set event channel 2 as trigger source
	DACX.EVCTRL |= DAC_EVSEL_2_gc;
	
	
	//check if the data to be converted may be written on channel 1 (0x02)
	DAC_wait();
	
}


void adc_Wait(void)
{
	while(ADCX.INTFLAGS != 0x04){};
}


//turns the ADCA on/off.
void adcA_switch(int status)
{
	if(status == 1)
	{
		ADCX.CTRLA |= 0x01;
	}
	else
	{
		ADCX.CTRLA &= 0b11111100;
	}
	
}

void DMA_wait(void)
{
	
	while((DMA.STATUS & DMA_CH0BUSY_bm ||  DMA.STATUS & DMA_CH2BUSY_bm)!= 0){};
}

//enable DMA channel 0
void DMA_Conf(void)
{
	//enable the DMA controller
	DMA.CTRL |= (DMA_ENABLE_bm | DMA_PRIMODE_CH0123_gc);
	
	//reset DMA (before enabling)
	DMA.CTRL |= DMA_RESET_bm;

	//wait DMA to complete pending transfers --> reset
	//while((DMA.STATUS & DMA_CH0BUSY_bm)!= 0);
	
	Rx_DMA_Conf();
	Rx_BDMA_Conf();
	ADC_DMA_Conf();
	DAC_DMA_Conf();
//	DAC_DMAStream_Conf();
}



void ADC_DMA_Conf(void)
{
	//set burst transfer length to 2-Bytes (bit 1 and 0 set to: 01)
	//enabling the single mode ensures the channel is never disabled
	ADC_DMACH.CTRLA = DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm;					// ADC result is 2 byte 12 bit word
	
	
	ADC_DMACH.ADDRCTRL |=	 (DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc);
	
	
	//setting up interrupts
	
	//Enable global interrupts.
	sei();
	
	//enable and configure to trigger at high priority by setting the bits at 0x03
	ADC_DMACH.CTRLB |=  0x03;
	
	//select Trigger Source as Software and not ADCA ch0 (0x10 + 0x02) and not Event System (0x01 + 0x00|0x01|0x02) and not  DACB (0x25 + 0x00|0x01)
	ADC_DMACH.TRIGSRC =  (0x10 + 0x00);
	
	//set the number of bytes to be transfer per block
	ADC_DMACH.TRFCNT = 128;
	
	//configure DMA source address to ADCA CH2's result and dest as the buffer
	Byte ADCA_Lowernib_bm =  (((uint16_t) &ADC_CH.RES >> 0) & 0xFF);
	Byte ADCA_Highernib_bm = ((((uint16_t) &ADC_CH.RES >> 8) & 0xFF));
	Byte Buffer_Lowernib_bm =  (((uint16_t) &BUFFER[0] >> 0) & 0xFF);
	Byte Buffer_Highernib_bm = (((uint16_t) &BUFFER[0] >> 8) & 0xFF);

	ADC_DMACH.SRCADDR0 = ADCA_Lowernib_bm; //((ADCA_CH0_RES & 0x00FF) >> 8);// 0x34;    //    //(ADCCH.RESL);//Doesn't change/ Looking for value?
	ADC_DMACH.SRCADDR1 =  ADCA_Highernib_bm; //((ADCA_CH0_RES & 0xFF00) >> 8);//0x02;    //
	ADC_DMACH.SRCADDR2  = 0x00;
	
	//Configure DMA destination address to DACB CH1's input
	ADC_DMACH.DESTADDR0 = Buffer_Lowernib_bm; //  0x38;//DACB_CH0DATA; address 0x0338
	ADC_DMACH.DESTADDR1 = Buffer_Highernib_bm; //0x03;//DACB_CH0DATA;
	ADC_DMACH.DESTADDR0 = 0x00;
	
	//enable DMA channel
	//ADC_DMACH.CTRLA |=  DMA_ENABLE_bm;//0x80;
}

//enable DMA channel 0
void DAC_DMA_Conf(void)
{
	
	
	DAC_DMACH.REPCNT = SAMPLES;
	//BURSTLEN IS SIZE OF INT = 2 bytes
	DAC_DMACH.CTRLA = DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm;					// ADC result is 2 byte 12 bit word
	
	DAC_DMACH.ADDRCTRL |=	 (DMA_CH_SRCRELOAD_TRANSACTION_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc);	// reload dest after every transaction
	
	//setting up interrupts
	//Enable global interrupts. yes, again!
	//sei();
	
	//TURN OFF INTERRUPTS
	DAC_DMACH.CTRLB |=  0x00;
	
	//reload the source address(msb) and destination (lsb) after every transaction (0xc0) and keep it fixed (0x00)
	//DAC_DMACH.ADDRCTRL |= 0xcc;
	
	//select Trigger Source as Software -- ADCA ch0 (0x10 + 0x02) and not Event System (0x01 + 0x00|0x01|0x02) and not  DACB (0x25 + 0x00|0x01)
	DAC_DMACH.TRIGSRC |=  0x00;//(0x10 + 0x00); //change this to event channel 0
	
	//set the number of bytes = number of ints times size of int in bytes (2)
	DAC_DMACH.TRFCNT = (2*SAMPLES);
	
	//configure DMA source address to ADCA CH2's result
	Byte BUFFER_Lowernib_bm =  (((uint16_t) &DBUFFER[0] >> 0) & 0xFF);
	Byte BUFFER_Highernib_bm = ((((uint16_t) &DBUFFER[0] >> 8) & 0xFF));
	Byte DACB_Lowernib_bm =  (((uint16_t) &DACX.CH1DATA >> 0) & 0xFF);
	Byte DACB_Highernib_bm = (((uint16_t) &DACX.CH1DATA >> 8) & 0xFF);

	DAC_DMACH.SRCADDR0 = BUFFER_Lowernib_bm; //((ADCA_CH0_RES & 0x00FF) >> 8);// 0x34;    //    //(ADCCH.RESL);//Doesn't change/ Looking for value?
	DAC_DMACH.SRCADDR1 =  BUFFER_Highernib_bm; //((ADCA_CH0_RES & 0xFF00) >> 8);//0x02;    //
	DAC_DMACH.SRCADDR2  = 0x00;
	
	//Configure DMA destination address to DACB CH1's input
	DAC_DMACH.DESTADDR0 = DACB_Lowernib_bm; //  0x38;//DACB_CH0DATA; address 0x0338
	DAC_DMACH.DESTADDR1 = DACB_Highernib_bm; //0x03;//DACB_CH0DATA;
	DAC_DMACH.DESTADDR0 = 0x00;
	
	//enable DMA channel
	DAC_DMACH.CTRLA |=  DMA_ENABLE_bm;//0x80;
}

////enable DMA channel 0
//void DAC_DMAStream_Conf(void)
//{
	//
	//
	//DAC_STRCH.REPCNT = 1;
	////BURSTLEN IS SIZE OF INT = 2 bytes
	//DAC_STRCH.CTRLA |= (DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm);
	//
	//DAC_STRCH.ADDRCTRL |=	 (DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc);	// reload dest after every transaction
	//
	////setting up interrupts
	////Enable global interrupts. yes, again!
	////sei();
	//
	////TURN OFF INTERRUPTS
	//DAC_STRCH.CTRLB |=  0x00;
	//
	////reload the source address(msb) and destination (lsb) after every transaction (0xc0) and keep it fixed (0x00)
	////DAC_STRCH.ADDRCTRL |= 0xcc;
	//
	////select Trigger Source as Software -- ADCA ch0 (0x10 + 0x02) and not Event System (0x01 + 0x00|0x01|0x02) and not  DACB (0x25 + 0x00|0x01)
	//DAC_STRCH.TRIGSRC |=  0x00;//(0x10 + 0x00); //change this to event channel 0
	//
	////set the number of bytes = number of ints times size of int in bytes (2)
	//DAC_STRCH.TRFCNT = (10);
	//
	////configure DMA source address to ADCA CH2's result
	//Byte BUFFER_Lowernib_bm =  (((uint16_t) &DVAR >> 0) & 0xFF);
	//Byte BUFFER_Highernib_bm = ((((uint16_t) &DVAR >> 8) & 0xFF));
	//Byte DACB_Lowernib_bm =  (((uint16_t) &DACX.CH1DATA >> 0) & 0xFF);
	//Byte DACB_Highernib_bm = (((uint16_t) &DACX.CH1DATA >> 8) & 0xFF);
//
	//DAC_STRCH.SRCADDR0 = BUFFER_Lowernib_bm; //((ADCA_CH0_RES & 0x00FF) >> 8);// 0x34;    //    //(ADCCH.RESL);//Doesn't change/ Looking for value?
	//DAC_STRCH.SRCADDR1 =  BUFFER_Highernib_bm; //((ADCA_CH0_RES & 0xFF00) >> 8);//0x02;    //
	//DAC_STRCH.SRCADDR2  = 0x00;
	//
	////Configure DMA destination address to DACB CH1's input
	//DAC_STRCH.DESTADDR0 = DACB_Lowernib_bm; //  0x38;//DACB_CH0DATA; address 0x0338
	//DAC_STRCH.DESTADDR1 = DACB_Highernib_bm; //0x03;//DACB_CH0DATA;
	//DAC_STRCH.DESTADDR0 = 0x00;
	//
	////enable DMA channel
	//DAC_STRCH.CTRLA |=  DMA_ENABLE_bm;//0x80;
//}
//


//should sweep adca ch0 continuously for values
void ADCA_EVENT_enable(void)
{

	//ADC Configuration
	ADC_CH.CTRL |= 0x01; //set inputmode bits to 1 (single-ended)
	
	//selecting the input  --- bits [6:3] define the input from 0(0000) - 15(1111)
	ADC_CH.MUXCTRL |= 0b00010111; //selecting ADC2 as input
	
	//configure events
	
	//enable event system channel 0
	EVSYS.CH0CTRL |= 0x01;
	
	//set the event channel to ch0
	EVSYS.CH0MUX |= 0x20; //EVSYS_CHMUX_ADCX_CH0_gc;
	
	//sweep on ch0, event channels 0-3, lowest number on ch0
	ADCX.EVCTRL |= ( ADC_EVACT0_bm);
	
	//Enable 12-bit resolution (bit 0&1 define this setting)
	ADCX.CTRLB &= 0b11111100;
	
	//set ADCA to free run mode
	ADCX.CTRLB |= ADC_FREERUN_bm;
	
	
	//Set the Vref bits 4-6 (000- int1v 1V 001- intvcc/1.6 010-Aref on portA 011- ArefB 100-Vcc/2)
	ADCX.REFCTRL |= ADC_REFSEL_AREFA_gc; //ADC_REFSEL_INTVCC2_gc;//0b00110000; //Vref = Vcc/2
	
	//selecting 32 MHz clock
	clock_config(1);
	
	//set the clock rate (Prescaler)
	ADCX.PRESCALER |= 0x03; //writing msb to 0 unused bits per guide and lsb to 3 to select /32 prescale
	
	//config interrupts
	sei();
	
	// Ensure the conversion complete flag is cleared
	ADCX.INTFLAGS = ADC_CH2IF_bm; // 0x01
	
	// Enable Conversion Complete Interrupt with high priority -- turn interrupts off!
	//ADCCH.INTCTRL |= ADC_CH_INTLVL1_bm | ADC_CH_INTLVL0_bm; // 0x03
	
	PMIC_CTRL |= PMIC_HILVLEN_bm;  //0x04; // enable high level interrupts (3rd bit)
	
	//turn on ADC
	ADCX.CTRLA |= ADC_ENABLE_bm; //0x01
	
}



void DAC_TransferStart(void)
{
	//enable the DAC DMA Channel
	//set burst transfer length to 2-Bytes (bit 1 and 0 set to: 01)
	DAC_DMACH.CTRLA |=  DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm;//0x01;
	
	//enable DMA channel
	DAC_DMACH.CTRLA |=  DMA_ENABLE_bm;//0x80;
	
	
	
	//start DAC transfer
	DAC_DMACH.CTRLA |= DMA_CH_TRFREQ_bm;
	
	//change trigger source to dacb1
	//DAC_DMACH.TRIGSRC |= 0x26;
	
	//wait for DAC transfer to finish
	while((DMA.STATUS & DMA_CH2BUSY_bm) != 0){};
	
	//DACX.CH1DATA = 0;
}
//void DAC_SreamStart(void)
//{
	////enable the DAC DMA Channel
	////set burst transfer length to 2-Bytes (bit 1 and 0 set to: 01)
	//DAC_STRCH.CTRLA |=  DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm;//0x01;
	//
	////enable DMA channel
	//DAC_STRCH.CTRLA |=  DMA_ENABLE_bm;//0x80;
	//
	////start DAC transfer
	//DAC_STRCH.CTRLA |= DMA_CH_TRFREQ_bm;
	//
	////wait for DAC transfer to finish
	//while((DMA.STATUS & DMA_CH2BUSY_bm) != 0){PORTR.OUTCLR |= PIN0_bm;};
	//
	//_delay_ms(1);
	//PORTA.OUTSET |= PIN0_bm;
	//
	//DACX.CH1DATA = 0;
//}
//
//
