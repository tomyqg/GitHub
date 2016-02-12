
#include "WM.h"
#include "commondefs.h"
#include "DataHandler.h"
#include "LEDHandler.h"
#include "DUII.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ConfigScreens.h"
#include "SFScreens.h"
#include "GeneralConfig.h"
#include "HelpScreens.h"
#include "VehicleSettingsScreens.h"

#ifndef _WINDOWS
#include "RTOS.H"
#include "datadecode.h"
#include "SPI_API.h"
#include "message.h"
#include "opcode.h"
#else
#include <time.h>
#endif

//
// Public data declarations
//
GlobalDataType DataValues[NUMBER_OF_GLOBALS] = { 0 };


//
// Constants
//
// Data refresh at 40Hz
#define	DATA_REFRESH_TIME		50
#define	TIME_RECT_CENTER_X		70
#define	TIME_RECT_TOP_Y			9
#define	CAMERA_X_POS			378
#define	SD_X_POS				414
#define	SU_SIGNAL_X_POS			442
#define	TIME_INCREMENT			DATA_REFRESH_TIME
#define	SIMULATION_LAP_TIME		82135
// Since RPM is unsigned we cannot watch for < 0 underflow so watch for rediculous RPM overflow
#define	MAX_POSSIBLE_RPM		25000
#define MAX_ANIMATION_CYCLES	4


//
//	Global Resources
//
#ifndef _WINDOWS
extern OS_RSEMA SEMA_GlobalData;			// resource semaphore around global data
extern OS_MAILBOX MB_SDcardRW;				// mailbox to SD card write task
extern OS_Q Queue_tqm;						// queue of data to be written to .tqm file
#endif

//
// External Code References
//
int GetRecordSpeed(void);

//
// External data declarations
//
extern GUI_CONST_STORAGE GUI_BITMAP bmNoSignal;
extern GUI_CONST_STORAGE GUI_BITMAP bmSignal1;
extern GUI_CONST_STORAGE GUI_BITMAP bmSignal2;
extern GUI_CONST_STORAGE GUI_BITMAP bmSignal3;
extern GUI_CONST_STORAGE GUI_BITMAP bmSignal4;
extern GUI_CONST_STORAGE GUI_BITMAP bmSignal5;
extern GUI_CONST_STORAGE GUI_BITMAP bmCameraReadyIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmCameraNotReadyIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmCameraRecordIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmCameraBlankIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmNoSUIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmSUGoodIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmInfoButton1;
extern GUI_CONST_STORAGE GUI_BITMAP bmInfoButton2;
extern GUI_CONST_STORAGE GUI_BITMAP bmInfoButton3;
extern GUI_CONST_STORAGE GUI_BITMAP bmInfoButton4;
extern GUI_CONST_STORAGE GUI_BITMAP bmInfoButton5;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenSD;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedSD;
extern GUI_CONST_STORAGE GUI_BITMAP bmWhiteSD;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowSD;
extern GUI_CONST_STORAGE GUI_BITMAP bmLockedSD;
extern GUI_HSPRITE hSUStatusSprite, hCameraSprite, hSignalStrengthSprite;

//
// Local defines and structure definitions
//
GUI_CONST_STORAGE GUI_BITMAP* SignalStrengthGraphics[] = {&bmNoSignal, &bmSignal1, &bmSignal2, &bmSignal3, &bmSignal4, &bmSignal5 };

typedef struct {
	int Dir;				// -1 = don't change, 0 = up, 1 = down
	float Max;				// upper limit
	float Min;				// lower limit
	float Inc;				// increment
	float CurrentValue;
} DummyDataType;

