
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "DIALOG.h"
#include "GeneralConfig.h"
#include "ConfigScreens.h"
#include "DataHandler.h"
#include "StandardList.h"
#include "HelpScreens.h"
#include "HelpTachSetup_1.c"
#include "HelpTachSetup_2.c"
#include "HelpTachSetup_3.c"
#include "help1.c"
#include "help2.c"
#include "help3.c"
#include "help4.c"
#include "Home1.c"
#include "Home2.c"
#include "Home3.c"
#include "Home4.c"
#include "Home5.c"
#include "HelpCarSelect3.c"
#include "HelpCarSelect4.c"
#include "HChooseVehicleSettings.c"
#include "HSelectVehicle.c"
#include "HCameraImportant.c"
#include "HSelectSUCam1.c"
#include "HSelectSUCam2.c"
#include "HSelectCamera.c"
#include "HSelectRaceSetup.c"
#include "HelpAutoOn.c"
#include "HelpSelectSystemType.c"
#include "HelpDynoTesting.c"
#include "HelpNormalGPS.c"
#include "HelpSelectGoRacing.c"
#include "HelpSelectARaceType.c"
#include "HelpConfirmTrack.c"
#include "HelpRaceScreens2.c"
#include "HelpRaceScreens1.c"
#include "HelpSelectTrack.c"
#include "HelpTrackType.c"
#include "HelpSelectRaceSetup.c"
#include "HelpSelectLapQualify.c"
#include "HelpLapsQualify.c"
#include "HSelectInputs.c"
#include "HCommonSensorSettings.c"
#include "HAnalogSensors.c"
#include "HelpLapTimeDisplay.c"
#include "HelpSelectRaceReview.c"
#include "HelpBestLapForSession.c"
#include "HelpSessionDetails.c"
#include "HelpBestLap.c"
#include "HelpTogglesMoreDetails.c"
#include "HelpAnalog7.c"
#include "HelpAnalog8.c"
#include "HelpAnalog9.c"
#include "HelpAnalog10.c"
#include "HelpAnalog11.c"
#include "HelpAnalog12.c"
#include "HelpAnalog13.c"
#include "HelpAnalog14.c"
#include "HSelectTDCam1.c"
#include "HSelectTDCam2.c"
#include "HelpFineTuneYourCar.c"
#include "AutoOn3.c"
#include "AutoOn4.c"
#include "AutoOn5.c"
#include "CamTestLast.c"

//
// Public Data
//
int suspendForHelp = FALSE;
HELP_SCREENS *pHelpScreens;

//
// Private Routines
//
void HelpScreenCallback(WM_MESSAGE * pMsg);
void HelpSelectionCallback(WM_MESSAGE * pMsg);
//void InitializeScreenBuffers(enum HELP_SCREENS prev, enum HELP_SCREENS next);
void UpdateButtonGraphics(void);

//
// Private Data
//
GUI_HWIN hDetailHelp;
GUI_HWIN hHelpSelection = 0;
LISTVIEW_Handle hHelpList;
BUTTON_Handle hHelpBtn;
WM_HWIN hParent;
int oldScreen;
enum SCREEN_TYPE oldScreenType;
HELP_SCREENS *pScreens;
int screenIndex;
BUTTON_Handle hPreviousButton;
BUTTON_Handle hNextButton;
WM_HWIN hOwner;

//
// External References
//
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelpBackButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelpBackDisabled;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelpExitButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelpDisabled;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelpPreviousButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelpNextButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern WM_HWIN GetCurrentWindowHandle(void);

//
// Local Defines
//
#define	HELP_BUTTON_WIDTH	100
#define	HELP_BUTTON_HEIGHT	30
#define	HELP_BUTTON_OFFSET	30

#define	NEXT_BUTTON			GUI_ID_USER + 1
#define	LAST_BUTTON			GUI_ID_USER + 2
#define	EXIT_BUTTON			GUI_ID_USER + 3
#define HELP_LIST			GUI_ID_USER + 4
#define	SHOW_HELP_BUTTON	GUI_ID_USER + 5

