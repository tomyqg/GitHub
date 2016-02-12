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
#include <ctype.h>
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
#include "RaceSetupScreens.h"
#include "SystemSettingsScreens.h"
#include "VehicleSettingsScreens.h"
#include "DataHandler.h"
#include "LEDHandler.h"
#include "TachSetupScreens.h"
#include "InputScreens.h"
#include "ReviewSessionsScreens.h"
#include "StandardList.h"
#include "TrackSettingsScreens.h"
#include "GearingScreens.h"
#include "AnalogInputSettings.h"
#include "DigitalInputSettings.h"
#include "InterTask.h"
#include "AnalogInputsScale.h"
#include "GearRatios.h"
#include "SFScreens.h"
#ifndef _WINDOWS
#include "FS.h"
#include "CommTask.h"
#include "dataflash.h"
#endif
#include "TrackDetect.h"
#include "Utilities.h"
#include "ConfigurableGaugeCommon.h"
#include "HelpScreens.h"


//
// Local conditional compile flags
//
//#define	SUPPORT_AUDIO


// KMC Development code for slideout test

#define	SLIDEOUT_OPEN_REGION_X		350
#define	SLIDEOUT_OPEN_REGION_Y		225
#define	SLIDEOUT_CLOSE_REGION		264
#ifdef SUPPORT_AUDIO
#define	SPRITE_END_POS				234
#else
#define	SPRITE_END_POS				240
#define	SLIDEOUT_CLOSE_REGION_Y		187
#endif
#define	SU_SPRITE_ICON_X_OFFSET			122
#define	CAMERA_SPRITE_ICON_X_OFFSET		165
#define	SIGSTRN_SPRITE_ICON_X_OFFSET	202

#define	SLIDEOUT_OPEN_TIME			5000
#define	TWO_SECONDS					2000

#ifdef _WINDOWS
// Slideout moves at different speeds on the traget HW so use seperate contants to control it
// PC version
#define	SLIDEOUT_STEP_FAST_TIME		10
#define	SLIDEOUT_STEP_FAST_SIZE		100
#define	SLIDEOUT_STEP_SLOW_TIME		10
#define	SLIDEOUT_STEP_SLOW_SIZE		8
#else
// Target HW version
#define	SLIDEOUT_STEP_FAST_TIME		10
#define	SLIDEOUT_STEP_FAST_SIZE		100
#define	SLIDEOUT_STEP_SLOW_TIME		10
#define	SLIDEOUT_STEP_SLOW_SIZE		24
#endif

enum SLIDEOUT_STATE {
	SO_CLOSED,
	SO_OPENING,
	SO_CLOSING,
	SO_WAITING
};
enum MOVE_BACK {
	ONE_SCREEN,
	ALL_THE_WAY
};

enum SLIDEOUT_STATE slideoutState;
int slideoutXPos = 0;
int slideoutYPos = 0;
long hSlideoutTimer = 0;
int backKeyPressedTime = 0;
int configKeyPressedTime = 0;
int slideoutStepSize, slideoutStepTime;
GUI_HSPRITE hSlideout = 0;
GUI_HSPRITE hSUStatusSprite = 0;
GUI_HSPRITE hCameraSprite = 0;
GUI_HSPRITE hSignalStrengthSprite = 0;

#ifdef	INCLUDE_RACE_HELP
int raceStartTime = 0;
int raceTimerActive = 0;

extern GUI_CONST_STORAGE GUI_BITMAP bmGenericGaugeHelp;
#endif

extern GUI_CONST_STORAGE GUI_BITMAP bmNoAudioSlideout;
//extern GUI_CONST_STORAGE GUI_BITMAP bmSystemStatusScreenHelp;
//extern GUI_CONST_STORAGE GUI_BITMAP bmSystemStatusScreenHelpBlackBack;

#ifdef SUPPORT_AUDIO
extern GUI_CONST_STORAGE GUI_BITMAP bmSlideOut;
extern GUI_CONST_STORAGE GUI_BITMAP bmSlideOutMuted;
#endif

void StartSlideout(void);
void ProcessSlideoutKeyPress(int lastx, int lasty);
void CloseSlider(void);
void DeleteSlideout(void);
int TouchPosIsReset(int lastx, int lasty);


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/
int screen_scrolling;
int pressingbutton_semaphore = 0;
long hScreenTimer;
int selectedSessionUnsorted;
char processingScreenTransition = false;
enum RACE_MODES raceModeState;
enum GUI_RACE_MODES selectedRaceMode;
int glbBlackoutFlag = false;
int numSessions;

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
extern GUI_CONST_STORAGE GUI_BITMAP bmGoRacingText;
extern GUI_CONST_STORAGE GUI_BITMAP bmDrive;
extern GUI_CONST_STORAGE GUI_BITMAP bmDragButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmDisplayText;
extern GUI_CONST_STORAGE GUI_BITMAP bmDriver;
extern GUI_CONST_STORAGE GUI_BITMAP bmDriverText;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceReview;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceReviewText;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceSetupText;
extern GUI_CONST_STORAGE GUI_BITMAP bmStorage;
extern GUI_CONST_STORAGE GUI_BITMAP bmSystemSetup;
extern GUI_CONST_STORAGE GUI_BITMAP bmSystemSettingsText;
extern GUI_CONST_STORAGE GUI_BITMAP bmSystemSetupText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeGPS;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTrack;
extern GUI_CONST_STORAGE GUI_BITMAP bmTrackText;
extern GUI_CONST_STORAGE GUI_BITMAP bmUnits;
extern GUI_CONST_STORAGE GUI_BITMAP bmUnitsText;
extern GUI_CONST_STORAGE GUI_BITMAP bmVehicle;
extern GUI_CONST_STORAGE GUI_BITMAP bmVehicleText;
extern GUI_CONST_STORAGE GUI_BITMAP bmDisplay;
extern GUI_CONST_STORAGE GUI_BITMAP bmGoRacing;
extern GUI_CONST_STORAGE GUI_BITMAP bmTraqmateLogo;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoxRally;
extern GUI_CONST_STORAGE GUI_BITMAP bmRace;
//extern GUI_CONST_STORAGE GUI_BITMAP bmInstruments;
extern GUI_CONST_STORAGE GUI_BITMAP bmCamera;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlackButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGrayButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBigGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmStdGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSmallYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlankListWheelScreen;
//extern GUI_CONST_STORAGE GUI_BITMAP bmLaunchGForce;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoStartButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmPredictive;
extern GUI_CONST_STORAGE GUI_BITMAP bmLapsQualifyButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRecordingOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmRecordingOff;
extern int testMode;			// Test variable to enable test mode - the 9th screen!
#ifndef _WINDOWS
extern char OkToUseSDCard(void);
extern int demoMode;
#endif
extern char fullSessionName[];
extern char sessionName[];
extern char fileList[];			// List of failed config files at startup
extern int popupTitleTextColor;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void MoveRight(enum MOVE_BACK howFar);
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
void DisplayGoRacing(WM_HWIN);
void DisplayReviewSessions(WM_HWIN);
void DisplaySystemSettings(WM_HWIN);
void DisplayRaceSetup(WM_HWIN);
void GoRacingPaint(WM_HWIN);
void ReviewSessionsPaint(WM_HWIN);
void SystemSettingsPaint(WM_HWIN);
void RaceSetupPaint(WM_HWIN);
void SetConfigWindowHandle(WM_HWIN);
WM_HWIN GetCurrentWindowHandle(void);
int GetNumScreens(enum SCREEN_TYPE screenType);
//int okToStopRecording(WM_HWIN hParent, int ypos);
void SetSessionSelection(int sel);
void ShowHelpPopup(int detailsAvail);
void HelpCallback(WM_MESSAGE * pMsg);
int ProcessScreenExit(void);
char *GetSystemStatsHelpText(const char *pText);
char *GetStorageStatsHelpText(const char *pText);

void DisplayClosestTracks(WM_HWIN hPrev);
void ProceedToRaceMode(enum GUI_RACE_MODES mode, WM_HWIN hParent);
void SaveRaceMode(enum GUI_RACE_MODES mode);
int GetCurrentScreenID(void);
WM_HWIN DisplaySavingData(void);
int DateTimeSort(void const *p1, void const *p2);
int LapTimeSort(void const *p1, void const *p2);
int PopulateSessionLISTVIEW(int selection);
int GetMins(const char *pData);
void SetButtonColors(int i);
void SetRecordModeGraphic(void);
enum SESSION_TYPE ConvertRaceModeToSessionType(enum GUI_RACE_MODES mode);
void FinishPressed(void);
int GetPrevGaugeScreen(void);
int GetNextGaugeScreen(void);
char InactiveGaugeScreen(int screen);
int GaugeScreenIndexToID(enum SCREEN_TYPE screenType, int screenIndex);
void UpdateSessionListView(WM_HWIN hParent, char updateSelectedSession);
void SetSpritePositions(void);
char TouchPosIsBackKey2(int lastx, int lasty);
void StartBackKeyTimer(void);
char BackTimerExpired(void);
void StartConfigKeyTimer(void);
char ConfigTimerExpired(void);
char AnyNonDragGaugeScreen(enum SCREEN_TYPE type);
char *GetAnalogInputText(int index, char *pBuffer);
char *GetDigitalInputText(int index, char *pBuffer);
unsigned char CheckForStopRecord(void);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
// Note: the order of these entries must correspond to the screen ID indexes
struct SCREEN_LIST ConfigScreenList[NUM_CONFIG_SCREENS+1] = {
	{ GO_RACING, DisplayGoRacing, GoRacingPaint },
	{ RACE_SETUP, DisplayRaceSetup, RaceSetupPaint },
	{ REVIEW_SESSIONS, DisplayReviewSessions, ReviewSessionsPaint },
	{ SYSTEM_SETUP, DisplaySystemSettings, SystemSettingsPaint },
	{ -1, NULL, NULL }
};

// Size of touchscreen dedicated to the reactive portion for the navigation (BACK) and INFO keys
#define	NAVI_SIZE_X		60
#define	NAVI_SIZE_Y		60
#define	INFO_SIZE_X		50
#define	INFO_SIZE_Y		50

#define	NARROW_BUTTON_WIDTH	80
#define	NARROW_BUTTON_LEFT	358
#define	BOTTOM_BUTTON_ROW	228
#define	COL1	36
#define	COL2	214
#define	COL3	302
#define	COL4	390


#define	GRAPHICS
#define	TRANSITION			2

#define	SCROLL_TIME			6
#define	SCROLL_STEP_SIZE	2

#define	DELETE_SESSION_BTN		GUI_ID_USER + 1
#define	CHECK_TIMES_BTN			GUI_ID_USER + 2
#define	SESSION_TYPE_BTN		GUI_ID_USER + 3
#define	DRIVER_BUTTON			GUI_ID_USER + 4
#define	VEHICLE_BUTTON			GUI_ID_USER + 5
#define	TRACK_BUTTON			GUI_ID_USER + 6
#define	ROADRACE_BUTTON			GUI_ID_USER + 7
#define	AUTOXRALLY_BUTTON		GUI_ID_USER + 8
#define	DRIVE_BUTTON			GUI_ID_USER + 9
#define	RECORDMODE_BUTTON		GUI_ID_USER + 10
#define	LAPSQUALIFYING_BUTTON	GUI_ID_USER + 11
#define	DISPLAY_BUTTON			GUI_ID_USER + 12
#define	UNITS_BUTTON			GUI_ID_USER + 13
#define	TIME_BUTTON				GUI_ID_USER + 14
#define	AUTOSTART_BUTTON		GUI_ID_USER + 15
#define	STORAGE_BUTTON			GUI_ID_USER + 16
#define	PREDICTIVE_BUTTON		GUI_ID_USER + 17
#define	GUI_ID_ABORT			GUI_ID_USER + 18
#define	BEST_LAP_BTN			GUI_ID_USER + 19
#define	DRAG_BUTTON				GUI_ID_USER + 20

// KMC - changed to LISTVIEW
//#define	REVIEW_SESSIONS_LIST		GUI_ID_LISTWHEEL1
#define	REVIEW_SESSIONS_LIST		GUI_ID_LISTVIEW0


/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hConfigScreen = 0;
//WM_HWIN hDataRequester;
static int previous_touch_state;
//static int KMCDialogSemaphone = 0;
static int nextScreen/*, peekingScreen*/;
static int clickedOnThisScreen;
BUTTON_Handle hBestTime, hDetailsButton, hDeleteButton;
BUTTON_Handle hSelectedReviewMode;
BUTTON_Handle hRecordModeButton;
// KMC - changed to LISTVIEW
//LISTWHEEL_Handle hListWheel;
LISTVIEW_Handle hListView;
GUI_HWIN hDisplayClosestTracks;
static char waitedForSF = false;
static char GSignoreRelease = false;
WM_HWIN hSavingDataDialog;
char userAbort;
char searchingSemaphore;
char reviewingSession = 0;
int selectedSessionSorted;

/*********************************************************************
*
*       Help Screen Definitions
*
**********************************************************************
*/
const char MAIN_SCREEN_INFO[] =
"Go Racing to record data or show instruments.\
\n\nRace Review to see sessions and lap times.\
\n\nRace Setup to select Driver, Vehicle, or Track or set up Inputs.\
\n\nSystem Setup for display, time, storage, and other general settings.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char GO_RACING_INFO[] =
"Lap/Race to record data on closed circuit racetrack.\
\n\nAuto-X/Rally for launch-enabled separate start/finish recording.\
\n\nDrive - touch left side of button for general drive recording. Right side of button toggles recording for this mode.\
\n\nDrag for launch-enabled drag race performance modes including 1/8mile, 1000', and 1/4mile.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char RACE_SETUP_INFO[] =
"Driver to select or create driver.\
\n\nVehicle to select or create vehicle or edit tach, gearing, inputs, and camera settings.\
\n\nTrack to manually select or create track.\
\n\nLaps/Qualify to choose Laps or Qualifying Predictive Timing Mode.\
\nIn Lap Mode, lap time is compared with best ever at this track.\
\nIn Qualifying Mode, lap time is compared with best time in the current session.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char SYSTEMS_SETUP_INFO[] =
"Display to adjust backlight and LED intensity.\
\n\nUnits to select between US and Metric units.\
\n\nTime to select Time Zone, Daylight Savings Time, and GPS Simulation.\
\n\nStartup to Enable AutoStart and AutoOn features.\
\n\nWiFi/SD to manage SD card and configure wireless download feature.\
\n\nPredictive Graph Scale to set scale of Predictive Lap Timer on timing screens.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char REVIEW_SESSIONS_INFO[] =
"Shows date that session was recorded, session number on that day, and track.\
\n\nTouch any session on list to select.\
\nTouch column headings to sort.\
\nTouch scrollbar to see additional laps.\
\n\nTouch large button on lower left to show Best Time, Driver, and Vehicle.\
\n\nTouch second button on bottom to show sessions recorded in Race, AutoX, Drive, and Drag Race modes.\
\n\nUse Details to see session lap times and other session information.\
\n\nDelete will erase currently selected session.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char SESSION_DETAILS_INFO[] =
"Touch blue button to step through session detail pages.\
\n\nTIMING\nFor Race mode, shows lap number, lap time in list and session information on right. \
Best Lap shown in green.\
\nFor Drag mode, shows time slip.\
\n\nTouch column headings to sort.\
\nTouch scrollbar to see additional laps.\
\n\nGENERAL\nShows session duration in minutes, distance traveled in session, range of satellites \
seen during session, and Dilution of Precision indicating quality of satellites available. For DOP \
lower numbers are better.\
\n\nPERFORMANCE\nShows Max Performance Values such as speed and accelerometer Gs.\
\n\nINPUTS\nShows tach overrevs, and range of all enabled analog inputs.\
\n\nCONFIG\nShows which analogs, digitals, and cameras are enabled.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char SELECT_DRIVER_INFO[] =
"Use list to select driver. Exit screen when complete.\
\n\nNew Driver to create a driver.\
\n\nEdit Name to change selected driver's name.\
\n\nDelete Driver to remove driver from list.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char SELECT_VEHICLE_INFO[] =
"Use list to select vehicle. Exit screen when complete.\
\n\nVehicle Settings to change tach, inputs, camera, gearing, etc.\
\n\nNew Vehicle to create a vehicle.\
\n\nEdit Name to change selected vehicle's name.\
\n\nDelete Vehicle to remove vehicle from list.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

// through the Race Setup menu
const char SELECT_TRACK_INFO[] =
"Use list to select track. Exit screen when complete.\
\n\nTop orange button chooses range of tracks to display.\
\n\nSettings to change start/finish, track type, clear predictive lap, and set lap hold time.\
\n\nRight button on second line to select which type of tracks to show, Road Race, Drive, AutoX, and Drag.\
\n\nNew Track to create a track.\
\n\nEdit Name to change selected track's name.\
\n\nDelete Track to remove track from list.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

