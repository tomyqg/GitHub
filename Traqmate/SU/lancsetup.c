// Part of traqmate.c
// 1/13/10
//
// Author: GAStephens
//
// These functions setup the LANC interface
// 

#include <stdlib.h>
#include <sensor.h>

void New_Accessory_Setup( void ) {

	if (0 == DIGIN0 && 1 == DIGIN1) {	// check for CHASECAM or LANC
		u16 timeoutctr = 0;	

		// get LANC setup bits in case we need it
		DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);

		// check for LANC
		DIGOUT0 = 0;
		DIGOUT2 = 0;			
		
		DELAY_MS(1);		// wait for LANC to handshake
		if (1 == DIGIN3 && 1 == DIGIN4) {	// either no accessory or LANC connected
			DIGOUT2 = 1;					// put out 1 on D2 to check for LANC

			for (timeoutctr = 0; timeoutctr < 1000; timeoutctr++)
				if (0 == DIGIN4) break;

			if (0 != DIGIN4) {					// LANC interface handshake failed
				accessory = DIRECTACC;			// no accessory (tach, data, sync) connected
			} // if
			else {		// continue handshake
				u08 bitctr;
				u08 handshakebits;

				// make sure we are valid
				if (flashpage.io.lancconfig < MINLANC || flashpage.io.lancconfig > MAXLANC)
					flashpage.io.lancconfig = DEFAULTLANC;
									
				// put same 4 bits in upper and lower nibble
				handshakebits = (((flashpage.io.lancconfig & 0x0F) << 4) | (flashpage.io.lancconfig & 0x0F));

// GPSLED = 1;		// indicate first part of handshake successful
	
				// write the configuration bits out to LANC interface
				for (bitctr = 0; bitctr < 8; bitctr++) {
					DIGOUT0 = handshakebits & 0x01;			// put bits out there, LSB first
					DIGOUT2 = ~DIGOUT2;						// toggle the strobe line
					DELAY_MS(1);							// wait for the interface
					handshakebits = handshakebits >> 1;		// shift down the next bit
				} // for
	
				for (timeoutctr = 0; timeoutctr < 1000; timeoutctr++)
					if (1 == DIGIN4) break;			// byte transfer ok
		
				if (1 == DIGIN4) {
					accessory = DIRECTLANC;			// handshake successful. LANC verified.
					camlanc.ports = ENABLED4LANC;	// indicate LANC connected on D4
					
					// initialize to not recording and not test mode
					LANCMODE = LANCNORMALMODE;
					LANCCMD = LANCRECSTOP;
					camlanc.status = CAMLANCNOTREADY;
				} // if			
				else {			
					accessory = DIRECTACC;				// bits not received correctly. handshake failed
					camlanc.ports = NOCAMLANC;			// other camera presence determined later
					camlanc.status = UNKNOWNSTATUS;		// status determined later
				} // else
			} // else
		} // if
		else {
			accessory = DIRECTACC;				// bits not received correctly. handshake failed
			camlanc.ports = NOCAMLANC;			// other camera presence determined later
			camlanc.status = UNKNOWNSTATUS;		// status determined later		

			if (su2) {						// test for new style accessories
				// **** INSERT SMBUS COMMUNICATIONS HERE - EXCEPT NOT INITIALIZED YET
			} // else
		} // else
	} // if
} // New_Accessory_Setup