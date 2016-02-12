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
#include <ctype.h>
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
#include "RaceSetupScreens.h"
#include "DataHandler.h"
#include "StandardList.h"
#include "VehicleSettingsScreens.h"
#include "TrackSettingsScreens.h"
#include <string.h>
#ifdef _WINDOWS
#include "..\Application\dirent.h"
#else
#include "FS.h"
#endif
#include "KeyboardAccess.h"
#include "TrackDetect.h"
#include "Utilities.h"

// tmtypes required for some session data definitions.  HOWEVER, including this will screw up
// structure definitions in TrackDetect.h so INCLUDE THIS LAST!!
#include "tmtypes.h"
#include "SFScreens.h"


/*********************************************************************
*
*       Constants and Defines
*
**********************************************************************
*/
#define	NEW_DRIVER_BUTTON			GUI_ID_USER + 1
#define	NEW_VEHICLE_BUTTON			GUI_ID_USER + 2
#define	EDIT_VEHICLE_NAME_BUTTON	GUI_ID_USER + 3
#define	VEHICLE_SETTINGS_BUTTON		GUI_ID_USER + 4
#define	NEW_TRACK_BUTTON			GUI_ID_USER + 5
#define	EDIT_TRACK_NAME_BUTTON		GUI_ID_USER + 6
#define	TRACK_SETTINGS_BUTTON		GUI_ID_USER + 7
#define	EDIT_DRIVER_NAME_BUTTON		GUI_ID_USER + 8
#define	LAPS_BUTTON					GUI_ID_USER + 9
#define	QUALIFYING_BUTTON			GUI_ID_USER + 10
#define	CONFIRM_TRACK_BUTTON		GUI_ID_USER + 11
#define	SELECT_RADIUS_BUTTON		GUI_ID_USER + 12
#define	DELETE_DRIVER_BUTTON		GUI_ID_USER + 13
#define	DELETE_VEHICLE_BUTTON		GUI_ID_USER + 14
#define	DELETE_TRACK_BUTTON			GUI_ID_USER + 15
#define	CANT_DELETE_TRACK			GUI_ID_USER + 16
#define	TRACK_TYPE_BUTTON			GUI_ID_USER + 17
#define	AUTO_CREATE_TRACK_BUTTON	GUI_ID_USER + 18
#define	FILE_ALREADY_EXISTS			GUI_ID_USER + 19
#define	TRACK_ALREADY_EXISTS		GUI_ID_USER + 20

#define DRIVERLIST					GUI_ID_LISTVIEW1
#define VEHICLELIST					GUI_ID_LISTVIEW2
#define	TRACKLIST					GUI_ID_LISTVIEW3

enum MODES {
	EDIT,
	NEW
};

enum TYPES {
	DRIVER,
	VEHICLE,
	TRACK
};


#define MAX_NUM_FILES_IN_DIRECTORY	200
#define	MAX_SESSION_FILES			MAX_NUM_FILES_IN_DIRECTORY
#define	MAX_NUM_LAPS_IN_FILE		200
#define	MAX_NUM_DRIVERS				50
#define	MAX_NUM_VEHICLES			150

#define	POSITION_ONE	51
#define	POSITION_TWO	93
#define	POSITION_THREE	135
#define	POSITION_FOUR	177
#define	POSITION_FIVE	219
#define LISTWIDTH		220
#define LISTHEIGHT		210
#define	LISTXOFFSET		30
#define	LISTYOFFSET		48

const char NullEntry[] = "";
const char NotAvailable[] = "Older File Format.\n\nData Not Available.";

/* The following chart provides an overview of the tables, lists, pointers and indexs associated with track selection
table name		description						format			index			sorted by	pointer			max size				actual size
trackTable		complete list of ALL tracks		TRACK_LIST_TYPE	trackTableIndex	name						NUM_TOTAL_TRACKS		numberOfTracks
trackShortList	index to track table of list	integer							distance					NUM_TOTAL_TRACKS		numberOfShortListTracks
				of ALL track types that are
				within range
							
				list of track names of all 		char **										pTracksFull		numberOfTracks			numberOfTracks
				types and all distances
				list of track names of SELECTED char **										pTracksShort	numberOfShortListTracks	numberOfShortListTracksUser
				track type and within range
							
				copy of either pTracksFull or												pTracks									numTracks (number of tracks currently being presented to the user)
				pTracksShort
*/

/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/
char **pTracksShort = 0;			// Pointer to list of names of tracks of the correct type AND within range
int numberOfShortListTracksUser;	// Number of tracks of the correct type AND within range (numberOfShortListTracksUser <= numberOfShortListTracks)
//char **pTracksFull = 0;
char *pDrivers[MAX_NUM_DRIVERS + 1];
char DriverList[MAX_NUM_DRIVERS + 1][NORMAL_FILENAME_SIZE];
char *pVehicles[MAX_NUM_VEHICLES + 1];
char VehicleList[MAX_NUM_VEHICLES + 1][NORMAL_FILENAME_SIZE];
// KMC 4/1/12 - Changed from dynamic list to static lists for drivers, vehicles and sessions
//char *pSessions[MAX_SESSION_FILES + 1];
//char SessionList[MAX_SESSION_FILES + 1][NORMAL_FILENAME_SIZE];
char **pLaps = 0;
int numLaps;
char sessionName[NORMAL_FILENAME_SIZE];
char fullSessionName[NORMAL_FILENAME_SIZE];
struct NAMESTRUCT NameStruct;
struct NAMESTRUCT SessionNameData[MAX_SESSION_FILES];
long BestLaps[MAX_SESSION_FILES];
tqmheaderstructtype SessionData;	// All Session Data
char vehicleSelectionChanged;
char tempVehicleName[NAMELEN+1];


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
extern GUI_CONST_STORAGE GUI_BITMAP bmDrive;
extern GUI_CONST_STORAGE GUI_BITMAP bmDisplayText;
extern GUI_CONST_STORAGE GUI_BITMAP bmDriverText;
extern GUI_CONST_STORAGE GUI_BITMAP bmInputText;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceReview;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceSetupText;
extern GUI_CONST_STORAGE GUI_BITMAP bmStorage;
extern GUI_CONST_STORAGE GUI_BITMAP bmStorageText;
extern GUI_CONST_STORAGE GUI_BITMAP bmSystemSetup;
extern GUI_CONST_STORAGE GUI_BITMAP bmSystemSetupText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTime;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTrackText;
extern GUI_CONST_STORAGE GUI_BITMAP bmChooseTrack;
extern GUI_CONST_STORAGE GUI_BITMAP bmUnits;
extern GUI_CONST_STORAGE GUI_BITMAP bmUnitsText;
extern GUI_CONST_STORAGE GUI_BITMAP bmVehicleText;
extern GUI_CONST_STORAGE GUI_BITMAP bmDisplay;
extern GUI_CONST_STORAGE GUI_BITMAP bmTraqmateLogo;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoxRally;
extern GUI_CONST_STORAGE GUI_BITMAP bmRace;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlackButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGrayButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmDoubleBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmLapsQualifyText;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenCheck;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmOrangeButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmMediumBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTopBar;
extern GUI_CONST_STORAGE GUI_BITMAP bmBottomBar;
extern GUI_CONST_STORAGE GUI_BITMAP bmShortTopBar;
extern GUI_CONST_STORAGE GUI_BITMAP bmShortBottomBar;
extern GUI_CONST_STORAGE GUI_BITMAP bmListBoxSmall;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlackBlankButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTopFakeButton;

extern GUI_HWIN hDisplayClosestTracks;
extern int modalWindowUp;
#ifndef _WINDOWS
extern char OkToUseSDCard(void);
#endif

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
void DisplayDriverSelection(WM_HWIN);
void DisplayVehicleSelection(WM_HWIN);
void DriverSelectionPaint(WM_HWIN);
void VehicleSelectionPaint(WM_HWIN);
void TrackSelectionPaint(WM_HWIN);
void LapsSelectionPaint(WM_HWIN);
void parseName(char *pDest, char *pSource, enum DATATYPES sel);
int FindSelection(enum DATATYPES sel);
void RSSKillKeyboard(void);
void DisplayKeyboard(int Id);
void TryAgainKeyboard(void);
int CreateNewEntry(void);
int RenameCurrentEntry(void);
int CheckForDuplicateName(void);
enum DATATYPES EditTypeToFileType(enum TYPES editType);
void UpdateList(void);
void SaveBestLap(int index, char *pName);
void ReadAllSessionData(char *pName);
void CreateDummyLapList(void);
void DeleteBestLaps(void);
void InvalidName(void);
char SelectThisTrack(int selection);
int FindFile(char *defaultTrack);
char *GetRadiusText(char *pText);
//char SessionIsRoadRace(char *pName);
#ifdef _WINDOWS
enum SESSION_TYPE parseSessionName(char *pName, struct NAMESTRUCT *NameStruct);
#else
enum SESSION_TYPE parseSessionName(FS_FIND_DATA *fd, struct NAMESTRUCT *NameStruct);
#endif
char *GetNextString(char *pDest, char *pSource);
char *GetNextChar(enum SESSION_TYPE *pDest, char *pSource);
char *GetNextNum(char *pDest, char *pSource);
char ValidSessionType(char ch);
char ValidRaceSession(enum SESSION_TYPE type, char *pName);
char GetSessionCharacter(enum SESSION_TYPE type);
void TooManySessionsPopup(WM_HWIN hParent, char primary);
int RaceSetupPopup(int screen);
void RSDeleteDriver(void);
void RSDeleteVehicle(void);
char RSDeleteTrack(void);
void RSDeleteFile(enum DATATYPES type, char *pName);
void SetTrackTypeButtonText(void);
void UpdateTrackList(WM_HWIN hParent);
void DisplayAlphaList(LISTVIEW_Handle *hList, unsigned int id, char **pList, unsigned int sel);
void AutoCreateNewTrack(void);
void CreateNewTrack(void);
int GetTrackNumber(char *pName, int offset);
int FindLastTrackOfCurrentType(void);
void ValidateName(char *pName);
void DeleteFileList(enum DATATYPES type);
void FinishLoadingTrackData(void);
void CreateSprites(const GUI_BITMAP *pTopBar, const GUI_BITMAP *pBottomBar);
void DeleteSprites(void);
WM_HWIN GetTrackSelectionWindowHandle(void);

/*********************************************************************
*
*       Tables
*
**********************************************************************
*/
struct SCREEN_LIST RaceSetupScreenList[NUM_RACE_SETUP_SCREENS+1] = {
	{ DRIVER_SELECTION, DisplayDriverSelection, DriverSelectionPaint },
	{ VEHICLE_SELECTION, DisplayVehicleSelection, VehicleSelectionPaint },
	{ TRACK_SELECTION, DisplayTrackSelection, TrackSelectionPaint },
	{ LAPSQUALIFYING_SELECTION, DisplayLapsSelection, LapsSelectionPaint },
	{ -1, NULL, NULL }
};


/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hRaceSetupScreens;
static int clickedOnThisScreen;
LISTVIEW_Handle hDriverList;
LISTVIEW_Handle hVehicleList;
LISTVIEW_Handle hTrackList = 0;
BUTTON_Handle hLapsCheck, hQualifyingCheck;
BUTTON_Handle hTrackTypeBtn;
int numDrivers, numVehicles, numTracks, button;
static void *pEntryKeyboard = 0;
enum MODES mode;
enum TYPES editType;
char newName[NAMELEN+1];
static int BestLapNumber[MAX_SESSION_FILES];
int trackTableIndex;		// Global index to the track table of the currently selected track
enum CHOOSE_TRACK_SCREEN_TYPE ctType;
char selectedTrackName[NAMELEN+1];
GUI_HWIN hRSDeleteButton;
GUI_HSPRITE hTop, hBottom;
int hideLevel = 0;


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/




WM_HWIN GetRaceSetupScreensWindowHandle(void)
{
	return hRaceSetupScreens;
}

void SetRaceSetupScreensWindowHandle(WM_HWIN hWin)
{
	hRaceSetupScreens = hWin;
}

void RSSKillKeyboard(void)
{
	if (pEntryKeyboard)		// Should alwaye be there - but just in case!
	{
		DestroyKeyboard(pEntryKeyboard);
		pEntryKeyboard = 0;
	}
	ResumeScreenScrolling();
	
	if (ctType == CT_NORMAL_MODE)
		SetScreenType(RACE_SETUP_SCREENS_TYPE);
	else SetScreenType(DISPLAY_CLOSEST_TRACKS_SCREEN_TYPE);
	WM_InvalidateWindow(GetTrackSelectionWindowHandle());
}

WM_HWIN GetTrackSelectionWindowHandle(void)
{
	if (GetScreenType() == RACE_SETUP_SCREENS_TYPE)	// Note: This test is equivelant to if (ctType == CT_NORMAL_MODE) but testing ScreenType seems safer
		return hRaceSetupScreens;
	else return hDisplayClosestTracks;
}

//
// Using the current text display the former keyboard again to ask for another shot at the name
//
// Input:	none
// Globals already set:
//			newName	pointer to current text
//			editType	VEHICLE/DRIVER/TRACK
//			button		type of keyboard to display
//
void TryAgainKeyboard(void)
{
	char *pTitle;
	char *pName;

	switch (button)
	{
		default:
		case NEW_DRIVER_BUTTON:
			pTitle = "Enter New Driver Name";
			break;
		case NEW_VEHICLE_BUTTON:
			pTitle = "Enter New Vehicle Name";
			break;
		case NEW_TRACK_BUTTON:
			pTitle = "Enter New Track Name";
			break;
		case EDIT_DRIVER_NAME_BUTTON:
			pTitle = "Edit Driver Name";
			break;
		case EDIT_VEHICLE_NAME_BUTTON:
			pTitle = "Edit Vehicle Name";
			break;
		case EDIT_TRACK_NAME_BUTTON:
			pTitle = "Edit Track Name";
			break;
	}
	pName = newName;

	if (pEntryKeyboard)
		DestroyKeyboard(pEntryKeyboard);
	pEntryKeyboard = CreateKeyboard(pTitle, pName, ALPHA, false, NAMELEN, 0, GetTrackSelectionWindowHandle(), false);
	PauseScreenScrolling();
	SetScreenType(RACE_SETUP_STANDALONE);
}

