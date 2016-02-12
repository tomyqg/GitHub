#define AUDIO_MAIN

#include <stdio.h>
#include <string.h>
#include "SPI_API.h"
#include "DUII.h"
#include "dbgu.h"
#include "twi.h"
#include "AudioTask.h"

//
// External data declarations
//

//
// External functions
//
void mSDelay( int );

//
// Local defines, constants and structure definitions
//
AT91S_TWI	*twi;

//
// Local function prototypes
//
void Speak( char *sound) {
	unsigned int bytectr = sizeof (*sound);
	char *pByte = sound;
	
//	TWI_WriteByte(&twi, 0);		// clear out the interface

	TWI_StartWrite(twi, DAC_SLAVE_ADDR, 0, 0, 0);

	while (bytectr--) {
		while (1 != TWI_ByteSent(twi)) ;	// wait for char to finish
		TWI_WriteByte(twi, *pByte++);		// send a character
	} // while		

} // Speak

static void InitAudioTask(void) {

	twi = (AT91S_TWI *) 0xFFFA8000;

	// create clock for TWI
//	PMC_SCER = 1 << 9;		// PCK1
	PMC_SCER = AT91C_PMC_PCK1;
	PMC_PCK1 = 0x0001;		// select main clock with no divider
//	PMC_PCK1 = 0x0002;		// select PLL clock with no divider
	
	// set up TWI interface
	TWI_ConfigureMaster(twi, TWI_CLOCK, MASTER_CLOCK);
		
	// program the dac
	// set audio divider for 12MHz input clock and 8KHz audio sampling rate
	TWI_StartWrite(twi, DAC_SLAVE_ADDR, DAC_SAMPLING1, 1, DAC_SYSCLK1);
	while (1 != TWI_ByteSent(twi)) ;	// wait for char to finish
	TWI_StartWrite(twi, DAC_SLAVE_ADDR, DAC_SAMPLING2, 1, DAC_SYSCLK2);
	while (1 != TWI_ByteSent(twi)) ;	// wait for char to finish
	
	// program the amplifier
	TWI_StartWrite(twi, AMP_SLAVE_ADDR, AMP_CONTROL_REGISTER, 1, AMP_ENABLE);
	while (1 != TWI_ByteSent(twi)) ;	// wait for char to finish
	TWI_StartWrite(twi, AMP_SLAVE_ADDR, AMP_CONTROL_REGISTER, 1, AMP_ENABLE);
	while (1 != TWI_ByteSent(twi)) ;	// wait for char to finish
	
	// clear any pending events
	OS_ClearEvents(&audioTCB);

	// say something
	Speak(welcomeToTraqDash);

} // InitAudioTask

#define ALL_AUDIO_EVENTS ((1 << AUDIO_SOUND_DONE))

void AudioTask( void ) {
	char which_event;										 // bitmask of events that occured

	OS_Delay(5000);				// wait for initialization to complete
	
  	InitAudioTask();

	while (1) {	
		// wait for something to happen
		which_event = OS_WaitEvent( (unsigned char) ALL_AUDIO_EVENTS );
		
		if (which_event & (1 << AUDIO_SOUND_DONE)) {
			
			// if there isnanother soundbyte on the queue, get it started

		} // if
		
	} // while
} // AudioTask