// when current track out of range
const char CHOOSE_TRACK_INFO[] =
"Reaching this screen indicates that the currently selected track is not close to your current GPS location.\
\nThe tracks nearby are shown in the list or you can have TraqDash automatically create a new track.\
\n\nUse list to select track.\
\n\nConfirm Track when track is selected.\
\n\nAuto-Create Track to create a track.\
\n\nNew Track to create a track by name.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char LAP_QUAL_INFO[] =
"Select Lap or Qualifying Mode for the Predictive Lap Timing.\
\n\nNOTE: Lapping Mode compares against the Best Lap recorded at this track.\
\n\nQualifying Mode compares against only the best lap within the current session.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char VEHICLE_SETTINGS_INFO[] =
"Tach to change range of tach LEDs.\
\n\nAero/Grip to change tire and vehicle aero settings.\
\n\nGear Ratio to enter gear and diff ratios for gear display.\
\n\nInputs to change analog, digital inputs for this vehicle.\
\n\nWeight to enter vehicle weight.\
\n\nCamera to select a camera for this vehicle.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TRACK_SETTINGS_INFO[] =
"Track Type to change configuration of race track.\
\n\nLap Holdtime to change duration that lap time help on display.\
\n\nStart/Finish to clear start/finish or enter new coordinates.\
\n\nPredictive Lap to clear predictive lap data for selected track.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TRACK_TYPE_INFO[] =
"Road Race - Closed = road course with start/finish.\
\n\nAuto-X/Rally - Open = autocross or rally stage with launch mode and separate start and finish.\
\n\nDragstrip - Open = launch mode for 1/8 mi, 1/4 mi, or 1000' distance.\
\n\nOval - Closed = oval track with start/finish.\
\n\nHill Climb - Open = rolling start stage with separate start and finish.\
\n\nDrive - Open = record data over distance with start at beginning, finish at end.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char LAP_HOLD_INFO[] =
"Use list wheel to select lap hold time. Exit screen when complete.\
\n\nThis is the amount of time the previous lap time will be displayed after crossing the start / finish line.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char START_FINISH_INFO[] =
"Touch LAT, LON, and HEADING buttons to enter the coordinates.\
\nExample: 34.150383, -83.814232, 121\
\n\nClear Start/Finish to clear coordinates and set during the next session.\
\nThis will also clear the Predictive Lap information.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char PREDICTIVE_LAP_INFO[] =
"Reset Time to clear the predictive lap time information for this track.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char DISPLAY_SETTINGS_INFO[] =
"Use sliders to set LCD backlight and Tach and Warning LED intensity.\
\n\nAuto-Adjust ON will automatically adjust brightness for changes in ambient light.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char UNITS_SETTINGS_INFO[] =
"Select US or Metric units.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TIME_SETTINGS_INFO[] =
"Use list wheel to set time zone.\
\n\nDaylight Savings ON/OFF will add or subtract an hour.\
\n\nWhen Normal GPS is shown, system is attempting to use real GPS signal. Pressing this button will cause system to enter GPS Simulation Mode.\
\n\nWhen Simulating GPS is shown, system is using a fake GPS signal. This is useful for testing cameras or doing dyno runs when GPS signal cannot \
be received. Pressing this button will cause system to stop simulation and enter Normal GPS Mode.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char AUTOSTART_SETTINGS_INFO[] =
"Auto Power On switches between power on with TraqDash button and power on when power is applied to the Traqmate system. \
Note that Auto Power On will disable the power button and only works when connected to a second generation Sensor Unit (black label, 3 axis).\
\n\nAutoStart to Race Screens will allow the unit to start recording if unit was powered down on a recording screen. \
AutoStart to Menu Screens will start on the last menu screen visited.\
\n\nPress Record Settings to set a speed to start recording and a sitting still time to stop recording.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char STORAGE_SETTINGS_INFO[] =
"Erase All Sessions will erase all data sessions from the SD card but retain other settings.\
\n\nFactory Defaults will erase all data sessions, drivers, vehicles, and tracks, to return to factory settings. Non-Traqmate data is not affected.\
\n\nFormat SD Card will format the card and reset to factory settings. All data on SD card is lost.\
\n\nPersonalize/WiFi will let you enter your Owner Name and Contact Info, configure the WiFi download feature and show serial number and build date.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char PREDICTIVE_SCALE_INFO[] =
"Use list wheel to choose the scale for the predictive lap time graphic.\
\n\nFor example, 1 second = +- 1 second.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TACH_SETTINGS_INFO[] =
"Current RPM is live engine speed for reference.\
\n\nMax RPM to set the true redline of the vehicle.\
\n\nDrag red slider to RPM where red shift light will illuminate. \
Touch corresponding number value for fine tuning.\
\n\nDrag yellow slider to RPM where first yellow shift light will illuminate. \
Touch corresponding number value for fine tuning.\
\n\nDrag green slider to set engine cylinders or touch number. \
Engine Cyl is the divider for the RPM, often equal to number of cylinders.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TACH_PWREND_INFO[] =
"Use list wheel to set the powerband end tach RPM.\
\n\nThis is the end of the green tach area and has no effect on the tach warning LEDs.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TACH_WARNING_INFO[] =
"Use list wheel to set the warning tach RPM.\
\n\nThis is the start of the yellow tach area and starting RPM for warning LEDs.\
\n\nThe RPM between WARNING and REDLINE is divided evenly to set the RPM where the yellow LEDs will illuminate.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TACH_REDLINE_INFO[] =
"Use list wheel to set the redline tach RPM.\
\n\nThis is the RPM where the red tach LED will illuminate.\
\n\nThe RPM between WARNING and REDLINE is divided evenly to set the RPM where the yellow LEDs will illuminate.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TACH_MAXRPM_INFO[] =
"Use list wheel to set the maximum tach RPM.\
\n\nThis is the maximum value for the tach display and the limit for the Traqview graph.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char NUM_CYL_INFO[] =
"Use list wheel to set divider for the RPM to adjust the tach value displayed.\
\n\nNOTE: For ECU tach wire connection, use cylinders = 1 or cylinders = 2.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char AERO_GRIP_INFO[] =
"These settings are used to calculate horsepower.\
\n\nUse slider to set aerodynamic drag of vehicle.\
\n\nUse slider to set tire friction coefficient of vehicle.\
\n\nNOTE:See User Manual for suggested settings.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char CAR_METRICS_INFO[] =
"This screen displays and controls various cumulative metrics. \
Respective reset buttons clear the cumulative metrics. \
Note that metrics span multiple sessions and power cycles.\
\n\nTotal time is how long the Traqmate has been operated since last cleared.\
\n\nTotal distance is amount the vehicle has traveled since last cleared.\
\n\nMax RPM is the maximum RPM recorded since last cleared.\
\n\nAero/Grip values are vehicle settings related to HP calculations.\
The Aero/Grip button transfers to setup screen for Aero/Grip settings.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char GEAR_RATIO_INFO[] =
"These settings are used to calculate the gear number shown on tach screen.\
\n\nTire Size to enter size of drive wheels/tires.\
\n\nDiff Ratio to enter ratio of differential to drive wheels.\
\n\nTrans Ratio to enter transmission gear ratios.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength.\
\n\nNOTE: In order to determine which gear is selected in the vehicle, the \
complete gearing information for the vehicle must be known.  This \
includes the gear ratios as well as the differential ratio and the \
tire rolling circumference. If you do not wish to use this \
feature it can be disabled by entering a differential ratio of 0.00. \
If this value is entered you will be asked to confirm that you do not \
wish to calculate gear ratios. If accepted then the calculated gear \
position will not be displayed.";

const char TIRE_SIZE_INFO[] =
"These settings are used to calculate the gear number shown on tach screen.\
\n\nTire Size to enter information based on manufacturer tire size.\
\n\nTire circumference to manually measure and enter the circumference of the tire.\
\n\nTire circumference is in inches or centimeters, depending on units setting.\
\n\nNOTE: In order to determine which gear is selected in the vehicle, the \
circumference. (distance around) the tire must be measured.  This is \
the distance the vehicle travels for every revolution of the tire. \
You may enter the number on the sidewall of the tire using the \
calculator or you can measure the distance around the tire with a tape \
measure.  Most tire circumferences are in the range of 50in (127cm) \
to 150in (381cm).\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TIRE_DIMENSIONS_INFO[] =
"These settings are used to calculate the gear number shown on tach screen.\
\n\nSelect the manufacturer tire size (usually found on sidewall).\
\n\nFirst column is tire width in mm.\
\n\nSecond column is tire aspect ratio.\
\n\nThird column is wheel diameter in inches.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char DIFF_RATIO_INFO[] =
"These settings are used to calculate the gear number shown on tach screen.\
\n\nUse the list wheels to enter the 3 digit differential ratio.\
\n\nNOTE: Enter 0.00 to disable the gear number display on the tach screen.\
\n\nFor example, if differential ratio is 4.10:1, enter 4.10.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char TRANS_RATIO_INFO[] =
"These settings are used to calculate the gear number shown on tach screen.\
\n\nTouch each of the ratios in turn to enter the gear ratios.\
\n\nFor example, if gear ratio is 1.33:1, enter 1.33.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char INPUTS_INFO[] =
"Touch the Analog or Digital to enable or disable the input.\
\n\nWhen enabled, the analog buttons show the value of the input in the defined units.\
\n\nWhen enabled, the digital buttons show the named logical state of the input.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char WEIGHT_INFO[] =
"Use the list wheels to enter the 4 digit weight in pounds or kilograms, depending on Unit setting.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char CAMERA_INFO[] =
"Use the list to select a camera for the Traqmate to control or NONE if no camera is connected.\
\n\nIf GoPro HERO1 or HERO2, choose the selection for the appropriate Sensor Unit interface module.\
\n\nIf Replay XD1080, choose the selection for the appropriate Sensor Unit interface module.\
\n\nIf Replay Mini or Prime X, choose the appropriate selection.\
\n\nIf Sony HD camcorder, select Sony HD.\
\n\nIf RaceCam HD camcorder, choose RaceCam HD.\
\n\nIf ChaseCam PDR-100 connected via TraqData HD (LANC), select ChaseCam SD LANC.\
\n\nNOTE: ChaseCam PDR-100 connected via TraqSync or TraqData II and all Sony SD camcorders are no longer supported.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char CAMERA_SETUP_INFO[] =
"Enables/disables and configures cameras.\
\n\nTraqDash USB Cam toggles enabling of camera attached to the TraqDash USB port.\
\n\nTraqDash Cam Delay sets the startup time for the USB camera. If a camera is attached to the Sensor Unit, the USB camera will be delayed by the set time. \
Otherwise, the data collection will be delayed by the set time to allow the camera time to initialize.\
\n\nIf no SU camera is connected, use these values: Mobius or Innovv camera = 4.0s, ReplayXD = 7.4s.\
\n\nSensor Unit Cam selects the camera type which is attached to the Sensor Unit via TraqData or TraqSync modules.\
\n\nTest Cameras will send out the command to turn on all enabled cameras to allow them to be tested. \
Cameras will be turned off when you leave this screen or press Test Cameras a second time.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char CAMERA_DELAY_INFO[] =
"Use the list wheel to choose the number of seconds. If a camera is attached to the Sensor Unit, the USB camera will be delayed by the set time. \
Otherwise, the data collection will be delayed by the set time to allow the camera time to initialize.\
Minimum value is 0.0 and maximum value is 9.9.\
\n\nUse approximately 7.4 seconds for a Replay camera and 4.0 second for a Mobius Action Cam.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char ANALOG_SETTINGS_INFO[] =
"Shows the physical and alarm state of the analog input.\
\n\nEnable or Disable button to turn on or off recording of this input.\
\n\nEdit Scale to change between voltage ranges.\
\n\nEdit Alarm to assign an alarm range to an Alarm LED.\
\n\nRename Input to change the name of the analog input.\
\n\nRename Units to change the name of the units displayed.\
\n\nSome common inputs:\
\n\n** AEM AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 8.5 HIGH 18.0,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** AEM 75PSI MAP Sensor **\
\nVolts: LOW 0.5V HIGH 4.5V,\
\nValues: LOW -14.7 HIGH 35.3,\
\n5 Volt Range,\
\nUnits = \"PSI\"\
\n\n** AEM Temperature Sensor **\
\nVolts: LOW 0.016V HIGH 0.9V,\
\nValues: LOW 32.0 HIGH 230.0,\
\n5 Volt Range,\
\nUnits = \"DegF\"\
\n\n** PLX AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 10.0 HIGH 20.0,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** PLX Pressure Sensor **\
\nVolts: LOW 0.0V HIGH 3.75V,\
\nValues: LOW 0.0 HIGH 150.0,\
\n5 Volt Range,\
\nUnits = \"PSI\"\
\n\n** PLX Temperature Sensor **\
\nVolts: LOW 0.0V HIGH 5.0V,\
\nValues: LOW 32.0 HIGH 302.0,\
\n5 Volt Range,\
\nUnits = \"DegF\"\
\n\n** PLX Exhaust Temperature **\
\nVolts: LOW 0.0V HIGH 3.33V,\
\nValues: LOW 32.0 HIGH 1000.0,\
\n5 Volt Range,\
\nUnits = \"DegF\"\
\n\n** PLX Boost Sensor **\
\nVolts: LOW 0.0V HIGH 5V,\
\nValues: LOW -15 HIGH 30,\
\n5 Volt Range,\
\nUnits = \"PSI\"\
\n\n** Innovate LC-1 AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 7.35 HIGH 22.33,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** APSX Wideband D1 AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 9.0 HIGH 19.0,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** Battery Voltage **\
\nVolts: LOW 0V HIGH 20V,\
\nValues: LOW 0.0 HIGH 20.0,\
\n20 Volt Range,\
\nUnits = \"Volts\",\
\nAlarm Below 11.5\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char DIGITAL_SETTINGS_INFO[] =
"Shows the physical state of the digital input.\
\n\nEnable or Disable button to turn on or off recording of this input.\
\n\nEdit Alarm to assign one state of the digital input to an Alarm LED.\
\n\nRename Input to change the name of the digtial input.\
\n\nRename High to change the name of the HIGH physical state.\
\n\nRename Low to change the name of the LOW physical state.\
\n\nTo Connect Brake Light:\
\nSet Input Name to \"Brake\"\
\nSet HIGH Name to \"Stop\"\
\nSet LOW Name to \"Go\"\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char ANALOG_SCALE_INFO[] =
"This screen sets the scaling factors for the analog input from the voltage to the entered units.\
\n\nVolts sets the measured range of voltage from the LOW reading to the HIGH reading.\
\n\nValue sets the actual reading range corresponding to the voltage range in the selected units.\
\n\nVoltage Range selects from 5, 10, or 20 volt ranges from the TraqData input module.\
\n\nNOTE: The Volts range cannot be outside the selected Voltage Range.\
\n\nSome common inputs:\
\n\n** AEM AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 8.5 HIGH 18.0,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** AEM 75PSI MAP Sensor **\
\nVolts: LOW 0.5V HIGH 4.5V,\
\nValues: LOW -14.7 HIGH 35.3,\
\n5 Volt Range,\
\nUnits = \"PSI\"\
\n\n** AEM Temperature Sensor **\
\nVolts: LOW 0.016V HIGH 0.9V,\
\nValues: LOW 32.0 HIGH 230.0,\
\n5 Volt Range,\
\nUnits = \"DegF\"\
\n\n** PLX AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 10.0 HIGH 20.0,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** PLX Pressure Sensor **\
\nVolts: LOW 0.0V HIGH 3.75V,\
\nValues: LOW 0.0 HIGH 150.0,\
\n5 Volt Range,\
\nUnits = \"PSI\"\
\n\n** PLX Temperature Sensor **\
\nVolts: LOW 0.0V HIGH 5.0V,\
\nValues: LOW 32.0 HIGH 302.0,\
\n5 Volt Range,\
\nUnits = \"DegF\"\
\n\n** PLX Exhaust Temperature **\
\nVolts: LOW 0.0V HIGH 3.33V,\
\nValues: LOW 32.0 HIGH 1000.0,\
\n5 Volt Range,\
\nUnits = \"DegF\"\
\n\n** PLX Boost Sensor **\
\nVolts: LOW 0.0V HIGH 5V,\
\nValues: LOW -15 HIGH 30,\
\n5 Volt Range,\
\nUnits = \"PSI\"\
\n\n** Innovate LC-1 AirFuel **\
\nVolts: LOW 0V HIGH 5V,\
\nValues: LOW 7.35 HIGH 22.33,\
\n5 Volt Range,\
\nUnits = \"Ratio\",\
\nAlarm Above 13.7\
\n\n** Battery Voltage **\
\nVolts: LOW 0V HIGH 20V,\
\nValues: LOW 0.0 HIGH 20.0,\
\n20 Volt Range,\
\nUnits = \"Volts\",\
\nAlarm Below 11.5\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char ANALOG_ALARM_INFO[] =
"LED 1 Enable / Disable to add or remove this input from the\
\nUpper Warning LED.\
\n\nLED 2 Enable / Disable to add or remove this input from the\
\nLower Warning LED.\
\n\nAlarm Level sets the limit for the alarm state, Above, Below, or Equal To the entered value.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char DIGITAL_ALARM_INFO[] =
"LED 1 Enable / Disable to add or remove this input from the\
\nUpper warning LED.\
\n\nLED 2 Enable / Disable to add or remove this input from the\
\nLower Warning LED.\
\n\nAlarm State selects the logical value corresponding to an alarm state.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char PERSONALIZE_INFO[] =
"TraqDash owner to change owner name.\
\n\nContact Info to enter a phone number or email address in case of loss or service.\
\n\nWiFi Name is the broadcast name for the wireless download option.\
\n\nWiFi Password is the password for the wireless network. Must be at least 8 characters.\
\n\nScreen also shows unit serial number and build date.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

const char DRAG_STAGE_SCREEN_INFO[] =
"Select timing screen from gauge screen buttons on left of screen.\
\n\nSelect the drag strip length from buttons in center column of screen.\
\n\nTouch to Stage when the vehicle is stopped at the starting line. \
Timing will begin when the vehicle starts moving.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.";

const char AUTOX_STAGE_SCREEN_INFO[] =
"Select timing screen from gauge screen buttons on left of screen.\
\n\nTouch to Stage when the vehicle is stopped at the starting line. \
Timing will begin when the vehicle starts moving.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.";

const char RECORD_SPEED_INFO[] =
"Use list wheel to set the vehicle speed at which you would like the Traqmate and cameras to \
begin recording. Then enter a Recording (Race or Drive) mode. Actual \
recording (data and video) will begin at the chosen speed.\
\n\nUse list wheel to set a stationary time. If the Traqmate is recording and the vehicle is not \
moving, recording will stop after the chosen time expires.\
\n\nPress Red Back Button to go up one menu or hold for Main Menu.\
\n\nIcons: Camera Status, SD Card Status, GPS Signal Strength";

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

//
// Set the last screen of any type and save changes to the file system
//
void SetLastScreen(enum SCREEN_TYPE screen_type, int screen)
{
	switch (sysData.screentype = screen_type)
	{
		case CONFIG_SCREEN_TYPE:			sysData.lastConfigScreen = screen;				break;
		case RACE_SETUP_SCREENS_TYPE:		sysData.lastRaceSetupScreen = screen;			break;
		case SYSTEM_SETTINGS_SCREENS_TYPE:	sysData.lastSystemSettingsScreen = screen;		break;
		case VEHICLE_SETTINGS_SCREENS_TYPE:	sysData.lastVehicleSettingsScreen = screen;		break;
		case TACH_SETUP_SCREENS_TYPE:		sysData.lastTachSetupScreen = screen;			break;
		case INPUTS_SCREENS_TYPE:			sysData.lastInputsScreen = screen;				break;
		case REVIEW_SESSIONS_SCREENS_TYPE:	sysData.lastReviewSessionsScreen = screen;		break;
		case TRACK_SETTINGS_SCREENS_TYPE:	sysData.lastTrackSettingsScreen = screen;		break;
		case GAUGES_SCREEN_TYPE:			sysData.lastGaugeScreen = screen;				break;
		case DRIVE_MODE_SCREEN_TYPE:		sysData.lastDriveScreen = screen;				break;
		case INSTRUMENTS_MODE_SCREEN_TYPE:	sysData.lastInstrumentsScreen = screen;			break;
		case GEARING_SCREENS_TYPE:			sysData.lastGearingScreen = screen;				break;
		case GEAR_RATIOS:					sysData.lastGearRatioScreen = screen;			break;
		case SFSCREENS_TYPE:				sysData.lastSFScreen = screen;					break;
		case ANALOG_SETTINGS_SCREEN_TYPE:	sysData.lastAnalogInputSettingsScreen = screen;	break;
		case ANALOG_INPUTS_SCALE:			sysData.lastAnalogScaleScreen = screen;			break;
		case DIGITAL_SETTINGS_SCREEN_TYPE:	sysData.lastDigitalInputSettingsScreen = screen;break;
		case TIRE_DIMENSIONS_SCREEN_TYPE:	sysData.lastTireDimensionsScreen = screen;		break;
	}
}

WM_HWIN GetCurrentWindowHandle(void)
{
	switch (GetScreenType())
	{
	case CONFIG_SCREEN_TYPE:			return hConfigScreen;
	case RACE_SETUP_SCREENS_TYPE:		return GetRaceSetupScreensWindowHandle();
	case SYSTEM_SETTINGS_SCREENS_TYPE:	return GetSystemSettingsWindowHandle();
	case VEHICLE_SETTING_SCREEN_TYPE:	return GetVehicleSettingMainWindowHandle();
	case VEHICLE_SETTINGS_SCREENS_TYPE:	return GetVehicleSettingsWindowHandle();
	case DRIVE_MODE_SCREEN_TYPE:
	case INSTRUMENTS_MODE_SCREEN_TYPE:
	case GAUGES_SCREEN_TYPE:			return GetRunningGaugesWindowHandle();
	case TACH_SETUP_SCREENS_TYPE:		return GetTachSetupWindowHandle();
	case INPUTS_SCREENS_TYPE:			return GetInputsWindowHandle();
	case REVIEW_SESSIONS_SCREENS_TYPE:	return GetReviewSessionsWindowHandle();
	case TRACK_SETTING_SCREEN_TYPE:		return GetTrackSettingMainWindowHandle();
	case TRACK_SETTINGS_SCREENS_TYPE:	return GetTrackSettingsWindowHandle();
	case GEARING_SCREENS_TYPE:			return GetGearingWindowHandle();
	case GEAR_RATIOS:					return GetGearRatiosWindowHandle();
	case ANALOG_SETTINGS_SCREEN_TYPE:	return GetAnalogInputSettingsWindowHandle();
	case ANALOG_INPUTS_SCALE:			return GetAnalogInputsScaleWindowHandle();
	case DIGITAL_SETTINGS_SCREEN_TYPE:	return GetDigitalInputSettingsWindowHandle();
	case GENERAL_CONFIG_SCREEN_TYPE:	return GetGeneralConfinScreenWindowHandle();
	default:							return 0;
	}
}