//
// Note: HELP_TOPIC and HelpList must match one to one
//
enum HELP_TOPIC {
	MAIN_HELP,
	GO_RACING_HELP,
	SU_CAMERA_HELP,
	DU_CAMERA_HELP,
	TACH_HELP,
	INPUTS_HELP,
	VEHICLE_SETUP_HELP,
	AUTO_ON_HELP,
	DYNO_HELP,
	TRACK_SETTINGS_HELP,
	REVIEW_SESSIONS_HELP,
	LAP_MODE_HELP,
	MAX_HELP_TOPICS
};

const char *HelpList[MAX_HELP_TOPICS] = {
	"Main Menu",
	"Go Racing",
	"SU Camera Setup",
	"DU Camera Setup",
	"Tach Setup",
	"Inputs Setup",
	"Vehicle Setup",
	"Auto-On",
	"Dyno Testing",
	"Track Settings",
	"Race Review (Laps)",
	"Lap Timing Setup",
};

typedef struct { enum SCREEN_TYPE screenType; enum HELP_TOPIC topic; } HELP_TABLE;




HELP_SCREENS MainHelpScreens[] = {
	{ acHome1, sizeof(acHome1) },
	{ acHome2, sizeof(acHome2) },
	{ acHome3, sizeof(acHome3) },
	{ acHome4, sizeof(acHome4) },
	{ acHome5, sizeof(acHome5) },
	{ 0, 0 }
};

HELP_SCREENS GoRacingHelpScreens[] = {
	{ acHelpSelectGoRacing, sizeof(acHelpSelectGoRacing) },
	{ acHelpSelectARaceType, sizeof(acHelpSelectARaceType) },
	{ acHelpConfirmTrack, sizeof(acHelpConfirmTrack) },
	{ acHelpRaceScreens1, sizeof(acHelpRaceScreens1) },
	{ acHelpRaceScreens2, sizeof(acHelpRaceScreens2) },
	{ 0, 0 }
};

HELP_SCREENS SUCameraHelpScreens[] = {
	{ acHSelectRaceSetup, sizeof(acHSelectRaceSetup) },
	{ acHSelectVehicle, sizeof(acHSelectVehicle) },
	{ acHChooseVehicleSettings, sizeof(acHChooseVehicleSettings) },
	{ acHSelectCamera, sizeof(acHSelectCamera) },
	{ acHCameraImportant, sizeof(acHCameraImportant) },
	{ acHSelectSUCam1, sizeof(acHSelectSUCam1) },
	{ acHSelectSUCam2, sizeof(acHSelectSUCam2) },
	{ acCamTestLast, sizeof(acCamTestLast) },
	{ 0, 0 }
};

HELP_SCREENS SUSpecificCameraHelpScreens[] = {
	{ acHCameraImportant, sizeof(acHCameraImportant) },
	{ acHSelectSUCam2, sizeof(acHSelectSUCam2) },
	{ acCamTestLast, sizeof(acCamTestLast) },
	{ 0, 0 }
};

HELP_SCREENS DUCameraHelpScreens[] = {
	{ acHSelectRaceSetup, sizeof(acHSelectRaceSetup) },
	{ acHSelectVehicle, sizeof(acHSelectVehicle) },
	{ acHChooseVehicleSettings, sizeof(acHChooseVehicleSettings) },
	{ acHSelectCamera, sizeof(acHSelectCamera) },
	{ acHCameraImportant, sizeof(acHCameraImportant) },
	{ acHSelectTDCam1, sizeof(acHSelectTDCam1) },
	{ acHSelectTDCam2, sizeof(acHSelectTDCam2) },
	{ acCamTestLast, sizeof(acCamTestLast) },
	{ 0, 0 }
};

HELP_SCREENS DUSpecificCameraHelpScreens[] = {
	{ acHSelectTDCam2, sizeof(acHSelectTDCam2) },
	{ acCamTestLast, sizeof(acCamTestLast) },
	{ 0, 0 }
};