void DisplayKeyboard(int Id)
{
	char *pTitle;
	char *pName;

	switch (Id)
	{
		default:
		case NEW_DRIVER_BUTTON:
			pTitle = "Enter New Driver Name";
			newName[0] = 0;				// Be sure newName string in blank
			editType = DRIVER;
			break;
		case NEW_VEHICLE_BUTTON:
			pTitle = "Enter New Vehicle Name";
			newName[0] = 0;				// Be sure newName string in blank
			editType = VEHICLE;
			break;
		case NEW_TRACK_BUTTON:
			pTitle = "Enter New Track Name";
			newName[0] = 0;				// Be sure newName string in blank
			editType = TRACK;
			break;
		case EDIT_DRIVER_NAME_BUTTON:
			pTitle = "Edit Driver Name";
			strcpy(newName, sysData.driver);
			editType = DRIVER;
			break;
		case EDIT_VEHICLE_NAME_BUTTON:
			pTitle = "Edit Vehicle Name";
			strcpy(newName, sysData.vehicle);
			editType = VEHICLE;
			break;
		case EDIT_TRACK_NAME_BUTTON:
			pTitle = "Edit Track Name";
			strcpy(newName, sysData.track);
			editType = TRACK;
			break;
	}
	pName = newName;

	if (pEntryKeyboard)
		DestroyKeyboard(pEntryKeyboard);
	pEntryKeyboard = CreateKeyboard(pTitle, pName, ALPHA, false, NAMELEN, 0, GetTrackSelectionWindowHandle(), false);
	PauseScreenScrolling();
	SetScreenType(RACE_SETUP_STANDALONE);
}

//
// Create new file of editType type
//
// Returns: false = file not created
//			true = file created
// Return:	0 if not a duplicate file name
//			1 if a dulpicate file name and OK to proceed
//			-1 if a duplicate file name and NOT OK to proceed
//			-2 duplicate TRACK file name and want to change name and try again
//
int CreateNewEntry(void)
{
	int retval;

	if ((retval = CheckForDuplicateName()) != 0)
	{
		if (retval == 1)
		{
			// Duplicate file name and the user has oked us to delete it
			char fileName[256];

			ConvertToFilename(fileName, newName, EditTypeToFileType(editType));
#ifdef _WINDOWS
			remove(fileName);
#else
			FS_Remove(fileName);
#endif
			// now fall through to create new item
		}
		else return retval;	// Duplicate file name and NOT okay to proceed - exit
	}

	if (editType == DRIVER)
	{
		SetDefaultDriverData();			// Optionally reset all data to defaults or leave as a copy of the current structure
		UpdateList();

		if (numDrivers > 1)
		{
			BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_BLACK);
		}
	}
	else if (editType == VEHICLE)
	{
		SetDefaultVehicleData(false);		// Optionally reset all data to defaults or leave as a copy of the current structure
		UpdateList();

		if (numVehicles > 1)
		{
			BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_BLACK);
		}
	}
	else if (editType == TRACK)
	{
		CreateNewTrack();
		UpdateList();
	}
	else return false;

	return true;
}

