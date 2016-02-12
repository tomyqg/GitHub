/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2009  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : MainTask.C
Purpose     : Main program, called from after main after initialisation
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "DIALOG.h"
#include "ConfigScreens.h"
#include "GeneralConfig.h"
#include "GaugeScreen.h"
#include "FlashReadWrite.h"
#include "GMeterScreen.h"
#include "RunningGauges.h"
#include "SystemSettingsScreens.h"
#include "DataHandler.h"
#include "StandardList.h"
#include "LEDHandler.h"
#ifdef _WINDOWS
#include "..\Application\dirent.h"
#include "SIMConf.h"
#else
#include "FS.h"
#include "adc.h"
#endif
#include "RaceSetupScreens.h"
#include "tmtypes.h"
#include "PersonalizeScreens.h"
#include "SystemSettingsScreens.h"


/*********************************************************************
*
*       Public Code Prototypes
*
**********************************************************************
*/
int GetRecordSpeed(void);
void SetRecordSpeed(int recordSpeed);

/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/
GUI_HWIN	hPersonalizationScreen;
int systemSettingsPopupScreen;

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
// Note that the button bitmaps must be encoded for the appropriate color
// conversion selected in LCDConf.c
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmEraseAll;
extern GUI_CONST_STORAGE GUI_BITMAP bmFactoryDefaults;
extern GUI_CONST_STORAGE GUI_BITMAP bmFormatSD;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmDisplayText;
extern GUI_CONST_STORAGE GUI_BITMAP bmUnitsText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeText;
extern GUI_CONST_STORAGE GUI_BITMAP bmStorageText;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlackButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGrayButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallWhiteButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenCheck;
extern GUI_CONST_STORAGE GUI_BITMAP bmBrightIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmDarkIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmSliderBlue;
extern GUI_CONST_STORAGE GUI_BITMAP bmSliderGreen;
extern GUI_CONST_STORAGE GUI_BITMAP bmOnOffWhite;
extern GUI_CONST_STORAGE GUI_BITMAP bmOnOffGreen;
extern GUI_CONST_STORAGE GUI_BITMAP bmSliderDotTransparent;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoStartSetupText;
extern GUI_CONST_STORAGE GUI_BITMAP bmPredictiveScaleText;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoOnButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoOffButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceScreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmMenuButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmPersonalizeButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmOrangeButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmPersonalizeText;
extern GUI_CONST_STORAGE GUI_BITMAP bmRecordSpeed;
extern GUI_CONST_STORAGE GUI_BITMAP bmMediumSpinner;
extern MULTIEDIT_HANDLE CreateTextBox(WM_HWIN hParent, char *pText);
extern int popupTitleTextColor;

#ifndef _WINDOWS
extern void ConfigureCache(void);
extern void PowerOffUnit(WM_HWIN owner, char reset, int delay);
#endif

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void DisplayDisplaySelection(WM_HWIN);
void DisplaySelectionPaint(WM_HWIN);
void DisplayUnitsSelection(WM_HWIN);
void UnitsSelectionPaint(WM_HWIN);
void DisplayTimeSelection(WM_HWIN);
void TimeSelectionPaint(WM_HWIN);
void DisplayStorageSelection(WM_HWIN);
void StorageSelectionPaint(WM_HWIN);
void DisplayAutoStartSelection(WM_HWIN);
void AutoStartSelectionPaint(WM_HWIN);
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
void setOnOffButtonColors(char onState, char setSliders);
void DisplayLapMeter(WM_HWIN);
void LapMeterPaint(WM_HWIN);
int SystemSettingsPopUp(int screen, WM_HWIN owner);
void nonModalPopupCallback(WM_MESSAGE * pMsg);
void EraseAllSessions(void);
void RestoreFactoryDefaults(WM_HWIN hWin);
char ValidSessionFileName(char *pName);
void DeleteSystemSettingsPopUp(WM_HWIN hDialog);
//void SetAutoOnGraphic(void);
//void SetAutoStartGraphic(void);
const char *GetAutoOnText(void);
//unsigned char *GetAutoStartText(void);
void PersonalizationCB(WM_MESSAGE * pMsg);
enum PERSONALIZE_SCREEN_ID SSSButtonToScreenID(int id);
void SetCurrentTempButtonText(void);
const char *GetDaylightSavingsStr(void);
const char *GetGPSSimulateStr(void);
int WiFiChangedPopUp(void);
void PaintRecordSpeed(WM_HWIN hParent);
const char *GetRecordSpeedString(void);
int GetRecordStopTime(void);
void SetRecordStopTime(int time);
char *GetRecordStopTimeString(void);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST SystemSettingsScreenList[NUM_SYSTEM_SETTINGS_SCREENS+1] = {
	{ DISPLAY_SELECTION, DisplayDisplaySelection, DisplaySelectionPaint },
	{ UNITS_SELECTION, DisplayUnitsSelection, UnitsSelectionPaint },
	{ TIME_SELECTION, DisplayTimeSelection, TimeSelectionPaint },
	{ AUTOSTART_SELECTION, DisplayAutoStartSelection, AutoStartSelectionPaint },
	{ STORAGE_SELECTION, DisplayStorageSelection, StorageSelectionPaint },
	{ LAP_METER, DisplayLapMeter, LapMeterPaint },
	{ RECORD_SPEED, DisplayRecordSpeed, PaintRecordSpeed },
	{ -1, NULL, NULL }
};

#define	BACKLIGHT_Y			85
#define	LED_INTENSITY_Y		160
#define	SLIDER_LEFT			60
#define	SLIDER_X_SIZE		343
// Default slider range == 100, change this value if use SLIDER_SetRange()
#define	SLIDER_RANGE		100
#define	SLIDER_BALL_RADIUS	16

#define	START_SPEED_SPINNER_LEFT	49
#define	STOP_TIME_SPINNER_LEFT		332
#define	SPINNER_WIDTH				105

//
// Widget defines for all System Settings Screens
//
#define	STANDARD_UNITS			GUI_ID_USER + 1
#define	METRIC_UNITS			GUI_ID_USER + 2
#define	AUTO_ADJUST_ON			GUI_ID_USER + 3
#define	AUTO_ADJUST_OFF			GUI_ID_USER + 4
#define	BACKLIGHT_SLIDER		GUI_ID_USER + 5
#define	INTENSITY_SLIDER		GUI_ID_USER + 6
#define	LAPMETER_BUTTON			GUI_ID_USER + 7
#define	AUTO_START_BUTTON		GUI_ID_USER + 8
#define	ERASEALL_BUTTON			GUI_ID_USER + 9
#define	FACTORYDEFAULTS_BUTTON	GUI_ID_USER + 10
#define	FORMATSD_BUTTON			GUI_ID_USER + 11
#define	AUTOON_BUTTON			GUI_ID_USER + 12
#define	DAYLIGHT_SAVINGS_BTN	GUI_ID_USER + 14
#define	TIME_BTN				GUI_ID_USER + 15
#define	PERSONALIZE_BUTTON		GUI_ID_USER + 16
#define	OWNER_BUTTON			GUI_ID_USER + 17
#define	CONTACTINFO_BUTTON		GUI_ID_USER + 18
#define	WIFI_NAME_BUTTON		GUI_ID_USER + 19
#define	WIFI_PASSWORD_BUTTON	GUI_ID_USER + 20
#define	SERIALNUMBER_BUTTON		GUI_ID_USER + 21
#define	BUILDDATE_BUTTON		GUI_ID_USER + 22
#define	TEMP_BUTTON				GUI_ID_USER + 23
#define	READING_BUTTON			GUI_ID_USER + 24
#define	GPS_SIMULATE_BTN		GUI_ID_USER + 25
#define	RECORD_START_SPEED_BTN	GUI_ID_USER + 26


#define	TIME_WHEEL			GUI_ID_LISTWHEEL1
#define	RECORD_SPEED_WHEEL	GUI_ID_LISTWHEEL2
#define	LAP_METER_WHEEL		GUI_ID_LISTWHEEL3
#define	STOP_TIME_WHEEL		GUI_ID_LISTWHEEL4

#ifdef _WINDOWS
#define	MINTEMP	440
#define	MAXTEMP	550
#endif

char * LapMeter[] = {
		"0.5 sec",
		"1 sec",
		"2 sec",
		"5 sec",
		"10 sec",
		NULL
};

char * RecordSpeed[] = {
	"Disabled",
	"20 mph",
	"25 mph",
	"30 mph",
	"35 mph",
	"40 mph",
	"45 mph",
	"50 mph",
	NULL
};

char * RecordSpeedMetric[] = {
	"Disabled",
	"32 kph",
	"40 kph",
	"48 kph",
	"56 kph",
	"64 kph",
	"72 kph",
	"80 kph",
	NULL
};