int GetLastScreen(enum SCREEN_TYPE screenType)
{
	switch (screenType)
	{
		case CONFIG_SCREEN_TYPE:			return sysData.lastConfigScreen;
		case RACE_SETUP_SCREENS_TYPE:		return sysData.lastRaceSetupScreen;
		case SYSTEM_SETTINGS_SCREENS_TYPE:	return sysData.lastSystemSettingsScreen;
		case VEHICLE_SETTINGS_SCREENS_TYPE:	return sysData.lastVehicleSettingsScreen;
		case TACH_SETUP_SCREENS_TYPE:		return sysData.lastTachSetupScreen;
		case INPUTS_SCREENS_TYPE:			return sysData.lastInputsScreen;
		case REVIEW_SESSIONS_SCREENS_TYPE:	return sysData.lastReviewSessionsScreen;
		case TRACK_SETTINGS_SCREENS_TYPE:	return sysData.lastTrackSettingsScreen;
		case DRIVE_MODE_SCREEN_TYPE:
			if (!testMode)
			{	// Only allow to return to TEST_INFO_SCREEN (last screen in list) if we are in testMode,
				// otherwise go back to the screen prior to the last screen
				if (sysData.lastDriveScreen >= NUM_DRIVE_SCREENS - 1)
					sysData.lastDriveScreen = NUM_DRIVE_SCREENS - 2;

				// Be sure the screen we ended up on is not the TEST_INFO_SCREEN screen
				if (GaugeScreenIndexToID(DRIVE_MODE_SCREEN_TYPE, sysData.lastDriveScreen) == TEST_INFO_SCREEN)
					sysData.lastDriveScreen -= 1;		// Rotate to the previous screen in the rotation
														// Note this means the test mode screen CANNOT be the first
														// screen in the rotation!
			}
			return sysData.lastDriveScreen;
		case INSTRUMENTS_MODE_SCREEN_TYPE:
			if (!testMode)
			{	// Only allow to return to TEST_INFO_SCREEN (last screen in list) if we are in testMode,
				// otherwise go back to the screen prior to the last screen
				if (sysData.lastInstrumentsScreen >= NUM_INSTRUMENTS_SCREENS - 1)
					sysData.lastInstrumentsScreen = NUM_INSTRUMENTS_SCREENS - 2;

				// Be sure the screen we ended up on is not the TEST_INFO_SCREEN screen
				if (GaugeScreenIndexToID(INSTRUMENTS_MODE_SCREEN_TYPE, sysData.lastInstrumentsScreen) == TEST_INFO_SCREEN)
					sysData.lastInstrumentsScreen -= 1;		// Rotate to the previous screen in the rotation
															// Note this means the test mode screen CANNOT be the first
															// screen in the rotation!
			}
			return sysData.lastInstrumentsScreen;
		case GAUGES_SCREEN_TYPE:
			if (!testMode)
			{	// Only allow to return to TEST_INFO_SCREEN or GPS_INFO (last two screens in list) if we are in testMode,
				// otherwise go back to the screen prior to the last screen
				if (sysData.lastGaugeScreen >= NUM_GAUGE_SCREENS - 1)
					sysData.lastGaugeScreen = NUM_GAUGE_SCREENS - 2;

				// Be sure the screen we ended up on is not the TEST_INFO_SCREEN screen
				if (GaugeScreenIndexToID(GAUGES_SCREEN_TYPE, sysData.lastGaugeScreen) == TEST_INFO_SCREEN)
					sysData.lastGaugeScreen -= 1;		// Rotate to the previous screen in the rotation
														// Note this means the test mode screen CANNOT be the first
														// screen in the rotation!
			}
			return sysData.lastGaugeScreen;
		case GEARING_SCREENS_TYPE:			return sysData.lastGearingScreen;
		case GEAR_RATIOS:					return sysData.lastGearRatioScreen;
		case SFSCREENS_TYPE:				return sysData.lastSFScreen;
		case ANALOG_SETTINGS_SCREEN_TYPE:	return sysData.lastAnalogInputSettingsScreen;
		case ANALOG_INPUTS_SCALE:			return sysData.lastAnalogScaleScreen;
		case DIGITAL_SETTINGS_SCREEN_TYPE:	return sysData.lastDigitalInputSettingsScreen;
		case TIRE_DIMENSIONS_SCREEN_TYPE:	return sysData.lastTireDimensionsScreen;
		case AERO_GRIP_SCREEN_TYPE:			return AERO_GRIP_SCREEN_TYPE;
		default:							return 0;
	}
}

int GetCurrentScreen(void)
{
	return GetLastScreen(GetScreenType());
}

int GetNumScreens(enum SCREEN_TYPE screenType)
{
	switch (screenType)
	{
		case CONFIG_SCREEN_TYPE:			return NUM_CONFIG_SCREENS;
		case RACE_SETUP_SCREENS_TYPE:		return NUM_RACE_SETUP_SCREENS;
		case SYSTEM_SETTINGS_SCREENS_TYPE:	return NUM_SYSTEM_SETTINGS_SCREENS;
		case VEHICLE_SETTINGS_SCREENS_TYPE:	return NUM_VEHICLE_SETTINGS_SCREENS;
		case DRIVE_MODE_SCREEN_TYPE:
			if (testMode)
				return NUM_DRIVE_SCREENS;
#if PROFILER
			else return NUM_DRIVE_SCREENS - 2;
#else
			else return NUM_DRIVE_SCREENS - 1;
#endif
		case INSTRUMENTS_MODE_SCREEN_TYPE:
			if (testMode)
				return NUM_INSTRUMENTS_SCREENS;
#if PROFILER
			else return NUM_INSTRUMENTS_SCREENS - 2;
#else
			else return NUM_INSTRUMENTS_SCREENS - 1;
#endif
		case GAUGES_SCREEN_TYPE:
			if (testMode)
				return NUM_GAUGE_SCREENS;
#if PROFILER
			else return NUM_GAUGE_SCREENS - 2;
#else
			else return NUM_GAUGE_SCREENS - 1;
#endif
		case TACH_SETUP_SCREENS_TYPE:		return NUM_TACH_SETUP_SCREENS;
		case INPUTS_SCREENS_TYPE:			return NUM_INPUTS_SCREENS;
		case REVIEW_SESSIONS_SCREENS_TYPE:	return NUM_REVIEW_SESSIONS_SCREENS;
		case TRACK_SETTINGS_SCREENS_TYPE:	return NUM_TRACK_SETTINGS_SCREENS;
		case GEARING_SCREENS_TYPE:			return NUM_GEARING_SCREENS;
		case GEAR_RATIOS:					return NUM_GEAR_RATIOS_SCREENS;
		case SFSCREENS_TYPE:				return NUM_SF_SCREENS;
		case ANALOG_SETTINGS_SCREEN_TYPE:	return NUM_ANALOG_INPUT_SCREENS;
		case ANALOG_INPUTS_SCALE:			return NUM_ANALOG_SCALE_SCREENS;
		case DIGITAL_SETTINGS_SCREEN_TYPE:	return NUM_DIGITAL_INPUT_SCREENS;
		default:							return 1;
	}
}

void SetScreenType(enum SCREEN_TYPE screentype)
{
	sysData.screentype = screentype;
}

enum SCREEN_TYPE GetScreenType(void)
{
	return sysData.screentype;
}


int GetPrevIndex(void)
{
int index;
	
	index = GetLastScreen(sysData.screentype);
	if (--index < 0)
		index = GetNumScreens(sysData.screentype) - 1;
	return index;
}

int GetNextIndex(void)
{
int index;

	index = GetLastScreen(sysData.screentype) + 1;
	if (index > GetNumScreens(sysData.screentype) - 1)
		index = 0;
	return index;
}

void ValidateRaceScreens(void)
{
	int maxDriveScreens, maxInstrumentsScreens, maxGaugesScreens;

	maxDriveScreens =  GetNumScreens(DRIVE_MODE_SCREEN_TYPE) - 1;
	maxInstrumentsScreens = GetNumScreens(INSTRUMENTS_MODE_SCREEN_TYPE) - 1;
	maxGaugesScreens = GetNumScreens(GAUGES_SCREEN_TYPE) - 1;

	if (sysData.lastDriveScreen > maxDriveScreens)
		sysData.lastDriveScreen = maxDriveScreens;
	if (sysData.lastInstrumentsScreen > maxInstrumentsScreens)
		sysData.lastInstrumentsScreen = maxInstrumentsScreens;
	if (sysData.lastGaugeScreen > maxGaugesScreens)
		sysData.lastGaugeScreen = maxGaugesScreens;
}

int GetNextGaugeScreen(void)
{
int index, limit;

	index = GetLastScreen(sysData.screentype);
	limit = GetNumScreens(sysData.screentype) - 1;

	do {
		if (++index > limit)
			index = 0;
	} while (InactiveGaugeScreen(index));	// If screen not 'active' get next screen

	return index;
}

int GetPrevGaugeScreen(void)
{
int index, limit;

	index = GetLastScreen(sysData.screentype);
	limit = GetNumScreens(sysData.screentype) - 1;

	do {
		if (--index < 0)
			index = limit;
	} while (InactiveGaugeScreen(index));	// If screen not 'active' get previous screen

	return index;
}

//
// Check to see if this screen is a valid gauge screen for this mode.
//		Screens will be invalid if it is an inactive display (ie gauges on screen are disabled)
//		or if it is a test mode screen and we are not in test mode.
// Return true if the screen is disabled
// Return false if the screen is active and can be displayed
//
char InactiveGaugeScreen(int index)
{
	int screen;

	screen = GaugeScreenIndexToID(sysData.screentype, index);

	// Now all dual analog and linear gauge screens are just one, MULTI_ANALOG_GAUGES
	// See if any analog inputs enabled, if so then display the screen and then determine
	// which gauges to show when displaying
	if (screen == MULTI_ANALOG_GAUGES)
	{
		if (DetermineAnalogScreenSelection() == 0)
			return true;
		else return false;
	}
	if (screen == TEST_INFO_SCREEN)
	{
		if (testMode)
			return false;	// screen allowed
		else return true;	// screen not allowed
	}
	else return false;
}

int GaugeScreenIndexToID(enum SCREEN_TYPE screenType, int screenIndex)
{
	switch (screenType)
	{
	case GAUGES_SCREEN_TYPE:
		if ((screenIndex < 0) || (screenIndex >= NUM_GAUGE_SCREENS))
			screenIndex = DEFAULT_RACE_MODE_SCREEN_INDEX;
		return GaugesScreenList[screenIndex].screenID;
	case DRIVE_MODE_SCREEN_TYPE:
		if ((screenIndex < 0) || (screenIndex >= NUM_DRIVE_SCREENS))
			screenIndex = DEFAULT_DRIVE_MODE_SCREEN_INDEX;
		return DriveScreenList[screenIndex].screenID;
	case INSTRUMENTS_MODE_SCREEN_TYPE:
		if ((screenIndex < 0) || (screenIndex >= NUM_INSTRUMENTS_SCREENS))
			screenIndex = DEFAULT_INSTRUMENTS_MODE_SCREEN_INDEX;
		return InstrumentsScreenList[screenIndex].screenID;
	default:
		return 0;
	}
}


void _cbMainCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;
	int Sel;

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
		ConfigScreensIgnoreNextKeyRelease();
		break;

	case WM_PAINT:
		if (GetScreenType() == CONFIG_SCREEN_TYPE)
		{
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_Font24_ASCII);
			// Call paint method of center screen
			ConfigScreenList[GetLastScreen(CONFIG_SCREEN_TYPE)].paint(hConfigScreen);
			DrawSignalStrength();
		}
		break;

	case WM_NOTIFY_PARENT:
		Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
		NCode = pMsg->Data.v;				// Notification code
		switch (NCode)
		{
		case WM_NOTIFICATION_CLICKED:
			clickedOnThisScreen = true;
			if (Id == REVIEW_SESSIONS_LIST)
			{
				if (SessionNameData[0].date[0] != 0)				// Be sure there is session data before attempting to sort/select
				{
					selectedSessionSorted = LISTVIEW_GetSel(hListView);		// Get current sorted selection
					Sel = LISTVIEW_GetSelUnsorted(hListView);	// Get current selection
					SetSessionSelection(Sel);
					GetSessionName();
				}
			}
			break;

		case WM_NOTIFICATION_RELEASED:
			if (!clickedOnThisScreen)
				break;
			switch (Id)
			{
			case DISPLAY_BUTTON:
				DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, DISPLAY_SELECTION, hConfigScreen);
				break;

			case UNITS_BUTTON:
				DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, UNITS_SELECTION, hConfigScreen);
				break;

			case TIME_BUTTON:
				DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, TIME_SELECTION, hConfigScreen);
				break;

			case AUTOSTART_BUTTON:
				DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, AUTOSTART_SELECTION, hConfigScreen);
				break;

			case STORAGE_BUTTON:
#ifdef _WINDOWS
				DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, STORAGE_SELECTION, hConfigScreen);
#else
				if (OkToUseSDCard())
					DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, STORAGE_SELECTION, hConfigScreen);
				else NotSupportedPopup(hConfigScreen);
#endif
				break;

			case PREDICTIVE_BUTTON:
				DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, LAP_METER, hConfigScreen);
				break;

			case ROADRACE_BUTTON:
				SaveRaceMode(ROADRACE_MODE);
				GoRacingStateMachine(hConfigScreen);
				break;
			case DRAG_BUTTON:
				SaveRaceMode(DRAG_MODE);
				GoRacingStateMachine(hConfigScreen);
				break;
			case AUTOXRALLY_BUTTON:
				SaveRaceMode(AUTOXRALLY_MODE);
				GoRacingStateMachine(hConfigScreen);
				break;
			case DRIVE_BUTTON:
				if (sysData.disableDriveRecordMode)
					SaveRaceMode(INSTRUMENTS_MODE);
				else SaveRaceMode(DRIVE_MODE);
				GoRacingStateMachine(hConfigScreen);
				break;
			case RECORDMODE_BUTTON:
				if (sysData.disableDriveRecordMode)
					sysData.disableDriveRecordMode = 0;
				else sysData.disableDriveRecordMode = 1;
				SetRecordModeGraphic();
				break;

			case DRIVER_BUTTON:
#ifdef _WINDOWS
				ctType = CT_NORMAL_MODE;
				SetWorkingTrackName();
				DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, DRIVER_SELECTION, hConfigScreen);
#else
				if (OkToUseSDCard())
				{
					ctType = CT_NORMAL_MODE;
					SetWorkingTrackName();
					DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, DRIVER_SELECTION, hConfigScreen);
				}
				else NotSupportedPopup(hConfigScreen);
#endif
				break;

			case VEHICLE_BUTTON:
#ifdef _WINDOWS
				ctType = CT_NORMAL_MODE;
				SetWorkingTrackName();
				DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, VEHICLE_SELECTION, hConfigScreen);
#else
				ctType = CT_NORMAL_MODE;
				SetWorkingTrackName();
				DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, VEHICLE_SELECTION, hConfigScreen);
#endif
				break;

			case TRACK_BUTTON:
				ctType = CT_NORMAL_MODE;
				SetWorkingTrackName();
				DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, TRACK_SELECTION, hConfigScreen);
				break;

			case LAPSQUALIFYING_BUTTON:
				ctType = CT_NORMAL_MODE;
				SetWorkingTrackName();
				DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, LAPSQUALIFYING_SELECTION, hConfigScreen);
				break;

			case SESSION_TYPE_BTN:
				{
					WM_HWIN hDialog;
					int endTime;

					hDialog = SystemSettingsPopUp(LOADING_SESSIONS, hConfigScreen);
					endTime = GUI_GetTime() + 500;
					GUI_Exec();

					IncrementSessionType();

					UpdateSessionListView(hDialog, true);

					while (endTime > GUI_GetTime())			// Wait so the popup is displayed for at least 1/2 second
						;									// so it doesn't flicker
					WM_DeleteWindow(hDialog);
				}
				break;

			case BEST_LAP_BTN:
				IncrementDisplayType();
				SetBestTimeButtonText();
				break;

			case DELETE_SESSION_BTN:
				if (numSessions > 0)
				{
					char *pName;

					pName = GetSessionName();
					if (strcmp(pName, ""))
					{
						if (SystemSettingsPopUp(DELETE_SESSION, hConfigScreen))
						{
							// Get current sorted position in list
							// Delete the current selection
							// Repopulate the list with the old session gone
							// Select the session that falls into the same sorted position as the deleted session

							int i;

							reviewingSession = true;
							DeleteSession();

							CreateFileList(SESSION_DATA);	// Get best lap information out of session files

							// Call GetSessionNameData with NO parent - don't bother to tell user (potentially again) that there are too many sessions
							GetSessionNameData(0, 1, (enum SESSION_TYPE)sysData.selectedSessionType);	// Populate SessionNameData table with ALL sessions of this type
							i = PopulateSessionLISTVIEW(-1);

							GUI_Exec();		// Let something/someone run to allow the list to reflect new contents
							LISTVIEW_SetSel(hListView, selectedSessionSorted);			// Selected the session in this position in the SORTED list after the deleted file has been removed
							LISTVIEW_SetBkColor(hListView, LISTVIEW_CI_SELFOCUS, GUI_LISTBLUE);
							selectedSessionUnsorted = LISTVIEW_GetSelUnsorted(hListView);	// Get new unsorted position

				//			BUTTON_SetText(hSelectedReviewMode, GetSelectedReviewModeText());
							GetSessionData();
							SetBestTimeButtonText();
							SetButtonColors(i);
						}
					}
				}
				break;

			case CHECK_TIMES_BTN:
				if (numSessions > 0)
				{
					// In order to preserve session tables selection setting do not delete the old window, just
					// create a new one for the details button and come back to this when we're done.
					DisplaySessionDetailsScreen();

					reviewingSession = true;
					sysData.navigatedToReviewAfterRace = false;		// Mark that we DID NOT get to the session details by ending a race
				}
				break;
			}	// End switch (Id)
			break;
		}
		break;	// End case for WM_NOTIFY_PARENT

	default:
			WM_DefaultProc(pMsg);
	}
}

void UpdateSessionListView(WM_HWIN hParent, char updateSelectedSession)
{
	int i;

	if ((i = CreateFileList(SESSION_DATA)) != 0)		// Get best lap information out of session files
	{
		if (updateSelectedSession)
			selectedSessionUnsorted = FindNewestFile((enum SESSION_TYPE)sysData.selectedSessionType);

		GetSessionNameData(hParent, 0, (enum SESSION_TYPE)sysData.selectedSessionType);
		GetSessionData();
	}
	else
	{
		selectedSessionUnsorted = -1;
		ClearSessionNameData();
	}
	
	i = PopulateSessionLISTVIEW(selectedSessionUnsorted);
	SetBestTimeButtonText();
	SetButtonColors(i);
}