HELP_SCREENS GenericCameraHelpScreens[] = {
	{ acHCameraImportant, sizeof(acHCameraImportant) },
	{ acHSelectSUCam1, sizeof(acHSelectSUCam1) },
	{ acHSelectSUCam2, sizeof(acHSelectSUCam2) },
	{ acHSelectTDCam1, sizeof(acHSelectTDCam1) },
	{ acHSelectTDCam2, sizeof(acHSelectTDCam2) },
	{ acCamTestLast, sizeof(acCamTestLast) },
	{ 0, 0 }
};

HELP_SCREENS VehicleSetupHelpScreens[] = {
	{ acHSelectRaceSetup, sizeof(acHSelectRaceSetup) },
	{ acHSelectVehicle, sizeof(acHSelectVehicle) },
	{ acHelpCarSelect4, sizeof(acHelpCarSelect4) },
	{ acHelpCarSelect3, sizeof(acHelpCarSelect3) },
	{ acHChooseVehicleSettings, sizeof(acHChooseVehicleSettings) },
	{ acHelpFineTuneYourCar, sizeof(acHelpFineTuneYourCar) },
	{ 0, 0 }
};

HELP_SCREENS VehicleSelectionHelpScreens[] = {
	{ acHelpCarSelect4, sizeof(acHelpCarSelect4) },
	{ acHelpCarSelect3, sizeof(acHelpCarSelect3) },
	{ acHChooseVehicleSettings, sizeof(acHChooseVehicleSettings) },
	{ acHelpFineTuneYourCar, sizeof(acHelpFineTuneYourCar) },
	{ 0, 0 }
};

HELP_SCREENS VehicleSetupSpecificHelpScreens[] = {
	{ acHelpFineTuneYourCar, sizeof(acHelpFineTuneYourCar) },
	{ 0, 0 }
};

HELP_SCREENS InputsHelpScreens[] = {
	{ acHelpSelectRaceSetup, sizeof(acHelpSelectRaceSetup) },
	{ acHSelectVehicle, sizeof(acHSelectVehicle) },
	{ acHChooseVehicleSettings, sizeof(acHChooseVehicleSettings) },
	{ acHSelectInputs, sizeof(acHSelectInputs) },
	{ acHCommonSensorSettings, sizeof(acHCommonSensorSettings) },
	{ acHAnalogSensors, sizeof(acHAnalogSensors) },
	{ acHelpAnalog7, sizeof(acHelpAnalog7) },
	{ acHelpAnalog8, sizeof(acHelpAnalog8) },
	{ acHelpAnalog9, sizeof(acHelpAnalog9) },
	{ acHelpAnalog10, sizeof(acHelpAnalog10) },
	{ acHelpAnalog11, sizeof(acHelpAnalog11) },
	{ acHelpAnalog12, sizeof(acHelpAnalog12) },
	{ acHelpAnalog13, sizeof(acHelpAnalog13) },
	{ acHelpAnalog14, sizeof(acHelpAnalog14) },
	{ 0, 0 }
};

HELP_SCREENS AnalogInputsHelpScreens[] = {
	{ acHelpAnalog7, sizeof(acHelpAnalog7) },
	{ acHelpAnalog8, sizeof(acHelpAnalog8) },
	{ acHelpAnalog9, sizeof(acHelpAnalog9) },
	{ acHelpAnalog10, sizeof(acHelpAnalog10) },
	{ acHelpAnalog11, sizeof(acHelpAnalog11) },
	{ acHelpAnalog12, sizeof(acHelpAnalog12) },
	{ acHelpAnalog13, sizeof(acHelpAnalog13) },
	{ acHelpAnalog14, sizeof(acHelpAnalog14) },
	{ 0, 0 }
};

