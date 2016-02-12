// Part of traqmate.c
// 11/10/2003
// Author: GAStephens
//
// This file contains all the common functions to setup the Cygnal 8051 for use in the 
// traqmate program including the system clock, SPI port, UARTs, timers, and
// crossbar switch. 

#ifdef SENSOR
#include <sensor.h>
#ifdef TSIP
#include <tsip.h>
#else
#include <ubx.h>
#endif
#endif
#ifdef DISPLAY
#include <display.h>
#endif
#include <stdlib.h>

// This function starts an interrupt-driven transmit of the trasmit buffer 
// on designated UART. Blocks only if previous transmission is in progress.
void XmitUART(comporthandle *handle, u16 numbytes) {

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

// ***** don't think we need to check CTS here but maybe
	while (0 != handle->txbytes) ;		// wait for previous transmission to finish
//	while (0 != CTS[handle->port]);
	handle->txptr = handle->txbuff;
	handle->txbytes = numbytes;
	if (0 == handle->port) {
		SFRPAGE = UART0_PAGE;
		TI0 = 1;						// generate an interrupt to get started
	}
	else {
		SFRPAGE = UART1_PAGE;
		TI1 = 1;						// generate an interrupt to get started
	} // else
	SFRPAGE = SFRPAGE_SAVE;
}

// simultaneously write	and read one unsigned char to SPI (mode 3). Blocking.
unsigned char xferSPI (unsigned char ch) {

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	SFRPAGE = SPI0_PAGE;

	SPI0DAT = ch;

	// wait for shifting
	while (!SPIF) ;

	SPIF = 0;			// clear the xfer complete flag

	ch = SPI0DAT;
	
	SFRPAGE = SFRPAGE_SAVE;

	return (ch);
}

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// This routine initializes the system clock.
//
void SYSCLK_Init (void)
{
#ifdef INTCLK
	int i;									// software timer
	u08 SFRPAGE_SAVE = SFRPAGE;				// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;					// set SFR page
	SFRPGCN = 0x01;							// turn on auto SFR paging for interrupts

	OSCICN = 0x83;							// 10000011 set internal oscillator to run at max frequency
	CLKSEL = 0x00;							// Select the internal osc. as the SYSCLK source

	PLL0CN= 0x00;							// Set internal osc. as PLL source

	SFRPAGE = LEGACY_PAGE;
	FLSCL	= 0x30;							// Set FLASH read time for 100MHz clk

	SFRPAGE = CONFIG_PAGE;
	PLL0CN |= 0x01;							// Enable Power to PLL
	PLL0DIV = 0x01;							// Set Pre-divide value to N (N = 1)
	PLL0FLT = 0x01;							// 00000001 Set the PLL filter register for
											// a reference clock from 19 - 30 MHz
											// and an output clock from 65-100 MHz
	PLL0MUL = 0x04;							// Multiply SYSCLK by M (M = 4)

	for (i=0; i < 256; i++) ;				// Wait at least 5us
	PLL0CN |= 0x02;							// Enable the PLL
	while(!(PLL0CN & 0x10));				// Wait until PLL frequency is locked
	CLKSEL= 0x02;							// Select PLL as SYSCLK source

	SFRPAGE = SFRPAGE_SAVE;					// Restore SFR page
#else // external clock
	int i;									// software timer
	u08 SFRPAGE_SAVE = SFRPAGE;				// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;					// set SFR page
	SFRPGCN = 0x01;							// turn on auto SFR paging for interrupts

	OSCXCN = 0x67;                  	    // 01100111 start external oscillator

	for (i=0; i < 256; i++);				// XTLVLD blanking interval (>1ms)
	
	while (!(OSCXCN & 0x80));				// Wait for crystal osc. to settle

	CLKSEL = 0x01;							// Select the external osc. as the SYSCLK source
	OSCICN = 0x00;							// 00000000 turn off internal osc

#ifdef EXTCLK4		// 24.576, no PLL
	SFRPAGE = LEGACY_PAGE;
	FLSCL	= 0x00;							// Set FLASH read time for 25MHz clk
#else // PLL
	PLL0CN = 0x05;							// external osc src, powered

	SFRPAGE = LEGACY_PAGE;

#ifdef EXTCLK5
	FLSCL	= 0x10;							// Set FLASH read time for 50MHz clk
#elif defined EXTCLK6
	FLSCL	= 0x20;							// Set FLASH read time for 75MHz clk
#else
	FLSCL	= 0x30;							// Set FLASH read time for 100MHz clk
#endif

	SFRPAGE = CONFIG_PAGE;
	PLL0DIV = 0x01;							// Set Pre-divide value to N (N = 1)
	PLL0FLT = 0x01;							// Set the PLL filter register for
											// a reference clock from 19 - 30 MHz
											// and an output clock from 65 - 100 MHz
#ifdef EXTCLK5
	PLL0MUL = 0x02;							// Multiply SYSCLK by M (M = 2)
#endif
#ifdef EXTCLK6
	PLL0MUL = 0x03;							// Multiply SYSCLK by M (M = 3)
#endif
#if defined (EXTCLK1) || defined (EXTCLK3)
	PLL0MUL = 0x04;							// Multiply SYSCLK by M (M = 4)
#endif
#ifdef EXTCLK2
	PLL0MUL = 0x05;							// Multiply SYSCLK by M (M = 4)
#endif

	for (i=0; i < 256; i++) ;				// Wait at least 5us
	PLL0CN |= 0x02;							// Enable the PLL
	while(!(PLL0CN & 0x10));				// Wait until PLL frequency is locked
	CLKSEL= 0x02;							// Select PLL as SYSCLK source
#endif // PLL

	SFRPAGE = SFRPAGE_SAVE;					// Restore SFR page
#endif
}