void IncrementSessionType(void)
{
	switch (sysData.selectedSessionType)
	{
	case ROADRACE_TYPE:		sysData.selectedSessionType = DRIVE_TYPE;		break;
	case DRIVE_TYPE:		sysData.selectedSessionType = AUTOCROSS_TYPE;	break;
	case AUTOCROSS_TYPE:	sysData.selectedSessionType = QUARTERMILE_TYPE;	break;
	//case QUARTERMILE_TYPE:	sysData.selectedSessionType = HILLCLIMB_TYPE;	break;
	//case EIGHTHMILE_TYPE:		sysData.selectedSessionType = HILLCLIMB_TYPE;	break;
	//case THOUSANDFOOT_TYPE:	sysData.selectedSessionType = HILLCLIMB_TYPE;	break;
	//case HILLCLIMB_TYPE:		sysData.selectedSessionType = ROADRACE_TYPE;	break;
	//case NOT_VALID:			sysData.selectedSessionType = ROADRACE_TYPE;	break;
	default:				sysData.selectedSessionType = ROADRACE_TYPE;	break;
	}
}

void SetSessionSelection(int sel)
{
	if (selectedSessionUnsorted != sel)
	{
		selectedSessionUnsorted = sel;
		GetSessionData();
		SetBestTimeButtonText();
		WM_InvalidateWindow(hConfigScreen);	// This seems to be needed on target HW to get the driver name and time to be displayed (but it is not required on the PC!).
	}
}

void StartPressed(WM_HWIN hLastScreen)
{
	GaugeScreensIgnoreNextKeyRelease();
	notifyStartSet();
	raceModeState = RM_HAVE_SF;
	GoRacingStateMachine(hLastScreen);
}

void FinishPressed(void)
{
#ifdef _WINDOWS
	SetUserMode(DU2_AUTOX_COMPLETE);
#endif
	notifyFinishSet();
}


WM_HWIN GetConfigWindowHandle(void)
{
	return hConfigScreen;
}

void SetConfigWindowHandle(WM_HWIN hWin)
{
	hConfigScreen = hWin;
}

//
// Save any data necessary and kill any keyboards, if required, before leaving previous screen
//	Note: saves sysData every time to save the screen transitions
// Returns: true if okay to transition to next screen
//			false if an error was detected with the data and we need to stay where we are
//
int ProcessScreenExit(void)
{
	switch (GetScreenType())
	{
		case GAUGES_SCREEN_TYPE:
		case DRIVE_MODE_SCREEN_TYPE:
		case INSTRUMENTS_MODE_SCREEN_TYPE:
// KMC 3/25/12 - if waiting for SF we have not yet started recording 'real' data, therefore exit waithout save
// Would case crash in the case of powerup, then go racing but no track yet existed so went to Press at SF, and then exit
//		case WAITING_FOR_SF_SCREEN_TYPE:
		case DRAG_AUTOX_GAUGE_SCREEN_TYPE:
			ExitRecord(true);
			break;

// KMC 3/25/12 - if waiting for SF we have not yet started recording 'real' data, therefore exit waithout save
// Would case crash in the case of powerup, then go racing but no track yet existed so went to Press at SF, and then exit
		case WAITING_FOR_SF_SCREEN_TYPE:			// KMC 3/25/12 - moved here
		case WAITING_FOR_STAGE_SCREEN_TYPE:
		case WAITING_FOR_LAUNCH_SCREEN_TYPE:
			ExitRecord(false);
			break;

		case AERO_GRIP_SCREEN_TYPE:
		case VEHICLE_SETTINGS_SCREENS_TYPE:
			if (ValidateVehicleData())
			{
				TestCameras( false );
				WriteDataStructures(VEHICLE_DATA);
				SetGear( GEAR_RESET );		// recalculate gear ratio table
			}
			else return false;
			break;

		case TRACK_SETTINGS_SCREENS_TYPE:
			if (alteredTrackData)
			{
				if (ValidateSFCoordinates())
				{
					trackTable[trackTableIndex].sdCardEntry = 0;	// May have edited track data, we now want to save data to disk!
					WriteUserDataEvent(TRACK_DATA);
					trackTable[trackTableIndex].sdCardEntry = 1;
					DeleteSFScreensKeyboards();
					alteredTrackData = false;
				}
				else return false;
			}
			break;

		case TACH_SETUP_SCREENS_TYPE:
		case INPUTS_SCREENS_TYPE:
		case GEAR_RATIOS:
		case DIGITAL_SETTINGS_SCREEN_TYPE:
			WriteDataStructures(VEHICLE_DATA);
			break;

		case GEARING_SCREENS_TYPE:
			if (ValidateDiffRatio())
			{
				WriteDataStructures(VEHICLE_DATA);
				DeleteGearRatiosKeyboards();
			}
			else return false;
			break;

		case ANALOG_SETTINGS_SCREEN_TYPE:
			if (ValidateAnalogData())
			{
				WriteDataStructures(VEHICLE_DATA);
				DeleteAnalogScaleKeyboards();
			}
			else return false;
			break;

		//case SFSCREENS_TYPE:
		//	return (ValidateSFCoordinates());
		//	break;

		case TIRE_DIMENSIONS_SCREEN_TYPE:
			if (GetLastScreen(TIRE_DIMENSIONS_SCREEN_TYPE) != TIRESIZE_SCREEN)
				TCKeyboardSaveButton();
			break;

		case ANALOG_NUMBER_ENTRY_STANDALONE:
			AISKillKeyboard();
			break;

		case DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE:
			ExitTrackSelectionScreen();				// Be sure this track is selected and loaded
			ResumeScreenScrolling();
			HideSprites();
			break;

		case RACE_SETUP_SCREENS_TYPE:
			ctType = CT_NORMAL_MODE;
			if (vehicleSelectionChanged == true)	// If the vehicle selection has changed on this screen
				ChangeVehicle();					// Validate and make the change in the database
			ExitTrackSelectionScreen();				// Be sure this track is selected and loaded
			break;

		case CAMERA_SELECTION_SCREEN_TYPE:
		case TRAQDASH_CAMERA_SCREEN:
			if (ValidateCamera())
				WriteDataStructures(VEHICLE_DATA);
			else return false;
			break;

		case CAMERA_DELAY_SCREEN_TYPE:
			SetCameraDelay();
			WriteDataStructures(VEHICLE_DATA);
			break;
	}

	WriteDataStructures(SYSTEM_DATA);
	return true;
}

//
// MoveRight - navigate backwards one of more screens
// Input	howFar = ONE_SCREEN = move back one screen only
//					 ALL_THE_WAY = move back all the way to the main menu
//
void MoveRight(enum MOVE_BACK howFar)
{
	if (ProcessScreenExit())
	{
		// see how long back key pressed
		if (howFar == ALL_THE_WAY)
			Display_GeneralConfig(GetCurrentWindowHandle());		// Back key pressed more than 2 seconds - jump to main menu
		// else back key pressed less than 2 seconds, perform normal one screen back function
		else switch (GetScreenType())
		{
		case DRIVE_MODE_SCREEN_TYPE:
		case INSTRUMENTS_MODE_SCREEN_TYPE:
		case GAUGES_SCREEN_TYPE:
// KMC 9/9/12 exit record directly to race review			Display_GeneralConfig(GetRunningGaugesWindowHandle());
			if (selectedRaceMode != INSTRUMENTS_MODE)		// If recording a session
			{												// Go directly to review session screen
				WM_DeleteWindow(GetRunningGaugesWindowHandle());
				SetConfigWindowHandle((GUI_HWIN)NULL);		// Be sure config screen handle is 0 to force exit from
															// review to go through code to rebuild session list
				DisplaySessionDetailsScreen();
				sysData.navigatedToReviewAfterRace = true;	// Mark that we navigated by race exit to the review screen
			}
			else Display_GeneralConfig(GetRunningGaugesWindowHandle());	// Otherwise return, as normal, to main screen
			break;

		case WAITING_FOR_SF_SCREEN_TYPE:
			DisplayNewScreenList(CONFIG_SCREEN_TYPE, GetLastScreen(CONFIG_SCREEN_TYPE),GetSFWindowHandle());
			break;

		case WAITING_FOR_STAGE_SCREEN_TYPE:
			DisplayNewScreenList(CONFIG_SCREEN_TYPE, GO_RACING, GetStageWindowHandle());
			break;

		case WAITING_FOR_LAUNCH_SCREEN_TYPE:
			if (selectedRaceMode == DRAG_MODE)
				DisplayDragStageScreen(GetLaunchScreenWindowHandle());
			else DisplayAutoXStageScreen(GetLaunchScreenWindowHandle());
			break;

		case DRAG_AUTOX_GAUGE_SCREEN_TYPE:
// KMC 9/9/12 exit record directly to race review 			Display_GeneralConfig(GetDragAutoXGaugesWindowHandle());
			WM_DeleteWindow(GetDragAutoXGaugesWindowHandle());
			SetConfigWindowHandle((GUI_HWIN)NULL);			// Be sure config screen handle is 0 to force exit from
															// review to go through code to rebuild session list
			DisplaySessionDetailsScreen();
			sysData.navigatedToReviewAfterRace = true;		// Mark that we navigated by race exit to the review screen
			break;

		case CONFIG_SCREEN_TYPE:
			Display_GeneralConfig(GetConfigWindowHandle());
			break;
		case VEHICLE_SETTINGS_SCREENS_TYPE:
			Display_VehicleSettingMainScreen(GetVehicleSettingsWindowHandle());
			break;
		case TRACK_SETTINGS_SCREENS_TYPE:
			Display_TrackSettingMainScreen(GetTrackSettingsWindowHandle());
			break;
		case RACE_SETUP_SCREENS_TYPE:
			DisplayNewScreenList(CONFIG_SCREEN_TYPE, GetLastScreen(CONFIG_SCREEN_TYPE), GetRaceSetupScreensWindowHandle());
			break;
		case SYSTEM_SETTINGS_SCREENS_TYPE:
			DisplayNewScreenList(CONFIG_SCREEN_TYPE, GetLastScreen(CONFIG_SCREEN_TYPE), GetSystemSettingsWindowHandle());
			break;
		case REVIEW_SESSIONS_SCREENS_TYPE:
			ReturnFromReviewSessionsScreen();
			break;
		case TACH_SETUP_SCREENS_TYPE:
			DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, GetLastScreen(VEHICLE_SETTINGS_SCREENS_TYPE), GetTachSetupWindowHandle());
			break;
		case INPUTS_SCREENS_TYPE:
			DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, GetLastScreen(VEHICLE_SETTINGS_SCREENS_TYPE), GetInputsWindowHandle());
			break;
		case VEHICLE_SETTING_SCREEN_TYPE:
			ctType = CT_NORMAL_MODE;
			DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, GetLastScreen(RACE_SETUP_SCREENS_TYPE), GetVehicleSettingMainWindowHandle());
			break;
		case GEARING_SCREENS_TYPE:
			DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, GetLastScreen(VEHICLE_SETTINGS_SCREENS_TYPE), GetGearingWindowHandle());
			break;
		case GEAR_RATIOS:
			DisplayNewScreenList(GEARING_SCREENS_TYPE, GetLastScreen(GEARING_SCREENS_TYPE), GetGearRatiosWindowHandle());
			break;
		case ANALOG_SETTINGS_SCREEN_TYPE:
			{
				int lastScreen;

				// Do not allow BACK function from keyboards
				lastScreen = GetLastScreen(ANALOG_SETTINGS_SCREEN_TYPE);
				if ((lastScreen == SCALE) || (lastScreen == ALARM))
					DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), GetAnalogInputSettingsWindowHandle());
				}
			break;
		case ANALOG_INPUTS_SCALE:
			DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, GetLastScreen(ANALOG_SETTINGS_SCREEN_TYPE), GetAnalogInputsScaleWindowHandle());
			break;
		case DIGITAL_SETTINGS_SCREEN_TYPE:
			// Do not allow BACK function from keyboards
			if (GetLastScreen(DIGITAL_SETTINGS_SCREEN_TYPE) == DIG_ALARM_STATE)
				DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), GetDigitalInputSettingsWindowHandle());
			break;
		case TRACK_SETTING_SCREEN_TYPE:	
			ctType = CT_NORMAL_MODE;
			DisplayNewScreenList(RACE_SETUP_SCREENS_TYPE, GetLastScreen(RACE_SETUP_SCREENS_TYPE), GetTrackSettingMainWindowHandle());
			break;
		case TIRE_DIMENSIONS_SCREEN_TYPE:
			if (GetLastScreen(TIRE_DIMENSIONS_SCREEN_TYPE) == TIRESIZE_SCREEN)
				DisplayNewScreenList(GEARING_SCREENS_TYPE, GetLastScreen(GEARING_SCREENS_TYPE), GetTireSizeScreenWindowHandle());
			break;
		case DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE:
			DisplayNewScreenList(CONFIG_SCREEN_TYPE, GO_RACING, hDisplayClosestTracks);
			break;
		case WAITING_FOR_SATELLITES_SCREEN_TYPE:
			KillWaitingForSatelliteScreen(false);
			break;
		case WAITING_FOR_CAMERA_SCREEN_TYPE:
			raceModeState = RM_CANCELLED_WAITING_FOR_CAMERA;
			GoRacingStateMachine(hWaitingForCamera);
			break;
		case PERSONALIZE_REVIEW_SCREEN_TYPE:
			PersonalizeScreenExit();			// Check before exiting screen - might want to reset
			DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, STORAGE_SELECTION, hPersonalizationScreen);
			break;
		case AERO_GRIP_SCREEN_TYPE:
			DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, CARMETRICS_SELECTION, GetAeroGripWindowHandle());
			break;
		case CAMERA_DELAY_SCREEN_TYPE:
			CreateTraqdashCamScreen(GetCameraDelayWindowHandle());
			break;
		case CAMERA_SELECTION_SCREEN_TYPE:
			DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, GetLastScreen(VEHICLE_SETTINGS_SCREENS_TYPE), GetCameraSelectionWindowHandle());
			break;
		case TRAQDASH_CAMERA_SCREEN:
			DisplayNewScreenList(VEHICLE_SETTINGS_SCREENS_TYPE, GetLastScreen(VEHICLE_SETTINGS_SCREENS_TYPE), GetTraqdashCamWindowHandle());
			break;
		case HELP_SCREEN_TYPE:
			DeleteHelpScreen();
			break;
		case RECORD_SPEED_SCREEN_TYPE:
			DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, AUTOSTART_SELECTION, hPersonalizationScreen);
			break;
		}
	}
}



void ProcessScreenMovement(void)
{
	GUI_PID_STATE TouchState;
	signed int x, y;
	static signed int lastx, lasty;

	GUI_TOUCH_GetState(&TouchState);  /* Get the touch position in pixel */

	if (TouchState.Pressed)			// If touch pressed
	{
		if (pressingbutton_semaphore == 1)
		{
// Screen self-scrolling no longer handled by executive loop - now handled by timer
			return;			// Wait for it to be released
		}
		x = TouchState.x;
		y = TouchState.y;

		lastx = x;
		lasty = y;

		if (previous_touch_state == 0)
		{
			// Screen just touched - see if any special "buttons" are pressed
			previous_touch_state = 1;

			// Screen just touched - if we just touched the BACK key then start the back key press timer
			if (TouchPosIsBackKey2(lastx, lasty))
			{
				StartBackKeyTimer();
				return;
			}
			else StartConfigKeyTimer();	// Else check to see if this is a configurable gauge screen 
										// and start the configure screen timer if it is

			if (configureGaugeMode == true)				// If we are in configurable gauge mode
			{
				ProcessGaugeButton(lastx, lasty);		// Send key position info to gauge screen for further processing
				GSignoreRelease = true;					// Since the rest of the screen keys respond to release tell to ignore this release
			}
		}
		else if (TouchPosIsBackKey2(lastx, lasty))		// key still pressed - if this key is the back key
		{
			if (BackTimerExpired())						// and it has now been held for the BACK key timeout time
				if (configureGaugeMode != true)			// and if we are not in configurable gauge mode
					MoveRight(ALL_THE_WAY);				// process key (jump to main menu)
		}
		else if (ConfigTimerExpired())
		{
			ConfigureGauge();
			GSignoreRelease = true;		// Since the rest of the screen keys respond to release tell to ignore this release
		}
	}

	// Else touch screen not pressed
	else if (screen_scrolling != HORIZONTAL_ONLY)
	{
		pressingbutton_semaphore = 0;	// clear semaphore
		if (previous_touch_state == 1)	// if were just touching
		{
			previous_touch_state = 0;	// Calculate position for auto continue

			// We can use this flag more globally than just gauge screens - use for any time we want to ignore the next key release
			if (GSignoreRelease == true)
			{
				GSignoreRelease = false;
				goto skip;
			}

			if (AnyGaugeScreen(GetScreenType()))
			// Jump directly to new screen if touching top or bottom half of screen (gauges screens only)
			{
				if ((selectedRaceMode == DRAG_MODE) || (selectedRaceMode == AUTOXRALLY_MODE))
				{
					// Drag/Autocross mode selected - see if any processing of the screen touch needs to happen.
					if (((unsavedSysData.systemMode == DU2_AUTOX_WAITFINISH) || (unsavedSysData.systemMode == DU2_HILLCLIMB_WAITFINISH))
							&& (selectedRaceMode != DRAG_MODE))		// KMC 2/11/12 - Do not end run in DRAG_MODE
					{	// Touching the screen in autocross mode is for setting the finish line only when the finish line is not set
						// Set the finish line and end the run
						FinishPressed();
						// Stay on this screen until the user exits manually
						return;
					}

					// Otherwise, process normal back action
					if (TouchPosIsBackKey2(lastx, lasty))
					{
						MoveRight(ONE_SCREEN);
						lastx = lasty = 0;
					}
				}
				// START SLIDEOUT PROCESS!
				else if (slideoutState == SO_WAITING)
				{
#ifdef SUPPORT_AUDIO
					if (lastx < SLIDEOUT_CLOSE_REGION)
#else
					if ((lastx < SLIDEOUT_CLOSE_REGION) || (lasty > SLIDEOUT_CLOSE_REGION_Y))
#endif
					{
						// Pressed outside slideout - fast close slideout
						WM_RestartTimer(hSlideoutTimer, slideoutStepTime = SLIDEOUT_STEP_FAST_TIME);
						slideoutStepSize = SLIDEOUT_STEP_FAST_SIZE;
						CloseSlider();
					}
					else ProcessSlideoutKeyPress(lastx, lasty);
				}
				else if ((lastx > SLIDEOUT_OPEN_REGION_X) && (lasty < SLIDEOUT_OPEN_REGION_Y))
				{
					// Process Slideout
					if (slideoutState == SO_CLOSED)
						StartSlideout();
				}
				// END SLIDEOUT PROCESS!

				else if (TouchPosIsBackKey2(lastx, lasty))
				{
					if (slideoutState != SO_CLOSED)
						DeleteSlideout();

					if (configureGaugeMode != true)
					{
						MoveRight(ONE_SCREEN);
						lastx = lasty = 0;
					}
				}
				else if (lasty < 110)
				{
					if (slideoutState != SO_CLOSED)
						DeleteSlideout();
					nextScreen = GetPrevGaugeScreen();
					RunningGauges(nextScreen, GetRunningGaugesWindowHandle());
					lastx = lasty = 0;
				}
				else if (lasty > 162)
				{
					int screenID;

					if (slideoutState != SO_CLOSED)
						DeleteSlideout();
					if ((screenID = GetCurrentScreenID()) == DRIVE_MODE_SCREEN)		// Watch out for buttons on DriveMode Screen
					{
						if ((lasty > 225) && (lastx > 145))				// These are buttons - leave them alone (give 5 pixel margin)
							goto skip;
					}
					else if (screenID == GMETER)					// Process RESET function before screen scrolling on the GMETER screen
					{
						if (TouchPosIsReset(lastx, lasty))
						{
							ResetMax();
							goto skip;
						}
					}

					nextScreen = GetNextGaugeScreen();
					RunningGauges(nextScreen, GetRunningGaugesWindowHandle());
					lastx = lasty = 0;
				}
				else
				{
					if (slideoutState != SO_CLOSED)
						DeleteSlideout();
				}
				// Else Drive Mode - see if G-Meter Screen
				if (GetCurrentScreenID() == GMETER)
				{
					if (TouchPosIsReset(lastx, lasty))
						ResetMax();
				}
			}
			else
			{
				if ((lastx > 0) && (lastx < INFO_SIZE_X) && (lasty > 0) && (lasty < INFO_SIZE_Y))
					CheckForInfoKey(false);
				else if (TouchPosIsBackKey2(lastx, lasty))
				{
					if (slideoutState != SO_CLOSED)
						DeleteSlideout();
// hitting navigation key = back
					MoveRight(ONE_SCREEN);
					lastx = lasty = 0;
				}
			}
		}
skip:
	lastx = lasty = 0;
	}
	else
	{
		if ((lastx > 0) && (lastx < INFO_SIZE_X) && (lasty > 0) && (lasty < INFO_SIZE_Y))
		{
			CheckForInfoKey(false);
			lastx = lasty = 0;
		}
		else if (TouchPosIsBackKey2(lastx, lasty))
		{
// hitting navigation key = back
			MoveRight(ONE_SCREEN);
			lastx = lasty = 0;
		}
		else
		{
			lastx = lasty = 0;
			previous_touch_state = 0;
		}
	}

	//
	// If currently racing check to see if the SU has timed out of record mode (timeout after stop)
	// If timedout perform GUI exit race mode processing.
	//
	if (raceModeState == RM_RACING)
	{
		if (CheckForStopRecord())
			MoveRight(ONE_SCREEN);
	}
}