DummyDataType DummyDataValues[NUMBER_OF_GLOBALS] = {
//		{	dir,	max,		min,		inc,	value	}
		{	1,		1.5F,		-1.2F, 		0.02F,	0.0F	},		//	X_G,				// f in Gs
		{	0,		1.7F,		-1.6F,		0.015F,	0.0F	},		//	Y_G,				// f in Gs
		{	0,		2.0F,		-2.0F,		0.40F,	1.0F	},		//	Z_G,				// f in Gs
		{	-1,		0,			0,			0,		0.0F	},		//	MAX_X_G,			// f, max g experienced in x direction since session start
		{	-1,		0,			0,			0,		0.0F	},		//	MAX_Y_G,			// f, max g experienced in y direction since session start
		{	-1,		0,			0,			0,		0.0F	},		//	MAX_Z_G,			// f, max g experienced in z direction since session start
		{	-1,		0,			0,			0,		0.0F	},		//	MIN_X_G,			// f, min g experienced in x direction since session start
		{	-1,		0,			0,			0,		0.0F	},		//	MIN_Y_G,			// f, min g experienced in y direction since session start
		{	-1,		0,			0,			0,		0.0F	},		//	MIN_Z_G,			// f, min g experienced in z direction since session start
		{	1,		1.5F,		-1.2F, 		0.02F,	0.0F	},		//	X_G-DAMPED,			// f in Gs
		{	0,		1.7F,		-1.6F,		0.015F,	0.0F	},		//	Y_G_DAMPED,			// 10 f in Gs
		{	0,		2.0F,		2.0F,		0.010F,	1.0F	},		//	Z_G_DAMPED,			// f in Gs	
		{	0,		34.151F,	34.149F,	0.00001F,0.0F	},		//	LATITUDE,			// d, gps lat out to 6 decimal places
		{	0,		-83.813F,	-83.815F,	0.00001F,0.0F	},		//	LONGITUDE,			// d, gps lon out to 6 decimal places
		{	-1,		0,			0,			0,		0.0F	},		//	SPEED,				// f, stored in kph
		{	0,		359.0,		0.0,		1.00F,	0.0F	},		//	HEADING,			// f, in degrees, 0.0 - 360.0
		{	0,		359.0,		0.0,		0.50F,	0.0F	},		//	HEADING_DAMPED,		// f, in degrees, 0.0 - 360.0
		{	0,		10000,		0,			0.1F,	0.0F	},		//	GPS_WEEKS,			// u, number of weeks since GPS start = Jan 06, 1980
		{	0,		1000,		0,			0.1F,	0.0F	},		//	GPS_TIME,			// u, number of msec since GPS start + number of weeks
		{	0,		600.0,		200.0,		0.01F,	0.0F	},		//	ALTITUDE,			// f, in meters
		{	0,		4.4F,		2.5F,		0.0001F,0.0F	},		//	GPS_DOP,			// f gps dilution of precision, range 0.0 - 100.0
		{	0,		16.0,		0.0,		0.05F,	0.0F	},		//	GPS_NUMSAT,			// 20 u, number of satellites being monitored, 0 = no GPS Lock
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	MIN_SATS,			// u, minimum number of satellites seen during current session
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	MAX_SATS,			// u, maximum number of satellites seen during current session
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	MIN_DOP,			// f, minimum dop number seen during current session
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	MAX_DOP,			// f, maximum dop number seen during current session
		{	-1,		0,			0,			0,		0.0F	},		//	MAX_SPEED,			// f, max speed in kph achieved since session start
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	MIN_ALTITUDE,		// f, in meters
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	MAX_ALTITUDE,		// f, in meters
		{	-1,		1.0F,		1.0F,		0.025F,	0.0F	},		//	LAP_DISTANCE,		// f, in meters, since passing s/f
		{	0,		99999.0F,	0.0F,		1.0F,	0.0F	},		//	TOTAL_DISTANCE,		// f, in meters, since session started
		{	0,		1.2F,		-0.6F,		0.003F,	0.0F	},		//	PREDICTIVE_LAP,		// 30 f, pace ahead or behind in seconds
		{	0,		0.0F,		0.0F,		0.0F,	0.0F	},		//	LAP_TIME,			// u, in msec since passing s/f
		{	0,		123000,		0,			25,		0.0F	},		//	LAST_LAP_TIME,		// u, in msec since passing s/f
		{	-1,		0,			0,			0,		0.0F	},		//	ELAPSED_TIME,		// u, in msec since session started
		{	-1,		0,			0,			0,		0.0F	},		//	ELAPSED_RACE_TIME,	// u, in msec since first passing s/f or launch
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	LAP_NUMBER,			// i, lap we are currently on. 0 = s/f not yet found, -1 = not in lapping mode

		{	1,		120.0F,		80.0F,		0.001F, 80.0F	},		//	VMIN,				// f, last velocity trough
		{	1,		300.0F,		190.0F,		0.001F,	200.0F	},		//	VMAX,				// f, last velocity peak
		{	-1,		0,			0,			0,		0.0F	},		//	SPEED_PEAKS,		// f, Vmin in corners (negative number), Vmax on straights (positive), in kph
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	ZERO_SIXTY,			// u, in msec, 0-60 mph time for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	ZERO_HUNDRED,		// 40 u, in msec, 0-100 mph time for drag racing modes	
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	SIXTY_FOOT,			// u, in msec, 60' time for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	THREE_THIRTY_FOOT,	// u, in msec, 330' time for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	THOUSAND_FOOT,		// u, in msec, 1000' time for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	EIGHTH_MILE,		// u, in msec, 1/8 mile time for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	QUARTER_MILE,		// u, in msec, 1/4 mile time for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	SIXTY_TRAP,			// f, in kph, 60' trap speed for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	THREE_THIRTY_TRAP,	// f, in kph, 330' trap speed for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	THOUSAND_TRAP,		// f, in kph, 1000' trap speed for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	EIGHTH_TRAP,		// f, in kph, 1/8 mile trap speed for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	QUARTER_TRAP,		// 50 f, in kph, 1/4 mile trap speed for drag racing modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	REACTION,			// u, in msec, reaction time for drag racing modes
		{	-1,		0,			0,			0,		0.0F	},		//	VEHICLE_RPM,		// i, rpm, -1 = undefined
		{	0,		4.9F,		0.0F,		0.005F,	0.0F	},		//	ANALOG_INPUT_0,		// f, analog input in scaled value DEMO OIL PRESSURE
		{	0,		4.5F,		0.1F,		0.005F,	0.0F	},		//	ANALOG_INPUT_1,		// f, analog input in scaled value DEMO WATER TEMP	
		{	0,		1.0F,		0.0F,		0.003F,	0.0F	},		//	ANALOG_INPUT_2,		// f, analog input in scaled value DEMO BATTERY VOLTAGE
		{	0,		8.7F,		2.0F,		0.005F,	0.0F	},		//	ANALOG_INPUT_3,		// f, analog input in scaled value DEMO FUEL
		{	0,		1.5,		0.5,		0.01F,	0.0F	},		//	DIGITAL_INPUT_4,	// u, digital input, 0 = OFF state, 1 = ON state
		{	0,		1.5,		0.5,		0.005F,	0.0F	},		//	DIGITAL_INPUT_5,	// u, digital input, 0 = OFF state, 1 = ON state
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MAX_A_0,			// f, max value recorded for input during session
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MAX_A_1,			// 60 f, max value recorded for input during session	
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MAX_A_2,			// f, max value recorded for input during session
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MAX_A_3,			// f, max value recorded for input during session
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MIN_A_0,			// f, min value recorded for input during session
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MIN_A_1,			// f, min value recorded for input during session	
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MIN_A_2,			// f, min value recorded for input during session
		{	-1,		1.0,		1.0,		0.025F,	0.0F	},		//	MIN_A_3,			// f, min value recorded for input during session
		{	0,		1.0,		1.0,		0.025F,	0.0F	},		//	MAX_RPM,			// i, maximum rpm reading since session started

		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	TOTAL_HIGHREVS,		// u, total number of high revs ( > red line < max rpm ) since session started
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	TOTAL_OVERREVS,		// u, total number of overrevs ( > max rpm ) since session started

		{	0,		14.3,		10.5,		0.001F,	12.0F	},		//	VEHICLE_12V,		// 70 f, vehicle battery voltage
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	HORSEPOWER,			// f, in Watts, for all modes
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	TORQUE,				// f, in N-m, for all modes
		{	-1,		0,			0,			0,		0.0F	},		//	GEAR,				// i, 0 = not moving, -1 = undefined
		{	0,		3.5F,		0.0,		0.010F,	0.0F	},		//	CAMERA_STATUS,		// i, 0 = camera not ready, 1 = camera ready, 2 = camera recording, -1 = no camera
		{	-1,		0.5F,		0.5F,		0.0,	0.5F	},		//	AUTOX_DRAG_LAUNCH,	// f, gs encountered during launch of auto-x or drag mode
		{	0,		20.0F,		30.0F,		0.001F,	0.0F	},		//	SU_TEMPERATURE		// f, core temp of SU - 50 degrees F
		{	-1,		0.0,		0.0,		0.0F,	0.0F	},		//	HOLDING,			// u = 1 = holding, 0 = not holding
		{	0,		2.95F,		0.0,		0.005F,	2.0F	},		//	BEST_LAP,			// 78 u = 2 = best lap ever, 1 = best lap of session, 0 = not best lap
		{	-1,		0.0F,		0.0F,		0.0F,	0.0F	},		//	COMBINED_G,			// f in Gs	
		{	0,		123000,		0,			25,		0.0F	}		//	BEST_SESSION_LAP,	// u, in msec best time so far in session
}; // DummyDataValues