HELP_SCREENS EditScaleHelpScreens[] = {
	{ acHelpAnalog7, sizeof(acHelpAnalog7) },
	{ acHelpAnalog8, sizeof(acHelpAnalog8) },
	{ acHelpAnalog9, sizeof(acHelpAnalog9) },
	{ acHelpAnalog10, sizeof(acHelpAnalog10) },
	{ acHelpAnalog11, sizeof(acHelpAnalog11) },
	{ acHelpAnalog12, sizeof(acHelpAnalog12) },
	{ acHelpAnalog13, sizeof(acHelpAnalog13) },
	{ acHelpAnalog14, sizeof(acHelpAnalog14) },
	{ 0, 0 }
};

HELP_SCREENS ReviewSessionsHelpScreens[] = {
	{ acHelpSelectRaceReview, sizeof(acHelpSelectRaceReview) },
	{ acHelpBestLapForSession, sizeof(acHelpBestLapForSession) },
	{ acHelpSessionDetails, sizeof(acHelpSessionDetails) },
	{ acHelpBestLap, sizeof(acHelpBestLap) },
	{ acHelpTogglesMoreDetails, sizeof(acHelpTogglesMoreDetails) },
	{ 0, 0 }
};

HELP_SCREENS TachHelpScreens[] = {
	{ acHelpSelectRaceSetup, sizeof(acHelpSelectRaceSetup) },
	{ acHSelectVehicle, sizeof(acHSelectVehicle) },
	{ acHChooseVehicleSettings, sizeof(acHChooseVehicleSettings) },
	{ achelp4, sizeof(achelp4) },
	{ acHelpTachSetup_3, sizeof(acHelpTachSetup_3) },
	{ acHelpTachSetup_2, sizeof(acHelpTachSetup_2) },
	{ acHelpTachSetup_1, sizeof(acHelpTachSetup_1) },
	{ 0, 0 }
};

HELP_SCREENS TachSpecificHelpScreens[] = {
	{ acHelpTachSetup_3, sizeof(acHelpTachSetup_3) },
	{ acHelpTachSetup_2, sizeof(acHelpTachSetup_2) },
	{ acHelpTachSetup_1, sizeof(acHelpTachSetup_1) },
	{ 0, 0 }
};

HELP_SCREENS TrackSettingsHelpScreens[] = {
	{ acHelpSelectRaceSetup, sizeof(acHelpSelectRaceSetup) },
	{ acHelpSelectTrack, sizeof(acHelpSelectTrack) },
	{ acHelpTrackType, sizeof(acHelpTrackType) },
	{ acHelpLapTimeDisplay, sizeof(acHelpLapTimeDisplay) },
	{ 0, 0 }
};

HELP_SCREENS TrackSettingsSpecificHelpScreens[] = {
	{ acHelpTrackType, sizeof(acHelpTrackType) },
	{ acHelpLapTimeDisplay, sizeof(acHelpLapTimeDisplay) },
	{ 0, 0 }
};

HELP_SCREENS AutoOnHelpScreens[] = {
	{ acHelpSelectSystemType, sizeof(acHelpSelectSystemType) },
	{ acHelpAutoOn, sizeof(acHelpAutoOn) },
	{ acAutoOn3, sizeof(acAutoOn3) },
	{ acAutoOn4, sizeof(acAutoOn4) },
	{ acAutoOn5, sizeof(acAutoOn5) },
	{ 0, 0 }
};

HELP_SCREENS AutoOnSpecificHelpScreens[] = {
	{ acAutoOn3, sizeof(acAutoOn3) },
	{ acAutoOn4, sizeof(acAutoOn4) },
	{ acAutoOn5, sizeof(acAutoOn5) },
	{ 0, 0 }
};

HELP_SCREENS DynoHelpScreens[] = {
	{ acHelpSelectSystemType, sizeof(acHelpSelectSystemType) },
	{ acHelpDynoTesting, sizeof(acHelpDynoTesting) },
	{ acHelpNormalGPS, sizeof(acHelpNormalGPS) },
	{ 0, 0 }
};

HELP_SCREENS TimeZoneGPSScreens[] = {
	{ acHelpNormalGPS, sizeof(acHelpNormalGPS) },
	{ 0, 0 }
};


