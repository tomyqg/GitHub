// Part of TraqDash application
//
// This file contains declarations and definitions for all intertask communications.
//

#if !defined(INTERTASK_H)
#define INTERTASK_H

#ifndef _WINDOWS
#include "RTOS.h"

// Task control blocks
extern OS_TASK SDcardRWTCB;					// Task-control-block
extern OS_TASK commTCB;						// Task-control-block
extern OS_TASK lapTimerTCB;					// Task-control-block
extern OS_TASK TQMWriteTCB;					// Task-control-block
extern OS_TASK audioTCB;					// Task-control-block

// Mailboxes
extern OS_MAILBOX MB_SDcardRW;				// mailbox to SD card write task

// Semaphores
extern OS_RSEMA SEMA_GlobalData;			// resource semaphore around global data

// Queues
extern OS_Q Queue_tqm;						// queue of data to be written to .tqm file

// Events

// Task Event Definitions
// these are used to wake up tasks when something has changed such as system operating mode or an input definition
//
// Comm and Timing Tasks - bit number ( to use, | (1 << bitnum) )
#define DU2_MODE_CHANGE			0				// change in operating mode (DU2_STATE_TYPE below
#define DU2_CONFIG_CHANGE		1				// change in analog, digital, autoon, vehicle
												// NOTE: no need to notify on driver or track change
#define DU2_CAMERA_CHANGE		3				// change in camera settings

// General Use
#define DU2_POWER_LOSS			2				// power is lost so close files
#define DU2_SDCARD_EJECT		4				// sd card has been ejected

// Comm Task only
#define TCAT_MSG_RECEIVED		7				// received a fully formed and checked message from SU or PC
#define DU2_SIMULATE_GPS_ON		6				// turn on GPS simulation
#define DU2_SIMULATE_GPS_OFF	5				// turn off GPS simulation

// Lap Timer Task only
#define DU2_NEW_GPS				4				// Comm has received a new GPS position
#define LAP_FILE_OPEN			5				// Comm has opened a lap file global variable
#define DU2_FINISH_SET			6				// user has pressed button to set finish line
#define DU2_START_SET			7				// user has pressed button to set start and/or start/finish line

// TQM Write Task only
#define DU2_TQM_OPEN			5				// opens session file, creates tqm header, and writes it
#define DU2_TQM_CLOSE			6				// writes an end date, lap information and close the file
#define DU2_TQM_WRITE			7				// data has been queued for writing to sessionFile

// Audio Task only
#define AUDIO_SOUND_DONE		5				// audio soundbyte has finished playing

#define ALL_TQM_EVENTS		((1 << DU2_POWER_LOSS) | (1 << DU2_TQM_OPEN) | (1 << DU2_TQM_CLOSE) | (1 << DU2_TQM_WRITE) | (1 << DU2_NOSDCARD))

#endif

void notifyModeChange( void );
void notifyConfigChange ( void );
void notifyCameraChange ( void );
void notifyStartSet ( void );
void notifyFinishSet ( void );
void notifyGPS ( void );
int tqmWrite ( void *, int );
int tqmOpen ( void *, int );
void notifyTQM ( int );
void notifyGPSSimChange ( int );

