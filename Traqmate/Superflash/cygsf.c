// Part of traqmate.c
// 11/10/2003
// Author: GAStephens
//
// This file contains all the common functions to setup the Cygnal 8051 for use in the 
// traqmate program including the system clock, SPI port, UARTs, timers, and
// crossbar switch. 

#ifdef DISPLAY
#include <display.h>
#endif
#include <stdlib.h>


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