HELP_SCREENS LapHelpScreens[] = {
	{ acHSelectRaceSetup, sizeof(acHSelectRaceSetup) },
	{ acHelpSelectLapQualify, sizeof(acHelpSelectLapQualify) },
	{ acHelpLapsQualify, sizeof(acHelpLapsQualify) },
	{ 0, 0 }
};

HELP_TABLE HelpTable[] = {
	{ GENERAL_CONFIG_SCREEN_TYPE,			MAIN_HELP },
	{ CONFIG_SCREEN_TYPE,					MAIN_HELP },
	{ GAUGES_SCREEN_TYPE,					MAIN_HELP },
	{ DRIVE_MODE_SCREEN_TYPE,				MAIN_HELP },
	{ INSTRUMENTS_MODE_SCREEN_TYPE,			MAIN_HELP },
	{ RACE_SETUP_SCREENS_TYPE,				MAIN_HELP },
	{ SYSTEM_SETTINGS_SCREENS_TYPE,			MAIN_HELP },
	{ VEHICLE_SETTINGS_SCREENS_TYPE,		MAIN_HELP },
	{ VEHICLE_SETTING_SCREEN_TYPE,			MAIN_HELP },
	{ TACH_SETUP_SCREENS_TYPE,				TACH_HELP },
	{ INPUTS_SCREENS_TYPE,					INPUTS_HELP },
	{ REVIEW_SESSIONS_SCREENS_TYPE,			REVIEW_SESSIONS_HELP },
	{ TRACK_SETTING_SCREEN_TYPE,			TRACK_SETTINGS_HELP },
	{ TRACK_SETTINGS_SCREENS_TYPE,			TRACK_SETTINGS_HELP },
	{ GEARING_SCREENS_TYPE,					MAIN_HELP },
	{ DIGITAL_SETTINGS_SCREEN_TYPE,			INPUTS_HELP },
	{ GEAR_RATIOS,							MAIN_HELP },
	{ RACE_SETUP_STANDALONE,				MAIN_HELP },
	{ SFSCREENS_TYPE,						MAIN_HELP },
	{ TIRE_DIMENSIONS_SCREEN_TYPE,			MAIN_HELP },
	{ WAITING_FOR_SATELLITES_SCREEN_TYPE,	MAIN_HELP },
	{ DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE,	TRACK_SETTINGS_HELP },
	{ WAITING_FOR_CAMERA_SCREEN_TYPE,		MAIN_HELP },
	{ WAITING_FOR_SF_SCREEN_TYPE,			MAIN_HELP },
	{ PERSONALIZE_REVIEW_SCREEN_TYPE,		MAIN_HELP },
	{ PERSONALIZE_ENTRY_SCREEN_TYPE,		MAIN_HELP },
	{ WAITING_FOR_STAGE_SCREEN_TYPE,		MAIN_HELP },
	{ WAITING_FOR_LAUNCH_SCREEN_TYPE,		MAIN_HELP },
	{ DRAG_AUTOX_GAUGE_SCREEN_TYPE,			MAIN_HELP },
	{ AERO_GRIP_SCREEN_TYPE,				MAIN_HELP },
	{ CAMERA_SELECTION_SCREEN_TYPE,			DU_CAMERA_HELP },
	{ CAMERA_DELAY_SCREEN_TYPE,				SU_CAMERA_HELP },
	{ HELP_SCREEN_TYPE,						MAIN_HELP },
	{ (enum SCREEN_TYPE)-1, (enum HELP_TOPIC)0 }
};

//  Help topic to help screens
typedef struct { enum HELP_TOPIC topic; HELP_SCREENS *pTopicScreens; } HELP_TOPIC_TO_SCREENS;

