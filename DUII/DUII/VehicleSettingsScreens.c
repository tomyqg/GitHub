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
#include "VehicleSettingsScreens.h"
#include "DataHandler.h"
#include "StandardList.h"
#include "TachSetupScreens.h"
#include "InputScreens.h"
#include "SystemSettingsScreens.h"
#include "GearingScreens.h"
#include "Utilities.h"
#include "LEDHandler.h"
#ifdef _WINDOWS
#include "SIMConf.h"
#endif

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
BUTTON_Handle hCurrentRPMBtn;
BUTTON_Handle hBtnMobius, hBtnPrimeX, hBtnMini, hBtnNone;
GUI_HWIN hAeroGripScreen, hCameraSelection, hCameraDelay, hTraqdashCam;
int ones, tenths;
int testCamera = false;
WM_HTIMER hCameraDelayTimer;


/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
// Note that the button bitmaps must be encoded for the appropriate color
// conversion selected in LCDConf.c
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmNarrowSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmOrangeButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmMediumBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmMediumBlueButtonDark;
extern GUI_CONST_STORAGE GUI_BITMAP bmGrayButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTachSetup;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenCheck;
extern GUI_CONST_STORAGE GUI_BITMAP bmCameraText;
extern GUI_CONST_STORAGE GUI_BITMAP bmInputsBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmInputText;
extern GUI_CONST_STORAGE GUI_BITMAP bmBrownInputButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenInputButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedInputButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallBlackButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmInputs;
extern GUI_CONST_STORAGE GUI_BITMAP bmCamera;
extern GUI_CONST_STORAGE GUI_BITMAP bmCameraSetupBanner;
extern GUI_CONST_STORAGE GUI_BITMAP bmTach;
extern GUI_CONST_STORAGE GUI_BITMAP bmCarMetricsButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmCarMetricsBanner;
extern GUI_CONST_STORAGE GUI_BITMAP bmAeroandTire;
extern GUI_CONST_STORAGE GUI_BITMAP bmWeight;
extern GUI_CONST_STORAGE GUI_BITMAP bmVehicleWeight;
extern GUI_CONST_STORAGE GUI_BITMAP bmGearRatio;
extern GUI_CONST_STORAGE GUI_BITMAP bmGearing;
extern GUI_CONST_STORAGE GUI_BITMAP bmVehicleSettingsText;
extern GUI_CONST_STORAGE GUI_BITMAP bmSliderBlue;
extern GUI_CONST_STORAGE GUI_BITMAP bmSliderGreen;
extern GUI_CONST_STORAGE GUI_BITMAP bmTachSliderGreen;
extern GUI_CONST_STORAGE GUI_BITMAP bmTachSliderYellow;
extern GUI_CONST_STORAGE GUI_BITMAP bmTachSliderRed;
extern GUI_CONST_STORAGE GUI_BITMAP bmTraqdashCamBanner;

extern char *ZeroThroughNine[];
extern char *ZeroThroughNinty[];

extern void InputsSelectionPaint(WM_HWIN);
extern void PowerOffUnit(WM_HWIN owner, char reset, int delay);
extern void DisplaySplashScreen1(void);
extern char gpsSimulate;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void DisplayTachSelection(WM_HWIN);
void TachSelectionPaint(WM_HWIN);
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
static int GetScreenID(int Id);
void RSDisplayInputsSelection(WM_HWIN hParent);
void DisplayInputsSelection(WM_HWIN hParent, int idA0, int idA1, int idD4, int idA2, int idA3, int idD5);
void InputsSelectionPaint(WM_HWIN);
void DisplayGearRatio(WM_HWIN);
void GearRatioPaint(WM_HWIN);
void AeroGripScreenCallback(WM_MESSAGE * pMsg);
void AeroGripScreenPaint(void);
void CameraSelectionCallback(WM_MESSAGE * pMsg);
void CameraDelayCallback(WM_MESSAGE * pMsg);
void CarMetricsSelectionPaint(WM_HWIN hParent);
void CameraSetupPaint(WM_HWIN hParent);
void DisplayCarMetricsSelection(WM_HWIN hParent);
void DisplayCameraSetupSelection(WM_HWIN hParent);
void WeightSelectionPaint(WM_HWIN hParent);
void DisplayWeightSelection(WM_HWIN hParent);
int VehicleSettingsPopUp(int screen);
void AutoFixTach(void);
void AutoFixGears(void);
void AutoFixWeight(void);

static void VehicleSettingMainScreenCallback(WM_MESSAGE * pMsg);
void SetAnalogColorAndText(BUTTON_Handle hBtn, int index);
void SetDigitalColorAndText(BUTTON_Handle hBtn, int index);
void SetRedlineButtonText(void);
void SetYellowButtonText(void);
void SetNumCylsButtonText(void);
void ResetSliderParams(void);
void UpdateAll(void);
void UpdateTime(void);
void UpdateDistance(void);
void UpdateRPM(void);
void TestCameras(int);
void TraqdashCamPaint(void);
static void TraqdashCamCallback(WM_MESSAGE * pMsg);
enum LENGTH { SHORT, LONG };
const char *GetTraqdashCameraName(enum LENGTH length);
void SetCameraButtonColors(void);
void UpdateCameraVariables(void);
int CustomDelay( void );


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST VehicleSettingsScreenList[NUM_VEHICLE_SETTINGS_SCREENS+1] = {
	{ TACH_SELECTION, DisplayTachSelection, TachSelectionPaint },
	{ CARMETRICS_SELECTION, DisplayCarMetricsSelection, CarMetricsSelectionPaint },
	{ GEARRATIO_SELECTION, DisplayGearRatio, GearRatioPaint },
	{ INPUTS_SELECTION, RSDisplayInputsSelection, InputsSelectionPaint },
	{ WEIGHT_SELECTION, DisplayWeightSelection, WeightSelectionPaint },
	{ CAMERA_SETUP_SELECTION, DisplayCameraSetupSelection, CameraSetupPaint },
	{ -1, NULL, NULL }
};

//
// Widget IDs
//
#define	CAMERA_LIST				GUI_ID_LISTWHEEL0
#define	WEIGHT_WHEEL_1			GUI_ID_LISTWHEEL1
#define	WEIGHT_WHEEL_2			GUI_ID_LISTWHEEL2
#define	WEIGHT_WHEEL_3			GUI_ID_LISTWHEEL3

#define	GEAR_RATIO_BUTTON		GUI_ID_USER + 1
#define	TIRE_SIZE_BUTTON		GUI_ID_USER + 2
#define	DIFF_RATIO_BUTTON		GUI_ID_USER + 3
#define	TRANS_RATIO_BUTTON		GUI_ID_USER + 4
#define	HELP_BUTTON				GUI_ID_USER + 5
#define	WARNING_BUTTON			GUI_ID_USER + 6
#define	REDLINE_BUTTON			GUI_ID_USER + 7
#define	CYLINDERS_BUTTON		GUI_ID_USER + 8

#define	END_BUTTON				GUI_ID_USER + 10
#define	PEAK_BUTTON				GUI_ID_USER + 11
#define	ANALOG0_BUTTON			GUI_ID_USER + 12
#define	ANALOG1_BUTTON			GUI_ID_USER + 13
#define	ANALOG2_BUTTON			GUI_ID_USER + 14
#define	ANALOG3_BUTTON			GUI_ID_USER + 15
#define	DIGITAL4_BUTTON			GUI_ID_USER + 16
#define	DIGITAL5_BUTTON			GUI_ID_USER + 17
#define	SHOW_RPM				GUI_ID_USER + 18
#define	SHOW_SPEED				GUI_ID_USER + 19
#define	WIND_BUTTON				GUI_ID_USER + 20
#define	ROLL_BUTTON				GUI_ID_USER + 21

#define RESET_TIME_BUTTON		GUI_ID_USER + 22
#define RESET_DIST_BUTTON		GUI_ID_USER + 23
#define RESET_RPM_BUTTON		GUI_ID_USER + 24
#define AERO_GRIP_BUTTON		GUI_ID_USER + 25
#define RESET_ALL_BUTTON		GUI_ID_USER + 26

#define USB_CAM_BUTTON			GUI_ID_USER + 27
#define SENSOR_UNIT_BUTTON		GUI_ID_USER + 29
#define TEST_CAM_BUTTON			GUI_ID_USER + 30

#define MOBIUS_BUTTON			GUI_ID_USER + 31
#define REPLAY_PRIMEX_BUTTON	GUI_ID_USER + 32
#define REPLAY_MINI_BUTTON		GUI_ID_USER + 33
#define	NONE_BUTTON				GUI_ID_USER + 34
#define CUSTOM_DELAY_BUTTON		GUI_ID_USER + 35

#define	REDLINE_SLIDER			GUI_ID_SLIDER0
#define	YELLOW_SLIDER			GUI_ID_SLIDER1
#define	GREEN_SLIDER			GUI_ID_SLIDER2
#define	WIND_SLIDER				GUI_ID_SLIDER3
#define	ROLL_SLIDER				GUI_ID_SLIDER4

// Stand-alone screen - not used at the same time as the above enumeration!
#define	TACH_BUTTON				GUI_ID_USER + 1
#define	CARMETRICS_BUTTON		GUI_ID_USER + 2
#define	GEARRATIO_BUTTON		GUI_ID_USER + 3
#define	INPUTS_BUTTON			GUI_ID_USER + 4
#define	WEIGHT_BUTTON			GUI_ID_USER + 5
#define	CAMERA_BUTTON			GUI_ID_USER + 6

#define	SLIDER_RANGE			1000
#define	SLIDER_BALL_RADIUS		16
#define	SLIDER_START			70
#define	RPM_SLIDER_X_SIZE		284
#define	MEDIUM_BUTTON_WIDTH		97


struct CAMERAS {
	const char *pName;
	const char *pInterface;
	unsigned char enumeration;
};

struct CAMERAS NewCameras[] = {
	{ "None",				"None",			CAMERA_NONE },
	{ "Mobius",				"TraqData USB", CAMERA_MOBIUS },
	{ "Replay Mini",		"TraqData USB", CAMERA_REPLAY_MINI },
	{ "Replay Prime X",		"TraqData USB", CAMERA_REPLAY_PRIMEX },
	{ "Replay XD1080",		"TraqData USB", CAMERA_REPLAY_XD_USB },
	{ "Replay XD1080",		"TraqData HD2", CAMERA_REPLAY_HD },
	{ "GoPro HERO1 & 2",	"TraqData/Sync Pro", CAMERA_GOPRO_HD_PRO },
	{ "GoPro HERO1 & 2",	"TraqData HD2", CAMERA_GOPRO_HD },
	{ "Sony HD",			"TraqData HD",	CAMERA_SONY_HD },
	{ "RaceCam HD",			"TraqData HD",	CAMERA_RACECAM_HD },
	{ "ChaseCam SD LANC",	"TraqData HD",	CAMERA_CHASECAM_LANC },
	{ NULL,					NULL,			CAMERA_NONE }
};

#define	INVALID_TACH_SETTINGS	1
#define	INVALID_GEAR_RATIOS		2
#define	INVALID_WEIGHT			3
#define	NO_GEAR_RATIOS			4
#define	GOPROHD_REV_WARNING		5
#define	CHANGED_CAMERA_WARNING	6
#define	RESET_CAMERA_WARNING	7
#define	SU_REV370_WARNING		8

