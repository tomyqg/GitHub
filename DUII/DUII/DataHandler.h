
#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GUI_Type.h"
	
typedef union {
	double dVal;
	float fVal;
	int iVal;
	unsigned int uVal;
} GlobalDataType;

// definitions to identify how good previous lap was
#define NOT_BEST_LAP			0
#define BEST_LAP_OF_SESSION		1
#define BEST_LAP_EVER			2

typedef enum {
	// accelerometer information
	X_G,				// f in Gs
	Y_G,				// f in Gs
	Z_G,				// f in Gs
	MAX_X_G,			// f, max g experienced in x direction since session start
	MAX_Y_G,			// f, max g experienced in y direction since session start
	MAX_Z_G,			// f, max g experienced in z direction since session start
	MIN_X_G,			// f, min g experienced in x direction since session start
	MIN_Y_G,			// f, min g experienced in y direction since session start
	MIN_Z_G,			// f, min g experienced in z direction since session start
	X_G_DAMPED,			// f in Gs, filtered
	Y_G_DAMPED,			// f in Gs, filtered
	Z_G_DAMPED,			// f in Gs, filtered

	// GPS information
	LATITUDE,			// 13 d, gps lat out to 6 decimal places
	LONGITUDE,			// d, gps lon out to 6 decimal places
	SPEED,				// f, stored in kph
	HEADING,			// f, in degrees, 0 - 360
	HEADING_DAMPED,		// f, in degrees, 0 - 360
	GPS_WEEKS,			// u, number of weeks since GPS start = Jan 06, 1980
	GPS_TIME,			// u, number of msec since GPS start + number of weeks
	ALTITUDE,			// f, in meters
	GPS_DOP,			// f gps dilution of precision, range 0.0 - 100.0
	GPS_NUMSAT,			// u, number of satellites being monitored, 0 = no GPS Lock
	MIN_SATS,			// u, minimum number of satellites seen during current session
	MAX_SATS,			// u, maximum number of satellites seen during current session
	MIN_DOP,			// f, minimum dop number seen during current session
	MAX_DOP,			// f, maximum dop number seen during current session
	MAX_SPEED,			// f, max speed in kph achieved since session start
	MIN_ALTITUDE,		// f, in meters
	MAX_ALTITUDE,		// f, in meters

	// distance information
	LAP_DISTANCE,		// 30 f, in meters, since passing s/f or start of session depending on mode
	TOTAL_DISTANCE,		// f, in meters, since session started

	// timing information
	PREDICTIVE_LAP,		// 32 f, pace ahead or behind in seconds
	LAP_TIME,			// u, in msec since passing s/f
	LAST_LAP_TIME,		// u, in msec last completed lap time
	ELAPSED_TIME,		// u, in msec since session started
	ELAPSED_RACE_TIME,	// u, in msec since first passing s/f or launch
	LAP_NUMBER,			// i, lap we are currently on. 0 = s/f not yet found, -1 = not in lapping mode
	VMIN,				// f, last velocity trough
	VMAX,				// f, last velocity peak
	SPEED_PEAKS,		// f, Vmin in corners (negative number), Vmax on straights (positive), in kph
	ZERO_SIXTY,			// u, in msec, 0-60 mph time for drag racing modes
	ZERO_HUNDRED,		// u, in msec, 0-100 mph time for drag racing modes
	SIXTY_FOOT,			// u, in msec, 60' time for drag racing modes
	THREE_THIRTY_FOOT,	// u, in msec, 330' time for drag racing modes
	THOUSAND_FOOT,		// u, in msec, 1000' time for drag racing modes
	EIGHTH_MILE,		// u, in msec, 1000' time for drag racing modes
	QUARTER_MILE,		// u, in msec, 1000' time for drag racing modes
	SIXTY_TRAP,			// f, in kph, 60' trap speed
	THREE_THIRTY_TRAP,	// f, in kph, 330' trap speed
	THOUSAND_TRAP,		// f, in kph, 1000' trap speed
	EIGHTH_TRAP,		// f, in kph, 1/8 mile trap speed
	QUARTER_TRAP,		// f, in kph, 1/4 mile trap speed
	REACTION,			// u, in msec, reaction time

	// input information
	VEHICLE_RPM,		// 55 i, rpm, -1 = undefined
	ANALOG_INPUT_0,		// f, analog input in scaled value
	ANALOG_INPUT_1,		// f, analog input in scaled value		
	ANALOG_INPUT_2,		// f, analog input in scaled value
	ANALOG_INPUT_3,		// f, analog input in scaled value
	DIGITAL_INPUT_4,	// u, digital input, 0 = OFF state, 1 = ON state
	DIGITAL_INPUT_5,	// u, digital input, 0 = OFF state, 1 = ON state
	MAX_A_0,			// f, max value recorded for input during session
	MAX_A_1,			// f, max value recorded for input during session	
	MAX_A_2,			// f, max value recorded for input during session
	MAX_A_3,			// f, max value recorded for input during session
	MIN_A_0,			// f, min value recorded for input during session
	MIN_A_1,			// f, min value recorded for input during session	
	MIN_A_2,			// f, min value recorded for input during session
	MIN_A_3,			// f, min value recorded for input during session
	MAX_RPM,			// u, maximum rpm reading since session started
	TOTAL_HIGHREVS,		// u, total number of high revs ( > red line < max rpm ) since session started
	TOTAL_OVERREVS,		// u, total number of overrevs ( > max rpm ) since session started
	VEHICLE_12V,		// f, vehicle battery voltage
	HORSEPOWER,			// f, in Watts, for all modes
	TORQUE,				// f, in N-m, for all modes

	// calculated car information
	GEAR,				// 76 i, 0 = not moving, -1 = undefined

	// status information
	CAMERA_STATUS,		// i, 0 = camera not ready, 1 = camera ready, 2 = camera recording, -1 = no camera
	AUTOX_DRAG_LAUNCH,	// f, gs encountered during launch of auto-x or drag mode
	SU_TEMPERATURE,		// f, in SU hw version 1, this is core temperature - 50F ~ ambient
	HOLDING,			// u, 1 = in hold time, 0 = not in hold
	BEST_LAP,			// u = 2 = best lap ever, 1 = best lap of session, 0 = not best lap
	COMBINED_G,			// f in Gs, combined X and Y G vectors
	BEST_SESSION_LAP,	// u, in msec best lap of current session
	NUMBER_OF_GLOBALS	// used to get the max value of enum type

} GlobalDataIndexType;

