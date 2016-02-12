// This is the main program for sensorunit.c, part of Traqmate
//
// Change Log:
// Jan 2004 - created, GAStephens
// 5/25/2004 - added differential, better flashing during collection, fixed bugs
// 6/3/2004 - added foolproof mode, fixed bugs
// 7/18/2004 - converted to sensorunit.c, split off common files with displayunit.c
// 8/18/2004 - eliminated print mode, streamlined crc calcs
// 11/16/2004 - added conditional compile for UBX and created ParseTsip and ParseUBX
// 12-14-2004 - converted to 132 microprocessor
// 2-4-2005 - reworked timing to reset adc sampling every gps pulse
// 3-24-2005 - added power up and power down code
// 4-11-2005 - added 20Hz operation
// 11-20-2005 - added I/O
// 12-04-2006 - fixed I/O synchronization
// 2-15-2007 - added digital outputs
// 3-8-2008 - added autostart, button disables, advanced chasecam control
// 2-1-2009 - added support for ublox lea5 gps chip and 3 axis accelerometer
//
// This version runs on the main circuit board with Dataflash
// This code is not portable. It depends on 8052 byte ordering MSB..LSB.
//
// Dependent files:
// sensor.h				contains DEFINES, typedefs, and global variable declarations
// tmtypes.h			contains DEFINES, typedefs, and global variable declarations
// tsip.h				Trimble binary protocol definitions
// ubx.h				Ublox binary protocol definitions
// crc.h				crc calculations
// session.h			data collection session definitions
// crc16table.c			crc lookup table
// message.h			inter-unit messaging
// opcode.h				messaging opcodes
// cygfx120.c			contains 8051 setup and operations functions
// adc.c				contains A-D converter functions
// dataflash.c			contains functions for ATMEL dataflash serial flash memory
// tsip.c				contains functions to decode / encode TSIP commands
// ubx.c				contains functions to decode / encode UBX commands
// message.c			implements messaging packet protocol
// session.c			data collection session control and indexing
// fdl.c				allows self reprogramming
// suports.c			sets up i/o ports
// smbus.c				drivers for smbus (i2c expansion bus)
// lancsetup.c			does handshake with traqdata lanc interface
// digipot.c			allows the auto-on function to operate on su2
//

#define SU_MAIN

// copyright notice
code char crnotice[] = "Copyright (C) 2014 Track Systems Technologies, LLC";

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sensor.h>

#include <crc16table.c>
#ifdef TSIP
#include <tsip.h>
#else
#include <ubx.h>
#endif

void Init_Cameras( void );
void Process_Sample_Set( void );
void Stop_Recording( void );
void Setup_Recording ( void );
void Process_IO( void );
void SMB_Slave_Test (void);

xdata u16 firstpulse = SKIPCYCLES;	// used to get synched up

#ifdef DISPLAYATTACHED
#define TRAQMATE_TYPE	1		// complete
#else
#define TRAQMATE_TYPE	0		// basic
#endif

