#ifdef PROFILER
#ifndef _WINDOWS
#include "profiler.h"
#define THIS_TASK	LEDHANDLERTASK_NUM
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commondefs.h"

#ifdef _WINDOWS
#include "SIM.h"
#include "SIMConf.h"
#else
#include "SPI_API.h"
#include "RTOS.H"
#include "adc.h"
#endif

#include "LEDHandler.h"
#include "DataHandler.h"
#include "DUII.h"
#include "ConfigScreens.h"


#define	WARNING_POPUP_WIDTH		250
#define	WARNING_POPUP_HEIGHT	40
#define POPUP_LEFT_POS			0
#define	UPPER_POPUP_POS			22
#define	LOWER_POPUP_POS			62
#define	TEXT_OFFSET				8
#define	BORDER_WIDTH			5
const GUI_RECT UpperRect = { POPUP_LEFT_POS + BORDER_WIDTH, UPPER_POPUP_POS, POPUP_LEFT_POS + WARNING_POPUP_WIDTH - BORDER_WIDTH - 1, UPPER_POPUP_POS + WARNING_POPUP_HEIGHT };
const GUI_RECT LowerRect = { POPUP_LEFT_POS + BORDER_WIDTH, LOWER_POPUP_POS, POPUP_LEFT_POS + WARNING_POPUP_WIDTH - BORDER_WIDTH - 1, LOWER_POPUP_POS + WARNING_POPUP_HEIGHT };

struct LED_DATA {
	int warningOn;
	int LEDMarkTime;
	int flashStartTime;
#ifdef	_WINDOWS
	unsigned int LED_ID;
#else
	U32 DUTY_CYCLE_REGISTER;
#endif
};

struct POPUPTABLE {
	char on;
	char visible;
};


//
// KMC NOTE!
//
// NEW METHOD FOR CONTOLLING LED (including shiftlights, warning LEDs and backlight) INTENSITY!
//
// Since the LED intensity is not linear I will use lookup tables to determine PWM values.
// The user brightness will always be 0 to 99, 0 is minimum acceptable brightness and 99
// is maximum brightness.  User controls will all range from 0 to 99.  User step sizes will be 1.
// The slideout controls during race modes will step somewhere betweeen 1 and 5, this is TBD.
//
// The tables will contain direct PWM values which can be put directly into the control registers.
// There will be PWM lookup tables for: shiftlight LEDs, warning LEDs and backlight (3 tables).
// The shiftlights and warning intensities will both be controlled by the same user index setting
// and so these tables should work in step with eachother.  The backlight uses it's own index.
//
#define	INTENSITY_STEPS			100
#define	BRIGHTNESS_QUICK_STEP	10

