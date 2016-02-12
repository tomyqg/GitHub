#define COMM_MAIN

#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	COMMTASK_NUM
#endif

#define AUTOON			// enables Auto-On function

#include <stdio.h>
#include <string.h>
#include "WM.h"
#include "SPI_API.h"
#include "DUII.h"
#include "FS.h"
#include "CommTask.h"
#include "message.h"
#include "DataHandler.h"
#include "datadecode.h"
#include "dbgu.h"
#include "FlashReadWrite.h"
#include "adc.h"
#include "TQMFile.h"

//
// External data declarations
//
extern char LapInitialize;				// semaphore between CommTask and LapTimerTask

//
// External functions
//
void mSDelay( int );

//
// Local defines, constants and structure definitions
//
//
#define	WAIT_FOR_DATA	1
#define WAIT_FOR_CAMERA	2

// Local function prototypes
//

//
// Local variable declarations
//
int waiting_usb_camera = FALSE;							// true when we are in delay for usb camera
OS_TIMER stationaryTimer;								// keeps track of time sitting still

static void ClearCTS( void) {
	int i;
	
	for (i=0; i < (NUMUNITS-1); i++) {
		// if already cleared, don't copy value (initial condition solution)
		if (0 != COMMCTS[i])
			COMMCTS[i] = OS_GetTimerValue(&(COMMCTSTIMERS[i]));
	} // for
	// if SU is not responding report it and report camera not connected
	if (0 == COMMCTS[SENSOR_UNIT-1]) {
		suData.suPresent = FALSE;
		DataValues[GPS_NUMSAT].uVal = 0;
		if ((CAMERA_NONE != vehicleData.camera) && (CAMERA_USB != vehicleData.camera))
			DataValues[CAMERA_STATUS].iVal = CAMERA_STATUS_NOT_READY;
	} // if
} // ClearCTS

// returns TRUE if we got an ack, FALSE if we didn't
//
static unsigned char Get_Ack( void ) {
	unsigned char retval = FALSE;

	// wait for a message to come in or time out
	if (0 != OS_WaitEventTimed( (unsigned char) (1 << TCAT_MSG_RECEIVED), 600 )) {
		
		if (NULL != com[0].rxptr) {
			u08 *tmpptr = com[0].rxptr;
	
			// mark buffer free
			com[0].rxptr = NULL;
	
			// check to see if this is an ACK
			if (ACK == tmpptr[OPCODE])
				retval = TRUE;
	
			// process the message
			Parse_Message((u08) 0, tmpptr);
		} // if
	} // if
	
	
	return (retval);
} // Get_Ack

// called before ISR is initialized to setup buffers
void App_Comm_Setup(void) {
	int i;

	// create com port handles
	com[0].use = MESSAGING;					// on DU2 only port used for messaging
	com[0].port = 0;
	com[0].txbytes = 0;						// initialize
	com[0].rxnextidx = 0;					// initialize
	com[0].parity = 'N';					// parity = NONE
	com[0].rxptr = com[0].txptr = NULL;		// initialize semaphor = NULL;
	com[0].rxbuff = msgrxbuff[0];			// point to first buffer
	com[0].txbuff = msgtxbuff;				// point to single buffer
	
	for (i=0; i< NUMUNITS-1; i++) {
		// create timers for communications
		OS_CreateTimer(&(COMMCTSTIMERS[i]), ClearCTS, CTS_COUNTDOWN);
	} // for
	
	suData.tmState = STARTUP;
}

// returns the iodata sample byte for the SU
u08 getIoData( void ) {
	u08 iodata = 0x83; 		// start with freq enabled
	int i;
	
	iobytesenabled = 2;		// rpm always on

	// which i/o data points are turned on/off
	//   bit 7 = 16bit Freq, 6 = all digitals, 5 = Anlg3, 4 = Anlg2
	//   bit 3 = Anlg1, 2 = Anlg0, 1 = Freq1, 0 = Freq0
	// set up analogs

	for (i = 0; i < NUMANALOGS; i++) {
		// analog enable bits
		if (vehicleData.analogInput[i].enabled) {
			iodata |= (u08) (1 << (2+i));
			iobytesenabled++;
		} // if
	} // for
	
		// if either digital input enabled, turn them both on
	if (vehicleData.digitalInput[0].enabled || vehicleData.digitalInput[1].enabled) {
		iodata |= (u08) (1 << 6);
		iobytesenabled++;
	} // if

	return iodata;
} // getIoData