void main (	void ) {
	xdata int retval;					// function call return value
	xdata int i;								// general counter
	u32 onbuttonctr = 0;				// timing loop for releasing on button
	xdata u16 nextcamupdate;			// second count to send camera status update
	xdata BOOL updatestatus = FALSE;
#ifndef DISPLAYATTACHED
	s16 eastvel, northvel;				// used for Basic Autorecord function
	u16 speed;
	float tempfloat;
	xdata speedlockout = 6;				// number of speed samples to skip to allow GPS to settle
#endif
			
	SFRPAGE = CONFIG_PAGE;

	// disable watchdog timer
	WDTCN = 0xde;
	WDTCN = 0xad;

	SYSCLK_Init();						// initialize oscillator
	PORT_Init_SU();						// enable ports

	// seize the 5V Enable line to keep power up
//	EN5V = 1;	// already done in suports

#ifdef DEBUG
	PWRLED = 0;		// use power led for debug signaling
#endif

	RECLED = GPSLED = COMLED = 0;

	SPIO_Init();						// set up the SPI
	DataFlash_Init(&numpages, &pagesize);	// set up the DataFlash - must come before scratchpad init

	// get the configuration data
	InitSUScratchpad();

	samps_per_sec = 20;					// initialize to something valid
	iocollect = 0;

#ifndef DISPLAYATTACHED
	Repair_Sessions();					// fix any power loss problems
	Init_Tables();						// initialize driver and data input tables
#endif

	// init the dead reckoning history
	for (i=0; i<2; i++) {
		previousgps[i].time = previousgps[i].lat = previousgps[i].lon = 0L;
		previousgps[i].weeks = previousgps[i].alt = 0;
		previousgps[i].velpack[0] =
		previousgps[i].velpack[1] =
		previousgps[i].velpack[2] =
		previousgps[i].velpack[3] = 0;
		previousgps[i].temp = 0;
	} // for
	lastgps = 0;

	ADC0_Init();						// init ADC
	Timer3_Init ((u16) (SYSCLK/SAMPLE_RATE/12));	// initialize Timer3 to overflow at sample rate
 	PPS_Init();							// start GPS pulse interrupts
	Freq0_Init();						// set up frequency measurement for RPM

	// create com ports
	com[0].use = MESSAGING;
#ifdef TSIP
	com[1].use = TSIPGPS;
	com[1].parity = 'O';				// parity = ODD
#else
	com[1].use = UBXGPS;
	com[1].parity = 'N';				// parity = NONE
#endif
	com[0].rxbuff = msgrxbuff;
	com[1].rxbuff = gpsrxbuff;
	com[0].txbuff = msgtxbuff;
	com[1].txbuff = gpstxbuff;
	com[0].port = 0;
	com[1].port = 1;
	com[0].parity = 'N';				// parity = NONE
	com[0].txbytes = com[1].txbytes = 0;	// initialize
	com[0].rxfirstidx = com[1].rxfirstidx = 0;		// initialize
	com[0].rxnextidx = com[1].rxnextidx = 0;
	com[0].rxptr = com[0].txptr =			// initialize semaphores
	com[1].rxptr = com[1].txptr = NULL;

	// initialize com ports
#ifdef DISPLAYATTACHED
	UART_Init(&(com[0]), DUMSGBAUD);
#else
//	UART_Init(&(com[0]), PCMSGBAUD);
	UART_Init(&(com[0]), PCUSBMSGBAUD);		// converted to 230K for all comm in V3.70
#endif

#ifdef TSIP
	UART_Init(&(com[1]), TSIPBAUD);
#else
	if (su2)
		UART_Init(&(com[1]), UBXLEABAUD);
	else
		UART_Init(&(com[1]), UBXBAUD);
#endif

	if (su2) {							// if we are capable, turn on the SMbus
		// create smbus port
		smbus.rxbuff = smbusrxbuff;		// point to buffers
		smbus.txbuff = smbustxbuff;
	    smbus.sent_byte_counter = 0;	// counts outgoing bytes
		smbus.rec_byte_counter = 0;		// counts incoming bytes
		smbus.data_ready = FALSE;		// semaphore
		SMB_Init();						// configure and enable SMBus
	} // if

	camlanc.ports = NOCAMLANC;
	// handshake with LANC and setup SMBus accessories
	New_Accessory_Setup();

#ifndef DISPLAYATTACHED
	// wait for buttons release so we don't go into recording
	// eventually give up and move on

	do {} while (0 == SW1 && ++onbuttonctr < 10000000L) ;

	// wait for buttons to settle
	DELAY_MS(100);

	// clear any button count
	debounce = 0;

	// initialize memory pointers
	Init_Session(&pagecnt, &iopagecnt);
#endif

	EA = 1;								// enable global interrupts

// Turns SU2 unit into a full-time SMbus slave test
#ifdef SUSLAVETEST
	SMB_Slave_Test();
#endif

	if (su2) {
		DELAY_MS(60);					// allows Ublox 5 time to cold start
		UBXBaudChange();				// change from 9600 bps to 57kbps
	} // if

	// initialize the Ublox
	UBXInit();
	ubxdone = TRUE;

#ifdef DEBUG
	// made it this far so look normal
	RECLED = 0;
	GPSLED = 0;
	COMLED = 0;
	PWRLED = 1;
#endif

	// init camera stuff
	camlanc.event = NOEVENT;

	nextcamupdate = secondtick + 10;			// send first update in 9. do not make this < 9

	Init_Cameras();

	// clear the main buffer
	for (i=0; i < pagesize; i++)
		flashpage.bigbuff[i] = 0;

#ifndef DISPLAYATTACHED
	// check to see if button is enabled
	if (AUTORECORD) 	// start recording when conditions are right
		tm_state = WAIT_COLLECT;	
#endif

	while (1) {
		// check for dropping power
		if (adcresults && result[2] < POWER_THRESHOLD) {
#ifndef DISPLAYATTACHED
			u08 status;
#endif
			// shut down LEDs to save power
			PWRLED = 0; RECLED = 0; GPSLED = 0; COMLED = 0;

			// shut down the GPS module to save power
			if (su2)
				EN3VGPS = 1;
			else
				EN3VGPS = 0;

#ifdef DISPLAYATTACHED
			// tell the display unit
			SendMessage((u08) SENSOR_UNIT, (u08) DISPLAY_UNIT, (u08) POWER_DOWN, (u08) 0);
#endif
			// wait for 1.5 seconds for message to be received
			DELAY_MS(1500);

#ifndef DISPLAYATTACHED
			Stop_Recording();

			// wait for Serial Flash to be finished
			do {
				status = DataFlash_Read_Status();
			} while (!(status & STATUS_READY_MASK));
#endif
			EN5V = 0;
			// loop until dead
			while (1) ;

		} // if

		// is it time to report camera status?
		if (nextcamupdate == secondtick) {
			updatestatus = TRUE;		// send a camera update regardless of status change
			nextcamupdate++;			// do it again in another second
		} // if

		// manage cameras and report status when not otherwise engaged
		// if usb camera connected and enabled, track status
		if (NOEVENT == camlanc.event) {
			if (ENABLED4USB == camlanc.ports) {
				// if we are recording, report it
				if (1 == DIGOUT4) {
					if (camlanc.status != CAMLANCRECORD) {
						camlanc.status = CAMLANCRECORD;
					} // if
				} // if
				else {
					if (camlanc.status != CAMLANCREADY) {
						camlanc.status = CAMLANCREADY;
					} // if
				} // else
			} // else if ENABLED4USB
			// status of gopro or replay
			else if (ENABLED4GOPRO == camlanc.ports || ENABLED4REPLAY == camlanc.ports) {
				if (DIGIN3) {
					if (camlanc.status != CAMLANCRECORD) {
						camlanc.status = CAMLANCRECORD;
						updatestatus = TRUE;
					} // if
				} // if
				else { // not in record mode
					if (DIGIN2) {
						if (camlanc.status != CAMLANCREADY) {
							camlanc.status = CAMLANCREADY;
							updatestatus = TRUE;
						} // if
					} // if
					else if (camlanc.status != CAMLANCNOTREADY) {
						camlanc.status = CAMLANCNOTREADY;
						updatestatus = TRUE;
					} // else if
				} // else
			} // else
			else if (ENABLED4LANC == camlanc.ports) {
				if (LANCNOTREADY)
					camlanc.status = CAMLANCNOTREADY;
				else {
					if (LANCRECORDING)
						camlanc.status = CAMLANCRECORD;
					else
						camlanc.status = CAMLANCREADY;
				} // else
			} // else if
		} // if

		// camera control, handle pulse on / pulse off for cameras	
		if (clocktick == camlanc.eventtick) {
			switch (camlanc.event) {
				case (PULSEON):
					switch (cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][SIGNALTYPE]) {
						case LEVEL:			// USB camera
							DIGOUT4 = 1;
							camlanc.status = CAMLANCRECORD;
							camlanc.event = NOEVENT;
							break;
						case PULSE:			// pulse style camera control
							DIGOUT4 = 1;
							camlanc.status = CAMLANCONPULSE;
							camlanc.event = PULSEOFF;					
							camlanc.eventtick = clocktick +
								(long) cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][RECONTIME] * (long) SAMPLE_RATE / 1000L;
							break;
						case LANC:
							LANCCMD = LANCRECORD;
							camlanc.event = NOEVENT;
							break;
					} // switch
					break;
				case (PULSEOFF):
					DIGOUT4 = 0;
					camlanc.status = CAMLANCRECORD;
					camlanc.event = LOCKOUT;
					// don't report status during data delay
					camlanc.eventtick = clocktick + 
						(long) cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][DATADELAY] * (long) SAMPLE_RATE / 1000L;
					break;
				case (LOCKOUT):
					DIGOUT4 = 0;
					camlanc.event = NOEVENT;
					break;
				default:	// nothing to do
					break;
			} // switch
		} // if
			