//-----------------------------------------------------------------------------
// UART_Init
//-----------------------------------------------------------------------------
//
// Configure the UART for interrupt driven operation, <baud> and <parity>.
//
void UART_Init (comporthandle *handle, u32 baud)
{
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	switch(handle->port) {
		case 0:									// Use Timer 2 for baud

			SFRPAGE = UART0_PAGE;
			SSTA0 = 0x15;						// 00010101 no doubler, user timer 2

			switch (handle->parity) {
				case 'n':
				case 'N':
					SCON0 = 0x50;				// SCON0: mode 1, 8-bit UART, enable RX
					break;
				case 'e':
				case 'E':
				case 'o':
				case 'O':
					SCON0 = 0xD0;				// SCON0: mode 3, 9-bit UART, enable RX
					break;
			} // switch

			SFRPAGE = TMR2_PAGE;
			TMR2CN = 0x04;						// 00000100 no ext trig, enabled, timer, auto reload
			TMR2CF = 0x08;						// 00001000 sysclk source, count up

			RCAP2 = -(SYSCLK/baud/16);			// set timer 2 reload value
			TMR2 = 0xFFFF;						// set to reload immediately
			TR2 = 1;							// start timer

			SFRPAGE = UART0_PAGE;
			RI0 = TI0 = 0;						// Clear interrupt flags
			ES0 = 1;							// Enable UART0 interrupts
			break;
		case 1:									// User Timer 1 for baud

			SFRPAGE = UART1_PAGE;
			switch (handle->parity) {
				case 'n':
				case 'N':
					SCON1 = 0x10;				// 00010000 SCON1: mode 1, 8-bit UART, enable RX
					break;
				case 'e':
				case 'E':
				case 'o':
				case 'O':
					SCON1 = 0x90;				// 10010000 SCON1: mode 3, 9-bit UART, enable RX
					break;
			} // switch

			SFRPAGE = TIMER01_PAGE;
			TMOD	&= ~0xF0;
			TMOD	|=  0x20;						// TMOD: timer 1, mode 2, 8-bit reload

			if (SYSCLK/baud/2/256 < 1) {
				TH1 = -(SYSCLK/baud/2);
				CKCON |= 0x10;						// T1M = 1; SCA1:0 = xx
			} else if (SYSCLK/baud/2/256 < 4) {
				TH1 = -(SYSCLK/baud/2/4);
				CKCON &= ~0x13;						// Clear all T1 related bits
				CKCON |=0x01;						// T1M = 0; SCA1:0 = 01
			} else if (SYSCLK/baud/2/256 < 12) {
				TH1 = -(SYSCLK/baud/2/12);
				CKCON &= ~0x13;						// T1M = 0; SCA1:0 = 00
			} else {
				TH1 = -(SYSCLK/baud/2/48);
				CKCON &= ~0x13;						// Clear all T1 related bits
				CKCON |=0x02;						// T1M = 0; SCA1:0 = 10
			}

			TL1 = TH1;								// initialize Timer1
			TR1 = 1;								// start Timer1

			SFRPAGE = UART1_PAGE;
			RI1 = TI1 = 0;							// Indicate TX1 ready

			EIE2 |= 0x40;							// ES1=1, Enable UART1 interrupts
			break;
	} // switch

	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page

} // UART_Init

// This function sets up the Cygnal 8051 for use with SPI
void SPIO_Init(void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = SPI0_PAGE;

	// program SPI0
	SPI0CKR = SPI_CLOCK_DIVIDE;			// set clock rate
	SPI0CFG = SPI_CONFIG_VALUE;			// setup up	configuration register
	SPIF = 0;							// clear interrupt flag
	SPIEN = 1;							// enable SPI

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}

