// Part of traqmate.c
// 11/15/2004
// 3/27/2009 added support for Ublox LEA-5 GPS chip
// Author: GAStephens
//
// This file contains the Ublox UBX protocol definitions.
//
#include <sensor.h>
#include <ubx.h>

#define NUMUBLOXMSGS	3			// number of UBX messages to handle
#define LEADYNPLAT		4			// dynamic platform for LEA chip, 4 = car

typedef struct {
	u16 messageid;
	u08 enabled;
} messagelisttype;

void UBXBaudChange() {
	u16 i;
	u32 timeoutctr;
	
	// point to the GPS receiver configuration packet
	u08 *packet = com[1].txbuff;
	
	// clear out any junk in the input buffer or messages received
	com[1].rxnextidx = com[1].rxfirstidx;

#ifdef DEBUG
	PWRLED = 1;			// signal that we are trying to communicate with Ublox
#endif

	// send a set measuring rate packet just to ensure that we are talking with Ublox
	while (1) {	// repeat until we get it right
		i = 0;
		timeoutctr = 0;

		packet[UBXPAYLOAD+i++] = GPS_MS & 0xFF;			// lower byte of GPS measurement rate in ms
		packet[UBXPAYLOAD+i++] = GPS_MS >> 8;			// upper byte of GPS measurement rate in ms
		packet[UBXPAYLOAD+i++] = 1;						// 1 report per measurement cycle
		packet[UBXPAYLOAD+i++] = 0;

		packet[UBXPAYLOAD+i++] = 1;						// align to GPS time
		packet[UBXPAYLOAD+i++] = 0;

		// send	configuration message
		UBXSendPacket(CFGRATE, i);

		// wait for transmission to finish
		while (0 != com[1].txbytes) ;

		i = clocktick;			// init message id

		// wait for a valid ubx response with approx 435 msec timeout (timeoutctr max / 11500)
		do {} while (com[1].rxnextidx == com[1].rxfirstidx && ++timeoutctr < 5000000L) ;

		if (com[1].rxnextidx != com[1].rxfirstidx)		// got a good message = ublox talking
			break;
	} ; // while

#ifdef DEBUG
	RECLED = 1;			// signal that we have successfully talked at 9600
#endif

	// we are talking so now configure protocols and baudrates on port
	i = 0;

	packet[UBXPAYLOAD+i++] = 1;			// 0 port number 1
	packet[UBXPAYLOAD+i++] = 0;			// 1 reserved
	packet[UBXPAYLOAD+i++] = 0;			// 2 reserved
	packet[UBXPAYLOAD+i++] = 0;			// 3 reserved
	packet[UBXPAYLOAD+i++] = 0xC0;		// 4 11000000	8 bits
	packet[UBXPAYLOAD+i++] = 0x08;		// 5 00001000	1 stop bit, no parity				
	packet[UBXPAYLOAD+i++] = 0;			// 6 bitmask second byte
	packet[UBXPAYLOAD+i++] = 0;			// 7 bitmask top byte
	* (s32 *) (packet + UBXPAYLOAD + i) = SWAP32(UBXBAUD);	// 8 57 Kbps
	i += 4;
	packet[UBXPAYLOAD+i++] = 0x01;		// 12 bitmask ublox input enabled
	packet[UBXPAYLOAD+i++] = 0x00;		// 13 bitmask top byte
	packet[UBXPAYLOAD+i++] = 0x01;		// 14 bitmask ublox output enabled
	packet[UBXPAYLOAD+i++] = 0x00;		// 15 bitmask top byte
	packet[UBXPAYLOAD+i++] = 0;			// 16 reserved
	packet[UBXPAYLOAD+i++] = 0;			// 17 reserved
	packet[UBXPAYLOAD+i++] = 0;			// 18 reserved
	packet[UBXPAYLOAD+i++] = 0;			// 19 reserved

	// send	configuration message
	UBXSendPacket(CFGPRT, i);

	// wait for transmission to finish
	while (0 != com[1].txbytes) ;

	// create a gap in case uart glitches on baudrate reprogram
	DELAY_MS(5);
//	DELAY_MS(30);

#ifdef DEBUG
	GPSLED = 1;				// signal that baud rate changed
#endif
		
	// change the UART baud rate
	UART_Init(&(com[1]), UBXBAUD);

	// breath a moment
	DELAY_MS(40);			// 40 msec is correct value = verified on o-scope
		
} // UBXBaudChange