#ifdef DISPLAYATTACHED
		if (duconnected && !RECORDING(tm_state) && !REPROGRAMMING(tm_state) && updatestatus) {
			updatestatus = FALSE;
			// tell the display unit
			if (NOCAMLANC != camlanc.ports) {
				com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START] = (u08) camlanc.status;
				SendMessage((u08) SENSOR_UNIT, (u08) DISPLAY_UNIT, (u08) CAM_STATUS, 1);
			} // if
		} // if
#endif
		
		// check for incoming message
		if (NULL != com[0].rxptr) {
			u08 *tmpptr = com[0].rxptr;

			// clear the semaphore to allow 2nd buffer to fill
			com[0].rxptr = NULL;
			// process the message
			retval = Parse_Message(0, tmpptr);
		} // if

#ifndef DISPLAYATTACHED
		// power off if button enabled and switch was held more than ON/OFF period
		if (!NOPOWEROFF && (debounce >= DEBOUNCE_ONOFF)) {
			u08 status;

			// save power
			PWRLED = 0; RECLED = 0; GPSLED = 0; COMLED = 0;

			// stop and save
			Stop_Recording();

			// wait for Serial Flash to be finished
			do {
				status = DataFlash_Read_Status();
			} while (!(status & STATUS_READY_MASK));

			// turn off camera		
			switch (cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][SIGNALTYPE]) {
				case LEVEL:		// usb camera
					DIGOUT4 = 0;
					break;
				case PULSE:		// pulse style camera
					// wait a bit in case it just turned on
					DELAY_MS(200);
					DIGOUT4 = 1;
					DELAY_MS(cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][RECOFFTIME]);
					DIGOUT4 = 0;
					break;
				case LANC:
					LANCCMD = LANCRECSTOP;				// stop recording on LANC
					break;
			} // switch

			// shut off the power
			EN5V = 0;
			P7 = 0;

			// loop until dead
			while (1) ;
		} // if

		// check for button not pressed
		if (0 != SW1) {
			// see if it has just been released
			if (debounce >= DEBOUNCE_NORMAL) {
				switch (tm_state) {
					case WAITING:
					case WAIT_COLLECT: {
						u16 tempclock;
						if (NORECORDOFF) break; // button is disabled for this function

						// initialize
						Setup_Recording();

						stoprecordtick = scratch.calibrate.turnofftime + secondtick;   // will overflow and wrap

						tempclock = clocktick;
						// clear the io buffer and start the session
						session = Start_Session(&pagecnt, &iopagecnt);
						if (0 == session) { // no sessions available
							RECLED = 0;
							tm_state = WAITING;
						} // if
						else { // got a good session number
							tm_state = COLLECTING;
							RECLED = 1;
						} // else
						break;
					} // case
					case COLLECTING:
						if (NORECORDOFF) break;	// button is disabled for this function
 						Stop_Recording();
					// fall through
					case ERASING:
						if (AUTORECORD)
							tm_state = WAIT_COLLECT;
						else
							tm_state = WAITING;
						// turn off gps signal LED
						GPSLED = 0;
						
						break;
				} // switch
			} // if
			// reset debounce counter, regardless
			debounce = 0;
		} // if