//
// See if a file of this name and this type already exists - if so, ask if they want to proceed.
// Note that this will be a case insensitive check
//
// Return:	0 if not a duplicate file name
//			1 if a dulpicate file name and OK to proceed
//			-1 if a duplicate file name and NOT OK to proceed
//			-2 duplicate TRACK file name and want to change name and try again
//
int CheckForDuplicateName(void)
{
	char fileName[256];
#ifdef _WINDOWS
	FILE *pFile;
#else
	FS_FILE *pFile;
#endif
	
	ConvertToFilename(fileName, newName, EditTypeToFileType(editType));		// Add full path and file extension

	// If this is a track type see if this name is one of the predefined tracks
	if (editType == TRACK)
	{
		int i;

		// See if this name exists in the track tape - case INSENSITIVE
		for (i = 0; i < numberOfTracks; i++)
		{
#ifdef _WINDOWS
			if (_stricmp(newName, trackTable[i].trackName) == 0)
#else
			if (kmc_stricmp(newName, trackTable[i].trackName) == 0)
#endif
			{
				if (RaceSetupPopup(TRACK_ALREADY_EXISTS))	// true = ok to try new name, false = not ok - abort
					return -2;				// duplicate name - want to try different name
				else return -1;				// duplicate name - don't want to bother
			}
		}
		return 0;					// Else name not found - good to go
	}

#ifdef _WINDOWS
	if (pFile = fopen(fileName, "r"))		// See if the file can be opened - i.e. if it exists
	{
		// File already exists - see if the user really wants to do this
		fclose(pFile);
#else
	if (pFile = FS_FOpen(fileName, "rb"))	// See if the file can be opened - i.e. if it exists
	{
		// File already exists - see if the user really wants to do this
		FS_FClose(pFile);
#endif
		if (!RaceSetupPopup(FILE_ALREADY_EXISTS))	// true = ok to replace, false = not ok
			return -1;				// Do not rename/create
		else return 1;				// OK to rename/overwrite
	}
	else return 0;					// File does not exist - good to go
}

enum DATATYPES EditTypeToFileType(enum TYPES editType)
{
	if (editType == TRACK)
		return TRACK_DATA;
	else if (editType == VEHICLE)
		return VEHICLE_DATA;
	else if (editType == DRIVER)
		return DRIVER_DATA;
	else return SYSTEM_DATA;
}


//
// Extract the "track number" from this file name
//
int GetTrackNumber(char *pName, int offset)
{
	int temp = 0;
	char ch;

	while (isdigit(ch = *(pName + offset++)))
	{
		temp *= 10;
		temp += ch - '0';
	}
	return temp;
}

//
// Find the last track number for this track type
//
int FindLastTrackOfCurrentType(void)
{
	int trackNumber = 0;
	char *pTrackType;
	int trackTypeLen;

#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

	pTrackType = GetSelectedReviewModeText();
	trackTypeLen = strlen(pTrackType);

#ifdef _WINDOWS
	dir = opendir(TRACK_PATH);
	if (dir != NULL)
	{
		while ((ent = readdir(dir)) != NULL) {
			if (ValidFileName(ent->d_name))
			{
				if (strncmp(ent->d_name, pTrackType, trackTypeLen) == 0)
				{
					int temp = GetTrackNumber(ent->d_name, trackTypeLen);
					if (temp > trackNumber)
						trackNumber = temp;
				}
			}
		}
		closedir(dir);
	}
#else
	if (FS_FindFirstFile(&fd, TRACK_PATH, acFilename, sizeof(acFilename)) == 0) {
		do {
			if (ValidFileName(acFilename))
			{
				if (strncmp(acFilename, pTrackType, trackTypeLen) == 0)
				{
					int temp = GetTrackNumber(acFilename, trackTypeLen);
					if (temp > trackNumber)
						trackNumber = temp;
				}
			}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#endif

	return trackNumber;
}

//
// Create a new track of the current track type with the next track number in the sequence
//
void AutoCreateNewTrack(void)
{
	int last = FindLastTrackOfCurrentType();
	sprintf(newName, "%s%d", GetSelectedReviewModeText(), ++last);
	CreateNewTrack();
}

void CreateNewTrack(void)
{
	TRACK_DATA_STRUCTURE newTrack;

	memset(&newTrack, 0, sizeof(newTrack));
	newTrack.courseType = SessionTypeToTrackType((enum SESSION_TYPE)sysData.selectedSessionType);
	AddTrack(newName, &newTrack);
	strcpy(selectedTrackName, newName);
	trackTableIndex = -1;		// force SetTrackTableDefaultData to create new default track
	CheckWorkingTrackName();		// Initialize data and save new file to disk
	trackTableIndex = FindFile(sysData.track);	// Reset trackTableIndex

	GetClosestTracks(sysData.radius);		// Added a new track - update the closest tracks list
	WriteUserDataEvent(TRACK_DATA);			// Call routine to synchronously write the data to the SD card
}

//
// Rename this entry to the name specified in newName
//
// Input:	newName = just the bare new file name with no path or extension
//			editType = type of file to be renamed
//			sysData.d/v/t = current file name  * - note that this might not always be right!!!
//
// Output:	0 = name change successful
//			-1 = rename failed
//			selectedTrackName updated with bare file name in case of renaming tracks
//			sysData.d/v/t = new file name along with all data loaded
//
int RenameCurrentEntry(void)
{
	int retval;
#ifdef _WINDOWS
	char fileName[256], OldName[256];
#else
	char fileName[256], OldName[256];
	char fileName2[256];
	char compareName[256];
#endif
	
	if ((retval = CheckForDuplicateName()) < 0)		// If duplicate file name and NOT OK to proceed
		return retval;							// exit

	if (editType == TRACK)
	{
#ifdef _WINDOWS
		ConvertToFilename(fileName, newName, TRACK_DATA);
#else
		strcpy(fileName, newName);
		strcat(fileName, DAT_EXT);
#endif
		ConvertToFilename(OldName, sysData.track, TRACK_DATA);
		strcpy(selectedTrackName, newName);
		strcpy(trackTable[trackTableIndex].trackName, newName);

		if (!trackTable[trackTableIndex].sdCardEntry)	// If this is not an SD card entry
		{
			// Create the new SD card file
			ReloadData(TRACK_DATA);			// Be sure trackData actually contains the data for this track (if we just selected this track but have not edited yet it will not!)
			strcpy(sysData.track, newName);			// Set new files track name for write to disk
			WriteUserDataEvent(TRACK_DATA);			// Call routine to synchronously write the data to the SD card
			UpdateList();
			trackTable[trackTableIndex].sdCardEntry = true;	// KMC TODO - is this necessary?
			return 0;
		}
	}
	else if (editType == VEHICLE)
	{
#ifdef _WINDOWS
		ConvertToFilename(fileName, newName, VEHICLE_DATA);
#else
		strcpy(fileName, newName);
		strcat(fileName, DAT_EXT);
#endif
		ConvertToFilename(OldName, sysData.vehicle, VEHICLE_DATA);
	}
	else if (editType == DRIVER)
	{
#ifdef _WINDOWS
		ConvertToFilename(fileName, newName, DRIVER_DATA);
#else
		strcpy(fileName, newName);
		strcat(fileName, DAT_EXT);
#endif
		ConvertToFilename(OldName, sysData.driver, DRIVER_DATA);
	}

	// Before renaming see if this file already exists
#ifdef _WINDOWS
	// If OldName and fileName are DIFFERENT files (ie not just different case versions of the same name)
	if (_stricmp(OldName, fileName) != 0)
		remove(fileName);		// Delete the "target" file name

	if (rename(OldName, fileName) < 0)		// syntax: old name, new name
		return -1;
#else
	if (!unsavedSysData.sdCardInserted)
		return -1;
	
	ConvertToFilename(compareName, newName, EditTypeToFileType(editType));

	if (kmc_stricmp(OldName, compareName) != 0)
		FS_Remove(fileName);	// Delete the "target" file name
	
	// Rename the file - note that in the case of the files differing only in case Segger OS cannot directly
	// rename the file!  You must go through an intermediate file name to change and then rename again to the
	// new name.
	strcpy(fileName2, newName);		// create temporary intermediate file name
	strcat(fileName2, ".tmp");
	if (FS_Rename(OldName, fileName2) < 0)	// rename to intermediate name
		return -1;
	AddPath(compareName, fileName2, EditTypeToFileType(editType));	// Add full path to intermediate file name
	if (FS_Rename(compareName, fileName) < 0)	// Rename to final file name
		return -1;
#endif

	UpdateList();
	return 0;
}

//
// Be sure there are no periods ('.') in name, change them to spaces (' ')
//
void ValidateName(char *pName)
{
	char ch;

	while (ch = *pName)
	{
		if (ch == '.')
			ch = ' ';
		*pName++ = ch;
	}
}


//
// Note: this routine had to be heavily modified to compensate for Segger's bug.
// You can only add entried to the LISTWHEEL up to a total of 6 entries using the
// normal LISTWHEEL_SetText function.  After that the list wheel craps out and
// gets lost so you must destroy the old list and recreate a new one from scratch!!!
// (I'm not joking!)
//
void UpdateList(void)
{
	unsigned int sel;
	LISTVIEW_Handle *hList;
	char **pList;
	int id;

	ValidateName(newName);					// Be sure there are no periods ('.') in name, change them to spaces (' ')

	if (editType == DRIVER)
	{
		strcpy(sysData.driver, newName);
		if (strcmp(newName, ""))			// As long as the new name is not blank
			WriteUserDataEvent(DRIVER_DATA);		// Call routine to synchronously write the data to the SD card
		CreateFileList(DRIVER_DATA);
		sel = FindSelection(DRIVER_DATA);
		hList = &hDriverList;
		id = DRIVERLIST;
		pList = pDrivers;
	}
	else if (editType == VEHICLE)
	{
		strcpy(sysData.vehicle, newName);
		if (strcmp(newName, ""))			// As long as the new name is not blank
			WriteUserDataEvent(VEHICLE_DATA);		// Call routine to synchronously write the data to the SD card
		SetGear(GEAR_RESET);					// recalculate gear tables
		CreateFileList(VEHICLE_DATA);
		sel = FindSelection(VEHICLE_DATA);
		hList = &hVehicleList;
		id = VEHICLELIST;
		pList = pVehicles;
	}
//	else if (editType == TRACK)
	else
	{
		strcpy(sysData.track, newName);
		GetClosestTracks(sysData.radius);
		CreateFileList(TRACK_DATA);
		//if ((ctType == CT_NORMAL_MODE) && (sysData.radius == WHOLE_WORLD))
		//	pList = pTracksFull;
		//else pList = pTracksShort;
		pList = pTracksShort;
		sel = FindSelection(TRACK_DATA);
		UpdateTrackList(GetTrackSelectionWindowHandle());
		return;
	}

	// Display alphabetically sorted list control
	DisplayAlphaList(hList, id, pList, sel);
}

void RaceSetupScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id, Sel;

	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
			if (pMsg->Data.p == pEntryKeyboard)
			{
				RSSKillKeyboard();
				UnhideSprites();
			}
			break;

		case KB_SAVE_AND_EXIT_MSG:
			if (pMsg->Data.p == pEntryKeyboard)
			{
				if ((strcmp(newName, "") == 0) || (newName[0] == '.'))	// Validate name.  If we want more detailed testing make this a routine and call it here
					InvalidName();
				else
				{
					int retval;

					RSSKillKeyboard();
					if (mode == NEW)
						retval = CreateNewEntry();
					else retval = RenameCurrentEntry();
					
					if (retval == 1)
					{
						// Just created a new track - if "going racing" then exit to go racing screens
						if (ctType == CT_RACE_ENTRY)
						{
							ReloadData(TRACK_DATA);		// Load data for newly created track
							GoRacingStateMachine(pMsg->hWin);
						}
					}
					else if (retval == -2)
					{
						// get new name - put up keyboard
						TryAgainKeyboard();
						break;			// DO NOT UnhideSprites as the keyboard is going right back up
					}
				}
				UnhideSprites();
				// KMC TODO - now go racing if this was the new create new track button when entering race mode!
			}
			break;

		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			RaceSetupIgnoreNextKeyRelease();
			vehicleSelectionChanged = false;		// Initially mark the vehicle as not changed
			break;

		case WM_DELETE:
			hTrackList = hDriverList = hVehicleList = 0;
			DeleteSprites();
			break;

		case WM_PAINT:
			if (ctType == CT_NORMAL_MODE)
			{
				GUI_SetColor(GUI_WHITE);
				GUI_SetFont(&GUI_Font24_ASCII);

				// Call paint method of center screen
				RaceSetupScreenList[GetLastScreen(RACE_SETUP_SCREENS_TYPE)].paint(hRaceSetupScreens);
				DrawSignalStrength();
			}
			else if (ctType == CT_RACE_ENTRY)
			{
				GUI_SetColor(GUI_WHITE);
				GUI_SetFont(&GUI_Font24_ASCII);

				TrackSelectionPaint(hDisplayClosestTracks);

				DrawSignalStrength();
			}
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == TRACKLIST)
						SelectThisTrack(LISTVIEW_GetSel(hTrackList));
					else
					{
						switch(Id)
						{
						case DRIVERLIST:
							Sel = LISTVIEW_GetSelUnsorted(pMsg->hWinSrc);
							strcpy(sysData.driver, pDrivers[Sel]);	// Set new driver name in sysData structure
							ReloadData(DRIVER_DATA);				// Load new driver data
							break;
						case VEHICLELIST:
							Sel = LISTVIEW_GetSelUnsorted(pMsg->hWinSrc);	// Get current selection
							strcpy(tempVehicleName, pVehicles[Sel]);	// Temporarily save the name of the new vahicle
							vehicleSelectionChanged = true;				// Mark the vehicle as changed to process at screen exit
							break;
						}
					}
					break;

				case WM_NOTIFICATION_CLICKED:
					clickedOnThisScreen = true;
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;
					if (!clickedOnThisScreen)
						break;
					{
						char scratch[30];

						switch (Id)
						{
						case SELECT_RADIUS_BUTTON:
							if (sysData.radius == 3219)
								sysData.radius = WHOLE_WORLD;
							else
							{
								if (!HaveSatelliteReception())
								{
									if (sysData.radius >= OLD_WORLD)
										break;
									else sysData.radius = WHOLE_WORLD;
								}
								else
								{
									switch (sysData.radius)
									{
									case DEFAULT_RADIUS:	sysData.radius = 322;				break;
									case 322:				sysData.radius = 805;				break;
									case 805:				sysData.radius = 1610;				break;
									case 1610:				sysData.radius = 3219;				break;
									default:
									case WHOLE_WORLD:		sysData.radius = DEFAULT_RADIUS;	break;
									}
								}
							}
							BUTTON_SetText(pMsg->hWinSrc, GetRadiusText(scratch));

							UpdateTrackList(GetTrackSelectionWindowHandle());

							Sel = FindSelection(TRACK_DATA);
							LISTVIEW_SetSel(hTrackList, Sel);

							SelectThisTrack(Sel);
							break;

						case VEHICLE_SETTINGS_BUTTON:
							if (vehicleSelectionChanged == true)	// If the vehicle selection has changed on this screen
								ChangeVehicle();					// Validate and make the change in the database
							HideSprites();
							Display_VehicleSettingMainScreen(hRaceSetupScreens);
							break;

						case TRACK_SETTINGS_BUTTON:
							if (numTracks != 0)
							{
#ifdef _WINDOWS
								CheckWorkingTrackName();
								Display_TrackSettingMainScreen(hRaceSetupScreens);
#else
								if (unsavedSysData.sdCardInserted)
								{
									CheckWorkingTrackName();
									Display_TrackSettingMainScreen(hRaceSetupScreens);
								}
								else NotSupportedPopup(hRaceSetupScreens);
#endif
							}
							break;

						case TRACK_TYPE_BUTTON:
							IncrementSessionType();
							SetTrackTypeButtonText();
							UpdateTrackList(hRaceSetupScreens);		// TRACK_TYPE_BUTTON only exists on the Race Setup Screen (not display closest tracks screen)
							SelectThisTrack(0);				// When cycling through track types always reselect the first track in the list (closest)
							break;

						case NEW_DRIVER_BUTTON:
						case NEW_VEHICLE_BUTTON:
						case NEW_TRACK_BUTTON:
							HideSprites();
#ifdef _WINDOWS
							mode = NEW;
							DisplayKeyboard(button = Id);
#else
							if (unsavedSysData.sdCardInserted)
							{
								mode = NEW;
								DisplayKeyboard(button = Id);
							}
							else NotSupportedPopup(hRaceSetupScreens);
#endif
							break;

						case DELETE_DRIVER_BUTTON:
							if (numDrivers > 1)
							{
								if (RaceSetupPopup(DELETE_DRIVER_BUTTON))
									RSDeleteDriver();
							}
							break;

						case DELETE_VEHICLE_BUTTON:
							if (vehicleSelectionChanged == true)	// If the vehicle selection has changed on this screen
								ChangeVehicle();					// Validate and make the change in the database
#ifndef _WINDOWS
							if (!unsavedSysData.sdCardInserted)
							{
								NotSupportedPopup(hRaceSetupScreens);
								break;
							}
#endif
							if (numVehicles > 1)
							{
								if (RaceSetupPopup(DELETE_VEHICLE_BUTTON))
									RSDeleteVehicle();
							}
							break;

						case DELETE_TRACK_BUTTON:
							if (numTracks != 0)
							{
								if (RaceSetupPopup(DELETE_TRACK_BUTTON))
								{
									if (!RSDeleteTrack())
										RaceSetupPopup(CANT_DELETE_TRACK);
								}
							}
							break;

						case EDIT_VEHICLE_NAME_BUTTON:
							if (vehicleSelectionChanged == true)	// If the vehicle selection has changed on this screen
								ChangeVehicle();					// Validate and make the change in the database
							// fall through...

						case EDIT_DRIVER_NAME_BUTTON:
							HideSprites();
#ifdef _WINDOWS
							mode = EDIT;
							DisplayKeyboard(button = Id);
#else
							if (unsavedSysData.sdCardInserted)
							{
								mode = EDIT;
								DisplayKeyboard(button = Id);
							}
							else NotSupportedPopup(hRaceSetupScreens);
#endif
							break;
							
						case EDIT_TRACK_NAME_BUTTON:
							if (numTracks != 0)
							{
								HideSprites();
#ifdef _WINDOWS
								mode = EDIT;
								DisplayKeyboard(button = Id);
#else
								if (unsavedSysData.sdCardInserted)
								{
									mode = EDIT;
									DisplayKeyboard(button = Id);
								}
								else NotSupportedPopup(hRaceSetupScreens);
#endif
							}
							break;

						case AUTO_CREATE_TRACK_BUTTON:
							AutoCreateNewTrack();
							FinishLoadingTrackData();
							break;

						case CONFIRM_TRACK_BUTTON:
							SelectThisTrack(LISTVIEW_GetSel(hTrackList));
							FinishLoadingTrackData();
							break;

						case LAPS_BUTTON:
							sysData.lappingMode = true;
							BUTTON_SetBitmapEx(hLapsCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
							BUTTON_SetBitmap(hQualifyingCheck, BUTTON_CI_UNPRESSED, NULL);
							WM_InvalidateWindow(hRaceSetupScreens);
							break;

						case QUALIFYING_BUTTON:
							sysData.lappingMode = false;
							BUTTON_SetBitmap(hLapsCheck, BUTTON_CI_UNPRESSED, NULL);
							BUTTON_SetBitmapEx(hQualifyingCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
							WM_InvalidateWindow(hRaceSetupScreens);
							break;
						}
					}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

//
// Exiting vehicle selection screen and vehicle selection has changed.  Validate and
// change the vehicle.
//
void ChangeVehicle(void)
{
	if (vehicleSelectionChanged == true)
	{
		strcpy(sysData.vehicle, tempVehicleName);	// Set new Vehicle name in sysData structure
		strcpy(newName, tempVehicleName);		// Be sure working variable string "newName" is up to date for exit from some subsequent screens
		ReloadData(VEHICLE_DATA);				// Load new Vehicle data
		SetGear(GEAR_RESET);					// Calculate gear table for new vehicle
		vehicleSelectionChanged = false;
		ValidateCameraData();					// Configure camera settings for this vehicle
	}
}

void FinishLoadingTrackData(void)
{
	ReloadData(TRACK_DATA);		// Be sure track loaded
	raceModeState = RM_SELECTED_TRACK;
	GoRacingStateMachine(hDisplayClosestTracks);
}

void ExitTrackSelectionScreen(void)
{
	CheckWorkingTrackName();
}

char *GetRadiusText(char *pText)
{
	if (sysData.radius >= OLD_WORLD)
		strcpy(pText, "All Tracks - World");
	else if (sysData.units == METRIC)
		sprintf(pText, "Within %d km", sysData.radius);
	else sprintf(pText, "Within %d miles", (int)(sysData.radius * KILOMETERS_TO_MILES));
	return pText;
}

char SelectThisTrack(int selection)
{
	if (selection < 0)
		return 0;				// Not a valid index

	// Determine index into trackTable and set 'global' index
	//if ((ctType == CT_NORMAL_MODE) && (sysData.radius == WHOLE_WORLD))
	//{
	//	trackTableIndex = selection;
	//	strcpy(selectedTrackName, pTracksFull[selection]);		// Set new Track name in temporary location
	//}
	//else
	{
		char ThisName[NAMELEN+1];
		int i;

		// Find this track in trackShortList to look up its index number in trackTable
		LISTVIEW_GetItemText(hTrackList, 0, selection, ThisName, NAMELEN);
		for (i = 0; i < numberOfShortListTracks; i++)
		{
			if (strcmp(trackTable[trackShortList[i]].trackName, ThisName) == 0)
				break;
		}
		if (i == numberOfShortListTracks)
			return 0;											// Error condition

		trackTableIndex = trackShortList[i];
		strcpy(selectedTrackName, pTracksShort[selection]);		// Set new Track name in temporary location
	}

	strcpy(sysData.track, selectedTrackName);					// Set new Track name in sysData structure
	if (selectedTrackName[0] == 0)								// If there is not really a track in the table at this position
		return 0;												// Error condition
	else return 1;												// else okay
}


void CheckWorkingTrackName(void)
{
//	if (strcmp(selectedTrackName, sysData.track))
	{
		strcpy(sysData.track, selectedTrackName);
		ReloadData(TRACK_DATA);				// Load new Track data
	}
}

void SetWorkingTrackName(void)
{
	strcpy(selectedTrackName, sysData.track);
}

// KMC added 6/6/12 to allow GoRacing to display CLOSEST track first, not currently selected track
void ClearWorkingTrackName(void)
{
	selectedTrackName[0] = 0;
	sysData.track[0] = 0;
}

#define	LIST_POSITION_ONE	topPosition + (2 * (gap + BUTTON_HEIGHT))
#define	LIST_POSITION_TWO	topPosition + (3 * (gap + BUTTON_HEIGHT))
#define	LIST_POSITION_THREE	topPosition + (4 * (gap + BUTTON_HEIGHT))
#define	LIST_POSITION_FOUR	topPosition + gap + BUTTON_HEIGHT

void DisplayDriverSelection(WM_HWIN hParent)
{
	BUTTON_Handle hButton;
	int topPosition, gap;

	editType = DRIVER;
	strcpy(newName, sysData.driver);		// Be sure working driver name string is initialized on screen entry
	UpdateList();		// This creates and displays the list of items on the screen - in this case Drivers

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	topPosition = 38;
	gap = 6;

	hRSDeleteButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_THREE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, DELETE_DRIVER_BUTTON);
	BUTTON_SetBitmap(hRSDeleteButton, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hRSDeleteButton, "Delete Driver");
	if (numDrivers > 1)
	{
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_WHITE);
	}
	else
	{
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_GRAY);
	}
	BUTTON_SetFocussable(hRSDeleteButton, 0);

	hButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_TWO, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, EDIT_DRIVER_NAME_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmYellowButton);
	BUTTON_SetText(hButton, "Edit Name");
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hButton, 0);

	hButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_ONE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, NEW_DRIVER_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hButton, "New Driver");
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hButton, 0);

	// Display static text
	hButton = BUTTON_CreateEx(BUTTON_LEFT,  83, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, 0);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmBlackBlankButton);
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hButton, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetText(hButton, "Select Driver");
}

