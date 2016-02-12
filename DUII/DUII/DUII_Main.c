/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2008  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS version: 3.52e                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : GUI_DUII_Main.c
Purpose : emWin and embOS demo application
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef _WINDOWS
#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	MAINTASK_NUM
#endif
#endif

#include "GUI.h"
#include "DUII.h"
#include "commondefs.h"
#include "DIALOG.h"
#include "ConfigScreens.h"
#include "GeneralConfig.h"
#include "FlashReadWrite.h"
#if	_WINDOWS
#include "windows.h"
#include "SIMConf.h"
#else
#include "RTOS.H"
#include "FS.h"
#include "MMC_MCI_HW.h"
#include "SPI_API.h"
#include "LCDConf.h"
#include "FlashLights.h"
#include "FactoryCheckout.h"
#include "dataflash.h"
#include "CommTask.h"
#include "adc.h"
#include "WiFiFunctions.h"
#endif
#include "LEDHandler.h"
#include "GaugeScreen.h"
#include <stdio.h>
#include <string.h>
#include "RunningGauges.h"
#include "CalibrateTouchScreen.h"
#include "DataHandler.h"
#include "RaceSetupScreens.h"
#include "TrackDetect.h"
#include "SystemSettingsScreens.h"
#include "SFScreens.h"
#include "TrackSettingsScreens.h"
#include "ReviewSessionsScreens.h"
#include "VehicleSettingsScreens.h"
#include "Archive.h"
#include "PersonalizeScreens.h"
#include "HelpScreens.h"


//
// External Declarations
//
extern GUI_CONST_STORAGE GUI_BITMAP bmSplashScreen;
extern GUI_CONST_STORAGE GUI_BITMAP bmSonyIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmChaseCamIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceCamIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmGoProIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmReplayIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmReplay1080Icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmReplayMiniIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmReplayPrimeIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmMobiusIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmCheckIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmXIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmDriverName;
extern GUI_CONST_STORAGE GUI_BITMAP bmVehicleName;
extern GUI_CONST_STORAGE GUI_BITMAP bmUSBCamIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmWiFiEnabledIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmWiFiDisabledIcon;

#ifndef _WINDOWS
extern void CommSimulatorTask(void);
extern OS_STACKPTR int commStack[1024];
extern char SDCardPresent(void);
extern void reset(void);
extern OS_TASK mainTCB;
#endif

#ifdef BUILD_DEBUG_PORT
#include "dbgu.h"

extern int glbErrorFlag;
#endif

//
// Global data declarations
//
char lockupSemaphore;
int popupTitleTextColor = GUI_WHITE;

//
// Local data definitions
//
typedef enum {
	CODESU,
	CODETQ,
	BOOTTQ,
	NOMATCH
} codefiletypes;

const char VERSION_PATH[] = { LEADING_PATH CFG_PATH "\\" VER_FILE DAT_EXT };		// location where Version.dat is stored
//
// Local function prototypes
//
int SDCardRemoved(void);
int SDCardReinserted(void);
int ReFlashYorN( codefiletypes, int );
int DemoYorN( void );
void PowerOffUnit(WM_HWIN owner, char reset, int delay);
void SaveFilesAndPowerDown(void);
#ifndef _WINDOWS
void ConfigureCache(void);
char OkToUseSDCard(void);
int RetrySDCard( void );
#endif
void SetPopupDefaults(enum POPUP_TYPES type);
void ConfigureForNoSDCard(void);
void InitializeTraqmateFiles(void);
void AskAndDoFormatSDCard(void);
void InitializeTraqmateData(void);
void DisplayInvalidFilesPopup(int files);
void WriteCodeImage(void);
void DisplaySplashScreen1(void);
void DisplaySplashScreen3(void);
void DisplayAnalogStatus(int i);
void RepaintSplash(void);
void CreateProgressBar(PROGBAR_Handle *hProgBar, int maxval);
int NewSWVersion(void);
int UserWantsToUpdateSF(void);
void local_itoa(int n, char s[]);
void reverse(char s[]);

//
// Local variable declarations
//
int testMode;
int demoMode;
int factMode;
static char localSDCardFlag;		// true = SD card present, false == absent
static char okToUseSDCard = false;
long splashScreenStartTime;

#define DEBOUNCE_ONOFF_MSEC				1000	// debounce value of onoff buttons in msec
#define FAT_VOLUME_LABEL_LENGTH			32		// number of characters in NFTS volume label (FAT = 11)
#define	SPLASH_SCREEN_PERSIST_TIME		4000	// 4 seconds

/*********************************************************************
*
*       main()
*/
void MainTask(void);