#endif
		if (mode_change) {				// check the semaphore
#ifndef DISPLAYATTACHED
//			u08 status;
#endif
			mode_change = FALSE;

			// check for shutdown
			if (SHUTDOWN == new_mode) {

				// DU ON/OFF button has been pressed or
				// DU notified of power failure, shut down, and telling SU to shut down

				if (ENABLED4LANC == camlanc.ports)
					LANCCMD = LANCRECSTOP;						// stop recording on LANC device
				else if (ENABLED4USB == camlanc.ports) {		// stop recording
					DIGOUT4 = 0;
				} // else
				else {
					// turn off -- generic
					if (CAMLANCRECORD == camlanc.status && CAMOFF != camlanc.status) {
						DELAY_MS(2000);
						DIGOUT4 = 1;
						DELAY_MS(cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][RECOFFTIME]);
						DIGOUT4 = 0;
					} // if
				} // else

				// turn off LEDs
				PWRLED = 0; RECLED = 0; GPSLED = 0; COMLED = 0;

				EN5V = 0;
				// loop until dead
// *** TEST gas 2012 what if we just keep operating?				while (1) ;
// *** 2/2013, seems to be fine. recommend leaving comment in place -- gas
			} // if
			// check for reprogramming
			else if (START_DOWNLOAD == new_mode) {
				tm_state = new_mode;

				// use flash for temporary storage
				pagecnt = 0;
				// use variable session to count the cksum
				session = 0xFF;

				// turn on all LEDs to indicate SW download
				PWRLED = 1; RECLED = 1; GPSLED = 1; COMLED = 1;

				// shut down interrupts
				EX0 = 0;							// disable INT0
				EIE2 &= ~0x02;						// disable ADC interrupts
				EIE2 &= ~0x40;						// disable UART1 interrupts

				new_mode = WAITING;
			} // else if
			else if (DOWNLOAD_COMPLETE == new_mode) {
				tm_state = new_mode;

				// turn off all LEDs to indicate SW reprogram
				PWRLED = 0; RECLED = 0;	GPSLED = 0; COMLED = 0;

				new_mode = WAITING;
			} // else if
			else if (CANCEL_DOWNLOAD == new_mode) {
				tm_state = new_mode;

				// turn on all LEDs to indicate something happening
				PWRLED = 1; RECLED = 1; GPSLED = 1; COMLED = 1;

				// wipe out everything and don't show progress
				DataFlash_Erase_Range(0, LASTLAPPAGE, FALSE);

				// reset the unit
				SFRPAGE = LEGACY_PAGE;

				// pull the reset line to restart everything
				RSTSRC = RSTSRC | 0x01;
			} // else if
#ifdef DISPLAYATTACHED
			// if changing into monitoring state, set it up
			else {	// this is structured like this to accommodate changing from non-recording
					// data modes to recording data modes on the fly
				// turn on REC LED if recording
				if (RECORDING(new_mode))
					RECLED = 1;

				// do this every time in case io setup has changed
				if (MONITORING(new_mode))
					Setup_Recording();

				if (MONITORING(tm_state) != MONITORING(new_mode)) {
					if (MONITORING(new_mode)) { // entering reporting mode
//						tm_state = new_mode;

						// timing starts NOW
						ADC0_Init();						// init ADC
						UBXInit();							// init Ublox GPS
						gpspulse = FALSE;					// clear out semaphore
						gps_past_due = clocktick + 1 + 10 * (SAMPLE_RATE / samps_per_sec);
					} // if
					else {		// dropping out of monitoring / recording
//						tm_state = WAITING;
						if (RECORDING(tm_state))
							Stop_Recording();
					} // else
				} // if
				tm_state = new_mode;				
				new_mode = WAITING;
			} // else
#endif // DISPLAYATTACHED
		} // if mode_change

		if (MONITORING(tm_state)) {
#ifdef DISPLAYATTACHED
			// only process events one at a time and if transmission channel available to send
			// proceed with tx only if no receive message available
			// make sure last tx either acked or timed out
			// make sure last tx is complete
			if (NULL == com[0].rxptr && 0 == CTS[DISPLAY_UNIT-1] && NULL == com[0].txptr) {
#endif
				// if accelerometer sample, record or send a message
				if (adcresults) {
					// don't reset the semaphore if power dropped
					if (result[2] > POWER_THRESHOLD)
						adcresults = FALSE;
					else
						adcresults = TRUE;

					if (secondcnt >= SAMP10S_PER_PAGE)
						secondcnt = SAMP10S_PER_PAGE - 1;

					if (su2)
						// put in fake temperature number
//						currenttemp = 0;		// degrees C
						// put in Z acceleration value
						currenttemp = (u08) (result[3] >> 4);
					else {
						// store the temperature in with the GPS data
						// calculated from SiLabs documentation
						currenttemp = (s08) ((((long) result[3] - 5125L) * 1000L) / 9445L);
						// subtract off die temp increase to approximate ambient
						currenttemp -= TEMPERATURE_OFFSET;
					} // else

#ifdef DISPLAYATTACHED
					// pack in the accelerometer data
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START] =
						((result[0] & 0x0F00) >> 4) + ((result[1] & 0x0F00) >> 8);
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+1] =
						flashpage.secsamp10[secondcnt].accel[gcnt][1] +
						result[0] & 0xFF;
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+2] =
						flashpage.secsamp10[secondcnt].accel[gcnt][2] +
						result[1] & 0xFF;
					// add on the io data
 					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+3] = ~P5;	// invert digitals
 					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+4] = result[7] >> ioscaling[3];	// analog 3
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+5] = result[6] >> ioscaling[2];	// analog 2
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+6] = result[5] >> ioscaling[1];	// analog 1
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+7] = result[4] >> ioscaling[0];	// analog 0
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+8] = (u08) ((period0 & 0xFF00) >> 8);	// freq	1
					com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+9] = (u08) (period0 & 0x00FF);	// freq	0

					// tell the display unit
					SendMessage((u08) SENSOR_UNIT, (u08) DISPLAY_UNIT, (u08) ACCEL_DATA, (sizeof(acceltype)+7));

					// turn on alarms if necessary
					Process_IO();