char * StopTime[] = {
	"Disabled",
	"1 min",
	"2 min",
	"3 min",
	"4 min",
	"5 min",
	"6 min",
	"7 min",
	"8 min",
	"9 min",
	"10 min",
	NULL
};

char *StdTimeZones[] = {
	"-12",
	"-11",
	"-10",
	"-9 Alaska",
	"-8 Pacific",
	"-7 Mountain",
	"-6 Central",
	"-5 Eastern",
	"-4 Atlantic",
	"-3 Greenland",
	"-2",
	"-1",
	"0 London",
	"+1 Paris, Rome",
	"+2 Athens",
	"+3 Moscow",
	"+4",
	"+5",
	"+6",
	"+7",
	"+8 Beijing",
	"+9 Tokyo, Seoul",
	"+10 Melbourne",
	"+11",
	"+12 New Zealand",
	NULL
};

char *DstTimeZones[] = {
	"-11",
	"-10",
	"-9",
	"-8 Alaska",
	"-7 Pacific",
	"-6 Mountain",
	"-5 Central",
	"-4 Eastern",
	"-3 Atlantic",
	"-2 Greenland",
	"-1",
	"0",
	"+1 London",
	"+2 Paris, Rome",
	"+3 Athens",
	"+4 Moscow",
	"+5",
	"+6",
	"+7",
	"+8",
	"+9 Beijing",
	"+10 Tokyo, Seoul",
	"+11 Melbourne",
	"+12",
	"-12 New Zealand",
	NULL
};

//char *LaunchForce[] = {
//	"0.1 G's",
//	"0.2 G's",
//	"0.3 G's",
//	"0.4 G's",
//	"0.5 G's",
//	"0.6 G's",
//	"0.7 G's",
//	"0.8 G's",
//	"0.9 G's",
//	"1.0 G's",
//	NULL
//};

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
WM_HWIN hSystemSettingsScreens;
int KMCDialogSemaphore = 0;
int clickedOnThisScreen;
LISTWHEEL_Handle hTimeWheel, hRecordSpeedWheel, hStopTimeWheel;
SLIDER_Handle hBacklightSlider, hIntensitySlider;
BUTTON_Handle hStdCheck, hMetricCheck;
BUTTON_Handle hOnBtn, hOffBtn;
int firstTimeThrough;
char intensityIgnore = false;
char backlightIgnore = false;
//static LISTWHEEL_Handle hLaunchListWheel;
static LISTWHEEL_Handle hLapMeterListWheel;
int modalWindowUp = false;
BUTTON_Handle hAutoOnButton, hAutoStartButton, hCurrentTempButton, hRecordStartSpeedButton;
MULTIEDIT_HANDLE hPersonalizeText;
#ifdef _WINDOWS
int temptemp = MINTEMP;
int tempIncreasing = 1;
#endif
char gpsSimulate = 0;
char initialName[WIFI_LEN];
char initialPass[WIFI_LEN];


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetSystemSettingsWindowHandle(void)
{
	return hSystemSettingsScreens;
}

void SetSystemSettingsWindowHandle(WM_HWIN hWin)
{
	hSystemSettingsScreens = hWin;
}


void SystemSettingsScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id, temp;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			firstTimeThrough = TRUE;
			SystemSettingsIgnoreNextKeyRelease();
			break;
			
		case WM_PAINT:
			if ((GetScreenType() == SYSTEM_SETTINGS_SCREENS_TYPE) || (GetScreenType() == RECORD_SPEED_SCREEN_TYPE))
			{
				GUI_SetColor(GUI_WHITE);
				GUI_SetFont(&GUI_Font24_ASCII);

				// Call paint method of center screen
				SystemSettingsScreenList[GetLastScreen(SYSTEM_SETTINGS_SCREENS_TYPE)].paint(hSystemSettingsScreens);
				DrawSignalStrength();
			}
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_SEL_CHANGED:
					if (abs(LISTWHEEL_GetVelocity(pMsg->hWinSrc)) <= 1)
					{
						int recordSpeed;

						switch(Id)
						{
						case TIME_WHEEL:
							LISTWHEEL_SetSel(pMsg->hWinSrc, temp = LISTWHEEL_GetPos(pMsg->hWinSrc));
							if (sysData.dstOn)
								sysData.timeZone = temp - 11;
							else sysData.timeZone = temp - 12;
							break;

						case LAP_METER_WHEEL:
							LISTWHEEL_SetSel(pMsg->hWinSrc, sysData.lapMeter = LISTWHEEL_GetPos(pMsg->hWinSrc));
							break;

						case RECORD_SPEED_WHEEL:
							LISTWHEEL_SetSel(pMsg->hWinSrc, temp = LISTWHEEL_GetPos(pMsg->hWinSrc));

							// Speed always stored in kph - Convert from spinner index to speed in kph
							switch (temp)
							{
							default:
							case 0:	recordSpeed = 0;	break;
							case 1:	recordSpeed = 32;	break;
							case 2:	recordSpeed = 40;	break;
							case 3:	recordSpeed = 48;	break;
							case 4:	recordSpeed = 56;	break;
							case 5:	recordSpeed = 64;	break;
							case 6:	recordSpeed = 72;	break;
							case 7:	recordSpeed = 80;	break;
							}
							// Save in sysData structure
							SetRecordSpeed(recordSpeed);
							break;

						case STOP_TIME_WHEEL:
							LISTWHEEL_SetSel(pMsg->hWinSrc, temp = LISTWHEEL_GetPos(pMsg->hWinSrc));
							SetRecordStopTime(temp);
							break;
						}
					}
					break;

				case WM_NOTIFICATION_VALUE_CHANGED:
					if (Id == BACKLIGHT_SLIDER)
					{
						if (backlightIgnore == true)
						{
							backlightIgnore = false;
							break;
						}
						sysData.backlight = SLIDER_GetValue(hBacklightSlider);
#ifndef _WINDOWS
						SetBacklightBrightness(sysData.backlight);
#endif
						setOnOffButtonColors(sysData.autoAdjustOn = false, false);
//						backlightXPos = (int)(((float)backlightXPos / (float)SLIDER_RANGE) * (float)SLIDER_X_SIZE) + SLIDER_LEFT - SLIDER_BALL_RADIUS;
					}
					else if (Id == INTENSITY_SLIDER)
					{
						if (intensityIgnore == true)
						{
							intensityIgnore = false;
							break;
						}

						if (firstTimeThrough)
							firstTimeThrough = false;
						else SuspendLEDUpdates(true);				// Suspend RPM and warning control of the LEDs and turn ON

						sysData.LEDBrightness = SLIDER_GetValue(hIntensitySlider);
#ifndef _WINDOWS
						SetLEDBrightness(sysData.LEDBrightness);		// light up all LEDs to gauge brightness
#endif
						setOnOffButtonColors(sysData.autoAdjustOn = false, false);
//						intensityXPos = (int)(((float)intensityXPos / (float)SLIDER_RANGE) * (float)SLIDER_X_SIZE) + SLIDER_LEFT - SLIDER_BALL_RADIUS;
					}
					break;

				case WM_NOTIFICATION_CLICKED:
					clickedOnThisScreen = true;
					switch (Id)
					{
						//case LAUNCH_G_LIST:
						//case CAMERA_WHEEL:
						case RECORD_SPEED_WHEEL:
						case STOP_TIME_WHEEL:
						case LAP_METER_WHEEL:
						case TIME_WHEEL:
							pressingbutton_semaphore = 1;
							break;

						case BACKLIGHT_SLIDER:				// Do not scroll screen while slider bars being updated
						case INTENSITY_SLIDER:
							SuspendEnvironmentalUpdates();
							PauseScreenScrolling();
							break;

						default:
							WM_DefaultProc(pMsg);
					}
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;
					if (!clickedOnThisScreen)
						break;

					switch (Id)
					{
						case ERASEALL_BUTTON:
							if (SystemSettingsPopUp(ERASE_ALL_SESSIONS, hSystemSettingsScreens))
								EraseAllSessions();
							break;

						case FACTORYDEFAULTS_BUTTON:
							if (SystemSettingsPopUp(FACTORY_DEFAULTS, hSystemSettingsScreens))
							{
								RestoreFactoryDefaults(hSystemSettingsScreens);
								// Done
								SystemSettingsPopUp(RESTORED_DEFAULTS, hSystemSettingsScreens);
							}
							break;

						case FORMATSD_BUTTON:
							if (SystemSettingsPopUp(FORMAT_SDCARD, hSystemSettingsScreens))
								FormatSDCard(hSystemSettingsScreens);
							break;

						case PERSONALIZE_BUTTON:
							DisplayPersonalizationScreen(hSystemSettingsScreens);
							break;

						case AUTOON_BUTTON:
							// if su not available or of wrong version (not version 2) then disable auto-on
							if (!suData.suPresent || (suData.suHwRev < 200)) {
								sysData.autoOn = false;
							} // if
							else {		// su2 is present
								sysData.autoOn = !sysData.autoOn;	// toggle autoOn
								notifyConfigChange();				// tell the SU the new setting
							}
// 							SetAutoOnGraphic();						// show the user
							WM_Invalidate(pMsg->hWin);				// invalidate window so text below buttons can be updated
							break;

						case AUTO_START_BUTTON:
							if (sysData.autoRecord)
								sysData.autoRecord = false;
							else sysData.autoRecord = true;
// 							SetAutoStartGraphic();
							WM_Invalidate(pMsg->hWin);		// Invalidate window so text below buttons can be updated
							break;

						case RECORD_START_SPEED_BTN:
							DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, RECORD_SPEED, pMsg->hWin);
							break;

						case DAYLIGHT_SAVINGS_BTN:
							if (sysData.dstOn)
							{
								sysData.dstOn = false;
								sysData.timeZone -= 1;
							}
							else
							{
								sysData.dstOn = true;
								sysData.timeZone += 1;
							}
							BUTTON_SetText(pMsg->hWinSrc, GetDaylightSavingsStr());
							LISTWHEEL_SetText(hTimeWheel, sysData.dstOn ? DstTimeZones : StdTimeZones);
							WM_Invalidate(hTimeWheel);
							break;

						case GPS_SIMULATE_BTN:
							if (gpsSimulate)
							{
								notifyGPSSimChange(gpsSimulate = false);
							}
							else
							{
								if (SystemSettingsPopUp(GPS_SIMULATE_ON, hSystemSettingsScreens))
									notifyGPSSimChange(gpsSimulate = true);
							}
							BUTTON_SetText(pMsg->hWinSrc, GetGPSSimulateStr());
							break;

						case STANDARD_UNITS:
							sysData.units = STD;
							BUTTON_SetBitmapEx(hStdCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
							BUTTON_SetBitmap(hMetricCheck, BUTTON_CI_UNPRESSED, NULL);
							WM_InvalidateWindow(hSystemSettingsScreens);
							break;

						case METRIC_UNITS:
							sysData.units = METRIC;
							BUTTON_SetBitmap(hStdCheck, BUTTON_CI_UNPRESSED, NULL);
							BUTTON_SetBitmapEx(hMetricCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
							WM_InvalidateWindow(hSystemSettingsScreens);
							break;

						case AUTO_ADJUST_ON:
							setOnOffButtonColors(sysData.autoAdjustOn = true, true);
							AutoAdjustOn(true);
							RecordAmbientLight();				// Record the ambient light value when these settings were made
							ResumeEnvironmentalUpdates();
							WM_InvalidateWindow(hSystemSettingsScreens);
							break;

						case AUTO_ADJUST_OFF:
							setOnOffButtonColors(sysData.autoAdjustOn = false, true);
							AutoAdjustOn(false);
							WM_InvalidateWindow(hSystemSettingsScreens);
							break;

						case BACKLIGHT_SLIDER:
							ResumeScreenScrolling();				// Allow screen scrolling when slider bars released
							RecordAmbientLight();					// Record the ambient light value when these settings were made
							ResumeEnvironmentalUpdates();
							break;

						case INTENSITY_SLIDER:
							ResumeLEDUpdates();						// Resume RPM and warning control of the LEDs
							ResumeScreenScrolling();				// Allow screen scrolling when slider bars released
							ResumeEnvironmentalUpdates();
							break;

						case LAPMETER_BUTTON:
						default:
							if (!KMCDialogSemaphore)
							{
								KMCDialogSemaphore = 1;
								KMCDialogSemaphore = 0;
							}
							break;
					}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void DisplayLapMeter(WM_HWIN hParent)
{
	DisplayStdListScreen(hParent, LapMeter, LAP_METER_WHEEL, &hLapMeterListWheel, sysData.lapMeter, TEXT_ONLY, "Graph Scale");
}

void DisplayDisplaySelection(WM_HWIN hParent)
{
	hBacklightSlider = SLIDER_CreateEx(SLIDER_LEFT - SLIDER_BALL_RADIUS, BACKLIGHT_Y - SLIDER_BALL_RADIUS, SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hParent, WM_CF_SHOW, 0, BACKLIGHT_SLIDER);
	SLIDER_SetNumTicks(hBacklightSlider, 0);

	hIntensitySlider = SLIDER_CreateEx(SLIDER_LEFT - SLIDER_BALL_RADIUS, LED_INTENSITY_Y - SLIDER_BALL_RADIUS, SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hParent, WM_CF_SHOW, 0, INTENSITY_SLIDER);
	SLIDER_SetNumTicks(hIntensitySlider, 0);

	hOnBtn = BUTTON_CreateEx( 245,  205, 87, 34, hParent, WM_CF_SHOW, 0, AUTO_ADJUST_ON);
	BUTTON_SetTextColor(hOnBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetFont(hOnBtn, &GUI_Font24B_ASCII);
	BUTTON_SetText(hOnBtn, "ON");

	hOffBtn = BUTTON_CreateEx( 355,  205, 87, 34, hParent, WM_CF_SHOW, 0, AUTO_ADJUST_OFF);
	BUTTON_SetTextColor(hOffBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetFont(hOffBtn, &GUI_Font24B_ASCII);
	BUTTON_SetText(hOffBtn, "OFF");

	setOnOffButtonColors(sysData.autoAdjustOn, true);
}

void setOnOffButtonColors(char onState, char setSliders)
{
	if (onState)
	{
		BUTTON_SetBitmap(hOnBtn, BUTTON_BI_UNPRESSED, &bmOnOffGreen);
		BUTTON_SetBitmap(hOffBtn, BUTTON_BI_UNPRESSED, &bmOnOffWhite);
		if (setSliders)
		{
			intensityIgnore = true;		// Since this routine just set the slider positions, ignore the next time
			backlightIgnore = true;		// the slider value changed event is handled as that event come from here.
			SLIDER_SetValue(hBacklightSlider, SLIDER_RANGE / 2);
			SLIDER_SetValue(hIntensitySlider, SLIDER_RANGE / 2);
		}
	}
	else
	{
		BUTTON_SetBitmap(hOffBtn, BUTTON_BI_UNPRESSED, &bmOnOffGreen);
		BUTTON_SetBitmap(hOnBtn, BUTTON_BI_UNPRESSED, &bmOnOffWhite);
		if (setSliders)
		{
			intensityIgnore = true;		// Since this routine just set the slider positions, ignore the next time
			backlightIgnore = true;		// the slider value changed event is handled as that event come from here.
			SLIDER_SetValue(hBacklightSlider, sysData.backlight);
			SLIDER_SetValue(hIntensitySlider, sysData.LEDBrightness);
		}
	}
}

void DisplayAutoStartSelection(WM_HWIN hParent)
{
#if 0
	hAutoOnButton = BUTTON_CreateEx(45, 50, 178, 115, hParent, WM_CF_SHOW, 0, AUTOON_BUTTON);
	SetAutoOnGraphic();

	hAutoStartButton = BUTTON_CreateEx(260, 50, 178, 115, hParent, WM_CF_SHOW, 0, AUTO_START_BUTTON);
	SetAutoStartGraphic();
#else
	hAutoOnButton = BUTTON_CreateEx(BUTTON_LEFT, 84, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, AUTOON_BUTTON);
	BUTTON_SetBitmap(hAutoOnButton, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetTextColor(hAutoOnButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hAutoOnButton, BUTTON_BI_PRESSED, GUI_BLACK);
	BUTTON_SetFocussable(hAutoOnButton, 0);
	BUTTON_SetText(hAutoOnButton, "Auto Power On");

	hAutoStartButton = BUTTON_CreateEx(BUTTON_LEFT, 158, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, AUTO_START_BUTTON);
	BUTTON_SetBitmap(hAutoStartButton, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetTextColor(hAutoStartButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hAutoStartButton, BUTTON_BI_PRESSED, GUI_BLACK);
	BUTTON_SetFocussable(hAutoStartButton, 0);
	BUTTON_SetText(hAutoStartButton, "Auto Start");

	hRecordStartSpeedButton = BUTTON_CreateEx(BUTTON_LEFT, 213, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RECORD_START_SPEED_BTN);
	BUTTON_SetBitmap(hRecordStartSpeedButton, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetTextColor(hRecordStartSpeedButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hRecordStartSpeedButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hRecordStartSpeedButton, 0);
	BUTTON_SetText(hRecordStartSpeedButton, "Record Settings");
#endif
}

const char *GetAutoOnText(void)
{
	if (!suData.suPresent || (suData.suHwRev < 200))
		return "Not Supported by SU";

	if (sysData.autoOn)
		return "Auto On with Power";
	else return "Turns on with Button";
}

#if 0
void SetAutoOnGraphic(void)
{
	if (sysData.autoOn)
		BUTTON_SetBitmap(hAutoOnButton, BUTTON_CI_UNPRESSED, &bmAutoOnButton);
	else
		BUTTON_SetBitmap(hAutoOnButton, BUTTON_CI_UNPRESSED, &bmAutoOffButton);
}

void SetAutoStartGraphic(void)
{
	if (sysData.autoRecord)
		BUTTON_SetBitmap(hAutoStartButton, BUTTON_CI_UNPRESSED, &bmRaceScreenButton);
	else BUTTON_SetBitmap(hAutoStartButton, BUTTON_CI_UNPRESSED, &bmMenuButton);
}

unsigned char *GetAutoStartText(void)
{
	if (sysData.autoRecord)
		return "Auto Record";
	else return "Do Not Auto Record";
}
#endif

char *GetLapMeterText(void)
{
	return LapMeter[sysData.lapMeter];
}


void DisplayUnitsSelection(WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	hBtn = BUTTON_CreateEx(155,  43, 300, 100, hParent, WM_CF_SHOW, 0, STANDARD_UNITS);
	BUTTON_SetBkColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetFont(hBtn, &GUI_Font32B_ASCII);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_RED);
	BUTTON_SetTextAlign(hBtn, GUI_TA_LEFT | GUI_TA_VCENTER);
	BUTTON_SetText(hBtn, "  US Standard");
	BUTTON_SetSkinClassic(hBtn);

	hBtn = BUTTON_CreateEx(155,  149, 300, 100, hParent, WM_CF_SHOW, 0, METRIC_UNITS);
	BUTTON_SetBkColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetFont(hBtn, &GUI_Font32B_ASCII);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_RED);
	BUTTON_SetTextAlign(hBtn, GUI_TA_LEFT | GUI_TA_VCENTER);
	BUTTON_SetText(hBtn, "  Metric");
	BUTTON_SetSkinClassic(hBtn);

	hStdCheck = BUTTON_CreateEx(30,  43, 115, 100, hParent, WM_CF_SHOW, 0, STANDARD_UNITS);
	BUTTON_SetBkColor(hStdCheck, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hStdCheck);

	hMetricCheck = BUTTON_CreateEx(30,  149, 115, 100, hParent, WM_CF_SHOW, 0, METRIC_UNITS);
	BUTTON_SetBkColor(hMetricCheck, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hMetricCheck);

	if (sysData.units == STD)
		BUTTON_SetBitmapEx(hStdCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
	else BUTTON_SetBitmapEx(hMetricCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
}

const char *GetGPSSimulateStr(void)
{
	return (gpsSimulate ? "Simulating GPS" : "Normal GPS");
}

const char *GetDaylightSavingsStr(void)
{
	return (sysData.dstOn ? "Daylight Savings On" : "Daylight Savings Off");
}

void DisplayTimeSelection(WM_HWIN hParent)
{
	int selection;
	BUTTON_Handle hBtn;

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	// Create spinner wheel and populate
	hTimeWheel = LISTWHEEL_CreateEx(SPINNER_START + 16, 50, 205, 205, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, TIME_WHEEL,
		sysData.dstOn ? DstTimeZones : StdTimeZones);
	LISTWHEEL_SetLineHeight(hTimeWheel, 41);
	LISTWHEEL_SetSnapPosition(hTimeWheel, 87);
	LISTWHEEL_SetFont(hTimeWheel, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(hTimeWheel, GUI_TA_LEFT);

	// Determine current selection to highlight
	if (sysData.dstOn)
		selection = sysData.timeZone + 11;
	else selection = sysData.timeZone + 12;

	LISTWHEEL_SetPos(hTimeWheel, selection);
	LISTWHEEL_SetSel(hTimeWheel, selection);

	// Create buttons
	hBtn = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, 108, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, DAYLIGHT_SAVINGS_BTN);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hBtn, GetDaylightSavingsStr());
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hBtn, 0);

	hBtn = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, 193, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, GPS_SIMULATE_BTN);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmYellowButton);
	BUTTON_SetText(hBtn, GetGPSSimulateStr());
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hBtn, 0);
}

void DisplayStorageSelection(WM_HWIN hParent)
{
BUTTON_Handle hButton;

	hButton = BUTTON_CreateEx(41, 38, 178, 115, hParent, WM_CF_SHOW, 0, ERASEALL_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmEraseAll);

	hButton = BUTTON_CreateEx(261, 38, 178, 115, hParent, WM_CF_SHOW, 0, FACTORYDEFAULTS_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmFactoryDefaults);

	hButton = BUTTON_CreateEx(41, 156, 178, 115, hParent, WM_CF_SHOW, 0, FORMATSD_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmFormatSD);

	hButton = BUTTON_CreateEx(261, 156, 178, 115, hParent, WM_CF_SHOW, 0, PERSONALIZE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmPersonalizeButton);

	////////// If newer hardware (version 2 or newer) allow auto-on feature
	////////if (suData.suPresent && (suData.suHwRev >= 200))
	////////{
	////////	hButton = BUTTON_CreateEx(261, 156, 178, 115, hParent, WM_CF_SHOW, 0, AUTOON_BUTTON);
	////////	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmAutoOn);
	////////}
}


void LapMeterPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmPredictiveScaleText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(LAP_METER), 0, 229);
}