//
// Local function prototypes
//
float CalcSpeed(void);
#ifdef _WINDOWS
void dataTickCallback(WM_MESSAGE * pMsg);
#endif

//
// Local variable declarations
//
#ifdef _WINDOWS
static long hDataUpdateTimer;
#endif
static char SDBusy = false;
unsigned int minSimRPM;
int maxSimGear;
int animationIndex, animationCycles;
int newScreen = true;
int hasInfoKey;
const GUI_BITMAP *pAnimation;


GlobalDataType GetRawValue(GlobalDataIndexType dataType) {
	GlobalDataType retval;

	// request use of global data
	OS_Use(&SEMA_GlobalData);

	// get value from table
	retval = DataValues[dataType];
	
	// relinquish global data
	OS_Unuse(&SEMA_GlobalData);
	
	return(retval);
} // GetRawValue

GlobalDataType GetValue(GlobalDataIndexType dataType) {
	GlobalDataType retval;

	// request use of global data
	OS_Use(&SEMA_GlobalData);

	// get value from table
	retval = DataValues[dataType];

	// relinquish global data
	OS_Unuse(&SEMA_GlobalData);

	switch (dataType) {
		// Convert kilometers to miles
		case SPEED:
		case MAX_SPEED:
		case SPEED_PEAKS:
		case VMIN:
		case VMAX:
		case SIXTY_TRAP:
		case THREE_THIRTY_TRAP:
		case THOUSAND_TRAP:
		case EIGHTH_TRAP:
		case QUARTER_TRAP:
			if (sysData.units == STD)
				retval.fVal *= KILOMETERS_TO_MILES;
			break;
		
		// round off rpm to nearest 20
		case VEHICLE_RPM:
			if ((retval.iVal = retval.iVal - retval.iVal % 20) < 0)		// Not that iVal IS signed so we can watch for underflow here!
				retval.iVal = 0;
			break;

		// convert meters to feet
		case LAP_DISTANCE:
		case TOTAL_DISTANCE:
		case ALTITUDE:
		case MIN_ALTITUDE:
		case MAX_ALTITUDE:
			if (sysData.units == STD)
				retval.fVal *= METERS_TO_FEET;
			break;

		case SU_TEMPERATURE:
			if (suData.suHwRev < 200) {		// only do this if we are not connected to 3 axis SU2
				if (sysData.units == STD)
					// convert C to F	
					retval.fVal = (retval.fVal * 9.0 / 5.0) + 32.0;
			} // if
			else
				retval.fVal = 0.0;
			break;
			
		case ANALOG_INPUT_0:		// note inputs are scaled to users units so no metric conversion
		case ANALOG_INPUT_1:		
		case ANALOG_INPUT_2:
		case ANALOG_INPUT_3:
		case MAX_A_0:
		case MAX_A_1:
		case MAX_A_2:
		case MAX_A_3:
		case MIN_A_0:
		case MIN_A_1:
		case MIN_A_2:
		case MIN_A_3:
			{
			float tempfloat;
			int i;
			
			// create index into scaling values so we can use one general routine
			if (dataType <= ANALOG_INPUT_3)
				i = dataType - (int) ANALOG_INPUT_0;	// analog value
			else if (dataType <= MAX_A_3)
				i = dataType - (int) MAX_A_0;			// max analog value
			else
				i = dataType - (int) MIN_A_0;			// min analog value

			// check for bad ranges
			if (vehicleData.analogInput[i].highVoltagePoint == vehicleData.analogInput[i].lowVoltagePoint)
				retval.fVal = 0.0;
			else {
				// snap values to rails
//					if (retVal < vehicleData.analogInputs[i].lowValuePoint;
//						tempfloat = 0.0;
//					else if (retVal > vehicleData.analogInputs[i].highValuePoint)
//						tempfloat = 1.0;
//					else
					// scale as percent of usable range
					tempfloat = (retval.fVal - vehicleData.analogInput[i].lowVoltagePoint) /
						(vehicleData.analogInput[i].highVoltagePoint - vehicleData.analogInput[i].lowVoltagePoint);

				// scale as actual units to be measured
				retval.fVal = tempfloat * (vehicleData.analogInput[i].highValuePoint - vehicleData.analogInput[i].lowValuePoint) + vehicleData.analogInput[i].lowValuePoint;
			} // else
			break;
		} // case
		
		case LAP_TIME:
			if (DataValues[LAP_NUMBER].uVal > 1) {
				if (retval.iVal < (trackData.laptimeHold * 1000)) {
					// request use of global data
					OS_Use(&SEMA_GlobalData);
				
					// get value from table
					retval.uVal = DataValues[LAST_LAP_TIME].uVal;
				
					// relinquish global data
					OS_Unuse(&SEMA_GlobalData);
				} // if
			} // if
			if (retval.iVal < 0)
				retval.iVal = 0;
		break;
		
		case HOLDING:
			{
				unsigned int lapnum;
				float laptime;

				// request use of global data
				OS_Use(&SEMA_GlobalData);
				
				// get values from table
				lapnum = DataValues[LAP_NUMBER].uVal;
				laptime = DataValues[LAP_TIME].fVal;
				
				// relinquish global data
				OS_Unuse(&SEMA_GlobalData);

				if (lapnum > 1) {
					if (laptime < (trackData.laptimeHold * 1000))
						retval.uVal = true;			// holding
					else retval.uVal = false;		// not holding
				}
				else retval.uVal = false;			// not holding
			}
			break;

		case COMBINED_G:
			retval.fVal = sqrt((DataValues[X_G_DAMPED].fVal * DataValues[X_G_DAMPED].fVal) + (DataValues[Y_G_DAMPED].fVal * DataValues[Y_G_DAMPED].fVal));
			break;

		case DIGITAL_INPUT_4:
		case DIGITAL_INPUT_5:
		default:
		// no processing necessary
			break;
	} // switch

	return retval;
} // GetValue