void UBXInit() {
	u16 i;
	u08 msgcnt;
	xdata messagelisttype messagelist[NUMUBLOXMSGS] = {
		{ NAVVELNED, 1 },
		{ NAVSOL, 1 },
		{ NAVPOSLLH, 1 }
		};

	// point to the GPS receiver configuration packet
	u08 *packet = com[1].txbuff;
	
	if (su2) {	// LEA-5A
		// clear out any junk in the input buffer from baud change
		com[1].rxnextidx = com[1].rxfirstidx;
	
		// turn UBX messages on/off
		for (msgcnt = 0; msgcnt < NUMUBLOXMSGS; msgcnt++) {
			do {	// repeat until we get it right
				i = 0;
		
				packet[UBXPAYLOAD+i++] = (messagelist[msgcnt].messageid >> 8) & 0xFF;	// class
				packet[UBXPAYLOAD+i++] = messagelist[msgcnt].messageid & 0xFF;		// id
				packet[UBXPAYLOAD+i++] = messagelist[msgcnt].enabled;		// enable?
		
				// send	configuration message
				UBXSendPacket(CFGMSG, i);
	
				// wait for transmission to finish
				while (0 != com[1].txbytes) ;
		
				// wait for a UBX message
				while (com[1].rxnextidx == com[1].rxfirstidx)
					if (10000 == i++) break;
		
				do { // check all queued UBX messages
					// get the message id
					i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
							
					// increment counter to point to next inbound message in queue
					com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
	
					if (i == ACKACK) break;
		
				} while (com[1].rxnextidx != com[1].rxfirstidx);
		
				DELAY_MS(10);			// pause a moment
			} while (i != ACKACK);
#ifdef DEBUG
			PWRLED = 0;				// signal that communication has succeeded at 576
#endif
		} // for
	} // if
	else {	// su1 = TIM-LC
		// turn UBX messages on/off and set the reporting rate
		// done in two stages to keep the message sizes down
		do {	// repeat until we get it right
			i = 0;
	
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVPOSLLH & 0xFF;		// position information
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 1;						// output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVPOSECEF & 0xFF;		// position information
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVSOL & 0xFF;			// navigation solution
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 1;						// output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVVELNED & 0xFF;		// velocities
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 1;						// output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVSTATUS & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVSVINFO & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVDOP & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVTIMEGPS & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;

			// send	configuration message
			UBXSendPacket(CFGMSG, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
	
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
			do { // check all queued UBX messages
				// get the message id
				i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
								
				// increment counter to point to next inbound message in queue
				com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
	
				if (i == ACKACK) break;
	
			} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(20);			// pause a moment
	
		} while (i != ACKACK);
	
		// turn UBX messages on/off and set the reporting rate
		do {	// repeat until we get it right
			i = 0;
	
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVVELECEF & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVTIMEUTC & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = NAV;
			packet[UBXPAYLOAD+i++] = NAVCLOCK & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = MON;
			packet[UBXPAYLOAD+i++] = MONRXBUF & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = MON;
			packet[UBXPAYLOAD+i++] = MONTXBUF & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = MON;
			packet[UBXPAYLOAD+i++] = MONIO & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = MON;
			packet[UBXPAYLOAD+i++] = MONHW & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = MON;
			packet[UBXPAYLOAD+i++] = MONSCHD & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = MON;
			packet[UBXPAYLOAD+i++] = MONIPC & 0xFF;
			packet[UBXPAYLOAD+i++] = 0;						// no output on target 0
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 1
			packet[UBXPAYLOAD+i++] = 0;						// no output on uart 2
			packet[UBXPAYLOAD+i++] = 0;

			// send	configuration message
			UBXSendPacket(CFGMSG, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
	
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
			do { // check all queued UBX messages
				// get the message id
				i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
								
				// increment counter to point to next inbound message in queue
				com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
	
				if (i == ACKACK) break;
	
			} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(20);			// pause a moment
	
		} while (i != ACKACK);

		// set the measuring rate (done earlier in su2)
		do {	// repeat until we get it right
			i = 0;
	
			packet[UBXPAYLOAD+i++] = GPS_MS & 0xFF;			// lower byte of GPS measurement rate in ms
			packet[UBXPAYLOAD+i++] = GPS_MS >> 8;			// upper byte of GPS measurement rate in ms
			packet[UBXPAYLOAD+i++] = 1;						// 1 report per measurement cycle
			packet[UBXPAYLOAD+i++] = 0;
	
			packet[UBXPAYLOAD+i++] = 1;						// align to GPS time
			packet[UBXPAYLOAD+i++] = 0;
			
			// send	configuration message
			UBXSendPacket(CFGRATE, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
	
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
			do { // check all queued UBX messages
				// get the message id
				i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
						
				// increment counter to point to next inbound message in queue
				com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
	
				if (i == ACKACK) break;
	
			} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(10);			// pause a moment
	
		} while (i != ACKACK);
	} // else
	
	// set the pulse parameters
	do {	// repeat until we get it right
		u32 templong;
		
		templong = 10000000L / (long) samps_per_sec;
		* ((u32 *) &(packet[UBXPAYLOAD])) = SWAP32(templong);	// pulse time interval

		templong = GPS_PULSE_WID;
		* ((u32 *) &(packet[UBXPAYLOAD+4])) = SWAP32(templong);	// pulse time interval

		i = 8;
		packet[UBXPAYLOAD+i++] = 1;						// positive pulse
		packet[UBXPAYLOAD+i++] = 1;						// GPS time
		packet[UBXPAYLOAD+i++] = 0;						// reserved
		packet[UBXPAYLOAD+i++] = 0;						// reserved
		packet[UBXPAYLOAD+i++] = 50;					// 50 ns cable delay (Ublox supplied typical value)
		packet[UBXPAYLOAD+i++] = 0;
		packet[UBXPAYLOAD+i++] = 820 & 0xFF;			// rcvr delay (Ublox supplied value)
		packet[UBXPAYLOAD+i++] = 820 >> 8;
		packet[UBXPAYLOAD+i++] = 0;						// 0 user delay
		packet[UBXPAYLOAD+i++] = 0;
		packet[UBXPAYLOAD+i++] = 0;
		packet[UBXPAYLOAD+i++] = 0;
		
		// send	configuration message
		UBXSendPacket(CFGTP, i);

		// wait for transmission to finish
		while (0 != com[1].txbytes) ;

		// wait for a UBX message
		while (com[1].rxnextidx == com[1].rxfirstidx) ;

		do { // check all queued UBX messages
			// get the message id
			i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
							
			// increment counter to point to next inbound message in queue
			com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;

			if (i == ACKACK) break;

		} while (com[1].rxnextidx != com[1].rxfirstidx);
		
		DELAY_MS(10);			// pause a moment
		
	} while (i != ACKACK);

	if (su2) {
#if 0		// drop this code after V2.20 is successful in field
		// change to max performance mode, Ublox firmware V4.00 - undocumented command from
		// Ublox document GPS.G5.CS-08022, Ublox5 Power Management Considerations
		do {	// repeat until we get it right
			i = 0;
		
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = 0;
			packet[UBXPAYLOAD+i++] = 0x40;			
			packet[UBXPAYLOAD+i++] = 0x1F;
			packet[UBXPAYLOAD+i++] = 0xE8;
			packet[UBXPAYLOAD+i++] = 0x03;
			packet[UBXPAYLOAD+i++] = 0x98;
			packet[UBXPAYLOAD+i++] = 0x3A;
			packet[UBXPAYLOAD+i++] = 0xB0;
			packet[UBXPAYLOAD+i++] = 0x04;
			packet[UBXPAYLOAD+i++] = 0xE8;
			packet[UBXPAYLOAD+i++] = 0x03;
			packet[UBXPAYLOAD+i++] = 0x03;				
			packet[UBXPAYLOAD+i++] = 0;			
										
			// send	configuration message
			UBXSendPacket(CFGUDOC, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
		
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
			do { // check all queued UBX messages
				// get the message id
				i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
						
				// increment counter to point to next inbound message in queue
				com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
	
				if (i == ACKACK) break;
	
			} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(10);			// pause a moment
		} while (i != ACKACK);

		// code trap
		i = i + 1;
#endif
		// change to max performance mode, Ublox LEA-5 firmware V5.00 and higher
		do {	// repeat until we get it right
			i = 0;
		
			packet[UBXPAYLOAD+i++] = 0;			// 0 reserved
			packet[UBXPAYLOAD+i++] = 0;			// 1 Max performance mode
													
			// send	configuration message
			UBXSendPacket(CFGRXM, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
		
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
			do { // check all queued UBX messages
				// get the message id
				i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
						
				// increment counter to point to next inbound message in queue
				com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
	
				if (i == ACKACK) break;
	
			} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(10);			// pause a moment
		} while (i != ACKACK);

		// turn off auto sbas to allow good 4Hz operation
		do {	// repeat until we get it right
			i = 0;
	
			packet[UBXPAYLOAD+i++] = 0;			// 0 sbas disabled
			packet[UBXPAYLOAD+i++] = 0;			// 1 no uses
			packet[UBXPAYLOAD+i++] = 0;			// 2 max sbas channels
			packet[UBXPAYLOAD+i++] = 0;			// 3 scanmode
			packet[UBXPAYLOAD+i++] = 0;			// 4 scanmode
			packet[UBXPAYLOAD+i++] = 0;			// 5 scanmode
			packet[UBXPAYLOAD+i++] = 0;			// 6 scanmode
			packet[UBXPAYLOAD+i++] = 0;			// 7 scanmode
			
			// send	configuration message
			UBXSendPacket(CFGSBAS, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
	
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
			do { // check all queued UBX messages
				// get the message id
				i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
								
				// increment counter to point to next inbound message in queue
				com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
				
				if (i == ACKACK) break;
	
			} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(10);			// pause a moment
	
		} while (i != ACKACK);
	
		// set the nav engine settings
		do {	// repeat until we get it right
			i = 0;
	
			packet[UBXPAYLOAD+i++] = 0x05;					// 0 parameter bitmask lower byte, change dynamic platform & fixmode
			packet[UBXPAYLOAD+i++] = 0x00;					// 1 parameter bitmask upper byte
			packet[UBXPAYLOAD+i++] = LEADYNPLAT;			// 2  dynamic platform, 0 = portable, 2 = stationery, 
															//	 3 = pedestrian, 4 = car, 5 = sea, 6 = airborne <1g,
															//	 7 = airborne <2g, 8 = airborne <4g
			packet[UBXPAYLOAD+i++] = 2;						// 3 fixing mode - 3d only
			* (s32 *) (packet + UBXPAYLOAD + i) = SWAP32(50000L);	// 4 fixed alt - 2d mode only
			i += 4;
			* (s32 *) (packet + UBXPAYLOAD + i) = SWAP32(0L);	// 8 fixed alt variance
			i += 4;
			packet[UBXPAYLOAD+i++] = 5;						// 12 min elevation
			packet[UBXPAYLOAD+i++] = 0;						// 13 dead reckoning max time
			* (u16 *) (packet + UBXPAYLOAD + i) = SWAP16(250); // 14 position dop mask
			i += 2;
			* (u16 *) (packet + UBXPAYLOAD + i) = SWAP16(250); // 16 time dop mask
			i += 2;
			* (u16 *) (packet + UBXPAYLOAD + i) = SWAP16(100); // 18 position accuracy mask
			i += 2;
			* (u16 *) (packet + UBXPAYLOAD + i) = SWAP16(300); // 20 time accuracy mask
			i += 2;
			packet[UBXPAYLOAD+i++] = 0;						// 22 static threshold in cm/s, 0 = disable
			packet[UBXPAYLOAD+i++] = 0;						// 23 reserved
			packet[UBXPAYLOAD+i++] = 0;						// 24 reserved 1
			packet[UBXPAYLOAD+i++] = 0;						// 25 reserved 2
			packet[UBXPAYLOAD+i++] = 0;						// 26 reserved 3
			packet[UBXPAYLOAD+i++] = 0;						// 27 reserved 4
			packet[UBXPAYLOAD+i++] = 0;						// 28 reserved 1
			packet[UBXPAYLOAD+i++] = 0;						// 29 reserved 2
			packet[UBXPAYLOAD+i++] = 0;						// 30 reserved 3
			packet[UBXPAYLOAD+i++] = 0;						// 31 reserved 4
			packet[UBXPAYLOAD+i++] = 0;						// 32 reserved 1
			packet[UBXPAYLOAD+i++] = 0;						// 33 reserved 2
			packet[UBXPAYLOAD+i++] = 0;						// 34 reserved 3
			packet[UBXPAYLOAD+i++] = 0;						// 35 reserved 4
							
			// send	configuration message
			UBXSendPacket(CFGNAV5, i);
	
			// wait for transmission to finish
			while (0 != com[1].txbytes) ;
	
			// wait for a UBX message
			while (com[1].rxnextidx == com[1].rxfirstidx) ;
	
				do { // check all queued UBX messages
					// get the message id
					i = * ((u16 *) (com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE));
					
					// increment counter to point to next inbound message in queue
					com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;
		
					if (i == ACKACK) break;
		
				} while (com[1].rxnextidx != com[1].rxfirstidx);
	
			DELAY_MS(10);			// pause a moment
	
		} while (i != ACKACK);	
	} // if
} // UBXInit

void UBXSendPacket(u16 ubxcmd, u16 datalen) {
	int	i;
	u08 cka, ckb;

	// first synch char
	com[1].txbuff[0] =	SYNCH1;
	// second synch char
	com[1].txbuff[1] =	SYNCH2;
	// class
	com[1].txbuff[2] =	ubxcmd >> 8;
	// message id
	com[1].txbuff[3] =	ubxcmd & 0xFF;
	// length lsb
	com[1].txbuff[4] =	datalen & 0xFF;
	// length msb
	com[1].txbuff[5] =	datalen >> 8;

	// calculate checksum
	cka = ckb = 0;
	for(i=2; i<(datalen+UBXPAYLOAD); i++) {
		cka = cka + com[1].txbuff[i];
		ckb = ckb + cka;
	}
	// checksum a
	com[1].txbuff[datalen+UBXPAYLOAD] =	cka;
	// checksum b
	com[1].txbuff[datalen+UBXPAYLOAD+1] = ckb;

	// send	out	the	packet
	XmitUART(&(com[1]), datalen+UBXPAYLOAD+2);
}

// the following routine will parse a ubx message based on the class and message id
void Parse_UBX(u08 *ubxptr, gpstype * gps) {
	static xdata BOOL ubxdone = FALSE;			// TRUE when ubx has been initialized
	static xdata s16 simevel = 3;				// e-w velocity for simulation
	static xdata s16 simnvel = -5;				// n-s velocity for simulation
	static xdata BOOL eastinc = FALSE;			// TRUE if east velocity increasing
	static xdata BOOL northinc = TRUE;			// TRUE if north velocity increasing
	static xdata BOOL updaterate = 0;			// controls change in east and north velocity
	u08 *ptr;									// general purpose pointer
	int i;										// counter index

	// upon first command, set up the unit
	if (!ubxdone) {
		ubxdone = TRUE;
		// initialize the Ublox
		UBXInit();
	} // if

	// switch on the class/message type
	else switch(* ((u16 *) ubxptr)) {
		case NAVPOSLLH: {			// position in LLH format (degrees)
			s32 templong;
			BOOL negative;

// V2.1 changed 11930L to 11931L which provides a closer approximation by compensating for round-down
			code long degarray[] = {	1L,
										12L,
										119L,
										1193L,
										11931L,
										119305L,
										1193046L,
										11930465L,
										119304647L,
										1193046471L }; 

			ubxptr += 6;			// skip message id and time

			// fill in longitude, swapping byte order
			ptr = ((u08 *) &(templong))+3;
			for (i = 0; i < 4; i++)
				*ptr-- = *ubxptr++;

// what we are doing is converting from degrees x 10^7 scaled from -1800000000 to 1800000000
// to -2^31 to 2^31 to match the original specification

// 2^31 / 1800000000 = 1.193046471

			negative = (templong < 0);
			templong = ABS(templong);
			gps->lon = 0L;

			// for each of the ten digits, multiply and accumulate
			for (i = 0; i < 10; i++) {
				gps->lon += (templong % 10L) * degarray[i];
				templong = templong / 10L;		// drop a digit
			} // for

			if (negative) gps->lon = -gps->lon;

			// fill in latitude, swapping byte order
			ptr = ((u08 *) &(templong))+3;
			for (i = 0; i < 4; i++)
				*ptr-- = *ubxptr++;

			negative = (templong < 0);
			templong = ABS(templong);
			gps->lat = 0L;

			// for each of the ten digits, multiply and accumulate
			for (i = 0; i < 10; i++) {
				gps->lat += (templong % 10L) * degarray[i];
				templong = templong / 10L;		// drop a digit
			} // for

			if (negative) gps->lat = -gps->lat;

			// use mean sea level for alt so skip ht above elipsoid
			ubxptr += 4;

			ptr = ((u08 *) &templong)+3;
			for (i = 0; i < 4; i++)
				*ptr-- = *ubxptr++;

			templong /= 1000L;

			// get the long version of altitude (mm) and convert to meters
			gps->alt = (s16) templong;


			ubxfullset |= 0x04;

		} // NAVPOSLLH
		break;

		case NAVSOL: {		// GPS solution info and time
			// get DOP
			dop = ubxptr[44+2] + ((ubxptr[45+2] & 0xFF) << 8);
	
			// get number of SVs
			gpslock = ubxptr[47+2];	// position + 2 chars for message id

			ubxptr += 2;	// skip over message id

			// skip if solution not valid
			if (0x0D != (0x0D & *(ubxptr + 11)))	// 1101	= mask for GPSOK, WEEK OK, TIME OK
				gpslock = 0;
			else {		// good solution
 				// fill in time of week in ms
				ptr = ((u08 *) &(gps->time))+3;
				for (i = 0; i < 4; i++)
					*ptr-- = *ubxptr++;
	
#ifndef DISPLAYATTACHED
				// save first good date as starting date
				ptr++;
				if (gpslock && 0xFFFFFFFF == sessstarttime &&
					0L != *((u32 *) ptr) &&
					0xFFFFFFFF != *((u32 *) ptr))
						sessstarttime = *((u32 *) ptr);
	
				// keep last good date as ending date
				if (gpslock && 0L != *((u32 *) ptr) &&
					0xFFFFFFFF != *((u32 *) ptr))
						sessendtime = *((u32 *) ptr);
#endif
				// skip over fraction
				ubxptr += 4;
	
				// fill in week number
				ptr = ((u08 *) &(gps->weeks))+1;
				for (i = 0; i < 2; i++)
					*ptr-- = *ubxptr++;
	
#ifndef DISPLAYATTACHED
				// save first good week as starting week
				ptr++;
				if (gpslock && 0xFFFF == sessstartweeks &&
					0 != *((u16 *) ptr) &&
					0xFFFF != *((u16 *) ptr))
						sessstartweeks = *((u16 *) ptr);
	
				// keep last good week as ending week
				if (gpslock && 0 != *((u16 *) ptr) &&
					0xFFFF != *((u16 *) ptr))
						sessendweeks = *((u16 *) ptr);
#endif
			} // if	

			// mark that we got a navsol even if we didn't use it
			ubxfullset |= 0x02;
		} // NAVSOL
		break;

		case NAVVELNED: {		// GPS velocities
			s32 templong;
			s16 eastvel, northvel, vertvel;

			ubxptr += 6;			// skip over message id and time

			// get velocity, flip byte order, and scale from cm/s to m/s
			templong = * (s32 *) (ubxptr);
			templong = SWAP32((u32) templong);
			templong /= 100;
			northvel = (s16) templong;

			ubxptr += 4;

			// get velocity, flip byte order, and scale from cm/s to m/s
			templong = * (s32 *) (ubxptr);
			templong = SWAP32((u32) templong);
			templong /= 100;
			eastvel = (s16) templong;

			ubxptr += 4;

			// get velocity, flip byte order, scale from cm/s to m/s, and invert
			templong = * (s32 *) (ubxptr);
			templong = SWAP32((u32) templong);
			templong /= 100;
			vertvel = - (s16) templong;

			// pack the velocity data
			gps->velpack[0] = (u08) ((((u16) eastvel) & 0x0700) >> 3);
			gps->velpack[0] += (u08) ((((u16) northvel) & 0x0700) >> 6);
			gps->velpack[0] += (u08) ((((u16) vertvel) & 0x0300) >> 8);
			gps->velpack[1] = ((u16) eastvel) & 0xFF;
			gps->velpack[2] = ((u16) northvel) & 0xFF;
			gps->velpack[3] = ((u16) vertvel) & 0xFF;

			ubxfullset |= 0x01;

		} // NAVVELNED
		break;

		default:
			// breakpoint trap
			i = 1;
			break;
	} // switch

// basic 2.10 include dead reckoning #ifdef DISPLAYATTACHED
	if (0x07 == ubxfullset) {		// got all 3 messages

		// clear message list (can also be done at gpspulse)
		ubxfullset = 0;

		// if no gpslock then interpolate
		if (!gpslock) {

	 		// copy the last one for a start
	 		*gps = previousgps[lastgps];
		
			// adjust the time by the GPS sample rate
			// NOTE: does not compensate for event of data loss at week boundary
			gps->time = gps->time + (1000 / GPS_RATE);

			// don't interpolate if not enough good history
			if (0L != previousgps[1-lastgps].time) {	
				// interpolate the lat and lon. factor in gps recording freq
				gps->lat = gps->lat + (previousgps[lastgps].lat - previousgps[1-lastgps].lat);
				gps->lon = gps->lon + (previousgps[lastgps].lon - previousgps[1-lastgps].lon);
			} // if
		} // if
		
		// store latest gps for historical purposes
		lastgps = 1 - lastgps;			// swap buffers to oldest one
		previousgps[lastgps] = *gps;	// overwrite with latest

#ifdef DISPLAYATTACHED		
		// copy data into transmit buffer
		if (simgps)	{
			gpslock = 6;
			gps->time = 10000L;			// fake day and time
			gps->weeks = 1710;		// end of 2012
			gps->lat = 12348023L;		// fake latitude
			gps->lon = 2763743L;		// fake longitude
			gps->alt = 300;			// altitude in meters
			
			updaterate++;
			// animated velocities and headings
			if (simevel >= 50) {	// e-w velocity m/s
				eastinc = FALSE;
			} // if
			else if (simevel <= -50) {
				eastinc = TRUE;
			} // else if
			if (updaterate % 5) {
				if (eastinc)
					simevel += 1;
				else
					simevel -= 1;
			} // if
			
			if (simnvel >= 50) {	// e-w velocity m/s
				northinc = FALSE;
			} // if
			else if (simnvel <= -50) {
				northinc = TRUE;
			} // else if
			if (updaterate+2 % 5) {
				if (northinc)
					simnvel += 1;
				else
					simnvel -= 1;
			} // if
			
			// pack the velocity data
			gps->velpack[0] = (u08) ((((u16) simevel) & 0x0700) >> 3);
			gps->velpack[0] += (u08) ((((u16) simnvel) & 0x0700) >> 6);
			gps->velpack[1] = ((u16) simevel) & 0xFF;
			gps->velpack[2] = ((u16) simnvel) & 0xFF;

			gps->velpack[3] = 1;		// z velocity
			gps->temp = 25;			// temperature
		} // if

		// copy data into transmit buffer
		ptr = com[msgrouting[DISPLAY_UNIT]].txbuff+DATA_START;
		*ptr++ = (u08) gpslock;
		*((gpstype *) ptr) = *gps;

		// tell the display unit
		SendMessage((u08) SENSOR_UNIT, (u08) DISPLAY_UNIT, (u08) GPS_DATA, 1+sizeof(gpstype));
#endif
	} // if
// basic 2.10 #endif
} // Parse_UBX