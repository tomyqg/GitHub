// Old Session File Archive

// alternate / complementary ideas
//		should have some hysteresis so that if user has 110 files under 6 mo old, it does not run each time
//		could archive all files older than 6 months. if number still over 100, try 5 mo, 4 mo, etc. until files < 80
//		could delete small race files (<10K) (must be careful with Drag and AutoX files)
//		could have multiple archive folders for Race, Drive, AutoX etc. I don't think we want to do this.
//
// Files in MySessions will be moved to Archive folder for following reasons
// 1. file older than a certain age
// 2. file is not a .tqm file
// 3. file does not begin with a 4 digit number (or date in the form of YYYY-MM-DD)


#include <stdlib.h>
#include <stdio.h>
#include "GUI.h"
#include "DUII.h"
#include "commondefs.h"
#include "GeneralConfig.h"
#include "FlashReadWrite.h"
#include "Archive.h"
#include "Utilities.h"

#if	_WINDOWS
#include "..\Application\dirent.h"
#else
#include "FS.h"
#endif

//
// Local prototypes
//
int ArchiveSessionsPopup(void);
int compare(const void *a, const void *b);
int UserPressedCancel(void);
void ArchiveSessionsCallback(WM_MESSAGE * pMsg);

#if	_WINDOWS
int MoveThisFile(WIN32_FIND_DATA *pFileData);
int FileIsNotTQMFile(WIN32_FIND_DATA *pFileData);
#else
int MoveThisFile(FS_FIND_DATA *pFileData);
int FileIsNotTQMFile(FS_FIND_DATA *pFileData);
#endif

//
// External prototypes
//
extern void RepaintSplash(void);		// from DUII_Main.c
extern int popupTitleTextColor;			// from DUII_Main.c
extern void CreateProgressBar( PROGBAR_Handle *hProgBar, int maxval);

//
// Local variables
//
GUI_HWIN hArchivePopup;
TEXT_Handle hText1, hText2, hText3;

//
// Local defines
//
#define	ABSOLUTE_MAX_FILES			500
#define	MAX_FILES_BEFORE_ARCHIVING	120		// archive when number of files > than this
#define CUTOFF_LIMIT				50		// number of files to leave after archiving

#ifdef _WINDOWS

const char archiveDir[] = "..\\Archive";

void ArchiveSessions(void)
{
	int count, index, total;
	PROGBAR_Handle hProgBar;
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	FILETIME cutoffDate, DateList[ABSOLUTE_MAX_FILES];

	// Fill list with dates of all files
	count = 0;

	if (SetCurrentDirectory(MY_SESSIONS_PATH))
	{
		if ((fh = FindFirstFile("*.*", &FindFileData)) != INVALID_HANDLE_VALUE)
		{
			DateList[count] = FindFileData.ftLastWriteTime;
			count = 1;

			while (FindNextFile(fh, &FindFileData) && (count < ABSOLUTE_MAX_FILES))
			{
				DateList[count] = FindFileData.ftLastWriteTime;
				count++;
			}
		}
		FindClose(fh);
	}

	// If not enough files to archive - don't do anything
	if (count < MAX_FILES_BEFORE_ARCHIVING)
		goto Exit_1;

	// Ask if the user would like to archive now
	if (!ArchiveSessionsPopup())
		goto Exit_2;

	// Sort list of dates
	qsort((void *)DateList, count, sizeof(FILETIME), compare);

	// Determine cutoff threshold
	if ((index = count / 2) > CUTOFF_LIMIT)
		index = CUTOFF_LIMIT;
	cutoffDate = DateList[count - index];
	total = count - index;		// count will now be the loop counter for the progress bar
	count = 0;

	// Create Archive directory if it does not exist
	if (CreateDirectory(archiveDir, NULL) == 0)
		if (GetLastError() != ERROR_ALREADY_EXISTS)	// If directory already exists, continue, otherwise fatal error
			goto Exit_2;

	// Create the progress bar
	CreateProgressBar(&hProgBar, total);							
	GUI_Exec();
	
	// Move files older than halfway point to archive folder
	if ((fh = FindFirstFile("*.*", &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		do {
			if (UserPressedCancel())		// Watch for user cancel
				goto Exit_3;

			if (strlen(FindFileData.cFileName) > 2)		// If a valid file name
			{
				if (FileIsNotTQMFile(&FindFileData))
				{
					if (!MoveThisFile(&FindFileData))
						goto Exit_3;
					else count++;
				}

				else if (compare(&FindFileData.ftLastWriteTime, &cutoffDate) < 0)	// Move files older (smaller) than cutoffDate
				{
					char scratch[50];
					
					// update the user
					sprintf(scratch, "Moving file %d of %d", count, total);
					PROGBAR_SetText(hProgBar, scratch);
					PROGBAR_SetValue(hProgBar, count);
					GUI_Exec();
					
					if (!MoveThisFile(&FindFileData))
						goto Exit_3;
					else count++;
				}
			}
			else count++;		// File count still counted this

		} while (FindNextFile(fh, &FindFileData));
	}

Exit_3:
	PROGBAR_Delete(hProgBar);
	FindClose(fh);

Exit_2:
	RepaintSplash();

Exit_1:
	SetCurrentDirectory("..");
}

//
// Return true if file IS NOT a tqm file
// otherwise return false
//
int FileIsNotTQMFile(WIN32_FIND_DATA *pFileData)
{
	int len;

	len = strlen(pFileData->cFileName);
	return (_stricmp(".tqm", pFileData->cFileName + len - 4));
}

//
// Move this file into the archive directory
// If the file already exisits or there is a tranfer error try renaming it and trying again.
// Return true if successful
// Return false if failure
//
int MoveThisFile(WIN32_FIND_DATA *pFileData)
{
	char destFilename[MAX_FILENAME_SIZE];

	// Create destination file name
	strcpy(destFilename, archiveDir);
	strcat(destFilename, "\\");
	strcat(destFilename, pFileData->cFileName);

	// Move the file
	if (MoveFile(pFileData->cFileName, destFilename) == 0)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// A file of this name has already been moved
			// give it a new name and copy anyway just in case
			strcat(destFilename, "_2");
			if (MoveFile(pFileData->cFileName, destFilename) == 0)
				return false;
		}
		else return false;
	}
	return true;
}

