// Part of traqmate.c
// 3/10/2004
// Author: BLBoyer
// Modified for traqmate: GAStephens, 3/24/2004
// added opcode return to ParseMessage 7/29/04
// added sw download: JPaulos, GAStephens 4/28/05
//
// This file contains the routines to format and send coded messages
// between the Traqmate and Traqview software.
//

#ifdef SENSOR
#include <sensor.h>
#endif
#ifdef DISPLAY
#include <display.h>
#endif

void Calibrate( void );
void InputTest( void );
void OutputTest( void );
void DataflashTest( u08 );
void ADTest (void);
void DigIOTest (void);
void AutoOnTest (void);
void SMB_Master_Test (void);
void SMB_Slave_Test (void);
void JumpCopy( u16 );
void Init_Cameras ( void );

#define SendACK_Data(DEST, LEN) SendMessage(msgrouting[0], (u08) DEST, (u08) ACK,(LEN))
#define SendACK(DEST) SendACK_Data(DEST, 0)
// #define Verify_CRC(DAT, COUNT) (((((u16)*(DAT))<<8) + *(DAT+1)) == crc16((DAT), (COUNT)))
#define MSG_SEG	(pagesize + MSG_WRAPPER)

// The following function gets passed all the input data
// necessary to build a complete message. For efficiency it does not
// get passed the data buffer itself.
// In addition to being passed the data, the function will also
// call supporting functions to calculate checksums for the header
// and if appropriate, crc's for the data portion of the message

s16 SendMessage(	u08 source,		// unit id of source
					u08 dest,		// unit id of destination
			 		u08 opcode,		// message opcode
					u16 bytes)	{	// number of bytes to transmit
	u08 chk = 0xFF;					// used to start the header checksum calculation
	u16 i = 0;						// general index
	u16 crc = 0;					// used to hold the calculated CRC
	u08 port = msgrouting[dest];

	// grab the unit
	CTS[dest-1] = CTS_COUNTDOWN;

	// record this message as outstanding if from this device
	if (source == msgrouting[0] && opcode != ACK && opcode != NAK)
		outstanding_msg[dest-1] = opcode;

	com[port].txbuff[START] = SOH ;  // start of header
	com[port].txbuff[SOURCE] = source; // source of the message
	com[port].txbuff[DESTINATION] = dest; // destination of the message (same as above)

	bytes += 2;					// add two bytes to the total for the CRC

	com[port].txbuff[UPPER_BYTE_COUNT] = (bytes & 0xff00)>>8 ;  // format byte count to
	com[port].txbuff[LOWER_BYTE_COUNT] = bytes & 0xff;          // two byte field

	com[port].txbuff[OPCODE]=opcode ;	// store the message op code
	
	for (i=0; i<CHECKSUM;i++)	// calculate the message header checksum
		chk ^= com[port].txbuff[i] ;		// over all bytes from SOH through op code
	
	com[port].txbuff[CHECKSUM] = chk ;	// store the checksum in final header byte

	bytes -= 2;					// remove crc from total

	crc = crc16(&(com[port].txbuff[DATA_START]),bytes);	// calculate the crc

	com[port].txbuff[++bytes+CHECKSUM] = (crc & 0xff00)>>8 ;		// store the high crc byte
	com[port].txbuff[++bytes+CHECKSUM] = (crc & 0xff) ;			// store the low crc byte

	XmitUART(&(com[port]), DATA_START+bytes);						// send it
	return(DATA_START+bytes) ;  // return total number of bytes processed in message

} // SendMessage