//
// Intensity tables (index 0 = dimmest, 99 = brightest)
//
const int BacklightBrightness[INTENSITY_STEPS] = {	
	370, 363, 356, 349, 342, 337, 332, 327, 322, 313,
	295, 285, 275, 265, 255, 245, 235, 225, 215, 205,
	195, 185, 175, 165, 155, 145, 135, 125, 115, 105,
	95, 85, 75, 65, 55, 45, 42, 39, 36, 33,
	30, 27, 24, 21, 18, 15, 12, 9, 6, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int WarningLEDBrightness[INTENSITY_STEPS] = {
	99, 98, 97, 96, 95, 94, 93, 92, 91, 90,
	89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
	79, 78, 77, 76, 75, 74, 73, 72, 71, 70,
	69, 68, 67, 66, 65, 64, 63, 62, 61, 60,
	59, 58, 57, 56, 55, 54, 53, 52, 51, 50,
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40,
	39, 38, 37, 36, 35, 34, 33, 32, 31, 30,
	29, 28, 27, 26, 25, 24, 23, 22, 21, 20,
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10,
	 9,  8,  7,  6,  5,  4,  3,  2,  1,  0
};

const int TachLEDBrightness[INTENSITY_STEPS] = {
	1, 1, 1, 2, 2, 2, 3, 3, 4, 4,
	5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
	10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
	15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
	100, 105, 110, 115, 120, 100, 120, 140, 160, 180,
	200, 225, 250, 300, 350, 300, 400, 400, 400, 400
};

const int AmbientLightIntensity[INTENSITY_STEPS] = {
	  3,   4,   5,   6,   7,   8,   9,  10,
	 11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
	 21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
	 31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
	 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
	 51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
	 61,  62,  63,  64,  65,  66,  67,  68,  69,  70,
	 71,  72,  73,  74,  76,  78,  80,  82,  84,  86,  90,  95,
	100, 110, 120, 130, 140, 150, 160, 180, 200, 225,
	250, 275, 300, 325, 350, 375, 400, 450, 500, 65535
};

#define FLOATABS(a) ((a >= 0.0)? (a) : -(a))

// ms per cycle
#define	LED_UPDATE_CYCLE	1000
#define	ON_CYCLE			125
#define	OFF_CYCLE			125
#define	WARNING_ON_CYCLE	125
#define	WARNING_OFF_CYCLE	125

#define	NONE	0
#define	UP		1
#define	DOWN	2

//  determined empirically
//	traqdash in freezer. turn on and measure values and temperature as it heats up -- gas

#define	TEMP_15C		672
#define	TEMP_20C		620
#define	TEMP_25C		550
#define	TEMP_30C		495
#define	TEMP_32C		470
#define	TEMP_35C		440
#define	TEMP_40C		388
#define	TEMP_45C		360
#define	TEMP_50C		340
#define	TEMP_51C		332
#define	TEMP_52C		324
#define	TEMP_53C		316
#define	TEMP_54C		308
#define	TEMP_55C		304
#define	TEMP_56C		296
#define	TEMP_57C		288
#define	TEMP_58C		280
#define	TEMP_59C		272
#define	TEMP_60C		264

#ifndef _WINDOWS
//
//	Local hardware constants definitions
//

// Simulation definitions
#define	WARNING_LED_1		8
#define	WARNING_LED_2		9

// Bitmasks of actual position in shift register for each specific LED
#define	TOP_LED_CENTER		0x0310
#define	TOP_LED_LEFT_1		0x0C00
#define	TOP_LED_LEFT_2		0x3000
#define	TOP_LED_LEFT_3		0x4000
#define	TOP_LED_RIGHT_1		0x000C
#define	TOP_LED_RIGHT_2		0x8002
#define	TOP_LED_RIGHT_3		0x0001
#endif


//
// Local function prototypes
//
#ifdef _WINDOWS
enum LED_STATE {
	SOLID,
	FLASHING
};
void CenterOn(enum LED_STATE state);
void Level1On(void);
void Level2On(void);
void Level3On(void);
void Level3Off(void);
void Level2Off(void);
void Level1Off(void);
void CenterOff(void);
#else
void LEDShift(U16 mask);
void ShiftLEDLevel(char level);
void ForceWarningLEDsOff(void);
void CheckDisplayConditions(void);
void UpdateLEDs(void);
void RestorePWMSettings(void);
void UpdateWarningPWMVariable(void);
void UpdateBacklightPWMVariable(void);
void UpdateTachPWMVariable(void);
void SetTempCorrection( signed int correction );
#endif
void CreateWarningPopup(char i);
void DeleteWarningPopup(char i, char gaugeScreen);
void UpdateWarningPopup(char i, char gaugeScreen);
void DrawWarningPopups(char i, char *pText, int alarmCount);
void WarningLEDOff(struct LED_DATA *pLED, int override);
void WarningLEDOn(struct LED_DATA *pLED, int override);


//
// Local variable declarations
//
#ifdef _WINDOWS
int level3On = FALSE;
int level2On = FALSE;
int level1On = FALSE;
int centerOn = FALSE;
#else
int LEDlevel = 0;
int warningLEDPWMSetting;
int tachLEDPWMSetting;
int backlightPWMSetting;
signed int tempCorrection;
signed int totalCorrection;
signed int lightCorrection;
int topLEDCenter;
#endif
static int LEDUpdatesSuspended;
static int EnvironmentalUpdatesSuspended = TRUE;	// Default to LED updates off until power-up is complete
struct POPUPTABLE WarningPopups[2];
static BUTTON_Handle hUpperWarning, hLowerWarning;
int centerLEDMarkTime;
int lowerLEDMarkTime;
struct LED_DATA UpperLED, LowerLED;
int lowerLEDOnReason[6];
int upperLEDOnReason[6];

U16 mask = 1;

//
// External data and code references
//
extern GUI_CONST_STORAGE GUI_BITMAP bmWarningPopupBlank;



#ifndef _WINDOWS
void ShiftLEDLevel(char level)
{
	switch (level)
	{
	default:
	case 0:	LEDShift(0);	return;
	case 1:	LEDShift(TOP_LED_LEFT_3 |                                                                                        TOP_LED_RIGHT_3 ); centerLEDMarkTime  = 0; return;
	case 2:	LEDShift(TOP_LED_LEFT_3 | TOP_LED_LEFT_2 |                                                     TOP_LED_RIGHT_2 | TOP_LED_RIGHT_3 ); centerLEDMarkTime  = 0; return;
	case 3:	LEDShift(TOP_LED_LEFT_3 | TOP_LED_LEFT_2 | TOP_LED_LEFT_1 |                  TOP_LED_RIGHT_1 | TOP_LED_RIGHT_2 | TOP_LED_RIGHT_3 ); centerLEDMarkTime  = 0; return;
	case 4:	LEDShift(TOP_LED_LEFT_3 | TOP_LED_LEFT_2 | TOP_LED_LEFT_1 | TOP_LED_CENTER | TOP_LED_RIGHT_1 | TOP_LED_RIGHT_2 | TOP_LED_RIGHT_3 );	centerLEDMarkTime  = 0; return;
	case 5:		// todo = flashing
		{
			int currentLEDTime;

			if (centerLEDMarkTime == 0)
			{	// In normal cases the LED will initially be ON so start the flashing cycle with the OFF state
				centerLEDMarkTime = GUI_GetTime();
				topLEDCenter = 0;						// Turn OFF center LED
			}
			else 
			{	// Note: use one continuous timer rather than restarting because the call to this routine
				// is dependent on the hardware update rate and may not be regular but we want the flash
				// cadence to be even.
				currentLEDTime = GUI_GetTime();
				if ((currentLEDTime - centerLEDMarkTime) > (OFF_CYCLE + ON_CYCLE))
				{
					topLEDCenter = 0;					// Turn OFF center LED
					centerLEDMarkTime += (OFF_CYCLE + ON_CYCLE);
				}
				else if ((currentLEDTime - centerLEDMarkTime) > OFF_CYCLE)
				{
					topLEDCenter = TOP_LED_CENTER;		// Turn ON center LED
				}
			}
			LEDShift(TOP_LED_LEFT_3 | TOP_LED_LEFT_2 | TOP_LED_LEFT_1 | topLEDCenter | TOP_LED_RIGHT_1 | TOP_LED_RIGHT_2 | TOP_LED_RIGHT_3 );	return;
		}
	}
}
#endif


void SetTachLEDs(int rpm)
{
	int step;

	if ((LEDUpdatesSuspended) || (unsavedSysData.systemMode == DU2_STARTUP))
		return;

	if (rpm < vehicleData.tach.upperYellowStart)
	{
#ifdef _WINDOWS
		CenterOff();
		Level1Off();
		Level2Off();
		Level3Off();
#else
		if (LEDlevel != 0)
			ShiftLEDLevel(LEDlevel = 0);
#endif
	}
	else if (rpm < vehicleData.tach.upperYellowStart + (step = (vehicleData.tach.upperRedStart - vehicleData.tach.upperYellowStart) / 3))
	{
#ifdef _WINDOWS
		CenterOff();
		Level1Off();
		Level2Off();
		Level3On();
#else
		if (LEDlevel != 1)
			ShiftLEDLevel(LEDlevel = 1);
#endif
	}
	else if (rpm < vehicleData.tach.upperYellowStart + (step * 2))
	{
#ifdef _WINDOWS
		CenterOff();
		Level1Off();
		Level2On();
		Level3On();
#else
		if (LEDlevel != 2)
			ShiftLEDLevel(LEDlevel = 2);
#endif
	}
	else if (rpm < vehicleData.tach.upperRedStart)
	{
#ifdef _WINDOWS
		CenterOff();
		Level1On();
		Level2On();
		Level3On();
#else
		if (LEDlevel != 3)
			ShiftLEDLevel(LEDlevel = 3);
#endif
	}
	else if (rpm < vehicleData.tach.upperRedStart + step)
	{
		// 1/3 interval beyond redline - start center shift light flashing
#ifdef _WINDOWS
		CenterOn(SOLID);
		Level1On();
		Level2On();
		Level3On();
#else
		if (LEDlevel != 4)
			ShiftLEDLevel(LEDlevel = 4);
#endif
	}
	else
	{
#ifdef _WINDOWS
		CenterOn(FLASHING);
		Level1On();
		Level2On();
		Level3On();
#else
		ShiftLEDLevel(LEDlevel = 5);	// Note: call ShiftLEDLevel EVERY time when flashing center LED
#endif
	}
}


#ifdef _WINDOWS
void CenterOn(enum LED_STATE state)
{
	if (state == SOLID)
	{
		if (!centerOn)
		{
			SIM_HARDKEY_SetState(TOP_LED_CENTER, 1);
			centerLEDMarkTime = 0;
		}
	}
	else		// Center on - FLASHING
	{
		int currentLEDTime;

		if (centerLEDMarkTime == 0)
		{	// In normal cases the LED will initially be ON so start the flashing cycle with the OFF state
			centerLEDMarkTime = GUI_GetTime();
			SIM_HARDKEY_SetState(TOP_LED_CENTER, 0);			// Turn OFF center LED
		}
		else 
		{	// Note: use one continuous timer rather than restarting because the call to this routine
			// is dependent on the hardware update rate and may not be regular but we want the flash
			// cadence to be even.
			currentLEDTime = GUI_GetTime();
			if ((currentLEDTime - centerLEDMarkTime) > (OFF_CYCLE + ON_CYCLE))
			{
				SIM_HARDKEY_SetState(TOP_LED_CENTER, 0);		// Turn off center LED
				centerLEDMarkTime += (OFF_CYCLE + ON_CYCLE);
			}
			else if ((currentLEDTime - centerLEDMarkTime) > OFF_CYCLE)
			{
				SIM_HARDKEY_SetState(TOP_LED_CENTER, 1);		// Turn on center LED
			}
		}
	}

	centerOn = TRUE;
}
void Level1On(void)
{
	if (!level1On)
	{
		SIM_HARDKEY_SetState(TOP_LED_LEFT_1, 1);
		SIM_HARDKEY_SetState(TOP_LED_RIGHT_1, 1);
		level1On = TRUE;
	}
}
void Level2On(void)
{
	if (!level2On)
	{
		SIM_HARDKEY_SetState(TOP_LED_LEFT_2, 1);
		SIM_HARDKEY_SetState(TOP_LED_RIGHT_2, 1);
		level2On = TRUE;
	}
}
void Level3On(void)
{
	if (!level3On)
	{
		SIM_HARDKEY_SetState(TOP_LED_LEFT_3, 1);
		SIM_HARDKEY_SetState(TOP_LED_RIGHT_3, 1);
		level3On = TRUE;
	}
}
void Level3Off(void)
{
	if (level3On)
	{
		SIM_HARDKEY_SetState(TOP_LED_LEFT_3, 0);
		SIM_HARDKEY_SetState(TOP_LED_RIGHT_3, 0);
		level3On = FALSE;
	}
}
void Level2Off(void)
{
	if (level2On)
	{
		SIM_HARDKEY_SetState(TOP_LED_LEFT_2, 0);
		SIM_HARDKEY_SetState(TOP_LED_RIGHT_2, 0);
		level2On = FALSE;
	}
}
void Level1Off(void)
{
	if (level1On)
	{
		SIM_HARDKEY_SetState(TOP_LED_LEFT_1, 0);
		SIM_HARDKEY_SetState(TOP_LED_RIGHT_1, 0);
		level1On = FALSE;
	}
}
void CenterOff(void)
{
	if (centerOn)
	{
		SIM_HARDKEY_SetState(TOP_LED_CENTER, 0);
		centerOn = FALSE;
		centerLEDMarkTime = 0;
	}
}
#else
void LEDShift(U16 mask)
{
  	LED_SPI_X_Write((unsigned char const *)&mask, 2);		// Shift out the data to the LED controller
}
#endif

void WarningLEDOff(struct LED_DATA *pLED, int override)
{
	if (LEDUpdatesSuspended && !override)
		return;

	if (pLED->warningOn)
	{
		pLED->warningOn = FALSE;
		pLED->LEDMarkTime = 0;
		pLED->flashStartTime = 0;

#ifdef	_WINDOWS
		SIM_HARDKEY_SetState(pLED->LED_ID, 0);
#else
		*(volatile U32 *)pLED->DUTY_CYCLE_REGISTER = WARNING_LED_OFF;
#endif
	}
}


void WarningLEDOn(struct LED_DATA *pLED, int override)
{
	int currentLEDTime;

	if (LEDUpdatesSuspended && !override)
		return;

	pLED->warningOn = TRUE;

	if (pLED->LEDMarkTime == 0)
	{	// In normal cases the LED will initially be OFF so start the flashing cycle with the ON state
		pLED->LEDMarkTime = GUI_GetTime();
#ifdef	_WINDOWS
		SIM_HARDKEY_SetState(pLED->LED_ID, 1);		// Turn on LED
#else
		*(volatile U32 *)pLED->DUTY_CYCLE_REGISTER = warningLEDPWMSetting;			// Turn on LED
#endif
	}
	else 
	{
		// Note: use one continuous timer rather than restarting because the call to this routine
		// is dependent on the hardware update rate and may not be regular but we want the flash
		// cadence to be even.
		currentLEDTime = GUI_GetTime();
		if (pLED->flashStartTime == 0)
			pLED->flashStartTime = currentLEDTime;
		else if (currentLEDTime - pLED->flashStartTime > FIVE_SECONDS)
		{
			// Be sure LED ends the cycle in a ON state
#ifdef	_WINDOWS
			SIM_HARDKEY_SetState(pLED->LED_ID, 1);		// Turn on LED
#else
			*(volatile U32 *)pLED->DUTY_CYCLE_REGISTER = warningLEDPWMSetting;			// Turn on LED
#endif
			return;
		}

		// Set to appropriate ON or OFF cycle
		if ((currentLEDTime - pLED->LEDMarkTime) > WARNING_ON_CYCLE + WARNING_OFF_CYCLE)
		{
#ifdef	_WINDOWS
			SIM_HARDKEY_SetState(pLED->LED_ID, 1);		// Turn on LED
#else
			*(volatile U32 *)pLED->DUTY_CYCLE_REGISTER = warningLEDPWMSetting;			// Turn on LED
#endif
			pLED->LEDMarkTime = currentLEDTime;
		}
		else if ((currentLEDTime - pLED->LEDMarkTime) > WARNING_ON_CYCLE)
		{
#ifdef	_WINDOWS
			SIM_HARDKEY_SetState(pLED->LED_ID, 0);		// Turn off LED
#else
			*(volatile U32 *)pLED->DUTY_CYCLE_REGISTER = WARNING_LED_OFF;				// Turn off LED
#endif
		}
	}
}
// Determines if an input is in alarm state
// note that input can only be in alarm state if input is enabled and alarms are enabled for that input
// pass in datapoint 0-3 = analog inputs, 4,5 are digitals
// returns true if in alarm, false if not in alarm
//
int InAlarm(int datapoint) {
	int alarm = false;
	
	if (datapoint >= 0 && datapoint <= 3) {	// analogs
		float tempfloat = GetValue((GlobalDataIndexType) (ANALOG_INPUT_0 + datapoint)).fVal;
	
		if (vehicleData.analogInput[datapoint].enabled) {
			if (BELOW == vehicleData.analogInput[datapoint].lowerAlarmTriggerComparison)
				alarm = (tempfloat < vehicleData.analogInput[datapoint].lowerAlarmTriggerLevel);
			if (ABOVE == vehicleData.analogInput[datapoint].upperAlarmTriggerComparison)
				alarm = alarm || (tempfloat > vehicleData.analogInput[datapoint].upperAlarmTriggerLevel);
		} // if	
	} // if
	else if (datapoint >= 4 && datapoint <= 5) {		// digitals
		signed int tempint = GetValue((GlobalDataIndexType) (DIGITAL_INPUT_4 + datapoint - 4)).uVal;

		if (vehicleData.digitalInput[datapoint-4].enabled) {
			if (tempint == vehicleData.digitalInput[datapoint-4].alarmTriggerState)
				alarm = true;
		} //if
	} // else
	
	return alarm;
} // InAlarm

char AlarmTriggerCause[2][6];		// Upper/Lower[2] - analog input trigger y/n[4] + digital input trigger y/n[2]

// check all the ranges for alarms and ranges
// type can be LOWER_WARNING_LED, UPPER_WARNING_LED, or BOTH_WARNING_LED
//
void SetWarningLED(int type, float value) {
	int iopoint;
	char upperledon = false;
	char lowerledon = false;

	if ((LEDUpdatesSuspended) || (unsavedSysData.systemMode == DU2_STARTUP))
		return;
	
	for (iopoint = 0; iopoint < NUMBER_OF_ANALOGS; iopoint++) {
		if (type != LOWER_WARNING_LED)
			if (vehicleData.analogInput[iopoint].EnableLED1)
				if (InAlarm(iopoint))
				{
					upperledon = true;
					AlarmTriggerCause[0][iopoint] = true;

					// See if this is a new alarm condition
					if (upperLEDOnReason[iopoint] == false)
					{
						// This is a NEW condition for this LED
						UpperLED.flashStartTime = 0;		// Be sure the flashing timer is 0 to restart the flashing
						upperLEDOnReason[iopoint] = true;
					}
				}
				else
				{
					AlarmTriggerCause[0][iopoint] = false;
					upperLEDOnReason[iopoint] = false;
				}

		if (type != UPPER_WARNING_LED)
			if (vehicleData.analogInput[iopoint].EnableLED2)
				if (InAlarm(iopoint))
				{
					lowerledon = true;
					AlarmTriggerCause[1][iopoint] = true;

					// See if this is a new alarm condition
					if (lowerLEDOnReason[iopoint] == false)
					{
						// This is a NEW condition for this LED
						LowerLED.flashStartTime = 0;		// Be sure the flashing timer is 0 to restart the flashing
						lowerLEDOnReason[iopoint] = true;
					}
				}
				else
				{
					AlarmTriggerCause[1][iopoint] = false;
					lowerLEDOnReason[iopoint] = false;
				}
	} // for
	
	for (iopoint = 0; iopoint < NUMBER_OF_DIGITALS; iopoint++) {
	if (type != LOWER_WARNING_LED)
		if (vehicleData.digitalInput[iopoint].EnableLED1)
			if (InAlarm(4 + iopoint))
			{
				upperledon = true;
				AlarmTriggerCause[0][iopoint + 4] = true;

				// See if this is a new alarm condition
				if (upperLEDOnReason[iopoint + 4] == false)
				{
					// This is a NEW condition for this LED
					UpperLED.flashStartTime = 0;		// Be sure the flashing timer is 0 to restart the flashing
					upperLEDOnReason[iopoint + 4] = true;
				}
			}
			else
			{
				AlarmTriggerCause[0][iopoint + 4] = false;
				upperLEDOnReason[iopoint + 4] = false;
			}

	if (type != UPPER_WARNING_LED)
		if (vehicleData.digitalInput[iopoint].EnableLED2)
			if (InAlarm(4 + iopoint))
			{
				lowerledon = true;
				AlarmTriggerCause[1][iopoint + 4] = true;

				// See if this is a new alarm condition
				if (lowerLEDOnReason[iopoint + 4] == false)
				{
					// This is a NEW condition for this LED
					LowerLED.flashStartTime = 0;		// Be sure the flashing timer is 0 to restart the flashing
					lowerLEDOnReason[iopoint + 4] = true;
				}
			}
			else
			{
				AlarmTriggerCause[1][iopoint + 4] = false;
				lowerLEDOnReason[iopoint + 4] = false;
			}
	} // for

	if (upperledon)
	{
		WarningLEDOn(&UpperLED, 0);
		WarningPopups[0].on = true;
	}
	else
	{
		WarningLEDOff(&UpperLED, 0);
		WarningPopups[0].on = false;
	}
	
	if (lowerledon)
	{
		WarningLEDOn(&LowerLED, 0);
		WarningPopups[1].on = true;
	}
	else
	{
		WarningLEDOff(&LowerLED, 0);
		WarningPopups[1].on = false;
	}

	if (!AnyGaugeScreen(GetScreenType()))	// If currently on a gauge screen
		CheckWarningPopups();
}

//
// While on the display settings screen do not make adjustments to the display
//
void SuspendEnvironmentalUpdates(void)
{
	EnvironmentalUpdatesSuspended = true;
#ifndef _WINDOWS
	tempCorrection = lightCorrection = 0;
#endif
}
void ResumeEnvironmentalUpdates(void)
{
	EnvironmentalUpdatesSuspended = false;
}

//
// User just changed state of auto-adjust setting
//
void AutoAdjustOn(char state)
{
#ifndef _WINDOWS
	if (!state)
	{
		// If turning auto-adjust off, restore PWM settings to defaults
		RestorePWMSettings();
	}
#endif
}

//
// Record the ambient light value when these settings were made
//
void RecordAmbientLight(void)
{
	sysData.lightLevelWhenIntensitySet = GetNormalizedLightIntensity();
}

unsigned int GetNormalizedLightIntensity(void)
{
#ifdef _WINDOWS
	return DEFAULT_LIGHT_LEVEL_WHEN_LED_SET;
#else
	unsigned int adcValue;
	unsigned char index;
	
	adcValue = GetADC(ADC_LIGHT_LEVEL);
	index = 1;
	do {
		if (adcValue < AmbientLightIntensity[index])
			break;
	} while (++index < INTENSITY_STEPS);
	return index - 1;
#endif
}

//
// Suspend RPM and warning control of the LEDs and turn ON
//
void SuspendLEDUpdates(char lightsOn)
{
	if (lightsOn)
	{
#ifdef _WINDOWS
		CenterOn(SOLID);
		Level1On();
		Level2On();
		Level3On();
		SIM_HARDKEY_SetState(UpperLED.LED_ID, 1);
		SIM_HARDKEY_SetState(LowerLED.LED_ID, 1);
#else
		ShiftLEDLevel(4);
		*(volatile U32 *)UpperLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
		*(volatile U32 *)LowerLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
#endif
	}
	LEDUpdatesSuspended = true;
}

//
// Resume RPM and warning control of the LEDs and turn ON
//
void ResumeLEDUpdates(void)
{
	RecordAmbientLight();
#ifndef _WINDOWS
	lightCorrection = 0;
#endif

	LEDUpdatesSuspended = false;
#ifdef _WINDOWS
	if (UpperLED.warningOn)
		SIM_HARDKEY_SetState(WARNING_LED_1, 1);
	else SIM_HARDKEY_SetState(WARNING_LED_1, 0);
	if (LowerLED.warningOn)
		SIM_HARDKEY_SetState(WARNING_LED_2, 1);
	else SIM_HARDKEY_SetState(WARNING_LED_2, 0);
#else
	ShiftLEDLevel(LEDlevel);
	
	if (UpperLED.warningOn)
  		*(volatile U32 *)UpperLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
	else *(volatile U32 *)UpperLED.DUTY_CYCLE_REGISTER = WARNING_LED_OFF;
	if (LowerLED.warningOn)
  		*(volatile U32 *)LowerLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
  	else *(volatile U32 *)LowerLED.DUTY_CYCLE_REGISTER = WARNING_LED_OFF;
#endif
}

//
// Turn off all LEDs
//
void TurnOffLEDs(void)
{
#ifdef _WINDOWS
	CenterOff();
	Level1Off();
	Level2Off();
	Level3Off();
#else
	ShiftLEDLevel(LEDlevel = 0);
#endif
	WarningLEDOff(&LowerLED, 1);
	WarningLEDOff(&UpperLED, 1);
}

//
// Completely shut down all power consuming devices - powering off
//
void LEDSystemShutdown(void)
{
	SuspendEnvironmentalUpdates();
	TurnOffLEDs();
#ifndef _WINDOWS
//		GAS -- this does not work. i do not know why
//  	PWMC_DTY0 = 0;		// Turn off backlight (completely)
	SetBacklightBrightness(0);
#endif
}

//
// Calculate where on the temperature verses ADC reading this temperature lays.
// Read the EXPECTED ADC reading for this temperature.
// Calculate the difference between the actual reading and expected reading - this
// is the fixed ofset correction value to be applied to future ADC temperature readings.
//
signed int CalculateTempCorrection(void)
{
	int calTemp = sysData.calibrationTemperature;
	float interpolatedValue;

	// Find cal temp in temp table
	if (calTemp <= 15)
		return 0;			// calibration below table level

	else if (calTemp < 20)
		interpolatedValue = TEMP_15C + (float)(calTemp - 15) / (float) (20 - 15) * (float)(TEMP_20C - TEMP_15C);
	else if (calTemp < 25)
		interpolatedValue = TEMP_20C + (float)(calTemp - 20) / (float) (25 - 20) * (float)(TEMP_25C - TEMP_20C);
	else if (calTemp < 30)
		interpolatedValue = TEMP_25C + (float)(calTemp - 25) / (float) (30 - 25) * (float)(TEMP_30C - TEMP_25C);
	else if (calTemp < 32)
		interpolatedValue = TEMP_30C + (float)(calTemp - 30) / (float) (32 - 30) * (float)(TEMP_32C - TEMP_30C);
	else if (calTemp < 35)
		interpolatedValue = TEMP_32C + (float)(calTemp - 32) / (float) (35 - 32) * (float)(TEMP_35C - TEMP_32C);
	else if (calTemp < 40)
		interpolatedValue = TEMP_35C + (float)(calTemp - 35) / (float) (40 - 35) * (float)(TEMP_40C - TEMP_35C);
	else if (calTemp < 45)
		interpolatedValue = TEMP_40C + (float)(calTemp - 40) / (float) (45 - 40) * (float)(TEMP_45C - TEMP_40C);
	else if (calTemp < 50)
		interpolatedValue = TEMP_45C + (float)(calTemp - 45) / (float) (50 - 45) * (float)(TEMP_50C - TEMP_45C);
	else
		return 0;			// Over temp

	return ((int) interpolatedValue - sysData.calibrationTempReading);
}

#ifndef _WINDOWS

void SetLEDBrightness(int brightness)
{
  	if (brightness < MINIMUM_BRIGHTNESS)
	  	brightness = MINIMUM_BRIGHTNESS;
	
	if (brightness > MAXIMUM_BRIGHTNESS)
		brightness = MAXIMUM_BRIGHTNESS;

  	warningLEDPWMSetting = WarningLEDBrightness[brightness];	// Set Upper MB LED
	*(volatile U32 *)UpperLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
	*(volatile U32 *)LowerLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;

  	tachLEDPWMSetting = TachLEDBrightness[brightness];			// Set tach LEDs
	PWMC_DTY3 = tachLEDPWMSetting;
}

void SetBacklightBrightness(int brightness)
{
  	if (brightness < MINIMUM_BRIGHTNESS)
	  	brightness = MINIMUM_BRIGHTNESS;
	
	if (brightness > MAXIMUM_BRIGHTNESS)
		brightness = MAXIMUM_BRIGHTNESS;

	backlightPWMSetting = BacklightBrightness[brightness];

  	PWMC_DTY0 = backlightPWMSetting;
}


/*********************************************************************
*
*       CheckDisplayConditions()
*
* Function decription:
*   Check ambient light for LED and backlight brightness and board
*	 temperature for LED duty cycle.
*
*	 This routine is called periodically (once per second during development)
*	 to read ambient conditions and adjust LED intensities as necessary.
*
*	 Return: none
*/
void CheckDisplayConditions(void)
{
	int LEDtemperature;

	// Do not make any adjustments while on the display settings screen
	if (EnvironmentalUpdatesSuspended)
	  return;

	// If tach LEDs are OFF and auto-adjust is on - calculate light adjustment factor
	if ((LEDlevel == 0) && sysData.autoAdjustOn && !LEDUpdatesSuspended)
	{	
		signed int lightLevel;

		// Read current ambient light level
		lightLevel = GetNormalizedLightIntensity();		// 0-99, Larger number = brighter light
		if (lightCorrection > lightLevel + 1)
			lightCorrection--;
		else if (lightCorrection < lightLevel - 1)
			lightCorrection++;
	}
	
	// Calculate temperature correction factor
	LEDtemperature = GetADC(ADC_LED_BOARD_TEMP);

	// Apply new temperature correction factor
	LEDtemperature += sysData.LEDtempOffset;

#ifdef IGNORE_TEMPERATURE					// this is here to make Glenn's bench system work right
		LEDtemperature = TEMP_50C + 1;
#endif

	if (LEDtemperature > TEMP_50C)			// If temp under 50C - leave alone - NOTE REVERSE LOGIC OF TEST!!!
		SetTempCorrection( 0 );
	else if (LEDtemperature > TEMP_51C)
		SetTempCorrection( -1 );
	else if (LEDtemperature > TEMP_52C)
		SetTempCorrection( -2 );
	else if (LEDtemperature > TEMP_53C)
		SetTempCorrection( -3 );
	else if (LEDtemperature > TEMP_54C)
		SetTempCorrection( -5 );
	else if (LEDtemperature > TEMP_55C)
		SetTempCorrection( -7 );
	else if (LEDtemperature > TEMP_56C)
		SetTempCorrection( -9 );
	else if (LEDtemperature > TEMP_57C)
		SetTempCorrection( -12 );
	else if (LEDtemperature > TEMP_58C)
		SetTempCorrection( -15 );
	else if (LEDtemperature > TEMP_59C)
		SetTempCorrection( -18 );
	else if (LEDtemperature <= TEMP_60C)		// If temp over 60C set to minimum brightness
		SetTempCorrection( -100 );

	// Calculate total correction - totalCorrection is always the current correction factor for all
	// lights.  If updates are suspended then totalCorrection will still contain the last valid
	// lightCorrection.  This means that when adjusting light levels using the slideouts it will still
	// keep track of the last valid light correction so that when light compensation resumes the
	// light level will remain steady.
	totalCorrection = tempCorrection + lightCorrection;

	// Apply corrections, range check and update PWM variables
	UpdateWarningPWMVariable();
	UpdateBacklightPWMVariable();
	UpdateTachPWMVariable();
	
	// Set new Levels
	UpdateLEDs();
}

//
// Incrementally approach requested temperature correction going not more than
// 10 counts in any one step.
//
void SetTempCorrection( signed int correction )
{
	if (tempCorrection == correction)
		return;
	
	if (tempCorrection < correction)
	{
		// Going "up"
		if ((tempCorrection += 1) > correction)
			tempCorrection = correction;
	}
	else
	{
		// Going "down"
		if ((tempCorrection -= 1) < correction)
			tempCorrection = correction;
	}
}

void UpdateWarningPWMVariable(void)
{
	int index;

	// Convert corrections into indexes
	index = sysData.LEDBrightness + totalCorrection;

	// Check for over limits
	if (index > MAXIMUM_BRIGHTNESS)
		index = MAXIMUM_BRIGHTNESS;
	else if (index < MINIMUM_BRIGHTNESS)
		index = MINIMUM_BRIGHTNESS;
			
	// Apply corrections
	warningLEDPWMSetting = WarningLEDBrightness[index];
}

void UpdateBacklightPWMVariable(void)
{
	int index;
	signed int correction;
	
	// Limit correction so that temperature correction cannot make the backlight too dim
	if (totalCorrection < -90)
		correction = -90;
	else correction = totalCorrection;
	
	// Convert corrections into indexes
//	index = sysData.backlight + totalCorrection;
	index = sysData.backlight + correction;

	// Check for over limits
	if (index > MAXIMUM_BRIGHTNESS)
		index = MAXIMUM_BRIGHTNESS;
	else if (index < MINIMUM_BRIGHTNESS)
		index = MINIMUM_BRIGHTNESS;

	// Apply corrections
	backlightPWMSetting = BacklightBrightness[index];
}

void UpdateTachPWMVariable(void)
{
	int index;

	// Convert corrections into indexes
	index = sysData.LEDBrightness + totalCorrection;

	// Check for over limits
	if (index > MAXIMUM_BRIGHTNESS)
		index = MAXIMUM_BRIGHTNESS;
	else if (index < MINIMUM_BRIGHTNESS)
		index = MINIMUM_BRIGHTNESS;

	// Apply corrections
	tachLEDPWMSetting = TachLEDBrightness[index];
}

//
// Update any LEDs that are currently ON with the new PWM values
// Note that this routine is only required if LEDs are turned on and off by
// setting the PWM to off.  If the PWMs were enabled/disabled as the means
// of turning on/off LEDs then the PWM values could just be written reguardless
// of the LED state.
//
void UpdateLEDs(void)
{
	// Adjust backlight
	PWMC_DTY0 = backlightPWMSetting;
	
	// Adjust tach LEDs
	if (LEDlevel != 0)
		PWMC_DTY3 = tachLEDPWMSetting;

	// Adjust warning LEDs
	if (UpperLED.warningOn)
		*(volatile U32 *)UpperLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
	if (LowerLED.warningOn)
		*(volatile U32 *)LowerLED.DUTY_CYCLE_REGISTER = warningLEDPWMSetting;
}

void RestorePWMSettings(void)
{
	SetBacklightBrightness(sysData.backlight);

  	warningLEDPWMSetting = WarningLEDBrightness[sysData.LEDBrightness];		// Set Upper MB LED
  	tachLEDPWMSetting = TachLEDBrightness[sysData.LEDBrightness];			// Set tach LEDs
}


/*********************************************************************
*
*       LED Handler Task
*
* Function decription:
*   Monitors and maintains LED brightness accounting for ambient light
*	 and LED board temperatures.
*
*/
void LEDHandlerTask(void)
{
	while (1)
	{
		CheckDisplayConditions();		// Check ambient light for LED brightness and board temperature for duty cycle
		OS_Delay(LED_UPDATE_CYCLE);
	}
}

#endif

void InitializeLEDStructures(void)
{
	// Initialize structures for warning LED handling
	UpperLED.warningOn = FALSE;
	UpperLED.flashStartTime = 0;
	LowerLED.flashStartTime = 0;

#ifdef _WINDOWS
	UpperLED.LED_ID = WARNING_LED_1;
	LowerLED.LED_ID = WARNING_LED_2;
#else
	UpperLED.DUTY_CYCLE_REGISTER = 0xFFFC8224;		// PWMC_DTY1
	LowerLED.DUTY_CYCLE_REGISTER = 0xFFFC8244;		// PWMC_DTY2
#endif
}


void InitializeLEDs(void)
{
#ifndef _WINDOWS
	RestorePWMSettings();
	ResumeEnvironmentalUpdates();		// Start LEDs updating
#endif
}


void WarningPopupInit(void)
{
	hUpperWarning = BUTTON_CreateEx(POPUP_LEFT_POS, UPPER_POPUP_POS, WARNING_POPUP_WIDTH, 40, WM_GetDesktopWindow(), WM_CF_HIDE | WM_CF_MEMDEV | WM_CF_STAYONTOP | WM_CF_DISABLED, 0, 0);
	BUTTON_SetBitmap(hUpperWarning, BUTTON_BI_UNPRESSED, &bmWarningPopupBlank);

	hLowerWarning = BUTTON_CreateEx(POPUP_LEFT_POS, LOWER_POPUP_POS, WARNING_POPUP_WIDTH, 40, WM_GetDesktopWindow(), WM_CF_HIDE | WM_CF_MEMDEV | WM_CF_STAYONTOP | WM_CF_DISABLED, 0, 0);
	BUTTON_SetBitmap(hLowerWarning, BUTTON_BI_UNPRESSED, &bmWarningPopupBlank);

	BUTTON_SetTextColor(hLowerWarning, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hUpperWarning, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetPressed(hUpperWarning, 1);
	BUTTON_SetPressed(hLowerWarning, 1);
}


void CheckWarningPopups(void)
{
	char i, gaugeScreen;

	if (AnyGaugeScreen(GetScreenType()))	// If currently on a gauge screen
		gaugeScreen = true;
	else gaugeScreen = false;

	for (i = 0; i < 2; i++)
	{
		if (WarningPopups[i].on)
		{
			if (!WarningPopups[i].visible)
				CreateWarningPopup(i);
			UpdateWarningPopup(i, gaugeScreen);
		}
		else if (WarningPopups[i].visible)
			DeleteWarningPopup(i, gaugeScreen);
	}
}


void CreateWarningPopup(char i)
{
	if (i)
	{
		WM_ShowWindow(hLowerWarning);
		WM_BringToTop(hLowerWarning);
	}
	else
	{
		WM_ShowWindow(hUpperWarning);
		WM_BringToTop(hUpperWarning);
	}

	WarningPopups[i].visible = true;
}


void DeleteWarningPopup(char i, char gaugeScreen)
{
	if (i)
	{
		WM_HideWindow(hLowerWarning);
		BUTTON_SetText(hLowerWarning, "");			// Clear out text so next time it shows it is initially blank until explicitly set
	}
	else
	{
		WM_HideWindow(hUpperWarning);
		BUTTON_SetText(hUpperWarning, "");			// Clear out text so next time it shows it is initially blank until explicitly set
	}
	
	WarningPopups[i].visible = false;

	if (gaugeScreen)
	{
		if (i)
			GUI_ClearRect(POPUP_LEFT_POS, LOWER_POPUP_POS, POPUP_LEFT_POS + WARNING_POPUP_WIDTH, LOWER_POPUP_POS + WARNING_POPUP_HEIGHT);
		else GUI_ClearRect(POPUP_LEFT_POS, UPPER_POPUP_POS, POPUP_LEFT_POS + WARNING_POPUP_WIDTH, UPPER_POPUP_POS + WARNING_POPUP_HEIGHT);
	}
}


void DrawWarningPopups(char i, char *pText, int alarmCount)
{
	int justify;

	GUI_SetColor(GUI_WHITE);

	if (alarmCount == 1)
	{
		GUI_SetFont(&GUI_Font24_ASCII);
		justify = GUI_TA_HCENTER | GUI_TA_VCENTER;
	}
	else
	{
		justify = GUI_TA_LEFT | GUI_TA_VCENTER;
		if (alarmCount == 2)
			GUI_SetFont(&GUI_Font16_ASCII);
		else GUI_SetFont(&GUI_Font10_ASCII);
	}

	GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
	if (i)
	{
		GUI_DrawBitmap(&bmWarningPopupBlank, POPUP_LEFT_POS, LOWER_POPUP_POS);
		GUI_DispStringInRect(pText, (GUI_RECT *)&LowerRect, justify);
	}
	else
	{
		GUI_DrawBitmap(&bmWarningPopupBlank, POPUP_LEFT_POS, UPPER_POPUP_POS);
		GUI_DispStringInRect(pText, (GUI_RECT *)&UpperRect, justify);
	}
}

void UpdateWarningPopup(char i, char gaugeScreen)
{
	int j, alarmCount;
	char scratch[50], outputString[306];

	outputString[0] = 0;
	for (j = alarmCount = 0; j < 6; j++)
	{
		if (AlarmTriggerCause[i][j])
		{
			if (++alarmCount > 1)
				strcat(outputString, ", ");

			if (j < 4)
			{
				float tempfloat = GetValue((GlobalDataIndexType) (ANALOG_INPUT_0 + j)).fVal;

				sprintf(scratch, "%s = %0.1f%s", vehicleData.analogInput[j].Name, tempfloat, vehicleData.analogInput[j].Units);
			}
			else
			{
				// handle digital
				sprintf(scratch, "%s = %s", vehicleData.digitalInput[j-4].Name,
						vehicleData.digitalInput[j-4].alarmTriggerState ? vehicleData.digitalInput[j-4].HighName : vehicleData.digitalInput[j-4].LowName);
			}
			strcat(outputString, scratch);
		}
	}

	if (!gaugeScreen)
	{
		BUTTON_Handle hButton;

		if (i)
			hButton = hLowerWarning;
		else hButton = hUpperWarning;

		BUTTON_SetText(hButton, outputString);
		if (alarmCount == 1)
		{
			BUTTON_SetFont(hButton, &GUI_Font24_ASCII);
			BUTTON_SetTextAlign(hButton, GUI_TA_HCENTER | GUI_TA_VCENTER);
		}
		else
		{
			BUTTON_SetTextAlign(hButton, GUI_TA_LEFT | GUI_TA_VCENTER);
			if (alarmCount == 2)
				BUTTON_SetFont(hButton, &GUI_Font16_ASCII);
			else BUTTON_SetFont(hButton, &GUI_Font10_ASCII);
		}
	}
	else DrawWarningPopups(i, outputString, alarmCount);
}

void DecreaseLEDBrightness(void)
{
	signed int temp;
			
	sysData.autoAdjustOn = false;					// Manually adjusting light levels turn off auto-adjust
	temp = sysData.LEDBrightness;
	if ((temp -= BRIGHTNESS_QUICK_STEP) < MINIMUM_BRIGHTNESS)
		temp = MINIMUM_BRIGHTNESS;
	sysData.LEDBrightness = temp;

#ifndef _WINDOWS
	lightCorrection = 0;
	UpdateTachPWMVariable();
	UpdateWarningPWMVariable();
	SetLEDBrightness(sysData.LEDBrightness);		// light up all LEDs to gauge brightness
#endif
}

void IncreaseLEDBrightness(void)
{
	sysData.autoAdjustOn = false;					// Manually adjusting light levels turn off auto-adjust
	if ((sysData.LEDBrightness += BRIGHTNESS_QUICK_STEP) > MAXIMUM_BRIGHTNESS)
		sysData.LEDBrightness = MAXIMUM_BRIGHTNESS;

#ifndef _WINDOWS
	lightCorrection = 0;
	UpdateTachPWMVariable();
	UpdateWarningPWMVariable();
	SetLEDBrightness(sysData.LEDBrightness);		// light up all LEDs to gauge brightness
#endif
}

#define	NUM_BACKLIGHT_QUICK_STEPS	9
char BackLightQuickSteps[NUM_BACKLIGHT_QUICK_STEPS] = {0, 1, 2, 3, 9, 15, 25, 35, 99};
void DecreaseBacklightBrightness(void)
{
	int i;
			
	sysData.autoAdjustOn = false;					// Manually adjusting light levels turn off auto-adjust
	for (i = NUM_BACKLIGHT_QUICK_STEPS - 1; i >= 0; i--)
		if (sysData.backlight > BackLightQuickSteps[i])
			break;
	sysData.backlight = BackLightQuickSteps[i];
	
#ifndef _WINDOWS
	lightCorrection = 0;
	UpdateBacklightPWMVariable();
	SetBacklightBrightness(sysData.backlight);
#endif
}

void IncreaseBacklightBrightness(void)
{
	int i;
	
	sysData.autoAdjustOn = false;					// Manually adjusting light levels turn off auto-adjust
	for (i = 0; i < NUM_BACKLIGHT_QUICK_STEPS - 1; i++)
		if (sysData.backlight < BackLightQuickSteps[i])
			break;
	sysData.backlight = BackLightQuickSteps[i];
	
#ifndef _WINDOWS
	lightCorrection = 0;
	UpdateBacklightPWMVariable();
	SetBacklightBrightness(sysData.backlight);
#endif
}