#define	CAMERA_TEST_DELAY	1000

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hVehicleSettingsScreens;
static int clickedOnVSScreen;
static int clickedOnVSMScreen;
static LISTWHEEL_Handle hWeightWheel1, hWeightWheel2, hWeightWheel3;
SLIDER_Handle hWindSlider, hRollSlider;
SLIDER_Handle hRedlineSlider, hYellowSlider, hCylindersSlider;
static BUTTON_Handle hAnalog0, hAnalog1, hAnalog2, hAnalog3, hDigital4, hDigital5;
BUTTON_Handle hRedlineBtn, hYellowBtn, hNumCylsBtn;

BUTTON_Handle hShowSpeedCheck, hShowRPMCheck;
WM_HWIN hVehicleSettingMainScreen;
static char redFirstTime, yellowFirstTime;
static char originalSUCameraSelection, originalTDCameraSelection, newSUCameraSelection, newTDCameraSelection;
static int tempCameraIndex = -1;			// Set to -1 to indicate never set.  Once set it will remain valid for this power cycle
LISTVIEW_Handle hCameraList;


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


///////////////////////////////////////////////////////////////////////////
//
// Special intermediate screen(s)
//
void Display_VehicleSettingMainScreen(WM_HWIN hPrev)
{
BUTTON_Handle hButton;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenScrollingHOnly();

	//GeneralConfigIgnoreNextKeyRelease();
//	firstTimeThrough = true;

	SetScreenType(VEHICLE_SETTING_SCREEN_TYPE);

	if ((hVehicleSettingMainScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, VehicleSettingMainScreenCallback, 0)) <= 0)
		SystemError();

	hButton = BUTTON_CreateEx(30, 38, 127, 115, hVehicleSettingMainScreen, WM_CF_SHOW, 0, TACH_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmTach);

	hButton = BUTTON_CreateEx(177, 38, 127, 115, hVehicleSettingMainScreen, WM_CF_SHOW, 0, CARMETRICS_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmCarMetricsButton);

	hButton = BUTTON_CreateEx(324, 38, 127, 115, hVehicleSettingMainScreen, WM_CF_SHOW, 0, GEARRATIO_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmGearRatio);

	hButton = BUTTON_CreateEx(30, 156, 127, 115, hVehicleSettingMainScreen, WM_CF_SHOW, 0, INPUTS_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmInputs);

	hButton = BUTTON_CreateEx(177, 156, 127, 115, hVehicleSettingMainScreen, WM_CF_SHOW, 0, WEIGHT_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmWeight);

	hButton = BUTTON_CreateEx(324, 156, 127, 115, hVehicleSettingMainScreen, WM_CF_SHOW, 0, CAMERA_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmCamera);

	hScreenTimer = WM_CreateTimer(hVehicleSettingMainScreen, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

static void VehicleSettingMainScreenCallback(WM_MESSAGE * pMsg) {
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(hVehicleSettingMainScreen, &BannerRect);
			}
			break;

	case WM_PAINT:
		GUI_DrawBitmap(&bmBackground, 0, 0);
		GUI_DrawBitmap(&bmVehicleSettingsText, 60, 1);
		GUI_DrawBitmap(&bmleftarrow, 0, 229);
		DrawSignalStrength();
		break;

	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
		NCode = pMsg->Data.v;                 // Notification code
		switch(NCode)
		{
			case WM_NOTIFICATION_CLICKED:
				clickedOnVSMScreen = true;
				break;

			case WM_NOTIFICATION_RELEASED:
				if (clickedOnVSMScreen)
					switch (Id)
					{
						case CARMETRICS_BUTTON:
						case WEIGHT_BUTTON:
						case TACH_BUTTON:
						case CAMERA_BUTTON:
						case INPUTS_BUTTON:
						case GEARRATIO_BUTTON:
							DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, Id - (TACH_BUTTON) + (TACH_SELECTION), hVehicleSettingMainScreen);

						default:
							break;
					}
				break;
	  };
	  break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

WM_HWIN GetVehicleSettingMainWindowHandle(void)
{
	return hVehicleSettingMainScreen;
}

void SetVehicleSettingMainWindowHandle(WM_HWIN hWin)
{
	hVehicleSettingMainScreen = hWin;
}


//
///////////////////////////////////////////////////////////////////////////



WM_HWIN GetVehicleSettingsWindowHandle(void)
{
	return hVehicleSettingsScreens;
}

void SetVehicleSettingsWindowHandle(WM_HWIN hWin)
{
	hVehicleSettingsScreens = hWin;
}


void VehicleSettingsScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;
	int Sel;
	int temp;
	signed int newPos, oldPos;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateWindow(pMsg->hWin);		// Allow timer to invalidate the whole window so we can show live data on the input buttons
			}
			else if (pMsg->Data.v == hCameraDelayTimer)
			{
				if (testCamera)
					SetUserMode(DU2_CAMERA_TEST);
				else
					SetUserMode(DU2_IDLE);
			}
			break;


		case WM_CREATE:
			VehicleSettingsIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == VEHICLE_SETTINGS_SCREENS_TYPE)
			{
				GUI_Clear();		// Clear the background here (and not in the 'constructor') to reduce flicker
				GUI_SetColor(GUI_WHITE);
				GUI_SetFont(&GUI_Font32_ASCII);

				// Call paint method of center screen
				VehicleSettingsScreenList[GetLastScreen(VEHICLE_SETTINGS_SCREENS_TYPE)].paint(hVehicleSettingsScreens);
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
						switch(Id)
						{
						case WEIGHT_WHEEL_1:
							{
								int temp;

								temp = ((Sel = LISTWHEEL_GetPos(hWeightWheel1)) * 1000) + (LISTWHEEL_GetPos(hWeightWheel2) * 100) + (LISTWHEEL_GetPos(hWeightWheel3) * 10);
								LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
								if (sysData.units == METRIC)
									vehicleData.weight = (int)(temp * KILOS_TO_POUNDS);
								else vehicleData.weight = temp;
							}
							break;

						case WEIGHT_WHEEL_2:
							{
								int temp;

								temp = (LISTWHEEL_GetPos(hWeightWheel1) * 1000) + ((Sel = LISTWHEEL_GetPos(hWeightWheel2)) * 100) + (LISTWHEEL_GetPos(hWeightWheel3) * 10);
								LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
								if (sysData.units == METRIC)
									vehicleData.weight = (int)(temp * KILOS_TO_POUNDS);
								else vehicleData.weight = temp;
							}
							break;

						case WEIGHT_WHEEL_3:
							{
								int temp;

								temp = (LISTWHEEL_GetPos(hWeightWheel1) * 1000) + (LISTWHEEL_GetPos(hWeightWheel2) * 100) + ((Sel = LISTWHEEL_GetPos(hWeightWheel3)) * 10);
								LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
								if (sysData.units == METRIC)
									vehicleData.weight = (int)(temp * KILOS_TO_POUNDS);
								else vehicleData.weight = temp;
							}
							break;
						}
					}
					break;

				case WM_NOTIFICATION_VALUE_CHANGED:
					switch (Id)
					{
					case REDLINE_SLIDER:
						oldPos = (vehicleData.tach.upperRedStart / TACH_SLIDER_STEP_SIZE) + 1;
						newPos = -1;
						if ((temp = SLIDER_GetValue(hRedlineSlider) - 1) < 0)	// Subtract 1 for left end extra position
						{
							temp = 0;
							newPos = oldPos;
						}
						else
						{
							temp *= TACH_SLIDER_STEP_SIZE;	// Subtract 1 for left end extra position
							if (temp < vehicleData.tach.upperYellowStart)
							{
								temp = vehicleData.tach.upperYellowStart;
								newPos = (temp / TACH_SLIDER_STEP_SIZE) + 1;
							}
							else if (temp > vehicleData.tach.scaleEnd)
							{
								temp = vehicleData.tach.upperRedStart;
								newPos = oldPos;
							}
							if (!redFirstTime)
							{
								vehicleData.tach.upperRedStart = temp;
								SetRedlineButtonText();
							}
							else redFirstTime = false;
						}

						if (newPos != -1)
							SLIDER_SetValue(hRedlineSlider, newPos);
						break;
					case YELLOW_SLIDER:
						oldPos = (vehicleData.tach.upperYellowStart / TACH_SLIDER_STEP_SIZE) + 1;
						newPos = -1;
						if ((temp = SLIDER_GetValue(hYellowSlider) - 1) < 0)
						{
							temp = 0;
							newPos = oldPos;
						}
						else
						{
							temp *= TACH_SLIDER_STEP_SIZE;
							if ((temp > vehicleData.tach.upperRedStart) && (temp > vehicleData.tach.scaleEnd))
							{
								temp = vehicleData.tach.upperYellowStart;
								newPos = oldPos;
							}
							else if (temp > vehicleData.tach.upperRedStart)
							{
								temp = vehicleData.tach.upperRedStart;
								newPos = (temp / TACH_SLIDER_STEP_SIZE) + 1;
							}
							else if (temp < vehicleData.tach.greenStart)
							{
								temp = vehicleData.tach.greenStart;
								newPos = (temp / TACH_SLIDER_STEP_SIZE) + 1;
							}
							if (!yellowFirstTime)
							{
								vehicleData.tach.upperYellowStart = temp;
								SetYellowButtonText();
							}
							else yellowFirstTime = false;
						}
						if (newPos != -1)
							SLIDER_SetValue(hYellowSlider, newPos);
						break;
					case GREEN_SLIDER:
						oldPos = CylindersToIndex(vehicleData.engineCylinders) + 1;
						newPos = -1;
						if ((temp = SLIDER_GetValue(hCylindersSlider) - 1) < 0)
						{
							temp = 0;
							newPos = oldPos;
						}
						else
						{
							if (temp > 10)
								newPos = oldPos;
							else
							{
								vehicleData.engineCylinders = IndexToCylinders(temp);
								SetNumCylsButtonText();
							}
						}
						if (newPos > 0)
							SLIDER_SetValue(hCylindersSlider, newPos);
						break;
					}
					break;

				case WM_NOTIFICATION_CLICKED:
					clickedOnVSScreen = true;
					switch(Id)
					{
						case CAMERA_LIST:
						case WEIGHT_WHEEL_1:
						case WEIGHT_WHEEL_2:
						case WEIGHT_WHEEL_3:
							pressingbutton_semaphore = 1;
							break;

						case REDLINE_SLIDER:
						case YELLOW_SLIDER:
						case GREEN_SLIDER:
							PauseScreenScrolling();
							break;

						default:
							WM_DefaultProc(pMsg);
							break;
					}
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;
					if (!clickedOnVSScreen)
						break;

					switch (Id)
					{
						case GEAR_RATIO_BUTTON:
							break;

						case TIRE_SIZE_BUTTON:
						case DIFF_RATIO_BUTTON:
						case TRANS_RATIO_BUTTON:
						case HELP_BUTTON:
							DisplayNewScreenList(GEARING_SCREENS_TYPE, GetScreenID(Id), hVehicleSettingsScreens);
							break;

						case WARNING_BUTTON:
						case REDLINE_BUTTON:
						case CYLINDERS_BUTTON:
						case END_BUTTON:
						case PEAK_BUTTON:
							DisplayNewScreenList(TACH_SETUP_SCREENS_TYPE, GetScreenID(Id), hVehicleSettingsScreens);
							break;

						case ANALOG0_BUTTON:
						case ANALOG1_BUTTON:
						case ANALOG2_BUTTON:
						case ANALOG3_BUTTON:
						case DIGITAL4_BUTTON:
						case DIGITAL5_BUTTON:
							DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetScreenID(Id), hVehicleSettingsScreens);
							break;

						case SHOW_SPEED:
							BUTTON_SetBitmapEx(hShowSpeedCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
							BUTTON_SetBitmap(hShowRPMCheck, BUTTON_CI_UNPRESSED, NULL);
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;

						case SHOW_RPM:
							BUTTON_SetBitmapEx(hShowRPMCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
							BUTTON_SetBitmap(hShowSpeedCheck, BUTTON_CI_UNPRESSED, NULL);
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;

						case REDLINE_SLIDER:
						case YELLOW_SLIDER:
						case GREEN_SLIDER:
							ResumeScreenScrolling();				// Allow screen scrolling when slider bars released
							break;

						case RESET_TIME_BUTTON:
							sysData.totalTimeEver = 0;
							UpdateTime();
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;

						case RESET_DIST_BUTTON:
							sysData.totalDistanceEver = 0;
							UpdateTime();
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;

						case RESET_RPM_BUTTON:
							sysData.maxRpmEver = 0;
							DataValues[MAX_RPM].uVal = 0;
							UpdateRPM();
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;

						case AERO_GRIP_BUTTON:
							CreateAeroGripScreen(hVehicleSettingsScreens);
							break;

						case RESET_ALL_BUTTON:
							sysData.totalTimeEver = 0;
							sysData.totalDistanceEver = 0;
							sysData.maxRpmEver = 0;
							UpdateAll();
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;

						case USB_CAM_BUTTON:
							CreateTraqdashCamScreen(hVehicleSettingsScreens);
							break;

						case SENSOR_UNIT_BUTTON:
							CreateCameraSelection(hVehicleSettingsScreens);
							break;

						case TEST_CAM_BUTTON:
							TestCameras (testCamera = !testCamera);			// change the state
							WM_InvalidateWindow(hVehicleSettingsScreens);
							break;
					}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void TestCameras(int turnOn)
{
	if ((turnOn != gpsSimulate) && (GetSUCam() != CAMERA_NONE) && (GetTDCam != TD_CAM_NONE)) {
		notifyGPSSimChange( gpsSimulate = turnOn );
#ifndef _WINDOWS
		OS_Delay(1000);
#endif
	}
	if (turnOn)
		SetUserMode(DU2_CAMERA_TEST);
	else
		SetUserMode(DU2_IDLE);
}

int GetScreenID(int Id)
{
	switch (Id)
	{
		case TIRE_SIZE_BUTTON:		return TIRESIZE_SELECTION_SCREEN;
		case DIFF_RATIO_BUTTON:		return DIFFRATIO_SCREEN;
		case TRANS_RATIO_BUTTON:	return TRANSRATIO_SCREEN;
		case HELP_BUTTON:			return HELP_SCREEN;
		case WARNING_BUTTON:		return WARNING_RPM_SETUP;
		case REDLINE_BUTTON:		return REDLINE_RPM_SETUP;
		case CYLINDERS_BUTTON:		return ENGINE_CYLINDERS_SETUP;
		case END_BUTTON:			return POWERBANDEND_RPM_SETUP;
		case PEAK_BUTTON:			return MAX_RPM_SETUP;
		case ANALOG0_BUTTON:		return ANALOG0;
		case ANALOG1_BUTTON:		return ANALOG1;
		case ANALOG2_BUTTON:		return ANALOG2;
		case ANALOG3_BUTTON:		return ANALOG3;
		case DIGITAL4_BUTTON:		return DIGITAL4;
		case DIGITAL5_BUTTON:		return DIGITAL5;
		default:					return 0;
	}
}


void DisplayTachSelection(WM_HWIN hParent)
{
	char scratch[20];
	BUTTON_Handle hBtn;

	BUTTON_SetDefaultFont(&GUI_Font24B_ASCII);
	BUTTON_SetDefaultTextColor(GUI_GREEN, BUTTON_BI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_GREEN, BUTTON_BI_PRESSED);

	hYellowBtn = BUTTON_CreateEx(375, 162, 87, 34, hParent, WM_CF_SHOW, 0, WARNING_BUTTON);
	BUTTON_SetBitmap(hYellowBtn, BUTTON_BI_UNPRESSED, &bmSmallBlackButton);

	hRedlineBtn = BUTTON_CreateEx(375, 107, 87, 34, hParent, WM_CF_SHOW, 0, REDLINE_BUTTON);
	BUTTON_SetBitmap(hRedlineBtn, BUTTON_BI_UNPRESSED, &bmSmallBlackButton);

	hCurrentRPMBtn = BUTTON_CreateEx(175, 52, 87, 34, hParent, WM_CF_SHOW, 0, 0);
	BUTTON_SetBitmap(hCurrentRPMBtn, BUTTON_BI_UNPRESSED, &bmSmallBlackButton);
	BUTTON_SetTextColor(hCurrentRPMBtn, BUTTON_BI_DISABLED, GUI_GREEN);

	hNumCylsBtn = BUTTON_CreateEx(375, 217, 87, 34, hParent, WM_CF_SHOW, 0, CYLINDERS_BUTTON);
	BUTTON_SetBitmap(hNumCylsBtn, BUTTON_BI_UNPRESSED, &bmSmallBlackButton);

	// Max RPM
	hBtn = BUTTON_CreateEx(375, 52, 87, 34, hParent, WM_CF_SHOW, 0, PEAK_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmSmallBlackButton);

	sprintf(scratch, "%u", vehicleData.tach.scaleEnd);
	BUTTON_SetText(hBtn, scratch);

	redFirstTime = yellowFirstTime = true;
	hRedlineSlider = SLIDER_CreateEx(SLIDER_START - SLIDER_BALL_RADIUS, 133 - SLIDER_BALL_RADIUS, RPM_SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hParent, WM_CF_SHOW, 0, REDLINE_SLIDER);
	SLIDER_SetNumTicks(hRedlineSlider, 0);
	hYellowSlider = SLIDER_CreateEx(SLIDER_START - SLIDER_BALL_RADIUS, 188 - SLIDER_BALL_RADIUS, RPM_SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hParent, WM_CF_SHOW, 0, YELLOW_SLIDER);
	SLIDER_SetNumTicks(hYellowSlider, 0);
	hCylindersSlider = SLIDER_CreateEx(SLIDER_START - SLIDER_BALL_RADIUS, 243 - SLIDER_BALL_RADIUS, RPM_SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hParent, WM_CF_SHOW, 0, GREEN_SLIDER);
	SLIDER_SetNumTicks(hCylindersSlider, 0);
	SLIDER_SetRange(hCylindersSlider, 0, 12);		// 11 steps for the number of cylinders slider plus two more steps to prevent ball going all the way to the end
	SLIDER_SetValue(hCylindersSlider, CylindersToIndex(vehicleData.engineCylinders) + 1);

	ResetSliderParams();
}

void SetRedlineButtonText(void)
{
	char scratch[20];

	sprintf(scratch, "%u", vehicleData.tach.upperRedStart);
	BUTTON_SetText(hRedlineBtn, scratch);
}

void SetYellowButtonText(void)
{
	char scratch[20];

	sprintf(scratch, "%u", vehicleData.tach.upperYellowStart);
	BUTTON_SetText(hYellowBtn, scratch);
}

void SetNumCylsButtonText(void)
{
	BUTTON_SetText(hNumCylsBtn, GetCylindersText());
}


void ResetSliderParams(void)
{
	int range, end;

	// Double-check limits
	end = vehicleData.tach.scaleEnd;
	if (vehicleData.tach.upperRedStart > end)
		vehicleData.tach.upperRedStart = end;
	if (vehicleData.tach.upperYellowStart > end)
		vehicleData.tach.upperYellowStart = end;
	if (vehicleData.tach.greenStart > end)
		vehicleData.tach.greenStart = end;

	range = (vehicleData.tach.scaleEnd / TACH_SLIDER_STEP_SIZE) + 2;		// Give two more steps to prevent ball going all the way to the end
	SLIDER_SetRange(hRedlineSlider, 0, range);
	SLIDER_SetRange(hYellowSlider, 0, range);

	SLIDER_SetValue(hRedlineSlider, (vehicleData.tach.upperRedStart / TACH_SLIDER_STEP_SIZE) + 1);		// Add 1 for end of slider
	SLIDER_SetValue(hYellowSlider, (vehicleData.tach.upperYellowStart / TACH_SLIDER_STEP_SIZE) + 1);

	SetRedlineButtonText();
	SetYellowButtonText();
	SetNumCylsButtonText();
}

void TachSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmTachSetup, 60, 0);

	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_SetTextMode(GUI_TM_TRANS);

	// Draw current RPM in box
	{
		char scratch[20];
		int rpm;
		rpm = GetValue(VEHICLE_RPM).iVal;
		sprintf(scratch, "%u", rpm);
		BUTTON_SetText(hCurrentRPMBtn, scratch);
	}



	GUI_SetColor(GUI_GRAY);
	GUI_DrawRoundedFrame(10, 40, XSIZE_PHYS - 10, YSIZE_PHYS - 10, 5, 2);
	GUI_SetColor(GUI_WHITE);
	GUI_FillRoundedRect(12, 42, XSIZE_PHYS - 12, YSIZE_PHYS - 12, 5);
	//GUI_DrawGradientRoundedV(22, 42, XSIZE_PHYS - 22, YSIZE_PHYS - 12, 5, GUI_WHITE, 0x00D0D0D0);
	GUI_SetPenSize(2);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(10, 41 + 55, XSIZE_PHYS - 10, 41 + 55);
	GUI_DrawLine(10, 41 + 110, XSIZE_PHYS - 10, 41 + 110);
	GUI_DrawLine(10, 41 + 165, XSIZE_PHYS - 10, 41 + 165);

	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringAt("Current RPM", 40, 58);
	GUI_DispStringAt("Max RPM", 273, 58);
	GUI_SetFont(&GUI_Font20B_ASCII);
	GUI_DispStringAt("Redline Start", 40, 100);
	GUI_DispStringAt("Warning Start", 40, 155);
	GUI_DispStringAt("Engine Cylinders", 40, 210);

	GUI_DrawBitmap(&bmTachSliderRed, SLIDER_START, 126);
	GUI_DrawBitmap(&bmTachSliderYellow, SLIDER_START, 181);
	GUI_DrawBitmap(&bmTachSliderGreen, SLIDER_START, 236);

	GUI_DrawBitmap(GetNavigationArrow(TACH_SELECTION), 0, 229);
}



void DisplayWeightSelection(WM_HWIN hParent)
{
	int thousands;
	int hundreds;
	int units;
	int weight;
	char *pText;

	if (sysData.units == METRIC)
	{
		weight = (int)(vehicleData.weight * POUNDS_TO_KILOS);
		pText = "Weight in kilos";
	}
	else
	{
		weight = vehicleData.weight;
		pText = "Weight in pounds";
	}

	thousands = weight / 1000;
	hundreds = (weight % 1000) / 100;
	units = (weight % 100) / 10;

	DisplayTripleListScreen(hParent,
		ZeroThroughNine, WEIGHT_WHEEL_1, &hWeightWheel1, thousands,
		ZeroThroughNine, WEIGHT_WHEEL_2, &hWeightWheel2, hundreds,
		ZeroThroughNinty, WEIGHT_WHEEL_3, &hWeightWheel3, units,
		pText,
		0, 0);
}

void RSDisplayInputsSelection(WM_HWIN hParent)
{
	DisplayInputsSelection(hParent, ANALOG0_BUTTON, ANALOG1_BUTTON, DIGITAL4_BUTTON, ANALOG2_BUTTON, ANALOG3_BUTTON, DIGITAL5_BUTTON);
}

