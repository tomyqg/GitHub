// This is the main program for displayunit.c
// This program runs the display for the Traqmate Product
//
// Change Log:
// Jan 2004 - created, GAStephens
// 7-15-2004 - converted to display unit
// 8-17-2004 - fixed flashpage overwrite
// 9-6-2004 - implemented menu system
// 9-16-2004 - implemented dual UART messaging
// 12-01-2004 - converted to 132 microprocessor
// 01-18-2005 - converted to new memory format (5)
// 01-27-2005 - final first release menu structure
// 11-25-2005 - added I/O
// 2006,2007 - added menu functions, autocross mode
// 2007,2008 - added predictive lap timing, segement timing mode
// 2009,2010 - added support for chasecam and lanc devices
//
// This version runs on the display circuit board with Dataflash
// This code is not portable. It depends on 8052 byte ordering MSB..LSB.
//
// Dependent files:
// display.h			contains all DEFINES, typedefs, and global variable declarations
// tmtypes.h			contains all DEFINES, typedefs, and global variable declarations
// message.h
// opcode.h
// cygfx120.c			contains 8051 setup and operations functions
// dataflash.c			contains functions for ATMEL dataflash serial flash memory
// message.c			implements messaging packet protocol
// session.c			data collection session control and indexing
// timer.c				display timer stuff
// crc16table.c			polynomial definitions for crc-16
// formatgps.c			formats a gps position in text
// menu.c				contains the menu system
// fonts.c				has fonts and icons
// cygnalflash.c		read/write the microprocessor scratchpad memory
// crc.c				calculates crc
// fdl.c				allows self-reprogramming
// factorytest.c		internal routines for factory testing
// roadblock.c			prevents runaway code
// copyright.c			copyright notice
//
// for V2.20 and later, compile with Level 9 Optimazation, except for fdl.c, dataflash.c, and cygfx120.c

#define DU_MAIN

// Copyright (C) 2005-2013 Track Systems Technologies, LLC

#include <stdlib.h>
#include <display.h>
#include <fonts.c>
#include <crc16table.c>

void Swallow_Message( void );

sbit TX1OUT = P1 ^ 0;

void main (	void ) {
	xdata BOOL collecting = FALSE;			// TRUE when collecting data
	xdata int i;							// general purpose small loop counter
	xdata char tempstring[21];
	xdata BOOL firstpulse = TRUE;			// used to get synched up
	xdata BOOL menuinitialized = FALSE;			// for camera not ready screen

	SFRPAGE = CONFIG_PAGE;

	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init_DU();						// enable ports

	// create com ports
	com[0].use = com[1].use = MESSAGING;	// on DU both ports used for messaging
	com[0].port = 0;
	com[1].port = 1;
	com[0].txbytes = com[1].txbytes = 0;	// initialize
	com[0].rxnextidx = com[1].rxnextidx = 0;	// initialize
	com[0].parity = com[1].parity = 'N';	// parity = NONE
	com[0].rxptr = com[0].txptr =			// initialize semaphores
	com[1].rxptr = com[1].txptr = NULL;
	com[0].rxbuff = msgrxbuff[0];			// point to buffers
	com[1].rxbuff = msgrxbuff[1];
	com[0].txbuff = msgtxbuff[0];
	com[1].txbuff = msgtxbuff[1];

	// initialize com ports
	UART_Init(&(com[0]), DUMSGBAUD);			// su port
	UART_Init(&(com[1]), PCUSBMSGBAUD);			// pc port (USB)
	Timer3_Init ((u16) (SYSCLK/SAMPLE_RATE/12));	// initialize Timer3 to overflow at sample rate
	SPIO_Init();						// set up the SPI

	DataFlash_Init(&numpages, &pagesize);	// set up the DataFlash
	Repair_Sessions();					// fix any power loss problems
	Init_Tables();						// initialize driver tables

	EA = 1;								// enable global interrupts

	Display_Init();

	// turn on display backlight
	BACKLITEDS = 1;

	// announce our arrival
	Clear_Screen();

	if (LITE_SW)
		// V2.00 slide from right
//		for (i=120; i>=4; i--) {
//			// Traqmate Logo
//			Write_Icon(i-1, 9, 113, 15, (u08 *) traqmateicon, '-');
	
//			DELAY_MS(3);
//		} // for

//	Clear_Screen();
	Write_Icon(3, 9, 113, 15, (u08 *) traqmateicon, '-');

	if (LITE_SW)
		DELAY_MS(1600);

//	Draw_Rect(2, 6, 118, 26, '^');

	// try 3 times to reset the sensor unit
	for (i=0; i<3; i++) {
		// Reset the sensor unit
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = WAITING;
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) MODE_CHANGE, 1);

		Swallow_Message();

		if (su_present) break;
	} // for

	if (su_present) {
		// Get config data from SU
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) READ_UNIT_INFO, 0);

		Swallow_Message();
	}  // if

	// remember su's sw, hw, and data revs before it goes away
	su_sw = scratch.calibrate.swrev;
	su_data = scratch.calibrate.datarev;
	su_hw = scratch.calibrate.hwrev;
	su_week = scratch.calibrate.week;
	su_year = scratch.calibrate.year;

	// if backlight switch pressed, go faster