// the following routine will parse a message based on the op code
// it is up to the opcode to determine any additional processing, such as
// CRC calculations, or the meaning of the data
s16 Parse_Message(u08 port, u08 *msgptr) 
{
	xdata int retval = 0 ;
	xdata u16 msgdatalen;

	msgdatalen = (msgptr[UPPER_BYTE_COUNT] << 8) + msgptr[LOWER_BYTE_COUNT] - 2;

	retval = Verify_CRC(msgptr+DATA_START, msgdatalen);

	if (retval) {		// bad data

		if (NAK != msgptr[OPCODE] && ACK != msgptr[OPCODE])	{ // don't NAK an ACK or NAK
			// grab the port
			CTS[msgptr[SOURCE]-1] = CTS_COUNTDOWN;

			retval = SendNAK(msgptr[SOURCE], (u08) BAD_CRC);		// no data is returned and no crc
		} // if
#if defined DISPLAY && defined DEBUG
			Write_Line((u08) 3, "BAD CRC", TRUE, '-');
#endif
	} // if
	else {		// message is good so parse it
		int i;
		u08 *tmpfrom;
		u08 *tmpto;

		// if message not for me, then route it
		if (msgptr[DESTINATION] != msgrouting[0] && msgptr[DESTINATION] != ANY_UNIT) {
			// copy message into appropriate outbound buffer

		// copy into output buffer
		tmpto = com[msgrouting[msgptr[DESTINATION]]].txbuff+DATA_START;
		tmpfrom = msgptr+DATA_START;
		for (i=0; i<msgdatalen; i++)
			*tmpto++ = *tmpfrom++;

			SendMessage(msgptr[SOURCE], msgptr[DESTINATION], msgptr[OPCODE], msgdatalen);
		} // if
		else {	// message for me so process it

#if defined SENSOR && defined DISPLAYATTACHED
			if (DISPLAY_UNIT == msgptr[SOURCE])
				// remember that we talked with a DU
				duconnected = TRUE;
#endif
			switch (msgptr[OPCODE]) {
			case ACK:

				// if ACK with data then parse the results
				switch (outstanding_msg[msgptr[SOURCE]-1]) {
#ifdef DISPLAY
					case READ_UNIT_INFO:
						// make sure this is the correct ACK
						if (SCRATCHLEN == msgdatalen)
							// save the config data in RAM
							scratch = *((scratchpadtype *) &(msgptr[DATA_START]));
						break;
#endif
					default:
						break;
				} // switch

				// clear the message as outstanding from this device
				outstanding_msg[msgptr[SOURCE]-1] = 0;

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;

			case NAK:
				// fall back and retry PUT CODE HERE

				// clear the message as outstanding from this device
				outstanding_msg[msgptr[SOURCE-1]] = 0;

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;

		// the SendACK function should be called within each
		// case to ensure the appropriate data is returned for
		// the corresponding op code
			case WHO_ARE_YOU:
				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else
					retval = SendACK(msgptr[SOURCE]);
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			case READ_SEGMENT: {			// get specified segment data
				int segnum;				// segment number to get

				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					segnum = (msgptr[DATA_START]<<8) + msgptr[DATA_START+1];

					DataFlash_Page_Read(pagesize, com[port].txbuff+DATA_START, segnum);
					retval = SendACK_Data(msgptr[SOURCE], pagesize);  // send the segment data
				} // else
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // READ_SEGMENT
			case WRITE_SEGMENT: {			// write data to specified segment
				int segnum;

				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					segnum = (msgptr[DATA_START]<<8) + msgptr[DATA_START+1];

					DataFlash_Page_Write_Erase(GENBUFFER, pagesize, msgptr+DATA_START+2, segnum);

					retval = SendACK(msgptr[SOURCE]);

#if defined SENSOR && defined DISPLAYATTACHED		
					if (IODEFPAGE == segnum)		// likely that camera definition change so reload
						Init_Cameras();
#endif						
				} // else
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // WRITE_SEGMENT
			case READ_UNIT_INFO: {		// gets the factory calibration data
				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					ReadScratchpad(com[port].txbuff+DATA_START);
					retval = SendACK_Data(msgptr[SOURCE], SCRATCHLEN);  // send the segment data
				} // else

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // READ_UNIT_INFO
			case WRITE_UNIT_INFO: {		// writes the factory calibration data
				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					retval = SendACK(msgptr[SOURCE]);
					EraseScratchpad();
					WriteScratchpad( msgptr+DATA_START, MIN(msgdatalen, SCRATCHLEN));

					// replenish local copy
					ReadScratchpad(scratch.scratchbuff);

#if defined SENSOR && !defined DISPLAYATTACHED
					// user settings may have changed so reprogram digipot if available
					if (su2)
						DigiPot(AUTOONENABLED);		// usersettings bit 4
#endif	
				} // else

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // WRITE_UNIT_INFO

#ifndef DISPLAYATTACHED
			case ERASE_SESSION: {			// erases a complete session
				tmstatetype save_state = tm_state;

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					tm_state = ERASING;
					retval = SendACK(msgptr[SOURCE]);
					Erase_Session(msgptr[DATA_START]);
					tm_state = save_state;
				} // else
				break;
			} // ERASE_SESSION
#ifdef PACKMEM
			case PACK_SESSIONS: {
				tmstatetype save_state = tm_state;

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					tm_state = ERASING;
					retval = SendACK(msgptr[SOURCE]);

				// pack them and report progress
				Pack_Sessions(TRUE);

					tm_state = save_state;
				} // else
				break;
			} // PACK_SESSIONS
#endif
#endif
			case ERASE_ALL:
			case ERASE_ALL_SESS: {
				tmstatetype save_state = tm_state;
				// if erase all then start at page 0, else first data page
				u16 starterase = ((ERASE_ALL == msgptr[OPCODE])? 0 : FIRSTDATAPAGE);
				u16 enderase = ((ERASE_ALL == msgptr[OPCODE])? numpages - 1 : LASTLAPPAGE);
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				if (OCCUPIED(tm_state))
					retval = SendNAK(msgptr[SOURCE], (u08) UNIT_BUSY);
				else {
					tm_state = ERASING;
					retval = SendACK(msgptr[SOURCE]);

					// wipe out the indexes
					DataFlash_Erase_Page(INDEXPAGE);
#ifdef DISPLAY
					// wipe out all data and don't show progress
					DataFlash_Erase_Range(starterase, enderase, FALSE);
#endif
#ifdef SENSOR
					// wipe out all data and show progress
					DataFlash_Erase_Range(starterase, enderase, TRUE);
#endif
					tm_state = save_state;
				} // else
				break;
			} // ERASE_ALL_SESS

			case RESET_UNIT: {
				u08 status;
				
				retval = SendACK(*(msgptr+SOURCE));
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				// wait for Serial Flash to be finished
				do {
					status = DataFlash_Read_Status();
				} while (!(status & STATUS_READY_MASK));

				SFRPAGE = LEGACY_PAGE;

				// pull the reset line to restart everything
				RSTSRC = RSTSRC | 0x01;

				break;
			} // RESET_UNIT
			
#ifdef SENSOR
			case SERIAL_PASSTHRU: {	// this can be used with a PC to run Ublox u-Center PC software on an SU
				retval = SendACK(*(msgptr+SOURCE));

				// initialize serial port to match Ublox
				UART_Init(&(com[0]), UBXBAUD);

				// breath a moment
				DELAY_MS(40);

				// shut off serial port interrupts
				SFRPAGE = UART0_PAGE;
				ES0 = 0;							// Disable UART0 interrupts
				RI0 = 0;							// ready to receive
				TI0 = 1;							// ready to transmit

				SFRPAGE = UART1_PAGE;
				EIE2 &= ~0x40;						// ES1=0, Disable UART1 interrupts
				RI1 = 0;							// ready to receive
				TI1 = 1;							// ready to transmit
				
				// loop until button press swapping bytes between ports
				while (1) {
					u08 tempch;

					SFRPAGE = CONFIG_PAGE;
					if (0 != SW1) {
						// see if it has just been released
						if (debounce >= DEBOUNCE_NORMAL) {
							// best way to exit is just to reset the unit
							SFRPAGE = LEGACY_PAGE;
							// pull the reset line to restart everything
							RSTSRC = RSTSRC | 0x01;
						} // if
					} // if

					SFRPAGE = UART0_PAGE;
					if (RI0) {							// byte received on UART0
						SFRPAGE = UART1_PAGE;
						if (TI1) {						// check to see if UART1 is ready
							TI1 = 0;					// clear ready bit
							SFRPAGE = UART0_PAGE;
							tempch = SBUF0;				// get a char
							RI0 = 0;					// clear receive interrupt flag
							LED3 = ~LED3;				// toggle COM LED
							SFRPAGE = UART1_PAGE;
							SBUF1 = tempch;				// transmit a char
						} // if
					} // if

					SFRPAGE = UART1_PAGE;
					if (RI1) {							// byte received on UART1
						SFRPAGE = UART0_PAGE;
						if (TI0) {						// check to see if UART0 is ready
							TI0 = 0;					// clear ready bit
							SFRPAGE = UART1_PAGE;
							tempch = SBUF1;				// get a char
							RI1 = 0;					// clear receive interrupt flag
							LED2 = ~LED2;				// toggle GPS LED
							SFRPAGE = UART0_PAGE;
							SBUF0 = tempch;				// transmit a char
						} // if
					} // if
				} // while
				break;
			} // SERIAL_PASSTHRU
			case SIMULATE_GPS:		// sets or clears GPS Simulate state in SU
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				simgps = msgptr[DATA_START];

				break;	
			case AUTO_ON:		// sets AutoOn bit in SU
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

#ifdef SENSOR
				if (su2) {
					DigiPot(msgptr[DATA_START]);
#ifndef DISPLAYATTACHED
					// read config data
					ReadScratchpad(scratch.scratchbuff);
					// set/clear the autoon bit in the configuration
					if (msgptr[DATA_START])
						scratch.calibrate.usersettings |= AUTOONMASK;
					else
						scratch.calibrate.usersettings &= !AUTOONMASK;
					EraseScratchpad();
					// save it permanently
					WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);
#endif
				} // if
#endif
				break;
			case CALIBRATE:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				Calibrate();
				break;
			case INPUTTEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				InputTest();
				break;
			case OUTPUTTEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				OutputTest();
				break;
			case ADTEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				ADTest();
				break;
			case DIGIOTEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				DigIOTest();
				break;
			case AUTOONTEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				AutoOnTest();
				break;
			case SMBMASTERTEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				SMB_Master_Test();
				break;
			case SMBSLAVETEST:
				retval = SendACK(*(msgptr+SOURCE));
				
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				SMB_Slave_Test();
				break;
#endif
			case DATAFLASHTEST:
			case DATAFLASHTEST2:
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				
				DataflashTest(msgptr[OPCODE]);
				break;
			case POWER_DOWN: {
#ifdef SENSOR
				new_mode = SHUTDOWN;
				mode_change = TRUE;
#else // DISPLAY
				u08 status;
				
				// SU has detected power failure and notified the DU
				// save power
				BACKLITEKB = 0;
				BACKLITEDS = 0;

				// tell user
				Info_Screen("PWR LOST");

				// save data
				if (RECORDING(tm_state))
					End_Session(session, pagecnt, iopagecnt);
				if (TIMING == tm_state ||
					AUTOXHOLDTIME == tm_state ||
					HILLCLIMBSTART == tm_state ||
					HILLCLIMBFINISH == tm_state)
					Lap_Store();

				// wait for Serial Flash to be finished
				do {
					status = DataFlash_Read_Status();
				} while (!(status & STATUS_READY_MASK));

				// go away
				while (1) {
					u16 retry;

					// tell SU to shut us down
					SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) POWER_DOWN, (u08) 0);

					// wait a bit. if still alive, send it again
					for (retry = 0; retry < 40000; retry++) ;
				} // while