#define	INCREASING	true
#define	DECREASING	false
#define	SPEED_INCREMENT	0.3			// was .3, tried .25
//#define	SPEED_DECREMENT	1.062		// make bigger if speed creeps up, make smaller if speed creeps down
#define	SPEED_DECREMENT	1.100		// make bigger if speed creeps up, make smaller if speed creeps down
#define	RPM_INCREMENT	23
#define	RPM_DECREMENT	92
#define	SHIFT_RPM		530
#define	DISTANCE_INCREMENT	1.0
int speedDirection = INCREASING;
int shifting = false;
unsigned char metRecordSpeedYet;

void SimStartSession(void)
{
	int i;

	DataValues[SPEED].fVal = 0;
	DataValues[SPEED_PEAKS].fVal = 0;
	minSimRPM = (unsigned int)(vehicleData.tach.scaleEnd * 0.7);			// Lowest RPM in simulated drive
	DataValues[VEHICLE_RPM].uVal = minSimRPM;
	DataValues[MAX_RPM].uVal = 0;
	DataValues[ELAPSED_TIME].uVal = 0;
	DataValues[ELAPSED_RACE_TIME].uVal = 0;
	DataValues[LAP_NUMBER].iVal = 0;
	DataValues[LAP_TIME].uVal = 0;
	DataValues[BEST_SESSION_LAP].uVal = 0;
	DataValues[GEAR].iVal = 1;
	DataValues[LAP_DISTANCE].fVal = 0.0F;
	speedDirection = INCREASING;
	// See how may gears to allow for this vehicle
	for (i = 0; i < NUMBER_OF_GEARS; i++)
	{
		if (vehicleData.gearRatios[i] == 0)
			break;
	}
	maxSimGear = i + 1;

	unsavedSysData.waitingForRecordSpeed = TRUE;
	metRecordSpeedYet = FALSE;
}

