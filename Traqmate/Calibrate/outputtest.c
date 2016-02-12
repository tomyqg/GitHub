#define EXTERNALOSC
// This is the main program for outputtest.c
// This program will test the digital outputs
// It outputs a walking one on the digital output pins.
//
// This version runs on the second gen SU with Cygnal 132 microprocessor
//

#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c8051f120.h>

typedef char BOOL;

#define TRUE 1
#define FALSE 0

sfr16 DP       = 0x82;                 // data pointer
sfr16 ADC0     = 0xbe;                 // ADC0 data
sfr16 ADC0GT   = 0xc4;                 // ADC0 greater than window
sfr16 ADC0LT   = 0xc6;                 // ADC0 less than window
sfr16 RCAP2    = 0xca;                 // Timer2 capture/reload
sfr16 RCAP3    = 0xca;                 // Timer3 capture/reload
sfr16 RCAP4    = 0xca;                 // Timer4 capture/reload
sfr16 TMR2     = 0xcc;                 // Timer2
sfr16 TMR3     = 0xcc;                 // Timer3
sfr16 TMR4     = 0xcc;                 // Timer4
sfr16 PCA0CP5  = 0xe1;                 // PCA0 Module 5 capture
sfr16 PCA0CP2  = 0xe9;                 // PCA0 Module 2 capture
sfr16 PCA0CP3  = 0xeb;                 // PCA0 Module 3 capture
sfr16 PCA0CP4  = 0xed;                 // PCA0 Module 4 capture
sfr16 PCA0     = 0xf9;                 // PCA0 counter
sfr16 PCA0CP0  = 0xfb;                 // PCA0 Module 0 capture
sfr16 PCA0CP1  = 0xfd;                 // PCA0 Module 1 capture

#define ASCIIBAUD		38400L			// Baud rate of UART0 in bps
#define INTCLK			24500000L		// Internal oscillator frequency in Hz
#define EXTCLK			24576000L
#ifdef EXTERNALOSC
#define SYSCLK			(4L*EXTCLK)		// Output of PLL
#else
#define SYSCLK			(4L*INTCLK)		// Output of PLL
#endif

#define SAMPLE_RATE	160					// THIS AFFECTS COMM TIMEOUT!!
										// ADC record rate in hz (must be 10x INT_DEC)
#define tohex(c) (c<10? (c+'0'):(c+'A'-10))
#define MIN(q,r) ((q)<(r)? (q):(r))
#define MAX(q,r) ((q)>(r)? (q):(r))
#define AVG(q,r) (((q)+(r))/2)
#define ABS(q) ((q)>0? q:-(q))
#define WAIT(CYCLES) do {int z=CYCLES; do {z--;} while (z > 0);} while(0)
/* universal data elements */
typedef unsigned char u08;
typedef unsigned int u16;
typedef unsigned long u32;
typedef char s08;
typedef int s16;
typedef long s32;

// function prototypes
void SYSCLK_Init (void);
void PORT_Init (void);
void ADC0_Init (void);
void UART0_Init (void);
void Timer3_Init (u16 counts);

// Global Variables
u08 t3_semaphore = FALSE;

void main (	void ) {
	u08 mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	int bitnum = 0;
	int counter = SAMPLE_RATE;

	SFRPAGE = CONFIG_PAGE;
	
	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init();						// enable ports
	UART0_Init();						// enable UART to write to

	Timer3_Init ((u16) (SYSCLK/SAMPLE_RATE/12));	// initialize Timer3 to overflow at sample rate

	EA = 1;								// enable global interrupts

	// initialize
	SFRPAGE = UART0_PAGE;
	printf( "\r\nTraqMate 132 SU, Output Test 03-15-2005\r\n");
	SFRPAGE = CONFIG_PAGE;

	while (1) {
		if (t3_semaphore) {
			t3_semaphore = FALSE;

			// change bit every 3 seconds
			if (counter == (SAMPLE_RATE*3)) {
				counter = 0;

				SFRPAGE = UART0_PAGE;
				printf("bit %d\r\n", bitnum);
				SFRPAGE = CONFIG_PAGE;

				P4 = mask[bitnum];
				bitnum = ++bitnum % 7;

			} // if
			else
				counter++;
		} // if
	} // while

} // main