void DisplayVehicleSelection(WM_HWIN hParent)
{
	BUTTON_Handle hButton;
	int topPosition, gap;

	editType = VEHICLE;
	strcpy(newName, sysData.vehicle);		// Be sure working vehicle name string is initialized on screen entry
	UpdateList();		// This creates and displays the list of items on the screen - in this case Vehicles

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	topPosition = 38;
	gap = 6;

	hButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_FOUR, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, VEHICLE_SETTINGS_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetText(hButton, "Vehicle Settings");
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hButton, 0);

	hRSDeleteButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_THREE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, DELETE_VEHICLE_BUTTON);
	BUTTON_SetBitmap(hRSDeleteButton, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hRSDeleteButton, "Delete Vehicle");
	if (numVehicles <= 1)
	{
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_GRAY);
	}
	else
	{
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_WHITE);
	}
	BUTTON_SetFocussable(hRSDeleteButton, 0);

	hButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_TWO, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, EDIT_VEHICLE_NAME_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmYellowButton);
	BUTTON_SetText(hButton, "Edit Name");
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hButton, 0);

	hButton = BUTTON_CreateEx(BUTTON_LEFT, LIST_POSITION_ONE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, NEW_VEHICLE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hButton, "New Vehicle");
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hButton, 0);

	// Display static text
	hButton = BUTTON_CreateEx(BUTTON_LEFT,  45, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, 0);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmTopFakeButton);
	BUTTON_SetTextColor(hButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hButton, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetText(hButton, "Select Vehicle");
}

void DisplayTrackSelection(WM_HWIN hParent)
{
// KMC - ??? what was this for ???	////if (ctType == CT_NORMAL_MODE)
	////	return;

	if (!HaveSatelliteReception() && (sysData.radius < OLD_WORLD))
	{
		DisplayWaitingForSatellites(DISPLAY_TRACKLIST_TYPE, (enum GUI_RACE_MODES)0, hParent);
		return;
	}
	else FinishDisplayTrackSelection(hParent);
}

void FinishDisplayTrackSelection(WM_HWIN hParent)
{
	char scratch[30];
	BUTTON_Handle hBtn;

	UpdateTrackList(hParent);		// Note this routine sets global variable numTracks

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	if (ctType == CT_RACE_ENTRY)
	{
		if (numTracks != 0)
		{
			hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_ONE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, DELETE_TRACK_BUTTON);
			BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
			BUTTON_SetText(hBtn, "Delete Track");
			BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
			BUTTON_SetFocussable(hBtn, 0);

			hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_FOUR, BUTTON_WIDTH, BUTTON_HEIGHT * 2, hParent, WM_CF_SHOW, 0, CONFIRM_TRACK_BUTTON);
			BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmDoubleBlueButton);
			BUTTON_SetText(hBtn, "Confirm Track");
			BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
			BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
			BUTTON_SetFocussable(hBtn, 0);
		}

		hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_TWO, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, AUTO_CREATE_TRACK_BUTTON);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmOrangeButton);
		BUTTON_SetText(hBtn, "Auto-Create Track");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hBtn, 0);
	}
	else
	{
		hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_ONE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, SELECT_RADIUS_BUTTON);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmOrangeButton);
		BUTTON_SetText(hBtn, GetRadiusText(scratch));
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hBtn, 0);

		hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_TWO, 97, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TRACK_SETTINGS_BUTTON);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmMediumBlueButton);
		BUTTON_SetText(hBtn, "Settings");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetFocussable(hBtn, 0);

		hTrackTypeBtn = BUTTON_CreateEx(BUTTON_LEFT + 103, POSITION_TWO, 97, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TRACK_TYPE_BUTTON);
	//	BUTTON_SetBitmap(hTrackTypeBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
		SetTrackTypeButtonText();
		BUTTON_SetTextColor(hTrackTypeBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetFocussable(hTrackTypeBtn, 0);

		hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_FOUR, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, EDIT_TRACK_NAME_BUTTON);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmYellowButton);
		BUTTON_SetText(hBtn, "Edit Name");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetFocussable(hBtn, 0);

		hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_FIVE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, DELETE_TRACK_BUTTON);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
		BUTTON_SetText(hBtn, "Delete Track");
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetFocussable(hBtn, 0);
	}

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, POSITION_THREE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, NEW_TRACK_BUTTON);
	BUTTON_SetText(hBtn, "New Track");
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetFocussable(hBtn, 0);
}

void CreateSprites(const GUI_BITMAP *pTopBar, const GUI_BITMAP *pBottomBar)
{
	if ((hTop != 0) || (hBottom != 0))
		DeleteSprites();

	hTop = GUI_SPRITE_Create(pTopBar, LISTXOFFSET, LISTYOFFSET);
	hBottom = GUI_SPRITE_Create(pBottomBar, LISTXOFFSET, LISTYOFFSET + LISTHEIGHT - 7);
}

void DeleteSprites(void)
{
	if (hTop)
	{
		GUI_SPRITE_Delete(hTop);
		hTop = 0;
	}
	if (hBottom)
	{
		GUI_SPRITE_Delete(hBottom);
		hBottom = 0;
	}
	hideLevel = 0;
}

void HideSprites(void)
{
	if (hTop)
	{
		if (hideLevel++ == 0)
		{
			GUI_SPRITE_Hide(hTop);
			GUI_SPRITE_Hide(hBottom);
		}
	}
}


void UnhideSprites(void)
{
	if (hTop)
	{
		if (--hideLevel <= 0)
		{
			GUI_SPRITE_Show(hTop);
			GUI_SPRITE_Show(hBottom);
			hideLevel = 0;
		}
	}
}

void UpdateTrackList(WM_HWIN hParent)
{
	char **pTracks;
	int sel, i;
	HEADER_Handle hHeader;
	const GUI_BITMAP *pTopBar, *pBottomBar;

	if (hTrackList)
		WM_DeleteWindow(hTrackList);
	DeleteSprites();

	CreateTracks();				// Completely update all track data as the track type may have changed

	if (ctType == CT_RACE_ENTRY)
		GetClosestTracks(161);
	else GetClosestTracks(sysData.radius);

	CreateTrackList();			// Note, this routine sets global variable numTracks

	//if ((ctType == CT_NORMAL_MODE) && (sysData.radius == WHOLE_WORLD))
	//{
	//	pTracks = pTracksFull;
	//	numTracks = numberOfTracks;
	//}
	//else
	{
		pTracks = pTracksShort;
		numTracks = numberOfShortListTracksUser;
	}

	sel = FindSelection(TRACK_DATA);

	// Display track list
	hTrackList = LISTVIEW_CreateEx(LISTXOFFSET, LISTYOFFSET, LISTWIDTH, LISTHEIGHT, hParent, WM_CF_SHOW, 0, TRACKLIST);
	LISTVIEW_AddColumn(hTrackList, LISTWIDTH - FINGER_WIDTH, "", GUI_TA_VCENTER);
	LISTVIEW_SetLBorder(hTrackList, 10);

	LISTVIEW_SetAutoScrollV(hTrackList, 1);
	LISTVIEW_SetFont(hTrackList, &GUI_Font24B_ASCII);
	hHeader = LISTVIEW_GetHeader(hTrackList);
	HEADER_SetHeight(hHeader, 0);
	LISTVIEW_SetRowHeight(hTrackList, FINGER_WIDTH);
	//LISTVIEW_SetCompareFunc(hTrackList, 0, &LISTVIEW_CompareDec);
	//LISTVIEW_SetCompareFunc(hTrackList, 1, &LISTVIEW_CompareText);
	//LISTVIEW_EnableSort(hTrackList);

	if (numTracks == 0)		// Watch for special case where no tracks in range - allow display of "0-th" entry of "None in range"
		LISTVIEW_AddRow(hTrackList, (const GUI_ConstString *)&pTracks[0]);
	else for (i = 0; i < numTracks; i++)
	{
		LISTVIEW_AddRow(hTrackList, (const GUI_ConstString *)&pTracks[i]);
	}
	LISTVIEW_SetSel(hTrackList, sel);
	LISTVIEW_SetBkColor(hTrackList, LISTVIEW_CI_SELFOCUS, GUI_LISTBLUE);

	if (numTracks > 6)
	{
		LISTVIEW_SetColumnWidth(hTrackList, 0, LISTWIDTH - FINGER_WIDTH);
		SCROLLBAR_SetWidth(WM_GetScrollbarV(hTrackList), FINGER_WIDTH);
		pTopBar = &bmShortTopBar;
		pBottomBar = &bmShortBottomBar;
	}
	else
	{
		LISTVIEW_SetColumnWidth(hTrackList, 0, LISTWIDTH);
		SCROLLBAR_SetWidth(WM_GetScrollbarV(hTrackList), 0);
		pTopBar = &bmTopBar;
		pBottomBar = &bmBottomBar;
	}

	// Set "ignore region" of list area to prevent interference with BACK button
	LISTVIEW_SetLFreeBorder(hTrackList, 30);

	CreateSprites(pTopBar, pBottomBar);

	WM_SetFocus(hTrackList);
}

void DisplayAlphaList(LISTVIEW_Handle *hList, unsigned int id, char **pList, unsigned int sel)
{
	char *pItem;
	HEADER_Handle hHeader;
	LISTVIEW_Handle hListLocal;
	int numItems;
	const GUI_BITMAP *pTopBar, *pBottomBar;

//	WM_DeleteWindow(*hList);
	if (!*hList)
	{
		*hList = hListLocal = LISTVIEW_CreateEx(LISTXOFFSET, LISTYOFFSET, LISTWIDTH, LISTHEIGHT, hRaceSetupScreens, WM_CF_SHOW, 0, id);

		LISTVIEW_AddColumn(hListLocal, LISTWIDTH - FINGER_WIDTH, "", GUI_TA_VCENTER);
		LISTVIEW_SetLBorder(hListLocal, 10);

		LISTVIEW_SetAutoScrollV(hListLocal, 1);
		LISTVIEW_SetFont(hListLocal, &GUI_Font24B_ASCII);
		hHeader = LISTVIEW_GetHeader(hListLocal);
		HEADER_SetHeight(hHeader, 0);
		LISTVIEW_SetRowHeight(hListLocal, FINGER_WIDTH);
		LISTVIEW_SetCompareFunc(hListLocal, 0, &LISTVIEW_CompareText);
		LISTVIEW_EnableSort(hListLocal);
	}
	else
	{
		int count;

		hListLocal = *hList;
		// Delete the contents of the old list
		count = LISTVIEW_GetNumRows(hListLocal);
		while (count--)
			LISTVIEW_DeleteRow(hListLocal, 0);
	}

	numItems = 0;
	while ((pItem = *pList++) != 0)
	{
		LISTVIEW_AddRow(*hList, (const GUI_ConstString *)&pItem);
		numItems++;
	}

	LISTVIEW_SetSort(hListLocal, 0, 1);			// Sort the list in ascending alphabetical order
	LISTVIEW_SetSelUnsorted(hListLocal, sel);	// Set selection AFTER sorting using the unsorted method - trying to set sorted was causing race conditions
	LISTVIEW_SetBkColor(hListLocal, LISTVIEW_CI_SELFOCUS, GUI_LISTBLUE);

	if (numItems > 6)
	{
		LISTVIEW_SetColumnWidth(hListLocal, 0, LISTWIDTH - FINGER_WIDTH);
		SCROLLBAR_SetWidth(WM_GetScrollbarV(hListLocal), FINGER_WIDTH);
		pTopBar = &bmShortTopBar;
		pBottomBar = &bmShortBottomBar;
	}
	else
	{
		LISTVIEW_SetColumnWidth(hListLocal, 0, LISTWIDTH);
		SCROLLBAR_SetWidth(WM_GetScrollbarV(hListLocal), 0);
		pTopBar = &bmTopBar;
		pBottomBar = &bmBottomBar;
	}

	// Set "ignore region" of list area to prevent interference with BACK button
	LISTVIEW_SetLFreeBorder(hListLocal, 30);

	CreateSprites(pTopBar, pBottomBar);

	WM_SetFocus(hListLocal);
}

int GetNumTracks(void)
{
	return numTracks;
}

void SetTrackTypeButtonText(void)
{
	BUTTON_SetText(hTrackTypeBtn, "Type");
	switch (sysData.selectedSessionType)
	{
	case DRIVE_TYPE:		BUTTON_SetText(hTrackTypeBtn, "Drive");	break;
	case AUTOCROSS_TYPE:	BUTTON_SetText(hTrackTypeBtn, "AutoX");	break;
	case QUARTERMILE_TYPE:
	case EIGHTHMILE_TYPE:
	case THOUSANDFOOT_TYPE:	BUTTON_SetText(hTrackTypeBtn, "Drag");	break;
	case HILLCLIMB_TYPE:	BUTTON_SetText(hTrackTypeBtn, "Hill");	break;
	case NOT_VALID:
	default:
	case ROADRACE_TYPE:		BUTTON_SetText(hTrackTypeBtn, "Road");	break;
	}
}

void TrackSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	if (ctType == CT_RACE_ENTRY)
	{
		GUI_SetTextMode(GUI_TEXTMODE_TRANS);

		if (numTracks == 0)
			GUI_DispStringHCenterAt("Track out of range", 360, 59);

		GUI_DrawBitmap(&bmChooseTrack, 60, 0);
	}
	else GUI_DrawBitmap(&bmTrackText, 60, 0);

	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}