//-----------------------------------------------------------------------------
// Timer3_Init
//-----------------------------------------------------------------------------
//
// Configure Timer3 to auto-reload at interval specified by <counts> and generate
// an interrupt which will start a sample sycle. Uses SYSCLK/12 as its time base.
//
void Timer3_Init (u16 counts)
{
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = TMR3_PAGE;

	TMR3CN = 0x00;						// Stop Timer3; Clear TF3;
	TMR3CF = 0x00;						// use SYSCLK/12 as timebase, count up

	RCAP3 = - (int) (counts + (1280/SAMPLE_RATE));	// Init reload values + crystal error margin

	TMR3 = 0xFFFF;						// set to reload immediately
	EIE2 |= 0x01;						// enable Timer3 interrupts
	TMR3CN |= 0x04;						// start Timer3

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}
//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

// MSG0 ISR - Message Protocol on UART 0
#define COM 0
void MSG0_ISR (void) interrupt 4 {

// this keeps the message protocol state machine in a single place
#include <msgstatemachine.c>

} // MSG0_ISR
#undef COM

#ifdef DISPLAY
#define COM 1
// MSG1 ISR - Message Protocol on UART 1
void MSG1_ISR (void) interrupt 20 {

// this keeps the message protocol state machine in a single place
#include <msgstatemachine.c>

} // MSG1_ISR
#endif // DISPLAY
#undef COM

#ifdef SENSOR
#ifdef TSIP
// TSIP1 ISR - TSIP binary protocol on UART 1
void TSIP1_ISR (void) interrupt 20 {
	static xdata int rxnextchar = 0;
	static xdata u08 rxnextidx = 0;
	static xdata enum tsip_state {	FIND_START,		// looking for starting DLE
								FOUND_DLE,		// found a DLE
								CAPTURE_PACKET,	// found starting DLE so capturing data
								CAPTURE_DLE		// capturing data, found a DLE
								} state = FIND_START;
	static u08 *tsipbuff;						// for quicker access to buffer
	char tempch;

	// determine if tx or rx or both caused interrupt
	if (SCON1 & 0x01) {			// byte received
		tempch = SBUF1;
		switch (state) {
			case FIND_START:
				if (DLE == tempch) state = FOUND_DLE;
				break;	
			case FOUND_DLE:
				if ((DLE == tempch) || (ETX == tempch)) state = FIND_START;
				else {
					state = CAPTURE_PACKET;
					tsipbuff = com[1].rxbuff+(com[1].rxnextidx*TSIPRXSIZE);	// for quicker access to buffer
					tsipbuff[rxnextchar++] = tempch;
				}
				break;
			case CAPTURE_PACKET:
				if (DLE == tempch) state = CAPTURE_DLE;
				else tsipbuff[rxnextchar++] = tempch;
				break;
			case CAPTURE_DLE:
				if (DLE == tempch) {		// DLE char is just data so save it
					tsipbuff[rxnextchar++] = tempch;
					state = CAPTURE_PACKET;
				}
				else {
					if (ETX == tempch) {	// found a complete message		
						if (NULL == com[1].rxfirstidx) {
							com[1].rxfirstidx = tsipbuff;
							rxnextidx ^= 1;
							rxnextchar = 0;
						} // if			// if not done processing dump the line
						else {
							rxnextchar = 0;
						} // else
						state = FIND_START;
					} // if
					else {			// single DTE so must be at start again
						state = CAPTURE_PACKET;
						rxnextchar = 1;
						tsipbuff[0] = tempch;
					} // else
				} // else
				break;
		} // switch
		SCON1 &= 0xFE;						// clear receive interrupt flag
	}

	if (SCON1 & 0x02) {		// byte transmitted
		if (0 != com[1].txbytes) {
			// set parity flag
			switch (com[1].parity) {
				case 'o':
				case 'O':
					if (*(com[1].txptr) % 2) SCON1 &= ~0x04;
					else SCON1 |= 0x04;
					break;
				case 'e':
				case 'E':
					if (*(com[1].txptr) % 2) SCON1 |= 0x04;
					else SCON1 &= ~0x04;
					break;
			} // switch
			SBUF1 = *(com[1].txptr);
			com[1].txptr++;
			com[1].txbytes--;
		}
		else {	
			com[1].txptr = NULL;
		}
		SCON1 &= 0xFD;						// clear transmit interrupt flag
	}
}
#else
// UBX_ISR - UBX binary protocol on UART 1
void UBX1_ISR (void) interrupt 20 {
	static xdata int rxnextchar = 0;
	static xdata enum ubx_state {
				FIND_START1,		// looking for first synch char
				FIND_START2,		// looking for second synch char
				GET_CLASS,			// found synch so get the class and verify
				GET_MSGID,			// get the message id
				GET_LEN1,			// looking for LSB of payload length
				GET_LEN2,			// looking for MSB of payload length
				CAPTURE_PACKET,		// capturing data in buffer
				GET_CKA,			// verify checksum A
				GET_CKB				// verify checksum B
	} state = FIND_START1;

	static u08 *ubxbuff;						// for quicker access to buffer
	static u16 len;								// length of buffer payload
	static u08 cka, ckb;						// A and B checksums
	u08 tempch;

	// determine if tx or rx or both caused interrupt
	if (SCON1 & 0x01) {			// byte received
		tempch = SBUF1;
		switch (state) {
			case FIND_START1:
				if (SYNCH1 == tempch)
					state = FIND_START2;
				break;
			case FIND_START2:
				if (SYNCH2 == tempch)
					state = GET_CLASS;
				else state = FIND_START1;
				break;
			case GET_CLASS:
				cka = ckb = 0;
				rxnextchar = 0;
				ubxbuff = com[1].rxbuff+(com[1].rxnextidx*UBXRXSIZE);	// for quicker access to buffer
				ubxbuff[rxnextchar++] = tempch;
				cka = cka + tempch;
				ckb = ckb + cka;
				state = GET_MSGID;
				break;
			case GET_MSGID:
				ubxbuff[rxnextchar++] = tempch;
				cka = cka + tempch;
				ckb = ckb + cka;
				state = GET_LEN1;
				break;
			case GET_LEN1:
				len = tempch;
				cka = cka + tempch;
				ckb = ckb + cka;
				state = GET_LEN2;
				break;
			case GET_LEN2:
				len += tempch << 8;
				cka = cka + tempch;
				ckb = ckb + cka;
				if (0 == len)
					state = GET_CKA;
				else
					state = CAPTURE_PACKET;
				break;
			case CAPTURE_PACKET:
				ubxbuff[rxnextchar++] = tempch;
				cka = cka + tempch;
				ckb = ckb + cka;
				if (rxnextchar == (len+2))
					state = GET_CKA;
				break;
			case GET_CKA:
				if (cka == tempch)
					state = GET_CKB;
				else
					state = FIND_START1;
				break;
			case GET_CKB: {
				// post buffer if open buffer available and second checksum good
				u08 temp = (com[1].rxnextidx == (GPSRXBUFCNT-1))? 0 : com[1].rxnextidx + 1;
				// if not done processing dump the line
				if ((ckb == tempch) && (temp != com[1].rxfirstidx))
					com[1].rxnextidx = temp;	// if done, go to next buffer slot
				state = FIND_START1;
				} break;
		} // switch
		SCON1 &= 0xFE;						// clear receive interrupt flag
	}

	if (SCON1 & 0x02) {		// byte transmitted
		if (0 != com[1].txbytes) {
			// set parity flag
			switch (com[1].parity) {
				case 'o':
				case 'O':
					if (*(com[1].txptr) % 2) SCON1 &= ~0x04;
					else SCON1 |= 0x04;
					break;
				case 'e':
				case 'E':
					if (*(com[1].txptr) % 2) SCON1 |= 0x04;
					else SCON1 &= ~0x04;
					break;
			} // switch
			SBUF1 = *(com[1].txptr);
			com[1].txptr++;
			com[1].txbytes--;
		}
		else {	 
			com[1].txptr = NULL;
		}
		SCON1 &= 0xFD;						// clear transmit interrupt flag
	} // if
} // UBX1_ISR
#endif // else
#endif // SENSOR