#else	// NO DISPLAYATTACHED
					if (RECORDING(tm_state)){
						// only process data if we have gotten gps
						if (!firstpulse) {
							// ensure no overflow in case of lost GPS signal
							if (gcnt >= 10)
								gcnt = 9;	// error condition
		
							// pack accel data into buffer
							flashpage.secsamp10[secondcnt].accel[gcnt][0] =
								((result[0] & 0x0F00) >> 4) + ((result[1] & 0x0F00) >> 8);
							flashpage.secsamp10[secondcnt].accel[gcnt][1] =
								result[0] & 0xFF;
							flashpage.secsamp10[secondcnt].accel[gcnt][2] =
								result[1] & 0xFF;
		
							// prepare for next accel sample
							gcnt++;
	
							// read and record io points as appropriate
							Process_IO();
						} // if firstpulse
					} // if
#endif	// NO DISPLAYATTACHED
				} // if
				// no adc so check for gps message
				else if (com[1].rxnextidx != com[1].rxfirstidx) { // received a GPS command
					// create a pointer to buffer to be processed
					u08 *tmpptr = com[1].rxbuff + com[1].rxfirstidx*UBXRXSIZE;

					// increment counter to point to next inbound message in queue
					com[1].rxfirstidx = (com[1].rxfirstidx == (GPSRXBUFCNT-1))? 0 : com[1].rxfirstidx + 1;

#ifdef TSIP
					Parse_Tsip(tmpptr, &(flashpage.secsamp10[secondcnt].gps));
#else
					// ensure up to date temperature
					flashpage.secsamp10[secondcnt].gps.temp = currenttemp;
					// if this is a good gps then put it in the flashpage
					Parse_UBX(tmpptr, &(flashpage.secsamp10[secondcnt].gps));
#endif

#ifndef DISPLAYATTACHED
					// Do Auto-Start
					// unpack the velocities
					eastvel = (s16) ((((u16) (flashpage.secsamp10[secondcnt].gps.velpack[0] & 0xE0)) << 3) |
						(u16) flashpage.secsamp10[secondcnt].gps.velpack[1]);
					// sign extend
					if (eastvel & 0x0400) eastvel = (u16) eastvel | 0xF800;
			
					northvel = (s16) ((((u16) (flashpage.secsamp10[secondcnt].gps.velpack[0] & 0x1C)) << 6) |
						(u16) flashpage.secsamp10[secondcnt].gps.velpack[2]);
					if (northvel & 0x0400) northvel = (u16) northvel | 0xF800;
			
					// calculate speed in meters/sec plus fudge factor
					tempfloat = GPSVELSCALE * sqrt((float) ((double) eastvel * (double) eastvel + (double) northvel * (double) northvel));
					speed = (u16) (tempfloat * (39.0/12.0/5280.0*60.0*60.0));
			
					// check to see if stop recording conditions are met
					if (COLLECTING == tm_state) {
			
						if (0 != speed) { // vehicle moving so reset counters
							stoprecordtick = scratch.calibrate.turnofftime + secondtick;   // will overflow and wrap
						} // if
						else { // vehicle stopped. let's see for how long
							if (secondtick == stoprecordtick && AUTORECORDSTOP) {
					 			Stop_Recording();
					
								if (AUTORECORD) 	// start recording when conditions are right
									tm_state = WAIT_COLLECT;
								else
									tm_state = WAITING;
							} // if
						} // else
					} // if

					// check to see if auto recording conditions are met (proper speed, good signal)
					if (WAIT_COLLECT == tm_state) {
						if (speedlockout) {		// skip the first few speed samples to allow GPS to settle
							speedlockout--;
						} // if
						else {
						
							if ((gpslock > 5) && (dop < 300) && speed >= (u16) scratch.calibrate.turnonspeed) {
				
								tm_state = COLLECTING; // start recording immediately
				
								// initialize
								Setup_Recording();
				
								// initialize timers for stop recording
								stoprecordtick = scratch.calibrate.turnofftime + secondtick;   // will overflow and wrap
				
								// clear the io buffer and start the session
								session = Start_Session(&pagecnt, &iopagecnt);
								if (0 == session) { // no sessions available
									RECLED = 0;
									tm_state = WAITING;
								} // if
								else { // got a good session number
									tm_state = COLLECTING;
									RECLED = 1;
								} // else
							} // if
						} // else
					} // if
#endif
				} // if got gps message

				// if pulse received or past due, start a new sampling period
				else if (gpspulse || clocktick == gps_past_due) {
					// figure out when next gps pulse is due
					gps_past_due = clocktick + 1 + 10 * (SAMPLE_RATE / samps_per_sec);

					// if we are faking pulse, then no satellites present
					if (!gpspulse) {
						if (simgps)	{
							gpspulse = 1;
							gpslock = 6;
							dop = 200;
						} // if
						else
							gpslock = 0;
						// already one clock tick late so don't creep
						gps_past_due--;
					} // if

#ifndef DISPLAYATTACHED
					if (RECORDING(tm_state)) {
#endif
					// don't save any data until we see a real GPS pulse so we can
					// synchronize position data with accelerometer data
					// synchronize if starting recording but not if just monitoring
					if (firstpulse && (RECORDING(tm_state))) {
						if (gpspulse) {
							if (NOCAMLANC == camlanc.ports || UNKNOWNCONFIG == camlanc.ports) {		// no camera
								camlanc.event = NOEVENT;
							} // if
							else {			// set up timing for camera
								if (camlanc.status == CAMLANCREADY && camlanc.event == NOEVENT) {
									camlanc.event = PULSEON;
									camlanc.eventtick = clocktick + 1;
								} // if	
								
// **** I think we don't need this anymore							
#if 0
								xdata u16 skip;

								// skip is number of GPS samples (1 - 4 per second depending on sample rate) to skip before issuing the camera start 
								skip = (long) cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][CAMDELAY] * (long) samps_per_sec / 10L / 1000L;
							
								if (skip == firstpulse) {		// start the camera when we come back through the camera state machine
									camlanc.event = PULSEON;
									camlanc.status = 
									camlanc.eventtick = clocktick + 1;
								} // if	
#endif
							} // else							
							if (0 == --firstpulse) {
								// synched up so start with fresh counters
								gcnt = 0;
								secondcnt = 0;
#ifndef DISPLAYATTACHED
								iobyte = pagesize - 1;		// point to MSB
#endif
							} // if
						} // if
					} // if
					else { // not first pulse
#ifdef DISPLAYATTACHED
						int i;

//						if (ENABLED4LANC == camlanc.ports)
//							LANCCMD = LANCRECORD;	// set bit to request record	

						// tell the display unit
						com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START] = gpslock;
						com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+1] = (u08) ((dop & 0xFF00) >> 8);
						com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+2] = (u08) (dop & 0x00FF);
						com[msgrouting[DISPLAY_UNIT]].txbuff[DATA_START+3] = (u08) camlanc.status;
						SendMessage((u08) SENSOR_UNIT, (u08) DISPLAY_UNIT, (u08) GPS_PULSE, 4);

						// clear the main buffer
						for (i=0; i < pagesize; i++)
							flashpage.bigbuff[i] = 0;
