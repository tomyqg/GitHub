// Part of traqmate.c
// 11/10/2003
// 12/15/2004 converted to x132 micro with PWM
// 4/24/2005 converted to variable sampling rate
//
// Author: GAStephens
//
// These functions read the ADC0 at a specified rate.
// 
// The ADC0 End of Conversion Interrupt Handler retrieves the sample
// from the ADC and adds it to a running accumulator.  Every <INT_DEC> 
// samples, the ADC updates and stores its result in the global variable
// <adcvalue>, which holds the current adcvalue in hundredths of a
// degree.  The sampling technique of adding a set of values and
// decimating them (posting results every (n)th sample) is called 'integrate
// and dump.'
//
// For each power of 4 of <INT_DEC>, you gain 1 bit of effective resolution.
// For example, <INT_DEC> = 256 gain you 4 bits of resolution: 4^4 = 256.
//
// mux settling time ~ 0.5 milliseconds

#include <stdlib.h>
#include <sensor.h>

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ADC0_Init
//-----------------------------------------------------------------------------
//
// Configure ADC0 to use Timer3 overflows as conversion source, to
// generate an interrupt on conversion complete, and to use right-justified
// output mode.  Enables ADC end of conversion interrupt. Leaves ADC disabled.
//
void ADC0_Init (void) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	
	// Get the calibration value for X, Y
	ReadScratchpad(scratch.scratchbuff);

	// configure the PCA for PWM operation
	if (!su2) {
		SFRPAGE = PCA0_PAGE;
		PCA0MD =	0x80;					// 1000000 suspend with microp, SYSCLK / 12
		PCA0CN =	0x40;					// 01000000 PCA0 enabled
		PCA0CPM0 =	0x42;					// CCM0 in 8-bit PWM mode
		PCA0CPM1 =	0x42;					// CCM1 in 8-bit PWM mode

		PCA0CPL0 =	scratch.calibrate.calx;		// initialize PCA PWM value
		PCA0CPH0 =	scratch.calibrate.calx;
		PCA0CPL1 =	scratch.calibrate.caly;		// initialize PCA PWM value			
		PCA0CPH1 =	scratch.calibrate.caly;
	} // if

	// configure sampling rate
	int_dec = SAMPLE_RATE * NUM_CHAN / samps_per_sec;

	// set up the ADC
	SFRPAGE = ADC0_PAGE;
	ADC0CN = 0xC0;						// 11000001 ADC0 enabled; special tracking
										// ADC0 conversions are initiated 
										// on AD0BUSY=1; ADC0 data is right-justified

	REF0CN = 0x07;                      // enable temp sensor, on-chip VREF,
   										// and VREF output buffer
	AMX0CF = 0x00;						// all non-differential inputs, no gain
	AMX0SL = 0x00;                      // Select AIN0 external input on mux
	ADC0CF = ((SYSCLK/2/ADC_RATE) << 3) | 0x00;	// ** ADC conv clock = 2.5MHz, Gain = 1

	EIE2 |= 0x02;						// enable ADC interrupts

	SFRPAGE = SFRPAGE_SAVE;
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
void ADC0_ISR (void) interrupt 15 {
	extern u08 channel;					// which ADC channel are we sampling
	extern u16 int_dec;   				// integrate/decimate counter
                                       // post results when int_dec = 0
	extern xdata u16 accumulator[NUM_CHAN]; // ** here's where we integrate ADC samples             
	int i;

	SFRPAGE = ADC0_PAGE;

	AD0INT = 0;							// clear ADC conversion complete indicator

	accumulator[channel] += ADC0;		// read ADC value and add to running total

	// reprogram mux to select next channel
	switch (channel) {
		case 0:															
			AMX0SL = 0x01;					// AIN0 moving to AIN1 (y accel)
			channel = 1;
			break;
		case 1:								// AIN1 moving to AIN2 (power fail detect)
			AMX0SL = 0x02;
			channel = 2;
			break;			
		case 2:								// AIN1 moving to AIN7 (z accel) or AIN8 (temperature)
			if (su2)
				AMX0SL = 0x07;
			else
				AMX0SL = 0x08;
			channel = 3;
//			ADC0CF = ((SYSCLK/ADC_RATE - 1) << 3) | 0x00;	// ADC Gain = 1
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
		u16 dividefactor = SAMPLE_RATE / samps_per_sec / 4;

		// divide out oversample on all but temperature
		for (i=0; i<NUM_CHAN; i++) {
			if (i == 3 && !su2)
				// fix up the temperature
				result[3] = accumulator[3] * (samps_per_sec / 10);
			else
				result[i] = accumulator[i] / dividefactor; // at 10, samprate value is x16
											// 10 bit A-D x 16 shift right 2
											// resultant range is 0-4095 or 12 bits
			accumulator[i] = 0;				// reset accumulator
		} // for


		if (!calibrating) {
			// invert X to handle latest hardware
 			result[0] = scratch.calibrate.xminusg +
					 scratch.calibrate.xplusg
					 - result[0];
//		result[1] = scratch.calibrate.yminusg +
//				 scratch.calibrate.yplusg
//				 - result[1];
		} // if
		
		int_dec = SAMPLE_RATE * NUM_CHAN / samps_per_sec;	// reset counter
		adcresults = TRUE;				// set semaphore
	}
	if (!(0 == channel)) {
		AD0BUSY = 1;	
	}
}