//
// Test code - dummy timer tick to change data.  The method of updating the data is fake,
// but the method of notifying the screens about the new data is real.
//
void DummyTick(void) {
	static unsigned char dummy_initialized = 0;
	int i;

	// Simulate time data, lap number and other non-cycling data
	DataValues[ELAPSED_TIME].uVal += TIME_INCREMENT;
	DataValues[ELAPSED_RACE_TIME].uVal += TIME_INCREMENT;
	DataValues[LAP_DISTANCE].fVal += DISTANCE_INCREMENT;
	if ((DataValues[LAP_TIME].uVal += TIME_INCREMENT) > SIMULATION_LAP_TIME)
	{
		DataValues[LAP_NUMBER].iVal += 1;
		DataValues[LAST_LAP_TIME].uVal = DataValues[LAP_TIME].uVal;
		if (0 == DataValues[BEST_SESSION_LAP].uVal || DataValues[LAST_LAP_TIME].uVal < DataValues[BEST_SESSION_LAP].uVal)
			DataValues[BEST_SESSION_LAP].uVal = DataValues[LAST_LAP_TIME].uVal;	
		DataValues[LAP_TIME].uVal = 0;
	}

	// Simulate speed and gears
	if (speedDirection == INCREASING)
	{
		if (shifting)
		{
			//int lowerLim;

			if ((DataValues[VEHICLE_RPM].uVal -= SHIFT_RPM) > MAX_POSSIBLE_RPM)		// Watch for overflow of unsigned int
				DataValues[VEHICLE_RPM].uVal = 0;
			if (DataValues[VEHICLE_RPM].uVal <= (unsigned int)minSimRPM)
				shifting = false;
		}
		else
		{
			DataValues[SPEED].fVal = CalcSpeed();
			if ((DataValues[VEHICLE_RPM].uVal += (RPM_INCREMENT * (maxSimGear - DataValues[GEAR].iVal))) >= (unsigned)(vehicleData.tach.scaleEnd - 150))
			{
				if (++DataValues[GEAR].iVal == maxSimGear)
				{
					if (maxSimGear > 1)
						DataValues[GEAR].iVal = DataValues[GEAR].iVal - 1;
					DataValues[SPEED_PEAKS].fVal = DataValues[SPEED].fVal;
					speedDirection = DECREASING;
				}
				else
				{
					shifting = true;
				}
			}
		}
	}
	else
	{
		if (shifting)
		{
			if ((DataValues[VEHICLE_RPM].uVal += SHIFT_RPM) >= (unsigned)(vehicleData.tach.scaleEnd - 100))
				shifting = false;
		}
		else
		{
			DataValues[SPEED].fVal = CalcSpeed();
			if ((DataValues[VEHICLE_RPM].uVal -= RPM_DECREMENT) > MAX_POSSIBLE_RPM)
				DataValues[VEHICLE_RPM].uVal = 0;
			if (DataValues[VEHICLE_RPM].uVal <= minSimRPM)
			{
				if (--DataValues[GEAR].iVal == 1)
				{
					if (maxSimGear > 1)
						DataValues[GEAR].iVal = 2;
					DataValues[SPEED_PEAKS].fVal = -DataValues[SPEED].fVal;
					speedDirection = INCREASING;
				}
				else
				{
					shifting = true;
				}
			}
		}
	}


	for (i = 0; i < NUMBER_OF_GLOBALS; i++) {

		// request use of global data
//		OS_Use(&SEMA_GlobalData);

		if (-1 != DummyDataValues[i].Dir) {
			switch (i) {
				case X_G:					// f in Gs
				case Y_G:					// f in Gs
				case Z_G:					// f in Gs
				case X_G_DAMPED:			// f in Gs
				case Y_G_DAMPED:			// f in Gs
				case Z_G_DAMPED:			// f in Gs
				case HEADING:				// f, in degrees, 0.0 - 360.0
				case HEADING_DAMPED:		// f, in degrees, 0.0 - 360.0
				case ALTITUDE:				// f, in meters
				case GPS_DOP:				// f gps dilution of precision, range 0.0 - 100.0
				case LAP_DISTANCE:			// f, in meters, since passing s/f
				case TOTAL_DISTANCE:		// f, in meters, since session started
				case PREDICTIVE_LAP:		// f, pace ahead or behind in seconds
				case ANALOG_INPUT_0:		// f, analog input in scaled value
				case ANALOG_INPUT_1:		// f, analog input in scaled value		
				case ANALOG_INPUT_2:		// f, analog input in scaled value
				case ANALOG_INPUT_3:		// f, analog input in scaled value
				case SU_TEMPERATURE:		// f
				case VEHICLE_12V:			// f
				case SIXTY_TRAP:			// f
				case THREE_THIRTY_TRAP:		// f
				case THOUSAND_TRAP:			// f
				case EIGHTH_TRAP:			// f
				case QUARTER_TRAP:			// f
				case VMAX:					// f
				case VMIN:					// f

					if (!dummy_initialized)
						DataValues[i].fVal = DummyDataValues[i].Min;

					if (DummyDataValues[i].Dir == 0)	{		// Increasing
						if ((DataValues[i].fVal += DummyDataValues[i].Inc) > DummyDataValues[i].Max)
							DummyDataValues[i].Dir = 1;			// Start decreasing
					} // if
					else {									// Decreasing
						if ((DataValues[i].fVal -= DummyDataValues[i].Inc) < DummyDataValues[i].Min)
							DummyDataValues[i].Dir = 0;			// Start increasing
					} // else

					break;

				case LATITUDE:			// d, gps lat out to 6 decimal places
				case LONGITUDE:			// d, gps lon out to 6 decimal places

					if (!dummy_initialized)
						DataValues[i].dVal = DummyDataValues[i].Min;

					if (-1 != DummyDataValues[i].Dir) {
						if (DummyDataValues[i].Dir == 0)	{		// Increasing
							if ((DataValues[i].dVal += DummyDataValues[i].Inc) > DummyDataValues[i].Max)
								DummyDataValues[i].Dir = 1;			// Start decreasing
						} // if
						else {									// Decreasing
							if ((DataValues[i].dVal -= DummyDataValues[i].Inc) < DummyDataValues[i].Min)
								DummyDataValues[i].Dir = 0;			// Start increasing
						} // else
					} // if
					break;

				case GPS_WEEKS:			// u, number of weeks since GPS start = Jan 06, 1980
				case GPS_TIME:			// u, number of msec since GPS start + number of weeks
				case GPS_NUMSAT:		// u, number of satellites being monitored, 0 = no GPS Lock
				case DIGITAL_INPUT_4:	// u, digital input, 0 = OFF state, 1 = ON state
				case DIGITAL_INPUT_5:	// u, digital input, 0 = OFF state, 1 = ON state
				case CAMERA_STATUS:		// i, 0 = camera not ready, 1 = camera ready, 2 = camera recording, -1 = no camera
				case BEST_LAP:			// u, 0 = not best lap, 1 = session best lap, 2 = best lap ever
					{
						if (!dummy_initialized)
							DataValues[i].iVal = (int)DummyDataValues[i].Min;

						if (DummyDataValues[i].Dir == 0)	{		// Increasing
							if ((DummyDataValues[i].CurrentValue += DummyDataValues[i].Inc) > (DummyDataValues[i].Max))
								DummyDataValues[i].Dir = 1;			// Start decreasing
						} // if
						else {									// Decreasing
							if ((DummyDataValues[i].CurrentValue -= DummyDataValues[i].Inc) < DummyDataValues[i].Min)
								DummyDataValues[i].Dir = 0;			// Start increasing
						} // else

						DataValues[i].iVal = (int)DummyDataValues[i].CurrentValue;
					} // case

					if (!suData.suPresent) {
						if ((DummyDataValues[GPS_NUMSAT].CurrentValue) >= 4.0)
							suData.suPresent = true;
					}
					else	// su is present, don't let satellites drop below 4
						if (DummyDataValues[GPS_NUMSAT].CurrentValue < 4.0) {
							DummyDataValues[GPS_NUMSAT].CurrentValue = 4.0;
							DummyDataValues[GPS_NUMSAT].Dir = 0;	// go up
						} // if
					break;

				case MAX_RPM:			// u, maximum rpm reading since session started
					// set max values
					if (DataValues[VEHICLE_RPM].uVal > MAX_POSSIBLE_RPM)
						DataValues[VEHICLE_RPM].uVal = 0;
					if (DataValues[MAX_RPM].uVal > MAX_POSSIBLE_RPM)
						DataValues[MAX_RPM].uVal = 0;
					if (sysData.maxRpmEver > MAX_POSSIBLE_RPM)
						sysData.maxRpmEver = 0;

					if (DataValues[VEHICLE_RPM].uVal > DataValues[MAX_RPM].uVal) {
						DataValues[MAX_RPM].uVal = DataValues[VEHICLE_RPM].uVal;
						if (DataValues[VEHICLE_RPM].uVal > sysData.maxRpmEver)
							sysData.maxRpmEver = DataValues[VEHICLE_RPM].uVal;
					} // if
					break;
				} // switch
			} // if

		// relinquish global data
//		OS_Unuse(&SEMA_GlobalData);

	} // for


	dummy_initialized = 1;

	// Check maxs
	if (DataValues[SPEED].fVal > DataValues[MAX_SPEED].fVal)
		DataValues[MAX_SPEED].fVal = DataValues[SPEED].fVal;
	if (DataValues[X_G_DAMPED].fVal > DataValues[MAX_X_G].fVal)
		DataValues[MAX_X_G].fVal = DataValues[X_G_DAMPED].fVal;
	if (DataValues[X_G_DAMPED].fVal < DataValues[MIN_X_G].fVal)
		DataValues[MIN_X_G].fVal = DataValues[X_G_DAMPED].fVal;
	if (DataValues[Y_G_DAMPED].fVal > DataValues[MAX_Y_G].fVal)
		DataValues[MAX_Y_G].fVal = DataValues[Y_G_DAMPED].fVal;
	if (DataValues[Y_G_DAMPED].fVal < DataValues[MIN_Y_G].fVal)
		DataValues[MIN_Y_G].fVal = DataValues[Y_G_DAMPED].fVal;
	if (DataValues[Z_G_DAMPED].fVal > DataValues[MAX_Z_G].fVal)
		DataValues[MAX_Z_G].fVal = DataValues[Z_G_DAMPED].fVal;
	if (DataValues[Z_G_DAMPED].fVal < DataValues[MIN_Z_G].fVal)
		DataValues[MIN_Z_G].fVal = DataValues[Z_G_DAMPED].fVal;

	// Since we know in this test we that we have updated ALL data values we should always
	// update all the LED status too
	SetWarningLED(BOTH_WARNING_LED, (int) GetValue(ANALOG_INPUT_0).fVal);
	SetTachLEDs(GetValue(VEHICLE_RPM).iVal);

	//
	// Handle fake start record speed/end record time variables
	//
	if (!metRecordSpeedYet)
		if (DataValues[SPEED].fVal >= GetRecordSpeed())
		{
			unsavedSysData.waitingForRecordSpeed = FALSE;
			metRecordSpeedYet = TRUE;
		}
} // DummyTick