// KMC TODO - note that this needs to be added to sysData struct.  When we do that there will be a change of
// file format for the config file.  So hold off adding this until we're really ready to add audio!
// In the meantime here's a working variable to we can see the control bahave correctly.
#ifdef SUPPORT_AUDIO
int audioMuted = 0;
#endif

#define	NONE					0
#define	VOLUME_DOWN				1	
#define	MUTE					2
#define	VOLUME_UP				3
#define	SHIFT_LIGHTS_DIMMER		4
#define	BLANK_1					5
#define	SHIFT_LIGHTS_BRIGHTER	6
#define	BACKLIGHT_DIMMER		7
#define	BLANK_2					8
#define	BACKLIGHT_BRIGHTER		9

#define	BOTTOM_COLUMN	408
#define	MIDDLE_COLUMN	339
#define	TOP_COLUMN		264

#ifdef SUPPORT_AUDIO
#define	MIDDLE_ROW	178
#define	TOP_ROW		92
#else
#define	MIDDLE_ROW	110
#define	TOP_ROW		36
#endif

void ProcessSlideoutKeyPress(int lastx, int lasty)
{
	int key = NONE;

	// Determine key press
#ifdef SUPPORT_AUDIO
	if (lastx >= BOTTOM_COLUMN)
	{
		if (lasty >= MIDDLE_ROW)
			key = BACKLIGHT_BRIGHTER;
		else if (lasty >= TOP_ROW)
			key = SHIFT_LIGHTS_BRIGHTER;
		else key = VOLUME_UP;
	}
	else if (lastx >= MIDDLE_COLUMN)
	{
		if (lasty >= MIDDLE_ROW)
			key = BLANK_2;
		else if (lasty >= TOP_ROW)
			key = BLANK_1;
		else key = MUTE;
	}
	else if (lastx >= TOP_COLUMN)
	{
		if (lasty >= MIDDLE_ROW)
			key = BACKLIGHT_DIMMER;
		else if (lasty >= TOP_ROW)
			key = SHIFT_LIGHTS_DIMMER;
		else key = VOLUME_DOWN;
	}
#else
	if (lastx >= BOTTOM_COLUMN)
	{
		if (lasty >= MIDDLE_ROW)
			key = BACKLIGHT_BRIGHTER;
		else if (lasty >= TOP_ROW)
			key = SHIFT_LIGHTS_BRIGHTER;
	}
	else if (lastx >= MIDDLE_COLUMN)
	{
		if (lasty >= MIDDLE_ROW)
			key = BLANK_2;
		else if (lasty >= TOP_ROW)
			key = BLANK_1;
	}
	else if (lastx >= TOP_COLUMN)
	{
		if (lasty >= MIDDLE_ROW)
			key = BACKLIGHT_DIMMER;
		else if (lasty >= TOP_ROW)
			key = SHIFT_LIGHTS_DIMMER;
	}
#endif

	// Process key (note: this could be combined with above but I wanted to make it clean and easy to read (you're welcome)
	switch (key)
	{
#ifdef SUPPORT_AUDIO
	case MUTE:
		ResumeLEDUpdates();					// Resume RPM and warning control of the LEDs and reset autoDimming algorithm
		if (audioMuted)
		{
			audioMuted = false;
			GUI_SPRITE_SetBitmap(hSlideout, &bmSlideOut);
		}
		else
		{
			audioMuted = true;
			GUI_SPRITE_SetBitmap(hSlideout, &bmSlideOutMuted);
		}
		break;
	case VOLUME_DOWN:
	case VOLUME_UP:
		ResumeLEDUpdates();						// Resume RPM and warning control of the LEDs and reset autoDimming algorithm
		break;
#endif

	case SHIFT_LIGHTS_DIMMER:
		SuspendLEDUpdates(true);				// Suspend RPM and warning control of the LEDs and turn ON
		DecreaseLEDBrightness();
		break;

	case SHIFT_LIGHTS_BRIGHTER:
		SuspendLEDUpdates(true);				// Suspend RPM and warning control of the LEDs and turn ON
		IncreaseLEDBrightness();
		break;

	case BACKLIGHT_DIMMER:
		ResumeLEDUpdates();						// Resume RPM and warning control of the LEDs and reset autoDimming algorithm
		DecreaseBacklightBrightness();
		break;

	case BACKLIGHT_BRIGHTER:
		ResumeLEDUpdates();						// Resume RPM and warning control of the LEDs and reset autoDimming algorithm
		IncreaseBacklightBrightness();
		break;

	default:
	case NONE:
	case BLANK_1:
	case BLANK_2:
		break;
	}

	WM_RestartTimer(hSlideoutTimer, SLIDEOUT_OPEN_TIME);
}

void StartSlideout(void)
{
	const GUI_BITMAP *pGraphic;

	slideoutState = SO_OPENING;

	GUI_GetOrg(&slideoutXPos, &slideoutYPos);
#ifdef SUPPORT_AUDIO
	if (audioMuted)
		pGraphic = &bmSlideOutMuted;
	else pGraphic = &bmSlideOut;
#else
	pGraphic = &bmNoAudioSlideout;
#endif

	slideoutXPos = XSIZE_PHYS;
	// Top most sprites are the FIRST ones created (even though the manual says the opposite!)
	// So create icons first
	hSUStatusSprite = GUI_SPRITE_Create(GetSUStatusIcon(), slideoutXPos + SU_SPRITE_ICON_X_OFFSET, slideoutYPos);
	hSignalStrengthSprite = GUI_SPRITE_Create(SlideoutGetSignalStrengthIcon(), slideoutXPos + SIGSTRN_SPRITE_ICON_X_OFFSET, slideoutYPos);
	hCameraSprite = GUI_SPRITE_Create(GetCameraIcon(), slideoutXPos + CAMERA_SPRITE_ICON_X_OFFSET, slideoutYPos);

	hSlideout = GUI_SPRITE_Create(pGraphic, slideoutXPos, slideoutYPos);

	hSlideoutTimer = WM_CreateTimer(GetRunningGaugesWindowHandle(), 0, slideoutStepTime = SLIDEOUT_STEP_FAST_TIME, 0);
	slideoutStepSize = SLIDEOUT_STEP_FAST_SIZE;
}

void CloseSlider(void)
{
	slideoutState = SO_CLOSING;
	ResumeLEDUpdates();					// Resume RPM and warning control of the LEDs and reset autoDimming algorithm
	ResumeEnvironmentalUpdates();
}

void DeleteSlideout(void)
{
	slideoutXPos = XSIZE_PHYS;
	slideoutState = SO_CLOSED;
	GUI_SPRITE_Delete(hSlideout);
	if (hCameraSprite)
	{
		GUI_SPRITE_Delete(hSUStatusSprite);
		GUI_SPRITE_Delete(hCameraSprite);
		GUI_SPRITE_Delete(hSignalStrengthSprite);
	}
	hSlideout = hCameraSprite = hSUStatusSprite = hSignalStrengthSprite = 0;
	// If not being called through the timer functional allow the timer to get deleted when the window is changed
}

char CheckSlideoutTimer(WM_HWIN hWin)
{
	if (hWin == hSlideoutTimer)
	{
		switch (slideoutState)
		{
		case SO_OPENING:
			if (slideoutXPos > SPRITE_END_POS)
				slideoutXPos -= SLIDEOUT_STEP_FAST_SIZE;
			if (slideoutXPos <= SPRITE_END_POS)
			{
				// Slideout now fully open!
				slideoutXPos = SPRITE_END_POS;
				slideoutState = SO_WAITING;
				SetSpritePositions();
				WM_RestartTimer(hSlideoutTimer, SLIDEOUT_OPEN_TIME);
				SuspendEnvironmentalUpdates();
			}
			else
			{
				SetSpritePositions();
				WM_RestartTimer(hSlideoutTimer, SLIDEOUT_STEP_FAST_TIME);
			}
			break;

		case SO_CLOSING:
			if (slideoutXPos < XSIZE_PHYS)
				slideoutXPos += slideoutStepSize;
			if (slideoutXPos >= XSIZE_PHYS)
			{
				DeleteSlideout();
				GUI_TIMER_Delete(hSlideoutTimer);
			}
			else
			{
				SetSpritePositions();
				WM_RestartTimer(hSlideoutTimer, slideoutStepTime);
			}
			break;

		case SO_WAITING:
			// Keypress timeout, start slow autoclose
			WM_RestartTimer(hSlideoutTimer, slideoutStepTime = SLIDEOUT_STEP_SLOW_TIME);
			slideoutStepSize = SLIDEOUT_STEP_SLOW_SIZE;
			CloseSlider();
			break;
		}
		return true;
	}
	else return false;
}

void SetSpritePositions(void)
{
	GUI_SPRITE_SetPosition(hSlideout, slideoutXPos, slideoutYPos);
	GUI_SPRITE_SetPosition(hSUStatusSprite, slideoutXPos + SU_SPRITE_ICON_X_OFFSET, slideoutYPos);
	GUI_SPRITE_SetPosition(hCameraSprite, slideoutXPos + CAMERA_SPRITE_ICON_X_OFFSET, slideoutYPos);
	GUI_SPRITE_SetPosition(hSignalStrengthSprite, slideoutXPos + SIGSTRN_SPRITE_ICON_X_OFFSET, slideoutYPos);
}

//
// Check status of slideout.  If slideout is fully extended return true, else return false.
// Used by Drive screen to see if it is safe to process the buttons.
//
char SlideOutDeployed(void)
{
	if (slideoutState != SO_CLOSED)
		return true;
	else return false;
}

//
// Check status of slideout.  If slideout is visible (including sliding) return true, else return false.
//
char SlideOutVisible(void)
{
	if (slideoutState == SO_WAITING)
		return true;
	else return false;
}


//
// Return true if the current touch position is the BACK navigation arrow key position
//
char TouchPosIsBackKey(void)
{
	GUI_PID_STATE TouchState;

	GUI_TOUCH_GetState(&TouchState);		// Get the touch position in pixel
	return TouchPosIsBackKey2(TouchState.x, TouchState.y);
}

char TouchPosIsBackKey2(int lastx, int lasty)
{
	if ((lastx < 0) || (lasty < 0))
		return false;
	if ((lastx < NAVI_SIZE_X) && (lasty > (YSIZE_PHYS - NAVI_SIZE_Y)))
		return true;
	else return false;
}

//
// Return true if current touch position is the RESET function on the G-meter screen
//
int TouchPosIsReset(int lastx, int lasty)
{
	if ((lastx > 100) && (lastx < 300) && (lasty > 100) && (lasty < 200))
		return true;
	else return false;
}

void StartBackKeyTimer(void)
{
	backKeyPressedTime = GUI_GetTime();
}

char BackTimerExpired(void)
{
	int backKeyReleasedTime = GUI_GetTime();

	if (backKeyPressedTime > 0)
	{
		if ((backKeyReleasedTime - backKeyPressedTime) > TWO_SECONDS)
		{
			backKeyPressedTime = 0;
			return true;
		}
	}
	return false;
}

void StartConfigKeyTimer(void)
{
	if (slideoutState == SO_CLOSED)		// only active when slideout closed
		if (AnyNonDragGaugeScreen(GetScreenType()))
			if ((selectedRaceMode == ROADRACE_MODE) || (selectedRaceMode == INSTRUMENTS_MODE) || (selectedRaceMode == DRIVE_MODE))
			{
				int screenID;

				screenID = GetCurrentScreenID();
				if (ScreenIsConfigurable(screenID))
				{
					configKeyPressedTime = GUI_GetTime();
					return;
				}
			}

	// Otherwise be sure "timer is off"
	configKeyPressedTime = 0;
}

char ConfigTimerExpired(void)
{
	if (configKeyPressedTime > 0)
	{
		int configKeyReleasedTime = GUI_GetTime();

		if ((configKeyReleasedTime - configKeyPressedTime) > TWO_SECONDS)
		{
			configKeyPressedTime = 0;
			return true;
		}
	}
	return false;
}

const char *pHelpText, *pHelpTitle;