//	if (LITE_SW)
//		DELAY_MS(700);

	Clear_Screen();

	// check for incompatibility between SU and DU data formats
	if (!InitDUScratchpad(su_present)) {
		Write_Big_Line(0, "SW MISMATCH", FALSE, '-');
		Write_Big_Line(2, " Reload SW", FALSE, '-');
		DELAY_MS(5000);
	}
	Clear_Screen();

	Write_Big_Line(0, "DRIVE SAFELY", FALSE, '-');
	sprintf(tempstring, "DU1 SW %d.%02d BD %02d/%02d", ((int) SWREV)/100, ((int) SWREV) % 100, (int) (1.0+(((float)(scratch.calibrate.week-1)/4.3))), (int) scratch.calibrate.year);
	Write_Line(2, tempstring, FALSE, '-');
	if (su_present) {
		sprintf(tempstring, "SU%d SW %d.%02d BD %02d/%02d", (int) su_hw/100, su_sw/100, su_sw % 100, (int) (1.0+(((float)(su_week-1)/4.3))), (int) su_year);
		Write_Line(3, tempstring, FALSE, '-');
	} // if
	else
		Write_Line(3, "* SU NOT CONNECTED *", FALSE, '-');

	if (LITE_SW)
		DELAY_MS(2500);

	Clear_Screen();
	Write_Line(0, "This unit belongs to", FALSE, '-');
	Write_Line(2, scratch.calibrate.owner, FALSE, '-');
	Write_Line(3, scratch.calibrate.phone, FALSE, '-');

	// Set up input scaling
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);
	
	// is sensor unit attached and camera enabled?
	waitforcam = (su_present && (0x03 == (flashpage.io.digouts[0] & 0x0F)));

	if (su_present) {
		// put in the write address
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = 0;
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = IODEFPAGE & 0xFF;
	
		// copy segment into output buffer
		for (i=0; i < MAX_PAGE_SIZE; i++)
			com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2+i] = flashpage.bigbuff[i];
	
		// send io defs to SU for use in digital outputs
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) WRITE_SEGMENT, MSGDATA);

		Swallow_Message();

		if (su_hw >= 200) {	// SU 2
			// Send AutoOn status to SU
			com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = AUTOONENABLED;
			SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) AUTO_ON, 1);
	
			Swallow_Message();
		} // if
	} // if

	for (i=0; i<NUMANALOGS; i++)
		analogscaling[i] = flashpage.io.analogs[i];
	for (i=0; i<NUMTDDIGITALS; i++)
		digitalscaling[i] = flashpage.io.digitals[i+4];

	// hold user id screen for a moment
	if (LITE_SW)
		DELAY_MS(1700);
	else	// button held, invert BACKLIGHT so it will stay on when button released
		BACKLITEDS = 0;	

	Clear_Screen();

	// initialize memory pointers
	Init_Session(&pagecnt, &iopagecnt);

	// set up the sampling rate
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	switch (flashpage.user.modesample & 0x0F) {
		case 2:
			samps_per_sec = 20;
			break;
		case 4:
			samps_per_sec = 40;
			break;
		case 0:
		default:
			samps_per_sec = 10;
	} // switch

	// set up the I/O collection
	iocollect = flashpage.user.iodata;

	// clear the io buffer
	DataFlash_Buffer_Clear(IOBUFFER, pagesize);
	// if 16 bit mode selected, turn on all the frequency lines
	if (iocollect & 0x80)
		iocollect |= 0x03;

	// clear the buffer
	for (i=0; i < MAX_PAGE_SIZE; i++) flashpage.bigbuff[i] = 0;

	// go live
	tm_state = WAITING;

