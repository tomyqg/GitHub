// Part of traqmate.c
// 3/28/2009
//
// Author: GAStephens
//
// These functions control the Microchip MCP4023 programmable resister (digital potentiometer)
// that is used in the second generation Sensor Unit to implement the AutoOn feature
// 

#include <stdlib.h>
#include <sensor.h>

//-----------------------------------------------------------------------------
// DigiPot(autoon)
// if autoon = ON, the resistance will be railed to the highest setting which
// will cause AutoOn to be enabled.
// autoon = OFF causes the opposite.
//-----------------------------------------------------------------------------
//
sbit DIGPOTUD = P3 ^ 1;
sbit DIGIPOTCS = P3 ^ 2;

void DigiPot(BOOL autoon) {

// The digipot is located on port 3, pins 3.1 (updown) and 3.2 (/cs)
#define DIGIPOTSTEPS		(64 + 2)	// number of steps in digipot plus a couple more
#define DIGIPOTDELAY		30			// how many loops to delay, must equal 500ns
										// 20 was slewing to 4.5V so increased to 30
	u08 i, j;

	if (autoon)		// incrementing
		DIGPOTUD = 0;					// set the initial state	
	else			// decrementing
		DIGPOTUD = 1;					// set the initial state	
	
	// let up/down select sink in
	for (j=0; j<DIGIPOTDELAY; j++) ;
		
	DIGIPOTCS = 1;						// chip enabled
	for (j=0; j<DIGIPOTDELAY; j++) ;	// let it sink in	

	// start toggling (double because going up and down)
	for (i = 0; i < (2*DIGIPOTSTEPS+1); i++) {
		DIGPOTUD = ~DIGPOTUD;
		for (j=0; j<DIGIPOTDELAY; j++) ;
	} // for

	DIGIPOTCS = 0;					// disable chip select, causing nonvolatile write
} // DigiPot