HELP_TOPIC_TO_SCREENS HelpTopicToScreens[] = {
	{ MAIN_HELP,				MainHelpScreens },
	{ GO_RACING_HELP,			GoRacingHelpScreens },
	{ SU_CAMERA_HELP,			SUCameraHelpScreens },
	{ DU_CAMERA_HELP,			DUCameraHelpScreens },
	{ TACH_HELP,				TachHelpScreens },
	{ INPUTS_HELP,				InputsHelpScreens },
	{ VEHICLE_SETUP_HELP,		VehicleSetupHelpScreens },
	{ AUTO_ON_HELP,				AutoOnHelpScreens },
	{ DYNO_HELP,				DynoHelpScreens },
	{ TRACK_SETTINGS_HELP,		TrackSettingsHelpScreens },
	{ REVIEW_SESSIONS_HELP,		ReviewSessionsHelpScreens },
	{ LAP_MODE_HELP,			LapHelpScreens },
	{ (enum HELP_TOPIC)-1, 0 }
};

enum HELP_TOPIC helpTopic;
void DisplayHelpScreenTutorial(void);

//
// CreateHelpSelection - display list of help topics OR display tutorial
//
// Input: caller - handle of calling window - will return focus to calling
//					window when tutorial screen closes
//		pHelpScreens 
//		if pHelpScreens = pointer to help screen list
//			go directly to that tutorial list
//		else if pHelpScreens = 0
//			no specific topic has been selected - display list of tutorial topics
//		else if pHelpScreens = -1
//			there is no tutorial supported at this level - should not be here - exit
//
void CreateHelpSelection(WM_HWIN caller)
{
	enum SCREEN_TYPE tempType;
	HEADER_Handle hHeader;
	int i;

	hOwner = caller;

	if (pHelpScreens == (HELP_SCREENS *)-1)
		return;

	suspendForHelp = TRUE;

	if (pHelpScreens != (HELP_SCREENS *)0)
	{
		hHelpSelection = 0;
		DisplayHelpScreenTutorial();
		return;
	}
	//
	// Get info on current screen
	//
	hParent = GetCurrentWindowHandle();
	SetLastScreen(oldScreenType = GetScreenType(), oldScreen = GetCurrentScreen());
	SetScreenType(HELP_SCREEN_TYPE);

	//
	// Convert screen type into help topic
	//
	i = 0;
	helpTopic = MAIN_HELP;
	while ((tempType = HelpTable[i].screenType) != (enum SCREEN_TYPE)-1)
	{
		if (tempType == oldScreenType)
		{
			helpTopic = HelpTable[i].topic;
			break;
		}
		else i++;
	}

	//
	// Create help topics screen and list
	//
	if ((hHelpSelection = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, HelpSelectionCallback, 0)) <= 0)
		SystemError();

	hHelpList = LISTVIEW_CreateEx(30, 50, 215, 205, hHelpSelection, WM_CF_SHOW, 0, HELP_LIST);
	WM_SetFocus(hHelpList);
	LISTVIEW_AddColumn(hHelpList, 215 - FINGER_WIDTH, "Tutorial Topic", GUI_TA_HCENTER | GUI_TA_VCENTER);
	LISTVIEW_SetAutoScrollV(hHelpList, 1);
	LISTVIEW_SetFont(hHelpList, &GUI_Font20_ASCII);
	LISTVIEW_SetRowHeight(hHelpList, FINGER_WIDTH - 5);

	HEADER_SetFont(hHeader = LISTVIEW_GetHeader(hHelpList), &GUI_Font20_ASCII);
	HEADER_SetTextAlign(hHeader, 0, GUI_TA_HCENTER | GUI_TA_VCENTER);
	HEADER_SetHeight(hHeader, 25);

	i = 0;
	while (i < MAX_HELP_TOPICS)
	{
		LISTVIEW_AddRow(hHelpList, &HelpList[i]);
		i++;
	}

	//
	// Highlight selection (current help topic/screen type)
	//
	LISTVIEW_SetSelUnsorted(hHelpList, helpTopic);

	// Scrollbar width must be set after the data is in the control
	SCROLLBAR_SetWidth(WM_GetScrollbarV(hHelpList), FINGER_WIDTH);
	LISTVIEW_SetBkColor(hHelpList, LISTVIEW_CI_SELFOCUS, GUI_LISTBLUE);

	// Set "ignore region" of list area to prevent interference with BACK button
	LISTVIEW_SetLFreeBorder(hHelpList, 35);

	// Display help button
	hHelpBtn = BUTTON_CreateEx(BUTTON_LEFT, 134, BUTTON_WIDTH, BUTTON_HEIGHT, hHelpSelection, WM_CF_SHOW, 0, SHOW_HELP_BUTTON);
	BUTTON_SetFont(hHelpBtn, &GUI_Font24_ASCII);
	BUTTON_SetBitmap(hHelpBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetTextColor(hHelpBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hHelpBtn, BUTTON_BI_DISABLED, GUI_BLACK);
	BUTTON_SetText(hHelpBtn, "Show Tutorial");

	// Display exit button
	hHelpBtn = BUTTON_CreateEx(BUTTON_LEFT, 220, BUTTON_WIDTH, BUTTON_HEIGHT, hHelpSelection, WM_CF_SHOW, 0, EXIT_BUTTON);
	BUTTON_SetFont(hHelpBtn, &GUI_Font24_ASCII);
	BUTTON_SetBitmap(hHelpBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetTextColor(hHelpBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hHelpBtn, BUTTON_BI_DISABLED, GUI_BLACK);
	BUTTON_SetText(hHelpBtn, "Exit");

	hScreenTimer = WM_CreateTimer(hHelpSelection, 0, SETUP_SCREEN_REFRESH_TIME, 0);
}

void DeleteHelpScreen(void)
{
	if (hHelpSelection)
	{
		WM_DeleteWindow(hHelpSelection);
		hHelpSelection = 0;
		SetScreenType(oldScreenType);

		WM_SetFocus(hOwner);		// Return focus to calling window
	}

	suspendForHelp = FALSE;
}


void HelpSelectionCallback(WM_MESSAGE * pMsg)
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
				if (Id == SHOW_HELP_BUTTON)
				{
					int i = 0;
					enum HELP_TOPIC topic;

					helpTopic = (enum HELP_TOPIC)LISTVIEW_GetSel(hHelpList);			// Get selected help topic selection
					DeleteHelpScreen();

					while ((topic = HelpTopicToScreens[i].topic) != (enum HELP_TOPIC)-1)
					{
						if (topic == helpTopic)
						{
							pHelpScreens = HelpTopicToScreens[i].pTopicScreens;
							break;
						}
						else i++;
					}
					DisplayHelpScreenTutorial();
				}
				else if (Id == EXIT_BUTTON)
				{
					DeleteHelpScreen();
				}
			}
			break;

		case WM_PAINT:
			GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);

			GUI_SetTextMode(GUI_TEXTMODE_TRANS);
			GUI_SetFont(&GUI_Font24_ASCII);
			GUI_DispStringHCenterAt("Select topic", 355, 65);
			GUI_DispStringHCenterAt("for tutorial", 355, 90);

	//		GUI_DrawBitmap(&bmCameraText, 60, 0);		// draw title banner
	//		GUI_DrawBitmap(&bmleftarrow, 0, 229);
			DrawSignalStrength();
			break;

		default:
		  WM_DefaultProc(pMsg);
	}
}