void MainTask(void)
{
	int screen;
	enum SCREEN_TYPE screenType;
#ifndef _WINDOWS
	char checkFormat, dontRunWithout;
	char sdCardLabel[FAT_VOLUME_LABEL_LENGTH+1];			// volume label of sd card
#endif

#ifdef BUILD_DEBUG_PORT
   DBGU_Configure(DBGU_STANDARD, 115200, BOARD_MCK);
#endif

	// initialize the who are we information
	unsavedSysData.duSwRev = DU2_SW_REV;					// software rev * 100
	unsavedSysData.duDataRev = DU2_DATA_REV;				// data rev for matching up with SU messages
	unsavedSysData.duHwRev = DU2_HW_REV;					// what hardware rev are we working with

	// initialize other data in unsavedSysData
	unsavedSysData.recordingTimedout = FALSE;				// Recording initially off

#ifdef _WINDOWS
	unsavedSysData.unitInfo.serno = DEFAULT_SERIAL_NUMBER;
	unsavedSysData.unitInfo.week = DEFAULT_BUILD_WEEK;
	unsavedSysData.unitInfo.year = DEFAULT_BUILD_YEAR;
	unsavedSysData.unitInfo.bootloaderSwRev = DEFAULT_BOOTLOADER_REV;
#endif
	
	GUI_Init();
	DisplaySplashScreen1();

	WarningPopupInit();			// Initialize warning popups

	BUTTON_SetDefaultSkin   (BUTTON_SKIN_FLEX);

	CHECKBOX_SetDefaultSkin (CHECKBOX_SKIN_FLEX);
	DROPDOWN_SetDefaultSkin (DROPDOWN_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin (FRAMEWIN_SKIN_FLEX);
	HEADER_SetDefaultSkin   (HEADER_SKIN_FLEX);
	RADIO_SetDefaultSkin    (RADIO_SKIN_FLEX);
	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
	FRAMEWIN_SetDefaultClientColor(GUI_OFFWHITE);

#ifdef _WINDOWS
	SIM_Init();
	InitializeTraqmateFiles();
  	InitCommSimulatorTask();		// Initialize simulation data (uses vehicle data set in InitializeTraqmateFiles)
	InitializeLEDStructures();		// Initialize LED structures before flashing starts
	InitializeLEDs();
#else
	FlashLights();
	FS_Init();
	FS_FAT_SupportLFN();
	
	ConfigureCache();

	checkFormat = true;	
	dontRunWithout = true;
	if ((unsavedSysData.sdCardInserted = SDCardPresent()) == false)		// card not inserted
	{
		if ((FS_MCI_HW_IsCardPresent(0) && SDCardWriteProtected()))
#ifdef SDCARDS_BEHAVING_WELL
			checkFormat = true;					// Card will behave properly when I ask it if it's formatted
#else
			checkFormat = false;				// Card is present but write protected - special case!!
#endif
			
		while ((dontRunWithout = RetrySDCard()) == true)	// Ask the user if he wants to retry or run without
		{
			if (SDCardPresent() == true)		// retry
			{
				DisplaySplashScreen1();			// Redisplay splash screen to clear popup
				goto continueStartup;
			}
		}
		checkFormat = false;
		InitializeTraqmateData();				// run without - set default data
	continueStartup:
		unsavedSysData.sdCardInserted = SDCardPresent();
		testMode = false;
	}
	
	DisplaySplashScreen1();			// Repaint the initial splash screen incase we displayed the popup and he said no

	localSDCardFlag = true;
	
	if (checkFormat)
	{
		if (((unsavedSysData.sdCardInserted)) && (FS_IsHLFormatted("") != 1))
			AskAndDoFormatSDCard();
		else okToUseSDCard = true;
		
		// check volume label of SD card and set if it is blank
		if (0 == FS_GetVolumeLabel("", sdCardLabel, FAT_VOLUME_LABEL_LENGTH)) {
			if (0 == strlen(sdCardLabel))
				FS_SetVolumeLabel("", "TRAQDASH");
		} // if
	
		InitializeTraqmateFiles();
	}
	else								// Handle special case of card inserted but was write protected
	{
		ConfigureForNoSDCard();			// In this case we can only run with no SD card
		InitializeTraqmateData();		// run without - set default data
		if (dontRunWithout == true)		// If the user does not want to run with no SD card
			SDCardReinserted();			// Ask the user if we should reset and do it if so
	}
	
	// Set the LEDs to user settings (note: InitializeLEDStructures called in   
	// Main.c which must be called before startup LED flashes)
	InitializeLEDs();					

	// load new code if available
	// needs to be after sd card read so that if user says no, su will get the right config
	WriteCodeImage();
	
#endif

	WIDGET_SetDefaultEffect(&WIDGET_Effect_None);

	screen = GetLastScreen(GetScreenType());
	screenType = GetScreenType();

	ValidateRaceScreens();

	// Be sure we have show the splash screen long enough
	while ((GUI_GetTime() - splashScreenStartTime) < SPLASH_SCREEN_PERSIST_TIME)
		;

	raceModeState = RM_INITIAL;
	// Set the last running mode
	if (AnyGaugeScreen(screenType))
	{
		if (!StartupGoToRaceMode())
		{
			screenType = CONFIG_SCREEN_TYPE;		// Go to Config Screens group instead
			screen = GO_RACING;						// Go Racing screen
		}
	}
	else SetUserMode(DU2_IDLE);

	switch (screenType)
	{
		case DRIVE_MODE_SCREEN_TYPE:
		case INSTRUMENTS_MODE_SCREEN_TYPE:
		case GAUGES_SCREEN_TYPE:
			// Screen transition handled in StartupGoToRaceMode
			break;

		case RACE_SETUP_SCREENS_TYPE:
			SetWorkingTrackName();
			// fall through...
		case CONFIG_SCREEN_TYPE:
		case SYSTEM_SETTINGS_SCREENS_TYPE:
		case VEHICLE_SETTINGS_SCREENS_TYPE:
		case TACH_SETUP_SCREENS_TYPE:
		case INPUTS_SCREENS_TYPE:
		case TRACK_SETTINGS_SCREENS_TYPE:
		case GEARING_SCREENS_TYPE:
		//case SFSCREENS_TYPE:
			DisplayNewScreenList(screenType, screen, (WM_HWIN)NULL);
			break;

		case VEHICLE_SETTING_SCREEN_TYPE:
			Display_VehicleSettingMainScreen((WM_HWIN)NULL);
			break;

		case REVIEW_SESSIONS_SCREENS_TYPE:
			if (sysData.navigatedToReviewAfterRace == true)		// See if we navigated by race exit to the review screen
				Display_GeneralConfig((WM_HWIN)NULL);			// If so just go to main menu
			else DisplaySessionDetailsScreen();					// otherwise go ahead to review screen
			break;

		case TRACK_SETTING_SCREEN_TYPE:
			Display_TrackSettingMainScreen((WM_HWIN)NULL);
			break;

		case SFSCREENS_TYPE:
			DisplaySFKeyboard(screen, (WM_HWIN)NULL);
			break;

			// Do not support returning directly to last selected digital or analog settings screen type.
			// Global data is set up in INPUTS_SCREENS_TYPE that is required but is not saved at power down.
		case DIGITAL_SETTINGS_SCREEN_TYPE:
			DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), (WM_HWIN)NULL);
			break;

		case ANALOG_SETTINGS_SCREEN_TYPE:
		case ANALOG_INPUTS_SCALE:
			DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), (WM_HWIN)NULL);
			break;

		case PERSONALIZE_REVIEW_SCREEN_TYPE:
			DisplayPersonalizationScreen((WM_HWIN)NULL);
			break;

		case AERO_GRIP_SCREEN_TYPE:
			CreateAeroGripScreen((WM_HWIN)NULL);
			break;

		case CAMERA_DELAY_SCREEN_TYPE:
			CreateCameraDelay((WM_HWIN)NULL);
			break;

		case CAMERA_SELECTION_SCREEN_TYPE:
			CreateCameraSelection((WM_HWIN)NULL);
			break;

		case TRAQDASH_CAMERA_SCREEN:
			CreateTraqdashCamScreen((WM_HWIN)NULL);
			break;

		case RECORD_SPEED_SCREEN_TYPE:
			DisplayNewScreenList(SYSTEM_SETTINGS_SCREENS_TYPE, RECORD_SPEED, (WM_HWIN)NULL);
			break;

		default:
			Display_GeneralConfig((WM_HWIN)NULL);
			break;
	}
	
	SetGear(GEAR_RESET);	// initialize gear tables
	
#ifndef _WINDOWS
	// check for demo mode
	if (demoMode) {
		if (OS_IsTask(&commTCB))
			OS_Terminate(&commTCB);		// kill comm task
						
		OS_CREATETASK(&commTCB, "CommSimulatorTask", CommSimulatorTask, 120, commStack);
	} // if
	else
		OS_Delay(500);					// give comm a chance to find mr. su
	
	// check for initial calibration of LED temperature
	if (0 == sysData.calibrationTempReading) {
		sysData.calibrationTemperature = ROOM_TEMPERATURE;				// Default calibration temp assumed to be 24C = 75F, units always stored in metric
		sysData.calibrationTempReading = GetADC(ADC_LED_BOARD_TEMP);	// Take current board reading and set temp correction offset
		sysData.LEDtempOffset = CalculateTempCorrection();				// Calculate hardware correction factor
	} // if
	
	// Done with initialization - drop the priority of the main task to ensure file writes and comm tasks have the priority
	OS_SetPriority(&mainTCB, 5);
#endif
		
	// Main task loop
	while(1)
	{
		static unsigned debounce = 0;	// used to count up button press debounce
	
		ProcessScreenMovement();
	
		GUI_Exec();
		lockupSemaphore = false;			// returned back to main - clear lockup semaphore
	
#ifdef BUILD_DEBUG_PORT
		if (glbErrorFlag)
			DBGU_Print("Error detected!\n\r");
#endif
	
#ifdef _WINDOWS
		if (SIM_Tick())
		{
			PowerOffUnit((WM_HWIN)NULL, false, 0);
			return;
		}
#else
		// check ON/OFF button
		if (!sysData.autoOn) {			// disable DU power switch if AutoOn is enabled
			if (0 == (PIOD_PDSR & (1 << ONOFF_BUTTON))) {	// if switch pressed, count up
	
				// if switch first pressed, remember the time
				if (0 == debounce)
					debounce = OS_GetTime();
				else {
					// power off if switch was held more than ON/OFF period
					if (OS_GetTime() > (debounce  + DEBOUNCE_ONOFF_MSEC)) {				// initiate shutdown sequence
						PowerOffUnit((WM_HWIN)NULL, false, 0);
					} // if
				} // else
			} // if
			else {
				// if button not pressed, reset ONOFF counter
				debounce = 0;
			} // else
		} // if
		
		// check status of SD card
		if (FS_MCI_HW_IsCardPresent(0))
		{
			// SD card is present - see if it was previously not there
			if (!localSDCardFlag)
			{
				// SD card just inserted
				SDCardReinserted();					// See what we're going to do
				localSDCardFlag = true;
			}
			// else - otherwise card was already there - no change - keep on truckin'
		}
		else
		{
			localSDCardFlag = false;				// SD card now not present
			// SD card not present - see if it was previously there
			if (unsavedSysData.sdCardInserted)
			{
				ConfigureForNoSDCard();

				// See what we're going to do
				if (SDCardRemoved() == false)
				{
					SetUserMode(DU2_SHUTDOWN);	// Do not go through normal power off sequence as we cannot save data - tell comm and laptimer to stop recording and shut off power at SU
					while (1)					// Stop here
						GUI_Exec();				// Allow other tasks to communicate with the SU to pull the plug (note: we cannot save files as there is no SD card present)
				}
			}
		}

// *** TEST		OS_Delay(1);								// Allow other tasks a chance to run
		OS_Yield();								// Allow other tasks a chance to run
#endif
	}
}

