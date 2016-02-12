// Part of traqmate.c
// 3/10/2004
// Author: GAStephens
//
// This file contains the Trimble GPS TSIP protocol definitions.
//
#include <sensor.h>
#include <tsip.h>

void TsipInit() {
	// compose GPS receiver	configuration packet
	xdata u08 packet[10];

#ifdef SETBAUD
	// this command will change the port settings on the GPS unit
	// baud should be passed in to TsipInit
	packet[0] = 0xFF;				// current port
	switch (baud) {
		case 4800:
			packet[1] = packet[2] = 6;
			break;
		case 9600:
			packet[1] = packet[2] = 7;
			break;
		case 19200:
			packet[1] = packet[2] = 8;
			break;
		case 38400:
			packet[1] = packet[2] = 9;
			break;
	} // switch
	packet[3] = 0x03;				// 8 data bits
	packet[4] = 0x00;				// No parity
	packet[5] = 0x00;				// 1 stop bit
	packet[6] = 0x00;				// no flow control
	packet[7] = 0x02;				// TSIP input
	packet[8] = 0x02;				// TSIP output
	packet[9] = 0x00;				// reserved
	// send	configuration
	TsipSendPacket((u08) CMD_PORTSETUP, (int) 10, packet);
	
	// now go change the port settings on the microprocessor to match	

	// wait for transmission to finish so we can re-use tx buffer
	while (0 != com[1].txbytes) ;

	// wait a lot
	{int q; for (q = 0; q < 30000; q++) ; }
#endif

	packet[0] =	SUPER_PACKETS;		// use super packet to get data in one shot
	packet[1] =	0;					// velocity report off
	packet[2] =	0;					// gps time and utc time off
	packet[3] =	0;					// raw measurement off
	// send	configuration
	TsipSendPacket((u08) CMD_CIOOPTIONS, (int) 4, packet);

	// wait for transmission to finish so we can re-use tx buffer
	while (0 != com[1].txbytes) ;

	// wait a little more
	{int q; for (q = 0; q < 5000; q++) ; }

	packet[0] =	CMD_SUPERLARGE;		// specify large superpacket with integer data
	packet[1] =	1;					// turn off automatic updates

	// send	packet selection
	TsipSendPacket((u08) CMD_CSUPER, (int) 2, packet);

}

void TsipSendPacket(char tsipcmd, int datalen, u08 *pktdata) {
	int	i;
	int	dataIdx	= 0;

	// start of	packet
	com[1].txbuff[dataIdx++] =	DLE;
	// packet type
	com[1].txbuff[dataIdx++] =	tsipcmd;
	// add packet data
	for(i=0; i<datalen; i++) {
		if(*pktdata ==	DLE) {
			// do double-DLE escape	sequence
			com[1].txbuff[dataIdx++] =	*pktdata;
			com[1].txbuff[dataIdx++] =	*pktdata++;
		}
		else
			com[1].txbuff[dataIdx++] =	*pktdata++;
	} // for
	// end of packet
	com[1].txbuff[dataIdx++] =	DLE;
	com[1].txbuff[dataIdx++] =	ETX;

	// send	out	the	packet
	XmitUART(&(com[1]), dataIdx);
}

// the following routine will parse a tsip message based on the message id
void Parse_Tsip(u08 *tsipptr, gpstype *gps) {
	static xdata BOOL tsipdone = FALSE;			// TRUE when tsip has been initialized

	// switch on the packet	type
	switch(*(tsipptr)) {
		// on first one of these, set up the GPS unit
		case CMD_RALLINVIEW:
			if (!tsipdone) {
				tsipdone = TRUE;
				TsipInit();
			} // if
			break;
		// command to parse
		case CMD_RSUPER:
			if (*++(tsipptr) == CMD_SUPERLARGE) {
				u08 *ptr1, *ptr2;
				int scalefactor;
				int i;

				// if you get this command no need to reprogram the GPS Unit
				tsipdone = TRUE;

				if (gpslock = tsipptr[28]) {
					int eastvel, northvel, vertvel;

					// get flags data
					scalefactor = (tsipptr[24] & 0x01)? 50 : 200;

					// skip opcode and KeyByte
					ptr1 = tsipptr + 2;

					eastvel =  *(s16*)ptr1 / scalefactor;
					ptr1 += 2;

					// get the north velocity
					northvel =  *(s16*)ptr1 / scalefactor;
					ptr1 += 2;

					// get the up velocity
					vertvel =  *(s16*)ptr1 / scalefactor;
					ptr1 += 2;

					// pack the velocity data
					gps->velpack[0] = (u08) ((eastvel & 0x0700) >> 3);
					gps->velpack[0] += (u08) ((northvel & 0x0700) >> 6);
					gps->velpack[0] += (u08) ((vertvel & 0x0300) >> 8);
					gps->velpack[1] = eastvel & 0xFF;
					gps->velpack[2] = northvel & 0xFF;
					gps->velpack[3] = vertvel & 0xFF;

					ptr2 = (u08 *) gps;

					// fill in time of week
					for (i=0; i<4; i++)
						*ptr2++ = *ptr1++;

					// fill in gps week
					ptr1 = tsipptr + 30;
					for (i=0; i<2; i++)
						*ptr2++ = *ptr1++;

					// fill in latitude, longitude
					ptr1 = tsipptr + 12;
					for (i=0; i<8; i++)
						*ptr2++ = *ptr1++;

					// get the long version of altitude (mm) and convert to meters
					gps->alt = (s16) ((* (s32 *) ptr1) / 1000L);

#ifdef DISPLAYATTACHED
					ptr2 = com[msgrouting[DISPLAY_UNIT]].txbuff+DATA_START;
					*((gpstype *) ptr2) = *gps;
					// tell the display unit
					SendMessage((u08) SENSOR_UNIT, (u08) DISPLAY_UNIT, (u08) GPS_DATA, sizeof(gpstype));
#endif
				} // if

			} // if superpacket
			break;
		default:
			break;
	} // switch
} // Parse_Tsip