int CheckForInfoKey(int hasHelp)
{
	int currentScreen;
	char handleSprites = false;

	currentScreen = GetCurrentScreen();
	pHelpScreens = (HELP_SCREENS *)-1;	// Initially assume no tutorial topic AT ALL for this screen
	pHelpText = 0;						// and no help text

#define ONE_K	(1024)
#define ONE_MEG	(ONE_K * 1024)
#define ONE_GIG (ONE_MEG * 1024)
	
	switch (GetScreenType())
	{
	case GENERAL_CONFIG_SCREEN_TYPE:
		pHelpTitle = "Main Menu";
		pHelpText = GetSystemStatsHelpText(MAIN_SCREEN_INFO);
		pHelpScreens = 0;			// NULL pHelpScreens means no specific help tutorial is known - show list of topics
		if (hasHelp)
			return true;
		break;

	case CONFIG_SCREEN_TYPE:
		switch (currentScreen)
		{
		case GO_RACING:
			pHelpTitle = "Go Racing Info";
			pHelpText = GO_RACING_INFO;
			pHelpScreens = GoRacingHelpScreens;
			if (hasHelp)
				return true;
			break;
		case RACE_SETUP:
			pHelpTitle = "Race Setup Info";
			pHelpText = RACE_SETUP_INFO;
			if (hasHelp)
				return true;
			break;
		case REVIEW_SESSIONS:
			pHelpTitle = "Review Sessions Info";
			pHelpText = REVIEW_SESSIONS_INFO;
// sample	pHelpScreens = (HELP_SCREENS *)-1;			// -1 means there is no tutorial topic AT ALL for this level
			pHelpScreens = ReviewSessionsHelpScreens;
			if (hasHelp)
				return true;
			break;
		case SYSTEM_SETUP:
			pHelpTitle = "System Setup Info";
			pHelpText = SYSTEMS_SETUP_INFO;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case RACE_SETUP_SCREENS_TYPE:
		switch (currentScreen)
		{
		case DRIVER_SELECTION:
			handleSprites = true;		// This screen uses sprites for shadow effect on the list - hide them during the popup
			pHelpTitle = "Driver Selection Info";
			pHelpText = SELECT_DRIVER_INFO;
			pHelpScreens = (HELP_SCREENS *)-1;			// -1 means there is no tutorial topic AT ALL for this level
			if (hasHelp)
				return true;
			break;

		case VEHICLE_SELECTION:
			handleSprites = true;		// This screen uses sprites for shadow effect on the list - hide them during the popup
			pHelpTitle = "Vehicle Selection Info";
			pHelpText = SELECT_VEHICLE_INFO;
			pHelpScreens = VehicleSelectionHelpScreens;
			if (hasHelp)
				return true;
			break;

		case TRACK_SELECTION:
			handleSprites = true;		// This screen uses sprites for shadow effect on the list - hide them during the popup
			pHelpTitle = "Track Selection Info";
			pHelpText = SELECT_TRACK_INFO;
			pHelpScreens = TrackSettingsSpecificHelpScreens;
			if (hasHelp)
				return true;
			break;

		case LAPSQUALIFYING_SELECTION:
			pHelpTitle = "Laps/Qualifying Info";
			pHelpText = LAP_QUAL_INFO;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case SYSTEM_SETTINGS_SCREENS_TYPE:
		switch (currentScreen)
		{
		case DISPLAY_SELECTION:
			pHelpTitle = "Display Settings Info";
			pHelpText = DISPLAY_SETTINGS_INFO;
			if (hasHelp)
				return true;
			break;

		case UNITS_SELECTION:
			pHelpTitle = "Units Selection Info";
			pHelpText = UNITS_SETTINGS_INFO;
			if (hasHelp)
				return true;
			break;

		case TIME_SELECTION:
			pHelpTitle = "Time Zone Info";
			pHelpText = TIME_SETTINGS_INFO;
			pHelpScreens = TimeZoneGPSScreens;
			if (hasHelp)
				return true;
			break;

		case AUTOSTART_SELECTION:
			pHelpScreens = AutoOnSpecificHelpScreens;
			pHelpTitle = "Auto Start Info";
			pHelpText = AUTOSTART_SETTINGS_INFO;
			if (hasHelp)
				return true;
			break;

		case STORAGE_SELECTION:
			pHelpTitle = "System Storage Info";
			pHelpText = GetStorageStatsHelpText(STORAGE_SETTINGS_INFO);
			if (hasHelp)
				return true;
			break;

		case LAP_METER:
			//pHelpScreens = PredictiveLapHelpScreens;
			pHelpTitle = "Predictive Scale Info";
			pHelpText = PREDICTIVE_SCALE_INFO;
			if (hasHelp)
				return true;
			break;

		}
		break;
		
	case VEHICLE_SETTING_SCREEN_TYPE:
		pHelpTitle = "Vehicle Settings Info";
		pHelpText = VEHICLE_SETTINGS_INFO;
		pHelpScreens = VehicleSetupSpecificHelpScreens;
		if (hasHelp)
			return true;
		break;

	case AERO_GRIP_SCREEN_TYPE:
		pHelpTitle = "Aero/Grip Info";
		pHelpText = AERO_GRIP_INFO;
		if (hasHelp)
			return true;
		break;

	case VEHICLE_SETTINGS_SCREENS_TYPE:
		switch (currentScreen)
		{
		case TACH_SELECTION:
			pHelpTitle = "Tach Settings Info";
			pHelpText = TACH_SETTINGS_INFO;
			pHelpScreens = TachSpecificHelpScreens;
			if (hasHelp)
				return true;
			break;

		case CARMETRICS_SELECTION:
			pHelpTitle = "Car Metrics Info";
			pHelpText = CAR_METRICS_INFO;
			if (hasHelp)
				return true;
			break;

		case GEARRATIO_SELECTION:
			pHelpTitle = "Gear Ratio Info";
			pHelpText = GEAR_RATIO_INFO;
			if (hasHelp)
				return true;
			break;

		case INPUTS_SELECTION:
			pHelpTitle = "Inputs Info";
			pHelpText = INPUTS_INFO;
			pHelpScreens = AnalogInputsHelpScreens;
			if (hasHelp)
				return true;
			break;

		case WEIGHT_SELECTION:
			pHelpTitle = "Vehicle Weight Info";
			pHelpText = WEIGHT_INFO;
			if (hasHelp)
				return true;
			break;

		case CAMERA_SETUP_SELECTION:
			pHelpTitle = "Camera Setup Info";
			pHelpText = CAMERA_SETUP_INFO;
			pHelpScreens = GenericCameraHelpScreens;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case TACH_SETUP_SCREENS_TYPE:
		switch (currentScreen)
		{
		case WARNING_RPM_SETUP:
			pHelpTitle = "Warning RPM Info";
			pHelpText = TACH_WARNING_INFO;
			if (hasHelp)
				return true;
			break;

		case REDLINE_RPM_SETUP:
			pHelpTitle = "Redline RPM Info";
			pHelpText = TACH_REDLINE_INFO;
			if (hasHelp)
				return true;
			break;

		case ENGINE_CYLINDERS_SETUP:
			pHelpTitle = "Engine Cyl RPM Info";
			pHelpText = NUM_CYL_INFO;
			if (hasHelp)
				return true;
			break;

		case POWERBANDEND_RPM_SETUP:
			pHelpTitle = "Powerband RPM Info";
			pHelpText = TACH_PWREND_INFO;
			if (hasHelp)
				return true;
			break;

		case MAX_RPM_SETUP:
			pHelpTitle = "Max RPM Info";
			pHelpText = TACH_MAXRPM_INFO;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case INPUTS_SCREENS_TYPE:
		switch (currentScreen)
		{
		case ANALOG0:
		case ANALOG1:
		case ANALOG2:
		case ANALOG3:
			pHelpTitle = "Analog Input Info";
			pHelpText = ANALOG_SETTINGS_INFO;
			pHelpScreens = AnalogInputsHelpScreens;
			if (hasHelp)
				return true;
			break;

		case DIGITAL4:
		case DIGITAL5:
			pHelpTitle = "Digital Input Info";
			pHelpText = DIGITAL_SETTINGS_INFO;
			pHelpScreens = AnalogInputsHelpScreens;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case REVIEW_SESSIONS_SCREENS_TYPE:
		pHelpScreens = ReviewSessionsHelpScreens;
		if (currentScreen == CHECK_TIMES_SCREEN)
		{
			pHelpTitle = "Check Times Info";
			pHelpText = SESSION_DETAILS_INFO;
			if (hasHelp)
				return true;
		}
		break;

	case TRACK_SETTING_SCREEN_TYPE:
		pHelpTitle = "Track Settings Info";
		pHelpText = TRACK_SETTINGS_INFO;
		if (hasHelp)
			return true;
		break;

	case TRACK_SETTINGS_SCREENS_TYPE:
		switch (currentScreen)
		{
		case TRACKTYPE_SELECTION:
			pHelpTitle = "Track Type Info";
			pHelpText = TRACK_TYPE_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		case HOLDTIME_SELECTION:
			pHelpTitle = "Hold Time Info";
			pHelpText = LAP_HOLD_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		case STARTFINISH_SELECTION:
			pHelpTitle = "Start/Finish Line Info";
			pHelpText = START_FINISH_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		case RESETPL_SELECTION:
			pHelpTitle = "Predictive Lap Info";
			pHelpText = PREDICTIVE_LAP_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case DRIVE_MODE_SCREEN_TYPE:
	case INSTRUMENTS_MODE_SCREEN_TYPE:
	case GAUGES_SCREEN_TYPE:
		break;

	case GEARING_SCREENS_TYPE:
		switch (currentScreen)
		{
		case TIRESIZE_SELECTION_SCREEN:
			pHelpTitle = "Tire Size Selection Info";
			pHelpText = TIRE_SIZE_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		case DIFFRATIO_SCREEN:
			pHelpTitle = "Diff Ratio Info";
			pHelpText = DIFF_RATIO_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		case TRANSRATIO_SCREEN:
			pHelpTitle = "Transmission Ratios Info";
			pHelpText = TRANS_RATIO_INFO;
			pHelpScreens = MainHelpScreens;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case GEAR_RATIOS:
		break;
	case SFSCREENS_TYPE:
		break;

	case ANALOG_SETTINGS_SCREEN_TYPE:
		switch (currentScreen)
		{
		case SCALE:
			pHelpTitle = "Analog Scale Info";
			pHelpText = ANALOG_SCALE_INFO;
			pHelpScreens = EditScaleHelpScreens;
			if (hasHelp)
				return true;
			break;

		case ALARM:
			pHelpTitle = "Analog Alarm Info";
			pHelpText = ANALOG_ALARM_INFO;
			pHelpScreens = AnalogInputsHelpScreens;
			if (hasHelp)
				return true;
			break;

		}
		break;

	case ANALOG_INPUTS_SCALE:
		break;

	case DIGITAL_SETTINGS_SCREEN_TYPE:
		if (currentScreen == DIG_ALARM_STATE)
		{
			pHelpTitle = "Digital Alarm Info";
			pHelpText = DIGITAL_ALARM_INFO;
			if (hasHelp)
				return true;
		}
		break;

	case TIRE_DIMENSIONS_SCREEN_TYPE:
		pHelpTitle = "Tire Size Info";
		pHelpText = TIRE_DIMENSIONS_INFO;
		if (hasHelp)
			return true;
		break;

	case DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE:
		handleSprites = true;		// This screen uses sprites for shadow effect on the list - hide them during the popup
		pHelpTitle = "Choose Track Info";
		pHelpText = CHOOSE_TRACK_INFO;
		if (hasHelp)
			return true;
		break;

	case PERSONALIZE_REVIEW_SCREEN_TYPE:
		pHelpTitle = "Personalize Unit Info";
		pHelpText = PERSONALIZE_INFO;
		if (hasHelp)
			return true;
		break;

	case WAITING_FOR_STAGE_SCREEN_TYPE:
		// No need for hasHelp test because the normal screen banner is not displayed and so that test will not be performed
		if (sysData.selectedSessionType == AUTOCROSS_TYPE)
		{
			pHelpTitle = "Autocross Stage Info";
			pHelpText = AUTOX_STAGE_SCREEN_INFO;
		}
		else
		{
			pHelpTitle = "Drag Stage Info";
			pHelpText = DRAG_STAGE_SCREEN_INFO;
		}
		break;

	case CAMERA_DELAY_SCREEN_TYPE:
		pHelpScreens = SUSpecificCameraHelpScreens;
		pHelpTitle = "Camera Delay Info";
		pHelpText = CAMERA_DELAY_INFO;
		if (hasHelp)
			return true;
		break;

	case TRAQDASH_CAMERA_SCREEN:
		pHelpScreens = DUSpecificCameraHelpScreens;
		pHelpTitle = "Camera Delay Info";
		pHelpText = CAMERA_DELAY_INFO;
		if (hasHelp)
			return true;
		break;

	case CAMERA_SELECTION_SCREEN_TYPE:
		pHelpScreens = SUSpecificCameraHelpScreens;
		pHelpTitle = "Camera Selection Info";
		pHelpText = CAMERA_INFO;
		if (hasHelp)
			return true;
		break;

	case RECORD_SPEED_SCREEN_TYPE:
		pHelpTitle = "Record Speed Info";
		pHelpText = RECORD_SPEED_INFO;
		if (hasHelp)
			return true;
		break;
	}

	if (pHelpText)
	{
		if (handleSprites)
			HideSprites();					// Take sprites away before showing help popup
		ShowHelpPopup( pHelpScreens == (HELP_SCREENS *)-1 ? false : true);
		if (handleSprites)
			UnhideSprites();				// Put sprites back when popup goes away
	}
	
	return false;
}

#define	POPUP_WIDTH		440
#define	POPUP_HEIGHT	262
#define	POPUP_OFFSET	10

#define	TITLE	GUI_ID_TEXT0

//#define	POPUP_WIDTH		XSIZE_PHYS
//#define	POPUP_HEIGHT	YSIZE_PHYS

static const GUI_WIDGET_CREATE_INFO HelpPopup[] = {
	{ FRAMEWIN_CreateIndirect, 0, 0, ((XSIZE_PHYS - POPUP_WIDTH) / 2) - POPUP_OFFSET, ((YSIZE_PHYS - POPUP_HEIGHT) / 2) - 5, POPUP_WIDTH, POPUP_HEIGHT, 0, 0 },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, (POPUP_WIDTH / 2) - 50, POPUP_HEIGHT - 82, 100, 45 },
};

static const GUI_WIDGET_CREATE_INFO HelpWithDetailsPopup[] = {
	{ FRAMEWIN_CreateIndirect, 0, 0, ((XSIZE_PHYS - POPUP_WIDTH) / 2) - POPUP_OFFSET, ((YSIZE_PHYS - POPUP_HEIGHT) / 2) - 5, POPUP_WIDTH, POPUP_HEIGHT, 0, 0 },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 80, POPUP_HEIGHT - 82, 100, 45 },
	{ BUTTON_CreateIndirect, "Tutorial", GUI_ID_HELP, 230, POPUP_HEIGHT - 82, 100, 45 }
};

void ShowHelpPopup(int detailsAvail)
{
	SetPopupDefaults(INFORMATIONAL_POPUP);
	if (detailsAvail)
		GUI_ExecDialogBox(HelpWithDetailsPopup, GUI_COUNTOF(HelpWithDetailsPopup), &HelpCallback, GetCurrentWindowHandle(), POPUP_OFFSET, POPUP_OFFSET);
	else GUI_ExecDialogBox(HelpPopup, GUI_COUNTOF(HelpPopup), &HelpCallback, GetCurrentWindowHandle(), POPUP_OFFSET, POPUP_OFFSET);
}

void HelpCallback(WM_MESSAGE * pMsg)
{
	WM_HWIN hDlg;
	int Id;
	MULTIEDIT_HANDLE hText;

	hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_INIT_DIALOG:
			FRAMEWIN_SetTextColor(hDlg, popupTitleTextColor);
			FRAMEWIN_SetTextAlign(hDlg, GUI_TA_HCENTER);
			FRAMEWIN_SetText(hDlg, pHelpTitle);

			SCROLLBAR_SetDefaultWidth(40);

			hText = MULTIEDIT_CreateEx(10, 30, POPUP_WIDTH - 16, POPUP_HEIGHT - 95, hDlg, WM_CF_SHOW,
				MULTIEDIT_CF_AUTOSCROLLBAR_V | MULTIEDIT_CF_READONLY, 0, strlen(pHelpText), pHelpText);
			MULTIEDIT_SetBkColor(hText, MULTIEDIT_CI_READONLY, GUI_OFFWHITE);
			MULTIEDIT_SetFont(hText, &GUI_Font24_ASCII);
			MULTIEDIT_SetWrapWord(hText);
			break;

		case WM_NOTIFY_PARENT:
			if ((pMsg->Data.v == WM_NOTIFICATION_RELEASED) && ((Id = WM_GetId(pMsg->hWinSrc)) == GUI_ID_OK))
			{
				GUI_EndDialog(hDlg, Id);
				glbBlackoutFlag = TRANSITION;	// Set flag to allow a screen clear to remove the dialog box
			}
			else if ((pMsg->Data.v == WM_NOTIFICATION_RELEASED) && ((Id = WM_GetId(pMsg->hWinSrc)) == GUI_ID_HELP))
				CreateHelpSelection(hDlg);
			else WM_DefaultProc(pMsg);
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}


/*********************************************************************
*
*       MainTask
*/
void DisplayNewScreenList(enum SCREEN_TYPE screentype, int screen, WM_HWIN hPrev)
{
	WM_HWIN hChild;
	GUI_HWIN hNewWin;
	SCREENLIST *pScreenList;
	WM_CALLBACK *pCallBack;
	void (*pSetNewWin)(WM_HWIN);
	int timerRate;

	SetLastScreen(screentype, screen);
	timerRate = SETUP_SCREEN_REFRESH_TIME;

	switch (screentype)
	{
		default:
			SystemError();
			// fall through
		case CONFIG_SCREEN_TYPE:
			pScreenList = ConfigScreenList;
			pCallBack = _cbMainCallback;
			pSetNewWin = SetConfigWindowHandle;
			break;
		case DRIVE_MODE_SCREEN_TYPE:
			pScreenList = (SCREENLIST *)DriveScreenList;
			pCallBack = GaugesScreensCallback;
			pSetNewWin = SetRunningGaugesWindowHandle;
			break;
		case INSTRUMENTS_MODE_SCREEN_TYPE:
			pScreenList = (SCREENLIST *)InstrumentsScreenList;
			pCallBack = GaugesScreensCallback;
			pSetNewWin = SetRunningGaugesWindowHandle;
			break;
		case GAUGES_SCREEN_TYPE:
			pScreenList = (SCREENLIST *)GaugesScreenList;
			pCallBack = GaugesScreensCallback;
			pSetNewWin = SetRunningGaugesWindowHandle;
			break;
		case RACE_SETUP_SCREENS_TYPE:
			pScreenList = RaceSetupScreenList;
			pCallBack = RaceSetupScreensCallback;
			pSetNewWin = SetRaceSetupScreensWindowHandle;
			break;
		case SYSTEM_SETTINGS_SCREENS_TYPE:
			pScreenList = SystemSettingsScreenList;
			pCallBack = SystemSettingsScreensCallback;
			pSetNewWin = SetSystemSettingsWindowHandle;
			break;
		case VEHICLE_SETTINGS_SCREENS_TYPE:
			timerRate = TACH_SCREEN_REFRESH_TIME;
			pScreenList = VehicleSettingsScreenList;
			pCallBack = VehicleSettingsScreensCallback;
			pSetNewWin = SetVehicleSettingsWindowHandle;
			break;
		case TACH_SETUP_SCREENS_TYPE:
			pScreenList = TachSetupScreenList;
			pCallBack = TachSetupScreensCallback;
			pSetNewWin = SetTachSetupWindowHandle;
			break;
		case INPUTS_SCREENS_TYPE:
			pScreenList = InputsScreenList;
			pCallBack = InputsScreensCallback;
			pSetNewWin = SetInputsWindowHandle;
			break;
		case TRACK_SETTINGS_SCREENS_TYPE:
			pScreenList = TrackSettingsScreenList;
			pCallBack = TrackSettingsScreensCallback;
			pSetNewWin = SetTrackSettingsWindowHandle;
			break;
		case GEARING_SCREENS_TYPE:
			pScreenList = GearingScreenList;
			pCallBack = GearingScreensCallback;
			pSetNewWin = SetGearingWindowHandle;
			break;
		case GEAR_RATIOS:
			pScreenList = GearRatiosScreenList;
			pCallBack = GearRatiosCallback;
			pSetNewWin = SetGearRatiosWindowHandle;
			break;
		case ANALOG_SETTINGS_SCREEN_TYPE:
			pScreenList = AnalogInputSettingsScreenList;
			pCallBack = AnalogInputSettingsCallback;
			pSetNewWin = SetAnalogInputSettingsWindowHandle;
			break;
		case DIGITAL_SETTINGS_SCREEN_TYPE:
			pScreenList = DigitalInputSettingsScreenList;
			pCallBack = DigitalInputSettingsCallback;
			pSetNewWin = SetDigitalInputSettingsWindowHandle;
			break;
		case ANALOG_INPUTS_SCALE:
			pScreenList = AnalogInputsScaleScreenList;
			pCallBack = AnalogInputsScaleCallback;
			pSetNewWin = SetAnalogInputsScaleWindowHandle;
			break;
	}

	hNewWin = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, pCallBack, 0);
	if (hNewWin <= 0)
		SystemError();
	else pSetNewWin(hNewWin);

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

//	WM_EnableMemdev(hConfigScreen);
	WM_EnableMemdev(hNewWin);


	// Display requested screen at 0, 0
//	ConfigScreenList[screen].constructor(0, hConfigScreen);
	pScreenList[screen].constructor(hNewWin);


	if (hPrev)
	{
		GUI_RECT Rect = {0, 0, XSIZE_PHYS, YSIZE_PHYS};
		WM_ValidateRect(hNewWin, &Rect);
		hChild = WM_GetFirstChild(hNewWin);
		while (hChild)
		{
			WM_GetWindowRectEx(hChild, &Rect);
			WM_ValidateWindow(hChild);

			hChild = WM_GetNextSibling(hChild);
		}
   }

	WM_Exec();
	screen_scrolling = ACTIVE;

//	WM_InvalidateWindow(hConfigScreen);
//	hChild = WM_GetFirstChild(hConfigScreen);
	WM_InvalidateWindow(hNewWin);
	hChild = WM_GetFirstChild(hNewWin);
	while (hChild)
	{
		GUI_RECT Rect;

		WM_GetWindowRectEx(hChild, &Rect);
		WM_InvalidateWindow(hChild);

		hChild = WM_GetNextSibling(hChild);
	}

	WM_Exec();
	WM_DeleteWindow(hPrev);

	hScreenTimer = WM_CreateTimer(hNewWin, 0, timerRate, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}


void DisplayGoRacing(WM_HWIN hParent)
{
	BUTTON_Handle hButton;

	hButton = BUTTON_CreateEx(41, 38, 178, 115, hParent, WM_CF_SHOW, 0, ROADRACE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmRace);

	hButton = BUTTON_CreateEx(261, 38, 178, 115, hParent, WM_CF_SHOW, 0, AUTOXRALLY_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmAutoxRally);

	hButton = BUTTON_CreateEx(41, 156, 100, 115, hParent, WM_CF_SHOW, 0, DRIVE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmDrive);

	hButton = BUTTON_CreateEx(261, 156, 178, 115, hParent, WM_CF_SHOW, 0, DRAG_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmDragButton);

	hRecordModeButton = BUTTON_CreateEx(141, 156, 78, 115, hParent, WM_CF_SHOW, 0, RECORDMODE_BUTTON);
	SetRecordModeGraphic();
}

void SetRecordModeGraphic(void)
{
	if (sysData.disableDriveRecordMode)
		BUTTON_SetBitmap(hRecordModeButton, BUTTON_BI_UNPRESSED, &bmRecordingOff);
	else BUTTON_SetBitmap(hRecordModeButton, BUTTON_BI_UNPRESSED, &bmRecordingOn);
}


void DisplayRaceSetup(WM_HWIN hParent)
{
	BUTTON_Handle hButton;

	hButton = BUTTON_CreateEx(41, 38, 178, 115, hParent, WM_CF_SHOW, 0, DRIVER_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmDriver);

	hButton = BUTTON_CreateEx(261, 38, 178, 115, hParent, WM_CF_SHOW, 0, TRACK_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmTrack);

	hButton = BUTTON_CreateEx(41, 156, 178, 115, hParent, WM_CF_SHOW, 0, VEHICLE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmVehicle);

	hButton = BUTTON_CreateEx(261, 156, 178, 115, hParent, WM_CF_SHOW, 0, LAPSQUALIFYING_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmLapsQualifyButton);
}

#define	NEW_BUTON_HEIGHT	40

void DisplayReviewSessions(WM_HWIN hParent)
{
	HEADER_Handle hHeader;
	char scratch[50];

	int i;

	GetSessionNameData(hParent, 1, (enum SESSION_TYPE)sysData.selectedSessionType);

	hListView = LISTVIEW_CreateEx(10, 38, 460, 185, hParent, WM_CF_SHOW, 0, REVIEW_SESSIONS_LIST);
	WM_SetFocus(hListView);
	LISTVIEW_AddColumn(hListView, 185, "Date & Time", GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTVIEW_AddColumn(hListView, 80, "Session", GUI_TA_HCENTER | GUI_TA_VCENTER);
	strcpy(scratch, "Track / ");
	strcat(scratch, GetSelectedReviewModeText());
//	LISTVIEW_AddColumn(hListView, 195 - FINGER_WIDTH, "Track", GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTVIEW_AddColumn(hListView, 195 - FINGER_WIDTH, scratch, GUI_TA_HCENTER | GUI_TA_VCENTER);
	LISTVIEW_SetAutoScrollV(hListView, 1);
	LISTVIEW_SetFont(hListView, &GUI_Font20_ASCII);
	HEADER_SetFont(hHeader = LISTVIEW_GetHeader(hListView), &GUI_Font20_ASCII);
	HEADER_SetTextAlign(hHeader, 0, GUI_TA_HCENTER | GUI_TA_VCENTER);
	HEADER_SetTextAlign(hHeader, 2, GUI_TA_HCENTER | GUI_TA_VCENTER);
	LISTVIEW_SetRowHeight(hListView, FINGER_WIDTH - 5);

// KMC START
	// If we were reviewing the details of a session (ie got here via the BACK button) then go back to that session
	// otherwise find the newest session
	if (reviewingSession == false)
		selectedSessionUnsorted = FindNewestFile((enum SESSION_TYPE)sysData.selectedSessionType);
	else reviewingSession = false;

	i = PopulateSessionLISTVIEW(selectedSessionUnsorted);
	GetSessionData();
// KMC END

	HEADER_SetHeight(hHeader, FINGER_WIDTH);
	LISTVIEW_SetCompareFunc(hListView, 0, &DateTimeSort);
	LISTVIEW_SetCompareFunc(hListView, 1, &LapTimeSort);
	LISTVIEW_SetCompareFunc(hListView, 2, &LISTVIEW_CompareText);
	LISTVIEW_EnableSort(hListView);
	LISTVIEW_SetSort(hListView, 0, 0);			// Set sort to sort by decending date


	hBestTime = BUTTON_CreateEx( COL1,  BOTTOM_BUTTON_ROW, 167, NEW_BUTON_HEIGHT, hParent, WM_CF_SHOW, 0, BEST_LAP_BTN);
	BUTTON_SetBitmap(hBestTime, BUTTON_BI_UNPRESSED, &bmStdGreenButton);
	BUTTON_SetTextColor(hBestTime, BUTTON_BI_UNPRESSED, GUI_BLACK);
	//BUTTON_SetTextColor(hBestTime, BUTTON_BI_PRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBestTime, BUTTON_BI_DISABLED, GUI_BLACK);
	SetBestTimeButtonText();

	hSelectedReviewMode = BUTTON_CreateEx( COL2,  BOTTOM_BUTTON_ROW, NARROW_BUTTON_WIDTH, NEW_BUTON_HEIGHT, hParent, WM_CF_SHOW, 0, SESSION_TYPE_BTN);
	BUTTON_SetBitmap(hSelectedReviewMode, BUTTON_BI_UNPRESSED, &bmSmallYellowButton);
	BUTTON_SetTextColor(hSelectedReviewMode, BUTTON_BI_UNPRESSED, GUI_BLACK);
	//BUTTON_SetTextColor(hSelectedReviewMode, BUTTON_BI_PRESSED, GUI_BLACK);
	//BUTTON_SetTextColor(hSelectedReviewMode, BUTTON_BI_DISABLED, GUI_BLACK);
	//BUTTON_SetText(hSelectedReviewMode, GetSelectedReviewModeText());
	BUTTON_SetText(hSelectedReviewMode, "Mode");

	hDeleteButton = BUTTON_CreateEx( COL3,  BOTTOM_BUTTON_ROW, NARROW_BUTTON_WIDTH, NEW_BUTON_HEIGHT, hParent, WM_CF_SHOW, 0, DELETE_SESSION_BTN);
	BUTTON_SetBitmap(hDeleteButton, BUTTON_BI_UNPRESSED, &bmSmallRedButton);
	BUTTON_SetText(hDeleteButton, "Delete");

	hDetailsButton = BUTTON_CreateEx( COL4,  BOTTOM_BUTTON_ROW, NARROW_BUTTON_WIDTH, NEW_BUTON_HEIGHT, hParent, WM_CF_SHOW, 0, CHECK_TIMES_BTN);
	BUTTON_SetBitmap(hDetailsButton, BUTTON_BI_UNPRESSED, &bmSmallBlueButton);
	BUTTON_SetText(hDetailsButton, "Details");
	SetButtonColors(i);
}

void SetButtonColors(int i)
{
	if (i != 0)
	{
		BUTTON_SetTextColor(hDeleteButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hDetailsButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hDeleteButton, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hDetailsButton, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hDeleteButton, BUTTON_BI_DISABLED, GUI_BLACK);
		BUTTON_SetTextColor(hDetailsButton, BUTTON_BI_DISABLED, GUI_WHITE);
	}
	else
	{
		BUTTON_SetTextColor(hDeleteButton, BUTTON_BI_UNPRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hDetailsButton, BUTTON_BI_UNPRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hDeleteButton, BUTTON_BI_PRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hDetailsButton, BUTTON_BI_PRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hDeleteButton, BUTTON_BI_DISABLED, GUI_GRAY);
		BUTTON_SetTextColor(hDetailsButton, BUTTON_BI_DISABLED, GUI_GRAY);
	}
}

int PopulateSessionLISTVIEW(int selection)
{
	int i;
	char *pDest[3];
	char scratch[50];

	// First delete any existing rows in case this is a redisplay
	i = LISTVIEW_GetNumRows(hListView);
	while (i)
		LISTVIEW_DeleteRow(hListView, --i);

	i = 0;

	LISTVIEW_DeleteColumn(hListView, 2);			// Delete old Track/Session type column so I can create a new
	strcpy(scratch, "Track / ");					// one with the correct column heading (you can't just change
	strcat(scratch, GetSelectedReviewModeText());	// the column heading text!).
	LISTVIEW_AddColumn(hListView, 195 - FINGER_WIDTH, scratch, GUI_TA_HCENTER | GUI_TA_VCENTER);

	while (SessionNameData[i].date[0])
	{
		char dateAndTime[50], sessionNumber[5];

		strcpy(dateAndTime, SessionNameData[i].date);
		strcat(dateAndTime, " ");
		strcat(dateAndTime, SessionNameData[i].time);
		pDest[0] = dateAndTime;
		
		sprintf(sessionNumber, "%d", SessionNameData[i].sessionNumber);
		pDest[1] = sessionNumber;

		pDest[2] = SessionNameData[i].track;

		LISTVIEW_AddRow(hListView, pDest);
		i++;
	}

	// Scrollbar width must be set after the data is in the control
	SCROLLBAR_SetWidth(WM_GetScrollbarV(hListView), FINGER_WIDTH);
	if ((i != 0) && (selection >= 0)) {	// Only select an entry if there are items to select (otherwise call fails!) and it has been requested
		LISTVIEW_SetSelUnsorted(hListView, selectedSessionUnsorted);
		LISTVIEW_SetBkColor(hListView, LISTVIEW_CI_SELFOCUS, GUI_LISTBLUE);
	}
	return numSessions = i;
}

int DateTimeSort(void const *p1, void const *p2)
{
	int retval;
	char date1[12], date2[12];

	strncpy(date1, p1, 11);		// Get the dates only
	date1[10] = 0;				// null terminate
	strncpy(date2, p2, 11);
	date2[10] = 0;				// null terminate

	if ((retval = strcmp(date2, date1)) == 0)
	{
		// Days equal, check the time of day
		int mins1, mins2;

		mins1 = GetMins(p1);
		mins2 = GetMins(p2);

		if (mins1 > mins2)
			return -1;
		else if (mins2 > mins1)
			return 1;
		else return 0;
	}
	else return retval;
}

int GetMins(const char *pData)
{
	char ch;
	char foundHours = 0;
	char hours = 0;
	char mins = 0;

	pData += 11;				// Index past date field
	while ((ch = *pData++) != 0)
	{
		if ((ch >= '0') && (ch <= '9'))
		{
			if (!foundHours)
			{
				if (hours)
					hours = (hours * 10) + (ch - '0');
				else hours = ch - '0';
			}
			else {
				if (mins)
					mins = (mins * 10) + (ch - '0');
				else mins = ch - '0';
			}
		}
		else if (ch == ':')
			foundHours = true;
		else if (tolower(ch) == 'a')
		{
			if (hours == 12)
				hours = 0;
		}
		else if (tolower(ch) == 'p')
		{
			if (hours != 12)
				hours += 12;
		}
	}
	return (hours * 60) + mins;
}

int LapTimeSort(void const *p1, void const *p2)
{
	return strcmp(p2, p1);
}

void DisplaySystemSettings(WM_HWIN hParent)
{
	BUTTON_Handle hButton;

	hButton = BUTTON_CreateEx(30, 38, 127, 115, hParent, WM_CF_SHOW, 0, DISPLAY_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmDisplay);

	hButton = BUTTON_CreateEx(177, 38, 127, 115, hParent, WM_CF_SHOW, 0, UNITS_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmUnits);

	hButton = BUTTON_CreateEx(324, 38, 127, 115, hParent, WM_CF_SHOW, 0, TIME_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmTimeGPS);

	hButton = BUTTON_CreateEx(30, 156, 127, 115, hParent, WM_CF_SHOW, 0, AUTOSTART_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmAutoStartButton);

	hButton = BUTTON_CreateEx(177, 156, 127, 115, hParent, WM_CF_SHOW, 0, STORAGE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmStorage);

	hButton = BUTTON_CreateEx(324, 156, 127, 115, hParent, WM_CF_SHOW, 0, PREDICTIVE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmPredictive);
}

void GoRacingPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmGoRacingText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(GO_RACING), 0, 229);
}


void RaceSetupPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmRaceSetupText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(RACE_SETUP), 0, 229);
}


void ReviewSessionsPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
//	GUI_DrawBitmap(&bmSingleSpinner, 16, 42);
	GUI_DrawBitmap(&bmRaceReviewText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(REVIEW_SESSIONS), 0, 229);

	// draw remaining session notes
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	////// Highlight details button
	////GUI_SetColor(GUI_MEDGREEN);
	////GUI_DrawRect(COL4 - 3, BOTTOM_BUTTON_ROW - 2, COL4 + NARROW_BUTTON_WIDTH + 2, BOTTOM_BUTTON_ROW + BUTTON_HEIGHT + 2);
}

void SystemSettingsPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmSystemSettingsText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(SYSTEM_SETUP), 0, 229);
}