float CalcSpeed(void)
{
	return (float)DataValues[VEHICLE_RPM].uVal 
		* vehicleData.tireCircumference 
		/ vehicleData.diffRatio 
		/ vehicleData.gearRatios[DataValues[GEAR].iVal - 1]
		* 0.0009469;				// 60 / (12 * 5280);
}

void InitCommSimulatorTask(void)
{
#ifdef _WINDOWS
	GUI_HWIN hDummyWin;
#endif

	SimStartSession();			// Initialize SIM data
	
#ifdef _WINDOWS
// Create perpetual window to run data tick timer
	hDummyWin = WM_CreateWindow(0,0,0,0, WM_CF_HIDE, dataTickCallback,0);
	hDataUpdateTimer = WM_CreateTimer(hDummyWin, 0, DATA_REFRESH_TIME, 0);
#endif
}

#ifdef _WINDOWS
void dataTickCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_TIMER)
	{
			if (pMsg->Data.v == hDataUpdateTimer)
			{
				DummyTick();
				WM_RestartTimer(hDataUpdateTimer, DATA_REFRESH_TIME);
			}
	}
	else WM_DefaultProc(pMsg);
}
#endif

#ifndef _WINDOWS
void CommSimulatorTask(void)
{
  	InitCommSimulatorTask();
	while (1)
	{
		if (DU2_SHUTDOWN == unsavedSysData.systemMode)	// user requested shutdown
			// go away, just go away
			SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) POWER_DOWN, (u08) 0);
		
		OS_Delay(DATA_TIMER_DELAY);				// Reduce to 1 when SU attached
		DummyTick();							// Remove altogether when SU attached
	}
}
#endif