#else	// no DISPLAYATTACHED
						// fill in any unprogrammed accel samples with previous values
						// *** NOTE - should fill in i/o samples as well to stay in synch
						// has not be a problem so leaving that for future - GAS
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
			
						// got a complete sample set so process it
						Process_Sample_Set();
#endif	// no DISPLAYATTACHED
					} // else
#ifndef DISPLAYATTACHED
					} // if
#endif
					// initialize semaphores
					gpspulse = FALSE;
					ubxfullset = 0;			// resynch Ublox messages

					// flash LED to show we have satellite coverage
					if (gpslock) {
						GPSLED = ~GPSLED;
						gpslock = 0;
					}
					else GPSLED = 0;
				} // if gpspulse

#ifdef DISPLAYATTACHED
			} // if check for transmit
#endif
		} // if monitoring
	} // while
} // main

void Init_Cameras( void ) {
	// Get IO setup
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);

	// break out digital output status
	digout.digoutstate[D4] = flashpage.io.digouts[0] & 0x0F;
	digout.digoutstate[D5] = (flashpage.io.digouts[0] & 0xF0) >> 4;

	// determine what type of interface is connected and which camera is configured
	// see if camera configured (lanc already checked)
	if (ENABLED4LANC != camlanc.ports) {	// skip config if lanc already detected
		if (!(P5 & TACHORDATA) && (ENABLED4LANC != camlanc.ports)) {
			camlanc.ports = NOCAMLANC;
			camlanc.status = CAMLANCNOTCONNECTED;
		} // if
		else {	// some sort of camera interface connected so see if software enabled
			camlanc.status = CAMLANCREADY;			
			switch (digout.digoutstate[D4]) {
				case SONYLANC:
					camlanc.ports = ENABLED4LANC;
					break;					
				case GOPROCAMERA:
					camlanc.ports = ENABLED4GOPRO;
					break;
				case REPLAYCAMERA:
					camlanc.ports = ENABLED4REPLAY;
					break;
				case REPLAYXDUSB:
					camlanc.ports = ENABLED4REPLAYUSB;
					break;
				case REPLAYMINI:
					camlanc.ports = ENABLED4MINI;
					break;
				case REPLAYPRIME:
					camlanc.ports = ENABLED4PRIME;
					break;
				case CAMERA:
				case MOBIUS:
					camlanc.ports = ENABLED4MOBIUS;
					break;
				default:
					camlanc.ports = NOCAMLANC;
					camlanc.status = CAMLANCNOTCONNECTED;
			} // switch
		} // else
	} // if
} // Init_Cameras