//	camreadytick = clocktick + (160 * 7);	// 6.5 seconds to wait for camera ready
	camreadytick = clocktick + (160 * 15);	// 15 seconds to wait for camera ready
	camlancstatus = UNKNOWNSTATUS;
	
	if (waitforcam) {
		// tell user we are waiting for camera
		Write_Screen((u08 *) waitforcamera, '-');
	} // if
	
	menuinitialized = FALSE;
	
	while (1) {
		xdata displaybutton b;

		// see if we have waited long enough for camera to get ready
		if (waitforcam) {
			// if we find out there is not traqdata/sync device or if timeout expires or if camera goes ready
			if ((CAMLANCNOTCONNECTED == camlancstatus) || (CAMLANCREADY == camlancstatus) || (clocktick >= camreadytick))
				waitforcam = FALSE;
		} // if
		else {
			if (!menuinitialized) {	// waiting for camera is over
				menuinitialized = TRUE;
				
				// start up the menu system
				Menu_Op(INIT, BACK);

				if (su_present) {		// don't try to auto-start if no su connected
					// NOTE: The following depends on the menu structure and must be revised if menus are changed!!
					// check to see if Autostart is set
					switch (scratch.calibrate.menustart) {
						case 0xFF:		// main menu - do nothing
							break;
						case 0x01:		// Laps - Recording
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, SEL);
							break;	
						case 0x02:		// Qual - Recording
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							break;
						case 0x03:		// Drive - Recording
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							break;
						case 0x04:		// Tach - Recording
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							break;
						case 0x05:		// Gauges - Recording
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							break;
						case 0x06:		// GPS Compass - Not Recording
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, SEL);
							break;
						case 0x07:		// Instrument - Not Recording
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);
							break;				
						case 0x08:		// GPS Location - Not Recording
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);	
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, DN);
							Menu_Op(BUTTONPRESS, SEL);			
							break;
					} // switch
				} // if
			} // if

			// now check the buttons
			if (!AUTOONENABLED) {			// disable DU power switch if AutoOn is enabled
				// power off if switch was held more than ON/OFF period
				if (debounce[ONOFF] >= DEBOUNCE_ONOFF) {
					// advance menu
					Menu_Op(BUTTONPRESS, ONOFF);
				} // if
		
				// if button not pressed, reset ONOFF counter
				if (ONOFF_SW)
					debounce[ONOFF] = 0;
			} // if

			// check all the buttons except ONOFF
			for (b = 0; b < ONOFF; b++) {
				// only act if button not pressed
				if ((P5 & (1<<b))) {
	
					// check to see if button was held more than debounce period
					if (debounce[b] >= DEBOUNCE_NORMAL)
						// advance menu
						Menu_Op(BUTTONPRESS, b);
	
					// button no longer pressed so reset counter
					debounce[b] = 0;
				} // if
			} // for
		} // else

		// check for incoming sensor unit message
		if (NULL != com[0].rxptr) {
			xdata u08 *tmpptr = com[0].rxptr;

			// clear the semaphore to signal buffer free
			com[0].rxptr = NULL;

			// process the message
			Parse_Message((u08) 0, tmpptr);
		} // if

		// check for incoming PC message
		if (NULL != com[1].rxptr) {
			xdata u08 *tmpptr = com[1].rxptr;

			// clear the semaphore to signal buffer free
			com[1].rxptr = NULL;

			// process the message
			Parse_Message((u08) 1, tmpptr);
		} // if

		// check to see if we are being reprogrammed
		if (mode_change) {				// check the semaphore
			mode_change = FALSE;

			// check for reprogramming
			if (START_DOWNLOAD == new_mode) {
				tm_state = new_mode;

				// use flash for temporary storage
				pagecnt = 0;
				// use variable session to count the cksum
				session = 0xFF;

				Info_Screen("Loading");

				new_mode = WAITING;
			} // if
			else if (DOWNLOAD_COMPLETE == new_mode) {
				tm_state = new_mode;

				Info_Screen("Storing");

				new_mode = WAITING;
			} // if
			else if (CANCEL_DOWNLOAD == new_mode) {
				tm_state = new_mode;

				Info_Screen("Canceling");

				// wipe out everything and show progress
				DataFlash_Erase_Range(0, LASTLAPPAGE, TRUE);

				// reset the unit
				SFRPAGE = LEGACY_PAGE;

				// pull the reset line to restart everything
				RSTSRC = RSTSRC | 0x01;
			} // if
		} // if
	} // while
} // main

void Swallow_Message( void ) {
	// wait for ACK or timeout from SU
	while (0 != CTS[SENSOR_UNIT-1]);

	// check for incoming sensor unit message
	if (NULL != com[0].rxptr) {
		xdata u08 *tmpptr = com[0].rxptr;

		su_present = TRUE;
		
		// clear the semaphore to signal buffer free
		com[0].rxptr = NULL;

		// process the message
		Parse_Message((u08) 0, tmpptr);
	} // if
} // Swallow_Message