void DisplayInputsSelection(WM_HWIN hParent, int idA0, int idA1, int idD4, int idA2, int idA3, int idD5)
{
	InputCleanUpKeyboards();

	hAnalog0 = BUTTON_CreateEx(36, 45, 121, 78, hParent, WM_CF_SHOW, 0, idA0);
	BUTTON_SetFont(hAnalog0, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hAnalog0, BUTTON_BI_UNPRESSED, GUI_BLACK);
	SetAnalogColorAndText(hAnalog0, 0);

	hAnalog1 = BUTTON_CreateEx(180, 45, 121, 78, hParent, WM_CF_SHOW, 0, idA1);
	BUTTON_SetFont(hAnalog1, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hAnalog1, BUTTON_BI_UNPRESSED, GUI_BLACK);
	SetAnalogColorAndText(hAnalog1, 1);

	hDigital4 = BUTTON_CreateEx(324, 45, 121, 78, hParent, WM_CF_SHOW, 0, idD4);
	BUTTON_SetFont(hDigital4, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hDigital4, BUTTON_BI_UNPRESSED, GUI_BLACK);
	SetDigitalColorAndText(hDigital4, 0);

	hAnalog2 = BUTTON_CreateEx(36, 155, 121, 78, hParent, WM_CF_SHOW, 0, idA2);
	BUTTON_SetFont(hAnalog2, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hAnalog2, BUTTON_BI_UNPRESSED, GUI_BLACK);
	SetAnalogColorAndText(hAnalog2, 2);

	hAnalog3 = BUTTON_CreateEx(180, 155, 121, 78, hParent, WM_CF_SHOW, 0, idA3);
	BUTTON_SetFont(hAnalog3, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hAnalog3, BUTTON_BI_UNPRESSED, GUI_BLACK);
	SetAnalogColorAndText(hAnalog3, 3);

	hDigital5 = BUTTON_CreateEx(324, 155, 121, 78, hParent, WM_CF_SHOW, 0, idD5);
	BUTTON_SetFont(hDigital5, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hDigital5, BUTTON_BI_UNPRESSED, GUI_BLACK);
	SetDigitalColorAndText(hDigital5, 1);
}

void InputsSelectionPaint(WM_HWIN hParent)
{
	GUI_RECT Rect;

	GUI_DrawBitmap(&bmInputsBackground, 0, 0);
	GUI_DrawBitmap(&bmInputText, 60, 0);

	Rect.x0 = 3;
	Rect.y0 = 0;
	Rect.x1 = 23;
	Rect.y1 = YSIZE_PHYS;
	GUI_SetFont(&GUI_Font20B_ASCII);
	GUI_SetColor(GUI_YELLOW);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_DispStringInRectEx("ANALOG",
		&Rect,
		GUI_TA_HCENTER,
		6,
		GUI_ROTATE_CCW);

	Rect.x0 = XSIZE_PHYS - 23;
	Rect.y0 = 0;
	Rect.x1 = XSIZE_PHYS - 3;
	Rect.y1 = YSIZE_PHYS;
	GUI_SetColor(GUI_CYAN);
	GUI_DispStringInRectEx("DIGITAL",
		&Rect,
		GUI_TA_HCENTER,
		20,
		GUI_ROTATE_CCW);


	GUI_SetColor(GUI_BLACK);					// Draw shadow text due to light background
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringHCenterAt(vehicleData.analogInput[0].Name, 98, 127);
	GUI_DispStringHCenterAt(vehicleData.analogInput[1].Name, 242, 127);
	GUI_DispStringHCenterAt(vehicleData.digitalInput[0].Name, 386, 127);

	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt(vehicleData.analogInput[0].Name, 96, 125);
	GUI_DispStringHCenterAt(vehicleData.analogInput[1].Name, 240, 125);
	GUI_DispStringHCenterAt(vehicleData.digitalInput[0].Name, 384, 125);
	GUI_DispStringHCenterAt(vehicleData.analogInput[2].Name, 96, 235);
	GUI_DispStringHCenterAt(vehicleData.analogInput[3].Name, 240, 235);
	GUI_DispStringHCenterAt(vehicleData.digitalInput[1].Name, 384, 235);

	// Draw the data values in USER UNITS
	SetAnalogColorAndText(hAnalog0, 0);
	SetAnalogColorAndText(hAnalog1, 1);
	SetAnalogColorAndText(hAnalog2, 2);
	SetAnalogColorAndText(hAnalog3, 3);
	SetDigitalColorAndText(hDigital4, 0);
	SetDigitalColorAndText(hDigital5, 1);

	GUI_DrawBitmap(GetNavigationArrow(INPUTS_SELECTION), 0, 229);
}


void DisplayGearRatio(WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 83, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TIRE_SIZE_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hBtn, "Enter Tire Size");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 129, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, DIFF_RATIO_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmYellowButton);
	BUTTON_SetText(hBtn, "Enter Diff Ratio");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 174, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TRANS_RATIO_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hBtn, "Enter Trans Ratios");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 218, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, HELP_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetText(hBtn, "Gearing Help");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
}

#define	LINE_SPACING	30
#define	LINE_GAP				15
#define	GEAR_TABLE_TOP		((LINE_SPACING*3) + 10)
#define	GEAR_TABLE_VGAP	25
#define	SECOND_COL_GAP		((((BOX_RIGHT - LINE_GAP) - (BOX_LEFT + LINE_GAP)) / 4) + LINE_GAP)
#define	THIRD_COL_GAP		((BOX_RIGHT - BOX_LEFT)/2 + (LINE_GAP/2))
#define	FORTH_COL_GAP		(((((BOX_RIGHT - LINE_GAP) - (BOX_LEFT + LINE_GAP)) / 4) * 3) + LINE_GAP)

void GearRatioPaint(WM_HWIN hParent)
{
	char scratch[20];

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmGearing, 60, 0);

	// Draw left side square for data
	GUI_DrawRoundedRect(BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(BOX_LEFT+2, BOX_TOP+2, BOX_RIGHT-2, BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(BOX_LEFT+2, BOX_TOP+2, BOX_RIGHT-2, BOX_BOTTOM - 2, 5);

	// draw 3 gray lines for field seperators
	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE_SPACING, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE_SPACING);

	GUI_SetPenSize(2);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE_SPACING*2, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE_SPACING*2);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE_SPACING*3, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE_SPACING*3);

	// Draw text:
	// Draw tire size
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_GREEN);
	if (vehicleData.userEnteredTireDimensions)
		sprintf(scratch, "%u/%u*%u", vehicleData.treadWidth, vehicleData.aspectRatio, vehicleData.wheelDiameter);
	else GetTireCircumferenceString(true, scratch);		// Get text of circumference and include the units text
	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + 4);

	// Draw trans ratio
	GUI_SetColor(GUI_YELLOW);
	if (vehicleData.diffRatio == 0.0)
		strcpy(scratch, "Gearing Disabled");
	else sprintf(scratch, "%.2f", vehicleData.diffRatio);
	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + LINE_SPACING + 4);

	// Draw fixed text Transmission Ratios
//	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_RED);
	GUI_DispStringHCenterAt("Transmission Ratios", BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + LINE_SPACING*2 + 4);

	// Draw gear numbers
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringAt("1st:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP);
	GUI_DispStringAt("2nd:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);
	GUI_DispStringAt("3rd:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*2);
	GUI_DispStringAt("4th:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*3);
	GUI_DispStringAt("5th:", BOX_LEFT + THIRD_COL_GAP, BOX_TOP + GEAR_TABLE_TOP);
	GUI_DispStringAt("6th:", BOX_LEFT + THIRD_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);
	GUI_DispStringAt("7th:", BOX_LEFT + THIRD_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*2);
	GUI_DispStringAt("8th:", BOX_LEFT + THIRD_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*3);

	// Draw gear ratios
	GUI_SetColor(GUI_RED);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[0]);
	GUI_DispStringAt(scratch, BOX_LEFT + SECOND_COL_GAP, BOX_TOP + GEAR_TABLE_TOP);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[1]);
	GUI_DispStringAt(scratch, BOX_LEFT + SECOND_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[2]);
	GUI_DispStringAt(scratch, BOX_LEFT + SECOND_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*2);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[3]);
	GUI_DispStringAt(scratch, BOX_LEFT + SECOND_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*3);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[4]);
	GUI_DispStringAt(scratch, BOX_LEFT + FORTH_COL_GAP, BOX_TOP + GEAR_TABLE_TOP);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[5]);
	GUI_DispStringAt(scratch, BOX_LEFT + FORTH_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[6]);
	GUI_DispStringAt(scratch, BOX_LEFT + FORTH_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*2);
	sprintf(scratch, "%.2f", vehicleData.gearRatios[7]);
	GUI_DispStringAt(scratch, BOX_LEFT + FORTH_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP*3);

	GUI_DrawBitmap(GetNavigationArrow(GEARRATIO_SELECTION), 0, 229);
}

void WeightSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_1, 49);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_2, 49);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_3, 49);

	GUI_DrawBitmap(&bmVehicleWeight, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(WEIGHT_SELECTION), 0, 229);
}

#define	WIND_Y				85
#define	ROLL_Y				160
#define	SLIDER_LEFT			30
#define	AG_SLIDER_X_SIZE	343

GUI_HWIN GetAeroGripWindowHandle(void)
{
	return hAeroGripScreen;
}

void CreateAeroGripScreen(WM_HWIN hParent)
{
	if (hParent)
		WM_DeleteWindow(hParent);

	SetScreenType(AERO_GRIP_SCREEN_TYPE);

	if ((hAeroGripScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, AeroGripScreenCallback, 0)) <= 0)
		SystemError();

	hWindSlider = SLIDER_CreateEx(SLIDER_LEFT - SLIDER_BALL_RADIUS, WIND_Y - SLIDER_BALL_RADIUS, AG_SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hAeroGripScreen, WM_CF_SHOW, 0, WIND_SLIDER);
	SLIDER_SetRange(hWindSlider, 0, 1000);
	SLIDER_SetNumTicks(hWindSlider, 0);

	hRollSlider = SLIDER_CreateEx(SLIDER_LEFT - SLIDER_BALL_RADIUS, ROLL_Y - SLIDER_BALL_RADIUS, AG_SLIDER_X_SIZE + (SLIDER_BALL_RADIUS * 2), SLIDER_BALL_RADIUS * 2, hAeroGripScreen, WM_CF_SHOW, 0, ROLL_SLIDER);
	SLIDER_SetRange(hRollSlider, 0, 1000);
	SLIDER_SetNumTicks(hRollSlider, 0);

	// Set the sliders to the initial value.  Note: DO NOT set the slider until AFTER the
	// button windows are created as they will be updated when the value is set on the
	// slider.
	SLIDER_SetValue(hWindSlider, (int)(vehicleData.windDrag * SLIDER_RANGE));
	SLIDER_SetValue(hRollSlider, (int)(vehicleData.rollingResistance * SLIDER_RANGE));

	hScreenTimer = WM_CreateTimer(hAeroGripScreen, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}


void AeroGripScreenCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateWindow(pMsg->hWin);		// Allow timer to invalidate the whole window so we can show live data on the input buttons
			}
			break;


		case WM_CREATE:
			VehicleSettingsIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			AeroGripScreenPaint();
			DrawSignalStrength();
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_VALUE_CHANGED:
					switch (Id)
					{
					case WIND_SLIDER:
						vehicleData.windDrag = SLIDER_GetValue(hWindSlider) / (float)SLIDER_RANGE;
						WM_InvalidateWindow(hAeroGripScreen);
						break;
					case ROLL_SLIDER:
						vehicleData.rollingResistance = SLIDER_GetValue(hRollSlider) / (float)SLIDER_RANGE;
						WM_InvalidateWindow(hAeroGripScreen);
						break;
					}
					break;
			}
			break;

		default:
		  WM_DefaultProc(pMsg);
	}
}