void AutoStartSelectionPaint(WM_HWIN hParent)
{
	char scratch[30];

#define	TOP_BUT			48
#define	GAP				43
#define	CM_LINE_GAP		20
#define	LEFT_CENTER		45 + (178 / 2)
#define	RIGHT_CENTER	BUTTON_LEFT + 101
#define	FIVE_BTN_BOX_LEFT			27
#define	FIVE_BTN_BOX_RIGHT			245
#define	CAMERA_TEXT_CENTER			136
#define	HALF_CAMERA_LINE			89
#define	FIVE_BTN_BOX_TOP			TOP_BUT
#define	FIVE_BTN_BOX_BOTTOM			(TOP_BUT + 209)

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmAutoStartSetupText, 60, 0);

	// Draw left side square for data
	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);

	// draw 2 gray lines for field seperators
	GUI_SetPenSize(2);
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DrawLine(FIVE_BTN_BOX_LEFT + CM_LINE_GAP, 107, FIVE_BTN_BOX_RIGHT - CM_LINE_GAP, 107);
	GUI_DrawLine(FIVE_BTN_BOX_LEFT + CM_LINE_GAP, 169, FIVE_BTN_BOX_RIGHT - CM_LINE_GAP, 169);

	// Draw left static text
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_SetColor(GUI_LIGHTBLUE);
	GUI_DispStringHCenterAt("Current Settings",		LEFT_CENTER, 52);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("Power On Control",	LEFT_CENTER, 80);
	GUI_SetColor(GUI_YELLOW);
	GUI_DispStringHCenterAt(GetAutoOnText(),		LEFT_CENTER, 110);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("Auto Start Location",	LEFT_CENTER, 142);
	GUI_SetColor(GUI_RED);
	GUI_DispStringHCenterAt( sysData.autoRecord ? "Race Screens" : "Menu Screens",
													LEFT_CENTER, 172);
	GUI_SetColor(GUI_GREEN);
	sprintf(scratch, "Start Speed %s", GetRecordSpeedString());
	GUI_DispStringHCenterAt(scratch,	LEFT_CENTER, 200);
	sprintf(scratch, "Stop Time %s", GetRecordStopTimeString());
	GUI_DispStringHCenterAt(scratch,	LEFT_CENTER, 225);

	// Draw right static text
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DispStringHCenterAt("Startup Settings",			RIGHT_CENTER, 52);
	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_DispStringHCenterAt("Disables Power Button",	RIGHT_CENTER, 126);

	//GUI_SetFont(&GUI_Font16B_ASCII);

	//if (sysData.autoOn)
	//{
	//	GUI_DispStringHCenterAt("Unit Turns On With Power", 45 + (178 / 2), 210);
	//	GUI_DispStringHCenterAt("Power Button Disabled", 45 + (178 / 2), 230);
	//}
	//else GUI_DispStringHCenterAt("Unit Turns On With Button", 45 + (178 / 2), 210);

	//if (!suData.suPresent || (suData.suHwRev < 200))
	//	GUI_DispStringHCenterAt("Not Supported by Sensor Unit", 45 + (178 / 2), 230);

	//if (sysData.autoRecord)
	//{
	//	GUI_DispStringHCenterAt("Returns to Recording", 260 + (178 / 2), 210);
	//	GUI_DispStringHCenterAt("Mode at Startup", 260 + (178 / 2), 230);
	//}
	//else
	//{
	//	GUI_DispStringHCenterAt("Returns to Menu", 260 + (178 / 2), 210);
	//	GUI_DispStringHCenterAt("at Startup", 260 + (178 / 2), 230);
	//}

	GUI_DrawBitmap(GetNavigationArrow(AUTOSTART_SELECTION), 0, 229);
}