#define GEARFACTOR	0.95						// multiply by this to get in the middle of gear range for reliability

void SetGear( int command ) {
	static float speedRPMTable[NUMBER_OF_GEARS] = { 0.0 };			// meters per engine revolution
	static int topgear = 0;											// max programmed gear
	int i;
	
	if (GEAR_RESET == command) {
		for (i=0; i < NUMBER_OF_GEARS; i++) {
			if (0.0 == vehicleData.gearRatios[i]) {
				topgear = i;
				break;
			}
			else {
				// circumference in m / diff / gear ratio = meters / engine rev
				speedRPMTable[i] = (vehicleData.tireCircumference / 39.0) / vehicleData.diffRatio / vehicleData.gearRatios[i];
			} // if
		} // for
	} // if
	else {		// must be GEAR_CALC
		// check for gears not configured
		if ((vehicleData.diffRatio < 0.01) || (-1 == DataValues[VEHICLE_RPM].iVal)) {
			DataValues[GEAR].iVal = -1;
		} // if
		// check for sitting still or no rpm
		else if ((0.1 > DataValues[SPEED].fVal) || (0 == DataValues[VEHICLE_RPM].iVal)) {
			DataValues[GEAR].iVal = 0;
		} // if
		else {
			float speedfactor;

			// speed in m/s divided by rpm in rev/s = meters / engine rev
			speedfactor = DataValues[SPEED].fVal * 1000.0 / 3600.0 / (((float) DataValues[VEHICLE_RPM].iVal) / 60.0);
			speedfactor = speedfactor * GEARFACTOR;
			
			for (i = 0; i < topgear; i++) {
				if (speedfactor <= speedRPMTable[i]) {
					DataValues[GEAR].iVal = i+1;
					break;
				} // if
			} // for
			// if no match then error
			if (i >= topgear)
				DataValues[GEAR].iVal = -1;
		} // else
	} // else
} // SetGear


void DrawSignalStrength(void)
{
	if (SlideOutVisible())
	{
		// Signal strength
		GUI_SPRITE_SetBitmap(hSignalStrengthSprite, SlideoutGetSignalStrengthIcon());

		// Draw camera icon
		GUI_SPRITE_SetBitmap(hCameraSprite, GetCameraIcon());

		// Draw SU Status icon
		GUI_SPRITE_SetBitmap(hSUStatusSprite, GetSUStatusIcon());
	}
	else
	{
		const GUI_BITMAP *pIcon;
		char scratch[30];

		// Signal strength
		GUI_DrawBitmap(GetSignalStrengthIcon(), SU_SIGNAL_X_POS, 0);

		// SD card
		if (SDBusy)
		{
			GUI_DrawBitmap(&bmRedSD, SD_X_POS, 0);
			SDBusy = false;
		}
		else
		{
			// Update SD card status ICON
#ifdef _WINDOWS
			GUI_DrawBitmap(&bmGreenSD, SD_X_POS, 0);
#else
			if (unsavedSysData.sdCardInserted == 0)										// If no card present
				pIcon = &bmWhiteSD;
			else if (SDCardWriteProtected() == 1)										// If SD card is write protected
				pIcon = &bmLockedSD;
			else if (OS_GetMessageCnt(&MB_SDcardRW) || OS_Q_GetMessageCnt(&Queue_tqm))	// If card is writing
				pIcon = &bmRedSD;
			else if (SDCardAlmostFull())												// If SD card is almost full
				pIcon = &bmYellowSD;
			else pIcon = &bmGreenSD;
			if (SlideOutVisible())
			{
			}
			else GUI_DrawBitmap(pIcon, SD_X_POS, 0);
#endif
		}

		// Draw camera icon if a camera is selected
		if (GetSUCam() != CAMERA_NONE)
		{
			pIcon = GetCameraIcon();
			if (pIcon != 0)
				GUI_DrawBitmap(pIcon, CAMERA_X_POS, 0);
		}

		// Draw the Information icon, if info is available on thie screen
		DrawInfo();
	
		// Put time on display

		GUI_SetFont(&GUI_Font20_ASCII);
		GUI_SetColor(GUI_WHITE);	
		GUI_SetTextMode(GUI_TEXTMODE_TRANS);
			
		if (HaveSatelliteReception())
			GUI_DispStringHCenterAt(GetTimeString(SHORT12, scratch), TIME_RECT_CENTER_X, TIME_RECT_TOP_Y);
		else
			GUI_DispStringHCenterAt("         ", TIME_RECT_CENTER_X, TIME_RECT_TOP_Y);			
	}
}

