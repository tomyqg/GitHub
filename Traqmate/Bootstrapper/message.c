// Part of traqmate.c
// 3/10/2004
// Author: BLBoyer
// Modified for traqmate: GAStephens, 3/24/2004
// added opcode return to ParseMessage 7/29/04
//
// This file contains the routines to format and send coded messages
// between the Traqmate and Traqview software.
//

#include <ctype.h>
#ifdef SENSOR
#include <sensor.h>
#endif
#ifdef DISPLAY
#include <display.h>
#endif
//jfp
#include <fdl.h>

static u08 current_page = 0;
static u16 buff_in = 0;
static u08 tmp_cnt =0;
//jfp
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
#ifdef DISPLAY
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
			switch (msgptr[OPCODE]) {
			case ACK:

				// if ACK with data then parse the results
				switch (outstanding_msg[msgptr[SOURCE]-1]) {
#ifdef DISPLAY
					case READ_UNIT_INFO:
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
				retval = SendACK(msgptr[SOURCE]);
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			case READ_SEGMENT: {			// get specified segment data
				int segnum;				// segment number to get

				segnum = (msgptr[DATA_START]<<8) + msgptr[DATA_START+1];

				DataFlash_Page_Read(pagesize, com[port].txbuff+DATA_START, segnum);
				retval = SendACK_Data(msgptr[SOURCE], pagesize);  // send the segment data

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // READ_SEGMENT
			case WRITE_SEGMENT: {			// write data to specified segment
				int segnum;

				segnum = (msgptr[DATA_START]<<8) + msgptr[DATA_START+1];

				DataFlash_Page_Write_Erase(1, pagesize, msgptr+DATA_START+2, segnum);

				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // WRITE_SEGMENT
			case READ_UNIT_INFO: {		// gets the factory calibration data

				ReadScratchpad(com[port].txbuff+DATA_START);
				retval = SendACK_Data(msgptr[SOURCE], SCRATCHLEN);  // send the segment data

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // READ_UNIT_INFO
			case WRITE_UNIT_INFO: {		// writes the factory calibration data

				EraseScratchpad();
				WriteScratchpad( msgptr+DATA_START, MIN(msgdatalen, SCRATCHLEN));

				// replenish local copy
				ReadScratchpad(scratch.scratchbuff);

				retval = SendACK(msgptr[SOURCE]);
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // WRITE_UNIT_INFO
#ifndef DISPLAYATTACHED
			case ERASE_SESSION: {			// erases a complete session
				Erase_Session(msgptr[DATA_START]);
				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // ERASE_SESSION
#endif
			case ERASE_ALL:
				// wipe out the indexes
				DataFlash_Erase_Page(INDEXPAGE);
#ifdef DISPLAY
				// wipe out all data and don't show progress
				DataFlash_Erase_Range(FIRSTDATAPAGE, LASTLAPPAGE, FALSE);
#endif
#ifdef SENSOR
				// wipe out all data and show progress
				DataFlash_Erase_Range(FIRSTDATAPAGE, LASTLAPPAGE, TRUE);
#endif

				break;
//JFP
#ifdef SENSOR
			case SW_DOWNLOAD:{
		
				SaveDownload(msgptr, REG_SAVE);

				retval = SendACK(msgptr[SOURCE]);
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				break;
			} //ENTER_DOWNLOAD
			case SW_REPROGRAM:{
				retval = SendACK(msgptr[SOURCE]);
				CopytoCodeSpace(current_page);  	// copy into code space
				break;
			} //ENTER_DOWNLOAD
#endif
//JFP

#ifdef SENSOR
			case MODE_CHANGE: {
				state_timer = TRUE;
				new_mode = (tmstatetype) msgptr[DATA_START];

				if (new_mode == ENTER_DOWNLOAD){
				//erase data flash to store new code image
					// wipe out the indexes
					DataFlash_Erase_Page(INDEXPAGE);

					// wipe out all data
					DataFlash_Erase_Range(FIRSTDATAPAGE, LASTLAPPAGE, TRUE);
				}
				else if (new_mode == DOWNLOAD_COMPLETE) {

					SaveDownload(msgptr, FINAL_SAVE);
					state_timer = TRUE;
					new_mode = (tmstatetype) msgptr[DATA_START];
					CopytoCodeSpace(current_page);
				}
					


				retval = SendACK(msgptr[SOURCE]);
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // MODE_CHANGE
#endif

#ifdef DISPLAY
			case WRITE_DISPLAY_TEXT: {
				Write_Line((u08) 3, msgptr+DATA_START, TRUE, '-');		
				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;
				break;
			} // WRITE_DISPLAY_TEXT
			case GPS_DATA: {
				u08 linenum;

				// ACK the message
				retval = SendACK(msgptr[SOURCE]);

				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				// put data in the structure for writing to flash
				flashpage.secsamp10[secondcnt].gps = *((gpstype *)(msgptr+DATA_START));

				switch (dispstate) {
					case TIMING:
						// see if we have lapped
						Lap_Check(NEW_POSITION, (gpstype *)(msgptr+DATA_START));
						// update the signal strength meter
						Write_Sigstrength(FALSE);
						break;
					case DRIVE:
						if (gpslock) {
							xdata char tempbuf[30];
							for (linenum=1; linenum <= 3; linenum++) {
								// print all the information on the screen
								formatgps(&(flashpage.secsamp10[secondcnt].gps), gcnt, tempbuf, linenum);
								Write_Line(linenum, tempbuf, FALSE, '-');
							} // for
						} // if
						break;
					case COMPASS:
						if (gpslock) {
							// print all the information on the screen
							writecompass(&(flashpage.secsamp10[secondcnt].gps));
						} // if
						break;
					case LAPS:
						if (gpslock >= 4) {
							// ready to mark the start/finish
							Info_Screen("Start/Fin>");
						} // if
						else {
							Info_Screen("GPS Search");
						} // else

						// get a fix on our position
						Lap_Check(INITIALIZE, (gpstype *)(msgptr+DATA_START));
						break;
					case WAITSTART:
						if (gpslock >= 4) {
							// ready to mark the start/finish
							Info_Screen("Find Start");
						} // if
						else {
							Info_Screen("GPS Search");
						} // else

						// watch for start/finish to begin timing
						Lap_Check(INITIALIZE, (gpstype *)(msgptr+DATA_START));
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

				gpslock = msgptr[DATA_START];

				// only do this if we are storing data
				if (dispstate <= WAITSTART && dispstate >= TIMING) {
					if (secondcnt == (SAMP10S_PER_PAGE-1)) {	// buffer filled
						if (firstpulse) {
							firstpulse = FALSE;
						} // if
						else {
							// write buffer to flash
							DataFlash_Page_Write_Erase(1, pagesize, flashpage.bigbuff, pagecnt);
							pagecnt++;
						} // else

						// clear the buffer
						for (i=0; i < MAX_PAGE_SIZE; i++) flashpage.bigbuff[i] = 0;
						secondcnt = 0;

						if (pagecnt >= LAPPAGESTART) { // unit is full
							pagecnt--;
							// cancel the mode as if the BACK button was pressed
							Menu_Op(BUTTONPRESS, BACK);
						}
					} // if
					else {			// buffer not filled so go to new second
						secondcnt++;
					} // else
				} // if
				gcnt = 0;		// either way, start a new second for G data
				break;
			} // GPS_PULSE
			case ACCEL_DATA: {

				retval = SendACK(*(msgptr+SOURCE));
			
				// free up unit for more communications
				CTS[msgptr[SOURCE]-1] = 0;

				// do the lap timer
				if (TIMING == dispstate) {
					// increment timer and update the display
					Lap_Check(INC_TIME, NULL);
					Write_Timer();
				} // if

				if (gcnt == 10) { // didn't get a GPS message so reset and show no lock
					gcnt = 0;
					gpslock = 0;
				}
				else {		// print it out
					// put accel data into buffer
					flashpage.secsamp10[secondcnt].accel[gcnt][0] = msgptr[DATA_START];
					flashpage.secsamp10[secondcnt].accel[gcnt][1] = msgptr[DATA_START+1];
					flashpage.secsamp10[secondcnt].accel[gcnt][2] = msgptr[DATA_START+2];

					if (DRIVE == dispstate) {
						xdata char tempbuf[30];
						u16 tempxy;
						float xg, yg;

						// scale the Gs
						tempxy = (flashpage.secsamp10[secondcnt].accel[gcnt][0] & 0xF0) << 4;
						tempxy += flashpage.secsamp10[secondcnt].accel[gcnt][1];
						xg = (float) tempxy - (float) scratch.calibrate.xzero;
						if (xg < 0.0)
							xg = xg / ((float) scratch.calibrate.xzero - (float) scratch.calibrate.xminusg);
						else 
							xg = xg / ((float) scratch.calibrate.xplusg - (float) scratch.calibrate.xzero);

						tempxy = (flashpage.secsamp10[secondcnt].accel[gcnt][0] & 0x0F) << 8;
						tempxy += flashpage.secsamp10[secondcnt].accel[gcnt][2];
						yg = (float) tempxy - (float) scratch.calibrate.yzero;
						if (yg < 0.0)
							yg = yg / ((float) scratch.calibrate.yzero - (float) scratch.calibrate.yminusg);
						else 
							yg = yg / ((float) scratch.calibrate.yplusg - (float) scratch.calibrate.yzero);

						// print it out
						sprintf(tempbuf, "X %+2.2f Y %+2.2f, %d", xg, yg, gcnt);
						Write_Line((u08) 0, tempbuf, FALSE, '-');
					}
					gcnt++;
				} // else
				break;
			} // ACCEL_DATA
#endif // DISPLAY
			default:
				// grab the port
				CTS[msgptr[SOURCE]-1] = CTS_COUNTDOWN;

				// say what?
				retval = SendNAK(*(msgptr+SOURCE), UNKNOWN_FUNCTION);
				retval = -1; // no valid opcode found - normally send NAK
#ifdef DISPLAY
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

//JFP

// this function will writete donloaded code to data flash
// for storage until the complete image is sent

void SaveDownload(u08 *msgptr, u08 save_type){
		int len, i;


	if (save_type == REG_SAVE){
		i=0;
		len = (msgptr[UPPER_BYTE_COUNT]<<8) | (msgptr[LOWER_BYTE_COUNT]);
		while (i<(len-2)){
			flashpage.bigbuff[buff_in++] = msgptr[DATA_START + i];
			i++;
			if (buff_in == MAX_PAGE_SIZE){
				DataFlash_Page_Write_Erase(1, pagesize, flashpage.bigbuff, current_page);
				buff_in = 0;
				current_page++;
			}
		}
	}
	else{
		// finish up any unsave pages
		DataFlash_Page_Write_Erase(1, pagesize, flashpage.bigbuff, current_page);

	}

}
//JFP