void PaintRecordSpeed(WM_HWIN hParent)
{
#define	TEXT_START	100
#define	TEXT_STEP	30

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmRecordSpeed, 68, 0);
	GUI_DrawBitmap(&bmMediumSpinner, START_SPEED_SPINNER_LEFT, 51);
	GUI_DrawBitmap(&bmMediumSpinner, STOP_TIME_SPINNER_LEFT, 51);

	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("Select Record",	XSIZE_PHYS/2,	TEXT_START);
	GUI_DispStringHCenterAt("Start Speed",		XSIZE_PHYS/2,	TEXT_START + (TEXT_STEP * 1));
	GUI_DispStringHCenterAt("and",				XSIZE_PHYS/2,	TEXT_START + (TEXT_STEP * 2));
	GUI_DispStringHCenterAt("Stop Time",		XSIZE_PHYS/2,	TEXT_START + (TEXT_STEP * 3));

	GUI_DrawBitmap(GetNavigationArrow(LAP_METER), 0, 229);
	DrawSignalStrength();
}

//
// GetRecordStopTime
//	Return record stop time setting in minutes
//	Default is 0 - disabled
//
int GetRecordStopTime(void)
{
	int temp;

	temp = ((sysData.multiSettings_2 >> 4) & 0x00000FFF) / 60;		// read value and convert from seconds to minutes

	// Validate value
	if (temp > 10)
		temp = 0;
	return temp;
}

//
// SetRecordStopTime
//	Input: record time in minutes
//	Convert to seconds - units used by SU and save value
//	Default is 0 - disabled
//
void SetRecordStopTime(int time)
{
	if (time > 10)
		time = 0;
	else time *= 60;

	sysData.multiSettings_2 = (sysData.multiSettings_2 & 0xFFFF000F) | (time << 4);
}

char *GetRecordStopTimeString(void)
{
	int time;
	static char timeStr[10];

	if ((time = GetRecordStopTime()) == 0)
		return "Disabled";
	else 
	{
		sprintf(timeStr, "%d min", time);
		return timeStr;
	}
}

//
// Returns the current record speed in kph
//
int GetRecordSpeed(void)
{
	return (sysData.multiSettings_2 & 0xFFFF0000) >> 16;
}

//
// Set the record speed to the given value in kph
//
void SetRecordSpeed(int recordSpeed)
{
	sysData.multiSettings_2 = (sysData.multiSettings_2 & 0x0000FFFF) | (recordSpeed << 16);
}

void DisplayRecordSpeed(WM_HWIN hParent)
{
	int recordSpeed, recordSpeedIndex, stopTimeIndex;

	SetScreenType(RECORD_SPEED_SCREEN_TYPE);

	recordSpeed = GetRecordSpeed();

	// Speed always stored in kph - Convert from speed to spinner index
	switch (recordSpeed)
	{
	default:
	case 0:		recordSpeedIndex = 0;	break;
	case 32:	recordSpeedIndex = 1;	break;
	case 40:	recordSpeedIndex = 2;	break;
	case 48:	recordSpeedIndex = 3;	break;
	case 56:	recordSpeedIndex = 4;	break;
	case 64:	recordSpeedIndex = 5;	break;
	case 72:	recordSpeedIndex = 6;	break;
	case 80:	recordSpeedIndex = 7;	break;
	}

//	DisplayStdListScreen(hParent, sysData.units == STD ? RecordSpeed : RecordSpeedMetric, 
//	RECORD_SPEED_WHEEL, &hRecordSpeedWheel, recordSpeedIndex, TEXT_ONLY, "Record Settings");

	// Display Start Speed (left) wheel
	hRecordSpeedWheel = LISTWHEEL_CreateEx(START_SPEED_SPINNER_LEFT, 50, SPINNER_WIDTH, 208, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, 
		RECORD_SPEED_WHEEL, sysData.units == STD ? RecordSpeed : RecordSpeedMetric);
	LISTWHEEL_SetLineHeight(hRecordSpeedWheel, 41);
	LISTWHEEL_SetSnapPosition(hRecordSpeedWheel, 87);
	LISTWHEEL_SetFont(hRecordSpeedWheel, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(hRecordSpeedWheel, GUI_TA_CENTER);
	LISTWHEEL_SetPos(hRecordSpeedWheel, recordSpeedIndex);
	LISTWHEEL_SetSel(hRecordSpeedWheel, recordSpeedIndex);

	// Display Stop Time (right) wheel
	hStopTimeWheel = LISTWHEEL_CreateEx(STOP_TIME_SPINNER_LEFT, 50, SPINNER_WIDTH, 208, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, STOP_TIME_WHEEL, StopTime);
	LISTWHEEL_SetLineHeight(hStopTimeWheel, 41);
	LISTWHEEL_SetSnapPosition(hStopTimeWheel, 87);
	LISTWHEEL_SetFont(hStopTimeWheel, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(hStopTimeWheel, GUI_TA_CENTER);
	LISTWHEEL_SetPos(hStopTimeWheel, stopTimeIndex = GetRecordStopTime());
	LISTWHEEL_SetSel(hStopTimeWheel, stopTimeIndex);
}

const char *GetRecordSpeedString(void)
{
	int recordSpeed;

	recordSpeed = GetRecordSpeed();

	// Speed always stored in kph - Convert from speed to spinner index
	if (sysData.units == METRIC)
	{
		switch (recordSpeed)
		{
		default:
		case 0:		return "Disabled";
		case 32:	return "32 kph";
		case 40:	return "40 kph";
		case 48:	return "48 kph";
		case 56:	return "56 kph";
		case 64:	return "64 kph";
		case 72:	return "72 kph";
		case 80:	return "80 kph";
		}
	}
	else
	{
		switch (recordSpeed)
		{
		default:
		case 0:		return "Disabled";
		case 32:	return "20 mph";
		case 40:	return "25 mph";
		case 48:	return "30 mph";
		case 56:	return "35 mph";
		case 64:	return "40 mph";
		case 72:	return "45 mph";
		case 80:	return "50 mph";
		}
	}
}


void DisplaySelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmDisplayText, 60, 0);
	GUI_DrawRoundedRect(10, 40, 460, 250, 5);
	GUI_FillRoundedRect(10, 40, 460, 250, 5);

	GUI_SetTextMode(GUI_TM_REV);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringHCenterAt("Backlight", 230, BACKLIGHT_Y - 40);
	GUI_DispStringHCenterAt("LED Intensity", 230, LED_INTENSITY_Y - 40);
	GUI_DispStringAt("Auto-Adjust", 100, 210);

	GUI_DrawBitmap(&bmDarkIcon, 18, BACKLIGHT_Y - 17);
	GUI_DrawBitmap(&bmBrightIcon, 412, BACKLIGHT_Y - 21);
	GUI_DrawBitmap(&bmSliderBlue, SLIDER_LEFT, BACKLIGHT_Y - 5);
	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(10, BACKLIGHT_Y + 30, 460, BACKLIGHT_Y + 30);

	GUI_DrawBitmap(&bmDarkIcon, 18, LED_INTENSITY_Y - 17);
	GUI_DrawBitmap(&bmBrightIcon, 412, LED_INTENSITY_Y - 21);
	GUI_DrawBitmap(&bmSliderGreen, SLIDER_LEFT, LED_INTENSITY_Y - 5);
	GUI_DrawLine(10, LED_INTENSITY_Y + 30, 460, LED_INTENSITY_Y + 30);

	GUI_DrawBitmap(GetNavigationArrow(DISPLAY_SELECTION), 0, 229);
}

void UnitsSelectionPaint(WM_HWIN hParent)
{
	GUI_SetColor(GUI_WHITE);
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmUnitsText, 60, 0);

	GUI_DrawRoundedRect(10, 40, 460, 250, 5);
	GUI_FillRoundedRect(10, 40, 460, 250, 5);

	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(150, 40, 150, 250);
	GUI_DrawLine(10, 145, 460, 145);

	GUI_DrawBitmap(GetNavigationArrow(UNITS_SELECTION), 0, 229);
}

void TimeSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmTimeText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(TIME_SELECTION), 0, 229);

	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_DispStringHCenterAt("Select GMT Offset", 130, 78);
	GUI_DispStringHCenterAt("Indoor Testing", 130, 163);
}