void AeroGripScreenPaint(void)
{
	char scratch[40];

	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmAeroandTire, 60, 0);


	GUI_DrawRoundedRect(10, 40, 460, 250, 5);
	GUI_FillRoundedRect(10, 40, 460, 250, 5);

	GUI_SetTextMode(GUI_TM_REV);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringHCenterAt("Drag Coefficient (Cd) x Frontal Area (m^2)", 230, WIND_Y - 40);
	GUI_DispStringHCenterAt("Tire Friction Crr Rolling Resistance", 230, ROLL_Y - 40);

	// Display coefficients values
	sprintf(scratch, "%0.03f", vehicleData.windDrag);
	GUI_DispStringHCenterAt(scratch, 420,  WIND_Y - 11);
	sprintf(scratch, "%0.03f", vehicleData.rollingResistance);
	GUI_DispStringHCenterAt(scratch, 420,  ROLL_Y - 10);

	sprintf(scratch, "Vehicle: %s", sysData.vehicle);
	GUI_DispStringHCenterAt(scratch, XSIZE_PHYS/2, 210);

	GUI_DrawBitmap(&bmSliderBlue, SLIDER_LEFT, WIND_Y - 5);
	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(10, WIND_Y + 30, 460, WIND_Y + 30);

	GUI_DrawBitmap(&bmSliderGreen, SLIDER_LEFT, ROLL_Y - 5);
	GUI_DrawLine(10, ROLL_Y + 30, 460, ROLL_Y + 30);

	GUI_DrawBitmap(GetNavigationArrow(CARMETRICS_SELECTION), 0, 229);
	GUI_SetTextMode(GUI_TEXTMODE_NORMAL);

	BUTTON_SetDefaultFont(&GUI_Font24B_ASCII);
}

#define	TOP_BUT			48
#define	GAP				43
void DisplayCarMetricsSelection(WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RESET_TIME_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetText(hBtn, "Reset Total Time");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (1 * GAP), BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RESET_DIST_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmYellowButton);
	BUTTON_SetText(hBtn, "Reset Total Distance");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (2 * GAP), BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RESET_RPM_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hBtn, "Reset Max RPM");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (3 * GAP), BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, AERO_GRIP_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hBtn, "Aero/Grip");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (4 * GAP), BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RESET_ALL_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmOrangeButton);
	BUTTON_SetText(hBtn, "Reset All");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_BLACK);
}

#define	FIVE_BTN_BOX_LEFT			27
#define	FIVE_BTN_BOX_RIGHT			245
#define	CAMERA_TEXT_CENTER			136
#define	HALF_CAMERA_LINE			89
#define	FIVE_BTN_BOX_TOP			TOP_BUT
#define	FIVE_BTN_BOX_BOTTOM			(TOP_BUT + 209)
#define	CM_LINE_GAP					20
#define	CM_TEXT_START				(FIVE_BTN_BOX_LEFT + 10)
#define	CM_TEXT_OFFSET				12
const GUI_RECT TimeRect = { 100, TOP_BUT +             CM_TEXT_OFFSET, FIVE_BTN_BOX_RIGHT - 10, TOP_BUT +             CM_TEXT_OFFSET + 20 };
const GUI_RECT DistRect = { 100, TOP_BUT + (1 * GAP) + CM_TEXT_OFFSET, FIVE_BTN_BOX_RIGHT - 10, TOP_BUT + (1 * GAP) + CM_TEXT_OFFSET + 20 };
const GUI_RECT RPMRect =  { 100, TOP_BUT + (2 * GAP) + CM_TEXT_OFFSET, FIVE_BTN_BOX_RIGHT - 10, TOP_BUT + (2 * GAP) + CM_TEXT_OFFSET + 20 };

void CarMetricsSelectionPaint(WM_HWIN hParent)
{
	char scratch[10];

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmCarMetricsBanner, 60, 0);

	GUI_SetFont(&GUI_Font20_ASCII);

	// Draw left side square for data
	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);

	// draw 3 gray lines for field seperators
	GUI_SetPenSize(2);
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DrawLine(FIVE_BTN_BOX_LEFT + CM_LINE_GAP, TOP_BUT + (1 * GAP) - 3, FIVE_BTN_BOX_RIGHT - CM_LINE_GAP, TOP_BUT + (1 * GAP) - 3);
	GUI_DrawLine(FIVE_BTN_BOX_LEFT + CM_LINE_GAP, TOP_BUT + (2 * GAP) - 3, FIVE_BTN_BOX_RIGHT - CM_LINE_GAP, TOP_BUT + (2 * GAP) - 3);
	GUI_DrawLine(FIVE_BTN_BOX_LEFT + CM_LINE_GAP, TOP_BUT + (3 * GAP) - 3, FIVE_BTN_BOX_RIGHT - CM_LINE_GAP, TOP_BUT + (3 * GAP) - 3);

	// Draw static text
	GUI_DispStringAt("Total Time:",		CM_TEXT_START, TOP_BUT +			 CM_TEXT_OFFSET);
	GUI_DispStringAt("Total Distance:", CM_TEXT_START, TOP_BUT + (1 * GAP) + CM_TEXT_OFFSET);
	GUI_DispStringAt("Max RPM:",		CM_TEXT_START, TOP_BUT + (2 * GAP) + CM_TEXT_OFFSET);
	GUI_DispStringAt("Aero (Cd)",		CM_TEXT_START, TOP_BUT + (3 * GAP) + CM_TEXT_OFFSET);
	GUI_DispStringAt("Grip (Crr)",		FIVE_BTN_BOX_RIGHT - 85, TOP_BUT + (3 * GAP) + CM_TEXT_OFFSET);

	// Draw Data Values
	UpdateAll();

	// Aero
	GUI_SetColor(GUI_GREEN);
	sprintf(scratch, "%0.03f", vehicleData.windDrag);
	GUI_DispStringHCenterAt(scratch, 75, TOP_BUT + (4 * GAP));

	// Grip
	sprintf(scratch, "%0.03f", vehicleData.rollingResistance);
	GUI_DispStringHCenterAt(scratch, 200, TOP_BUT + (4 * GAP));

	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}

// Draw All Data Values
void UpdateAll(void)
{
	UpdateTime();
	UpdateDistance();
	UpdateRPM();
}

// Total Time
void UpdateTime(void)
{
	char scratch[20];

	GUI_SetFont(&GUI_Font20_ASCII);				// Be sure font is selected at this time (might be called from button press)

	if (sysData.totalTimeEver >= 360000)		// if 100 hours or more
		sprintf(scratch, "%u hr", (int)sysData.totalTimeEver / 3600);
	else sprintf(scratch, sysData.totalTimeEver >= 36000 ? "%0.1f hr" : "%0.2f hr", (float)sysData.totalTimeEver / 3600.0);

	GUI_SetColor(GUI_LIGHTBLUE);
	GUI_DispStringInRect(scratch, (GUI_RECT *)&TimeRect, GUI_TA_RIGHT);
}

// Total Distance
void UpdateDistance(void)
{
	char scratch[20];

	GUI_SetFont(&GUI_Font20_ASCII);				// Be sure font is selected at this time (might be called from button press)

	if (sysData.units == STD)
	{
		if (sysData.totalDistanceEver >= 160934)		// if 100 miles or more
			sprintf(scratch, "%u mi", (int)(sysData.totalDistanceEver / 1609.34));
		else sprintf(scratch, sysData.totalDistanceEver >= 16093.4 ? "%0.1f mi" : "%0.2f mi", sysData.totalDistanceEver / 1609.34);
	}
	else
	{
		if (sysData.totalDistanceEver >= 100000)		// if 100 km or more
			sprintf(scratch, "%u km", (int)(sysData.totalDistanceEver / 1000.0));
		else sprintf(scratch, sysData.totalDistanceEver >= 10000.0 ? "%0.1f km" : "%0.2f km", sysData.totalDistanceEver / 1000.0);
	}

	GUI_SetColor(GUI_YELLOW);
	GUI_DispStringInRect(scratch, (GUI_RECT *)&DistRect, GUI_TA_RIGHT);
}

// Max RPM
void UpdateRPM(void)
{
	char scratch[20];

	GUI_SetFont(&GUI_Font20_ASCII);				// Be sure font is selected at this time (might be called from button press)

	sprintf(scratch, "%u", sysData.maxRpmEver);
	GUI_SetColor(GUI_RED);
	GUI_DispStringInRect(scratch, (GUI_RECT *)&RPMRect, GUI_TA_RIGHT);
}

#define	CAMERA_BUTTON_ONE	99
#define	CAMERA_BUTTON_TWO	152
#define	CAMERA_BUTTON_THREE	216
#define	LEFT_LINE_WIDTH		180
#define	CAMERA_LINE_1_Y		60
#define	TD_CAMERA_LINE_Y	120
#define	SU_CAMERA_LINE_Y	190
#define	CAMERA_STATUS_LINE	230
#define	CAMERA_UPPER_LINE_OFFSET	25
#define	CAMERA_LOWER_LINE_OFFSET	5
#define	CAMERA_BUTTON_LINE	203

void DisplayCameraSetupSelection(WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	if (unsavedSysData.unitInfo.bootloaderSwRev > FIRST_GEN_BOOTLOADER)
	{
		hBtn = BUTTON_CreateEx(BUTTON_LEFT, CAMERA_BUTTON_ONE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, USB_CAM_BUTTON);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
		BUTTON_SetText(hBtn, "TraqDash USB Port");
		BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_WHITE);
		BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_WHITE);
	}

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, CAMERA_BUTTON_TWO, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, SENSOR_UNIT_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hBtn, "Sensor Unit Box");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, CAMERA_BUTTON_THREE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TEST_CAM_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hBtn, "Test Camera(s)");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_BLACK);
}