int compare(const void *a, const void *b)
{
	ULONGLONG aLarge, bLarge;

	aLarge = ((ULONGLONG)((*(FILETIME *)a).dwHighDateTime) << 32) | (*(FILETIME *)a).dwLowDateTime;
	bLarge = ((ULONGLONG)((*(FILETIME *)b).dwHighDateTime) << 32) | (*(FILETIME *)b).dwLowDateTime;

	if ( aLarge <  bLarge )
		return -1;
	if ( aLarge == bLarge )
		return 0;
	if ( aLarge > bLarge )
		return 1;
	return 0;			// just in case
}

#else		// Target hardware version

const char archiveDir[] = "\\Archive";

void ArchiveSessions(void)
{
	U32 cutoffDate;
	int count, index, total;
	FS_FIND_DATA FindFileData;
	PROGBAR_Handle hProgBar;
	FS_DIR *pDir;
	U32 DateList[ABSOLUTE_MAX_FILES];
	char sourceFilename[MAX_FILENAME_SIZE];

	// Do fast pre-check for too many session files
	pDir = FS_OpenDir(MY_SESSIONS_PATH);
	count = FS_GetNumFiles(pDir);
	if (count < MAX_FILES_BEFORE_ARCHIVING)
	{
		FS_CloseDir(pDir);
		return;
	}
	// Otherwise do full check and begin archive process

	// Fill list with dates of all files
	count = 0;
	if (FS_FindFirstFile(&FindFileData, MY_SESSIONS_PATH, sourceFilename, MAX_FILENAME_SIZE) == 0)
	{
		DateList[count] = FindFileData.LastWriteTime;
		count = 1;

		while (FS_FindNextFile(&FindFileData)  && (count < ABSOLUTE_MAX_FILES))
		{
			DateList[count] = FindFileData.LastWriteTime;
			count++;
		}
	}
	FS_FindClose(&FindFileData);

	// If not enough files to archive - don't do anything
	if (count < MAX_FILES_BEFORE_ARCHIVING)
	{
		FS_CloseDir(pDir);
		return;
	}

	// Ask if the user would like to archive now
	if (!ArchiveSessionsPopup())
		goto Exit_2;

	// Sort list of dates - oldest at the top, newest at the bottom
	qsort((void *)DateList, count, sizeof(U32), compare);

	// Determine cutoff threshold
	if ((index = count / 2) > CUTOFF_LIMIT)
		index = CUTOFF_LIMIT;
	cutoffDate = DateList[count - index];
	total = (count - index) - 2;		// count will now be the loop counter for the progress bar _ subtract 2 for the . and .. files
	count = 0;
	
	// Create Archive directory if it does not exist
	FS_MkDir(archiveDir);

	// Create the progress bar
	CreateProgressBar(&hProgBar, total);							
	GUI_Exec();

	// Move files older than halfway point to archive folder
	if (FS_FindFirstFile(&FindFileData, MY_SESSIONS_PATH, sourceFilename, MAX_FILENAME_SIZE) == 0)
	{
		do {
			if (UserPressedCancel())		// Watch for user cancel
				goto Exit_3;

			if (strlen(FindFileData.sFileName) > 2)		// if a valid file name
			{
				if (FileIsNotTQMFile(&FindFileData))
				{
					if (!MoveThisFile(&FindFileData))
						goto Exit_3;
					else count++;
				}

				else if (FindFileData.LastWriteTime < cutoffDate)	// Move files older (smaller) than cutoffDate
				{
					char scratch[50];
					
					// update the user
					sprintf(scratch, "Moving file %d of %d", count, total);
					PROGBAR_SetText(hProgBar, scratch);
					PROGBAR_SetValue(hProgBar, count);
					GUI_Exec();

					if (!MoveThisFile(&FindFileData))
						goto Exit_3;
					else count++;
				}
			}
			else count++;		// File count still counted this
		} while (FS_FindNextFile(&FindFileData));
	}

Exit_3:
	PROGBAR_Delete(hProgBar);
	FS_FindClose(&FindFileData);

Exit_2:
	FS_CloseDir(pDir);
	RepaintSplash();
}