void DisplayHelpScreenTutorial(void)
{
	BUTTON_Handle hButton;

	suspendForHelp = TRUE;
	screenIndex = 0;

	// If no specific help screen topic is selected then display the main help list
	if (pHelpScreens == 0)
		pScreens = MainHelpScreens;
	else pScreens = pHelpScreens;

	screenIndex = 0;

//	hDetailHelp = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, &HelpScreenCallback, 0);
	hDetailHelp = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, &HelpScreenCallback, 0);

	hButton = BUTTON_CreateEx((XSIZE_PHYS - HELP_BUTTON_WIDTH) / 2, YSIZE_PHYS - HELP_BUTTON_HEIGHT, HELP_BUTTON_WIDTH, HELP_BUTTON_HEIGHT, hDetailHelp, WM_CF_SHOW, 0, EXIT_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmHelpExitButton);
	BUTTON_SetTextColor(hButton, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hButton, BUTTON_CI_PRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hButton, BUTTON_CI_DISABLED, GUI_WHITE);

	hPreviousButton = BUTTON_CreateEx(HELP_BUTTON_OFFSET, YSIZE_PHYS - HELP_BUTTON_HEIGHT, HELP_BUTTON_WIDTH, HELP_BUTTON_HEIGHT, hDetailHelp, WM_CF_SHOW, 0, LAST_BUTTON);
	BUTTON_SetTextColor(hPreviousButton, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hPreviousButton, BUTTON_CI_PRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hPreviousButton, BUTTON_CI_DISABLED, GUI_WHITE);

	hNextButton = BUTTON_CreateEx(XSIZE_PHYS - HELP_BUTTON_OFFSET - HELP_BUTTON_WIDTH, YSIZE_PHYS - HELP_BUTTON_HEIGHT, HELP_BUTTON_WIDTH, HELP_BUTTON_HEIGHT, hDetailHelp, WM_CF_SHOW, 0, NEXT_BUTTON);
	BUTTON_SetTextColor(hNextButton, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hNextButton, BUTTON_CI_PRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hNextButton, BUTTON_CI_DISABLED, GUI_BLACK);

	UpdateButtonGraphics();
}