void CameraSetupPaint(WM_HWIN hParent)
{
	char scratch[30];

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmCameraSetupBanner, 60, 0);

	GUI_SetFont(&GUI_Font20_ASCII);

	// Draw left side square for data
	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);

	// Prepare to draw gray lines for field seperators
	GUI_SetPenSize(2);
	GUI_SetColor(GUI_LIGHTGRAY);

	// Draw static text
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_DispStringHCenterAt("Camera Connection", BUTTON_LEFT + (BUTTON_WIDTH / 2), TOP_BUT + (0 * GAP) + CM_TEXT_OFFSET);

	if (unsavedSysData.unitInfo.bootloaderSwRev > FIRST_GEN_BOOTLOADER)
	{
		// draw 3 gray lines for field seperators
		GUI_SetPenSize(2);
		GUI_SetColor(GUI_LIGHTGRAY);
		GUI_DrawLine(CAMERA_TEXT_CENTER - HALF_CAMERA_LINE, TD_CAMERA_LINE_Y, CAMERA_TEXT_CENTER + HALF_CAMERA_LINE, TD_CAMERA_LINE_Y);
		GUI_DrawLine(CAMERA_TEXT_CENTER - HALF_CAMERA_LINE, SU_CAMERA_LINE_Y, CAMERA_TEXT_CENTER + HALF_CAMERA_LINE, SU_CAMERA_LINE_Y);

		// Draw line separating Sensor Unit Box button and Text Camera(s) button
		GUI_DrawLine((BUTTON_LEFT + (BUTTON_WIDTH / 2)) - (LEFT_LINE_WIDTH / 2), CAMERA_BUTTON_LINE, (BUTTON_LEFT + (BUTTON_WIDTH / 2)) + (LEFT_LINE_WIDTH / 2), CAMERA_BUTTON_LINE);
		// Draw static text
		// Draw USB Camera Enable State
		GUI_SetColor(GUI_LIGHTGRAY);
		GUI_DispStringHCenterAt("Camera Settings", CAMERA_TEXT_CENTER, CAMERA_LINE_1_Y);
		GUI_DispStringHCenterAt("TraqDash USB Port", CAMERA_TEXT_CENTER, TD_CAMERA_LINE_Y - CAMERA_UPPER_LINE_OFFSET);
		GUI_DispStringHCenterAt("Sensor Unit Camera Box", CAMERA_TEXT_CENTER, SU_CAMERA_LINE_Y - CAMERA_UPPER_LINE_OFFSET);
		
		GUI_SetColor(GUI_LIGHTBLUE);
		if (newTDCameraSelection == TD_CAM_NONE)
			sprintf(scratch, "%s", GetTraqdashCameraName(SHORT));
		else 
			sprintf(scratch, "%s (%u.%usec)", GetTraqdashCameraName(SHORT), (vehicleData.usbCameraDelay & 0x7F) / 10, (vehicleData.usbCameraDelay & 0x7F) % 10);
		GUI_DispStringHCenterAt(scratch, CAMERA_TEXT_CENTER, TD_CAMERA_LINE_Y + CAMERA_LOWER_LINE_OFFSET);

		GUI_SetColor(GUI_RED);
		GUI_DispStringHCenterAt(GetCameraName(), CAMERA_TEXT_CENTER, SU_CAMERA_LINE_Y + CAMERA_LOWER_LINE_OFFSET);
	}
	else
	{
		// draw 1 gray line for field seperator
		GUI_DrawLine(FIVE_BTN_BOX_LEFT + CM_LINE_GAP, TOP_BUT + (3 * GAP) - 3, FIVE_BTN_BOX_RIGHT - CM_LINE_GAP, TOP_BUT + (3 * GAP) - 3);

		// Display not-supported text
		GUI_SetColor(GUI_WHITE);
		GUI_DispStringHCenterAt("TraqDash USB Camera", 135, TOP_BUT + 22);
		GUI_DispStringHCenterAt("Not Supported", 135, TOP_BUT + GAP + 5);
	}

	// Display camera(s) active/not active
	GUI_SetColor(GUI_GREEN);
	if (testCamera)
		GUI_DispStringHCenterAt("Camera(s) Active", CAMERA_TEXT_CENTER, CAMERA_STATUS_LINE);
	else GUI_DispStringHCenterAt("Camera(s) Not Active", CAMERA_TEXT_CENTER, CAMERA_STATUS_LINE);

	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}

int CustomDelay( void )
{
	if (newTDCameraSelection == TD_CAM_MOBIUS)
	{
		if (vehicleData.usbCameraDelay == MOBIUS_DELAY)
			return false;
	}
	else if (newTDCameraSelection == TD_CAM_PRIMEX)
	{
		if (vehicleData.usbCameraDelay == PRIME_X_DELAY)
			return false;
	}
	else if (newTDCameraSelection == TD_CAM_MINI)
	{
		if (vehicleData.usbCameraDelay == MINI_DELAY)
			return false;
	}
	else if (newTDCameraSelection == TD_CAM_NONE)
		return false;

	return true;
}

char *GetFullCameraText(char *pBuffer)
{
	strcpy(pBuffer, "Selected Camera(s)\nSU: ");
	strcat(pBuffer, GetCameraName());
	
	strcat(pBuffer, "\nUSB: ");
	if (0x80 & vehicleData.usbCameraDelay)		// upper bit is enable bit
		strcat(pBuffer, "USB Camera");
	else
		strcat(pBuffer, "None");
						
	strcat(pBuffer, "\n\n");
	return pBuffer;
}

const char *GetCameraName(void)
{
	int i = 0;
	while (NewCameras[i].pName)
	{
		// See if this is the selected camera
		if (GetSUCam() == NewCameras[i].enumeration)
			return NewCameras[i].pName;

		i++;
	}
	return NewCameras[0].pName;
}

const char *GetTraqdashCameraName(enum LENGTH length)
{
	if (newTDCameraSelection == TD_CAM_MOBIUS)
		return "Mobius";
	else if (newTDCameraSelection == TD_CAM_PRIMEX)
	{
		//if (length == LONG)		if we don't display "Delay" we can fit "Replay"
		//	return "Replay Prime-X";
		//else return "Prime-X";			// not everywhere can fit the whole name
		return "Replay Prime-X";
	}
	else if (newTDCameraSelection == TD_CAM_MINI)
		return "Replay Mini";

	else newTDCameraSelection = TD_CAM_NONE;		// Clear out ANY other SU camera selection
	return "None selected";
}

void ValidateCameraData(void)
{
//
// Initialize working variables used by the camera displays and controls
// Note that we pay power up directly into a camera setup screen
//
	originalTDCameraSelection = newTDCameraSelection = vehicleData.camera & TD_CAMERA_MASK;
	originalSUCameraSelection = newSUCameraSelection = vehicleData.camera & SU_CAMERA_MASK;

	// If upgrading old camera selections to new selections - warn user
	if ((newSUCameraSelection == CAMERA_CHASECAM) ||
		(newSUCameraSelection == CAMERA_SONY_CAMCORDER) ||
		(newSUCameraSelection == CAMERA_SONY_VCR_HARD) ||
		(newSUCameraSelection == CAMERA_SONY_VCR_SOFT))
	{
		//
		// Glenn - I'm not sure what we're trying to do here!   What defaults do you want to select in these cases?
		//
		if (vehicleData.usbCameraDelay & 0x80)	// usb camera selected
			vehicleData.camera = CAMERA_USB;
		else	// usb camera cleared
			vehicleData.camera = CAMERA_NONE;

		VehicleSettingsPopUp(RESET_CAMERA_WARNING);
		// Popup destroyed the splash screen - redisplay it
		DisplaySplashScreen1();

		// Update variables on more time after altering!
		originalTDCameraSelection = newTDCameraSelection = vehicleData.camera & TD_CAMERA_MASK;
		originalSUCameraSelection = newSUCameraSelection = vehicleData.camera & SU_CAMERA_MASK;
	}

}


void CreateTraqdashCamScreen(WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	if (hParent)
		WM_DeleteWindow(hParent);

	SetScreenType(TRAQDASH_CAMERA_SCREEN);

	if ((hTraqdashCam = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, TraqdashCamCallback, 0)) <= 0)
		SystemError();

	BUTTON_SetDefaultFont(&GUI_Font20_ASCII);

	hBtnMobius = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (1 * GAP), MEDIUM_BUTTON_WIDTH, BUTTON_HEIGHT, hTraqdashCam, WM_CF_SHOW, 0, MOBIUS_BUTTON);
	BUTTON_SetText(hBtnMobius, "Mobius");
	BUTTON_SetTextColor(hBtnMobius, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnMobius, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnMobius, BUTTON_CI_PRESSED, GUI_WHITE);

	hBtnPrimeX = BUTTON_CreateEx(BUTTON_LEFT + 103, TOP_BUT + (1 * GAP), MEDIUM_BUTTON_WIDTH, BUTTON_HEIGHT, hTraqdashCam, WM_CF_SHOW, 0, REPLAY_PRIMEX_BUTTON);
	BUTTON_SetText(hBtnPrimeX, "Prime-X");
	BUTTON_SetTextColor(hBtnPrimeX, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnPrimeX, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnPrimeX, BUTTON_CI_PRESSED, GUI_WHITE);

	hBtnMini = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (2 * GAP), MEDIUM_BUTTON_WIDTH, BUTTON_HEIGHT, hTraqdashCam, WM_CF_SHOW, 0, REPLAY_MINI_BUTTON);
	BUTTON_SetText(hBtnMini, "Mini");
	BUTTON_SetTextColor(hBtnMini, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnMini, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnMini, BUTTON_CI_PRESSED, GUI_WHITE);

	hBtnNone = BUTTON_CreateEx(BUTTON_LEFT + 103, TOP_BUT + (2 * GAP), MEDIUM_BUTTON_WIDTH, BUTTON_HEIGHT, hTraqdashCam, WM_CF_SHOW, 0, NONE_BUTTON);
	BUTTON_SetText(hBtnNone, "None");
	BUTTON_SetTextColor(hBtnNone, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnNone, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtnNone, BUTTON_CI_PRESSED, GUI_WHITE);

	SetCameraButtonColors();		// Set the button colors based on the current selection

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, TOP_BUT + (4 * GAP), BUTTON_WIDTH, BUTTON_HEIGHT, hTraqdashCam, WM_CF_SHOW, 0, CUSTOM_DELAY_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
	BUTTON_SetText(hBtn, "Custom Sync Time");
	BUTTON_SetTextColor(hBtn, BUTTON_CI_DISABLED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_WHITE);

	hScreenTimer = WM_CreateTimer(hTraqdashCam, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

void SetCameraButtonColors(void)
{
	BUTTON_SetBitmap(hBtnMobius, BUTTON_BI_UNPRESSED, (newTDCameraSelection == TD_CAM_MOBIUS) ? &bmMediumBlueButton : &bmMediumBlueButtonDark);
	BUTTON_SetBitmap(hBtnPrimeX, BUTTON_BI_UNPRESSED, (newTDCameraSelection == TD_CAM_PRIMEX) ? &bmMediumBlueButton : &bmMediumBlueButtonDark);
	BUTTON_SetBitmap(hBtnMini,   BUTTON_BI_UNPRESSED, (newTDCameraSelection == TD_CAM_MINI)   ? &bmMediumBlueButton : &bmMediumBlueButtonDark);
	BUTTON_SetBitmap(hBtnNone,   BUTTON_BI_UNPRESSED, (newTDCameraSelection == TD_CAM_NONE)   ? &bmMediumBlueButton : &bmMediumBlueButtonDark);
}

GUI_HWIN GetTraqdashCamWindowHandle(void)
{
	return hTraqdashCam;
}


static void TraqdashCamCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(hTraqdashCam, &BannerRect);
			}
			break;

	case WM_PAINT:
		TraqdashCamPaint();
		break;

	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
		NCode = pMsg->Data.v;                 // Notification code
		switch(NCode)
		{
		case WM_NOTIFICATION_CLICKED:
			switch (Id)
			{
				case MOBIUS_BUTTON:
					newTDCameraSelection = TD_CAM_MOBIUS;
					vehicleData.usbCameraDelay = MOBIUS_DELAY;
					SetCameraButtonColors();
					WM_InvalidateWindow(hTraqdashCam);
					break;

				case REPLAY_PRIMEX_BUTTON:
					newTDCameraSelection = TD_CAM_PRIMEX;
					vehicleData.usbCameraDelay = PRIME_X_DELAY;
					SetCameraButtonColors();
					WM_InvalidateWindow(hTraqdashCam);
					break;

				case REPLAY_MINI_BUTTON:
					newTDCameraSelection = TD_CAM_MINI;
					vehicleData.usbCameraDelay = MINI_DELAY;
					SetCameraButtonColors();
					WM_InvalidateWindow(hTraqdashCam);
					break;

				case NONE_BUTTON:
					newTDCameraSelection = TD_CAM_NONE;
					SetCameraButtonColors();
					WM_InvalidateWindow(hTraqdashCam);
					break;

				case CUSTOM_DELAY_BUTTON:
					CreateCameraDelay(hTraqdashCam);
					break;
			}
			break;
	  };
	  break;

	default:
		  WM_DefaultProc(pMsg);
	};
}