//int OwnerDraw(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
//	switch (pDrawItemInfo->Cmd) {
//		case WIDGET_DRAW_OVERLAY:
//			GUI_SetColor(GUI_RED);
//			GUI_DrawHLine(67, 0, 180);
//			GUI_DrawHLine(103, 0, 180);
//			break;
//		default:
//	return LISTWHEEL_OwnerDraw(pDrawItemInfo);
//	}
//return 0;
//}

//
// Tell the caller if this screen ID is currently visible on the display
//
// If a modal dialog box is being displayed then updates going on behind it
// are suspended to prevent the updating data from being displayed on top of
// the modal dialog box.
// Also, when the dialog box is removed we force a single screen clear to
// take place on the gauge screen to clear out the remnants of the dialog
// box between active features on the screen.
//
int CS_IsVisible(int screen)
{
	if (glbBlackoutFlag == true)
		return false;
	else if (glbBlackoutFlag == TRANSITION)
	{
		GUI_Clear();
		glbBlackoutFlag = false;
		// continue with normal processing...
	}

	if ((selectedRaceMode == DRAG_MODE) || (selectedRaceMode == AUTOXRALLY_MODE))
		return true;

	if ((screen == GetCurrentScreenID()) || (screen == DRIVE_MODE_SCREEN))
		return true;
	else return false;
}

int GetCurrentScreenID(void)
{
	switch (GetScreenType())
	{
	case DRIVE_MODE_SCREEN_TYPE:		return DriveScreenList[GetLastScreen(DRIVE_MODE_SCREEN_TYPE)].screenID;
	case INSTRUMENTS_MODE_SCREEN_TYPE:	return InstrumentsScreenList[GetLastScreen(INSTRUMENTS_MODE_SCREEN_TYPE)].screenID;
	default:
	case GAUGES_SCREEN_TYPE:			return GaugesScreenList[GetLastScreen(GAUGES_SCREEN_TYPE)].screenID;
	}
}

static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	switch (screen)
	{
		default:
		case GO_RACING:
		case REVIEW_SESSIONS:
		case RACE_SETUP:
		case SYSTEM_SETUP:		return &bmleftarrow;
	}
}

void ConfigScreensIgnoreNextKeyRelease(void)
{
	clickedOnThisScreen = false;
}

//static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
//	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
//	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
//	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
//	{ TEXT_CreateIndirect, "This will end your recording session", 0, 0, 30, 360, 30, TEXT_CF_HCENTER },
//	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 70, 360, 30, TEXT_CF_HCENTER }
//};

//
// Display a modal dialog box to see if the user wants to stop recording to exit this screen
//
// Returns	YES - proceed to exit record mode
//				NO - abort and stay
//
//int okToStopRecording(WM_HWIN hParent, int ypos)
//{
//	if (unsavedSysData.systemMode == DU2_IDLE)
//		return YES;
//
//	// KMC TODO - if DriveMode screen uses WM_CF_MEMDEV_ON_REDRAW then the popup screen makes the
//	// window background go brown.  But by allowing the paint to continue the brown does not show.
//	// This might be "fixed" for all screens if they all use WM_CF_MEMDEV_ON_REDRAW.
//	if (GetCurrentScreen() != DRIVE_MODE_SCREEN)
//		glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed
//
//	FRAMEWIN_SetDefaultFont(&GUI_Font24_ASCII);
//	TEXT_SetDefaultFont(&GUI_Font24_ASCII);
//	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);
//	BUTTON_SetDefaultBkColor(GUI_BLUE, BUTTON_BI_UNPRESSED);
//	BUTTON_SetDefaultTextColor(GUI_WHITE, BUTTON_BI_UNPRESSED);
//
//	if (GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &popupCallback, hParent, 10, ypos + 10) == GUI_ID_OK)
//		return YES;
//	else return NO;
//}
void popupCallback(WM_MESSAGE * pMsg)
{
	WM_HWIN hDlg;
	int NCode, Id;

	hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_INIT_DIALOG:
			FRAMEWIN_SetTextAlign(hDlg, GUI_TA_HCENTER);
			FRAMEWIN_SetTextColor(hDlg, popupTitleTextColor);
			if (systemSettingsPopupScreen == DELETE_SESSION)
			{
				TEXT_Handle hText;
				size_t len;

				hText = TEXT_CreateEx(0, 76, 360, 20, hDlg, WM_CF_SHOW, GUI_TA_CENTER | GUI_TA_VCENTER, 0, fullSessionName);
				if ((len = strlen(fullSessionName)) > 50)
					TEXT_SetFont(hText, &GUI_Font13B_ASCII);
				else if (len > 40)
					TEXT_SetFont(hText, &GUI_Font16B_ASCII);
				else if (len > 30)
					TEXT_SetFont(hText, &GUI_Font20_ASCII);
				else TEXT_SetFont(hText, &GUI_Font24_ASCII);
			}
			else if (systemSettingsPopupScreen == INVALID_FILES)
				TEXT_CreateEx(0, 100, 400, 30, hDlg, WM_CF_SHOW, GUI_TA_CENTER | GUI_TA_VCENTER, 0, fileList);

			systemSettingsPopupScreen = 0;		// Clear out flag
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
						case GUI_ID_CANCEL:
						case GUI_ID_YES:
						case GUI_ID_NO:
						case GUI_ID_CLOSE:
						case GUI_ID_HELP:
						case GUI_ID_MAXIMIZE:
						case GUI_ID_MINIMIZE:
							GUI_EndDialog(hDlg, Id);
							break;

						case GUI_ID_ABORT:
							userAbort = true;
							break;
					}
//					if (Id == GUI_ID_OK)				// OK Button
//						GUI_EndDialog(hDlg, 1);
//					else if (Id == GUI_ID_CANCEL)	// Cancel Button
//						GUI_EndDialog(hDlg, 0);

					glbBlackoutFlag = TRANSITION;	// Set flag to allow a screen clear to remove the dialog box
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

//
// Exit recording mode.  Perform state transition, communicate with the SU, tell the user, update tables
//
void ExitRecord(char saveData)
{
	SetUserMode(DU2_IDLE);
	raceModeState = RM_INITIAL;
	if ((selectedRaceMode == INSTRUMENTS_MODE) || (saveData == false))
		return;
	
	hSavingDataDialog = DisplaySavingData();
	trackTable[trackTableIndex].sdCardEntry = 0;	// We now want to save data to disk!  Set flag to force WriteUserDataEvent to write data to SD card
	WriteUserDataEvent(TRACK_DATA);					// Save new breadcrumb data
	trackTable[trackTableIndex].sdCardEntry = 1;	// We now KNOW that this entry is on the SD card, manually set the flag to reflect that

#ifndef _WINDOWS
	// Must now wait for session data write to disk to be complete as we are about to read those files
	// to generate the fastest lap information for displaying in Review Sessions.  Sit here and
	// poll sessionFile until it is cleared by TQMWriteTask or until the user aborts waiting.
	while (sessionFile && !userAbort)
	{
		GUI_Delay(100);
		GUI_Exec();
	}
#endif
	
	// Run data now saved to SD card, parse data to get lap information
	CreateFileList(SESSION_DATA);					// Get best lap information out of session files
	
	// Always select the last completed session of the current race type
	selectedSessionUnsorted = FindNewestFile((enum SESSION_TYPE)sysData.selectedSessionType);
													// Note: this routine is ready for all race session types
	GUI_EndDialog(hSavingDataDialog, 0);
	glbBlackoutFlag = TRANSITION;					// Resume painting
}


static const GUI_WIDGET_CREATE_INFO SavingDataResources[] = {
	{ FRAMEWIN_CreateIndirect, "System Status", 0, 40, 20, 380, 220, 0, 0 },
	//{ TEXT_CreateIndirect, "Saving your session data", 0, 0, 50, 360, 30, TEXT_CF_HCENTER },
	//{ TEXT_CreateIndirect, "Please Wait", 0, 0, 100, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Saving your session data", 0, 0, 30, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Please Wait", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_ABORT, 130, 115, 100, 60 }
};