// returns the mode samp definition byte for the SU
u08 getModeSamp( void ) {
	u08 mode = 4;					// 1 = 10hz, 2 = 20hz, 4 = 40hz
	
	// put recording mode in upper nibble
	mode |= ((u08) unsavedSysData.recordingMode) << 4;

	return mode;
} // getModeSamp

static void ChangeSUMode(tmstatetype targetstate) {
	int inputnum;

	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = (unsigned char) targetstate;	// new operating mode
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = getModeSamp();

	// which i/o data points are turned on/off
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2] = iocollect = getIoData();
	
	// analog input ranges
	for (inputnum = 0; inputnum < NUMANALOGS; inputnum++) {
		switch (vehicleData.analogInput[inputnum].inputRange) {
			case 10:
				com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+inputnum] = 3;
				break;
			case 5:
				com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+inputnum] = 2;
				break;
			case 20:
			default:
				com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+inputnum] = 4;
				break;
		} // switch
	} // for
	
	// send frequency alarm data for currently selected car
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+7] = SWAP16(vehicleData.tach.upperYellowStart);
	
	// Tell SU to start sending data
	SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) MODE_CHANGE, 10);

} // ChangeSUMode

static unsigned char ChangeSUCamera( void ) {
	int i;
	flashpagetype flashpage;				// dataflash sector for SU

	for (i=0; i < sizeof(flashpagetype); i++)
		flashpage.bigbuff[i] = 0xFF;

	// put in analog scales
	for (i=0; i < NUMANALOGS; i++)
		flashpage.io.analogs[i].maxscale = vehicleData.analogInput[i].inputRange;
	
	// digital outputs not supported except for camera control
	flashpage.io.digouts[0] = 0x00;			// digital input
	flashpage.io.digouts[1] = 0x00;			// digital input

//	choosing not to put these in since they get changed during modechange
//	analogdefs analogs[NUMANALOGS];			// 40x4 = 160
//	digitaldefs digitals[NUMDIGITALS];		// 36x7 = 252
//	freqdefs frequency[NUMFREQS]; 			// 5x2 = 10
			
	// determine which selection was chosen and program the camera bits
	switch (vehicleData.camera) {
		case CAMERA_NONE: // None
		case CAMERA_USB:
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + DISABLED;
			break;
		case CAMERA_CHASECAM:		// deprecated to Mobius
		case CAMERA_MOBIUS: // Mobius
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + MOBIUS;
			break;
		case CAMERA_REPLAY_MINI: // Replay Mini USB
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYMINI;
			break;
		case CAMERA_REPLAY_PRIMEX: // Replay PrimeX USB
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYPRIME;
			break;
		case CAMERA_GOPRO_HD: // GoPro
		case CAMERA_GOPRO_HD_PRO:
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + GOPROCAMERA;
			break;
		case CAMERA_REPLAY_HD: // ReplayXD
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYCAMERA;
			break;
		case CAMERA_CHASECAM_LANC:
		case CAMERA_SONY_HD:
		case CAMERA_RACECAM_HD: // Sony HD
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + SONYLANC;
			break;
		case CAMERA_REPLAY_XD_USB: // ReplayXD USB
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYXDUSB;
			break;
	} // switch

	// put in the write address
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = (u08) 0;
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = (u08) (IODEFPAGE & 0xFF);
	
	// copy segment into output buffer
	for (i=0; i < MAX_PAGE_SIZE; i++)
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2+i] = flashpage.bigbuff[i];

	for (i=0; i<3; i++) {
		// send io defs to SU for use in digital outputs
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) WRITE_SEGMENT, MSGDATA);
		if (Get_Ack()) return TRUE;
	} // if
	return FALSE;
		
} // ChangeSUCamera

static void ChangeUSBCamera( int activate ) {

	if (activate)
		PIOA_SODR = (1 << USB_ENABLE_PIN);			// turn on usb
	else
		PIOA_CODR = (1 << USB_ENABLE_PIN);			// turn off usb
	
} // ChangeUSBCamera

// changes flag to stop recording after user defined stationary period
static void stopRecording ( void ) {
	unsavedSysData.recordingTimedout = TRUE;
} // stopRecording