#ifndef DISPLAYATTACHED
//
// save the data and go to the next Flash page
//
void Process_Sample_Set( void ) {
	int i;

	// check to see if flash buffer full
	if (secondcnt >= (SAMP10S_PER_PAGE-1)) {	// buffer filled

		// write buffer to flash
		DataFlash_Page_Write_Erase(GENBUFFER, pagesize, flashpage.bigbuff, pagecnt);

		// check to see if we are full
		if (pagecnt >= iopagecnt-1) { // unit is full
			// write the io buffer
			DataFlash_Page_Write_From_Buffer_Erase(IOBUFFER, iopagecnt);
			// roll up the sidewalks and go home
			End_Session(session, pagecnt, iopagecnt);
			tm_state = WAITING;
			gpslock = 0;
			RECLED = 0; GPSLED = 0;
		}
		else {
			// go to next page
			pagecnt++;
		}

		// clear the buffer
		for (i=0; i < pagesize; i++) flashpage.bigbuff[i] = 0;
		// reset sample set counter
		secondcnt = 0;
	} // if
	else				// buffer not filled so go to new second
		secondcnt++;	// go to the next sample set

	// no matter what, reset the accelerometer counter
	gcnt = 0;

} // Process_Sample_Set
#endif	// !DISPLAYATTACHED
//
// Common code when dropping out of a record mode
// Saves everything
//
void Stop_Recording( void ) {

	GPSLED = RECLED = 0;

#ifndef DISPLAYATTACHED
	// save the data
	if (RECORDING(tm_state)) {
		// save the io data
		if (!firstpulse && iocollect)
			DataFlash_Page_Write_From_Buffer_Erase(IOBUFFER, iopagecnt);
		End_Session(session, pagecnt, iopagecnt);
	} // if
#endif

	// if camera is recording, stop recording	
	if (NOCAMLANC != camlanc.ports && UNKNOWNCONFIG != camlanc.ports) {		// camera installed
		switch (cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][SIGNALTYPE]) {
			case LEVEL:		// usb camera
				DIGOUT4 = 0;
				break;
			case PULSE:		// pulse style camera
				// wait a bit in case it just turned on
				DELAY_MS(200);
				DIGOUT4 = 1;
				DELAY_MS(cameracontrol[camlanc.ports][TRAQMATE_TYPE][samps_per_sec/20][RECOFFTIME]);
				DIGOUT4 = 0;
				break;
			case LANC:
				LANCCMD = LANCRECSTOP;				// stop recording on LANC
				break;
		} // switch
		camlanc.status = CAMLANCREADY;
		camlanc.event = NOEVENT;
	} // if
		
	// turn off digital outputs
	if (ENABLED4LANC != camlanc.ports)	
		P4 = 0x00;

} // Stop_Recording

//
// Common code when entering a record mode
//
void Setup_Recording( void ) {
	int itmp;
	u16 clockin = clocktick;

	// initialize
	gcnt = secondcnt = 0;
	gpslock = 0;
	gpspulse = FALSE;

	// set up the sampling rate
	DataFlash_Page_Read(pagesize, flashpage.bigbuff, USERPAGE);

#ifndef DISPLAYATTACHED
	// load the values from configuration
	new_samp = flashpage.user.modesample & 0x0F;
	new_iocollect = flashpage.user.iodata;

	// set the rpm warning level for digital output alarm
	cyl = flashpage.user.car[flashpage.user.selectedcar].carspecs.cylandweight & 0x0F;
	revs = flashpage.user.car[flashpage.user.selectedcar].carspecs.revwarning;
#endif

	// set up the proper sampling rate
	switch (new_samp) {
		case 4:
			samps_per_sec = 40;
			break;
		case 2:
			samps_per_sec = 20;
			break;
		case 0:
		default:
			samps_per_sec = 10;
			break;
	} // switch

	if (NOCAMLANC == camlanc.ports || UNKNOWNCONFIG == camlanc.ports) {		// no camera
		firstpulse = 4;				// start almost immediately
	} // if
	else {			// set up timing for camera
		// setup data delay to allow for camera head start, number of GPS samples to skip = 1 - 4 per second depending on sample rate
		firstpulse = (long) cameracontrol[camlanc.ports][TRAQMATE_TYPE][new_samp/2][DATADELAY] * (long) samps_per_sec / 10L / 1000L;
		if ((int) firstpulse <= 0) firstpulse = 1;		// make sure the table can't be wrong
} // else
	
	// set up the I/O collection
	iocollect = new_iocollect;

	if (0 == revs)
		digout.minperiod = 0;
	else {
		if (0 == cyl)  	// 2-stroke kart
			digout.minperiod = (120 * 2) * (u16) ((float) FREQ_SAMPLE_RATE / (float) revs);
		else {
			if (15 == cyl) cyl = 16; 
			digout.minperiod = (120 / cyl) * (u16) ((float) FREQ_SAMPLE_RATE / (float) revs);
		} // else
	} // else
	// only save data if Data Interface attached
	if (!(P5 & TACHORDATA))
		iocollect = 0;
	else {
		// Set up analog inputs for alarm level detection
		DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);

		// break out digital output status
		digout.digoutstate[D4] = flashpage.io.digouts[0] & 0x0F;
		digout.digoutstate[D5] = (flashpage.io.digouts[0] & 0xF0) >> 4;

		// Set up input scaling
		for (itmp=0; itmp<NUMANALOGS; itmp++) {
			switch (flashpage.io.analogs[itmp].maxscale) {
				case 10: ioscaling[itmp] = 3;
					break;
				case 5: ioscaling[itmp] = 2;
					break;
				case 20:
				default: ioscaling[itmp] = 4;
					break;
			} // switch
		} // for

		// figure out analog alarm levels and direction to watch for
		for (itmp=0; itmp<NUMANALOGS; itmp++) {
			digout.analogwarning[itmp].alarmdirection = flashpage.io.analogs[itmp].alarmdirection;
			// get percentage scale of alarm threshold and convert to 8 bit value
			digout.analogwarning[itmp].alarmnum = flashpage.io.analogs[itmp].lowreading +
				(float) (flashpage.io.analogs[itmp].highreading - flashpage.io.analogs[itmp].lowreading) *
				ABS((flashpage.io.analogs[itmp].alarmval - flashpage.io.analogs[itmp].lowval) / 
				(flashpage.io.analogs[itmp].highval - flashpage.io.analogs[itmp].lowval));
		} // for
	} // else

	// if 16 bit mode selected, turn on all the frequency lines
	if (iocollect & 0x80)
		iocollect |= 0x03;

	// clear the main buffer
	for (itmp=0; itmp < pagesize; itmp++)
		flashpage.bigbuff[itmp] = 0;
		
} // Setup_Recording