// This are the states for sysData.du_state. They capture the operating mode of the TraqDash so the various
// tasks can act appropriately.
// * indicates modes to be used in 1.00 release
//
typedef enum {
	
	// startup - initialized state. some tasks do not run during startup
	DU2_STARTUP,				// system is initializing
								// entered by Main during startup. exited by GUI when screens presented

	// everything initialized, menu system operating
	DU2_IDLE,					// * idle state, user traversing menus or in non-recording screens
								// entered by GUI when system initializes and loads GUI, exited by GUI
								// entered and exited by SDCardManager when SD Card inserted
								
	DU2_NOSDCARD,				// idle state but no sd card installed
								// entered and exited by SDCardManager

	// when system enters a data screen, system status changes from idle to one below
	// intermediate modes requiring user input
	DU2_AUTOX_WAITSTAGE,		// waiting for user to indicate they have staged (autox/rally)
								// entered by GUI, exited by GUI
	DU2_DRAG_WAITSTAGE,			// waiting for user to indicate they have staged (drag)
								// entered by GUI, exited by GUI

	// in DU2, recording modes are below this line
	DU2_LAPS_WAITNOSF,			// * lapping mode but no s/f set so waiting for user to press button
								// entered by GUI, exited by Timer
	DU2_AUTOX_WAITLAUNCH,		// waiting for user to indicate they have staged (autox/rally)
								// entered by GUI, exited by Timer
	DU2_DRAG_WAITLAUNCH,		// waiting for user to indicate they have staged (drag)
								// entered by GUI, exited by Timer
	DU2_HILLCLIMB_WAITNOSF,		// hillclimb mode but no s/f set so waiting for user to press button
								// entered by GUI, exited by GUI

	// intermediate modes requiring no user input
	DU2_LAPS_WAITSTART,			// * lapping mode with s/f set so waiting for first time s/f. only here for possible user notification
								// entered by GUI, exited by Timer
	DU2_HILLCLIMB_WAITSTART,	// rolling start hillclimb mode with s/f set so waiting for first time s/f. only here for possible user notification
								// entered by GUI, exited by Timer

	// operating modes, requiring no user input
	DU2_LAPS,					// * recording closed course race, s/f set
								// entered by GUI, exited by GUI
	DU2_AUTOX,					// recording open course race, looking for finish line
								// entered by GUI, exited by Timer
	DU2_DRAG,					// recording drag, looking for finish distance
								// entered by GUI, exited by Timer
	DU2_HILLCLIMB,				// recording open course rolling start, looking for start
								// entered by GUI, exited by GUI
	DU2_DRIVE,					// * recording point to point drive
								// entered by GUI, exited by GUI
	DU2_CAMERA_TEST,			// turns on camera by faking SU into thinking it is recording

	// ending modes, requiring user input
	DU2_AUTOX_WAITFINISH,		// timing but waiting for user to hit button at finish (autox/rally)
								// entered by Timer, exited by GUI
	DU2_HILLCLIMB_WAITFINISH,	// timing but waiting for user to hit button at finish (hillclimb)
								// entered by Timer, exited by GUI
	DU2_HILLCLIMB_FINISH,		// timing, found start, now looking for finish line
								// entered by Timer, exited by Timer
	DU2_AUTOX_COMPLETE,			// autox passed finish line. hold time on screen. continue recording
								// entered by Timer, exited by GUI
	DU2_DRAG_COMPLETE,			// drag, passed finish line. hold time on screen. continue recording
								// entered by Timer, exited by GUI

	// other operating modes
	DU2_INSTRUMENT,				// non-recording entered by GUI, exited by GUI
	
	DU2_SUCODE_FOUND,			// su program, updated su code found on SD card
								// entered by DUIIMain / GUI during startup. exited by Comm
	DU2_SU_SENDING,				// su reprogram, sending new code
								// entered by Comm, exited by Comm
	DU2_SU_SAVING,				// su reprogram, waiting for SU to finish burning flash
								// entered by Comm, exited by Comm
	DU2_TDBOOT_FOUND,			// du2 program, updated du2 bootloader found on SD card
								// entered by DUIIMain / GUI during startup. exited by GUI
	DU2_TDBOOT_SAVING,			// du2 program, waiting to finish burning flash
								// entered by GUI during startup. exited by GUI
	DU2_TDCODE_FOUND,			// du2 program, updated du2 firmware found on SD card
								// entered by DUIIMain / GUI during startup. exited by GUI
	DU2_TDCODE_SAVING,			// du2 program, waiting to finish burning flash
								// entered by GUI during startup. exited by GUI

	// shutdown and reprogramming modes
	DU2_SHUTDOWN,				// system is shutting down due to user interface
								// entered by DUIIMain / GUI, exited when power goes away	
	DU2_REPROGRAMMING_SU,		// sending code down to SU
								// entered by DUIIMain / GUI, exited by DUIIMain / GUI
	DU2_REPROGRAMMING,			// rewriting TraqDash Flash code memory
								// entered by DUIIMain, exited by reset
	DU2_POWERLOSS				// system is shutting down due to power loss
								// entered by Comm when notified of power loss by SU, exited when power goes away
} DU2_STATE_TYPE;

// traqdash modes
#define DU2_RECORDING(x) (((x) >= DU2_AUTOX_WAITSTAGE) && ((x) <= DU2_DRAG_COMPLETE))
#define DU2_OCCUPIED(x) (DU2_RECORDING(x) || ((x) == DU2_SHUTDOWN) || ((x) >= DU2_SUCODE_FOUND))
#define DU2_DONT_INTERRUPT(x) ((x) >= DU2_SUCODE_FOUND)

#endif