//
// SD card was just removed - configure the system to run without it
//
void ConfigureForNoSDCard(void)
{
	// SD card was just removed
	unsavedSysData.sdCardInserted = 0;		// new status

#ifndef _WINDOWS
	// Unmount all drives (flushes cache)
	FS_Unmount("");			// gas commented out. in my tests unmounting a volume that is ejected causes unpredictable results

	if (DU2_RECORDING(unsavedSysData.systemMode))
		notifyTQM(DU2_SDCARD_EJECT);
#endif

	unsavedSysData.systemMode = DU2_NOSDCARD;
	
	// Reset all lists and tables
	ResetAllFileLists(true);

	ClearNameStruct();					// Erase session notes line (used on Race Review screen)
}

#ifndef _WINDOWS
//
// SD card not inserted at powerup.
// Display a modal dialog box to ask user to insert an SD card and retry
//
// Returns	YES - retry
//			NO - just go on
//
int RetrySDCard(void)
{
static const GUI_WIDGET_CREATE_INFO NoSDCardResources[] = {
	{ FRAMEWIN_CreateIndirect, "SD CARD NOT READY!", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "No SD card or write protected", 0, 0, 30, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Insert SD card now or power down", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "RETRY",		GUI_ID_OK,		20, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "POWER DOWN", GUI_ID_CANCEL, 160, 115, 180, 60 }
};

	SetPopupDefaults(ERROR_POPUP);

	glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed

	if (GUI_ExecDialogBox(NoSDCardResources, GUI_COUNTOF(NoSDCardResources), &popupCallback, WM_GetDesktopWindow(), 10, 10) == GUI_ID_OK)
		return true;
	else
	{
		PowerOffUnit(WM_GetDesktopWindow(), false, 1000);				// Pause for 1 second to allow time for the SU to process the new camera setting
		return false;
	}
}
#endif

//#define BUTTON_SKINFLEX_PI_PRESSED     0
//#define BUTTON_SKINFLEX_PI_FOCUSSED    1
//#define BUTTON_SKINFLEX_PI_ENABLED     2
//#define BUTTON_SKINFLEX_PI_DISABLED    3

//
// Perform standard initialization for error message popups
//
void SetPopupDefaults(enum POPUP_TYPES type)
{
	FRAMEWIN_SKINFLEX_PROPS Props;
	//BUTTON_SKINFLEX_PROPS ButtonProps;

	switch (type)
	{
	case INFORMATIONAL_POPUP:
		FRAMEWIN_GetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		Props.aColorFrame[0] = 0x000000;
		Props.aColorFrame[1] = 0xffd070;
		Props.aColorFrame[2] = 0xaf0000;
		Props.aColorTitle[0] = 0xaf0000;
		Props.aColorTitle[1] = 0xffd070;
		Props.Radius = 7;
		FRAMEWIN_SetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		popupTitleTextColor = GUI_WHITE;
		break;

	case WARNING_POPUP:
		FRAMEWIN_GetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		Props.aColorFrame[0] = 0x000000;
		Props.aColorFrame[1] = 0x0096ff;
		Props.aColorFrame[2] = 0x00ffff;
		Props.aColorTitle[0] = 0x0096ff;
		Props.aColorTitle[1] = 0x00ffff;
		Props.Radius = 7;
		FRAMEWIN_SetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		popupTitleTextColor = GUI_BLACK;
		break;

	case ERROR_POPUP:
		FRAMEWIN_GetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		Props.aColorFrame[0] = 0x000000;
		Props.aColorFrame[1] = 0x0303b1;
		Props.aColorFrame[2] = 0x0000ff;
		Props.aColorTitle[0] = 0x0303b1;
		Props.aColorTitle[1] = 0x0000ff;
		Props.Radius = 7;
		FRAMEWIN_SetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		popupTitleTextColor = GUI_WHITE;
		break;

	case PROGRAMMING_POPUP:
		FRAMEWIN_GetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		Props.aColorFrame[0] = 0x000000;
		Props.aColorFrame[1] = 0x22a508;
		Props.aColorFrame[2] = 0x2aff00;
		Props.aColorTitle[0] = 0x22a508;
		Props.aColorTitle[1] = 0x2aff00;
		Props.Radius = 7;
		FRAMEWIN_SetSkinFlexProps(&Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
		popupTitleTextColor = GUI_WHITE;
		break;
	}

	FRAMEWIN_SetDefaultFont(&GUI_Font24_ASCII);
	TEXT_SetDefaultFont(&GUI_Font24_ASCII);
	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);
	BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_BI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_BI_PRESSED);
}