#define CAMERA_STATUS_NOT_CONNECTED		0
#define CAMERA_STATUS_NOT_READY			1
#define CAMERA_STATUS_READY				2
#define CAMERA_STATUS_RECORDING			3

#define GPS_GOOD						4				// must have 4 satellites to be "in coverage"

#define GEAR_RESET						0				// recreate the gear table
#define GEAR_CALC						1				// calculate gear and update global data

// function prototypes
GlobalDataType GetValue(GlobalDataIndexType);
void InitCommSimulatorTask(void);
void DrawSignalStrength(void);
void DrawInfo(void);
void ResetTrip(void);
void ResetMax(void);
GlobalDataType GetRawValue(GlobalDataIndexType);
GlobalDataType GetValue(GlobalDataIndexType);
void ForceSDBusy(GUI_HWIN hWin);
const GUI_BITMAP *GetCameraIcon(void);
const GUI_BITMAP *GetSignalStrengthIcon(void);
const GUI_BITMAP *SlideoutGetSignalStrengthIcon(void);
const GUI_BITMAP *GetSUStatusIcon(void);
signed int CalculateTempCorrection(void);
void ResetNewScreen(void);

// externs
extern GlobalDataType DataValues[NUMBER_OF_GLOBALS];		// global data area
extern const GUI_RECT BannerRect;
extern const GUI_RECT BannerTimeRect;
extern const GUI_RECT InfoTimeRect;


void DummyTick(void);
float f_to_c(float val);
float c_to_f(float val);

#define	GAUGE_SCREEN_REFRESH_TIME	25			// 25 = 40hz screen refresh rate - same as data rate
#define	SETUP_SCREEN_REFRESH_TIME	125

//#define	PSI_TO_BAR				(float)0.068948
#define	PSI_TO_MBAR			(float)68.948

#define	GALLONS_TO_LITERS	(float)3.78541178

#define	POUNDS_TO_KILOS		(float)0.4535924
#define	KILOS_TO_POUNDS		(float)2.2046225

#define	METERS_TO_FEET		(float)3.2808
#define	FEET_TO_METERS		(float)0.3048

#define	INCHES_TO_CM		(float)2.54
#define	CM_TO_INCHES		(float)0.39370079

#define	MILLIMETERS_TO_INCHES	(float)0.03937

#define	KILOMETERS_TO_MILES	(float)0.621371192
#define	MILES_TO_KILOMETERS	(float)1.609344

void SimStartSession(void);

#ifndef _WINDOWS
#define	DATA_TIMER_DELAY	25
void CommSimulatorTask(void);
#endif

typedef enum {
	SHORTDANDT,
	LONGDANDT,
	SHORTTIME,
	SHORT12,
	SHORTDATE
} timeforms;

char *GetTimeString(timeforms, char *);
char *GetAMPMString(char *pString);
void SetGear( int );

#ifdef __cplusplus
}
#endif

#endif