void TraqdashCamPaint(void)
{
	char scratch[30];

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmTraqdashCamBanner, 60, 0);

	GUI_SetFont(&GUI_Font20_ASCII);

	// Draw left side square for data
	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT, FIVE_BTN_BOX_TOP, FIVE_BTN_BOX_RIGHT, FIVE_BTN_BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(FIVE_BTN_BOX_LEFT+2, FIVE_BTN_BOX_TOP+2, FIVE_BTN_BOX_RIGHT-2, FIVE_BTN_BOX_BOTTOM - 2, 5);

	// Draw static text
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DispStringHCenterAt("Select Camera", BUTTON_LEFT + (BUTTON_WIDTH / 2), TOP_BUT + (0 * GAP) + CM_TEXT_OFFSET);
	GUI_DispStringHCenterAt("Current Settings", CAMERA_TEXT_CENTER, CAMERA_LINE_1_Y);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("TraqDash USB Port", CAMERA_TEXT_CENTER, TD_CAMERA_LINE_Y - CAMERA_UPPER_LINE_OFFSET);

	// draw gray lines for field seperator
	GUI_SetPenSize(2);
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DrawLine(CAMERA_TEXT_CENTER - HALF_CAMERA_LINE, TD_CAMERA_LINE_Y, CAMERA_TEXT_CENTER + HALF_CAMERA_LINE, TD_CAMERA_LINE_Y);

	GUI_SetColor(GUI_LIGHTBLUE);
	sprintf(scratch, "%s", GetTraqdashCameraName(LONG));
	GUI_DispStringHCenterAt(scratch, CAMERA_TEXT_CENTER, TD_CAMERA_LINE_Y + CAMERA_LOWER_LINE_OFFSET);

	if (newTDCameraSelection == TD_CAM_NONE)
	{
		GUI_SetColor(GUI_WHITE);
		GUI_DispStringHCenterAt("No Camera Enabled", CAMERA_TEXT_CENTER, 225);
	}
	else
	{
		if (CustomDelay())
			GUI_SetColor(GUI_WHITE);
		sprintf(scratch, "%u.%usec", (vehicleData.usbCameraDelay & 0x7F) / 10, (vehicleData.usbCameraDelay & 0x7F) % 10);
		GUI_DispStringHCenterAt(scratch, CAMERA_TEXT_CENTER, 200);

		GUI_DispStringHCenterAt(CustomDelay() ? "Custom Sync Time" : "Default Sync Time", CAMERA_TEXT_CENTER, 225);
	}

	DrawSignalStrength();
	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}


static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	return &bmleftarrow;
}

void VehicleSettingsIgnoreNextKeyRelease(void)
{
	clickedOnVSScreen = false;
}

int ValidateVehicleData(void)
{
	if ((vehicleData.tach.greenStart > vehicleData.tach.upperYellowStart) ||
		(vehicleData.tach.upperYellowStart > vehicleData.tach.upperRedStart) ||
		(vehicleData.tach.upperRedStart > vehicleData.tach.scaleEnd))
	{
		if (VehicleSettingsPopUp(INVALID_TACH_SETTINGS) == GUI_ID_OK)
			return false;
		else AutoFixTach();
	}

	// If the diffRatio is NOT zero (verified on an earlier screen) then gearing is enabled so
	// we need to check the ratios.  Otherwise gearing is disabled so we don't need to check.
	if (vehicleData.diffRatio != 0.0)
		if	((vehicleData.gearRatios[0] == 0.0) ||
			(vehicleData.gearRatios[1] != 0.0 && (vehicleData.gearRatios[0] < vehicleData.gearRatios[1])) ||
			(vehicleData.gearRatios[2] != 0.0 && (vehicleData.gearRatios[1] < vehicleData.gearRatios[2])) ||
			(vehicleData.gearRatios[3] != 0.0 && (vehicleData.gearRatios[2] < vehicleData.gearRatios[3])) ||
			(vehicleData.gearRatios[4] != 0.0 && (vehicleData.gearRatios[3] < vehicleData.gearRatios[4])) ||
			(vehicleData.gearRatios[5] != 0.0 && (vehicleData.gearRatios[4] < vehicleData.gearRatios[5])) ||
			(vehicleData.gearRatios[6] != 0.0 && (vehicleData.gearRatios[5] < vehicleData.gearRatios[6])) ||
			(vehicleData.gearRatios[7] != 0.0 && (vehicleData.gearRatios[6] < vehicleData.gearRatios[7])))
		{
			if (VehicleSettingsPopUp(INVALID_GEAR_RATIOS) == GUI_ID_OK)
				return false;
			else AutoFixGears();
		}

	if (vehicleData.weight == 0.0)
	{
		if (VehicleSettingsPopUp(INVALID_WEIGHT) == GUI_ID_OK)
			return false;
		else AutoFixWeight();
	}

	return true;
}

//
// Exiting camera selection screen.  If new camera selection warn user
// of any restrictions or power cycle requirements.  Give user the option 
// to abort the change if he wants.  If not, make changes.
//
// Return value: true - okay to exit screen
//               false - go back to this screen so user can make changes
//
int ValidateCamera(void)
{
	if (tempCameraIndex != -1)
		newSUCameraSelection = NewCameras[tempCameraIndex].enumeration;

	// If the camera did not change at all - exit with no change
	if ((newSUCameraSelection == originalSUCameraSelection) && (newTDCameraSelection == originalTDCameraSelection))
		return true;

	// Otherwise new camera selection - save selection tell SU about camera change
	vehicleData.camera = newSUCameraSelection | newTDCameraSelection;

	// Tell SU about change of camera
	notifyCameraChange();

	// If the SU camera selection has changed and is not none
	if ((newSUCameraSelection != CAMERA_NONE) && (newSUCameraSelection != originalSUCameraSelection))
	{
		// If a camera using a TraqData USB interface is connected we required SU version 3.70 or later
		if (newSUCameraSelection == CAMERA_MOBIUS || 
			newSUCameraSelection == CAMERA_REPLAY_MINI ||
			newSUCameraSelection == CAMERA_REPLAY_PRIMEX ||
			newSUCameraSelection == CAMERA_REPLAY_XD_USB)
		{
			if (!suData.suPresent || (suData.suSwRev < 370))
				if (VehicleSettingsPopUp(SU_REV370_WARNING) == GUI_ID_CANCEL)
				{
					UpdateCameraVariables();
					return false;
				}
		}
		else if (newSUCameraSelection == CAMERA_GOPRO_HD || 
			newSUCameraSelection == CAMERA_GOPRO_HD_PRO || 
			newSUCameraSelection == CAMERA_REPLAY_HD)
		{
			if (!suData.suPresent || (suData.suSwRev < 340))
				if (VehicleSettingsPopUp(GOPROHD_REV_WARNING) == GUI_ID_CANCEL)
				{
					UpdateCameraVariables();
					return false;
				}
		}

		// Camera being selected, be sure digital input 4 is disabled
		vehicleData.digitalInput[0].enabled = false;
		if (VehicleSettingsPopUp(CHANGED_CAMERA_WARNING) == GUI_ID_OK)
		{
			WriteUserDataEvent(VEHICLE_DATA);
#ifdef _WINDOWS
			SIM_Powerkey_CB(POWER_KEY, 0);
#else
			PowerOffUnit(hVehicleSettingsScreens, false, 1000);				// Pause for 1 second to allow time for the SU to process the new camera setting
#endif
			UpdateCameraVariables();
			return false;
		}
	}

	UpdateCameraVariables();
	return true;
}


int ValidateDiffRatio(void)
{
	if (vehicleData.diffRatio == 0.0)
	{
		if (VehicleSettingsPopUp(NO_GEAR_RATIOS) == GUI_ID_CANCEL)
			return false;
	}
	return true;
}

void AutoFixTach(void)
{
	int i[4];

	i[0] = vehicleData.tach.greenStart;
	i[1] = vehicleData.tach.upperYellowStart;
	i[2] = vehicleData.tach.upperRedStart;
	i[3] = vehicleData.tach.scaleEnd;

	// If any are 0s
	if ((i[0] == 0.0) || (i[1] == 0.0) || (i[2] == 0.0) || (i[3] == 0.0))
	{
		SetDefaultTach();
		return;
	}

	// Otherwise attempt to sort values in numeric order
	iSort(i, 4);

	// Save sorted values
	vehicleData.tach.greenStart = i[0];
	vehicleData.tach.upperYellowStart = i[1];
	vehicleData.tach.upperRedStart = i[2];
	vehicleData.tach.scaleEnd = i[3];
}

void AutoFixGears(void)
{
	int i;
	float f[8], temp;

	// Check diff ratio
	if (vehicleData.diffRatio == 0.0)
		vehicleData.diffRatio = DEFAULT_DIFF_RATIO;

	// Add up all the gears to see if they're ALL 0.0
	temp = 0.0;
	i = 0;
	while (i < 8)
		temp += vehicleData.gearRatios[i++];

	if (temp == 0.0)
	{
		SetDefaultGears();
		return;
	}

	// Otherwise sort the gear ratios in the correct order
	// Sort gears in REVERSE ORDER
	f[0] = vehicleData.gearRatios[7];
	f[1] = vehicleData.gearRatios[6];
	f[2] = vehicleData.gearRatios[5];
	f[3] = vehicleData.gearRatios[4];
	f[4] = vehicleData.gearRatios[3];
	f[5] = vehicleData.gearRatios[2];
	f[6] = vehicleData.gearRatios[1];
	f[7] = vehicleData.gearRatios[0];

	// sort in ascending order
	fSort(f, 8);

	// Save sorted values
	vehicleData.gearRatios[7] = f[0];
	vehicleData.gearRatios[6] = f[1];
	vehicleData.gearRatios[5] = f[2];
	vehicleData.gearRatios[4] = f[3];
	vehicleData.gearRatios[3] = f[4];
	vehicleData.gearRatios[2] = f[5];
	vehicleData.gearRatios[1] = f[6];
	vehicleData.gearRatios[0] = f[7];
}

void AutoFixWeight(void)
{
	vehicleData.weight = DEFAULT_VEHICLE_WEIGHT;
}

static const GUI_WIDGET_CREATE_INFO InvalidTachResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Invalid tachometer configuration.", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to go back or", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "have us auto-fix the data?", 0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Go Back",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Auto-Fix", GUI_ID_CANCEL, 195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO InvalidGearResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Invalid Gear Ratios.", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to go back or", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "have us auto-fix the data?", 0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Go Back",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Auto-Fix", GUI_ID_CANCEL, 195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO InvalidWeightResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Invalid vehicle weight.",      0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to go back or", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "have us auto-fix the data?",   0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Go Back",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Auto-Fix", GUI_ID_CANCEL, 195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO NoGearsResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "You entered a diff ratio of 0.00.",    0, 0, 20, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "This will disable gear calculations.", 0, 0, 60, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Confirm",	GUI_ID_OK,    195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO GoProWarningResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "GoPro HD and Replay XD cameras", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "require Sensor Unit firmware",    0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "version 3.40 or newer.",   0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "OK",	GUI_ID_OK, 195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO SURev370WarningResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Cameras connected to a TraqData", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "USB module require Sensor Unit",    0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "firmware version 3.70 or newer.",   0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "OK",	GUI_ID_OK, 195, 115, 120, 60 },
};

static const GUI_WIDGET_CREATE_INFO ChangedCameraResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "You have changed camera selection", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "which requires a power cycle.", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to power down now?",      0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK,   35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "NO", GUI_ID_CANCEL, 195, 115, 120, 60 }
};

static const GUI_WIDGET_CREATE_INFO ResetCameraResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Warning", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The camera you had selected", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "is no longer supported. Your", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "selection has been disabled.",      0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK,   145, 115, 100, 60 },
};