//
// Common code for processing IO and checking for alarms
//
void Process_IO ( void ) {
	// only process i/o data if enabled and plugged in
	if (iocollect) {
		s08 checkbits;			// used to check off the enable bits for data i/o
		// save the i/o points in the order MSB - LSB, DIA3A2A1A0F1F0
		for (checkbits = 6; checkbits >= 0; checkbits--) {
			u08 tmpch;

			// if enable bit set, save data point into dataflash buffer
			if ((iocollect >> checkbits) & 0x01) {
				switch (checkbits) {
					// Digitals
					case 6:
						tmpch = (u08) ~P5;		// invert digitals
						break;
					// Analogs
					case 4:		// non-alarmable analogs
					case 5:
						tmpch = (u08) (result[checkbits+2] >> (ioscaling[checkbits-2]));										
						break;
					case 3:
					case 2:	{  	// alarmable analogs
						BOOL warning = FALSE;

						tmpch = (u08) (result[checkbits+2] >> (ioscaling[checkbits-2]));										

						// proceed if analog alarm notification is requested
						if (ANALOGALARM == digout.digoutstate[checkbits - 2]) {
							// check for alarm condition
							switch (digout.analogwarning[checkbits-2].alarmdirection) {
								case 1:
									warning = tmpch > digout.analogwarning[checkbits-2].alarmnum;
									break;
								case 0:
									warning = tmpch == digout.analogwarning[checkbits-2].alarmnum;
									break;
								case -1:
									warning = tmpch < digout.analogwarning[checkbits-2].alarmnum;
									break;
							} // switch

							if (warning) // activate alarm
								P4 |= 1 << (checkbits+2);	
							else 		// deactivate alarm
								P4 &= ~(1 << (checkbits+2));
						} // if
						break;
					} // case
					// Frequencies - 16 bit mode!!
					case 1:	 // MSB = F1
						tmpch = (u08) ((period0 & 0xFF00) >> 8);
						break;
					case 0:	{ // LSB = F0
						int outbit;
						tmpch = (u08) (period0 & 0x00FF);

						// check for rpm alarm
						if (0 != digout.minperiod && 0 != period0) {
							if (digout.minperiod <= period0) {	// turn on alarm
								for (outbit = D4; outbit <= D5 ; outbit++)
									if (RPMWARN == digout.digoutstate[outbit])
										P4 &= ~(1 << (outbit+4));
							} // if								
							else { // turn off alarm
								for (outbit = D4; outbit <= D5 ; outbit++)
									if (RPMWARN == digout.digoutstate[outbit])
										P4 |= 1 << (outbit+4);
							} // else
						} // if
						break;
					} // case 0
				} // switch

#ifndef DISPLAYATTACHED
				// write the character
				DataFlash_Buffer_Write(IOBUFFER, 1, iobyte, &tmpch);

				// check for dataflash buffer full
				if (iobyte != 0)
					iobyte--;	// point to the next byte down in the dataflash buffer
				else {
					// write the buffer
					DataFlash_Page_Write_From_Buffer_Erase(IOBUFFER, iopagecnt);

					// check to see if entire dataflash full
					if (pagecnt >= iopagecnt-1) { // unit is full
						End_Session(session, pagecnt, iopagecnt);
						tm_state = WAITING;
						gpslock = 0;
						RECLED = 0; GPSLED = 0;

						// turn off all digital outputs
						if (ENABLED4LANC != camlanc.ports)
							P4 = 0x00;
					}
					else {
						// go to next page
						iopagecnt--;
						iobyte = pagesize - 1;		// point to MSB
						// clear the io buffer
						DataFlash_Buffer_Clear(IOBUFFER, pagesize);
					} // else
				} // else
#endif
			} // if
		} // for
	} // if iocollect
} // Process_IO