void DisplayLapsSelection(WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	hBtn = BUTTON_CreateEx(155,  43, 300, 100, hParent, WM_CF_SHOW, 0, LAPS_BUTTON);
	BUTTON_SetBkColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetFont(hBtn, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_RED);
	BUTTON_SetTextAlign(hBtn, GUI_TA_HCENTER | GUI_TA_VCENTER);
	BUTTON_SetText(hBtn, "LAPPING MODE\nPredictive lap timer\ncompares against Best Lap\never recorded");
	BUTTON_SetSkinClassic(hBtn);

	hBtn = BUTTON_CreateEx(155,  149, 300, 100, hParent, WM_CF_SHOW, 0, QUALIFYING_BUTTON);
	BUTTON_SetBkColor(hBtn, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetFont(hBtn, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(hBtn, BUTTON_CI_PRESSED, GUI_RED);
	BUTTON_SetTextAlign(hBtn, GUI_TA_HCENTER | GUI_TA_VCENTER);
	BUTTON_SetText(hBtn, "QUALIFYING MODE\nPredictive lap timer\ncompares against Current\nSession lap time only");
	BUTTON_SetSkinClassic(hBtn);

	hLapsCheck = BUTTON_CreateEx(30,  43, 115, 100, hParent, WM_CF_SHOW, 0, LAPS_BUTTON);
	BUTTON_SetBkColor(hLapsCheck, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hLapsCheck);

	hQualifyingCheck = BUTTON_CreateEx(30,  149, 115, 100, hParent, WM_CF_SHOW, 0, QUALIFYING_BUTTON);
	BUTTON_SetBkColor(hQualifyingCheck, BUTTON_CI_UNPRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hQualifyingCheck);

	if (sysData.lappingMode)
		BUTTON_SetBitmapEx(hLapsCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
	else BUTTON_SetBitmapEx(hQualifyingCheck, BUTTON_CI_UNPRESSED, &bmGreenCheck, 15, 15);
}

void LapsSelectionPaint(WM_HWIN hParent)
{
	GUI_SetColor(GUI_WHITE);
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmLapsQualifyText, 60, 0);

	GUI_DrawRoundedRect(10, 40, 460, 250, 5);
	GUI_FillRoundedRect(10, 40, 460, 250, 5);

	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(150, 40, 150, 250);
	GUI_DrawLine(10, 145, 460, 145);

	GUI_DrawBitmap(GetNavigationArrow(LAPSQUALIFYING_SELECTION), 0, 229);
}


void DriverSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmDriverText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(DRIVER_SELECTION), 0, 229);
}

void VehicleSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmVehicleText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(VEHICLE_SELECTION), 0, 229);
}

static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	switch (screen)
	{
		default:
		case DRIVER_SELECTION:
		case VEHICLE_SELECTION:
		case TRACK_SELECTION:
		case LAPSQUALIFYING_SELECTION:
			return &bmleftarrow;
	}
}

void RaceSetupIgnoreNextKeyRelease(void)
{
	clickedOnThisScreen = false;
}

//
// Get the name of the currently selected session
//
char *GetSessionName(void)
{
	char count = 0;
#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

#ifdef _WINDOWS
	dir = opendir(MY_SESSIONS_PATH);
	if (dir != NULL)
	{
		// find the selected session file
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidRaceSession(sysData.selectedSessionType, ent->d_name))
				if (count++ == selectedSessionUnsorted)
				{
					parseSessionName(ent->d_name, &NameStruct);
					strcpy(fullSessionName, ent->d_name);		// Save full name for file delete
					parseName(sessionName, ent->d_name, SESSION_DATA);
					closedir(dir);
					return sessionName;
				}
		}
		closedir(dir);
	}
#else
	if (FS_FindFirstFile(&fd, MY_SESSIONS_PATH, acFilename, NORMAL_FILENAME_SIZE) == 0) {
		do {
			if (ValidRaceSession((enum SESSION_TYPE)sysData.selectedSessionType, acFilename))
				if (count++ == selectedSessionUnsorted)
				{
					parseSessionName(&fd, &NameStruct);
					strcpy(fullSessionName, acFilename);		// Save full name for file delete
					parseName(sessionName, acFilename, SESSION_DATA);
					FS_FindClose(&fd);
					return sessionName;
				}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#endif

	return "";
}

//
// Delete session file indicated by fullSessionName
//
void DeleteSession(void)
{
	char fullFileName[MAX_FILENAME_SIZE];

	// Prepend directoy name
	strcpy(fullFileName, MY_SESSIONS_PATH);
	strcat(fullFileName, "\\");
	strcat(fullFileName, fullSessionName);

#ifndef _WINDOWS
	FS_Remove(fullFileName);
#else
	remove(fullFileName);
#endif
}


//
// Create list of all laps in the session
//
// If a previous list exists - delete that one and create a new one
//
char **CreateLapsList(void)
{
	char count = 0;
#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

	// Delete any previous list of this type
	DeleteFileList(LAP_DATA);

#ifndef _WINDOWS

	if (FS_FindFirstFile(&fd, MY_SESSIONS_PATH, acFilename, NORMAL_FILENAME_SIZE) == 0) {
		do {
			if (ValidRaceSession((enum SESSION_TYPE)sysData.selectedSessionType, acFilename))
				if (count++ == selectedSessionUnsorted)
				{
					parseSessionName(&fd, &NameStruct);
					ReadAllSessionData(acFilename);
					FS_FindClose(&fd);
					return pLaps;
				}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);

#else

	dir = opendir(MY_SESSIONS_PATH);
	if (dir != NULL)
	{
		// find the selected session file
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidRaceSession((enum SESSION_TYPE)sysData.selectedSessionType, ent->d_name))
				if (count++ == selectedSessionUnsorted)
				{
					parseSessionName(ent->d_name, &NameStruct);
					ReadAllSessionData(ent->d_name);
					closedir(dir);
					return pLaps;
				}
		}
		closedir(dir);
	}

#endif

	// Else failed somewhere along the line - create a dummy list
	CreateDummyLapList();
	return pLaps;
}

void CreateDummyLapList(void)
{
	char **pNextLink;

	// Create dummy list
	pNextLink = pLaps = (char **)(malloc((MAX_NUM_LAPS_IN_FILE + 1) * sizeof(char *)));
	*pNextLink = 0;
}