// Timer 3 Interrupt
//
// When T3 expires, start a series of ADC conversions and flash LED as required
void T3_ISR (void) interrupt 14 {
	u16 i;

	SFRPAGE = TMR3_PAGE;

	// clear Timer 3 interrupt
	TMR3CN &= 0x7F;

	// increment a clock tick
	clocktick++;

	// keep track of seconds
	if (clocktick == nextsecond) {
		nextsecond += SAMPLE_RATE;
		secondtick++;
	} // if

	// count down timeouts for serial communication
	for (i = 0; i<NUMUNITS; i++)
		if(CTS[i]) CTS[i]--;

#ifdef DISPLAY
	for (i = 0; i<NUMBER_OF_BUTTONS; i++) {
		SFRPAGE = CONFIG_PAGE;
		// if switch pressed, count up
		if (!(P5 & (1<<i)) && (debounce[i] < 0xFFFF))
			debounce[i]++;
	} // for
#endif // DISPLAY

#ifdef SENSOR
	// if switch pressed, count up
	SFRPAGE = CONFIG_PAGE;
	if (0 == SW1 && debounce < 0xFFFF) debounce++;

	SFRPAGE = ADC0_PAGE;
	// set mux back to first sample
	AMX0SL = 0x00;

	// clear conversion complete flag
	AD0INT = 0;

	// start ADC conversion
	AD0BUSY = 1;

#endif // SENSOR
} // T3_ISR