static int USBCameraOn ( void ) {
	return (0 != (PIOA_PDSR & (1 << USB_ENABLE_PIN)));
} // USBCameraOn

static unsigned char ChangeAutoOn( void ) {
	int i;

#ifdef AUTOON
	// program AutoOn into SU
	if (suData.suHwRev < 200)	// only do this if we are connected to SU2
		return TRUE;
	else {
		for (i=0; i<3; i++) {
			// Send AutoOn status to SU
			com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = (sysData.autoOn? 1 : 0);
			SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) AUTO_ON, 1);
			
			if (Get_Ack()) return TRUE;
		
		} // for
	} // else
	return FALSE;
#else
	return TRUE;
#endif
} // ChangeAutoOn


// returns true if SU was successfully stopped
//
static BOOL StopSU( void ) {
	int i;

	// try 7 times to reset the sensor unit
	for (i=0; i<7; i++) {
		
		// Reset the sensor unit
		ChangeSUMode(WAITING);

		if (Get_Ack()) {
			return true;		// worked so report all is good
		} // if
	} // for
	return false;				// didn't work
} // StopSU

static void InitCommTask(void) {

	// clear any pending events
	OS_ClearEvents(&commTCB);

	// mark buffer free
	com[0].rxptr = NULL;
	
// Not sure what to do here. If no SU and AutoOn is enabled, can't turn off Traqdash.
// Weigh that against having to re-enable AutoOn when next connected to SU2.
//	if (!(suData.suPresent = StopSU()))				// if initialization worked, su is present
//		sysData.autoOn = false;						// turn off autoOn if we are not connected to SU
	
	suData.suPresent = StopSU();					// if initialization worked, su is present
	
	unsavedSysData.recordingMode = TQM_MODE_DRIVE;		// initialize to something
	
	if (!suData.suPresent) {
		// didn't hook up with SU so keep trying but at a slower rate
		do {
			
			OS_Delay(1000);			// wait a second
	
			// Reset the sensor unit
			suData.suPresent = StopSU();					// if initialization worked, su is present
	
		} while (!suData.suPresent);
	} // if

	do {
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) READ_UNIT_INFO, 0);
		Get_Ack();
	} while (!gotscratchpad);

	// translate all the useful info into separate variables
	suData.suSwRev = SWAP16(scratchpad.calibrate.swrev);
	suData.suDataRev = SWAP16(scratchpad.calibrate.datarev);
	suData.suHwRev = scratchpad.calibrate.hwrev;
	suData.buildDateWeek = scratchpad.calibrate.week;
	suData.buildDateYear = scratchpad.calibrate.year;
	suData.serialNumber = SWAP32(scratchpad.calibrate.serno);
	accelx.min = (float) SWAP16(scratchpad.calibrate.xminusg);
	accelx.zero = (float) SWAP16(scratchpad.calibrate.xzero);
	accelx.max = (float) SWAP16(scratchpad.calibrate.xplusg);
	accely.min = (float) SWAP16(scratchpad.calibrate.yminusg);
	accely.zero = (float) SWAP16(scratchpad.calibrate.yzero);
	accely.max = (float) SWAP16(scratchpad.calibrate.yplusg);
	accelz.min = ((float) SWAP16(scratchpad.calibrate.zminusg)) / 16.0;
	accelz.zero = ((float) SWAP16(scratchpad.calibrate.zzero)) / 16.0;
	accelz.max = ((float) SWAP16(scratchpad.calibrate.zplusg)) / 16.0;
	
	OS_Delay(3500);					// give it a rest while rest of system boots and config is loaded, worked at 3000 but changed due to change in startup priority

	if (suData.suHwRev < 200)		// if connected to SU1, disable AutoOn
		sysData.autoOn = false;
	
	ChangeAutoOn();					// program AutoOn settings into SU

	if (CAMERA_CHASECAM == vehicleData.camera)
		OS_Delay(7000);				// needed for chasecam
	
	ChangeSUCamera();				// put camera settings into SU
	
	if (CAMERA_CHASECAM == vehicleData.camera)
		OS_Delay(7000);				// needed for chasecam
	else
		OS_Delay(500);				// just for safety, works at 500, seems to work fine when commented out
	
	do {
		ChangeSUMode(GPSINFO);
	} while(!Get_Ack());

	suData.tmState = GPSINFO;		

} // InitCommTask