//
// Return pointer to appropriate camera icon
//
const GUI_BITMAP *GetCameraIcon(void)
{
	switch (GetValue(CAMERA_STATUS).iVal)
	{
		case CAMERA_STATUS_NOT_READY:		return &bmCameraNotReadyIcon;
		case CAMERA_STATUS_READY:			return &bmCameraReadyIcon;
		case CAMERA_STATUS_RECORDING:		return &bmCameraRecordIcon;
		case CAMERA_STATUS_NOT_CONNECTED:	return &bmCameraBlankIcon;
		default:							return 0;
	}
}

const GUI_BITMAP *GetSUStatusIcon(void)
{
	if (suData.suPresent)
		return &bmSUGoodIcon;
	else return &bmNoSUIcon;
}

const GUI_BITMAP *SlideoutGetSignalStrengthIcon(void)
{
	const GUI_BITMAP *pIcon;

	if ((pIcon = GetSignalStrengthIcon()) == &bmNoSUIcon)
		pIcon = &bmNoSignal;
	return pIcon;
}

const GUI_BITMAP *GetSignalStrengthIcon(void)
{
	if (!suData.suPresent)
		return &bmNoSUIcon;
	else
	{
		int numsats;
		int bitmapindex;
		
		numsats = GetValue(GPS_NUMSAT).iVal;

		bitmapindex = 0;
		
		if (numsats >= 3) {
			if (numsats >= 5) {
				if (numsats >= 7) {
					if (numsats >= 9) {
						if (numsats >= 11) {
							bitmapindex = 5;
						} // if
						else
							bitmapindex = 4;
					} // if
					else
						bitmapindex = 3;
				} // if
				else
					bitmapindex = 2;
			} // if
			else
				bitmapindex = 1;
		} // if
		return SignalStrengthGraphics[bitmapindex];
	}
}

//
// Beginning a process where we KNOW the SD card will be busy (like formatting the card)
// let the application force the state to busy and then let the background task come by
// and return the state to not busy when it has the chance to run and sees that the card
// is no longer busy.
//
void ForceSDBusy(WM_HWIN hWin)
{
	WM_InvalidateRect(hWin, &BannerRect);

	SDBusy = true;		// Set flag
	GUI_Exec();			// Give a chance for the paint routine to run before the format or write process begins
}

//
// Inform title bar that this is a new screen
//
void ResetNewScreen(void)
{
	animationIndex = 0;
	animationCycles = MAX_ANIMATION_CYCLES;
	newScreen = true;
}


void DrawInfo(void)
{
	if (newScreen == true)
	{
		// First time displaying this screen - fully populate help variables
		hasInfoKey = CheckForInfoKey(true);
		newScreen = false;
	}

	// First and subsequent paints of this screen - only animate Info Icon of there is a
	// tutorial associated with this screen.  pHelpScreens == -1 used to show if there is a tutorial
	if (pHelpScreens != (HELP_SCREENS *)-1)
	{
		if (animationCycles)
		{
			if (animationIndex >= 10)
			{
				animationIndex = 0;
				animationCycles--;
			}
			switch (animationIndex)
			{
			default:
			case 9:
			case 0:	pAnimation = &bmInfoButton1;	break;
			case 8:
			case 1:	pAnimation = &bmInfoButton2;	break;
			case 7:
			case 2:	pAnimation = &bmInfoButton3;	break;
			case 6:
			case 3:	pAnimation = &bmInfoButton4;	break;
			case 5:
			case 4:	pAnimation = &bmInfoButton5;	break;
			}
			animationIndex++;
		}
	}
	else pAnimation = &bmInfoButton1;

	if (pAnimation)
		GUI_DrawBitmap(pAnimation, 0, 0);
}

//
// Convert temperature in degrees fahrenheit to centigrade
//
 float f_to_c(float val)
 {
	 return (val - 32.0) * 5.0 / 9.0;
 }

//
// Convert temperature in degrees centigrade to fahrenheit
//
 float c_to_f(float val)
 {
	 return ((val * 9.0) / 5.0) + 32.0;
 }

char *GetTimeString(timeforms what, char *pString)
{
#ifdef _WINDOWS
		time_t t;
		struct tm *currentTime;
		char hour;
		char pm;

		t = time((time_t *)NULL);
		currentTime = localtime(&t);

		hour = currentTime->tm_hour;
		pm = (hour >= 12);
		
		if (hour > 12)
			hour = hour % 12;
		if (hour == 0)
			hour = 12;

		sprintf(pString, "%d:%02d%s", hour, currentTime->tm_min, pm? "pm": "am");
#else
		formattime (what, pString);
#endif
		return pString;
}

void ResetTrip(void)
{
	sysData.driveTripDistance = 0.0;
	sysData.driveTripTime = 0;
}

void ResetMax(void)
{
	DataValues[MAX_SPEED].fVal = 0.0;
	DataValues[MAX_X_G].fVal = 0.0;
	DataValues[MAX_Y_G].fVal = 0.0;
	DataValues[MAX_Z_G].fVal = 0.0;
	DataValues[MIN_X_G].fVal = 0.0;
	DataValues[MIN_Y_G].fVal = 0.0;
	DataValues[MIN_Z_G].fVal = 0.0;
}