void HelpScreenCallback(WM_MESSAGE * pMsg)
{
//	WM_HWIN hDlg;
	int Id;
	int NCode;

//	hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_PAINT:
			GUI_JPEG_Draw(pScreens[screenIndex].pScreen, pScreens[screenIndex].size, 0, 0);
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
			case WM_NOTIFICATION_CLICKED:
				switch (Id)
				{
				case EXIT_BUTTON:
					WM_DeleteWindow(hDetailHelp);
					pHelpScreens = 0;			// Force (enable) user to select new help topic next time through
					suspendForHelp = FALSE;
					WM_SetFocus(hOwner);		// Return focus to calling window
					break;

				case LAST_BUTTON:
					if (--screenIndex < 0)
						screenIndex = 0;
					else WM_InvalidateWindow(hDetailHelp);
					UpdateButtonGraphics();
					break;

				case NEXT_BUTTON:
					if (pScreens[++screenIndex].pScreen == 0)
						screenIndex--;
					else WM_InvalidateWindow(hDetailHelp);
					UpdateButtonGraphics();
					break;
				}
				break;

			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

//
// Set the button graphics to reflect if there are more screens to the "right" or "left"
//
// Input: pScreens = current screen rotation list
//		  screenIndex = current help window in current rotation
//
void UpdateButtonGraphics(void)
{
	if (screenIndex == 0)
	{
		BUTTON_SetBitmap(hPreviousButton, BUTTON_BI_UNPRESSED, &bmHelpDisabled);

		// See if only one screen in rotation
		if (pScreens[screenIndex + 1].pScreen == 0)
			BUTTON_SetBitmap(hNextButton, BUTTON_BI_UNPRESSED, &bmHelpDisabled);
		else BUTTON_SetBitmap(hNextButton, BUTTON_BI_UNPRESSED, &bmHelpNextButton);
	}
	else if (pScreens[screenIndex + 1].pScreen == 0)
	{
		BUTTON_SetBitmap(hPreviousButton, BUTTON_BI_UNPRESSED, &bmHelpPreviousButton);
		BUTTON_SetBitmap(hNextButton, BUTTON_BI_UNPRESSED, &bmHelpDisabled);
	}
	else
	{
		BUTTON_SetBitmap(hPreviousButton, BUTTON_BI_UNPRESSED, &bmHelpPreviousButton);
		BUTTON_SetBitmap(hNextButton, BUTTON_BI_UNPRESSED, &bmHelpNextButton);
	}
}

void DrawJPEGSplash(void)
{
	GUI_JPEG_Draw(acHome1, sizeof(acHome1), 0, 0);
}

