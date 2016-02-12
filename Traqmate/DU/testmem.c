// 
typedef unsigned char u08;
typedef unsigned long u32;

#define DU_MAIN

#define ASCIIBAUD		38400L			// Baud rate of UART0 in bps
#define MSGBAUD			115200L			// Baud rate of UART0 in bps
#define INTCLK			24500000		// Internal oscillator frequency in Hz
#define SYSCLK			98000000		// Output of PLL derived from (INTCLK*2)

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
sfr16 DAC0     = 0xd2;                 // DAC0 data
sfr16 DAC1     = 0xd2;                 // DAC1 data
sfr16 PCA0CP5  = 0xe1;                 // PCA0 Module 5 capture
sfr16 PCA0CP2  = 0xe9;                 // PCA0 Module 2 capture
sfr16 PCA0CP3  = 0xeb;                 // PCA0 Module 3 capture
sfr16 PCA0CP4  = 0xed;                 // PCA0 Module 4 capture
sfr16 PCA0     = 0xf9;                 // PCA0 counter
sfr16 PCA0CP0  = 0xfb;                 // PCA0 Module 0 capture
sfr16 PCA0CP1  = 0xfd;                 // PCA0 Module 1 capture

#include <stdlib.h>
#include <c8051f120.h>

void SYSCLK_Init (void);
void PORT_Init_DU(void);
// void IE6_Init (void);
void UART0_Init (u32 );
// BOOL ComPort(comporthandle *, comaction );
// void Timer3_Init (u16);
// void Timer3_ISR (void);


void main (	void ) {

	unsigned char *testptr;
	xdata unsigned char bigmem[1000];
	int i;
	unsigned char j;

	SFRPAGE = CONFIG_PAGE;

	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init_DU();						// enable ports

	UART0_Init(MSGBAUD);

	EA = 1;								// enable global interrupts

	for (i=0; i < 1000; i++)
		bigmem[i] = i & 0xFF;

	while (1) {

		for (i=0; i<1000; i++) {

			j = bigmem[i];
			if ((i & 0xFF) != j) {
				i++;
			}
			testptr = bigmem + i;
			j = *testptr;
			if ((i & 0xFF) != j) {
				i++;
			}
		}
#if 0
//		check for incoming sensor unit message
		if (NULL != com[0].rxptr) {

			// process the message
			retval = Parse_Message((u08) 0, com[0].rxptr);

			// clear the semaphore to signal buffer free
			com[0].rxptr = NULL;

			// Turn on/off GPS Lock Icon
			if (dispstate == TIMING || dispstate == LAPS)
				Write_Icon(0, 0, 10, 9, &(sigstrength[(gpslock+2)/3][0][0]), '-');
		} // if
#endif

	} // while
} // main

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// This routine initializes the system clock to use the internal oscillator
// at 24.5 MHz multiplied by two using the PLL.
//
void SYSCLK_Init (void)
{
	int i;									// software timer
	u08 SFRPAGE_SAVE = SFRPAGE;				// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;					// set SFR page
	SFRPGCN = 0x01;							// turn on auto SFR paging for interrupts

	OSCICN = 0x83;							// 10000011 set internal oscillator to run at max frequency
	CLKSEL = 0x00;							// Select the internal osc. as the SYSCLK source

	PLL0CN= 0x00;							// Set internal osc. as PLL source

	SFRPAGE = LEGACY_PAGE;
	FLSCL	= 0x50;							// Set FLASH read time for 100MHz clk

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

//------------------------------------------------------------------------------------
// PORT_Init_DU
//------------------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports
//
void PORT_Init_DU (void) {

	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;			// set SFR page

	XBR0 = 0xFF;					// all functions available at port pins
	XBR1 = 0xFF;					// ditto
	XBR2 = 0xC4;					// 11000100 all functions, no pullups, xbar enable, t4 off,
									// Uart1 Enable, Ext Mem Disable, Ext Convert off

	P0MDOUT = 0x15;					// 00010101 enable TX0, SI, SCK, as push-pulls
	P0 = 0xEA;						// set all open drain pins to 1

	P1MDIN	= 0x03;					// 00000011 set all analog inputs except TX1, RX1
	P1MDOUT = 0x01;					// 00000001 enable tx1 as push-pull

	P2MDOUT = 0x00;					// all pins open drain
	P2 = 0xFF;						// write a 1 to each bit

	P3MDOUT = 0x00;					// all pins open drain 
	P3 = 0xFF;						// write a 1 to each bit

	P7MDOUT = 0xFF;
	P7 = 0x86;						// 00000111 all backlights off, force disp on, dataflash wp on, 
									//			dataflash cs off, dataflash reset off
	P6MDOUT = 0x00;
	P6 = 0xFF;						// 11110111

	P5MDOUT = 0x00;
	P5 = 0xFF;						// 11111111 all inputs

	P4MDOUT = 0x00;					// 4 all pushpull
	P4 = 0xFF;						// 11111111

	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page
}

void UART0_Init (u32 baud)
{
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	SFRPAGE = UART0_PAGE;
	SSTA0 = 0x15;						// 00010101 no doubler, user timer 2

	SCON0 = 0x50;				// SCON00: mode 1, 8-bit UART, enable RX

	SFRPAGE = TMR2_PAGE;
	TMR2CN = 0x04;						// 00000100 no ext trig, enabled, timer, auto reload
	TMR2CF = 0x08;						// 00001000 sysclk source, count up

	RCAP2 = -(SYSCLK/baud/16);			// set timer 2 reload value
	TMR2 = 0xFFFF;						// set to reload immediately
	TR2 = 1;							// start timer

	SFRPAGE = UART0_PAGE;
	RI0 = TI0 = 0;						// Clear interrupt flags
	ES0 = 1;							// Enable UART0 interrupts


	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page

} // UART0_Init

#if 0
//-----------------------------------------------------------------------------
// Timer3_Init
//-----------------------------------------------------------------------------
//
// Configure Timer3 to auto-reload at interval specified by <counts> and generate
// an interrupt which will start a sample sycle. Uses SYSCLK as its time base.
//
void Timer3_Init (u16 counts)
{
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
#endif
//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

// UART0 ISR
void UART0_ISR (void) interrupt 4 {
	u08 tempch;

	// determine if tx or rx or both caused interrupt

	if (SCON0 & 0x01) {			// byte received
		tempch = SBUF0;
		SCON0 &= ~0x01;			// clear receive interrupt flag

		SBUF0 = tempch;			// echo it out
	} // if
	SCON0 &= ~0x02;				// clear transmit interrupt flag

}