//
// SD card removed while running.
// Display a modal dialog box to ask user if he wants to power off the unit or run without the SD card
//
// Returns	true - continue operating without the SD card
//			false - we're powering off, sequence already begun - do nothing and wait to die
//
int SDCardRemoved(void)
{

static const GUI_WIDGET_CREATE_INFO SDCardRemovedResources[] = {
	{ FRAMEWIN_CreateIndirect, "SD CARD REMOVED!", 0, 20, 20, 400, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Cannot record further data without card", 0, 0, 30, 400, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "What would you like to do?", 0, 0, 70, 400, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "POWER OFF", GUI_ID_OK, 20, 115, 150, 60 },
	{ BUTTON_CreateIndirect, "RUN WITHOUT", GUI_ID_CANCEL, 190, 115, 180, 60 }
};

	HideSprites();
	
	SetPopupDefaults(ERROR_POPUP);

	glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed

	if (GUI_ExecDialogBox(SDCardRemovedResources, GUI_COUNTOF(SDCardRemovedResources), &popupCallback, WM_GetDesktopWindow(), 10, 10) == GUI_ID_OK)
	{
		UnhideSprites();
		return false;
	}
	else
	{
		UnhideSprites();
		return true;
	}
}

//
// SD card inserted while running without the SD card
// Display a modal dialog box to ask user if he wants to reset the unit so he can use this card
// or ignore the card and continue to run without it
//
// Returns	true - continue operating without the SD card
//			false - we're resetting, sequence already begun - do nothing and wait to die
//
int SDCardReinserted(void)
{

static const GUI_WIDGET_CREATE_INFO SDCardReinsertedResources[] = {
	{ FRAMEWIN_CreateIndirect, "SD CARD INSERTED!", 0, 20, 20, 420, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Must reset unit to start using this card", 0, 0, 50, 420, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "RESET UNIT", GUI_ID_OK, 120, 115, 140, 60 },
};

	HideSprites();
	
	SetPopupDefaults(ERROR_POPUP);

	glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed

	if (GUI_ExecDialogBox(SDCardReinsertedResources, GUI_COUNTOF(SDCardReinsertedResources), &popupCallback, WM_GetDesktopWindow(), 10, 10) == GUI_ID_OK)
	{
#ifndef _WINDOWS
		reset();			// Reset immediately
#endif
		UnhideSprites();
		return false;
	}
	else
	{
		UnhideSprites();
		return true;
	}
}

//
// Display a modal dialog box to see if the user wants to reflash his SU or DU2
//
// Returns	YES - proceed to reflash
//			NO - skip and go on
//
int ReFlashYorN(codefiletypes codetype, int rev) {

	static GUI_WIDGET_CREATE_INFO NewFirmwareResources[] = {
		{ FRAMEWIN_CreateIndirect, "", 0, 50, 20, 360, 220, 0, 0 },
		{ BUTTON_CreateIndirect, "UPDATE",		GUI_ID_OK,		40, 115, 120, 60 },
		{ BUTTON_CreateIndirect, "SKIP", GUI_ID_CANCEL, 190, 115, 120, 60 },
		{ TEXT_CreateIndirect, "", 0, 0, 10, 360, 30, TEXT_CF_HCENTER },
		{ TEXT_CreateIndirect, "", 0, 0, 40, 360, 30, TEXT_CF_HCENTER },
		{ TEXT_CreateIndirect, "Do Not Disconnect or Power Down", 0, 0, 70, 360, 30, TEXT_CF_HCENTER }
	};
	char tempstr[40];
	int oldrev;
	
	SetPopupDefaults(PROGRAMMING_POPUP);

	NewFirmwareResources[3].pName = tempstr;

	switch (codetype) {
		case CODESU:
			oldrev = suData.suSwRev;
			NewFirmwareResources[0].pName = "NEW SU FIRMWARE FOUND";
			NewFirmwareResources[4].pName = "Update Connected SU Firmware?";
			break;
		case CODETQ:
			oldrev = unsavedSysData.duSwRev;
			NewFirmwareResources[0].pName = "NEW FIRMWARE FOUND";
			NewFirmwareResources[4].pName = "Update TraqDash Firmware?";
			break;
		case BOOTTQ:
			oldrev = unsavedSysData.unitInfo.bootloaderSwRev;
			NewFirmwareResources[0].pName = "NEW BOOTLOADER FOUND";
			NewFirmwareResources[4].pName = "Update TraqDash Bootloader?";
			break;
	} // switch

	sprintf(tempstr, "Current Rev %1.2f, New Rev %1.2f", ((float) oldrev / 100.0), ((float) rev / 100.0));
	
	if (GUI_ID_OK == GUI_ExecDialogBox(NewFirmwareResources, GUI_COUNTOF(NewFirmwareResources), &popupCallback, WM_GetDesktopWindow(), 10, 10))
	{
#ifndef _WINDOWS
		StopTouchScreenScan();
#endif
		return true;
	}
	else return false;
}

static const GUI_WIDGET_CREATE_INFO ShuttingDownResources[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Saving Files", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
};
static const GUI_WIDGET_CREATE_INFO SafeToPowerOffUnit[] = {
	{ FRAMEWIN_CreateIndirect, "STATUS", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "All files saved.", 0, 0, 30, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Powering off the unit.", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
};

//
// Controlled power down sequence - when we have pleanty of time to inform the user what we're doing
//
void PowerOffUnit(WM_HWIN owner, char reset_unit, int delay)
{
	WM_HWIN hDialog;
	
	if (unsavedSysData.systemMode != DU2_IDLE)
		ExitRecord(true);

	HideSprites();

	glbBlackoutFlag = true;		// Suspend gauge screen updates while the popup dialog box is displayed
	SetPopupDefaults(INFORMATIONAL_POPUP);

	// Display "Saving Files"
	hDialog = GUI_CreateDialogBox(ShuttingDownResources, GUI_COUNTOF(ShuttingDownResources), &nonModalPopupCallback, owner, 10, 10);
	GUI_Exec();					// Allow popup to be shown

	SaveFilesAndPowerDown();	// Save the files

	if (!delay)					// If no particular delay was requested at least wait 1ms before shutting down
		delay = 1;
	GUI_Delay(delay);			// give time for messages to be processed before displaying that it is safe to remove power

	WM_DeleteWindow(hDialog);	// Delete "Saving Files" popup
	
#ifdef _WINDOWS
	// Create non-modal dialog box in windows to allow the power key to work
	GUI_CreateDialogBox(SafeToPowerOffUnit, GUI_COUNTOF(SafeToPowerOffUnit), &nonModalPopupCallback, owner, 10, 10);
#else
	GUI_CreateDialogBox(SafeToPowerOffUnit, GUI_COUNTOF(SafeToPowerOffUnit), &nonModalPopupCallback, owner, 10, 10);
	GUI_Exec();				// Wait to die
	if (reset_unit)
		reset();			// Reset the unit
	// just wait for powerdown
	OS_Delay(60000);
#endif
}

//
// Essential core function of the power down - in case of emergency power down call this directly!
//
void SaveFilesAndPowerDown(void)
{
	// tell comm and laptimer to stop recording and shut off power at SU
	SetUserMode(DU2_SHUTDOWN);
	
	// save configuration (use synchronous calls, not event posting!)
	WriteUserDataEvent(SYSTEM_DATA);

// COMMENTED OUT BECAUSE THESE SHOULD ALREADY BE UP TO DATE  --  GAS
//	if (trackTableIndex >= 0)
//		if (trackTable[trackTableIndex].sdCardEntry)	// Only write if a disk file already exists
//			WriteUserDataEvent(TRACK_DATA);
//	WriteUserDataEvent(VEHICLE_DATA);
//	WriteUserDataEvent(DRIVER_DATA);
						
#ifndef _WINDOWS
	// Unmount all drives (flushes cache)
	FS_Unmount("");
#endif

}

#ifndef _WINDOWS
void ConfigureCache(void)
{
	static U32 *fileCache = (U32 *) START_OF_ISRAM;

	// **** GAS TEST -- these are not ideal but done to test file system speedup
//	FS_ConfigUpdateDirOnWrite( 0 );										// do not update directory after each write for speed
	FS_SetFileWriteMode( FS_WRITEMODE_FAST );							// not safe but allegedly fast
//	FS_SetFileWriteMode( FS_WRITEMODE_SAFE );							// does not work well. dropouts. crashes if you remove and reinsert card
	FS_AssignCache("", fileCache, FS_CACHE_SIZE, FS_CACHE_RW);			// use a cache to speed things up
	FS_CACHE_SetMode("", FS_SECTOR_TYPE_MASK_ALL, FS_CACHE_MODE_WB);
}

char OkToUseSDCard(void)
{
	return okToUseSDCard;
}
#endif

void InitializeTraqmateFiles(void)
{
#ifdef _WINDOWS
	FILE *pTestFile;

	ConfigureHomeDirectory();

	if ((pTestFile = fopen("544729.sno", "r")) != NULL)		// serial number mode allows serial and build date to be set
	{
		fclose(pTestFile);
		unsavedSysData.enableSerialNumber = true;
	}
	else unsavedSysData.enableSerialNumber = false;
	
	if ((pTestFile = fopen("testmode", "r")) != NULL)
	{
		fclose(pTestFile);
		testMode = true;
	}
	else testMode = false;

	if ((pTestFile = fopen("tempcal", "r")) != NULL)			// operator ability to calibrate LED temperature correction value
	{
		fclose(pTestFile);
		unsavedSysData.enableTempCalibration = true;
	}
	else unsavedSysData.enableTempCalibration = false;

	LoadPersonalizationData();

	InitializeDirectoryStructure();

#else
#ifdef BUILD_DEBUG_PORT
	char tempstr[60];
#endif
	U8 attr;
	
	InitializeDirectoryStructure();
			
	// check for special mode files or folders present
	unsavedSysData.enableSerialNumber = (0xFF != FS_GetFileAttributes("\\544729.sno"));		// serial number mode allows serial and build date to be set
	testMode = (0xFF != FS_GetFileAttributes("\\testmode"));		// test mode adds extra screen
	demoMode = (0xFF != FS_GetFileAttributes("\\demomode"));		// demo mode simulates operation
	factMode = (0xFF != FS_GetFileAttributes("\\factmode"));		// factory checkout mode
	unsavedSysData.enableTempCalibration = (0xFF != FS_GetFileAttributes("\\tempcal"));	// operator ability to calibrate LED temperature correction value
//	unsavedSysData.wifiCardInserted = (0xFF != FS_GetFileAttributes(WIFI_FILE));	// Toshiba FlashAir card inserted
	attr = FS_GetFileAttributes("\\SD_WLAN\\CONFIG");
	unsavedSysData.wifiCardInserted = (0xFF != attr);
//	unsavedSysData.wifiCardInserted = (0xFF != FS_GetFileAttributes("\\SD_WLAN\\CONFIG"));	// Toshiba FlashAir card inserted

	// Configure WiFi SD card if necessary
	if (unsavedSysData.wifiEnabled && unsavedSysData.wifiCardInserted)
		WiFiSetup(false);
	
	if (factMode)
		FactoryCheckout();
	
#ifdef BUILD_DEBUG_PORT
	sprintf(tempstr, "TraqDash SW %3.2f, HW %3.2f, DATA %d !\n\r", ((float) unsavedSysData.duSwRev)/100.0, ((float) unsavedSysData.duHwRev)/100.0, unsavedSysData.duDataRev);
	DBGU_Print(tempstr);
#endif

#endif

	// Check for too many session files - move to archive folder if approved
	ArchiveSessions();

	// Check to see if new software version and update the file
	if (NewSWVersion())
	{
		// See if the user wants to verify the track SF lines
		if (UserWantsToUpdateSF())
		{
			// Update start finish lines
			UpdateSFLines();
		}
	}

	InitializeTraqmateData();
}


void local_itoa(int n, char s[])
{
	int i;

	i = 0;
	do {       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	s[i] = '\0';
	reverse(s);
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

//
//See if the current software version is different from the last version run
// Update the file to the current version when done
//
// Returns: true = new software version or the version file did not previously exist
//			false = same software version as before
// file updated to current rev in both cases
//
#ifdef _WINDOWS
int NewSWVersion(void)
{
	HFILE hFile;
	OFSTRUCT fStruct;
	DWORD bytesReadWritten;
	int version = DU2_SW_REV;
	char newVersionStr[10];

	// Build current version string
	local_itoa(version, newVersionStr);

	if ((hFile = OpenFile(VERSION_PATH, &fStruct, OF_READWRITE)) != HFILE_ERROR)
	{
		char oldVersionStr[10];

		// Read file to get last version
		if (ReadFile((HANDLE)hFile, &oldVersionStr, sizeof(oldVersionStr), &bytesReadWritten, NULL) == TRUE)
		{
			// Put a NULL at the end of the string read so we can compare strings
			oldVersionStr[bytesReadWritten] = 0;

			// Compare old version with new version
			if (strcmp(newVersionStr, oldVersionStr) == 0)	// if strings the same
			{
				CloseHandle((HANDLE)hFile);
				return false;
			}
			else
			{
				// Else file old version - update and return indicating wrong version
				LONG dist;

				dist = 0;
				SetFilePointer((HANDLE)hFile, dist, NULL, FILE_BEGIN);		// Point back to start of file
				WriteFile((HANDLE)hFile, newVersionStr, strlen(newVersionStr), &bytesReadWritten, NULL);
				CloseHandle((HANDLE)hFile);
				return true;
			}
		}
		else
		{
			CloseHandle((HANDLE)hFile);
			return true;
		}
	}
	else
	{
		// else file does not exist - make it
		if ((hFile = OpenFile(VERSION_PATH, &fStruct, OF_CREATE)) != HFILE_ERROR)
		{
			// else file old version - update and return indicating wrong version
			WriteFile((HANDLE)hFile, newVersionStr, strlen(newVersionStr), &bytesReadWritten, NULL);
			CloseHandle((HANDLE)hFile);
		}
	}
	return true;
}
#else
// Target hardware version
int NewSWVersion(void)
{
	FS_FILE *pFile;
	int version = DU2_SW_REV;
	char newVersionStr[10];

	// Build current version string
	local_itoa(version, newVersionStr);

	if ((pFile = FS_FOpen(VERSION_PATH, "r+")) != NULL)
	{
		U32 bytesRead;
		char oldVersionStr[10];

		// Read file to get last version
		if ((bytesRead = FS_Read(pFile, &oldVersionStr, sizeof(oldVersionStr))) != 0)
		{
			// Put a NULL at the end of the string read so we can compare strings
			oldVersionStr[bytesRead] = 0;

			// Compare old version with new version
			if (strcmp(newVersionStr, oldVersionStr) == 0)	// if strings the same
			{
				FS_FClose(pFile);
				return false;
			}
			else
			{
				// Else file old version - update and return indicating wrong version
				FS_SetFilePos(pFile, 0, FS_FILE_BEGIN);			// Point back to start of file
				FS_Write(pFile, newVersionStr, strlen(newVersionStr));
				FS_FClose(pFile);
				return true;
			}
		}
		else
		{
			FS_FClose(pFile);
			return true;
		}
	}
	else
	{
		// else file does not exist - make it
		if ((pFile = FS_FOpen(VERSION_PATH, "w")) != NULL)
		{
			// Else file old version - update and return indicating wrong version
			FS_Write(pFile, newVersionStr, strlen(newVersionStr));
			FS_FClose(pFile);
		}
	}
	return true;
}
#endif

// See if the user wants to verify the track SF lines
int UserWantsToUpdateSF(void)
{
	int retval;

	static const GUI_WIDGET_CREATE_INFO CheckUpdateTracks[] = {
		{ FRAMEWIN_CreateIndirect, "New Software Detected", 0, 20, 20, 400, 220, 0, 0 },
		{ TEXT_CreateIndirect, "New Track Definitions. Would you like", 0, 0, 30, 400, 30, TEXT_CF_HCENTER },
		{ TEXT_CreateIndirect, "to update Start/Finish coordinates?", 0, 0, 70, 400, 30, TEXT_CF_HCENTER },
		{ BUTTON_CreateIndirect, "YES", GUI_ID_YES, 60, 115, 100, 60 },
		{ BUTTON_CreateIndirect, "NO", GUI_ID_NO, 240, 115, 100, 60 },
	};

	SetPopupDefaults(WARNING_POPUP);

	if (GUI_ExecDialogBox(CheckUpdateTracks, GUI_COUNTOF(CheckUpdateTracks), &popupCallback, WM_GetDesktopWindow(), 10, 10) == GUI_ID_YES)
		retval = true;
	else retval = false;

	DisplaySplashScreen1();
	return retval;
}


void InitializeTraqmateData(void)
{
	int retval;
	
	CreateTracks();					// Initialize trackTable

	if ((retval = ReadDataStructures()) != 0)
#ifndef _WINDOWS
		if (unsavedSysData.sdCardInserted)			// Only tell the user about missing files if the SD card is actually present!
#endif
			DisplayInvalidFilesPopup(retval);

	ResetAllFileLists(false);		// Reset file lists without calling CreateTracks
}

//
// Invalid files found at startup
// Tell user what failed
//
char fileList[100];

void DisplayInvalidFilesPopup(int files)
{
	char anyItemSet = false;

static const GUI_WIDGET_CREATE_INFO InvalidFilesResources[] = {
	{ FRAMEWIN_CreateIndirect, "Check Your Settings", 0, 30, 20, 400, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The following configuration(s)", 0, 0, 10, 390, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "were set to defaults:",    0, 0, 40, 390, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 145, 115, 100, 60 },
};

	SetPopupDefaults(WARNING_POPUP);

	glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed

	systemSettingsPopupScreen = INVALID_FILES;

	fileList[0] = 0;
	if (files & INVALID_SYSTEM_DATA)
	{
		strcat(fileList, "System");
		anyItemSet = true;
	}
	if (files & INVALID_VEHICLE_DATA)
	{
		if (anyItemSet)
			strcat(fileList, ", ");
		else anyItemSet = true;
		strcat(fileList, "Vehicle");
	}
	if (files & INVALID_TRACK_DATA)
	{
		if (anyItemSet)
			strcat(fileList, ", ");
		else anyItemSet = true;
		strcat(fileList, "Track");
	}
	if (files & INVALID_DRIVER_DATA)
	{
		if (anyItemSet)
			strcat(fileList, ", ");
		else anyItemSet = true;
		strcat(fileList, "Driver");
	}

	GUI_ExecDialogBox(InvalidFilesResources, GUI_COUNTOF(InvalidFilesResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
}

void AskAndDoFormatSDCard(void)
{
	// SD card not formatted
	WM_HWIN hWin;

	if (SystemSettingsPopUp(INVALID_SDCARD, hWin = WM_GetDesktopWindow()))	// Tell the user the SD card is invalid
		if (SystemSettingsPopUp(FORMAT_SDCARD, hWin))						// See if the user is willing to format this SD card
			FormatSDCard(hWin);												// yes, yes - format the card!
}

void CreateProgressBar( PROGBAR_Handle *hProgBar, int maxval) {
	*hProgBar = PROGBAR_CreateEx( 90, 170, 300, 70, 0, WM_CF_SHOW, PROGBAR_CF_HORIZONTAL, 1);
	PROGBAR_SetFont(*hProgBar, &GUI_Font32_ASCII);
	PROGBAR_SetTextColor(*hProgBar, 0, GUI_BLACK);
	PROGBAR_SetTextColor(*hProgBar, 1, GUI_WHITE);
	PROGBAR_SetMinMax(*hProgBar, 0, maxval);
	PROGBAR_SetBarColor(*hProgBar, 0, GUI_GREEN);
	PROGBAR_SetBarColor(*hProgBar, 1, GUI_BLACK);
	WIDGET_SetEffect(*hProgBar, &WIDGET_Effect_3D);
}

#ifndef _WINDOWS
void WriteCodeImage(void) {
	FS_FILE *pFile;								// file handle
	char pagebuff[MAX_PAGE_64MBIT];				// max size for flash buffer
	unsigned long bytesread, totalbytes;		// progress counters
	u16 pagenum;								// which flash page are we writing
	int rev;									// which rev is code data file
	PROGBAR_Handle hProgBar;					// progress bar
#ifdef BUILD_DEBUG_PORT
	char tempstr[60];							// for writing debug messages
#endif
	
#define TRAQDASHCODENAME		"tdfirm"		// traqdash firmware
#define TRAQDASHBOOTNAME		"tdboot"		// traqdash bootloader
#define SUCODENAME				"sufirm"		// su firmware
#define CODENAMESIZE			6				// length of strings above

#define TRAQMATECODEEXT			"tqc"			// application code
#define TRAQMATEBOOTEXT			"tqb"			// bootloader code
#define EXTNAMESIZE				3				// length of strings above

#define CODEFILENAMESIZE	(CODENAMESIZE + 3 + 1 + EXTNAMESIZE)		// file name size of code files
									// valid code update filenames in format AAAAAANNN.TQC
									// where AAAAA is TDCODE, TDBOOT, or SUCODE
									// NNN is 3 digit number
									// TQC is extension for all Traqmate code
#define CODEFILENAMEDOT		(CODEFILENAMESIZE - 4)
#define CODEFILENAMEEXT		(CODEFILENAMEDOT + 1)
#define CODEFILENAMEDIGITS	(CODEFILENAMEDOT - 3)

FS_FIND_DATA findstruct;
char filename[MAX_FILENAME_SIZE];		// where to stash the filename
typedef struct {
	char name[CODEFILENAMESIZE+1];
	int rev;
} codefilestruct;

codefilestruct codefiles[NOMATCH] =	{				// 0 = su code, 1 = traqdash boot, 3 = traqdash code
	{ "", 0 },
	{ "", 0 },
	{ "", 0 } };

codefiletypes whichcodefile;

	// step through directory looking for code files
	if (0 == FS_FindFirstFile(&findstruct, "\\", filename, MAX_FILENAME_SIZE)) {

		do {	// process file filename
			if ((CODEFILENAMESIZE == strlen(filename)) && ('.' == filename[CODEFILENAMEDOT])) {
				filename[CODEFILENAMEDOT] = '\0';

				rev = atoi(filename + CODEFILENAMEDIGITS);

				if (0 != rev) {		// no match
					int i;
					
					for (i=0; i < CODEFILENAMESIZE; i++) {
						filename[i] = tolower(filename[i]);		// go to lowercase for comparison
					} // for
	
					whichcodefile = NOMATCH;
					if (0 == strncmp(TRAQMATECODEEXT, filename + CODEFILENAMEEXT, EXTNAMESIZE)) {
						// see if it matches one of the ones we need
						if (0 == strncmp(TRAQDASHCODENAME, filename, CODENAMESIZE))
							whichcodefile = CODETQ;
						else if (0 == strncmp(SUCODENAME, filename, CODENAMESIZE))
							whichcodefile = CODESU;
					} // if
					else if (0 == strncmp(TRAQMATEBOOTEXT, filename + CODEFILENAMEEXT, EXTNAMESIZE)) {
						if (0 == strncmp(TRAQDASHBOOTNAME, filename, CODENAMESIZE))
							whichcodefile = BOOTTQ;
					} // else if
					
					if (NOMATCH != whichcodefile) {
						if (rev > codefiles[whichcodefile].rev) {					// only remember the latest of each type
							filename[CODEFILENAMEDOT] = '.';						// put the dot back
							strcpy(codefiles[whichcodefile].name, filename);		// save filename for later
							codefiles[whichcodefile].rev = rev;
						} // if
					 } // if
				} // if
			} // if
			
		} while (0 != FS_FindNextFile(&findstruct));
	} // if

	// clean up
	FS_FindClose(&findstruct);

	// process the files that we found
	for (whichcodefile = CODESU; whichcodefile < NOMATCH; whichcodefile++) {
		if (0 != codefiles[whichcodefile].rev) {
			switch (whichcodefile) {
				case CODESU: {

					if (0 != suData.suSwRev) {					// only attempt SU reflash if we have made contact with SU
						if (suData.suSwRev < codefiles[whichcodefile].rev) {		// and make sure SU code is newer

							// give SU a chance to finish boot up
							OS_Delay(2500);

							// give user chance to back out
							if (!ReFlashYorN(CODESU, codefiles[whichcodefile].rev))
								break;
#ifdef BUILD_DEBUG_PORT
							sprintf(tempstr, "SU code program, file = %s, rev = %d \n\r", codefiles[whichcodefile].name, codefiles[whichcodefile].rev);
							DBGU_Print(tempstr);
#endif
							// change mode and notify tasks
							SetUserMode(DU2_REPROGRAMMING_SU);
							
							// Attempt to read SD card code binary file
							if (NULL == (pFile = FS_FOpen(codefiles[whichcodefile].name, "r")))
								break;
							else {		// opened the file						
								char destaddr;
								char cksum;
								BOOL downloaderror = FALSE;

								CreateProgressBar(&hProgBar, NUMBER_OF_DOWNLOAD_MSGS);							
								OS_Delay(1000);					// wait for things to quiet down
								
								PROGBAR_SetText(hProgBar, "Sending");
								GUI_Exec();

								// get the first byte = destination
								FS_Read(pFile, &destaddr, 1);
								
								// tell SU to start the reprogramming process
								com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = (unsigned char) START_DOWNLOAD; // change operating mode
								SendMessage((u08) DISPLAY_UNIT, destaddr, (u08) MODE_CHANGE, 1);
								WAIT_ACK(destaddr);
							
								for (pagenum = 0; pagenum < NUMBER_OF_DOWNLOAD_MSGS; pagenum++) {
									// read each page and send it over
									if (CODE_DOWNLOAD_SIZE != FS_Read(pFile, &(com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START]), CODE_DOWNLOAD_SIZE)) {
										downloaderror = TRUE;
										break;
									} // if
							
									// send a 512 byte chunk over to SU
									SendMessage((u08) DISPLAY_UNIT, destaddr, (u08) SW_DOWNLOAD, CODE_DOWNLOAD_SIZE);	
									WAIT_ACK(destaddr);

									PROGBAR_SetValue(hProgBar, pagenum);		// update the user
									GUI_Exec();
								} // while
							
								if (downloaderror) {								
									PROGBAR_SetText(hProgBar, "Error");
									GUI_Exec();
									// tell SU to cancel the reprogramming process
									SendMessage((u08) DISPLAY_UNIT, destaddr, (u08) CANCEL_DOWNLOAD, 0);	
									WAIT_ACK(destaddr);
								} // if
								else {
									// tell SU that download is finished
									com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = (unsigned char) DOWNLOAD_COMPLETE;
									SendMessage((u08) DISPLAY_UNIT, destaddr, (u08) MODE_CHANGE, 1);	
									WAIT_ACK(destaddr);
									
									// get the checksum from the file
									FS_Read(pFile, &cksum, 1);
									
									// sent all the sectors, now send the checksum
									com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = cksum;
									SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) SW_REPROGRAM, 1);	
									WAIT_ACK(destaddr);
								} // else
							} // else
#ifdef BUILD_DEBUG_PORT
							sprintf(tempstr, "SU reprogram complete\n\r");
							DBGU_Print(tempstr);
#endif			
							FS_FClose(pFile);

							PROGBAR_SetValue(hProgBar, 0);		
							PROGBAR_SetText(hProgBar, "Wait for SU to Turn Off");
							GUI_Exec();

							// give user something to look at while SU is reflashing
							for (pagenum=0; pagenum <= NUMBER_OF_DOWNLOAD_MSGS; pagenum++) {
								OS_Delay(350);		// 300 is about right. 350 is extra slow to ensure completion
								PROGBAR_SetValue(hProgBar, pagenum);
								GUI_Exec();						
							} // for
							
							// change mode and notify tasks
							SetUserMode(DU2_STARTUP);

							// just wait for powerdown
							OS_Delay(60000);
						} // if
					} // if
					
					break;
				} // case
				case BOOTTQ:
				case CODETQ: {
					unsigned int pagemismatch;
					unsigned char status;
					unsigned int firstpage;

					// Attempt to read SD card code binary file
					if (NULL == (pFile = FS_FOpen(codefiles[whichcodefile].name, "r")))
						break;
					else {		// opened the file
						u16 rangeupper, partialrange, lastpartial;

						if (BOOTTQ == whichcodefile) {		// TraqDash bootloader
							if (unsavedSysData.unitInfo.bootloaderSwRev >= codefiles[whichcodefile].rev)
								break;				// don't update if we don't need to
							
							if (!ReFlashYorN(BOOTTQ, codefiles[whichcodefile].rev))	// ask user if they want to do it
								break;
							
							firstpage = 0;
							rangeupper = CODELOAD(0)-2;		// don't reflash the serial number at top of bootloader memory space	
#ifdef BUILD_DEBUG_PORT
							DBGU_Print("Erasing Boot Area \n\r");
#endif
						} // if
						else {						// TraqDash Code
							if (unsavedSysData.duSwRev >= codefiles[whichcodefile].rev)
								break;				// don't update if we don't need to
							
							if (!ReFlashYorN(CODETQ, codefiles[whichcodefile].rev))	// ask user if they want to do it
								break;

							firstpage = CODELOAD(0);
							rangeupper = unsavedSysData.dataflash[0].numPages-1;								
#ifdef BUILD_DEBUG_PORT
							DBGU_Print("Erasing Code Area \n\r");
#endif						
						} // else
						
						// tell tasks to leave us alone
						SetUserMode(DU2_REPROGRAMMING);

						CreateProgressBar(&hProgBar, rangeupper);							
						OS_Delay(500);					// wait for things to quiet down
	
						totalbytes = 0;
						
						PROGBAR_SetText(hProgBar, "Erasing");
						GUI_Exec();
						
						partialrange = firstpage;
						do {
							lastpartial = partialrange;
							partialrange += 256;				// clear in 256 chunks for progress bar
													
							if (partialrange > rangeupper)
								partialrange = rangeupper+1;
							
							DataFlash_Erase_Range(0, lastpartial, partialrange-1);
							
							PROGBAR_SetValue(hProgBar, partialrange);		// update the user
							GUI_Exec();
						} while (partialrange < rangeupper);

						PROGBAR_SetText(hProgBar, "Writing");
						PROGBAR_SetValue(hProgBar, 0);						// clear the progress bar
						GUI_Exec();
						
						pagenum = firstpage;
						if (CODETQ == whichcodefile) {
							// just in case code is not going at an even page boundary
							pagemismatch = CODESTART % unsavedSysData.dataflash[0].bytesPerPage;
							
							if (0 != pagemismatch) {
								int i;
	
								// fill start of buffer with blank flash bytes
								for (i=0; i < pagemismatch; i++)
									pagebuff[i] = 0xFF;
								// read a partial
								bytesread = FS_Read(pFile, &(pagebuff[pagemismatch]), (unsavedSysData.dataflash[0].bytesPerPage - pagemismatch));
								// write the first page
								DataFlash_Page_Write(0, (1 + (pagenum % 1)), unsavedSysData.dataflash[0].bytesPerPage, pagebuff, pagenum);
	
								pagenum++;
								totalbytes += bytesread;
							} // if
						} // if
						
						while (0 !=	(bytesread = FS_Read(pFile, &pagebuff, unsavedSysData.dataflash[0].bytesPerPage))) {
							unsigned int *iptr;

							// store size of bootloader in image at 0x0014. Use 8000 for now.
							if ((BOOTTQ == whichcodefile) && (0 == pagenum)) {
								iptr = (unsigned int *) (pagebuff + 0x14);
								*iptr = 8000;				// store size of bootloader
							} //if
						
							if (bytesread != unsavedSysData.dataflash[0].bytesPerPage) {
#ifdef BUILD_DEBUG_PORT
								sprintf(tempstr, "bytes left over = %ld \n\r", bytesread);
								DBGU_Print(tempstr);
#endif		
							} // if
								
							DataFlash_Page_Write(0, (1 + (pagenum % 1)), unsavedSysData.dataflash[0].bytesPerPage, pagebuff, pagenum);

							if (0 == (pagenum % 256)) {						
								PROGBAR_SetValue(hProgBar, pagenum);		// update the user
								GUI_Exec();
#ifdef BUILD_DEBUG_PORT
								sprintf(tempstr, "Writing page = %d \n\r", pagenum);
								DBGU_Print(tempstr);
#endif								
							} // if

							pagenum++;
							totalbytes += bytesread;
						} // while
							
						PROGBAR_SetValue(hProgBar, rangeupper);		// show 100%
						GUI_Exec();
						
						// make sure it is finished
						do {
							status = DataFlash_Read_Status(0);
						} while (!(status & STATUS_READY_MASK));
									
#ifdef BUILD_DEBUG_PORT
						sprintf(tempstr, "Write complete, pages = %d, total bytes = %ld \n\r", pagenum, totalbytes);
						DBGU_Print(tempstr);
#endif			
						FS_FClose(pFile);

						// if bootloader, erase it from SD card and write new bootloader rev into flash
						if (BOOTTQ == whichcodefile) {
#ifdef BUILD_DEBUG_PORT
							DBGU_Print("Updating bootloader rev\n\r");
#endif
							unsavedSysData.unitInfo.bootloaderSwRev = codefiles[whichcodefile].rev;
							WriteUserData();
#ifdef BUILD_DEBUG_PORT
							DBGU_Print("Removing bootloader file \n\r");
#endif
							FS_Remove(codefiles[whichcodefile].name);
						} // if
						
						FS_Sync("");
						
						unsavedSysData.systemMode = DU2_IDLE;
						
						// reboot
#ifdef BUILD_DEBUG_PORT
						DBGU_Print("Rebooting \n\r");
#endif
						OS_Delay(3000);
						reset();			// reset to reload

					} // else
					break;
				} // case
			} // switch
		} // if
	} // for
}
#endif

#define	ICON_ROW	190
#define	TEXT_ROW	250
#define	ICON_START	5
#define	INCREMENT	68
#define	TEXT_START	35

const GUI_RECT DriverRect = {100, 107, 230, 180};
const GUI_RECT VehicleRect = {330, 107, 460, 180};

void RepaintSplash(void)
{
	DisplaySplashScreen1();
// Do not want to do here - causes issues with Archive Popup sequence	DisplaySplashScreen3();
}

void DisplaySplashScreen1(void)
{
	GUI_Clear();
	GUI_DrawBitmap(&bmSplashScreen, 39, 10);
	GUI_DrawBitmap(&bmDriverName, 20, 97);
	GUI_DrawBitmap(&bmVehicleName, 240, 110);
}

void DisplaySplashScreen3(void)
{
	const GUI_BITMAP *pGraphic;
	int i;
	unsigned int temp;

	// Start screen persistance timer
	splashScreenStartTime = GUI_GetTime();

	temp = sysData.multiSettings_2 & 0x0000000F;

	// If on main menu and one of first 4 times to power up - draw help splash screen
	if ((temp < HELP_SPLASH_COUNT) && (sysData.screentype == GENERAL_CONFIG_SCREEN_TYPE))
	{
		if (++temp >= HELP_SPLASH_COUNT)
			temp = HELP_SPLASH_COUNT;
		sysData.multiSettings_2 = (sysData.multiSettings_2 & 0xFFFFFFF0) | temp;

		DrawJPEGSplash();
	}
	else
	{
		GUI_SetColor(GUI_WHITE);
		GUI_SetFont(&GUI_Font24_ASCII);
		GUI_DispStringInRectWrap(sysData.driver, (GUI_RECT *)&DriverRect, GUI_TA_LEFT | GUI_TA_VCENTER, GUI_WRAPMODE_WORD);
		GUI_DispStringInRectWrap(sysData.vehicle, (GUI_RECT *)&VehicleRect, GUI_TA_LEFT | GUI_TA_VCENTER, GUI_WRAPMODE_WORD);

		// Display inputs and camera configuration graphics
		GUI_SetFont(&GUI_Font16B_ASCII);
		if (GetSUCam() != CAMERA_NONE)
		{
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringHCenterAt("Camera", TEXT_START, TEXT_ROW);

			switch (GetSUCam())
			{
			case CAMERA_SONY_CAMCORDER:
			case CAMERA_SONY_VCR_HARD:
			case CAMERA_SONY_VCR_SOFT:
			case CAMERA_SONY_HD:			pGraphic = &bmSonyIcon;			break;
			case CAMERA_RACECAM_HD:			pGraphic = &bmRaceCamIcon;		break;
			case CAMERA_CHASECAM_LANC:
			case CAMERA_CHASECAM:			pGraphic = &bmChaseCamIcon;		break;
			case CAMERA_GOPRO_HD_PRO:
			case CAMERA_GOPRO_HD:			pGraphic = &bmGoProIcon;		break;
			case CAMERA_REPLAY_HD:			pGraphic = &bmReplayIcon;		break;
			case CAMERA_REPLAY_XD_USB:		pGraphic = &bmReplay1080Icon;	break;	// XD 1080 connected via TraqData USB
			case CAMERA_REPLAY_MINI:		pGraphic = &bmReplayMiniIcon;	break;	// Mini connected via TraqData USB
			case CAMERA_REPLAY_PRIMEX:		pGraphic = &bmReplayPrimeIcon;	break;	// Prime X connected via TraqData USB
			case CAMERA_MOBIUS:				pGraphic = &bmMobiusIcon;		break;	// Mobius connected via TraqData USB

			default:						pGraphic = &bmCheckIcon;		break;
			}
		
			GUI_DrawBitmap(pGraphic, ICON_START, ICON_ROW);

			if ((GetTDCam() != TD_CAM_NONE) && (unsavedSysData.unitInfo.bootloaderSwRev > FIRST_GEN_BOOTLOADER))
			{
				GUI_SetTextMode(GUI_TM_TRANS);
				GUI_SetColor(GUI_RED);
				GUI_DispStringHCenterAt("USB", ICON_START + 40, ICON_ROW + 40);
			}
		}
		else 
		{
			if (GetTDCam() && (unsavedSysData.unitInfo.bootloaderSwRev > FIRST_GEN_BOOTLOADER))
			{
				GUI_SetColor(GUI_WHITE);
				GUI_DispStringHCenterAt("Camera", TEXT_START, TEXT_ROW);

				GUI_SetFont(&GUI_Font20B_ASCII);
				GUI_DrawBitmap(&bmUSBCamIcon, ICON_START, ICON_ROW);
				GUI_SetTextMode(GUI_TM_TRANS);
				GUI_SetColor(GUI_RED);
				GUI_DispStringHCenterAt("USB", ICON_START + 30, ICON_ROW + 20);
				GUI_SetFont(&GUI_Font16B_ASCII);
			}
			else
			{
				if (vehicleData.digitalInput[0].enabled)
					pGraphic = &bmCheckIcon;
				else pGraphic = &bmXIcon;
				GUI_DrawBitmap(pGraphic, ICON_START, ICON_ROW);
				GUI_SetColor(GUI_LIGHTCYAN);
				GUI_DispStringHCenterAt("Digital 4", TEXT_START, TEXT_ROW);
			}
		}

		GUI_SetColor(GUI_YELLOW);

		for (i = 0; i < 4; i++)
			DisplayAnalogStatus(i);

		if (vehicleData.digitalInput[1].enabled)
			pGraphic = &bmCheckIcon;
		else pGraphic = &bmXIcon;
		GUI_DrawBitmap(pGraphic, ICON_START + (INCREMENT * 5), ICON_ROW);
		GUI_SetColor(GUI_LIGHTCYAN);
		GUI_DispStringHCenterAt("Digital 5", TEXT_START + (INCREMENT * 5), TEXT_ROW);

		// Dislpay WiFi Icon
		if (unsavedSysData.wifiEnabled && unsavedSysData.wifiCardInserted)
			pGraphic = &bmWiFiEnabledIcon;
		else pGraphic = &bmWiFiDisabledIcon;

		GUI_DrawBitmap(pGraphic, ICON_START + (INCREMENT * 6), ICON_ROW);

		GUI_SetColor(GUI_WHITE);
		GUI_DispStringHCenterAt("WiFi", TEXT_START + (INCREMENT * 6), TEXT_ROW);
	}
}


void DisplayAnalogStatus(int i)
{
	char buffer[10];
	const GUI_BITMAP *pGraphic;

	if (vehicleData.analogInput[i].enabled)
		pGraphic = &bmCheckIcon;
	else pGraphic = &bmXIcon;
	GUI_DrawBitmap(pGraphic, ICON_START + (INCREMENT * (i + 1)), ICON_ROW);
	sprintf(buffer, "Analog %d", i);
	GUI_DispStringHCenterAt(buffer, TEXT_START + (INCREMENT * (i + 1)), TEXT_ROW);
}

/****** End of File *************************************************/
