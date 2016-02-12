// Part of sensorunit.c
// 11/10/2003
// 12/21/2004 converted to x132 microprocessor
// 11/24/2005 added data i/o stuff
// Author: GAStephens
//
// This file contains all the functions to setup the Cygnal 8051 for use in the 
// traqmate program including the system clock, SPI port, UARTs, timers, and
// crossbar switch.

#include <sensor.h>

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

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
//		enables the SMB bus
//		enables the crossbar itself and sets weak pullups
//		sets GPS_PPS signal ports to high impedance
//		sets ACC_ST, 3.3V_GPS_ENABLE, 3.3V_IF_ENABLE signal ports to push-pull
//		sets 

//
void PORT_Init_SU (void) {
//	u16 i;
	
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	SFRPAGE = CONFIG_PAGE;				// set SFR page

	XBR0	= 0xEF;						// 11101111 Enable all but CEX5
	XBR1    = 0x07;						// 00000111 Turn on INT0, T0, CP1
	XBR2    = 0x44;                     // 01000100 Enable crossbar, weak pull-ups, UART1

	P0MDOUT = 0x15;						// 00010101 enable TX0, SI, SCK, as push-pulls
	P0 = 0xEA;							// 11101010 set all open drain pins to 1

	P1MDIN	= 0xFF;						// set all digital i/o points (no analog)
	P1MDOUT = 0x3D;						// 00111101 enable cex2, cex3, tx1, pwmx,y as push-pull
	P1 = 0x01;							// set rx1 to high impedance

	P2MDOUT = 0x00;						// all pins open drain
	P2 = 0xFF;							// high impedance

	P6MDOUT = 0x00;						// all pins open drain
	P6 = 0xFF;							// high impedance
	// test for SU2.0
	su2 = (SU2MASK == (P6 & 0xE0));		// test for SU2.0

	if (su2) {
		P3 = 0x79;						// 01111001 P3.6 - P3.3 LEDs on, P3.2 - P3.1 off, P3.0 = 1	
		P3MDOUT = 0xFE;					// 11111110 upper 5 bits push-pull, P3.2 - P3.1 AutoOn Pushpull, P3.0 Open Drain
	} // if 
	else {
		P3 = 0x7E;						// lower 3 bits high impedance, LEDs on, P3.0 = 1
		P3MDOUT = 0xF8;					// 11111000 upper 5 bits push-pull
	} // else

	// Digital Outs	- write port first so no glitch on ports
	P4 = 0x00;							// set to low
	P4MDOUT = 0xFF;						// all pins push-pull

	// Digital Ins
	P5MDOUT = 0x00;						// all pins open drain
	P5 = 0xFF;							// 11111111

	if (su2) {							// set port first to eliminate glitches on 3VEn
		P7 = 0x97;						// 10011111 5VEn=1, n/c, gselect=0, 3VGPS=1, /I2Cpu=0, dataflash=111
		P7MDOUT = 0xF7;					// all pins push-pull except P7.3 = /I2Cpu
	} // if
	else {
		P7MDOUT = 0xFF;					// all pins push-pull
		P7 = 0xF7;						// 11110111 5VEn=1, n/c, 3VIFEN=1, 3VGPS=1, GPSRESET=0, dataflash=111
	} // else

	// SU2 version 2
	if (su2) {
		EN3VGPS = 0;
		ENSMBPU = 1;					// disable I2C pullup
	} // if
	else
		EN3VGPS = 1;

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
} // Port_Init_SU

// Initializes the counters for use with Frequency 0 input (RPM)
//
void Freq0_Init (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page

	// configure the PCA to generate timing signal for RPM measurement
	SFRPAGE = PCA0_PAGE;
	PCA0CN =	0x00;					// 01000000 PCA0 disabled
	PCA0MD =	0x80;					// 1000000 suspend with microp, SYSCLK / 12
	PCA0L = 0x00;						// clear the counter
	PCA0H = 0x00;
	PCA0CPL2 = 0;						// clear lower byte of register
	PCA0CPH2 = (SYSCLK/12/FREQ_SAMPLE_RATE/2);	// magic number to get widest RPM range	= 256000 freq0ticks/sec = 12 for 75Mhz
	PCA0CPM2 = 0x46;					// 01000110 Frequency Output Mode
	PCA0CN = 0x40;						// 01000000 PCA0 enabled

	// clear global period counterss
	period0 = negperiod0 = 0;

	// configure T0 to count pulses from PCA, generate interrupt on overflow
	SFRPAGE = TIMER01_PAGE;
	TR0 = 0;							// disable Timer 0
	TMOD = (TMOD & 0xFF00) | 0x05;		// xxxx0101, not gated, counter, 16 bit
	CKCON = CKCON | 0x08;				// xxxx1xxx, use system clock	
	TL0 = 0x00;							// clear the counter/timer
	TH0 = 0x00;
	TF0 = 0;							// clear T0 overflow flag
	TR0 = 1;							// enable Timer 0

	// configure Comparator 0
	SFRPAGE = CPT0_PAGE;
	CPT0CN = 0x80;						// 10000000, CPT0 enabled, no hysteresis
//	CPT0CN = 0x8F;						// 11001111, CPT0 enabled, compare CPT0+ > CPT0-, +,- hysteresis
//	CPT0MD = 0x23;						// 00100011 rising edge, slow response time
	CPT0MD = (RISING_EDGE | FALLING_EDGE);		// 00110000 rising and falling edge, fast response time

	// enable interrupts
//	EIP1 &= ~0x20;						// set comparator input low priority
	ET0 = 1;							// enable T0 interrupt
	EIE1 |= (RISING_EDGE | FALLING_EDGE);	// xx11xxxx enable CPT0 rising and falling edge interrupts

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page

} // Freq0_Init

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
	EX0 = 1;							// enable INT0

	SFRPAGE = SFRPAGE_SAVE;				// Restore SFR page
}

