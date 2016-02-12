#include <c8051f120.h>

/* universal data elements */
typedef unsigned char u08;
typedef unsigned int u16;
typedef unsigned long u32;
typedef char s08;
typedef int s16;
typedef long s32;
#define TRUE 1
#define FALSE 0

#define HI(x) ((P##x) |= (1<<(B##x)))
#define LO(x) ((P##x) &= ~(1<<(B##x)))
#define XOR(x) ((P##x) ^= (1<<(B##x)))

#define PLED1	P5
#define BLED1	4
#define PLED2	P5
#define BLED2	3

void PPS_Init (void);
void SYSCLK_Init (void);
void PORT_Init_SU (void);

// globals
xdata u08 gpspulse;			// TRUE when GPS PPS signal received

void main (	void ) {
	int j, k;


	SFRPAGE = CONFIG_PAGE;

	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init_SU();						// enable ports

	PPS_Init();							// start GPS pulse interrupts

	EA = 1;								// enable global interrupts

	LO(LED1);
	LO(LED2);

	while (1) {
		k = P2;
		if (0x08 & k)
			XOR(LED2);


//		if (gpspulse) {
//			gpspulse = FALSE;
//			XOR(LED2);
//		} // if
	} // while
} // main

//-----------------------------------------------------------------------------
//
// This routine initializes the system clock to use the internal oscillator
// at 24.5 MHz multiplied by four using the PLL to get 98 MHz
//
void SYSCLK_Init (void) {
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
}
//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports
//
// this function does the following:
//		enables uarts 0,1 and sets their tx lines as push-pull
//		enables the SPI ports and sets SO and SCK as push-pull
//		configures digital inputs as inputs
//		enables the SMB bus (unused at this time)
//		enables the crossbar itself and sets weak pullups
//		sets GPS_PPS signal ports to high impedance
//		sets ACC_ST, 3.3V_GPS_ENABLE, 3.3V_IF_ENABLE signal ports to push-pull
//		sets 

//
void PORT_Init_SU (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;				// set SFR page

	XBR0	= 0xEF;						// 11101111 Enable all but CEX5
	XBR1    = 0x07;						// Turn on INT0, T0, CP1
	XBR2    = 0x44;                     // Enable crossbar, weak pull-ups, UART1

//	XBR0	= 0x07;						// 11101111 Enable all but CEX5
//	XBR1    = 0x04;						// Turn on all items
//	XBR2    = 0x44;                     // Enable crossbar, weak pull-ups, UART1

//	SFRPAGE = EMI0_PAGE;
//	EMI0CF	= 0x00;						// turn off external memory interface

//	SFRPAGE = CONFIG_PAGE;				// set SFR page

	P0MDOUT = 0x15;						// 00010101 enable TX0, SI, SCK, as push-pulls
	P0 = 0xEA;							// set all open drain pins to 1

//	P0MDOUT = 0x00;						// 00010101 enable TX0, SI, SCK, as push-pulls
//	P0 = 0xFF;							// set all open drain pins to 1

//	P1MDIN	= 0xFF;						// 00000011 set all analog inputs except TX1, RX1
//	P1MDOUT = 0x00;
//	P1 = 0xFF;

	P1MDIN	= 0xFF;						// 00000011 set all analog inputs except TX1, RX1
	P1MDOUT = 0x01;						// 00000001 enable tx1 as push-pull

	P2MDOUT = 0x00;						// all pins open drain

	P3MDOUT = 0x00;						// all pins open drain

	P2 = P3 = 0xFF;						// set to open

	P4MDOUT = 0x00;						// all pins open drain
	P4 = 0xFF;							// 11111111

	P5MDOUT = 0xFF;						// all pins push-pull
	P5 = 0x61;							// 01100001

	P6MDOUT = 0x00;						// all pins open drain
	P6 = 0xFF;							// 11110111

	P7MDOUT = 0xFF;						// all pins push-pull
	P7 = 0xFE;							// 11111110

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}

//
// PPS_Init
//
// Setup external interrupt for GPS Pulse Per Second
//
void PPS_Init (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = LEGACY_PAGE;				// set SFR page

	IE0 = 0;							// clear pending flag
	IT0 = 1;							// INT0 edge triggered
//	IT0 = 0;							// INT0 level triggered
	TMOD = 0;							// turn off INT0 timer functions
//	PX0 = 1;							// INT0 high priority
	EX0 = 1;							// enable INT0

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}


// interrupt for pulse per second sync signal from GPS
void GPSPPS_ISR (void) interrupt 0 {
static unsigned int count = 0;

	SFRPAGE = CONFIG_PAGE;

	count++;
	HI(LED1);
	gpspulse = TRUE;				// set semaphore
}