#endif

				break;
			} // POWER_DOWN
			// uses global variable session to keep cksum total
			case SW_DOWNLOAD:{
				if (START_DOWNLOAD == tm_state) {
					SaveDownload(msgptr, &session);		// save in dataflash
					retval = SendACK(msgptr[SOURCE]);
					pagecnt++;
				}
				else {
					retval = SendNAK(msgptr[SOURCE], (u08) FUNCTION_FAILED);
				}
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				break;
			} // SW_DOWNLOAD

			case SW_REPROGRAM:{
				// make sure we are in the right mode and cksum of image is good
				if (DOWNLOAD_COMPLETE != tm_state || session != msgptr[DATA_START]) {
					retval = SendNAK(msgptr[SOURCE], (u08) FUNCTION_FAILED);
					// free up unit for more communications
					CTS[msgptr[SOURCE]-1] = 0;

					DELAY_MS(100);				// give NAK chance to arrive

					// abort download, clear flash and reset
					new_mode = CANCEL_DOWNLOAD;
					mode_change = TRUE;
				}
				else {
					retval = SendACK(msgptr[SOURCE]);
					CTS[msgptr[SOURCE]-1] = 0;

					DELAY_MS(100);				// give ACK chance to arrive

					JumpCopy(pagecnt);			// copy into code space
				}
				break;
			} // SW_REPROGRAM

			case MODE_CHANGE: {
				mode_change = TRUE;
				new_mode = (tmstatetype) msgptr[DATA_START];
#if defined SENSOR && defined DISPLAYATTACHED
				if (msgdatalen > 1) {		// parameter change
					// set up sampling rate
					new_samp = msgptr[DATA_START+1] & 0x0F;
					// set up the I/O collection
					new_iocollect = msgptr[DATA_START+2];
					// set up input scaling
					for (i=0; i<NUMANALOGS; i++)
						ioscaling[i] = msgptr[DATA_START+3+i];
					// frequency alarm data for currently selected car
					cyl = msgptr[DATA_START+7];
					revs = * (u16*) (msgptr+DATA_START+8);
				} // if
#endif
				retval = SendACK(msgptr[SOURCE]);
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // MODE_CHANGE

#ifdef DISPLAY
			case WRITE_DISPLAY_TEXT: {
				Write_Line((u08) 3, msgptr+DATA_START, TRUE, '-');		
				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // WRITE_DISPLAY_TEXT
			case CAM_STATUS: {
				// ignore pesky messages during startup
				if (STARTUP != tm_state) {
	
					// first byte tells whether position data is good
					camlancstatus = (camlancstatustype) msgptr[DATA_START];
	
					retval = SendACK(*(msgptr+SOURCE));
									
					// free up unit for more communications
					CTS[msgptr[SOURCE]-1] = 0;
	
					if (WAITING == tm_state) {
						// write PDR status icon
						switch (camlancstatus) {
							case UNKNOWNSTATUS:
							case CAMLANCNOTCONNECTED:
								break;
							case CAMLANCREADY:
								Write_Icon(105, 0, 16, 8, &(camready[0]), '-');
								break;
							case CAMLANCRECORD:
								Write_Icon(105, 0, 16, 8, &(camrecord[0]), '-');
								break;
							default:
								Write_Icon(105, 0, 16, 8, &(camnotready[0]), '-');
						} // switch	
					} // if
				} // if
				break;
			} // CAM_STATUS				
			case GPS_DATA: {
				u08 linenum;

				// ACK the message
				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				// first byte tells whether position data is good
				gpslock = msgptr[DATA_START];

				// store the gps position data into the buffer
				// done with or without gpslock because su will interpolate
				flashpage.secsamp10[secondcnt].gps = *((gpstype *)(msgptr+DATA_START+1));

				if (gpslock) {
					// save first good date as starting date
					if (gpslock && 0xFFFFFFFF == sessstarttime &&
						0L != flashpage.secsamp10[secondcnt].gps.time &&
						0xFFFFFFFF != flashpage.secsamp10[secondcnt].gps.time)
							sessstarttime = flashpage.secsamp10[secondcnt].gps.time;

					// keep last good date as ending date
					if (gpslock && 0L != flashpage.secsamp10[secondcnt].gps.time &&
						0xFFFFFFFF != flashpage.secsamp10[secondcnt].gps.time)
							sessendtime = flashpage.secsamp10[secondcnt].gps.time;

					// save first good week as starting week
					if (gpslock && 0xFFFF == sessstartweeks &&
						0 != flashpage.secsamp10[secondcnt].gps.weeks &&
						0xFFFF != flashpage.secsamp10[secondcnt].gps.weeks)
							sessstartweeks = flashpage.secsamp10[secondcnt].gps.weeks;

					// keep last good week as ending weeks
					if (gpslock && 0 != flashpage.secsamp10[secondcnt].gps.weeks &&
						0xFFFF != flashpage.secsamp10[secondcnt].gps.weeks)
							sessendweeks = flashpage.secsamp10[secondcnt].gps.weeks;
				} // if

				switch (tm_state) {
					case TIMING:
					case HILLCLIMBSTART:
					case HILLCLIMBFINISH:
					case HILLCLIMBGETFINISH:
					case AUTOXGETFINISH:
					case AUTOXFINISH:
					case DRAGFINISH:
						// see if we have lapped
						Lap_Check(NEW_POSITION, (gpstype *)(msgptr+DATA_START+1));
						break;
					case GPSINFO:
					case GPSREC:
						if (gpslock >= GPS_GOOD) {
							xdata char tempbuf[CHARSSMALL+2];
							for (linenum=1; linenum <= 3; linenum++) {
								// print all the information on the screen
								formatgps((gpstype *)(msgptr+DATA_START+1), gcnt, tempbuf, linenum);
								Write_Line(linenum, tempbuf, FALSE, '-');
							} // for
						} // if
						break;
					case COMPASS:
					case DRIVE:
						if (gpslock >= GPS_GOOD) {
							Clear_Screen();
							// put up a line bar
							Write_Icon(0, 0, NUMBER_OF_COLUMNS, 7, &(menubar[0]), '-');
							// write menu centered name
							Write_Text(0, 45, "Drive", '-');
							// write undertitles
							if (scratch.calibrate.usersettings & 0x01) // english
								Write_Line(3, "HDG     TIME     MPH", FALSE, '-');
							else
								Write_Line(3, "HDG     TIME     KPH", FALSE, '-');
							// print all the information on the screen
							writecompass((gpstype *)(msgptr+DATA_START+1));
						} // if
						else {
							Write_Screen((u08 *) findgps, '-');
						} // else
						break;
					case GAUGEA0:
					case GAUGEA1:
					case GAUGEA2:
					case GAUGEA3:
					case GAUGED4:
					case GAUGED5:
						formatgauges(tm_state);
						break;
					case GPSRPM:
					case INSTRUMENT:
						writerpm((gpstype *)(msgptr+DATA_START+1));
						break;
					case INPUTS:
					case CHKINPUTS:
						// print all the information on the screen
						writeinputs();
						break;
					case LAPS: // lap and qualifying mode
					case HILLCLIMB:
						if (gpslock >= GPS_GOOD) {
							// ready to mark the start/finish
							Write_Screen((u08 *) startfin, '-');
							if (HILLCLIMB == tm_state) {	// blank out "/ FINISH"
								Write_Icon(40, 19, 45, 10, (u08 *) blankicon, '-');
							} // if						
							// get a fix on our position
							Lap_Check(INITIALIZE, (gpstype *)(msgptr+DATA_START+1));
						} // if
						else {
							Write_Screen((u08 *) findgps, '-');
						} // else
						break;
					case AUTOXSTAGE:
						if (gpslock >= GPS_GOOD) {
							// prompt user to stage
							Write_Screen((u08 *) autoxstage, '-');
						} // if
						else
							Write_Screen((u08 *) findgps, '-');

						// get ready to begin timing
						Lap_Check(INITIALIZE, (gpstype *)(msgptr+DATA_START+1));

						break;
					case DRAGSTAGE:
						if (gpslock >= GPS_GOOD) {
							// prompt user to stage
							Write_Screen((u08 *) dragstage, '-');
						} // if
						else
							Write_Screen((u08 *) findgps, '-');

						// get ready to begin timing
						Lap_Check(INITIALIZE, (gpstype *)(msgptr+DATA_START+1));
	
						break;
					case HILLCLIMBWAITSTART:
					case WAITSTART:
						if (gpslock >= GPS_GOOD) {
							// ready to look for the start/finish
							Write_Screen((u08 *) findstart, '-');
							// put up track name
							Write_Text(3, 1, menunames.track[lap.tracknum], '-');

							// watch for start/finish to begin timing
							Lap_Check(INITIALIZE, (gpstype *)(msgptr+DATA_START+1));
						} // if
						else {
							Write_Screen((u08 *) findgps, '-');
						} // else
 						break;
					default:
						break;
				} // switch

				// exit display scroll mode
				Reset_Top_Line();

				break;
			} // GPS_DATA
			case GPS_PULSE: {
				int i;

				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				// record the dillution of precision number
				dop = ((u16) msgptr[DATA_START+1] << 8) + (u16) (msgptr[DATA_START+2] & 0x00FF);

				// first byte has camera status
				camlancstatus = (camlancstatustype) (msgptr[DATA_START+3]);
					
				// don't save any data until we see a GPS pulse so we can
				// synchronize position and io data with accelerometer data
				if (firstpulse) {
					firstpulse = FALSE;
					// synched up so start with fresh counters
					gcnt = 0;
					secondcnt = 0;
				}
				// only do this if we are storing data
				else if (RECORDING(tm_state)) {
					// fill in any unprogrammed accel samples with previous values
					for (; gcnt < SAMP10S_PER_PAGE; gcnt++) {
						if (gcnt == 0) {
							flashpage.secsamp10[secondcnt].accel[gcnt][0] =
								((scratch.calibrate.xzero & 0x0F00) >> 4) +
								((scratch.calibrate.yzero & 0x0F00) >> 8);
							flashpage.secsamp10[secondcnt].accel[gcnt][1] =
								scratch.calibrate.xzero & 0xFF;
							flashpage.secsamp10[secondcnt].accel[gcnt][2] =
								scratch.calibrate.yzero & 0xFF;
						} // if
						else {
							flashpage.secsamp10[secondcnt].accel[gcnt][0] =
								flashpage.secsamp10[secondcnt].accel[gcnt-1][0];
							flashpage.secsamp10[secondcnt].accel[gcnt][1] =
								flashpage.secsamp10[secondcnt].accel[gcnt-1][1];
							flashpage.secsamp10[secondcnt].accel[gcnt][2] =
								flashpage.secsamp10[secondcnt].accel[gcnt-1][2];
						} // else
					} // for

					if (secondcnt >= (SAMP10S_PER_PAGE-1)) {	// buffer filled
						// write buffer to flash
						DataFlash_Page_Write_Erase(GENBUFFER, pagesize, flashpage.bigbuff, pagecnt);
						pagecnt++;

						// clear the buffer
						for (i=0; i < MAX_PAGE_SIZE; i++) flashpage.bigbuff[i] = 0;
						secondcnt = 0;

						if (pagecnt >= iopagecnt-1) { // unit is full
							pagecnt--;
							// cancel the mode as if the BACK button was pressed
							Menu_Op(BUTTONPRESS, BACK);
						}
					} // if
					else {			// buffer not filled so go to new second
						secondcnt++;
					} // else
				} // else
				gcnt = 0;		// either way, start a new second for G data
				break;
			} // GPS_PULSE
			case ACCEL_DATA: {
				s08 checkbits;			// used to check off the enable bits for data i/o
				float xg, yg;
				xdata u16 tempxy;
				xdata char xchar, ychar;

				retval = SendACK(*(msgptr+SOURCE));
			
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				if (gcnt >= 10)
					gcnt = 9;	// error condition

				/// put accel data into buffer
				flashpage.secsamp10[secondcnt].accel[gcnt][0] = msgptr[DATA_START];
				flashpage.secsamp10[secondcnt].accel[gcnt][1] = msgptr[DATA_START+1];
				flashpage.secsamp10[secondcnt].accel[gcnt][2] = msgptr[DATA_START+2];

				// scale the Y Gs
				tempxy = (flashpage.secsamp10[secondcnt].accel[gcnt][0] & 0x0F) << 8;
				tempxy += flashpage.secsamp10[secondcnt].accel[gcnt][2];
				yg = (float) tempxy - (float) scratch.calibrate.yzero;
				if (yg < 0.0) {
					ychar = 'L';
					yg = yg / ((float) scratch.calibrate.yzero - (float) scratch.calibrate.yminusg);
				} // if
				else  {
					ychar = 'R';
					yg = yg / ((float) scratch.calibrate.yplusg - (float) scratch.calibrate.yzero);
				} // else

				// scale the X Gs
				tempxy = (flashpage.secsamp10[secondcnt].accel[gcnt][0] & 0xF0) << 4;
				tempxy += flashpage.secsamp10[secondcnt].accel[gcnt][1];
				xg = (float) tempxy - (float) scratch.calibrate.xzero;

				if (xg < 0.0) {
					xchar = 'A';
					xg = xg / ((float) scratch.calibrate.xzero - (float) scratch.calibrate.xminusg);
				} // if
				else {
					xchar = 'B';
					xg = xg / ((float) scratch.calibrate.xplusg - (float) scratch.calibrate.xzero);
				} // else

				switch (tm_state) {
					xdata char tempbuf[CHARSSMALL+2];
					
					case TIMING:			// do the lap timer
					case HILLCLIMBFINISH:
					case HILLCLIMBGETFINISH:
					case AUTOXGETFINISH:
					case AUTOXFINISH:
					case DRAGFINISH:
						// only do this if launch has occurred and G is acceleration (negative)
						if (0 != dragnumbers.launchg && xg < 0) {
							xdata u16 tempval = (u16) (-xg * 100.0);
							if (tempval > dragnumbers.launchg)
								dragnumbers.launchg = tempval;
						} // if
						// increment timer and update the display
						Lap_Check(INC_TIME, NULL);
						Write_Timer();
						break;
					case DRAGREADY:
#define LIGHTSPACING	2					// seconds between lights
#define MAXRATE			40					// autox and drag sampling rate = 40hz

						draglighttimer++;			// increment drag light timer by sampling rate
						switch (draglighttimer) {	// normalized to 40hz
							case (MAXRATE * LIGHTSPACING):		// turn on 1st light
								Write_Icon(101, 4, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(112, 4, 4, 6, (u08 *) stoplight, '^');								
								break;
							case (2 * MAXRATE * LIGHTSPACING):		// turn off 1st light, turn on 2nd light
								Write_Icon(101, 4, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(112, 4, 4, 6, (u08 *) stoplight, '^');	
								Write_Icon(101, 13, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(112, 13, 4, 6, (u08 *) stoplight, '^');								
								break;
							case (3 * MAXRATE * LIGHTSPACING):	// turn off 2nd light, turn on 3rd light and GO
								Write_Icon(101, 13, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(112, 13, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(101, 22, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(112, 22, 4, 6, (u08 *) stoplight, '^');
								Write_Icon(0, 0, 59, 32, (u08 *) go, '-');
								break;					
						} // switch
						
						// fall thru
					case AUTOXREADY:	// look for launch
						if (xg < LAUNCHVAL) {
							dragnumbers.reaction = (((s16) draglighttimer - (s16) (3 * LIGHTSPACING * samps_per_sec)) * 100) / samps_per_sec;
							dragnumbers.launchg = (u16) (-xg * 100.0);
							gcnt = 0;		// starting with data now
							Lap_Check(START_POSITION, NULL);
						} // if
						break;
#undef MAXRATE
#undef LIGHTSPACING
					case GPSINFO:
					case GPSREC:
						// print it out
						sprintf(tempbuf, "X %+2.2f Y %+2.2f", xg, yg);

						if (gpslock)
							sprintf(tempbuf + strlen(tempbuf), "%5d ", dop);
						else
							sprintf(tempbuf + strlen(tempbuf), " ---- ", dop);

						Write_Line((u08) 0, tempbuf, FALSE, '-');
						break;
					case GFORCES:
						// do every other sample
//						if (gcnt & 0x01) {
							if (xg < 0.0) xg = -xg;
	 						if (yg < 0.0) yg = -yg;
	
							sprintf(tempbuf, "%c:%1.1f ", xchar, xg);
							Write_Big_Text(2, 3, tempbuf, '-');
							sprintf(tempbuf, "%c:%1.1f", ychar, yg);
							// put value on display
							Write_Big_Text(2, 69, tempbuf, '-');
//						} // if
						break;
				} // switch

				// make copy of inputs for display on DU
				for (checkbits = 0; checkbits < NUMINPUTBYTES; checkbits++)
					allinputs[checkbits] = msgptr[DATA_START+3+checkbits];

				// check if Data Interface attached - digital byte inverted at this point
				if ((traqdataconnected = (~msgptr[DATA_START+3] & TACHORDATA)) && RECORDING(tm_state)) {
					// only save i/o data if we have gotten good gps signal
					if (!firstpulse) {
						// save the i/o points in the order they arrive, MSB - LSB, DIA3A2A1A0F1F0
						for (checkbits = 6; checkbits >= 0; checkbits--) {
							// check for bit set enabling i/o point
							if ((iocollect >> checkbits) & 0x01) {
								// write the character
								DataFlash_Buffer_Write(IOBUFFER, 1, iobyte, (msgptr+DATA_START+9-checkbits));						

								// check for io buffer full
								if (iobyte != 0)
									iobyte--;	// point to the next byte down in the dataflash buffer
								else {
									// check to see if entire dataflash full
									if (pagecnt >= iopagecnt-1) { // unit is full, write performed in menu
										// cancel the mode as if the BACK button was pressed
										Menu_Op(BUTTONPRESS, BACK);
									}
									else {
										// write the buffer
										DataFlash_Page_Write_From_Buffer_Erase(IOBUFFER, iopagecnt);

										// go to next page
										iopagecnt--;
										iobyte = MAX_PAGE_SIZE - 1;		// point to MSB
										// clear the buffer
										DataFlash_Buffer_Clear(IOBUFFER, pagesize);
									} // else
								} // else
							} // if
						} // for
					} // if
 				} // if
				gcnt++;
				break;
			} // ACCEL_DATA
#endif // DISPLAY
			default:
				// grab the port
				CTS[msgptr[SOURCE]-1] = CTS_COUNTDOWN;

				// say what?
				retval = SendNAK(*(msgptr+SOURCE), UNKNOWN_FUNCTION);
				retval = -1; // no valid opcode found - normally send NAK
#if defined DISPLAY && defined DEBUG
				Write_Line((u08) 3, "NAK UNKNOWN OP", TRUE, '-');
#endif
				break;		
			} // switch
		} // else
	} // else

	return(retval);  // this should be the return value of sending ACK or ACK with
	                 // data, assuming the opcode processes successfully
}

// The following function calculates the crc16 result and verifies it against*/
// the received value in the data.  The function returns:    
// 0 if the crc compares
// -1 if the crc does not match

s16 Verify_CRC(u08 * dat, u16 bytecount) {
	u16 retval, data_crc ;

	retval = crc16(dat, bytecount); // calculate the crc on the received data

	data_crc = (dat[bytecount])<<8 ;  // get the crc from the message
	data_crc += (dat[bytecount+1]);

	if (retval == data_crc)  // check calculated against received
		return(0) ;  // if the crc's match
	else
		return(-1); // -1 otherwise
}

s16 SendNAK(u08 dest,		// unit id of destination
			u08 reason) {
	com[msgrouting[dest]].txbuff[DATA_START] = reason ;
	SendMessage(msgrouting[0], dest,NAK,1);

	return(0) ;

}

// this function will write the downloaded code to data flash in 512 byte segments
// for storage until the complete image is sent
// it also calculates the checksum for later verification

void SaveDownload(u08 *msgptr, u08 *cksum) {
	int i= 0;
	int len;

	len = (msgptr[UPPER_BYTE_COUNT]<<8) | (msgptr[LOWER_BYTE_COUNT]);
	// copy into flash buffer
	for (i=0; i < MIN(len-2, CODE_DOWNLOAD_SIZE); i++) {
		*cksum ^= (flashpage.bigbuff[i] = msgptr[DATA_START + i]);
	}

	// write using alternate flash buffers
	DataFlash_Page_Write_Erase((pagecnt % 2), pagesize, flashpage.bigbuff, pagecnt);

} // SaveDownload

// this function will skip over the roadblock and get to the real copytocodespace
void JumpCopy(u16 lastpage) {
	xdata void (*f)( void);
	xdata u16 tmpint;

	tmpint = * (u16 *) CopytoCodeSpace;

	// check for roadblock at address pointed to by CopytoCodeSpace (usually 0xFE00)
	if (0x80FE == tmpint) {
		// start bootloader at FE02 instead of FE00
		f = (void *) 0xFE02;
		(*f)();
	} // if
	else	
		CopytoCodeSpace(pagecnt);  	// copy into code space
} // JumpCopy