//
// Tell user we're saving his data, he has to wait until the process is complete
//
WM_HWIN DisplaySavingData(void)
{
	WM_HWIN retval;
	
	userAbort = false;
	SetPopupDefaults(INFORMATIONAL_POPUP);
	glbBlackoutFlag = true;			// Suspend gauge screen updates while the popup dialog box is diaplyed
	retval = GUI_CreateDialogBox(SavingDataResources, GUI_COUNTOF(SavingDataResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
	GUI_Exec();						// Pause to allow popup to be drawn
	return retval;
}


void SetUserMode(DU2_STATE_TYPE mode)
{
	sysData.lastSystemMode = unsavedSysData.systemMode = mode;
	notifyModeChange();
}

//
// Checks the SU status to see if it self-timed out due to the stop
// recording timer.  Returns TRUE if the recording timed out.
//
unsigned char CheckForStopRecord(void)
{
	if (unsavedSysData.recordingTimedout == TRUE)
	{
		unsavedSysData.recordingTimedout = FALSE;
		return TRUE;
	}
	else return FALSE;
}

//
// Save the user selected race mode before staring the RaceMode state machine.
// If all checks pass then we will set the race mode to this selection.
//
void SaveRaceMode(enum GUI_RACE_MODES mode)
{
	selectedRaceMode = mode;
	sysData.selectedSessionType = ConvertRaceModeToSessionType(mode);
}

enum SESSION_TYPE ConvertRaceModeToSessionType(enum GUI_RACE_MODES mode)
{
	switch (mode)
	{
	case AUTOXRALLY_MODE:	return AUTOCROSS_TYPE;
	case DRIVE_MODE:		return DRIVE_TYPE;
	case DRAG_MODE:			return QUARTERMILE_TYPE;

	default:
	case INSTRUMENTS_MODE:
	case ROADRACE_MODE:		return ROADRACE_TYPE;
	}
}

//
// Entering the gauges screen with the touch screen currently pressed, ignore the next release of the touch screen
//
void GaugeScreensIgnoreNextKeyRelease(void)
{
	GSignoreRelease = true;
}

void GoRacingStateMachine(WM_HWIN hParent)
{
	switch (raceModeState)
	{
	// Check for satellite reception
	case RM_INITIAL:
		if (selectedRaceMode == INSTRUMENTS_MODE)
		{
			ProceedToRaceMode(selectedRaceMode, hParent);
			break;
		}
		else if (!HaveSatelliteReception())
		{
			GSignoreRelease = false;	// Any ignore that might have been set by a previous screen is not longer valid since
										// we're displaying a new screen that will ignore the release anyway and then, possibly, reset it
			DisplayWaitingForSatellites(GO_RACING_TYPE, selectedRaceMode, hParent);
			break;
		}
		// else fall through...

	// Check for tracks
	case RM_FOUND_SATELLITES:
// KMC - Modified 8/28/12 to fix issue where dummy tracks were being created - be sure track has valid name, otherwise force track selection
		if (!CurrentTrackInRange() || (trackData.courseType != SessionTypeToTrackType((enum SESSION_TYPE)sysData.selectedSessionType)) || (strcmp(sysData.track, "") == 0))
			{
				GSignoreRelease = false;	// Any ignore that might have been set by a previous screen is not longer valid since
										// we're displaying a new screen that will ignore the release anyway and then, possibly, reset it
				DisplayClosestTracks(hParent);
				break;
			}
		// else fall through...

	// Check for camera
	case RM_SELECTED_TRACK:
		////////if ((vehicleData.camera != CAMERA_NONE) && (GetValue(CAMERA_STATUS).iVal != CAMERA_STATUS_RECORDING))
		////////{
		////////	GSignoreRelease = false;	// Any ignore that might have been set by a previous screen is not longer valid since
		////////								// we're displaying a new screen that will ignore the release anyway and then, possibly, reset it
		////////	DisplayWaitingForCameraScreen(hParent);
		////////	break;
		////////}
		////////// else fall through...

	// Check for start/finish
	case RM_CAMERA_READY:			// Note: whether camera becomes ready of user presses RUN WITHOUT we come here
		switch (selectedRaceMode)
		{
		case ROADRACE_MODE:
			if (trackData.startLineLat == 0.0)
			{
				waitedForSF = true;
				SetUserMode(DU2_LAPS_WAITNOSF);
				GSignoreRelease = false;	// Any ignore that might have been set by a previous screen is not longer valid since
											// we're displaying a new screen that will ignore the release anyway and then, possibly, reset it
				DisplayTapAtSFScreen(hParent);
				return;
			}
			else waitedForSF = false;
			break;							// allow to fall through
		case AUTOXRALLY_MODE:
			SetUserMode(DU2_AUTOX_WAITSTAGE);
			DisplayAutoXStageScreen(hParent);
			waitedForSF = true;
			return;
		case DRAG_MODE:
			SetUserMode(DU2_DRAG_WAITSTAGE);	// KMC TODO - we need to combine these to a single WAITSTAGE state as the user does not get the run length choice until we are on this state
			DisplayDragStageScreen(hParent);
			waitedForSF = true;
			return;
		default:
			waitedForSF = false;
			break;
		}
		// else fall through...

	// Go racing
	case RM_STAGED:
	case RM_HAVE_SF:
		if ((selectedRaceMode == AUTOXRALLY_MODE) || (selectedRaceMode == DRAG_MODE))
			if ((GetSUCam() != CAMERA_NONE) && (GetValue(CAMERA_STATUS).iVal != CAMERA_STATUS_RECORDING))
			{
				GSignoreRelease = false;	// Any ignore that might have been set by a previous screen is not longer valid since
											// we're displaying a new screen that will ignore the release anyway and then, possibly, reset it
				DisplayWaitingForCameraScreen(hParent);
				break;
			}
		// else fall through...

	case RM_CAMERA_RECORDING:
		raceModeState = RM_CAMERA_RECORDING;		// Be sure the race mode state is updated
		switch (selectedRaceMode)
		{
		case ROADRACE_MODE:
			if (waitedForSF)
				ProceedToRaceMode(NO_CHANGE_MODE, hParent);
			else ProceedToRaceMode(selectedRaceMode, hParent);
			break;
		case AUTOXRALLY_MODE:
//			if ((vehicleData.camera != CAMERA_NONE) && (GetValue(CAMERA_STATUS).iVal != CAMERA_STATUS_RECORDING))
//			{
//				GSignoreRelease = false;	// Any ignore that might have been set by a previous screen is not longer valid since
//											// we're displaying a new screen that will ignore the release anyway and then, possibly, reset it
//				DisplayWaitingForCameraScreen(hParent);
//				break;
//			}
			SetUserMode(DU2_AUTOX_WAITLAUNCH);
			DisplayLaunchScreen(hParent);
			break;
		case DRAG_MODE:
			SetUserMode(DU2_DRAG_WAITLAUNCH);
			DisplayLaunchScreen(hParent);
			break;
		default:
			ProceedToRaceMode(selectedRaceMode, hParent);
			break;
		}
		break;

	case RM_LAUNCH_DETECTED:
		ProceedToRaceMode(selectedRaceMode, hParent);
		break;

	case RM_RACING:
		break;

	case RM_CANCELLED_WAITING_FOR_CAMERA:
	case RM_CANCELLED_WFS:
	case RM_CANCELLED_SELECT_TRACK:
		raceModeState = RM_INITIAL;
		ResumeScreenScrolling();
		DisplayNewScreenList(CONFIG_SCREEN_TYPE, GO_RACING, hParent);
		break;
	}
}


void DisplayClosestTracks(WM_HWIN hPrev)
{
	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenType(DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE);
	ctType = CT_RACE_ENTRY;

	SetScreenScrollingHOnly();

// Removed as this is redundent - GetClosestTracks will be called when the list wheel is displayed
//	GetClosestTracks(161);
//	CreateTrackList();

	if ((hDisplayClosestTracks = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, RaceSetupScreensCallback, 0)) <= 0)
		SystemError();

// KMC 6/6/12 - if displaying closest tracks on GoRacing want to display CLOSEST first, not currently selected
//	SetWorkingTrackName();
	ClearWorkingTrackName();

	DisplayTrackSelection(hDisplayClosestTracks);

	hScreenTimer = WM_CreateTimer(hDisplayClosestTracks, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}


//
// Configure the unit for the selected race mode
//
void ProceedToRaceMode(enum GUI_RACE_MODES mode, WM_HWIN hParent)
{
	int screen;

	unsavedSysData.recordingTimedout = FALSE;	// Be sure recording mode is off (it should be already but be sure)

#ifdef	INCLUDE_RACE_HELP
	StartRaceHelpScreenTimer();
#endif

#ifdef _WINDOWS
	SimStartSession();
#else
	if (demoMode)
		SimStartSession();
#endif

	switch (mode)
	{
	case ROADRACE_MODE:		
		SetUserMode(DU2_LAPS_WAITSTART);	
		break;
	case INSTRUMENTS_MODE:	
		SetUserMode(DU2_INSTRUMENT);		
		break;
	case DRIVE_MODE:		
		SetUserMode(DU2_DRIVE);				
		break;

	case DRAG_MODE:
	case AUTOXRALLY_MODE:
	case NO_CHANGE_MODE:
		break;
	}
	raceModeState = RM_RACING;

	ResumeScreenScrolling();
	if (mode == DRIVE_MODE)
	{
		screen = GetLastScreen(DRIVE_MODE_SCREEN_TYPE);
		sysData.screentype = DRIVE_MODE_SCREEN_TYPE;
		if (screen == -1)
			screen = DEFAULT_DRIVE_MODE_SCREEN_INDEX;	// Default screen INDEX (not ID) for this mode - Drive Screen
		else while (InactiveGaugeScreen(screen))		// Be sure we don't start out on an inactive screen
			screen = GetNextGaugeScreen();
	}
	else if (mode == INSTRUMENTS_MODE)
	{
		screen = GetLastScreen(INSTRUMENTS_MODE_SCREEN_TYPE);
		sysData.screentype = INSTRUMENTS_MODE_SCREEN_TYPE;
		if (screen == -1)
			screen = DEFAULT_INSTRUMENTS_MODE_SCREEN_INDEX;	// Default screen INDEX (not ID) for this mode
		else while (InactiveGaugeScreen(screen))			// Be sure we don't start out on an inactive screen
			screen = GetNextGaugeScreen();
	}
	else if ((mode == AUTOXRALLY_MODE) || (mode == DRAG_MODE))
	{
		DragAutoXGauges(hParent);
		WriteDataStructures(SYSTEM_DATA);				// Save sysData once when entering race mode
		return;
	}
	else
	{
		screen = GetLastScreen(GAUGES_SCREEN_TYPE);
		sysData.screentype = GAUGES_SCREEN_TYPE;
		if (screen == -1)
			screen = DEFAULT_RACE_MODE_SCREEN_INDEX;	// Default screen INDEX (not ID) for this mode
		else while (InactiveGaugeScreen(screen))		// Be sure we don't start out on an inactive screen
			screen = GetNextGaugeScreen();
	}

	RunningGauges(screen, hParent);
	WriteDataStructures(SYSTEM_DATA);					// Save sysData once when entering race mode
}


int StartupGoToRaceMode(void)
{
	if (sysData.autoRecord)
	{
		if (GetScreenType() == GAUGES_SCREEN_TYPE)
			selectedRaceMode = ROADRACE_MODE;
		else if (GetScreenType() == INSTRUMENTS_MODE_SCREEN_TYPE)
			selectedRaceMode = INSTRUMENTS_MODE;
		else if (GetScreenType() == DRIVE_MODE_SCREEN_TYPE)
			selectedRaceMode = DRIVE_MODE;
		else
		{
			SetUserMode(DU2_IDLE);
			return false;
		}
		GoRacingStateMachine((WM_HWIN)NULL);
		return true;
	}
	else
	{
		SetUserMode(DU2_IDLE);					// Don't allow back to gauge screen
		return false;
	}
}

#ifdef	INCLUDE_RACE_HELP
//
// StartRaceHelpScreenTimer
//		First check to see if we have already shown the race mode help overlay
//		the maximum number of times, RACE_HELP_COUNT
//		If not, then start timer and set flags for first 5 seconds of race to 
//		display help screen with Alpha blending
//
void StartRaceHelpScreenTimer(void)
{
	int temp;

	// Check to see if we've displayed the race screens help overlay enough times
	temp = (sysData.multiSettings_2 & 0x000000F0) >> 4;
	if (temp < RACE_HELP_COUNT)
	{
		temp++;
		sysData.multiSettings_2 = (sysData.multiSettings_2 & 0xFFFFFF0F) | (temp << 4);

		raceStartTime = GUI_GetTime();
		raceTimerActive = true;
	}
	else raceTimerActive = false;
}


//
// Check the help screen timer - if still active display the help screen for this race screen
// otherwise restore alpha mode to normal, clear flag and return.
//
void CheckRaceHelpScreen(int screenID)
{
	if (GUI_GetTime() < raceStartTime + FIVE_SECONDS)
	{
		GUI_EnableAlpha(1);
		GUI_SetColor(0x38000000 | GUI_BLACK);
		GUI_DrawBitmap(&bmGenericGaugeHelp, 0, 0);

		//GUI_FillRect(0, 0, XSIZE_PHYS, YSIZE_PHYS);
		//GUI_SetColor(GUI_BLACK);

		//switch (screenID)
		//{
		//case RPM_SWEEP_GAUGE:
		////	GUI_DrawBitmap(&bmSystemStatusScreenHelpBlackBack, 0, 0);
		//	break;
		//case ANALOG_GAUGE:
		//default:
		//	GUI_DrawBitmap(&bmGenericGaugeHelp, 0, 0);
		//}
	}
	else
	{
		GUI_EnableAlpha(0);
		raceTimerActive = false;

		//if (screenID == DRIVE_MODE_SCREEN)	// If ending the help period and this is the Drive Mode screen (compass screen)
		//	GUI_Clear();					// Must remove help text as not all of the screen is redrawn
	}
}
#endif

char *GetSystemStatsHelpText(const char *pText)
{
	static char mainHelp[sizeof(MAIN_SCREEN_INFO) + 550];		// yes you really need that much extra space!
	char buffer[100];
	int i;

//	SYSTEM STATUS
//  Owner: nnnn
//  Contact: Info ####
// blank line
//  Inputs Configuration:
// details...
// blank line
//	TraqDash Firmware x.xx
//	Sensor Unit Firmware y.yy
// blank line
//	DU Ser# ssssss, HW n.nn, Built mm/yy
//	SU Ser# ssssss, HW n.nn, Built mm/yy
		
	strcpy(mainHelp, pText);
	sprintf(mainHelp + strlen(pText), "\n\nSYSTEM STATUS\nOwner: %s\nContact Info: %s\n\n",
		unsavedSysData.unitInfo.owner, unsavedSysData.unitInfo.phone);

	strcat(mainHelp, "Inputs Configuration:\n");
	strcat(mainHelp, GetFullCameraText(buffer));

	for (i = 0; i < 4; i++)
	{
		strcat(mainHelp, GetAnalogInputText(i, buffer));
	}

	strcat(mainHelp, GetDigitalInputText(0, buffer));
	strcat(mainHelp, GetDigitalInputText(1, buffer));

#ifndef _WINDOWS
	if (MAX_PAGE_64EMBIT == unsavedSysData.dataflash[0].bytesPerPage)
		sprintf(mainHelp + strlen(mainHelp),"\nAdesto Dataflash");
	else
		sprintf(mainHelp + strlen(mainHelp),"\nAtmel Dataflash");
#endif

	sprintf(mainHelp + strlen(mainHelp),"\nTraqDash Bootloader %2.2f",
		((float) unsavedSysData.unitInfo.bootloaderSwRev / 100.0));
		
	sprintf(mainHelp + strlen(mainHelp),"\nTraqDash Firmware %2.2f\nSensor Unit Firmware %2.2f",
		((float) unsavedSysData.duSwRev / 100.0), ((float) suData.suSwRev / 100.0));
	
	sprintf(mainHelp + strlen(mainHelp),"\n\nDU Ser# %u, HW %2.2f, Built %02d/%02d\n",
		unsavedSysData.unitInfo.serno, ((float) unsavedSysData.duHwRev / 100.0),
		(int) (1.0+(((float)(unsavedSysData.unitInfo.week-1)/4.3))), unsavedSysData.unitInfo.year);
				
	sprintf(mainHelp + strlen(mainHelp),"SU Ser# %u, HW %2.2f, Built %02d/%02d",
		suData.serialNumber, ((float) suData.suHwRev / 100.0),
		(int) (1.0+(((float)(suData.buildDateWeek-1)/4.3))), (int) suData.buildDateYear);
	
	return mainHelp;
}

char *GetStorageStatsHelpText(const char *pText)
{
	static char storageHelp[sizeof(STORAGE_SETTINGS_INFO) + 200];

#ifndef _WINDOWS
	char sizeunit = ' ';
#endif

	strcpy(storageHelp, pText);

#ifndef _WINDOWS
	if (OkToUseSDCard()) {
		float freespace = (float)FS_GetVolumeFreeSpace("");
	
		if (freespace > ONE_GIG) {
			freespace /= ONE_GIG;
			sizeunit = 'G';
		}
		else if (freespace > ONE_MEG) {
			freespace /= ONE_MEG;
			sizeunit = 'M';
		}
		else if (freespace > ONE_K) {
			freespace /= ONE_K;
			sizeunit = 'K';
		}
		
		sprintf(storageHelp + strlen(storageHelp), "\n\nSD Card Free Space %.3f %cBytes", freespace, sizeunit);
	} // if
	else strcat(storageHelp, "\n\nNo SD card present");
#endif
	
	return storageHelp;
}

char *GetAnalogInputText(int index, char *pBuffer)
{
	sprintf(pBuffer, "Analog Input %d: ", index);
	if (vehicleData.analogInput[index].enabled)
		strcat(pBuffer, vehicleData.analogInput[index].Name);
	else strcat(pBuffer, "Disabled");
	strcat(pBuffer, "\n");

	return pBuffer;
}

char *GetDigitalInputText(int index, char *pBuffer)
{
	sprintf(pBuffer, "Digital Input %d: ", index + 4);
	if (vehicleData.digitalInput[index].enabled)
		strcat(pBuffer, vehicleData.digitalInput[index].Name);
	else strcat(pBuffer, "Disabled");
	strcat(pBuffer, "\n");

	return pBuffer;
}

static const GUI_WIDGET_CREATE_INFO NotSupportedResources[] = {
	{ FRAMEWIN_CreateIndirect, "ERROR", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "This function is not supported", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "without an SD card inserted!", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

int NotSupportedPopup(WM_HWIN owner)
{
	int retval;

	SetPopupDefaults(ERROR_POPUP);

	retval = false;
	modalWindowUp = true;

	retval = GUI_ExecDialogBox(NotSupportedResources, GUI_COUNTOF(NotSupportedResources), &nonModalPopupCallback, owner, 10, 10);
	
	modalWindowUp = false;

	return retval;
}

//
// Returns true if this is any race screen of any type
//
char AnyGaugeScreen(enum SCREEN_TYPE type)
{
	if ((type == GAUGES_SCREEN_TYPE) || (type == DRIVE_MODE_SCREEN_TYPE) || (type == INSTRUMENTS_MODE_SCREEN_TYPE) || (type == DRAG_AUTOX_GAUGE_SCREEN_TYPE))
		return true;
	else return false;
}

//
// Returns true if this is any gauge screen other than in drag or autocross modes (these screens do not "scroll")
//
char AnyNonDragGaugeScreen(enum SCREEN_TYPE type)
{
	if ((type == GAUGES_SCREEN_TYPE) || (type == DRIVE_MODE_SCREEN_TYPE) || (type == INSTRUMENTS_MODE_SCREEN_TYPE) || (type == DRAG_AUTOX_GAUGE_SCREEN_TYPE))
		return true;
	else return false;
}

//
// Set the number of analog inputs currently enabled to determine which analog
// screen to show.
//
int DetermineAnalogScreenSelection(void)
{
	int numAnalogsDisplayed, i;

	for (numAnalogsDisplayed = i = 0; i < 4; i++)
	{
		if (vehicleData.analogInput[i].enabled)
			numAnalogsDisplayed++;
	}
	return numAnalogsDisplayed;
}

/*************************** End of file ****************************/