void SYSCLK_Init (void) {
#ifdef EXTERNALOSC
	int i;									// software timer
	u08 SFRPAGE_SAVE = SFRPAGE;				// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;					// set SFR page
	SFRPGCN = 0x01;							// turn on auto SFR paging for interrupts

	OSCXCN = 0x67;                  	    // 01100111 start external oscillator with
                                 	    	// 22.1184MHz crystal

	for (i=0; i < 256; i++) ;           // XTLVLD blanking interval (>1ms)
	
	while (!(OSCXCN & 0x80)) ;          // Wait for crystal osc. to settle

	CLKSEL = 0x01;							// Select the external osc. as the SYSCLK source

	OSCICN = 0x00;							// 00000000 turn off internal osc
	PLL0CN = 0x05;							// external osc src, powered

	SFRPAGE = LEGACY_PAGE;
	FLSCL	= 0x30;							// Set FLASH read time for 100MHz clk

	SFRPAGE = CONFIG_PAGE;
	PLL0DIV = 0x01;							// Set Pre-divide value to N (N = 1)
	PLL0FLT = 0x01;							// Set the PLL filter register for
											// a reference clock from 19 - 30 MHz
											// and an output clock from 65 - 100 MHz
	PLL0MUL = 0x04;							// Multiply SYSCLK by M (M = 4)

	for (i=0; i < 256; i++) ;				// Wait at least 5us
	PLL0CN |= 0x02;							// Enable the PLL
	while(!(PLL0CN & 0x10));				// Wait until PLL frequency is locked
	CLKSEL= 0x02;							// Select PLL as SYSCLK source

	SFRPAGE = SFRPAGE_SAVE;					// Restore SFR page
#else
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
	PLL0FLT = 0x01;							// Set the PLL filter register for
											// a reference clock from 19 - 30 MHz
											// and an output clock from 45 - 80 MHz
	PLL0MUL = 0x04;							// Multiply SYSCLK by M (M = 4)

	for (i=0; i < 256; i++) ;				// Wait at least 5us
	PLL0CN |= 0x02;							// Enable the PLL
	while(!(PLL0CN & 0x10));				// Wait until PLL frequency is locked
	CLKSEL= 0x02;							// Select PLL as SYSCLK source

	SFRPAGE = SFRPAGE_SAVE;					// Restore SFR page
#endif
}

void PORT_Init (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;				// set SFR page

	XBR0	= 0xEF;						// 11101111 Enable all but CEX5
	XBR1    = 0x07;						// Turn on INT0, T0, CP1
	XBR2    = 0xC4;                     // Enable crossbar, no weak pull-ups, UART1

	P0MDOUT = 0x15;						// 00010101 enable TX0, SI, SCK, as push-pulls
	P0 = 0xEA;							// set all open drain pins to 1

	P1MDIN	= 0xFF;						// set all digital i/o points (no analog)
	P1MDOUT = 0x0D;						// 00001101 enable tx1, pwmx,y as push-pull
	P1 = 0x01;							// set rx1 to impedance

	P2MDOUT = 0x00;						// all pins open drain
	P2 = 0xFF;							// high impedance

	P3MDOUT = 0xF8;						// 11111000 upper 5 bits push-pull
	P3 = 0x7F;							// lower 3 bits high impedance, LEDs on

	P4MDOUT = 0xFF;						// all pins push-pull
	P4 = 0x00;							// initialize to one (inverting)

	P5MDOUT = 0x00;						// all pins open drain
	P5 = 0xFF;							// high impedance

	P6MDOUT = 0x00;						// all pins open drain
	P6 = 0xFF;							// high impedance

	P7MDOUT = 0xFF;						// all pins push-pull
	P7 = 0xF7;							// 11110111 gps reset low

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}

//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//
// Configure the UART0 using Timer2, for <baudrate> and 8-N-1.
//
void UART0_Init (void) {
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

	SFRPAGE = UART0_PAGE;
	SSTA0 = 0x15;						// 00010101 no doubler, use timer 2

	SCON0 = 0x50;						// SCON0: mode 1, 8-bit UART, enable RX

	SFRPAGE = TMR2_PAGE;
	TMR2CN = 0x04;						// 00000100 no ext trig, enabled, timer, auto reload
	TMR2CF = 0x08;						// 00001000 sysclk source, count up

	RCAP2 = -(SYSCLK/ASCIIBAUD/16);		// set timer 2 reload value
	TMR2 = 0xFFFF;
	TR2 = 1;							// start timer 2

	SFRPAGE = UART0_PAGE;
	TI0 = 1;                            // Indicate TX0 ready

	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}
//-----------------------------------------------------------------------------
// Timer3_Init
//-----------------------------------------------------------------------------
//
// Configure Timer3 to auto-reload at interval specified by <counts> and generate
// an interrupt which will start a sample sycle. Uses SYSCLK as its time base.
//
void Timer3_Init (u16 counts) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = TMR3_PAGE;

	TMR3CN = 0x00;						// Stop Timer3; Clear TF3;
	TMR3CF = 0x01;						// use SYSCLK/12 as timebase, count up

	RCAP3 = - (int) counts;				// Init reload values

	TMR3 = 0xFFFF;						// set to reload immediately
	EIE2 |= 0x01;						// enable Timer3 interrupts
	TMR3CN |= 0x04;						// start Timer3

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}

// Timer 3 Interrupt
//
// When T3 expires, set a semaphore
void T3_ISR (void) interrupt 14 {
	SFRPAGE = TMR3_PAGE;

	// clear Timer 3 interrupt
	TMR3CN &= 0x7F;

	t3_semaphore = TRUE;
}