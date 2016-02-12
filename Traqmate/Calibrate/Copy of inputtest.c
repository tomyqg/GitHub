#undef EXTERNALOSC
// This is the main program for inputtest.c
// This program will test the digital and analog inputs
// It prints the status of digital inputs and analog inputs.
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
#define EXTCLK			22118400L
#ifdef EXTERNALOSC
#define SYSCLK			(4L*EXTCLK)		// Output of PLL
#else
#define SYSCLK			(4L*INTCLK)		// Output of PLL
#endif

#define SAMPLE_RATE	160					// THIS AFFECTS COMM TIMEOUT!!
										// ADC record rate in hz (must be 10x INT_DEC)
#define INT_DEC		16					// integrate and decimate ratio (16x oversample)
#define NUM_CHAN	8					// x, y, 5V, temp, ai0, ai1, ai2, ai3
#define ADC_RATE	2500000				// ADC conversion clock

#define PWMHI		255					// highest PWM calibration value
#define PWMLO		0					// lowest PWM calibration value
#define STARTZERO	(PWMHI-PWMLO)/2		// middle of road value to start with

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
xdata u16 result[] = {0, 0, 0, 0, 0, 0, 0, 0};		// array to hold ADC samples    
xdata u08 adcresults = FALSE;		// TRUE when samples collected

void main (	void ) {
	u08 mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	int bitnum = 0;
	int i;

	SFRPAGE = CONFIG_PAGE;
	
	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init();						// enable ports
	UART0_Init();						// enable UART to write to

	Timer3_Init ((u16) (SYSCLK/SAMPLE_RATE/12));	// initialize Timer3 to overflow at sample rate
	ADC0_Init();                        // init ADC

	EA = 1;								// enable global interrupts

	// initialize
	SFRPAGE = UART0_PAGE;
	printf( "\r\nTraqMate 132 SU, Input Test 03-15-2005\r\n");
	SFRPAGE = CONFIG_PAGE;

	while (1) {
		u08 port5;

		if (adcresults) {
			adcresults = FALSE;

			SFRPAGE = CONFIG_PAGE;

			port5 = P5;

			SFRPAGE = UART0_PAGE;

			printf( "DIGIN: ", port5);
			for (i = 2; i<8; i++)
				printf((port5 & mask[i])? "1" : "0");
			printf( "\r\n");

			printf( "X, Y, 5V, T: %u,%u,%u,%u\r\n",	result[0], result[1], result[2], result[3]);
			printf( "A0, A1, A2, A3: %u,%u,%u,%u\r\n",	result[4], result[5], result[6], result[7]);
			SFRPAGE = CONFIG_PAGE;
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

void ADC0_Init (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	// configure the PCA for PWM operation
	SFRPAGE = PCA0_PAGE;
	PCA0MD =	0x80;					// 1000000 suspend with microp, SYSCLK / 12
	PCA0CN =	0x40;					// 01000000 PCA0 enabled
	PCA0CPM0 =	0x42;					// CCM0 in 8-bit PWM mode
	PCA0CPM1 =	0x42;					// CCM1 in 8-bit PWM mode

	PCA0CPL0 =	STARTZERO;		// initialize PCA PWM value
	PCA0CPH0 =	STARTZERO;
	PCA0CPL1 =	STARTZERO;		// initialize PCA PWM value			
	PCA0CPH1 =	STARTZERO;

	// set up the ADC
	SFRPAGE = ADC0_PAGE;
	ADC0CN = 0xC0;						// 11000001 ADC0 enabled; special tracking
										// ADC0 conversions are initiated 
										// on AD0BUSY=1; ADC0 data is right-justified

	REF0CN = 0x07;                      // enable temp sensor, on-chip VREF,
   										// and VREF output buffer
	AMX0CF = 0x00;						// all non-differential inputs, no gain
	AMX0SL = 0x00;                      // Select AIN0 external input on mux
	ADC0CF = ((SYSCLK/2/ADC_RATE - 1) << 3) | 0x00;	// ** ADC conv clock = 2.5MHz, Gain = 1

	EIE2 |= 0x02;						// enable ADC interrupts

	SFRPAGE = SFRPAGE_SAVE;
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
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ADC0_ISR
//-----------------------------------------------------------------------------
//
// ADC0 end-of-conversion ISR 
// Here we take the ADC0 sample, add it to a running total <accumulator>, and
// decrement our local decimation counter <int_dec>.  When <int_dec> reaches
// zero, we post the decimated result in the global variable <result>.
//
void ADC0_ISR (void) interrupt 15
{
	static u08 channel = 0;				// which ADC channel are we sampling
	static u16 int_dec=INT_DEC*NUM_CHAN;    // integrate/decimate counter
                                       // post results when int_dec = 0
	static xdata u16 accumulator[NUM_CHAN] = { 0 }; // ** here's where we integrate ADC samples             
	int i;

	// don't do anything unless the last set was used
	if (!adcresults) {

		SFRPAGE = ADC0_PAGE;

		AD0INT = 0;							// clear ADC conversion complete indicator

		accumulator[channel] += ADC0;		// read ADC value and add to running total

		// reprogram mux to select next channel
		switch (channel) {
			case 0:															
				AMX0SL = 0x01;					// AIN0 moving to AIN1
				channel = 1;
				break;
			case 1:								// AIN1 moving to AIN2 (5V)
				AMX0SL = 0x02;
				channel = 2;
				break;
			case 2:								// AIN2 moving to TEMP
				AMX0SL = 0x08;
				channel = 3;
				break;
			case 3:															
				AMX0SL = 0x03;					// TEMP moving to AIN3
				channel = 4;
				break;
			case 4:								// AIN3 moving to AIN4
				AMX0SL = 0x04;
				channel = 5;
				break;
			case 5:								// AIN4 moving to AIN5
				AMX0SL = 0x05;
				channel = 6;
				break;
			case 6:															
				AMX0SL = 0x06;					// AIN5 moving to AIN6
				channel = 7;
				break;
			case 7:															
				AMX0SL = 0x00;					// AIN6 moving to AIN0
				channel = 0;
				break;
		}

		// if all channels sampled and all samples taken then post results
		if (0 == --int_dec) {

			for (i=0; i<NUM_CHAN; i++) {
				result[i] = accumulator[i] >> 4;		// ** note value is x16
				accumulator[i] = 0;				// reset accumulator
			}
			int_dec = INT_DEC*NUM_CHAN;		// reset counter
			adcresults = TRUE;				// set semaphore
		}
		if (!(0 == channel)) {
			AD0BUSY = 1;	
		}
	} // if
}

// Timer 3 Interrupt
//
// When T3 expires, start a series of ADC conversions and flash LED as required
void T3_ISR (void) interrupt 14 {
	SFRPAGE = TMR3_PAGE;

	// clear Timer 3 interrupt
	TMR3CN &= 0x7F;

	SFRPAGE = ADC0_PAGE;
	// added 3/30/04 set mux back to first sample
	AMX0SL = 0x00;

	AD0INT = 0;
	// start ADC conversion
	AD0BUSY = 1;
}