//
// Return true if file IS NOT a tqm file
// otherwise return false
//
int FileIsNotTQMFile(FS_FIND_DATA *pFileData)
{
	int len;

	len = strlen(pFileData->sFileName);
	return (kmc_stricmp(".tqm", pFileData->sFileName + len - 4));
}

//
// Move this file into the archive directory
// If the file already exisits or there is a tranfer error try renaming it and trying again.
// Return true if successful
// Return false if failure
//
int MoveThisFile(FS_FIND_DATA *pFileData)
{
	char destFilename[MAX_FILENAME_SIZE];
	char tempFilename[MAX_FILENAME_SIZE];

	strcpy(destFilename, archiveDir);
	strcat(destFilename, "\\");
	strcat(destFilename, pFileData->sFileName);
	
	strcpy(tempFilename, MY_SESSIONS_PATH);
	strcat(tempFilename, "\\");
	strcat(tempFilename, pFileData->sFileName);

	// move the file
	if (FS_Move(tempFilename, destFilename) == -1)
	{
		// Can't read error so just try changing the name and trying again
		strcat(destFilename, "_2");
		if (FS_Move(tempFilename, destFilename) == -1)
			return false;
	}
	return true;
}


int compare(const void *a, const void *b)
{
	if ( *(U32 *)a  <  *(U32 *)b )
		return -1;
	if ( *(U32 *)a == *(U32 *)b )
		return 0;
	if ( *(U32 *)a  >  *(U32 *)b )
		return 1;
	return 0;			// Just in case
}

#endif

//
// Too many session files - ask if the user wants to archive.
//
// Returns	true - archive
//			false - just go on
//
int ArchiveSessionsPopup(void)
{
	static const GUI_WIDGET_CREATE_INFO ArchiveResources[] = {
		{ FRAMEWIN_CreateIndirect, "Archiving Sessions", 0, 50, 20, 360, 220, 0, 0 },
		{ BUTTON_CreateIndirect, "NOW", GUI_ID_OK,   35, 115, 120, 60 },
		{ BUTTON_CreateIndirect, "LATER", GUI_ID_CANCEL, 195, 115, 120, 60 }
	};

	SetPopupDefaults(WARNING_POPUP);

	if (GUI_ExecDialogBox(ArchiveResources, GUI_COUNTOF(ArchiveResources), &ArchiveSessionsCallback, WM_GetDesktopWindow(), 10, 10) == GUI_ID_OK)
		return true;
	else return false;
}


void ArchiveSessionsCallback(WM_MESSAGE * pMsg)
{
	WM_HWIN hDlg;
	int NCode, Id;
	BUTTON_Handle hBtn;

	hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_INIT_DIALOG:
			FRAMEWIN_SetTextAlign(hDlg, GUI_TA_HCENTER);
			FRAMEWIN_SetTextColor(hDlg, popupTitleTextColor);
			hText1 = TEXT_CreateEx(0, 40, 350, 20, hDlg, WM_CF_SHOW, GUI_TA_CENTER | GUI_TA_VCENTER, 0,    "A large number of session files");
			hText2 = TEXT_CreateEx(0, 70, 350, 20, hDlg, WM_CF_SHOW, GUI_TA_CENTER | GUI_TA_VCENTER, 0,    "slows Traqmate operation. Would");
			hText3 = TEXT_CreateEx(0, 100, 350, 20, hDlg, WM_CF_SHOW, GUI_TA_CENTER | GUI_TA_VCENTER, 0, "you like to archive the older files?");
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
					{
						// Change screen to "progress screen"
					TEXT_Delete(hText1);
					TEXT_Delete(hText2);
					TEXT_Delete(hText3);
					TEXT_CreateEx(0, 35, 350, 30, hDlg, WM_CF_SHOW, GUI_TA_CENTER | GUI_TA_VCENTER, 0, "This may take a moment");
					hBtn = BUTTON_CreateEx(120, 70, 120, 60, hDlg, WM_CF_SHOW, 0, GUI_ID_CANCEL);
					BUTTON_SetText(hBtn, "Cancel");
					GUI_Delay(1);			// Pause momentarily for the screen updates to be drawn
					GUI_EndDialog(hDlg, Id);
					}
					break;

				case GUI_ID_CANCEL:
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

int UserPressedCancel(void)
{
	GUI_PID_STATE TouchState;

	GUI_TOUCH_GetState(&TouchState);	// Get the touch position in pixels
	if (TouchState.Pressed)				// If touch pressed
	{
		int x, y;

		x = TouchState.x;
		y = TouchState.y;

		// See if the is the Cancel Key pos
		if ((x > 175) && (x < 300))
			if ((y > 95) && (y < 160))
				return true;
	}
	return false;
}