#define ALL_COMM_EVENTS ((1 << DU2_MODE_CHANGE) | (1 << DU2_CONFIG_CHANGE) | (1 << TCAT_MSG_RECEIVED | (1 << DU2_CAMERA_CHANGE) | (1 << DU2_SIMULATE_GPS_ON) | (1 << DU2_SIMULATE_GPS_OFF)))

void CommTask( void ) {
	char which_event;										 // bitmask of events that occured
	static DU2_STATE_TYPE previous_state;
	char filepath[MAX_FILENAME_SIZE];
	OS_TIMER USBCameraTimer;
	
	unsavedSysData.waitingForRecordSpeed = FALSE;
	
  	InitCommTask();
	
	OS_CreateTimer(&USBCameraTimer, notifyModeChange, 0);				// create timer for usb camera delay
	OS_CreateTimer(&stationaryTimer, stopRecording, 0);					// create timer to stop recording when sitting still

	previous_state = unsavedSysData.systemMode;
	sessionFile = NULL;
	
	while (1) {
		// wait for something to happen
		which_event = OS_WaitEvent( (unsigned char) ALL_COMM_EVENTS );
		
		// read the current vehicle power
		DataValues[VEHICLE_12V].fVal = (BATTERY_VOLTAGE_CONVERSION / 255.0 * (float) (GetADC(ADC_VEHICLE_12V))) + 0.15;
		
		// check for power loss
		if (DataValues[VEHICLE_12V].fVal < SHUTDOWN_VOLTAGE) {
			// save files and shut it down
			
			unsavedSysData.systemMode = DU2_POWERLOSS;		// change global state
				
			// shut down backlight and LEDs to save power
//			LEDSystemShutdown();

			// close data file and save data
			if (NULL != sessionFile) {

//				too slow
//				notifyTQM ( DU2_POWER_LOSS );					// tell tqm task to finish up and close file
//				OS_SetPriority( &TQMWriteTCB, 220);				// make tqm task run		  
			  
//				save lap times before losing all power
				FS_FSeek( sessionFile, TQM_LAP_OFFSET, FS_SEEK_SET);	// jump to start of lap times in header
//				
				// write header lap times
//				FS_Write( sessionFile, &(tqmheader.tqmheaderstruct.lapInfo), sizeof(tqmheader.tqmheaderstruct.lapInfo));	// write all laps. works most of the time but it failed once.
				FS_Write( sessionFile, &(tqmheader.tqmheaderstruct.lapInfo), 212);	// write first 50 header lap times. works every time.

//				close the data file and flush
				FS_FClose(sessionFile);
				FS_Sync("");
				sessionFile = NULL;
			} // if

			StopSU();			
			OS_Delay(500);				// give it a second
	
			// turn off
			while (1) {			
				// go away, just go away
				SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) POWER_DOWN, (u08) 0);
				
				// wait a bit then send again until it works
				OS_Delay(100);			// wait - but allow other tasks to run!
			} // while
		} // if
		
		// if we are reprogramming ourselves, just stop
		if (DU2_REPROGRAMMING == unsavedSysData.systemMode) {	// don't do anything if we are reflashing unit
			StopSU();
			do {
				OS_Delay(600000);				// go to sleep for a long time during programming
			} while (DU2_REPROGRAMMING == unsavedSysData.systemMode);
		} // if

		// set the status of the usb camera
		if (CAMERA_USB == vehicleData.camera) {
			if (USBCameraOn())
				DataValues[CAMERA_STATUS].iVal = CAMERA_STATUS_RECORDING;
			else
				DataValues[CAMERA_STATUS].iVal = CAMERA_STATUS_READY;
		} // if
		
		// check for GPS simulator request
		if (which_event & (1 << (DU2_SIMULATE_GPS_ON))) {
			com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = 1;
			SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) SIMULATE_GPS, 1);
		} // if
		if (which_event & (1 << (DU2_SIMULATE_GPS_OFF))) {
			com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = 0;
			SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) SIMULATE_GPS, 1);
		} // if

		if (unsavedSysData.waitingForRecordSpeed || (which_event & (1 << DU2_MODE_CHANGE))) {		// user changed modes
			if (DU2_REPROGRAMMING_SU == unsavedSysData.systemMode) {	// user requested re-flashing of SU
				StopSU();
			} // if
			
			if (DU2_SHUTDOWN == unsavedSysData.systemMode) {	// user requested shutdown				
				// close the files

				if (unsavedSysData.sdCardInserted) {		// only save file if we have an SD card
					notifyTQM ( DU2_TQM_CLOSE );					// tell tqm task to finish up and close file
					OS_SetPriority( &TQMWriteTCB, 200);						// make tqm task run
				} // if

				// turn off power to usb
				ChangeUSBCamera( 0 );
				
				StopSU();			
				OS_Delay(2000);				// give it a second to see messages on screen

				// turn off
				while (1) {			
					// go away, just go away
					SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) POWER_DOWN, (u08) 0);
					
					// wait a bit then send again until it works
					OS_Delay(100);			// wait - but allow other tasks to run!

				} // while
			} // if

			if (DU2_INSTRUMENT == unsavedSysData.systemMode)		// not entering recording mode so clear timers another way
				LapInitialize = true;
			
			// if we have entered a recording mode, reinitialize SU with recording info
			if (previous_state != unsavedSysData.systemMode) {
				
				// entering recording state
				if (!DU2_RECORDING(previous_state) && DU2_RECORDING(unsavedSysData.systemMode)) {
					int filenum;
					char sessionkind;
					
					if (DataValues[SPEED].fVal >= ((sysData.multiSettings_2 & 0xFFFF0000) >> 16)) {
						unsavedSysData.waitingForRecordSpeed = FALSE;
					} // if
					else {
						unsavedSysData.waitingForRecordSpeed = TRUE;
						goto quick_exit;
					} // if
				
					// check to see whether TraqDash usb camera is enabled
					if (!(0x80 & vehicleData.usbCameraDelay)) {		// upper bit is enable bit
						// not enabled so turn off USB camera
						ChangeUSBCamera ( 0 );
						waiting_usb_camera = FALSE;
					} // if
					else { // usb camera is enabled
						int delay = 100 * (unsigned int) (0x7F & vehicleData.usbCameraDelay);
						
						if ((CAMERA_NONE == vehicleData.camera) || (CAMERA_USB == vehicleData.camera)) {	// start camera first
							if (USBCameraOn()) {		// turned on previously so we can stop waiting
								waiting_usb_camera = FALSE;
							} // if
							else {		// camera not on
								// turn on USB camera
								ChangeUSBCamera ( 1 );
	
								if (0 != delay) {
									waiting_usb_camera = WAIT_FOR_DATA;
									OS_SetTimerPeriod(&USBCameraTimer, delay);
									OS_RetriggerTimer(&USBCameraTimer);
								} // if
							} // else
						} // if
						else {	// SU camera attached so start data first unless we are just testing
							if (WAIT_FOR_CAMERA == waiting_usb_camera) {
								// turn on USB camera
								ChangeUSBCamera ( 1 );
								waiting_usb_camera = FALSE;
								goto quick_exit;
							} // if
							else if ((0 == delay) || (DU2_CAMERA_TEST == unsavedSysData.systemMode)) {
								// turn on USB camera
								ChangeUSBCamera ( 1 );
								waiting_usb_camera = FALSE;
							}
							else {	// first time through
								waiting_usb_camera = WAIT_FOR_CAMERA;
								OS_SetTimerPeriod(&USBCameraTimer, delay);
								OS_RetriggerTimer(&USBCameraTimer);
							} // else
						} // else
					} // else
					
					if (WAIT_FOR_DATA != waiting_usb_camera) { // process the mode change		

						// clear the flag for not moving recording stop - now done in ConfigScreens -- gas
//						unsavedSysData.recordingTimedout = FALSE;
						
						// entered a recording mode - translate to SU mode
						switch (unsavedSysData.systemMode) {
							case DU2_LAPS_WAITSTART:
								// set working start line to current track start line
								unsavedSysData.workingStartLineLat = trackData.startLineLat;
								unsavedSysData.workingStartLineLong = trackData.startLineLong;
								unsavedSysData.workingStartLineHeading = trackData.startLineHeading;
								// fall through
							case DU2_LAPS_WAITNOSF:
							case DU2_LAPS:							// This case can still happen if the user presses Set S/F line quickly
								suData.tmState = LAPS;
								unsavedSysData.recordingMode = TQM_MODE_LAPPING;
								firstpulse = true;
								sessionkind = 'R';					// circuit racing
								break;
							case DU2_AUTOX_WAITSTAGE:
//							case DU2_AUTOX:				// can't happen
							case DU2_AUTOX_WAITLAUNCH:
//							case DU2_AUTOX_WAITFINISH:	// can't happen
//							case DU2_AUTOX_COMPLETE:	// can't happen
								// set working finish line to current track finish line
								unsavedSysData.workingFinishLineLat = trackData.finishLineLat;
								unsavedSysData.workingFinishLineLong = trackData.finishLineLong;
								unsavedSysData.workingFinishLineHeading = trackData.finishLineHeading;
	
								suData.tmState = AUTOXREADY;
								unsavedSysData.recordingMode = TQM_MODE_AUTOCROSS;
								firstpulse = true;
								sessionkind = 'A';					// autocross
								break;
	
							case DU2_DRAG_WAITSTAGE:
							case DU2_DRAG_WAITLAUNCH:
//							case DU2_DRAG:				// can't happen
//							case DU2_DRAG_COMPLETE:		// can't happen
								suData.tmState = DRAGREADY;
	
								switch (sysData.dragDistance) {
									case DD_1_4:
										unsavedSysData.recordingMode = TQM_MODE_QTR_MILE;
										sessionkind = 'Q';					// drag racing
										break;
									case DD_1_8:
										unsavedSysData.recordingMode = TQM_MODE_EIGHTHMILE;
										sessionkind = 'E';					// drag racing
										break;
									case DD_1000:
										unsavedSysData.recordingMode = TQM_MODE_THOUSAND;
										sessionkind = 'T';					// drag racing
										break;
								} // switch
								firstpulse = true;
								break;
							case DU2_HILLCLIMB_WAITSTART:
								// set working start line to current track start line
								unsavedSysData.workingStartLineLat = trackData.startLineLat;
								unsavedSysData.workingStartLineLong = trackData.startLineLong;
								unsavedSysData.workingStartLineHeading = trackData.startLineHeading;
								
								// set working finish line to current track finish line
								unsavedSysData.workingFinishLineLat = trackData.finishLineLat;
								unsavedSysData.workingFinishLineLong = trackData.finishLineLong;
								unsavedSysData.workingFinishLineHeading = trackData.finishLineHeading;
								// fall through
							case DU2_HILLCLIMB_WAITNOSF:
//							case DU2_HILLCLIMB:
//							case DU2_HILLCLIMB_WAITFINISH:							
								suData.tmState = HILLCLIMB;
								unsavedSysData.recordingMode = TQM_MODE_HILLCLIMB;
								firstpulse = true;
								sessionkind = 'H';					// hillclimb
								break;
	
							case DU2_DRIVE:
							case DU2_CAMERA_TEST:
							default:
								suData.tmState = DRIVE;
								unsavedSysData.recordingMode = TQM_MODE_DRIVE;
								
								// record where we started
								unsavedSysData.workingStartLineLat = trackData.startLineLat = GetValue(LATITUDE).dVal;
								unsavedSysData.workingStartLineLong = trackData.startLineLong = GetValue(LONGITUDE).dVal;
								unsavedSysData.workingStartLineHeading = trackData.startLineHeading = GetValue(HEADING).fVal;
								firstpulse = true;
								sessionkind = 'D';					// drive
								break;
						} // switch					
	
						// change SU to record mode
						StopSU();
						ChangeSUMode(suData.tmState);
							
						if (DU2_CAMERA_TEST != unsavedSysData.systemMode && unsavedSysData.sdCardInserted) {		// create file if we have an SD card and not a camera test
							strcpy(filepath, MY_SESSIONS_PATH);
							strcat(filepath, "\\");
							formattime(SHORTDATE, filepath+(strlen(filepath)));
							sprintf(filepath+strlen(filepath), "_%s_%s_%s_%c01.tqm", sysData.track, sysData.vehicle, sysData.driver, sessionkind);
							filenum = 1;
							
							// find a unique filename
							while (0xFF != FS_GetFileAttributes(filepath)) {
								sprintf((filepath + strlen(filepath) - 6), "%02d.tqm", ++filenum);
							}
							
							// open tqm file and write out tqm header
							tqmOpen (filepath, strlen(filepath)+1);
						} // if		
					} // if not waitusbcamera
				} // if entered recording
				else if (!DU2_RECORDING(unsavedSysData.systemMode)) {		// entering non-recording state
					// exited a recording mode
					if (DU2_RECORDING(previous_state) && (NULL != sessionFile) && (DU2_CAMERA_TEST != previous_state)) {
						if (DU2_DRIVE == previous_state) {
							// record where we ended
							unsavedSysData.workingFinishLineLat = trackData.finishLineLat = GetValue(LATITUDE).dVal;
							unsavedSysData.workingFinishLineLong = trackData.finishLineLong = GetValue(LONGITUDE).dVal;
							unsavedSysData.workingFinishLineHeading = trackData.finishLineHeading = GetValue(HEADING).fVal;
						} // if
						if (unsavedSysData.sdCardInserted)		// only save file if we have an SD card
							notifyTQM (DU2_TQM_CLOSE);								// orderly close
					} // if

					// no longer waiting to start
					unsavedSysData.waitingForRecordSpeed = FALSE;
					
					// no longer recording so stop stationary timer
					OS_StopTimer(&stationaryTimer);
					unsavedSysData.timeToStopRecording = 0;
					
					// turn off USB camera
					ChangeUSBCamera ( 0 );
					StopSU();
					
					ChangeSUMode(suData.tmState = GPSINFO);
				} // else if

quick_exit:		// skipping out without processing mode change
		
				// if we have truly started recording, allow mode to change
				if ((FALSE == waiting_usb_camera) && (FALSE == unsavedSysData.waitingForRecordSpeed))
					previous_state = unsavedSysData.systemMode;
			} // if
		} // if
		
		if (which_event & (1 << TCAT_MSG_RECEIVED)) {	// got a message from SU
			if (NULL != com[0].rxptr) {
				
				u08 *tmpptr = com[0].rxptr;
	
				// mark buffer free
				com[0].rxptr = NULL;
	
				// process the message
				Parse_Message((u08) 0, tmpptr);
				
				// if we got our first GPS position of the session, reset the timers to sync with cameras
				if (firstpulse) {
					LapInitialize = true;
				} // if
				else if (!unsavedSysData.waitingForRecordSpeed && DU2_RECORDING(unsavedSysData.systemMode)) {
					// check to see if we are recording, not waiting for a speed, and we are sitting still
					if (DataValues[SPEED].fVal >= 1.0) {		// moving
						// stop timer
						OS_StopTimer(&stationaryTimer);
						unsavedSysData.timeToStopRecording = 0;
					} // if
					else { 		// stationary
						int temp;

						if (0 != (temp = ((sysData.multiSettings_2 & 0x0000FFF0) >> 4))) {	// stationary time set
							// don't do anything if timer already running
							if (0 == OS_GetTimerStatus(&stationaryTimer)) {
								// timer is stopped so restart
								if (temp > 5) {		// sanity check on timer value in case of initialization problem. minumum is 60 seconds
									OS_SetTimerPeriod(&stationaryTimer, temp * 1000);
									OS_RetriggerTimer(&stationaryTimer);
									unsavedSysData.timeToStopRecording = temp;
								} // if
							} // if
							else {
								// countdown to stop recording for user interface
								unsavedSysData.timeToStopRecording = OS_GetTimerValue(&stationaryTimer) / 1000;			// how much time is left?
							} // else
						} // if
					} // else
				} // else
			} // if
		} // if
		
		if (!(which_event & (1 << DU2_MODE_CHANGE))) {
			// must be a configuration change. we are not entering record mode but must reset SU
			if ((which_event & (1 << DU2_CONFIG_CHANGE)) || (which_event & (1 << DU2_CAMERA_CHANGE))) {
				// reinitialize SU with new configuration data
//				StopSU();  // works with this enabled but occasionally gets stuck after turning off camera
				ChangeAutoOn();
				// if camera changed, send new setup to SU
//				if (which_event & (1 << DU2_CAMERA_CHANGE))
				ChangeSUCamera();
				OS_Delay(100);
				ChangeSUCamera();
				
				// do it twice so it will take every time
				ChangeSUMode(suData.tmState);	// return to previous SU state
				ChangeSUMode(suData.tmState);	// return to previous SU state
			} // if
		} // if
	} // while
} // CommTask
