/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: DUII.h
Description: config file for project
-----------------------------------------------------------------
*/
#ifndef DU2_H
#define DU2_H

#include "DIALOG.h"
#include "DU2rev.h"
#include "InterTask.h"

#ifdef __cplusplus
extern "C" {
#endif

// These are the default values that get programmed into new units.
// The date values should be updated with each new build.

#define DEFAULT_SERIAL_NUMBER	399999						/* indicates serial number not programmed */
#define DEFAULT_BUILD_WEEK		02							/* week the unit was programmed at factory */
#define DEFAULT_BUILD_YEAR		16							/* year the unit was programmed at factory */
#define FIRST_GEN_BOOTLOADER	135							/* bootloader that does not turn off USB */
#define SECOND_GEN_BOOTLOADER	150							/* bootloader that does turn off USB */	
#define DEFAULT_WIFI_NAME		"TraqDashWiFi"				/* wifi network name for wifi card */
#define DEFAULT_WIFI_PASS		"traqmate"					/* password for wifi card */
	
#ifdef _WINDOWS
#define DEFAULT_BOOTLOADER_REV	SECOND_GEN_BOOTLOADER		/* includes USB camera capability */
#else
#define DEFAULT_BOOTLOADER_REV	FIRST_GEN_BOOTLOADER		/* first 1000 traqdashes built with this rev */
#endif

/****************************************************************
*
*       Conditional compile defines
*
******************************************************************/
// Allow SHOW_GREEN_LINE to be defined to show green line on analog tach and sweep tach screens
//#define	SHOW_GREEN_LINE


/****************************************************************
*
*       defines
*
******************************************************************/
#define YES 1
#define NO 0
#define CANCEL -1

#define NUMBER_OF_ANALOGS		4										/* a0 - a3 */
#define NUMBER_OF_DIGITALS		2										/* d4, d5 */
#define NUMBER_OF_GEARS			8
#define MAX_TRACK_LEN			65										/* 65 km = Isle of Man */
#define SHORT_PREDICTIVE_DISTANCE		10								/* 10 meters between breadcrumbs for autox and drag racing */
#define LAP_PREDICTIVE_DISTANCE			50								/* 50 meters between breadcrumbs for closed circuit racing */
#define NUMBER_OF_SECTORS		(MAX_TRACK_LEN * 1000 / LAP_PREDICTIVE_DISTANCE)	/* 1300 number of intermediate points for predictive lap timing */

#ifndef OWNER_LEN
#define OWNER_LEN					20		/* number of characters in owner name */
#endif

#define PHONE_LEN					20		/* number of characters in owner phone number */
#define WIFI_LEN					20		/* number of characters in each wifi configuration field */
#define ROOM_TEMPERATURE			24		/* default temperature for calibration of LED board */
#define ROOM_TEMP_READING			565		/* equivalent value for room temperature on "normal" board */

/****************************************************************
*
*       Enums
*
******************************************************************/
enum DATATYPES { SYSTEM_DATA, TRACK_DATA, VEHICLE_DATA, DRIVER_DATA, SESSION_DATA, LAP_DATA };

enum SCREEN_TYPE {
	GENERAL_CONFIG_SCREEN_TYPE,
	CONFIG_SCREEN_TYPE,
	GAUGES_SCREEN_TYPE,
	DRIVE_MODE_SCREEN_TYPE,
	INSTRUMENTS_MODE_SCREEN_TYPE,
	RACE_SETUP_SCREENS_TYPE,
	SYSTEM_SETTINGS_SCREENS_TYPE,
	VEHICLE_SETTINGS_SCREENS_TYPE,
	VEHICLE_SETTING_SCREEN_TYPE,
	TACH_SETUP_SCREENS_TYPE,
	INPUTS_SCREENS_TYPE,
	REVIEW_SESSIONS_SCREENS_TYPE,
	TRACK_SETTING_SCREEN_TYPE,
	TRACK_SETTINGS_SCREENS_TYPE,
	GEARING_SCREENS_TYPE,
	ANALOG_SETTINGS_SCREEN_TYPE,
	DIGITAL_SETTINGS_SCREEN_TYPE,
	ANALOG_NUMBER_ENTRY_STANDALONE,
	ANALOG_INPUTS_SCALE,
	GEAR_RATIOS,
	RACE_SETUP_STANDALONE,
	SFSCREENS_TYPE,
	TIRE_DIMENSIONS_SCREEN_TYPE,
	WAITING_FOR_SATELLITES_SCREEN_TYPE,
	DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE,
	WAITING_FOR_CAMERA_SCREEN_TYPE,
	WAITING_FOR_SF_SCREEN_TYPE,
	PERSONALIZE_REVIEW_SCREEN_TYPE,
	PERSONALIZE_ENTRY_SCREEN_TYPE,
	WAITING_FOR_STAGE_SCREEN_TYPE,
	WAITING_FOR_LAUNCH_SCREEN_TYPE,
	DRAG_AUTOX_GAUGE_SCREEN_TYPE,
	AERO_GRIP_SCREEN_TYPE,
	CAMERA_SELECTION_SCREEN_TYPE,
	CAMERA_DELAY_SCREEN_TYPE,
	TRAQDASH_CAMERA_SCREEN,
	HELP_SCREEN_TYPE,
	RECORD_SPEED_SCREEN_TYPE
};

enum COMPARISON_STATES { OFF, ABOVE, BELOW, EQUAL_TO };
enum DIGITAL_ALARM_ENABLE_STATES { DI_DISABLED = -1, DI_OFF, DI_ON };
// Analog input enable/filtering states
enum ANALOG_INPUT_FILTERING {
	FILTER_NONE = 1,
	FILTER_LOW = 5,
	FILTER_MEDIUM = 11,
	FILTER_HIGH = 21,
	FILTER_VERY_HIGH = 51
};

// cameras
#define CAMERA_NONE				0
#define CAMERA_SONY_HD			1
#define CAMERA_RACECAM_HD		2
#define CAMERA_CHASECAM			3
#define CAMERA_CHASECAM_LANC	4
#define CAMERA_SONY_CAMCORDER	5
#define CAMERA_SONY_VCR_HARD	6
#define CAMERA_SONY_VCR_SOFT	7
#define CAMERA_GOPRO_HD			8
#define CAMERA_REPLAY_HD		9			/* XD 1080 connected via TraqData HD2 and Replay cable */
#define CAMERA_REPLAY_XD_USB	10			/* XD 1080 connected via TraqData USB */
#define CAMERA_GOPRO_HD_PRO		11			/* GoPro Hero with TraqData Pro */
#define CAMERA_REPLAY_MINI		12			/* Mini connected via TraqData USB */
#define CAMERA_REPLAY_PRIMEX	13			/* Prime X connected via TraqData USB */
#define CAMERA_MOBIUS			14			/* Mobius connected via TraqData USB */
#define CAMERA_USB				17			/* TraqDash USB camera of any type. Note this cannot be sent to the SU */

#define	SU_CAMERA_MASK			0x1F
#define	TD_CAMERA_MASK			0xE0

#define	TD_CAM_MOBIUS			0x20
#define	TD_CAM_PRIMEX			0x40
#define	TD_CAM_MINI				0x60
#define	TD_CAM_NONE				0x00

// camera delay values
#define MOBIUS_DELAY	(0x80 + 40)	// default to camera enabled and 4.0 second delay
#define PRIME_X_DELAY	(0x80 + 70)	// default to camera enabled and 4.0 second delay
#define MINI_DELAY		(0x80 + 70)	// default to camera enabled and 4.0 second delay

// SU Modes
// NOTE: SOME CODE DEPENDS ON ORDER OF THESE ITEMS
typedef enum {
	WAITING,		// idle state
	WAITING_AUTOX,	// temp state for menu access
	WAITING_LAP,	// temp state for menu access
	WAITING_HILL,	// temp state for menu access
	WAITING_DRAG,	// temp state for menu access
	TIMING,			// recording - timing started
	DRIVE,			// recording point to point drive
	AUTOXGETFINISH,	// recording at an autocross, user presses select at finish
	HILLCLIMB,		// recording at a hillclimb
	LAPS,			// recording lap timing or qualifying
	GPSREC,			// 10 recording GPS information screen
	GPSRPM,			// recording RPM screen
	INPUTS,			// recording inputs screen
	GAUGEA0,		// gauge screens for individual inputs
	GAUGEA1,
	GAUGEA2,
	GAUGEA3,
	GAUGED4,
	GAUGED5,
	GFORCES,
	RALLY,			// 20 recording point to point rally with stages
	DRAGREADY,		// recording, waiting to launch at drag start
	AUTOXREADY,		// recording, waiting to launch at autocross start
	HILLCLIMBWAITSTART,	// recording, waiting to cross start/finish 1st time
	HILLCLIMBGETFINISH,	// recording, waiting for user to hit SEL at finish
	HILLCLIMBSTART,		// recording, looking for hillclimb start line
	HILLCLIMBFINISH,	// recording, looking for hillclimb finish line
	AUTOXFINISH,	// recording, looking for autocross finish line
	AUTOXHOLDTIME,	// showing finish time, still recording
	DRAGFINISH,		// looking for end of strip
	DRAGHOLDTIME,	// 30 showing finish time, still recording
	DRAGHOLDNUMS,	// showing dragrace numbers, still recording
	WAITSTART,		// recording - waiting to cross start/finish
	AUTOXSTAGE,		// non-recording waiting to stage at autocross start
	DRAGSTAGE,		// non-recording waiting to stage at dragrace start
	GPSINFO,		// non-recording gps information screen
	INSTRUMENT,		// non-recording tach and speedo screen
	COMPASS,		// non-recording digital compass
	CHKINPUTS,		// non-recording inputs screen
	REVIEW,			// 39 reviewing lap times

					// standalone sensor unit modes
	WAIT_COLLECT = 0xE0,	// 224 waiting for event to start collecting data
	COLLECTING,		// collecting and storing data
	ERASING,		// erasing memory
	WAIT4BACK,		// waiting for user to acknowledge message

					// software download modes
	START_DOWNLOAD = 0xF0,	// 240 causes SU or DU to clear flash and wait for software download packets
	DOWNLOAD_COMPLETE,		// causes SU or DU to verify sw received and ACK or NAK
							// after ACKing this mode change, SU or DU should wait for SW_DOWNLOAD command
	CANCEL_DOWNLOAD,			// cancels sw download and resume normal operation
					// startup and shutdown modes
	STARTUP = 0xFE,
	SHUTDOWN = 0xFF
} tmstatetype;


enum SESSION_TYPE {
	ROADRACE_TYPE = 'R',
	DRIVE_TYPE = 'D',
	AUTOCROSS_TYPE = 'A',
	QUARTERMILE_TYPE = 'Q',
	EIGHTHMILE_TYPE = 'E',
	THOUSANDFOOT_TYPE = 'T',
	HILLCLIMB_TYPE = 'H',
	NOT_VALID = 0
};

enum DRAG_DISTANCE { DD_1_4, DD_1000, DD_1_8 };

/****************************************************************
*
*       Screens
*
******************************************************************/
int ShowDialog(const char *sTitle, const char *sText, int x_org, int y_org);
void ExecGaugeTest(void);
int ShowDialog(const char *sTitle, const char *sText, int x_org, int y_org);

/****************************************************************
*
*       Windows management
*
******************************************************************/
void DisplayWindow(WM_HWIN _hNewWin);

/****************************************************************
*
*       Structures
*
******************************************************************/
typedef struct {
	int x;
	int y;
} Point;

extern GUI_CONST_STORAGE GUI_BITMAP bmSweep2;
//GUI_CONST_STORAGE GUI_BITMAP bmdashgas;3

#ifndef PI
#define PI 3.141592653589
#endif
#define DEGREES_TO_RADIANS(x) x * PI / 180.0
#define RADIANS_TO_DEGREES(x) x * 180.0 / PI

#define CIRCUM		40455.0										/* circumference of earth in km */
#define TODEGREE(A) ((A) * (180.0 / 2147483647.0))				/* converts from storage format to degrees */
#define LATMET 		(1000.0/360.0*CIRCUM)						/* meters per degree */
#define LONMET(B)	(1000.0/360.0*CIRCUM*cos((B)/57.29577951))	/* meters per degree, B must be current latitude */

#define	VERSION_SIZE	11

// For some reason they don't define orange in GUI.h!
#define	GUI_ORANGE			0x000080FF
#define GUI_PURPLE			0x00D000D0
#define GUI_OLIVEGREEN		0x00008080
#define GUI_SAGEGREEN		0x0000B080
#define GUI_MEDGREEN		0x0000B800	/* good on hw */
#define GUI_DARKBROWN		0x002A2A75
#define	GUI_OFFWHITE		0x00F0F0F0
// definition for list highlight bar
#ifdef _WINDOWS
#define	GUI_LISTBLUE	0x00FF653F
#else // hardware
#define	GUI_LISTBLUE	0x00FF5480
#endif

// gui.h also stops with listwheel 3!  These codes are available (they are the next IDs and were not used)
#define	GUI_ID_LISTWHEEL4		0x264
#define	GUI_ID_LISTWHEEL5		0x265
#define	GUI_ID_LISTWHEEL6		0x266
#define	GUI_ID_LISTWHEEL7		0x267
#define	GUI_ID_LISTWHEEL8		0x268
#define	GUI_ID_LISTWHEEL9		0x269
#define	GUI_ID_LISTWHEEL10		0x26A

#define	STD		0
#define	METRIC	1

#define NORMAL_FILENAME_SIZE	80				/* normal filename */
#define MAX_FILENAME_SIZE		150				/* big filename */
												/* session and lap filenames = path + date, track, driver, car, number.ext */
//
// Counters for multiSettings_2 variables - note - these cannot exceed their 4 bit field
//
#define	HELP_SPLASH_COUNT		4
#define	RACE_HELP_COUNT			4

// Personalization file name
#define	PERSONALIZE_FILE_NAME	"personalize"

// note: these match up with declarations inside the Sensor Unit
#define NAMELEN					16				/* length of text string for driver and track and vehicle */
#define IONAMELEN				12				/* length of text string for input name */
#define	MAX_UNITS				10				/* length of text string for input units */
#define	MAX_HILO_NAME			IONAMELEN		/* length of text string for digital hi/low state */


typedef struct {
	unsigned short numPages;					// for serial dataflash, how many flash pages
	unsigned short bytesPerPage;				// for serial dataflash, how many bytes per flash page
} DATAFLASH_STATUS_STRUCTURE;

typedef struct {
	char owner[OWNER_LEN];						// 20 name of Traqmate owner
	char phone[PHONE_LEN];						// 20 contact number for owner
	char wifiName[WIFI_LEN];					// 20 wifi name field
	char wifiPass[WIFI_LEN];					// 20 wifi password field
	unsigned int model;							// 4 model number, entered by Factory Cal = 1 for SU, 2 for DU, 3 for TraqDash
	unsigned int hwrev;							// 4 hardware revision * 100 (237 = 2.37), entered by Factory Cal
	unsigned int serno;							// 4 serial number, entered by Factory Cal
	unsigned int week;							// 4 week of year of manufacture, entered by Factory Cal
	unsigned int year;							// 4 year of manufacture, entered by Factory Cal
	unsigned int bootloaderSwRev;				// 4 revision of bootloader * 100. >135 allows TraqDash USB camera.
	unsigned int feature1;						// 4 used to enable future sw features
	unsigned int feature2;						// 4 used to enable future sw features
	unsigned int feature3;						// 4 used to enable future sw features
	unsigned int feature4;						// 4 used to enable future sw features
} UNIT_INFO_STRUCTURE;

typedef struct {
	char	version[VERSION_SIZE + 1];			// Version string always the first element in the structure
	int	AccelerometerSampleRate;
	int	GPSSampleRate;
	enum SCREEN_TYPE screentype;
	int	lastConfigScreen;
	int	lastGaugeScreen;
	int lastDriveScreen;
	int lastInstrumentsScreen;
	int	lastRaceSetupScreen;
	int	lastSystemSettingsScreen;
	int	lastVehicleSettingsScreen;
	int	lastTachSetupScreen;
	int	lastInputsScreen;
	int	lastReviewSessionsScreen;
	int	lastTrackSettingsScreen;
	int	lastGearingScreen;
	int lastGearRatioScreen;
	int lastSFScreen;
	int	lastAnalogInputSettingsScreen;
	int lastAnalogScaleScreen;
	int	lastDigitalInputSettingsScreen;
	int lastTireDimensionsScreen;
	int	touchScreenCalibrated;
	int	units;
	unsigned int	backlight;				// USER brightness setting 0 = dimmest, 100 = brightest - HW settings derived from this value
	char	autoAdjustOn;
	unsigned int	LEDBrightness;			// USER brightness setting 0 = dimmest, 100 = brightest - HW settings derived from this value
	unsigned int	lightLevelWhenIntensitySet;
	char	launchGForce;					// Launch g-force times 10.  Min = 1, 1G = 10, 2G = 20 (although we only use 1-10)
	char	lapMeter;						// predictive lap graph scale, default 1, 0 = 0.5, 1 = 1.0, 2 = 2.0, 3 = 5.0, 4 = 10.0
	signed char	timeZone;					// -12 to 12
	char	dstOn;
	char	autoOn;							// true if unit set to turn on with vehicle power
	char	autoRecord;						// true if unit set to auto record
	char	driver[NAMELEN+1];
	char	vehicle[NAMELEN+1];
	char	track[NAMELEN+1];
// KMC note: cannot use enumerations in data structures to be saved to disk because
// the PC compiler sets their size to 4 bytes but the IAR compiler makes them one byte.
// But for some reason screentype is okay.
//	DU2_STATE_TYPE lastSystemMode;			// Last System mode selected on previous power cycle
	char	lastSystemMode;					// Last System mode selected on previous power cycle
	char	lappingMode;					// true = lapping mode, false = qualifying mode
	int		radius;							// Radius, in kilometers, for track searching
	float	maxSpeedEver;					// max speed in kph achieved since last data reset
	unsigned int	maxRpmEver;				// max rpm collected since last data reset
	float	totalDistanceEver;				// in meters, cumulative until data reset
	float	totalTimeEver;					// in seconds, cumulative until data reset
	unsigned int driveTripTime;				// in msec since session since last Drive Mode trip reset
	float	driveTripDistance;				// in meters since last Drive Mode trip reset
//	enum SESSION_TYPE selectedSessionType;	// Last selected user session review type
	char selectedSessionType;				// Last selected user session review type
	char lastRaceDisplay;					// Last selected user data display for road race mode
	char lastDragDisplay;					// Last selected user data display for drag race mode
	char lastDriveDisplay;					// Last selected user data display for drive mode
	char lastAutoXDisplay;					// Last selected user data display for autocross mode
	char lastHillDisplay;					// Last selected user data display for hill climb mode
//	enum DRAG_DISTANCE dragDistance;		// Last seleted drag race distance (1/4 mile = 0, 1000ft = 1, 1/8 mile = 2)
	char dragDistance;						// Last seleted drag race distance (1/4 mile = 0, 1000ft = 1, 1/8 mile = 2)
	char disableDriveRecordMode;			// Enable (0) or disable (1) record mode during DRIVE (ie 0 == DRIVE_MODE, 1 == INSTRUMENTS_MODE)
// Items added for sysData version 00.02
	int  calibrationTemperature;			// temperature in C at which display calibration was performed
	int  calibrationTempReading;			// actual temperature reading read from HW at the above temperature
	signed int LEDtempOffset;				// Calculated temperature offset to be applied to LED temperature readings
	char navigatedToReviewAfterRace;		// True if ended a race session and went to review screen - cleared once leave review screen
	unsigned int multiSettings_1;			// Spare integers for testing/prototyping new features without changing memory map
											// bits 8 - 31: Upper 24 bits still unused and available for use
											// bits 0 - 7: lowest 8 bits used for configurable gauge item
	unsigned int multiSettings_2;			// bits 16 - 31: upper 16 bits = record speed in integer kph
											// bits 4 - 15: 12 bits record stop time in seconds
#ifdef	INCLUDE_RACE_HELP
	undefined bits!							// bits 4 - 7: race mode help counter NOTE CONFLICT WITH RECORD STOP TIME
#endif
											// bits 0 - 3: lowest 4 bits = help splash screen startup cycle counter
// Last item always the checksum
	int	checksum;
} SYS_DATA_STRUCTURE;

//
// data collection modes
//
typedef  enum {
	TQM_MODE_LAPPING = 0,						// recording modes for TQM file
	TQM_MODE_AUTOCROSS = 1,
	TQM_MODE_QTR_MILE = 2,
	TQM_MODE_EIGHTHMILE = 3,
	TQM_MODE_HILLCLIMB = 4,
	TQM_MODE_DRIVE = 5,
	TQM_MODE_THOUSAND = 6
} TQM_RECORDING_MODES;

typedef struct {
	unsigned short duSwRev;						// 2 software rev * 100
	unsigned short duDataRev;					// 2 data rev for matching up with SU messages
	unsigned char duHwRev;						// 1 hardware rev * 100
	UNIT_INFO_STRUCTURE unitInfo;				// 120 personalization data and serial number
	DATAFLASH_STATUS_STRUCTURE dataflash[2];	// 4 status of onboard serial dataflash memory
	DU2_STATE_TYPE systemMode;					// 4 overall operating mode of the DU
	TQM_RECORDING_MODES recordingMode;			// 1 recording mode to save into .tqm file
	unsigned char enableSerialNumber;			// 1 true if user can change serial number and build date
	double	workingStartLineLat;				// 8 working copy shared between tasks. not saved.
	double	workingStartLineLong;				// 8
	float	workingStartLineAlt;				// 4
	float	workingStartLineHeading;			// 4
	double	workingFinishLineLat;				// 8
	double	workingFinishLineLong;				// 8
	float	workingFinishLineAlt;				// 4
	float	workingFinishLineHeading;			// 4
	unsigned int sdCardInserted;				// 4 true if card inserted, false if not
	unsigned int sdCardInitialized;				// 4 true if card initialized, false if not
	unsigned int enableTempCalibration;			// 4 allow setting of LED temperature calibration
	unsigned int wifiCardInserted;				// 4 true if Toshiba FlashAir card inserted
	unsigned int wifiEnabled;					// 4 true if bootloader bit enabling WiFi operation is set
	unsigned char recordingTimedout;			// 1 set to true when the recording times itself out
	unsigned char waitingForRecordSpeed;		// 1 set to true if there we are in a recording mode and have not reached speed
	int timeToStopRecording;					// 4 amount of standing still seconds left before stop recording. 0 = not in countdown
} UNSAVED_SYS_DATA_STRUCTURE;

typedef struct {
	int scaleStart, scaleEnd;
	int greenStart, greenEnd;
	int upperYellowStart;
	int upperRedStart;
	int lowerYellowStart;
	int lowerRedStart;
} GAUGE_STRUCTURE;

typedef struct {
	char Name[NAMELEN+1];					// User-assigned name of input
	char Units[MAX_UNITS];					// User-asigned name of units
	unsigned char enabled;					// Enable or disable state of this input. Also indicates filtering level, 1=none, 2 or more indicates averaging over that many samples
//	enum COMPARISON_STATES lowerAlarmTriggerComparison;	// Trigger above, below or equal to trigger point
	char lowerAlarmTriggerComparison;		// For lower alarm range, trigger below, equal, or none to trigger point
	char upperAlarmTriggerComparison;		// For upper alarm range, trigger above, equal, or none to trigger point
	float lowerAlarmTriggerLevel;			// Trigger point in user-units
	float upperAlarmTriggerLevel;			// Trigger point in user-units
	char inputRange;						// 5, 10 or 20 volt range (value = 5, 10, 20)
	float lowVoltagePoint;					// Low intercept point volts
	float highVoltagePoint;					// High intercept point volts
	float lowValuePoint;					// Low user-defined data point
	float highValuePoint;					// High user-defined data point
	char EnableLED1;						// Illuminate Upper LED when alarm triggers
	char EnableLED2;						// Illuminate Lower LED when alarm triggers
} ANALOG_INPUT_STRUCTURE;

typedef struct {
	char Name[NAMELEN+1];					// User-assigned name of input
	char Units[MAX_UNITS];					// User-asigned name of units
	char enabled;							// Enable or disable state of this input
//	enum COMPARISON_STATES alarmTriggerComparison;	// Trigger above, below or equal to trigger point
	char alarmTriggerComparison;			// For lower alarm range, trigger below, equal, or none to trigger point
	float alarmTriggerLevel;				// Trigger point in user-units
	char inputRange;						// 5, 10 or 20 volt range (value = 5, 10, 20)
	float lowVoltagePoint;					// Low intercept point volts
	float highVoltagePoint;					// High intercept point volts
	float lowValuePoint;					// Low user-defined data point
	float highValuePoint;					// High user-defined data point
	char EnableLED1;						// Illuminate Upper LED when alarm triggers
	char EnableLED2;						// Illuminate Lower LED when alarm triggers
} V0_ANALOG_INPUT_STRUCTURE;

typedef struct {
	char Name[NAMELEN+1];					// User-assigned name of input
	char HighName[MAX_HILO_NAME+1];
	char LowName[MAX_HILO_NAME+1];
	char enabled;							// Enable or disable state of this input
//	enum DIGITAL_ALARM_ENABLE_STATES alarmTriggerState;	// DISABLED, OFF, ON
	signed char alarmTriggerState;			// DISABLED, OFF, ON
	char EnableLED1;						// Illuminate Upper LED when alarm triggers
	char EnableLED2;						// Illuminate Lower LED when alarm triggers
} DIGITAL_INPUT_STRUCTURE;


//
// Definition of the complete vehicle ddescription.  This structure is saved to disk.
// Note: whenever making changes to VEHICLE_DATA_STRUCTURE you MUST also make the corresponding
// change in MINI_VEHICLE_DATA_STRUCTURE as it is a subset of this data.
//
typedef struct {
	char	version[VERSION_SIZE + 1];			// Version string always the first element in the structure
	unsigned char	usbCameraDelay;				// top bit = usb camera enable, lower 7 bits = delay * 10 (78 is 7.8 seconds, max 9.9)
	char	engineCylinders;
	GAUGE_STRUCTURE tach;
	float	gearRatios[NUMBER_OF_GEARS];
	float	diffRatio;
	float	tireCircumference;					// currently inches, probably should be centimeters
	int		treadWidth;
	int		wheelDiameter;						// inches, as is customary in industry
	int		aspectRatio;
	unsigned char	userEnteredTireDimensions;	// flag to indicate that user entered the tire dimensions rather than circumference
	int		weight;
	float	windDrag;
	float	rollingResistance;
	unsigned char	camera;						// camera selected, default = none
	ANALOG_INPUT_STRUCTURE analogInput[NUMBER_OF_ANALOGS];
	DIGITAL_INPUT_STRUCTURE digitalInput[NUMBER_OF_DIGITALS];
// Last item always the checksum
	int	checksum;
} VEHICLE_DATA_STRUCTURE;

typedef struct {
	char	version[VERSION_SIZE + 1];			// Version string always the first element in the structure
	unsigned char	usbCameraDelay;
	char	engineCylinders;
	GAUGE_STRUCTURE tach;
	float	gearRatios[NUMBER_OF_GEARS];
	float	diffRatio;
	float	tireCircumference;					// currently inches, probably should be centimeters
	int		treadWidth;
	int		wheelDiameter;						// inches, as is customary in industry
	int		aspectRatio;
	unsigned char	userEnteredTireDimensions;
	int		weight;
	float	windDrag;
	float	rollingResistance;
	unsigned char	camera;								// camera selected, default = gopro hd
	V0_ANALOG_INPUT_STRUCTURE analogInput[NUMBER_OF_ANALOGS];
	DIGITAL_INPUT_STRUCTURE digitalInput[NUMBER_OF_DIGITALS];
// Last item always the checksum
	int	checksum;
} V0_VEHICLE_DATA_STRUCTURE;

//
// The following (MINI_VEHICLE_DATA_STRUCTURE) is a SUBSET of VEHICLE_DATA_STRUCTURE and MUST maintain element
// alignment with VEHICLE_DATA_STRUCTURE as a direct memory copy is used to initialize all the vehicle data
// files at initialization.  This structure contains the data unique to each vehicle.
//

typedef struct {
	unsigned char	usbCameraDelay;
	char			engineCylinders;
	GAUGE_STRUCTURE tach;
	float			gearRatios[NUMBER_OF_GEARS];
	float			diffRatio;
	float			tireCircumference;
	int				treadWidth;
	int				wheelDiameter;
	int				aspectRatio;
	unsigned char	userEnteredTireDimensions;
	int				weight;
	float			windDrag;
	float			rollingResistance;
} MINI_VEHICLE_DATA_STRUCTURE;

// this is the structure that is used to create the default vehicle list
typedef struct {
	char *vehicleName;
	MINI_VEHICLE_DATA_STRUCTURE carData;
} DEFAULT_VEHICLE_STRUCTURE;

typedef struct {
	char	version[VERSION_SIZE + 1];			// Version string always the first element in the structure
// Last item always the checksum
	int	checksum;
} DRIVER_DATA_STRUCTURE;

typedef struct {
	char	version[VERSION_SIZE + 1];			// 12 Version string always the first element in the structure
	double	startLineLat;						// 8
	double	startLineLong;						// 8
	float	startLineAlt;						// 4
	float	startLineHeading;					// 4
	double	finishLineLat;						// 8
	double	finishLineLong;						// 8
	float	finishLineAlt;						// 4
	float	finishLineHeading;					// 4
	unsigned char courseType;					// 1 = 4
	unsigned char laptimeHold;					// 1 = 4
	unsigned int bestLapEver;					// 4 lap time associated with the sectors in msec
	unsigned int numberOfSectors;				// 4 number of sectors stored. Will vary by track = tracklength / sectorlength
	unsigned int sectorTimes[NUMBER_OF_SECTORS];	// 5200 = 4 * 1300 predictive lap sectors
// Last item always the checksum
	int	checksum;								// 4
} TRACK_DATA_STRUCTURE;							// 5284

typedef struct {
	char	version[VERSION_SIZE + 1];			// Version string always the first element in the structure
	unsigned char suPresent;					// whether or not we are hooked to an SU
	unsigned char traqDataConnected;			// whether io interface is present
	tmstatetype tmState;						// where in the state machine
	unsigned short suSwRev;						// software rev from SU * 100
	unsigned short suDataRev;					// data rev from SU
	unsigned char suHwRev;						// hardware rev from SU * 100
	unsigned char buildDateWeek;				// su build date week number (1 - 52)
	unsigned char buildDateYear;				// su build date year (last 2 digits since 2000)
	unsigned int serialNumber;					// su serial number
	char auto_on;								// true if auto-on is enabled in SU
// Last item always the checksum
	int	checksum;
} SENSOR_UNIT_STATUS_STRUCTURE;

typedef struct {								// structure to hold intermediate and current information for predictive laptimes
	unsigned int reference;						// reference lap timestamp for a sector boundary in msec since s/f
	unsigned int current;						// current lap timestamp for a sector boundary in msec since s/f
} SECTOR_COMPARISON_TYPE;

typedef union {
	SYS_DATA_STRUCTURE sysdata;
	TRACK_DATA_STRUCTURE trackdata;
	VEHICLE_DATA_STRUCTURE vehicledata;
	DRIVER_DATA_STRUCTURE driverdata;
} ALL_DATA_UNION;

typedef struct {
	char profileName[10];		// text name for printout
	U32 lastStartTime;			// last time function was started
	U32 numberOfStarts;			// number of times function was started
	U32 maxExecTime;			// longest execution time for this function
	U32 totalExecTime;			// running total of execution time for this task
} PROFILESTRUCT;

extern SYS_DATA_STRUCTURE sysData;
extern UNSAVED_SYS_DATA_STRUCTURE unsavedSysData;
extern VEHICLE_DATA_STRUCTURE vehicleData;
extern DRIVER_DATA_STRUCTURE driverData;
extern TRACK_DATA_STRUCTURE trackData;
extern SENSOR_UNIT_STATUS_STRUCTURE suData;
extern SECTOR_COMPARISON_TYPE sectors[NUMBER_OF_SECTORS];
extern PROFILESTRUCT taskProfile[];

enum POPUP_TYPES {
	INFORMATIONAL_POPUP,
	WARNING_POPUP,
	ERROR_POPUP,
	PROGRAMMING_POPUP
};

extern void SetPopupDefaults(enum POPUP_TYPES type);

#ifdef _WINDOWS
// Define number of tasks on windows side as this is used by ProfilerScreen.c and FlashReadWrite.c
// for profiler-related functions - even on the PC side.
#define	NUMBER_OF_TASKS	7
#endif


#ifdef __cplusplus
}
#endif

#endif