//
// Display a modal dialog box to see if the user wants to stop recording to exit this screen
//
// Returns	YES - proceed to exit record mode
//				NO - abort and stay
//
int VehicleSettingsPopUp(int screen)
{
	SetPopupDefaults(WARNING_POPUP);

	switch (systemSettingsPopupScreen = screen)
	{
		case INVALID_TACH_SETTINGS:
			return GUI_ExecDialogBox(InvalidTachResources, GUI_COUNTOF(InvalidTachResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case INVALID_GEAR_RATIOS:
			return GUI_ExecDialogBox(InvalidGearResources, GUI_COUNTOF(InvalidGearResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case INVALID_WEIGHT:
			return GUI_ExecDialogBox(InvalidWeightResources, GUI_COUNTOF(InvalidWeightResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case NO_GEAR_RATIOS:
			return GUI_ExecDialogBox(NoGearsResources, GUI_COUNTOF(NoGearsResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case GOPROHD_REV_WARNING:
			return GUI_ExecDialogBox(GoProWarningResources, GUI_COUNTOF(GoProWarningResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case CHANGED_CAMERA_WARNING:
			return GUI_ExecDialogBox(ChangedCameraResources, GUI_COUNTOF(ChangedCameraResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case RESET_CAMERA_WARNING:
			return GUI_ExecDialogBox(ResetCameraResources, GUI_COUNTOF(ResetCameraResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case SU_REV370_WARNING:
			return GUI_ExecDialogBox(SURev370WarningResources, GUI_COUNTOF(SURev370WarningResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
	}
	
	return false;
}

void SetAnalogColorAndText(BUTTON_Handle hBtn, int index)
{
	char scratch[20];

	if (vehicleData.analogInput[index].enabled == false)
		strcpy(scratch, "Disabled");
	else sprintf(scratch, "%0.1f\n%s", GetAnalogInput(USER, index), vehicleData.analogInput[index].Units);
	BUTTON_SetText(hBtn, scratch);

	if (!vehicleData.analogInput[index].enabled ||
		((BELOW != vehicleData.analogInput[index].lowerAlarmTriggerComparison) && (ABOVE != vehicleData.analogInput[index].upperAlarmTriggerComparison))) {
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBrownInputButton);
	} // if
	else if(InAlarm(index))
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedInputButton);
	else
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenInputButton);
}

void SetDigitalColorAndText(BUTTON_Handle hBtn, int index)
{
	if (!vehicleData.digitalInput[index].enabled) {
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBrownInputButton);
	} // if
	else if(InAlarm(index+4))
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedInputButton);
	else
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenInputButton);

	if (vehicleData.digitalInput[index].enabled == false)
		BUTTON_SetText(hBtn, "Disabled");
	else BUTTON_SetText(hBtn, GetDigitalText(USER, index));
}

//
// Camera selection screen routines
//
GUI_HWIN GetCameraSelectionWindowHandle(void)
{
	return hCameraSelection;
}

void CreateCameraSelection(WM_HWIN hParent)
{
	HEADER_Handle hHeader;
	int i, selectionIndex;

	if (hParent)
		WM_DeleteWindow(hParent);

	SetScreenType(CAMERA_SELECTION_SCREEN_TYPE);

	if ((hCameraSelection = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, CameraSelectionCallback, 0)) <= 0)
		SystemError();

	hCameraList = LISTVIEW_CreateEx(30, 50, 430, 205, hCameraSelection, WM_CF_SHOW, 0, CAMERA_LIST);
	WM_SetFocus(hCameraList);
	LISTVIEW_AddColumn(hCameraList, 200, "Camera", GUI_TA_HCENTER | GUI_TA_VCENTER);
	LISTVIEW_AddColumn(hCameraList, 200, "Interface", GUI_TA_HCENTER | GUI_TA_VCENTER);
	LISTVIEW_SetAutoScrollV(hCameraList, 1);
	LISTVIEW_SetFont(hCameraList, &GUI_Font20_ASCII);
	LISTVIEW_SetRowHeight(hCameraList, FINGER_WIDTH - 5);

	HEADER_SetFont(hHeader = LISTVIEW_GetHeader(hCameraList), &GUI_Font20_ASCII);
	HEADER_SetTextAlign(hHeader, 0, GUI_TA_HCENTER | GUI_TA_VCENTER);
	HEADER_SetTextAlign(hHeader, 1, GUI_TA_HCENTER | GUI_TA_VCENTER);
	HEADER_SetHeight(hHeader, 25);

	i = 0;
	selectionIndex = -1;
	while (NewCameras[i].pName)
	{
		const char *pDest[2];

		pDest[0] = NewCameras[i].pName;
		pDest[1] = NewCameras[i].pInterface;

		// See if this is the selected camera
		if (originalSUCameraSelection == NewCameras[i].enumeration)
			selectionIndex = i;

		LISTVIEW_AddRow(hCameraList, pDest);
		i++;
	}
	if (selectionIndex == -1)
	{
		// Tell user the selection changed!
		selectionIndex = 0;
	}
	// Highlight selection
	tempCameraIndex = selectionIndex;
	LISTVIEW_SetSelUnsorted(hCameraList, selectionIndex);

	// Scrollbar width must be set after the data is in the control
	SCROLLBAR_SetWidth(WM_GetScrollbarV(hCameraList), FINGER_WIDTH);
	LISTVIEW_SetBkColor(hCameraList, LISTVIEW_CI_SELFOCUS, GUI_LISTBLUE);

	LISTVIEW_SetCompareFunc(hCameraList, 0, &LISTVIEW_CompareText);
	LISTVIEW_SetCompareFunc(hCameraList, 1, &LISTVIEW_CompareText);
	LISTVIEW_EnableSort(hCameraList);
	LISTVIEW_SetSort(hCameraList, 0, 0);			// Set sort to sort by decending camera name

	// Set "ignore region" of list area to prevent interference with BACK button
	LISTVIEW_SetLFreeBorder(hCameraList, 35);

	hScreenTimer = WM_CreateTimer(hCameraSelection, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}


void CameraSelectionCallback(WM_MESSAGE * pMsg)
{
	int Id, NCode;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateWindow(pMsg->hWin);		// Allow timer to invalidate the whole window so we can show live data on the input buttons
			}
			break;

		case WM_NOTIFY_PARENT:
			NCode = pMsg->Data.v;                 // Notification code
			if (NCode == WM_NOTIFICATION_CLICKED)
			{
				Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
				if (Id == CAMERA_LIST)
				{
					tempCameraIndex = LISTVIEW_GetSelUnsorted(hCameraList);			// Get current sorted selection
					// Only set temp variable rather than actually set the camera here because the notification
					// comes about 5 times (literally) for every press.  Formerly it would call the hardware
					// notification routine for each pass through.  This seems better.  Note the restriction is
					// that you must exit the screen for the change to take affect rather than just press the button
					// and power cycle.
				}
			}
			break;

		case WM_PAINT:
			GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
			GUI_DrawBitmap(&bmCameraText, 60, 0);
			GUI_DrawBitmap(&bmleftarrow, 0, 229);
			DrawSignalStrength();
			break;

		default:
		  WM_DefaultProc(pMsg);
	}
}

//
// Camera delay screen routines
//
GUI_HWIN GetCameraDelayWindowHandle(void)
{
	return hCameraDelay;
}

LISTWHEEL_Handle hOnesWheel, hTenthsWheel;
char *Digits[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	NULL
};

void CreateCameraDelay(WM_HWIN hParent)
{
	if (hParent)
		WM_DeleteWindow(hParent);

	SetScreenType(CAMERA_DELAY_SCREEN_TYPE);

	if ((hCameraDelay = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, CameraDelayCallback, 0)) <= 0)
		SystemError();

	ones = (vehicleData.usbCameraDelay & 0x7F) / 10;
	tenths = (vehicleData.usbCameraDelay & 0x7F) % 10;

	// Display first (left) wheel
	hOnesWheel = LISTWHEEL_CreateEx(TRIPLE_SPINNER_START_1, 50, 69, 208, hCameraDelay, WM_CF_SHOW | WM_CF_HASTRANS, 0, GUI_ID_LISTWHEEL1, Digits);
	LISTWHEEL_SetLineHeight(hOnesWheel, 41);
	LISTWHEEL_SetSnapPosition(hOnesWheel, 87);
	LISTWHEEL_SetFont(hOnesWheel, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(hOnesWheel, GUI_TA_CENTER);
	if (ones < 0)		// Safety check!
		ones = 0;
	else if (ones > 9)
		ones = 9;
	LISTWHEEL_SetPos(hOnesWheel, ones);
	LISTWHEEL_SetSel(hOnesWheel, ones);

	// Display second (middle) wheel
	hTenthsWheel = LISTWHEEL_CreateEx(TRIPLE_SPINNER_START_2, 50, 69, 208, hCameraDelay, WM_CF_SHOW | WM_CF_HASTRANS, 0, GUI_ID_LISTWHEEL2, Digits);
	LISTWHEEL_SetLineHeight(hTenthsWheel, 41);
	LISTWHEEL_SetSnapPosition(hTenthsWheel, 87);
	LISTWHEEL_SetFont(hTenthsWheel, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(hTenthsWheel, GUI_TA_CENTER);
	if (tenths < 0)		// Safety check!
		tenths = 0;
	else if (tenths > 9)
		tenths = 9;
	LISTWHEEL_SetPos(hTenthsWheel, tenths);
	LISTWHEEL_SetSel(hTenthsWheel, tenths);

	hScreenTimer = WM_CreateTimer(hCameraDelay, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

void SetCameraDelay(void)
{
	vehicleData.usbCameraDelay = (vehicleData.usbCameraDelay & 0x80) + ((ones * 10) + tenths);
}

void CameraDelayCallback(WM_MESSAGE * pMsg)
{
	int Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateWindow(pMsg->hWin);		// Allow timer to invalidate the whole window so we can show live data on the input buttons
			}
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			if (pMsg->Data.v == WM_NOTIFICATION_SEL_CHANGED)
				if (abs(LISTWHEEL_GetVelocity(pMsg->hWinSrc)) <= 1)
				{
					if (Id == GUI_ID_LISTWHEEL1)
						LISTWHEEL_SetSel(pMsg->hWinSrc, ones = LISTWHEEL_GetPos(pMsg->hWinSrc));
					else if (Id == GUI_ID_LISTWHEEL2)
						LISTWHEEL_SetSel(pMsg->hWinSrc, tenths = LISTWHEEL_GetPos(pMsg->hWinSrc));
				}
			break;

		case WM_PAINT:
			GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
			GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_1, 50);
			GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_2, 50);
			GUI_DrawBitmap(&bmCameraText, 60, 0);
			GUI_DrawBitmap(&bmleftarrow, 0, 229);

			GUI_SetTextMode(GUI_TM_TRANS);
			GUI_SetFont(&GUI_Font24_ASCII);
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringHCenterAt("Sync Time", 120, 135);
			GUI_DispStringAt("seconds", 380, 135);
			GUI_SetFont(&GUI_FontD32);
			GUI_DispStringHCenterAt(".", TRIPLE_SPINNER_START_2 - 5, 120);
			DrawSignalStrength();
			break;

		default:
		  WM_DefaultProc(pMsg);
	}
}

//
// Change of camera selection - update working variables
//
void UpdateCameraVariables(void)
{
	originalSUCameraSelection = newSUCameraSelection;
	originalTDCameraSelection = newTDCameraSelection;
}

//
// Return working values of camera selections.  These routines return the
// working values of the caemra selection which are always valid - even
// while on camera settings screens.
//
unsigned char GetSUCam(void)
{
	return newSUCameraSelection;
}

unsigned char GetTDCam(void)
{
	return newTDCameraSelection;
}


/*************************** End of file ****************************/