//
// Create a list of all the laps of this session
//
// Input: name of session file
//
void ReadAllSessionData(char *pName)
{
	int i;
	char fileName[MAX_FILENAME_SIZE];
	int count = 0;
	char **pNextLink, *pData;
#ifdef _WINDOWS
	FILE *pFile;
	size_t results;
#else
	FS_FILE *pFile;
	unsigned long results;
#endif

	// Create new list
	pNextLink = pLaps = (char **)(malloc((MAX_NUM_LAPS_IN_FILE + 1) * sizeof(char *)));
	*pNextLink = 0;		// Be sure next link is 0 in case there is no more data

	strcpy(fileName, MY_SESSIONS_PATH);
	strcat(fileName, "\\");
	strcat(fileName, pName);

#ifdef _WINDOWS
	if ((pFile = fopen(fileName, "rb")) != 0)
	{
		unsigned char numberOfFields;

		// Check for valid fields
		fseek(pFile, TQM_NUMBER_USER_FIELDS, SEEK_SET);
		results = fread(&numberOfFields, sizeof(numberOfFields), 1, pFile);

		if (results != 1)
		{
			fclose(pFile);
			return;
		}

		if (numberOfFields == TQM_USER_FIELDS)		// If new-style file
		{
			fseek(pFile, 0, SEEK_SET);			// Return to point to the beginning of the file
			results = fread(&SessionData, sizeof(tqmheaderstructtype), 1, pFile);
			fclose(pFile);

			if (results == 1)
#else
	if (OkToUseSDCard())
		if (pFile = FS_FOpen(fileName, "rb"))
		{
			unsigned char numberOfFields;
	
			FS_FSeek(pFile, TQM_NUMBER_USER_FIELDS, FS_SEEK_SET);
			results = FS_Read(pFile, &numberOfFields, sizeof(numberOfFields));
	
			if (results != sizeof(numberOfFields))
			{
				FS_FClose(pFile);
				return;
			}
			
			if (numberOfFields == TQM_USER_FIELDS)
			{
				FS_FSeek(pFile, 0, FS_SEEK_SET);
				results = FS_Read(pFile, &SessionData, sizeof(tqmheaderstructtype));
				FS_FClose(pFile);
	
				if (results == sizeof(tqmheaderstructtype))
#endif
				{
					// Copy Lap Data
					count = SessionData.lapInfo.numberOfLaps;
					for (i = 0; i < count; i++)
					{
						*pNextLink++ = pData = (char *)malloc(20);
						sprintf(pData, "%s", formatLapTime(SessionData.lapInfo.lapTime[i], true));
					}
				}
			}
			else	// Old style file
			{
				LAP_LIST_TYPE *LapList;
	
				LapList = malloc(sizeof(LAP_LIST_TYPE));
				if (!LapList)
				{
#ifdef _WINDOWS
					fclose(pFile);
#else
					FS_FClose(pFile);
#endif
					return;
				}
				// older tqm file so session descriptions not supported
				strcpy(SessionData.UserFieldDescConfig, NotAvailable);
				strcpy(SessionData.UserFieldDescGeneral, NotAvailable);
				strcpy(SessionData.UserFieldDescInputs, NotAvailable);
				strcpy(SessionData.UserFieldDescPerformance, NotAvailable);

#ifdef _WINDOWS
				fseek(pFile, TQM_LAP_OFFSET_OLD, SEEK_SET);
				results = fread(LapList, sizeof(LAP_LIST_TYPE), 1, pFile);
				fclose(pFile);
				if (results == 1)
#else
				FS_FSeek(pFile, TQM_LAP_OFFSET_OLD, FS_SEEK_SET);
				results = FS_Read(pFile, LapList, sizeof(LAP_LIST_TYPE));
				FS_FClose(pFile);
	
				if (results == sizeof(LAP_LIST_TYPE))
#endif
				{
					count = LapList->numberOfLaps;
					if (count > NUMLAPS)		// safety check
						count = NUMLAPS;
					for (i = 0; i < count; i++)
					{
						*pNextLink++ = pData = (char *)malloc(20);
						sprintf(pData, "%s", formatLapTime(LapList->lapTime[i], true));
					}
				}
				free(LapList);
			}
		}

	numLaps = count;

	*pNextLink = 0;
}

char *GetBestLapAutocross(char *pDest)
{
	sprintf(pDest, "%s", formatLapTime(SessionData.lapInfo.bestLapTime, true));
	return pDest;
}

int CreateFileList(enum DATATYPES type)
{
	int count = 0;
	char **pList = 0;
	char *pData;
	const char *pDirectory;
	int limit;
#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

	// Create new list
	// Point to directory as well as clar out old list
	switch (type)
	{
		case DRIVER_DATA:	pDirectory = DRIVER_PATH;
			memset(pList = pDrivers, 0, sizeof(pDrivers));
			limit = MAX_NUM_DRIVERS;
			memset(pData = &DriverList[0][0], 0, sizeof(DriverList));
			break;
		case VEHICLE_DATA:	pDirectory = VEHICLE_PATH;
			memset(pList = pVehicles, 0, sizeof(pVehicles));
			limit = MAX_NUM_VEHICLES;
			memset(pData = &VehicleList[0][0], 0, sizeof(VehicleList));
			break;
		case SESSION_DATA:	pDirectory = MY_SESSIONS_PATH;
			limit = MAX_SESSION_FILES;
			break;

		default:
			return 0;
	}

#ifndef _WINDOWS

	if (FS_FindFirstFile(&fd, pDirectory, acFilename, NORMAL_FILENAME_SIZE) == 0) {
		do {
			if (ValidFileName(acFilename))		// KMC ?  Shouldn't this be fd->something
			{

				unsigned char processFile = true;
				
				if (type == SESSION_DATA) {
					// only show files collected in race mode
					if (ValidRaceSession((enum SESSION_TYPE)sysData.selectedSessionType, acFilename))
					{
						SaveBestLap(count, acFilename);
						count++;
					}
					else
						processFile = false;
				}
				// KMC now only use CreateFileList to update BestLaps list so no longer need to add to list
				else if (processFile) {
					parseName(pData, acFilename, type);		// Truncate ".dat" off the file name and put in allocated buffer
					*pList++ = pData;
					pData += NORMAL_FILENAME_SIZE;
					count++;
				}
			}
		} while (FS_FindNextFile(&fd) && (count < limit));
	}
	FS_FindClose(&fd);

#else

	dir = opendir(pDirectory);
	if (dir != NULL)
	{
		/* print all the files and directories within directory */
		while (((ent = readdir(dir)) != NULL) && (count < limit)) {
			if (ValidFileName(ent->d_name))
			{
				unsigned char processFile = true;
				
				if (type == SESSION_DATA)
				{
					// only show files collected in race mode
					if (parseSessionName(ent->d_name, &NameStruct) == sysData.selectedSessionType)
					{
						SaveBestLap(count, ent->d_name);
						count++;
					}
					else processFile = false;
				}

				// KMC now only use CreateFileList to update BestLaps list so no longer need to add to list
				else if (processFile) {
					parseName(pData, ent->d_name, type);		// Truncate ".dat" off the file name and put in allocated buffer
					*pList++ = pData;
					pData += NORMAL_FILENAME_SIZE;
					count++;
				}
			}
		}
		closedir(dir);
	}

#endif

	switch (type)
	{
		case DRIVER_DATA:
			numDrivers = count;
			break;
		case VEHICLE_DATA:	
			numVehicles = count;
			break;
	}
	
	return count;
}

//char SessionIsRoadRace(char *pName)
//{
//	int namelen = strlen(pName);
//
//	if (*(pName + namelen - 7) == 'R' || *(pName + namelen - 8) == 'R')
//		return true;
//	else return false;
//}

void SaveBestLap(int index, char *pName)
{
	LAP_LIST_TYPE LapList;
	char fileName[MAX_FILENAME_SIZE];

#ifdef _WINDOWS
	FILE *pFile;
	size_t results;
#else
	FS_FILE *pFile;
	unsigned long results;
#endif

	if (index >= MAX_SESSION_FILES)
		return;

	strcpy(fileName, MY_SESSIONS_PATH);
	strcat(fileName, "\\");
	strcat(fileName, pName);

#ifdef _WINDOWS
	if ((pFile = fopen(fileName, "rb")) != 0)
	{
		char numberOfFields;

		fseek(pFile, TQM_NUMBER_USER_FIELDS, SEEK_SET);
		results = fread(&numberOfFields, sizeof(numberOfFields), 1, pFile);
		fseek(pFile, 0, SEEK_SET);			// Return to point to the beginning of the file

		if (results != 1)
		{
			fclose(pFile);
			return;
		}

		if (numberOfFields == TQM_USER_FIELDS)		// If new-style file
			fseek(pFile, TQM_LAP_OFFSET, SEEK_SET);
		else fseek(pFile, TQM_LAP_OFFSET_OLD, SEEK_SET);
		results = fread(&LapList, sizeof(LAP_LIST_TYPE), 1, pFile);
		fclose(pFile);

		if (results == 1)
		{
			BestLaps[index] = LapList.bestLapTime;
			BestLapNumber[index] = LapList.bestLapNumber;
		}
		else
		{
			BestLaps[index] = 0;
		}
	}
	else BestLaps[index] = 0;
#else
	if (OkToUseSDCard())
	{
		if (pFile = FS_FOpen(fileName, "rb"))
		{
			char numberOfFields;
	
			FS_FSeek(pFile, TQM_NUMBER_USER_FIELDS, FS_SEEK_SET);
			results = FS_Read(pFile, &numberOfFields, sizeof(numberOfFields));
			FS_FSeek(pFile, 0, FS_SEEK_SET);
	
			if (results != sizeof(numberOfFields))
			{
				FS_FClose(pFile);
				return;
			}
	
			if (numberOfFields == TQM_USER_FIELDS)		// If new-style file
				FS_FSeek(pFile, TQM_LAP_OFFSET, FS_SEEK_SET);
			else FS_FSeek(pFile, TQM_LAP_OFFSET_OLD, FS_SEEK_SET);
			results = FS_Read(pFile, &LapList, sizeof(LAP_LIST_TYPE));
			FS_FClose(pFile);
			
			if (results == sizeof(LAP_LIST_TYPE))
			{
				BestLaps[index] = LapList.bestLapTime;
				BestLapNumber[index] = LapList.bestLapNumber;
			}
			else
			{
				BestLaps[index] = 0;
			}
		}
		else BestLaps[index] = 0;
	}
	else BestLaps[index] = 0;
#endif
}

char *GetBestLap(int displayLapNumber, int index, char *pStr)
{
	switch (displayLapNumber) {
	case 0:			// Best Lap: 1:35.2
			strcpy(pStr, "Best Lap: ");
			strcat(pStr, ((index < MAX_SESSION_FILES) && (index >= 0)) ? formatLapTime(BestLaps[index], false) : "");
			break;
	case 1:			// Time: 1:35.21
			strcpy(pStr, "Time: ");
			strcat(pStr, ((index < MAX_SESSION_FILES) && (index >= 0)) ? formatLapTime(BestLaps[index], true) : "");
			break;
	case 2:			// Best Lap: 4
		sprintf(pStr, "Best Lap: %d", ((index < MAX_SESSION_FILES) && (index >= 0)) ? BestLapNumber[index] : 0);
			break;
	} // switch
	return pStr;
}

char *GetAutoxTime(int index, char *pStr)
{
	strcpy(pStr, "Run Time: ");

	strcat(pStr, ((index < MAX_SESSION_FILES) && (index >= 0)) ? formatLapTime(BestLaps[index], false) : "");
	return pStr;
}

char *GetDragTime(int index, char *pStr)
{
	if (selectedSessionUnsorted >= 0)
	{
		switch (SessionNameData[selectedSessionUnsorted].dragTrackType) {
			case QUARTERMILE_TYPE:
				strcpy(pStr, "1/4mi ET: ");
				break;
			case EIGHTHMILE_TYPE:
				strcpy(pStr, "1/8mi ET: ");
				break;
			case THOUSANDFOOT_TYPE:
				strcpy(pStr, "1000' ET: ");
				break;
			default:
				strcpy(pStr, "Elapsed: ");
				break;
		} // switch

		strcat(pStr, ((index < MAX_SESSION_FILES) && (index >= 0)) ? formatLapTime(BestLaps[index], false) : "");
	}
	else strcpy(pStr, "Elapsed:");
	return pStr;
}

void DeleteFileList(enum DATATYPES type)
{
	char **pList, ***ppList;

	switch (type)
	{
		//case DRIVER_DATA:
		//	pList = pDrivers;
		//	ppList = &pDrivers;
		//	break;
		//case VEHICLE_DATA:
		//	pList = pVehicles;
		//	ppList = &pVehicles;
		//	break;
		//case TRACK_DATA:
		//	if ((ctType == CT_NORMAL_MODE) && (sysData.radius == WHOLE_WORLD))
		//	{
		//		if (pTracksFull)
		//		{
		//			free(pTracksFull);
		//			pTracksFull = 0;
		//		}
		//	}
		//	else
		//	{
		//		if (pTracksShort)
		//		{
		//			free(pTracksShort);
		//			pTracksShort = 0;
		//		}
		//	}
		//	return;
		//default:
		//case SESSION_DATA:	
		//	pList = pSessions;
		//	ppList = &pSessions;
		//	DeleteBestLaps();
		//	break;
		case LAP_DATA:
			pList = pLaps;
			ppList = &pLaps;
			break;
	}

	if (pList)
	{
		char *pNext, **pData;

		pData = pList;
		while (pNext = *pData++)
			free(pNext);
		free(pList);
		pList = 0;
	}

	*ppList = 0;		// zero out the original list pointer
}

void DeleteBestLaps(void)
{
	int i;

	for (i = 0; i < MAX_SESSION_FILES; i++)
	{
		BestLaps[i] = 0;
		BestLapNumber[i] = 0;
	}
}

int GetBestLapNumber(int session)
{
	return BestLapNumber[session];
}

void parseName(char *pDest, char *pSource, enum DATATYPES type)
{
	char ch;
	int i;

	if (type == SESSION_DATA)
	{
		// take the date (first 10 characters)
		for (i = 0; i < 10; i++)
			*pDest++ = *pSource++;

		// Search for the '.'
		while (*pSource++ != '.')
			;

		// Back up and get the session number
		pSource -= 4;
		*pDest++ = ' ';
		*pDest++ = *pSource++;
		*pDest++ = *pSource++;
		*pDest++ = *pSource++;
		*pDest = '\0';
	}
	else
	{
		while (((ch = *pSource++) != '.') && (ch != '\0'))
			*pDest++ = ch;
		*pDest = '\0';
	}
}

//
// Make sure this is a valid file name ending in ".dat" or ".tqm"
//
char ValidFileName(char *pName)
{
	char ch;

	// check to make sure we have a letter or number to start the name
	if (!isalnum(pName[0]))
		return false;

	// get to file extension
	while (((ch = *pName++) != '.') && (ch != '\0'))
		;

	if ((ch == '.') && (tolower(*pName) == 'd') && (tolower(*(pName + 1)) == 'a') && (tolower(*(pName + 2)) == 't'))
		return true;
	else if ((ch == '.') && (tolower(*pName) == 't') && (tolower(*(pName + 1)) == 'q') && (tolower(*(pName + 2)) == 'm'))
		return true;
	else return false;
}

char ValidRaceSession(enum SESSION_TYPE type, char *pName)
{
	char len, sessionChar;

	// check to make sure we have a letter or number to start the name
	if (!isalnum(pName[0]))
		return false;

	// get to file extension
	len = strlen(pName);

	if ((*(pName + len - 4) == '.') && (tolower(*(pName + len - 3)) == 't') && (tolower(*(pName + len - 2)) == 'q') && (tolower(*(pName + len - 1)) == 'm'))
	{
		sessionChar = toupper(*(pName + len - 7));
		// If we are looking for drag race session files there are three valid drag race characters
		if (GetSessionCharacter(type) == QUARTERMILE_TYPE)
		{
			if ((sessionChar == QUARTERMILE_TYPE) || (sessionChar == EIGHTHMILE_TYPE) || (sessionChar == THOUSANDFOOT_TYPE))
				return true;
			else return false;
		}
		// Otherwise other race session types - handle normally
		else if (sessionChar == GetSessionCharacter(type))
			return true;
		else return false;
	}
	else return false;
}


int FindSelection(enum DATATYPES sel)
{
	char *pSelection, **pList;
	int count;

	switch (sel)
	{
		case DRIVER_DATA:
			pList = pDrivers;
			pSelection = sysData.driver;
			break;
		case VEHICLE_DATA:
			pList = pVehicles;
			pSelection = sysData.vehicle;
			break;
		default:
		case TRACK_DATA:
			//if ((ctType == CT_NORMAL_MODE) && (sysData.radius == WHOLE_WORLD))
			//	pList = pTracksFull;
			//else pList = pTracksShort;
			pList = pTracksShort;
			pSelection = sysData.track;
			break;
	}

	if (*pSelection == 0)
		return 0;
	
	count = 0;
	while (*pList)
	{
		if (strcmp(*pList, pSelection))
		{
			pList++;
			count++;
		}
		else return count;
	}
	return 0;
}


static const GUI_WIDGET_CREATE_INFO BadNameDialog[] = {
	{ FRAMEWIN_CreateIndirect, "ERROR!", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Not a valid name.", 0, 0, 25, 360, 50, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Please enter a valid name.", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 }
};

//
// Display a modal dialog box to tell the user he entered a bad name
//
void InvalidName(void)
{
	SetPopupDefaults(ERROR_POPUP);

	GUI_ExecDialogBox(BadNameDialog, GUI_COUNTOF(BadNameDialog), &popupCallback, hRaceSetupScreens, 10, YSIZE_PHYS + 10);

	DisplayKeyboard(button);	// Must kill old screen and dislpay new one - keyboard cannot be repainted properly
}

//
// System just initializing or SD card was formatted - reset all file lists
//
void ResetAllFileLists(char doCreateTracks)
{
	if (doCreateTracks)
		CreateTracks();				// read any new ones from the SD card

	if (sysData.radius < OLD_WORLD)
		GetClosestTracks(sysData.radius);
	
	trackTableIndex = FindFile(sysData.track);	// Initialize global track table index

	CreateFileList(TRACK_DATA);
	CreateFileList(DRIVER_DATA);
	CreateFileList(VEHICLE_DATA);
	CreateFileList(SESSION_DATA);				// Get best lap information out of session files

	selectedSessionUnsorted = FindNewestFile((enum SESSION_TYPE)sysData.selectedSessionType);
}

const char NoneInRange[NAMELEN+1] = "None in range";

void CreateTrackList(void)
{
	int count = 0;
	int temp, loopCount;
//	char **pList = 0;
	char **pNextLink;
	char sortByDistance;

	if ((ctType == CT_NORMAL_MODE) && (sysData.radius >= OLD_WORLD))
		sortByDistance = false;
	else sortByDistance = true;


	//if ((ctType == CT_NORMAL_MODE) && (sysData.radius == WHOLE_WORLD))
	//{
	//	if (pTracksFull)
	//		free(pTracksFull);

	//	temp = numTracks = numberOfTracks;
	//	if (temp < 5)
	//		temp = 5;

	//	// Create new list
	//	pNextLink = pTracksFull = (char **)(malloc((temp + 1) * sizeof(char *)));

	//	while (count < numberOfTracks)
	//	{
	//		*pNextLink++ = trackTable[count].trackName;
	//		count++;
	//	}
	//}
	////else if (type == SHORT_LIST)
	//else		// type == SHORT_LIST
	{
		enum TRACK_TYPES currentCourseType;

		if (pTracksShort)
			free(pTracksShort);

		numTracks = 0;

		if (sortByDistance)
			temp = numberOfShortListTracks;
		else temp = numberOfTracks;
		loopCount = temp;

		if (temp < 5)
			temp = 5;
		
		// Create new list
		pNextLink = pTracksShort = (char **)(malloc((temp + 1) * sizeof(char *)));
		currentCourseType = SessionTypeToTrackType((enum SESSION_TYPE)sysData.selectedSessionType);
		if ((currentCourseType == DRAG_1_8) || (currentCourseType == DRAG_1000))	// List ALL types of Drag strips
			currentCourseType = DRAG;

		while (count < loopCount)
		{
			int courseType;

			// Get course type of this course
			if (sortByDistance)
				courseType = trackTable[trackShortList[count]].courseType;
			else courseType = trackTable[count].courseType;

			// "Normalize" course types to standard search types (DRAG, ROAD_COURSE, AUTOXRALLY, DRIVE_COURSE)
			if ((courseType == DRAG_1_8) || (courseType == DRAG_1000))	// Be 100% certain we 'normalize' all drag types to DRAG
				courseType = DRAG;
			else if (courseType == OVAL)
				courseType = ROAD_COURSE;

			if (courseType == currentCourseType)
			{
				if (sortByDistance)
					*pNextLink++ = trackTable[trackShortList[count]].trackName;
				else *pNextLink++ = trackTable[count].trackName;

				numTracks++;
			}
			count++;
		}
		numberOfShortListTracksUser = count = numTracks;	// Reset the number of short list tracks to reflect
										// the number that are BOTH within range AND of the correct track type.
	}

	if (count == 0)
	{
		// Add indication to no tracks are within range
		*pNextLink++ = (char *)NoneInRange;
	}

	*pNextLink = 0;
}

void SetTrackTableDefaultData(void)
{
//	if ((trackTableIndex = FindFile(sysData.track)) >= 0)		// trackTableIndex shuold now be a persistant global variable
	if (trackTableIndex == 0)
		trackTableIndex = FindFile(sysData.track);

	if (trackTableIndex >= 0)
	{
		trackData.startLineLat = trackTable[trackTableIndex].startLineLat;
		trackData.startLineLong = trackTable[trackTableIndex].startLineLon;
		trackData.startLineHeading = trackTable[trackTableIndex].startLineHeading;
		trackData.finishLineLat = trackTable[trackTableIndex].finishLineLat;
		trackData.finishLineLong = trackTable[trackTableIndex].finishLineLon;
		trackData.finishLineHeading = trackTable[trackTableIndex].finishLineHeading;
		trackData.courseType = trackTable[trackTableIndex].courseType;
	}
	else
	{
		trackData.startLineLat = 0;
		trackData.startLineLong = 0;
		trackData.startLineHeading = 0;
		trackData.finishLineLat = 0;
		trackData.finishLineLong = 0;
		trackData.finishLineHeading = 0;
		trackData.courseType = SessionTypeToTrackType((enum SESSION_TYPE)sysData.selectedSessionType);
	}
}

enum TRACK_TYPES SessionTypeToTrackType(enum SESSION_TYPE type)
{
	switch (type)
	{
	case DRIVE_TYPE:		return DRIVE_COURSE;
	case AUTOCROSS_TYPE:	return AUTOXRALLY;
	case QUARTERMILE_TYPE:	return DRAG;
	case EIGHTHMILE_TYPE:	return DRAG_1_8;
	case THOUSANDFOOT_TYPE:	return DRAG_1000;
	case HILLCLIMB_TYPE:	return HILL_CLIMB;
	default:
	case ROADRACE_TYPE:		return ROAD_COURSE;
	}
}

enum SESSION_TYPE TrackTypeToSessionType(enum TRACK_TYPES type)
{
	switch (type)
	{
	case DRIVE_COURSE:	return DRIVE_TYPE;
	case AUTOXRALLY:	return AUTOCROSS_TYPE;
	case DRAG:			return QUARTERMILE_TYPE;
	case DRAG_1_8:		return EIGHTHMILE_TYPE;
	case DRAG_1000:		return THOUSANDFOOT_TYPE;
	case HILL_CLIMB:	return HILLCLIMB_TYPE;
	default:
	case ROAD_COURSE:	return ROADRACE_TYPE;
	}
}


int FindFile(char *defaultTrack)
{
	int i;

	for (i=0; i < numberOfTracks-1; i++)
		if (strncmp(trackTable[i].trackName, defaultTrack, NAMELEN) == 0)
			return i;

	return -1;
}

#ifdef _WINDOWS
enum SESSION_TYPE parseSessionName(char *pName, struct NAMESTRUCT *NameStruct)
{
	char scratch[5];
	char fullFileName[MAX_FILENAME_SIZE];
	HANDLE hFile;
	FILETIME lastWriteTime;

	strcpy(fullFileName, MY_SESSIONS_PATH);
	strcat(fullFileName, "\\");
	strcat(fullFileName, pName);

	if ((hFile = CreateFile(fullFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
	{
		if (GetFileTime(hFile, NULL, NULL, &lastWriteTime))
		{
			SYSTEMTIME stUTC, stLocal;
			// Convert the last-write time to local time.
			FileTimeToSystemTime(&lastWriteTime, &stUTC);
			SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

			if (stLocal.wHour == 0)
				sprintf(NameStruct->time, "12:%02d am", stLocal.wMinute);
			else if (stLocal.wHour < 12)
				sprintf(NameStruct->time, "%d:%02d am", stLocal.wHour, stLocal.wMinute);
			else if (stLocal.wHour == 12)
				sprintf(NameStruct->time, "12:%02d pm", stLocal.wMinute);
			else sprintf(NameStruct->time, "%d:%02d pm", stLocal.wHour - 12, stLocal.wMinute);
		}
		else NameStruct->time[0] = 0;

		CloseHandle(hFile);
	}
	else NameStruct->time[0] = 0;
#else
enum SESSION_TYPE parseSessionName(FS_FIND_DATA *fd, struct NAMESTRUCT *NameStruct)
{
	char scratch[5];
	FS_FILETIME FileTime;
	char *pName;
	
	if (fd)
	{
		FS_TimeStampToFileTime(fd->LastWriteTime, &FileTime);
		if (FileTime.Hour == 0)
			sprintf(NameStruct->time, "12:%02d am", FileTime.Minute);
		else if (FileTime.Hour < 12)
			sprintf(NameStruct->time, "%d:%02d am", FileTime.Hour, FileTime.Minute);
		else if (FileTime.Hour == 12)
			sprintf(NameStruct->time, "12:%02d pm", FileTime.Minute);
		else sprintf(NameStruct->time, "%d:%02d pm", FileTime.Hour - 12, FileTime.Minute);
	}
	else NameStruct->time[0] = 0;
	
	pName = fd->sFileName;
#endif
	pName = GetNextString(NameStruct->date, pName);
	pName = GetNextString(NameStruct->track, pName);
	pName = GetNextString(NameStruct->vehicle, pName);
	pName = GetNextString(NameStruct->driver, pName);
	pName = GetNextChar(&NameStruct->sessionType, pName);
	GetNextNum(scratch, pName);
	NameStruct->sessionNumber = atoi(scratch);
	if ((NameStruct->sessionType == EIGHTHMILE_TYPE) || (NameStruct->sessionType == THOUSANDFOOT_TYPE) || (NameStruct->sessionType == QUARTERMILE_TYPE))
	{
		NameStruct->dragTrackType = NameStruct->sessionType;
		NameStruct->sessionType = QUARTERMILE_TYPE;
	}
	return NameStruct->sessionType;
}

char *GetNextString(char *pDest, char *pSource)
{
	char ch;

	while ((ch = *pSource) != 0)
	{
		pSource++;
		if (ch == '_')
			break;
		*pDest++ = ch;
	}
	*pDest = 0;		// Be sure to null-terminate the strings
	return pSource;
}

char *GetNextChar(enum SESSION_TYPE *pDest, char *pSource)
{
	char ch;

	*pDest = NOT_VALID;

	if ((ch = *pSource) != 0)
	{
		pSource++;
		if (ValidSessionType(ch))
			*pDest = (enum SESSION_TYPE)ch;
	}

	return pSource;
}

char *GetNextNum(char *pDest, char *pSource)
{
	char ch;

	while ((ch = *pSource) != 0)
	{
		pSource++;
		if (!isdigit(ch))
			break;
		*pDest++ = ch;
	}
	*pDest = 0;		// Be sure to null-terminate the strings
	return pSource;
}


char ValidSessionType(char ch)
{
	switch (ch)
	{
	case 'R':
	case 'D':
	case 'A':
	case 'Q':
	case 'E':
	case 'T':
	case 'H':
		return true;
	default:
		return false;
	}
}

char GetSessionCharacter(enum SESSION_TYPE type)
{
	switch (type)
	{
	case ROADRACE_TYPE:		return 'R';
	case DRIVE_TYPE:		return 'D';
	case AUTOCROSS_TYPE:	return 'A';
	case QUARTERMILE_TYPE:	return 'Q';
	case EIGHTHMILE_TYPE:	return 'E';
	case THOUSANDFOOT_TYPE:	return 'T';
	case HILLCLIMB_TYPE:	return 'H';
	default:
	case NOT_VALID:			return 0;
	}
}

void GetSessionData(void)
{
	char count = 0;
#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;

	dir = opendir(MY_SESSIONS_PATH);
	if (dir != NULL)
	{
		// find the selected session file
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidRaceSession((enum SESSION_TYPE)sysData.selectedSessionType, ent->d_name))
				if (count++ == selectedSessionUnsorted)
				{
					parseSessionName(ent->d_name, &NameStruct);		// Get full session info
					ReadAllSessionData(ent->d_name);
					closedir(dir);
					return;
				}
		}
		closedir(dir);
	}
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];

	if (FS_FindFirstFile(&fd, MY_SESSIONS_PATH, acFilename, NORMAL_FILENAME_SIZE) == 0) {
		do {
			if (ValidRaceSession((enum SESSION_TYPE)sysData.selectedSessionType, acFilename))
				if (count++ == selectedSessionUnsorted)
				{
					parseSessionName(&fd, &NameStruct);
					ReadAllSessionData(acFilename);
					FS_FindClose(&fd);
					return;
				}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#endif
}

//
// Populate SeesionNameData structure with the data for all sessions.
// Note that if parent == NULL do not bother to tell the user that there are too many
// sessions
//
void GetSessionNameData(WM_HWIN hParent, char primary, enum SESSION_TYPE type)
{
#ifdef _WINDOWS
	char count = 0;
	DIR *dir;
	struct dirent *ent;
#else
	char count = 0;
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

	ClearSessionNameData();

#ifdef _WINDOWS
	dir = opendir(MY_SESSIONS_PATH);
	if (dir != NULL)
	{
		// find the selected session file
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidRaceSession(type, ent->d_name))
			{
				parseSessionName(ent->d_name, &SessionNameData[count++]);		// Get full session info

				if (count >= MAX_SESSION_FILES)
				{
					closedir(dir);
					if (hParent)
					{
						// Display warning popup
						TooManySessionsPopup(hParent, primary);
					}
					return;
				}
			}
		}
		closedir(dir);
	}
#else
	if (FS_FindFirstFile(&fd, MY_SESSIONS_PATH, acFilename, NORMAL_FILENAME_SIZE) == 0) {
		do {
			if (ValidRaceSession(type, acFilename))
			{
				parseSessionName(&fd, &SessionNameData[count++]);
				
				if (count >= MAX_SESSION_FILES)
				{
					FS_FindClose(&fd);
					if (hParent)
					{
						// Display warning popup
						TooManySessionsPopup(hParent, primary);
					}
					return;
				}
			}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#endif
}

void ClearSessionNameData(void)
{
	memset(SessionNameData, 0, sizeof(SessionNameData));
}

int FindNewestFile(enum SESSION_TYPE type)
{
	int index = -1;
	int tempIndex = 0;
	
#ifdef _WINDOWS
	char fullFileName[MAX_FILENAME_SIZE];
	HANDLE hFile;
	DIR *dir;
	struct dirent *ent;
	FILETIME lastWriteTime;
	FILETIME date;

	date.dwHighDateTime = 0;
	date.dwLowDateTime = 0;
	dir = opendir(MY_SESSIONS_PATH);
	if (dir != NULL)
	{
		// find the selected session file
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidRaceSession(type, ent->d_name))
			{
				strcpy(fullFileName, MY_SESSIONS_PATH);
				strcat(fullFileName, "\\");
				strcat(fullFileName, ent->d_name);

				if ((hFile = CreateFile(fullFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
				{
					if (GetFileTime(hFile, NULL, NULL, &lastWriteTime))
					{
						if (CompareFileTime(&date, &lastWriteTime) == -1)
						{
							date = lastWriteTime;
							index = tempIndex;
						}
					}
					CloseHandle(hFile);
				}

				tempIndex++;
			}
		}
		closedir(dir);
	}
#else
	U32 date = 0;
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];

	if (FS_FindFirstFile(&fd, MY_SESSIONS_PATH, acFilename, NORMAL_FILENAME_SIZE) == 0) {
		do {
			if (ValidRaceSession(type, acFilename))
			{
				if (date < fd.LastWriteTime)
				{
					date = fd.LastWriteTime;
					index = tempIndex;
				}
				tempIndex++;
			}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#endif
	
	return index;
}

static const GUI_WIDGET_CREATE_INFO DeleteDriverResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to Delete Driver!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO DeleteVehicleResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to Delete Vehicle!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO DeleteTrackResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "About to Delete Track!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO FileAlreadyExistsResources[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "A file of this name already exists!", 0, 0, 25, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Replace?", 0, 0, 65, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_YES, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "NO", GUI_ID_NO, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO TrackAlreadyExistsResources[] = {
	{ FRAMEWIN_CreateIndirect, "ERROR", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "This is the name of a predefined", 0, 0, 15, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "track or this name is already in use!", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Please change the name.", 0, 0, 75, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO CantDeleteResources[] = {
	{ FRAMEWIN_CreateIndirect, "ERROR", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Cannot delete this track!", 0, 0, 40, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 },
};

//
// Display a modal dialog box to see if the user wants to stop recording to exit this screen
//
// Returns	YES - proceed to exit record mode
//				NO - abort and stay
//
int RaceSetupPopup(int screen)
{
	int retval;
	WM_HWIN owner;

	retval = false;
	modalWindowUp = true;

	HideSprites();

	owner = WM_GetDesktopWindow();		// Get the desktop window - that is the owner

	switch (screen)
	{
		case DELETE_DRIVER_BUTTON:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(DeleteDriverResources, GUI_COUNTOF(DeleteDriverResources), &popupCallback, owner, 10, 10) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case DELETE_VEHICLE_BUTTON:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(DeleteVehicleResources, GUI_COUNTOF(DeleteVehicleResources), &popupCallback, owner, 10, 10) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case DELETE_TRACK_BUTTON:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(DeleteTrackResources, GUI_COUNTOF(DeleteTrackResources), &popupCallback, owner, 10, 10) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case CANT_DELETE_TRACK:
			SetPopupDefaults(ERROR_POPUP);
			if (GUI_ExecDialogBox(CantDeleteResources, GUI_COUNTOF(CantDeleteResources), &popupCallback, owner, 10, 10) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;

		case FILE_ALREADY_EXISTS:
			SetPopupDefaults(WARNING_POPUP);
			if (GUI_ExecDialogBox(FileAlreadyExistsResources, GUI_COUNTOF(FileAlreadyExistsResources), &popupCallback, owner, 10, 10) == GUI_ID_YES)
				retval = true;
			else retval = false;
			break;

		case TRACK_ALREADY_EXISTS:
			SetPopupDefaults(ERROR_POPUP);
			if (GUI_ExecDialogBox(TrackAlreadyExistsResources, GUI_COUNTOF(TrackAlreadyExistsResources), &popupCallback, owner, 10, 10) == GUI_ID_OK)
				retval = true;
			else retval = false;
			break;
	}
	
	modalWindowUp = false;

	UnhideSprites();

	return retval;
}

static const GUI_WIDGET_CREATE_INFO TooManyTracksResourcesPrim[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "There are too many session files!", 0, 0, 15, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Some sessions might not be listed", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Delete some sessions to make room", 0, 0, 75, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 },
};

static const GUI_WIDGET_CREATE_INFO TooManyTracksResourcesSub[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 0, 0, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "There are too many session files!", 0, 0, 15, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Some sessions might not be listed", 0, 0, 45, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Delete some sessions to make room", 0, 0, 75, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 130, 115, 100, 60 },
};

void TooManySessionsPopup(WM_HWIN hParent, char primary)
{
	modalWindowUp = true;

	SetPopupDefaults(WARNING_POPUP);
	if (primary)
		GUI_ExecDialogBox(TooManyTracksResourcesPrim, GUI_COUNTOF(TooManyTracksResourcesPrim), &popupCallback, hParent, 10, 10);
	else GUI_ExecDialogBox(TooManyTracksResourcesSub, GUI_COUNTOF(TooManyTracksResourcesSub), &popupCallback, hParent, 0, 0);

	modalWindowUp = false;
}

void RSDeleteDriver(void)
{
	int Sel;

	editType = DRIVER;

	RSDeleteFile(DRIVER_DATA, sysData.driver);

	Sel = LISTVIEW_GetSelUnsorted(hDriverList);		// Get current selection
	if (Sel == 0)
		Sel = 1;
	else Sel--;
	strcpy(newName, pDrivers[Sel]);					// Set new driver name in newName structure for reloading
	strcpy(sysData.driver, pDrivers[Sel]);
	ReloadData(DRIVER_DATA);
	strcpy(newName, sysData.driver);				// If we just deleted the last driver then we reset to the default list
													// update the newName field before calling UpdateList
	UpdateList();
	if (numDrivers <= 1)
	{
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_GRAY);
	}
}

void RSDeleteVehicle(void)
{
	int Sel;

	editType = VEHICLE;

	RSDeleteFile(VEHICLE_DATA, sysData.vehicle);

	Sel = LISTVIEW_GetSelUnsorted(hVehicleList);	// Get current selection
	if (Sel == 0)
		Sel = 1;
	else Sel--;
	strcpy(newName, pVehicles[Sel]);				// Set new Vehicle name in newName structure for reloading
	strcpy(sysData.vehicle, pVehicles[Sel]);
	ReloadData(VEHICLE_DATA);
	strcpy(newName, sysData.vehicle);				// If we just deleted the last vehicle then we reset to the default list
													// update the newName field before calling UpdateList
	UpdateList();
	if (numVehicles <= 1)
	{
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_UNPRESSED, GUI_GRAY);
		BUTTON_SetTextColor(hRSDeleteButton, BUTTON_BI_PRESSED, GUI_GRAY);
	}
}

char RSDeleteTrack(void)
{
	// Start KMC 2/11/12 - fix to allow delete track when showing whole world list (not the short list)
	int testNumTracks;

	if (sysData.radius >= OLD_WORLD)
		testNumTracks = numTracks;
	else testNumTracks = numberOfShortListTracksUser;

	if (!trackTable[trackTableIndex].sdCardEntry)		// If this is not an SD card entry
		return false;									// cannot delete this track
	else if (testNumTracks != 0)
//	else if (numberOfShortListTracksUser != 0)
	// End KMC 2/11/12 fix
	{
		editType = TRACK;

		DeleteTrack(sysData.track);
		RSDeleteFile(TRACK_DATA, sysData.track);
		CreateTracks();									// Re-evaluate all tracks on both SD card and flash

		GetClosestTracks(sysData.radius);				// Update the closest tracks list
		CreateTrackList();

		//if (pTracksShort)
		//	strcpy(selectedTrackName, pTracksShort[0]);	// Set selection to closest track
		//else strcpy(selectedTrackName, pTracksFull[trackTableIndex]);		// or the next track in the alpha list
		strcpy(selectedTrackName, pTracksShort[0]);	// Set selection to closest track

// KMC 9/10/12 - fix to prevent creating a new track called "None in range" when deleting the last available track and
// "None in range" gets shown in the track list.  When this happens numTracks is actually set to one for the spinner
// even though there are actually no tracks.  There might be other or perhaps better ways to solve this - this was the
// first attempt.
		if ((testNumTracks == 1) && (strcmp(selectedTrackName, NoneInRange) == 0))
		{
			strcpy(sysData.track, "");
			strcpy(selectedTrackName, "");
			strcpy(newName, "");
		}
		else
		{
			strcpy(sysData.track, selectedTrackName);
			trackTableIndex = FindFile(sysData.track);		// Reset trackTableIndex
			if (trackTable[trackTableIndex].sdCardEntry)	// If this is not an SD card entry
				ReloadData(TRACK_DATA);						// Load new Track data


			strcpy(newName, sysData.track);					// If we just deleted the last vehicle then we reset to the default list
															// update the newName field before calling UpdateList
		}
		UpdateList();
		return true;
	}
	else return false;
}


void RSDeleteFile(enum DATATYPES fileType, char *pName)
{
	char fullFileName[MAX_FILENAME_SIZE];

	ConvertToFilename(fullFileName, pName, fileType);
#ifdef _WINDOWS
	remove(fullFileName);
#else
	FS_Remove(fullFileName);
#endif
}


char *GetSessionText(enum SESSION_TEXT_TYPE type)
{
	switch (type)
	{
	case ST_GENERAL:		return SessionData.UserFieldDescGeneral;
	case ST_PERFORMANCE:	return SessionData.UserFieldDescPerformance;
	case ST_INPUTS:			return SessionData.UserFieldDescInputs;
	default:
	case ST_CONFIG:			return SessionData.UserFieldDescConfig;
	}
}







char *GetSelectedReviewModeText(void)
{
	switch (sysData.selectedSessionType)
	{
	default:
	case NOT_VALID:		
	case ROADRACE_TYPE:		return "Race";
	case DRIVE_TYPE:		return "Drive";
	case AUTOCROSS_TYPE:	return "AutoX";
	case QUARTERMILE_TYPE:
	case EIGHTHMILE_TYPE:
	case THOUSANDFOOT_TYPE:	return "Drag";
	case HILLCLIMB_TYPE:	return "Hill";
	}
}

enum DISPLAY_TYPES {
	DT_END = 0,
	DT_BEST_LAP,
	DT_DRIVER,
	DT_VEHICLE,
	DT_DRIVE_DURATION,
	DT_DRIVE_DISTANCE,
	DT_ET,
	DT_TOP_SPEED,
	DT_TIME
};

enum DISPLAY_TYPES RoadRaceDisplayTypes[] = { DT_BEST_LAP, DT_DRIVER, DT_VEHICLE, DT_END };
enum DISPLAY_TYPES DriveDisplayTypes[] = { DT_DRIVE_DURATION, DT_DRIVE_DISTANCE, DT_DRIVER, DT_VEHICLE, DT_END };
enum DISPLAY_TYPES DragDisplayTypes[] = { DT_ET, DT_TOP_SPEED, DT_DRIVER, DT_VEHICLE, DT_END };
enum DISPLAY_TYPES AutoXDisplayTypes[] = { DT_TIME, DT_DRIVER, DT_VEHICLE, DT_END };
enum DISPLAY_TYPES HillDisplayTypes[] = { DT_TIME, DT_DRIVER, DT_VEHICLE, DT_END };

char reviewRaceDisplay = 0;
char reviewDriveDisplay = 0;
char reviewAutoXDisplay = 0;
char reviewDragDisplay = 0;
char reviewHillDisplay = 0;

extern BUTTON_Handle hBestTime;

void InitReviewDisplayTypes(void)
{
	reviewRaceDisplay = 0;
	reviewDriveDisplay = 0;
	reviewAutoXDisplay = 0;
	reviewDragDisplay = 0;
	reviewHillDisplay = 0;
}

void SetBestTimeButtonText(void)
{
	enum DISPLAY_TYPES nextDisplay;
	char scratch[50], scratch2[50];;

	switch (sysData.selectedSessionType)
	{
	default:
	case NOT_VALID:		
	case ROADRACE_TYPE:
		nextDisplay = RoadRaceDisplayTypes[reviewRaceDisplay];
		break;
	case DRIVE_TYPE:
		nextDisplay = DriveDisplayTypes[reviewDriveDisplay];
		break;
	case AUTOCROSS_TYPE:
		nextDisplay = AutoXDisplayTypes[reviewAutoXDisplay];
		break;
	case QUARTERMILE_TYPE:
	case EIGHTHMILE_TYPE:
	case THOUSANDFOOT_TYPE:
		nextDisplay = DragDisplayTypes[reviewDragDisplay];
		break;
	case HILLCLIMB_TYPE:
		nextDisplay = HillDisplayTypes[reviewHillDisplay];
		break;
	}

	switch (nextDisplay)
	{
	default:
	case DT_DRIVER:		
		sprintf(scratch, "Drv: %s", (selectedSessionUnsorted >= 0) ? SessionNameData[selectedSessionUnsorted].driver : "");
		break;

	case DT_BEST_LAP:
		strcpy(scratch, GetBestLap(false, selectedSessionUnsorted, scratch));
		break;

	case DT_VEHICLE:
		sprintf(scratch, "Veh: %s", (selectedSessionUnsorted >= 0) ? SessionNameData[selectedSessionUnsorted].vehicle : "");
		break;

	case DT_DRIVE_DURATION:
		strcpy(scratch, "Time: ");
		if (selectedSessionUnsorted >= 0)
		{
			GetStringNumber(1, scratch2, SessionData.UserFieldDescGeneral);
			if (scratch2[0] == 'D')					// If this is a real Duration
				strcat(scratch, &scratch2[10]);		// Replace the word "Duration: " with "Time: "
			else strcat(scratch, scratch2);			// else copy over whatever it says
		}
		break;

	case DT_TIME:
		strcpy(scratch, GetAutoxTime(selectedSessionUnsorted, scratch));
		break;

	case DT_ET:
		strcpy(scratch, GetDragTime(selectedSessionUnsorted, scratch));
		break;

	case DT_TOP_SPEED:
		strcpy(scratch, "Max: ");
		if (selectedSessionUnsorted >= 0)
		{
			GetStringNumber(1, scratch2, SessionData.UserFieldDescPerformance);
			if (scratch2[0] == 'M')					// If this is a real Max speed
				strcat(scratch, &scratch2[11]);		// Replace the word "Max Speed: " with "Max: "
			else strcat(scratch, scratch2);			// else copy over whatever it says
		}
		break;

	case DT_DRIVE_DISTANCE:
		strcpy(scratch, "Dst: ");			
		if (selectedSessionUnsorted >= 0)
		{
			GetStringNumber(2, scratch2, SessionData.UserFieldDescGeneral);
			if (scratch2[0] == 'D')					// If this is a real distance
				strcat(scratch, &scratch2[10]);		// Replace word "Distance: " with "Dst: "
			else strcat(scratch, scratch2);			// else copy over whatever it says
		}
		break;
	}
	BUTTON_SetTextAlign(hBestTime, GUI_TA_LEFT | GUI_TA_VCENTER);
	BUTTON_SetText(hBestTime, scratch);
}

char *GetSessionDataString(char index, enum SESSION_TEXT_TYPE type, char *pDest)
{
	switch (type)
	{
	case ST_GENERAL:		GetStringNumber(index, pDest, SessionData.UserFieldDescGeneral);		break;
	case ST_PERFORMANCE:	GetStringNumber(index, pDest, SessionData.UserFieldDescPerformance);	break;
	case ST_INPUTS:			GetStringNumber(index, pDest, SessionData.UserFieldDescInputs);			break;
	default:
	case ST_CONFIG:			GetStringNumber(index, pDest, SessionData.UserFieldDescConfig);			break;
	}
	return pDest;
}

void GetStringNumber(char index, char *pDest, char *pSource)
{
	char ch;

	*pDest = 0;

	if (strncmp("Older File Format", pSource, 17) == 0)
	{
		strcpy(pDest, "N/A");
		return;
	}
	while (--index)
	{
		while ((ch = *pSource++) != '\n')
			if (ch == 0)
				return;
	}

	while ((ch = *pSource++) != '\n')
	{
		if (ch == 0)
			return;
		else *pDest++ = ch;
	}

	*pDest = 0;
}

void IncrementDisplayType(void)
{
	switch (sysData.selectedSessionType)
	{
	default:
	case NOT_VALID:		
	case ROADRACE_TYPE:
		if (RoadRaceDisplayTypes[++reviewRaceDisplay] == DT_END)
			reviewRaceDisplay = 0;
		break;
	case DRIVE_TYPE:
		if (DriveDisplayTypes[++reviewDriveDisplay] == DT_END)
			reviewDriveDisplay = 0;
		break;
	case AUTOCROSS_TYPE:
		if (AutoXDisplayTypes[++reviewAutoXDisplay] == DT_END)
			reviewAutoXDisplay = 0;
		break;
	case QUARTERMILE_TYPE:
	case EIGHTHMILE_TYPE:
	case THOUSANDFOOT_TYPE:
		if (DragDisplayTypes[++reviewDragDisplay] == DT_END)
			reviewDragDisplay = 0;
		break;
	case HILLCLIMB_TYPE:
		if (HillDisplayTypes[++reviewHillDisplay] == DT_END)
			reviewHillDisplay = 0;
		break;
	}
}

/*************************** End of file ****************************/