//void InputsSelectionPaint(WM_HWIN hParent)
//{
//	GUI_RECT Rect;
//
//	GUI_DrawBitmap(&bmInputsBackground, 0, 0);
//	GUI_DrawBitmap(&bmInputText, 60, 0);
//
//	Rect.x0 = 3;
//	Rect.y0 = 0;
//	Rect.x1 = 23;
//	Rect.y1 = YSIZE_PHYS;
//	GUI_SetFont(&GUI_Font20B_ASCII);
//	GUI_SetColor(GUI_RED);
//	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
//	GUI_DispStringInRectEx("ANALOG",
//		&Rect,
//		GUI_TA_HCENTER,
//		6,
//		GUI_ROTATE_CCW);
//
//	Rect.x0 = XSIZE_PHYS - 23;
//	Rect.y0 = 0;
//	Rect.x1 = XSIZE_PHYS - 3;
//	Rect.y1 = YSIZE_PHYS;
//	GUI_SetColor(GUI_GREEN);
//	GUI_DispStringInRectEx("DIGITAL",
//		&Rect,
//		GUI_TA_HCENTER,
//		20,
//		GUI_ROTATE_CCW);
//
//
//	GUI_SetColor(GUI_BLACK);					// Draw shadow text due to light background
//	GUI_SetFont(&GUI_Font24B_ASCII);
//	//GUI_DispStringHCenterAt("Analog0", 98, 127);
//	//GUI_DispStringHCenterAt("Analog1", 242, 127);
//	//GUI_DispStringHCenterAt("Digital4", 386, 127);
//	GUI_DispStringHCenterAt("Analog0", 98, 127);
//	GUI_DispStringHCenterAt("Analog1", 242, 127);
//	GUI_DispStringHCenterAt("Digital4", 386, 127);
//
//	GUI_SetColor(GUI_WHITE);
////	GUI_SetFont(&GUI_Font24B_ASCII);
////	GUI_SetTextMode(GUI_TM_TRANS);
//	GUI_DispStringHCenterAt("Analog0", 96, 125);
//	GUI_DispStringHCenterAt("Analog1", 240, 125);
//	GUI_DispStringHCenterAt("Digital4", 384, 125);
//	GUI_DispStringHCenterAt("Analog2", 96, 235);
//	GUI_DispStringHCenterAt("Analog3", 240, 235);
//	GUI_DispStringHCenterAt("Digital5", 384, 235);
//
//	GUI_DrawBitmap(GetNavigationArrow(INPUTS_SELECTION), 0, 229);
//}
//
void StorageSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmStorageText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(STORAGE_SELECTION), 0, 229);
}


static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	return &bmleftarrow;
}

void SystemSettingsIgnoreNextKeyRelease(void)
{
	clickedOnThisScreen = false;
}

//int GetScreenID(int Id)
//{
//	switch (Id)
//	{
//		case ANALOG0_BUTTON:		return ANALOG0;
//		case ANALOG1_BUTTON:		return ANALOG1;
//		case ANALOG2_BUTTON:		return ANALOG2;
//		case ANALOG3_BUTTON:		return ANALOG3;
//		case DIGITAL4_BUTTON:	return DIGITAL4;
//		default:
//		case DIGITAL5_BUTTON:	return DIGITAL5;
//	}
//}

static const GUI_WIDGET_CREATE_INFO EraseAllResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to Erase All Sessions!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO GPSSimulateResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to enable GPS simulation mode", 0, 0, 15, 353, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "used for camera or dyno testing.", 0, 0, 45, 353, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 75, 353, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO EraseSessionResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to delete session", 0, 0, 15, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 75, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO FactoryDefaultsResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to Restore All Settings!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO FactoryDefaultsRestoredResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Restored to factory settings.", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

static const GUI_WIDGET_CREATE_INFO FormatSDResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to Format SD card!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};
static const GUI_WIDGET_CREATE_INFO InvalidSDResources[] = {
	{ FRAMEWIN_CreateIndirect, "ERROR", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Cannot read SD card!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Format Required!", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "FORMAT", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};
//static const GUI_WIDGET_CREATE_INFO AutoOnEnableResources[] = {
//	{ FRAMEWIN_CreateIndirect, "ENABLE AUTO-ON?", 0, 50, 20, 360, 220, 0, 0 },
//	{ TEXT_CreateIndirect, "Would you like to enable Auto-On?", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
//	{ TEXT_CreateIndirect, "Makes unit turn on automatically", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
//	{ BUTTON_CreateIndirect, "YES", GUI_ID_YES, 60, 115, 100, 60 },
//	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
//};
//static const GUI_WIDGET_CREATE_INFO AutoOnDisableResources[] = {
//	{ FRAMEWIN_CreateIndirect, "DISABLE AUTO-ON?", 0, 50, 20, 360, 220, 0, 0 },
//	{ TEXT_CreateIndirect, "Would you like to disable Auto-On?", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
//	{ TEXT_CreateIndirect, "Power on/off only with the power key", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
//	{ BUTTON_CreateIndirect, "YES", GUI_ID_YES, 60, 115, 100, 60 },
//	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
//};

static const GUI_WIDGET_CREATE_INFO FormattingSDResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Working!", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

static const GUI_WIDGET_CREATE_INFO AllSessionsErasedResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "All sessions erased", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

static const GUI_WIDGET_CREATE_INFO RealFormattingSDResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Formatting!", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
};

static const GUI_WIDGET_CREATE_INFO ErasingSessionsResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Erasing session(s)!", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
};

static const GUI_WIDGET_CREATE_INFO LoadingSessionsResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Loading session data.", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Please wait.", 0, 0, 85, 360, 30, TEXT_CF_HCENTER },
};

static const GUI_WIDGET_CREATE_INFO FormattingCompleteResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Formatting complete!", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