// interrupt for Frequency 0 Input (RPM), rising edge on Comparator 0
void Freq0_Rising_ISR (void) interrupt 11 {
	u16 temp;
	static u08 freqbufferindex = 0xFF;
	static u32 freqtotal;

	EIE1 &= ~(RISING_EDGE | FALLING_EDGE);		// disable CPT0 rising and falling edge interrupts

	SFRPAGE = TIMER01_PAGE;
	temp = (((u16) TH0) << 8) + TL0;	// save the timer count in freq0ticks

	// ignore if not enough time has passed since negative-going pulse
	if ((temp - negperiod0) > PULSE_KEEPOUT) {

		TR0 = 0;							// stop Timer0
		TL0 = 0x00;							// clear the counter/timer
		TH0 = 0x00;
		TF0 = 0;							// clear T0 overflow flag
		TR0 = 1;							// enable Timer 0
		if (0xFF == freqbufferindex) { 		// first time through
			u08 i;
			freqbufferindex = 0;			// initialize index
			for	(i=0; i<RPM_AVERAGING; i++)	// prime the buffer
				freqtotal += (periodbuff[i] = temp);
			period0 = temp;					// publish the value
		} // if
		else {
			if (++freqbufferindex >= RPM_AVERAGING) freqbufferindex = 0; // roll index back around
			freqtotal -= periodbuff[freqbufferindex];			// subtract off oldest value
			freqtotal += (periodbuff[freqbufferindex] = temp);	// add in new value
			temp = freqtotal / RPM_AVERAGING;					// get the average
			if (ABS(period0 - temp) >= (period0 >> 9))			// suppress small changes
				period0 = temp;			
		} // else		 

		negperiod0 = 0;						// reset negative-going pulse
	} // if	

	SFRPAGE = CPT0_PAGE;
	CPT0CN &= 0xCF;						// 11001111 clear rising & falling edge flags
										// 11001111, CPT0 enabled, compare CPT0+ > CPT0-, +,- hysteresis
	EIE1 |= (RISING_EDGE | FALLING_EDGE);		// enable CPT0 rising and falling edge interrupts
}

// interrupt for Frequency 0 Input (RPM) falling edge on Comparator 0
void Freq0_Falling_ISR (void) interrupt 10 {

	EIE1 &= ~(RISING_EDGE | FALLING_EDGE);		// disable CPT0 rising and falling edge interrupts

	SFRPAGE = TIMER01_PAGE;
	negperiod0 = (((u16) TH0) << 8) + TL0;		// save the position of the negative-going pulse

	SFRPAGE = CPT0_PAGE;
	CPT0CN &= 0xCF;						// 11001111 clear rising & falling edge flags
										// 11001111, CPT0 enabled, compare CPT0+ > CPT0-, +,- hysteresis
	EIE1 |= (RISING_EDGE | FALLING_EDGE);		// enable CPT0 rising and falling edge interrupts
}

// interrupt for Timer 0 Overflow = Frequency 0 Input (RPM) is not working
void T0_ISR (void) interrupt 1 {

	EIE1 &= ~(RISING_EDGE | FALLING_EDGE);		// disable CPT0 rising edge interrupt

	SFRPAGE = TIMER01_PAGE;
	TR0 = 0;							// stop Timer0
	TL0 = 0x00;							// clear the counter/timer
	TH0 = 0x00;
	TR0 = 1;							// enable Timer 0

//  may want to eliminate this to cover up falsing
	period0 = negperiod0 = 0;			// didn't get ignition pulse so RPM = 0

	SFRPAGE = CPT0_PAGE;
	CPT0CN &= 0xCF;						// clear rising edge flag
										// 11001111, CPT0 enabled, compare CPT0+ > CPT0-, +,- hysteresis

	EIE1 |= (RISING_EDGE | FALLING_EDGE);						// enable CPT0 rising edge interrupt
}

// interrupt for pulse per second sync signal from GPS
void GPSPPS_ISR (void) interrupt 0 {
	extern u16 int_dec;
	extern xdata u16 accumulator[NUM_CHAN];
	extern u08 channel;					// which ADC channel are we sampling
	u08 i;
	u08 EASAVE = EA;

	gpspulse = TRUE;					// set semaphore
	int_dec = SAMPLE_RATE * NUM_CHAN / samps_per_sec;

	channel = 0;
	for (i=0; i<NUM_CHAN; i++)
		accumulator[i] = 0;				// reset adc accumulator

	EA = 0;								// pause interrupts
	EIE2 &= ~0x01;						// stop Timer3 interrupts

	SFRPAGE = ADC0_PAGE;
	AD0BUSY = 0;						// turn off adc interrupts
	AD0INT = 0;							// clear ADC conversion complete indicator
	AMX0SL = 0x00;						// set mux back to first sample

	SFRPAGE = TMR3_PAGE;
	TMR3CN = 0x00;						// Stop Timer3; Clear TF3;
	TMR3 = 0xFFFF;						// set to reload real soon
	EIE2 |= 0x01;						// enable Timer3 interrupts
	TMR3CN |= 0x04;						// start Timer3
	
	EA = EASAVE;
}