static const GUI_WIDGET_CREATE_INFO FormattingFailedResources[] = {
	{ FRAMEWIN_CreateIndirect, "ERROR", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Formatting FAILED!", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

//
// Display a modal dialog box to see if the user wants to stop recording to exit this screen
//
// Returns	YES - proceed to exit record mode
//				NO - abort and stay
//
int SystemSettingsPopUp(int screen, WM_HWIN owner)
{
	int retval, x, y;

	retval = false;
	modalWindowUp = true;

	GUI_GetOrg(&x, &y);			// Get y offset so popups work reguardless of scroll position

	switch (systemSettingsPopupScreen = screen)
	{
		case DELETE_SESSION:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(EraseSessionResources, GUI_COUNTOF(EraseSessionResources), &popupCallback, owner, 10, 10 + y) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case ERASE_ALL_SESSIONS:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(EraseAllResources, GUI_COUNTOF(EraseAllResources), &popupCallback, owner, 10, 10 + y) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case GPS_SIMULATE_ON:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(GPSSimulateResources, GUI_COUNTOF(GPSSimulateResources), &popupCallback, owner, 10, 10 + y) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case FACTORY_DEFAULTS:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(FactoryDefaultsResources, GUI_COUNTOF(FactoryDefaultsResources), &popupCallback, owner, 10, 10 + y) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case FORMAT_SDCARD:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(FormatSDResources, GUI_COUNTOF(FormatSDResources), &popupCallback, owner, 10, 10 + y) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case INVALID_SDCARD:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(InvalidSDResources, GUI_COUNTOF(InvalidSDResources), &popupCallback, owner, 10, 10 + y) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		////////case ENABLE_AUTO_ON:
		////////	retval = GUI_ExecDialogBox(AutoOnEnableResources, GUI_COUNTOF(AutoOnEnableResources), &popupCallback, owner, 10, 10 + y);
		////////	break;

		////////case DISABLE_AUTO_ON:
		////////	retval = GUI_ExecDialogBox(AutoOnDisableResources, GUI_COUNTOF(AutoOnDisableResources), &popupCallback, owner, 10, 10 + y);
		////////	break;

		case FORMATTING_COMPLETE:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			retval = GUI_ExecDialogBox(FormattingCompleteResources, GUI_COUNTOF(FormattingCompleteResources), &nonModalPopupCallback, owner, 10, 10 + y);
			break;
			
		case FORMATTING_FAILED:
			SetPopupDefaults(ERROR_POPUP);
			retval = GUI_ExecDialogBox(FormattingFailedResources, GUI_COUNTOF(FormattingFailedResources), &nonModalPopupCallback, owner, 10, 10 + y);
			break;
			
		// Display true non-modal dialog box
			// These boxes expect messages from other tasks for completion.  So return here and DO NOT clear modalWindowUp
			// That is handled when the message comes indicating the job is done - or, in the caese of my testing, when you press OK
			//
		case ALL_SESSIONS_ERASED:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			return GUI_CreateDialogBox(AllSessionsErasedResources, GUI_COUNTOF(AllSessionsErasedResources), &nonModalPopupCallback, owner, 10, 10 + y);
			
		case RESTORED_DEFAULTS:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			return GUI_CreateDialogBox(FactoryDefaultsRestoredResources, GUI_COUNTOF(FactoryDefaultsRestoredResources), &nonModalPopupCallback, owner, 10, 10 + y);

		case FORMATTING_SDCARD:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			return GUI_CreateDialogBox(FormattingSDResources, GUI_COUNTOF(FormattingSDResources), &nonModalPopupCallback, owner, 10, 10 + y);

		case REAL_FORMATTING_SDCARD:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			return GUI_CreateDialogBox(RealFormattingSDResources, GUI_COUNTOF(RealFormattingSDResources), &nonModalPopupCallback, owner, 10, 10 + y);

		case ERASING_SESSIONS:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			return GUI_CreateDialogBox(ErasingSessionsResources, GUI_COUNTOF(ErasingSessionsResources), &nonModalPopupCallback, owner, 10, 10 + y);

		case LOADING_SESSIONS:
			SetPopupDefaults(INFORMATIONAL_POPUP);
			return GUI_CreateDialogBox(LoadingSessionsResources, GUI_COUNTOF(LoadingSessionsResources), &nonModalPopupCallback, owner, 10, 10 + y);
	}
	
	modalWindowUp = false;

	return retval;
}

void DeleteSystemSettingsPopUp(WM_HWIN hDialog)
{
	WM_DeleteWindow(hDialog);
	modalWindowUp = false;
}

void nonModalPopupCallback(WM_MESSAGE * pMsg)
{
   WM_HWIN hDlg;
	int NCode, Id;

   hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_INIT_DIALOG:
			FRAMEWIN_SetTextAlign(hDlg, GUI_TA_HCENTER);
			FRAMEWIN_SetTextColor(hDlg, popupTitleTextColor);
			PauseScreenScrolling();
			break;

		case WM_DELETE:
			ResumeScreenScrolling();
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_RELEASED:
					switch (Id)
					{
						case GUI_ID_OK:
							modalWindowUp = false;
							ResumeScreenScrolling();
							GUI_EndDialog(hDlg, Id);
							break;
					}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void FormatSDCard(WM_HWIN hWin)
{
	WM_HWIN hDialog;

	ForceSDBusy(hWin);					// Set the SD card busy indication - will be cleared automatically when all writing is complete

	hDialog = SystemSettingsPopUp(REAL_FORMATTING_SDCARD, hWin);
	GUI_Exec();

#ifndef _WINDOWS
	FS_CACHE_Clean("");
	FS_Sync("");
	FS_AssignCache("", NULL, 0, 0);		// Disable cache
	
	if (FS_FormatSD("") != 0)		// Format using default parameters
	{
		WM_DeleteWindow(hDialog);
		SystemSettingsPopUp(FORMATTING_FAILED, hWin);
		return;
	}
	ConfigureCache();				// Turn cache back on
#endif
	
	RestoreFactoryDefaults(hWin);
	
	WM_DeleteWindow(hDialog);
	SystemSettingsPopUp(FORMATTING_COMPLETE, hWin);
}

void RestoreFactoryDefaults(WM_HWIN hWin)
{
	ForceSDBusy(hWin);					// Set the SD card busy indication - will be cleared automatically when all writing is complete

	// Be sure directory structure is there (in case someone just stuck in a
	// new card and we haven't built the directory structure yet).
	InitializeDirectoryStructure();

	// Delete all old files (if any)
	DeleteAllFiles(DRIVER_DATA);
	DeleteAllFiles(VEHICLE_DATA);
	DeleteAllFiles(TRACK_DATA);
	DeleteAllFiles(SESSION_DATA);

	InitializeData();					// Reset all selections back to factory selections
	ReloadData(VEHICLE_DATA);			// Reset default values for the default driver/vehicle/track
	ReloadData(TRACK_DATA);
	ReloadData(DRIVER_DATA);

	WriteUserDataEvent(SYSTEM_DATA);	// Use SYNCHRONOUS calls to save data so we know they are all comlpeted before calling
	WriteUserDataEvent(VEHICLE_DATA);	// FS_Sync to be sure they all made it to the disk (in case the user fulls the card without
	WriteUserDataEvent(DRIVER_DATA);	// doing a controlled power down).

	// Create default drivers, vehicles and tracks (beyond the standard Road Atlanta, Driver A, Spec Miata)
	CreateAllFiles(DRIVER_DATA);

	// Initialize all secondary (beyond the Miata) vehicles and write their files
	CreateAllFiles(VEHICLE_DATA);

	GUI_Exec();							// Let SD card data get written before attempting to read for file lists
	
	// Reset all lists and tables
	ResetAllFileLists(true);
	ClearNameStruct();					// Erase session notes line (used on Race Review screen)

#ifndef _WINDOWS
	FS_Sync("");						// Be sure all new settings are written to SD card
#endif
}


void EraseAllSessions(void)
{
	WM_HWIN hDialog;
	char fileName[MAX_FILENAME_SIZE];
#ifndef _WINDOWS
	char fullFileName[MAX_FILENAME_SIZE];
	FS_FIND_DATA fd;
#else
	DIR *dir;
	struct dirent *ent;
#endif

	ForceSDBusy(hSystemSettingsScreens);	// Set the SD card busy indication - will be cleared automatically when all writing is complete

	hDialog = SystemSettingsPopUp(ERASING_SESSIONS, hSystemSettingsScreens);
	GUI_Exec();

#ifndef _WINDOWS
	if (FS_FindFirstFile(&fd, MY_SESSIONS_PATH, fileName, MAX_FILENAME_SIZE) == 0)
	{
		do {
			if (ValidSessionFileName(fileName))		// KMC ?  Shouldn't this be fd->something
			{
				// Prepend directoy name
				strcpy(fullFileName, MY_SESSIONS_PATH);
				strcat(fullFileName, "\\");
				strcat(fullFileName, fileName);
				FS_Remove(fullFileName);
			}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#else
	dir = opendir(MY_SESSIONS_PATH);
	if (dir != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidSessionFileName(ent->d_name))
			{
				strcpy(fileName, MY_SESSIONS_PATH);
				strcat(fileName, "\\");
				strcat(fileName, ent->d_name);
				remove(fileName);
			}
		}
		closedir(dir);
	}
#endif

	ClearNameStruct();
	CreateFileList(SESSION_DATA);	// Update the sessions file list and BestLap list(to nothing)

	WM_DeleteWindow(hDialog);
	SystemSettingsPopUp(ALL_SESSIONS_ERASED, hSystemSettingsScreens);
}

void ClearNameStruct(void)
{
	NameStruct.date[0] = 0;
	NameStruct.driver[0] = 0;
	NameStruct.time[0] = 0;
	NameStruct.track[0] = 0;
	NameStruct.vehicle[0] = 0;
	NameStruct.sessionNumber = 0;
	NameStruct.sessionType = NOT_VALID;
}

//
// Make sure this is a valid file name ending in ".tqm"
//
char ValidSessionFileName(char *pName)
{
	char ch;

	// get to file extension
	while (((ch = *pName++) != '.') && (ch != '\0'))
		;

	if ((ch == '.') && (*pName == 't') && (*(pName + 1) == 'q') && (*(pName + 2) == 'm'))
		return true;
	else return false;
}


void PersonalizationCB(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);

				SetCurrentTempButtonText();
			}
			break;

		case WM_CREATE:
			break;

		case WM_PAINT:
			GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
			GUI_DrawBitmap(&bmPersonalizeText, 60, 0);
			GUI_DrawBitmap(&bmleftarrow, 0, 229);
			DrawSignalStrength();
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_RELEASED:
					switch (Id)
					{
						case WIFI_NAME_BUTTON:
						case WIFI_PASSWORD_BUTTON:
							if (!(unsavedSysData.wifiEnabled && unsavedSysData.wifiCardInserted))
								break;
							// else fall through

						case OWNER_BUTTON:
						case CONTACTINFO_BUTTON:
						case SERIALNUMBER_BUTTON:
						case BUILDDATE_BUTTON:
						case TEMP_BUTTON:
							DisplayNewKeyboardScreen(SSSButtonToScreenID(Id), hPersonalizationScreen);
							break;
						case READING_BUTTON:
#ifdef _WINDOWS
							// Function not supported on the PC - do not alter value
							// But allow the update to progress to save the temperature selection
							sysData.calibrationTempReading = temptemp;
#else
							sysData.calibrationTempReading = GetADC(ADC_LED_BOARD_TEMP);
							sysData.LEDtempOffset = CalculateTempCorrection();
#endif
							WriteUserDataEvent(SYSTEM_DATA);		// Record new settings and save to disk immediately (in case factory personnel does not exit the screen)
							break;
					}
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}


void DisplayPersonalizationScreen(WM_HWIN hPrev)
{
	// Save initial values for comparison testing at screen exit
	strcpy(initialName, unsavedSysData.unitInfo.wifiName);
	strcpy(initialPass, unsavedSysData.unitInfo.wifiPass);
	ReturnToPersonalizationScreen(hPrev);
}

void ReturnToPersonalizationScreen(WM_HWIN hPrev)
{
	BUTTON_Handle hBtn;
	char scratch[50];

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenType(PERSONALIZE_REVIEW_SCREEN_TYPE);
	SetScreenScrollingHOnly();
	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	hPersonalizationScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, PersonalizationCB, 0);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 48, BUTTON_WIDTH, BUTTON_HEIGHT, hPersonalizationScreen, WM_CF_SHOW, 0, OWNER_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hBtn, "TraqDash Owner");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
	BUTTON_SetFocussable(hBtn, 0);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 91, BUTTON_WIDTH, BUTTON_HEIGHT, hPersonalizationScreen, WM_CF_SHOW, 0, CONTACTINFO_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmOrangeButton);
	BUTTON_SetText(hBtn, "Contact Info");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
	BUTTON_SetFocussable(hBtn, 0);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 134, BUTTON_WIDTH, BUTTON_HEIGHT, hPersonalizationScreen, WM_CF_SHOW, 0, WIFI_NAME_BUTTON);
	BUTTON_SetText(hBtn, "WiFi Name");
	if (unsavedSysData.wifiEnabled && unsavedSysData.wifiCardInserted)
	{
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
		BUTTON_SetFocussable(hBtn, 0);
	}
	else
	{
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_PRESSED, &bmGrayButton);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_DISABLED, &bmGrayButton);
	}

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 177, BUTTON_WIDTH, BUTTON_HEIGHT, hPersonalizationScreen, WM_CF_SHOW, 0, WIFI_PASSWORD_BUTTON);
	BUTTON_SetText(hBtn, "WiFi Password");
	if (unsavedSysData.wifiEnabled && unsavedSysData.wifiCardInserted)
	{
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
		BUTTON_SetFocussable(hBtn, 0);
	}
	else
	{
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_PRESSED, &bmGrayButton);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_DISABLED, &bmGrayButton);
	}

	hCurrentTempButton = 0;
	if (unsavedSysData.enableTempCalibration)
	{
		hBtn = BUTTON_CreateEx(BUTTON_LEFT, 220, 96, BUTTON_HEIGHT - 2, hPersonalizationScreen, WM_CF_SHOW, 0, TEMP_BUTTON);
		BUTTON_SetText(hBtn, "Temp");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hBtn, 0);

		hCurrentTempButton = BUTTON_CreateEx(BUTTON_LEFT + 104, 220, 96, BUTTON_HEIGHT - 2, hPersonalizationScreen, WM_CF_SHOW, 0, READING_BUTTON);
		BUTTON_SetTextColor(hCurrentTempButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hCurrentTempButton, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hCurrentTempButton, 0);
		SetCurrentTempButtonText();
	}
	else if (unsavedSysData.enableSerialNumber)
	{
		hBtn = BUTTON_CreateEx(BUTTON_LEFT, 220, 96, BUTTON_HEIGHT - 2, hPersonalizationScreen, WM_CF_SHOW, 0, SERIALNUMBER_BUTTON);
		BUTTON_SetText(hBtn, "S/N");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hBtn, 0);

		hBtn = BUTTON_CreateEx(BUTTON_LEFT + 104, 220, 96, BUTTON_HEIGHT - 2, hPersonalizationScreen, WM_CF_SHOW, 0, BUILDDATE_BUTTON);
		BUTTON_SetText(hBtn, "Date");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hBtn, 0);
	}

	hPersonalizeText = CreateTextBox(hPersonalizationScreen, "");
	MULTIEDIT_AddText(hPersonalizeText, unsavedSysData.unitInfo.owner);
	MULTIEDIT_AddText(hPersonalizeText, "\r\n\r\n");
	MULTIEDIT_AddText(hPersonalizeText, unsavedSysData.unitInfo.phone);
	MULTIEDIT_AddText(hPersonalizeText, "\r\n\r\n");
	MULTIEDIT_AddText(hPersonalizeText, unsavedSysData.unitInfo.wifiName);
	MULTIEDIT_AddText(hPersonalizeText, "\r\n");
	MULTIEDIT_AddText(hPersonalizeText, unsavedSysData.unitInfo.wifiPass);
	MULTIEDIT_AddText(hPersonalizeText, "\r\n\r\n");
	sprintf(scratch, "Serial # %u", unsavedSysData.unitInfo.serno);
	MULTIEDIT_AddText(hPersonalizeText, scratch);
	MULTIEDIT_AddText(hPersonalizeText, "\r\n");
	sprintf(scratch, "Built week %02u, 20%02u\n\r", unsavedSysData.unitInfo.week, unsavedSysData.unitInfo.year);
	MULTIEDIT_AddText(hPersonalizeText, scratch);

	// Display temperature calibration factor
	{
		int temp;

		temp = sysData.calibrationTemperature;
		if (sysData.units == STD)
			temp = (int)c_to_f((float)temp);
		sprintf(scratch, "CAL %d%s = %d", temp, sysData.units == METRIC ? "C" : "F", sysData.calibrationTempReading);
	}
	MULTIEDIT_AddText(hPersonalizeText, scratch);
	SCROLLBAR_SetWidth(WM_GetScrollbarV(hPersonalizeText), FINGER_WIDTH);

	hScreenTimer = WM_CreateTimer(hPersonalizationScreen, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

//
// If WiFi data changed ask if user wants to restart now or later
//
void PersonalizeScreenExit(void)
{
	int results1, results2;

	results1 = strcmp(initialName, unsavedSysData.unitInfo.wifiName);
	results2 = strcmp(initialPass, unsavedSysData.unitInfo.wifiPass);
	if ((results1 != 0) || (results2 != 0))
	{
		// WiFi settings changed - see if user wants to reset
		if (WiFiChangedPopUp() == GUI_ID_OK)
		{
			WriteUserDataEvent(VEHICLE_DATA);
#ifdef _WINDOWS
			SIM_Powerkey_CB(POWER_KEY, 0);
#else
			PowerOffUnit(hPersonalizationScreen, false, 1000);				// Pause for 1 second to allow time for the SU to process the new camera setting
#endif
		}
	}
}


//
// Returns	GUI_ID_OK to go back and edit password again or
//			GUI_ID_CANCEL - to accept the default password
//
int WiFiChangedPopUp(void)
{
static const GUI_WIDGET_CREATE_INFO WiFiChanged[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "You have changed WiFi settings", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "which requires a power cycle.", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to reset now?",      0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK,   35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "NO", GUI_ID_CANCEL, 195, 115, 120, 60 }
};

	SetPopupDefaults(WARNING_POPUP);
	return GUI_ExecDialogBox(WiFiChanged, GUI_COUNTOF(WiFiChanged), &popupCallback, hPersonalizationScreen, 10, 10);
}

void SetCurrentTempButtonText(void)
{
	char scratch[50];

	if (hCurrentTempButton)
	{
#ifdef _WINDOWS
		if (tempIncreasing)
		{
			if (temptemp++ > MAXTEMP)
				tempIncreasing = 0;
		}
		else
		{
			if (temptemp-- < MINTEMP)
				tempIncreasing = 1;
		}
		sprintf(scratch, "%d", temptemp);
#else
		sprintf(scratch, "%d", GetADC(ADC_LED_BOARD_TEMP));
#endif
	//	sprintf(scratch, "%d", sysData.calibrationTempReading);
		BUTTON_SetText(hCurrentTempButton, scratch);
	}
}


enum PERSONALIZE_SCREEN_ID SSSButtonToScreenID(int id)
{
	switch (id)
	{
	default:
	case OWNER_BUTTON:			return OWNER_SCREEN;
	case CONTACTINFO_BUTTON:	return CONTACTINFO_SCREEN;
	case WIFI_NAME_BUTTON:		return WIFI_NAME_SCREEN;
	case WIFI_PASSWORD_BUTTON:	return WIFI_PASSWORD_SCREEN;
	case SERIALNUMBER_BUTTON:	return SERIALNUMBER_SCREEN;
	case BUILDDATE_BUTTON:		return BUILDDATE_SCREEN;
	case TEMP_BUTTON:			return TEMP_SCREEN;
	}
}


/*************************** End of file ****************************/

