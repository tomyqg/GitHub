//
//	AnalogInputScale.c
//
//	Gathers user input for analog input scale settings
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DataHandler.h"
#include "InputScreens.h"
#include "KeyboardAccess.h"
#include "PersonalizeScreens.h"
#include "Utilities.h"
#include "GeneralConfig.h"
#include "SystemSettingsScreens.h"
#include "FlashReadWrite.h"
#ifndef _WINDOWS
#include "dataflash.h"
#include "FS.h"
#include "WiFiFunctions.h"
#endif

/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
#ifdef _WINDOWS
void trimCR(char *pStr);
#endif

void PersonalizeSaveData(const void *type);
int PasswordTooSmallPopUp(void);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
#define	DEFAULT_WIFI_PASSWORD	"traqmate"


/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hPersonalizeEntryScreen;
static void *pDataEntryKeyboard = 0;
static char *pDest;
char workString[51];
static enum PERSONALIZE_SCREEN_ID screenID;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetPersonalizeEntryWindowHandle(void)
{
	return hPersonalizeEntryScreen;
}

void SetPersonalizeEntryWindowHandle(WM_HWIN hWin)
{
	hPersonalizeEntryScreen = hWin;
}


void PersonalizeSaveData(const void *pData)
{
	switch (screenID)
	{
	case OWNER_SCREEN:
	case CONTACTINFO_SCREEN:
		break;

	case WIFI_NAME_SCREEN:
#ifndef _WINDOWS
		WiFiSetup(true);
#endif
		break;

	case WIFI_PASSWORD_SCREEN:
#ifndef _WINDOWS
		WiFiSetup(true);
#endif
		break;

	case SERIALNUMBER_SCREEN:
		{
			unsigned int tempi;

			if ((tempi = atoi(workString)) == 0x7FFFFFFF)	// atoi and atol will only handle up to 0x7FFFFFFF!
			{
				char ch, size;

				size = strlen(workString);
				ch = workString[size-1];
				workString[size-1] = 0;
				tempi = atoi(workString);
				tempi = (tempi * 10) + (ch - '0');
			}
			unsavedSysData.unitInfo.serno = tempi;
		}
		break;
	case BUILDDATE_SCREEN:
		unsavedSysData.unitInfo.week = atoi(workString) / 100;
		unsavedSysData.unitInfo.year = atoi(workString) % 100;
		break;
	case TEMP_SCREEN:
		{
			int temp;

			temp = atoi(workString);
			if (sysData.units == STD)
			{
				float tempF = f_to_c((float)temp);
				if (tempF > 0.0)
					tempF += 0.5;
				else tempF -= 0.5;
				temp = (int)tempF;
			}
			sysData.calibrationTemperature = temp;
		}
		break;
	}

#ifdef _WINDOWS
	// For PC simulation save the owner name and phone number in a file to be displayed
	// at power up.  This data was not previously being saved in the PC simulation.
{
	FILE *pFile;

	if ((pFile = fopen(PERSONALIZE_FILE_NAME, "w")) != NULL)
	{
		fputs(unsavedSysData.unitInfo.owner, pFile);
		fputc('\n', pFile);
		fputs(unsavedSysData.unitInfo.phone, pFile);
		fputc('\n', pFile);
		fputs(unsavedSysData.unitInfo.wifiName, pFile);
		fputc('\n', pFile);
		fputs(unsavedSysData.unitInfo.wifiPass, pFile);
		fputc('\n', pFile);
		fclose(pFile);
	}
}
#else
	WriteUserData();			// Save the data to on-board flash
#endif
}

#ifdef _WINDOWS

void LoadPersonalizationData(void)
{
	FILE *pTestFile;

	// Load up porsonalization data
	if ((pTestFile = fopen(PERSONALIZE_FILE_NAME, "r")) != NULL)
	{
		if (fgets(unsavedSysData.unitInfo.owner, OWNER_LEN, pTestFile) != NULL)
			trimCR(unsavedSysData.unitInfo.owner);
		if (fgets(unsavedSysData.unitInfo.phone, PHONE_LEN, pTestFile) != NULL)
			trimCR(unsavedSysData.unitInfo.phone);
		if (fgets(unsavedSysData.unitInfo.wifiName, WIFI_LEN, pTestFile) != NULL)
			trimCR(unsavedSysData.unitInfo.wifiName);
		if (fgets(unsavedSysData.unitInfo.wifiPass, WIFI_LEN, pTestFile) != NULL)
			trimCR(unsavedSysData.unitInfo.wifiPass);
		fclose(pTestFile);

		// Validate data - particularly wifiPass as that may have been uninitialized data  
		// since it may have been past the end of the old "name" field string.
		if (!(isalnum(unsavedSysData.unitInfo.wifiName[0])))
			unsavedSysData.unitInfo.wifiName[0] = '\0';
		if (!(isalnum(unsavedSysData.unitInfo.wifiPass[0])))
			unsavedSysData.unitInfo.wifiPass[0] = '\0';
	}
}

void trimCR(char *pStr)
{
	int len;

	len = strlen(pStr);
	if (len != 0)
	{
		pStr += len - 1;
		if (*pStr == '\n')
			*pStr = 0;
	}
}

#endif


void PersonalizeEntryCB(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
			ReturnToPersonalizationScreen(hPersonalizeEntryScreen);
			break;

		case KB_SAVE_MSG:
		case KB_SAVE_AND_EXIT_MSG:
			if (screenID == WIFI_PASSWORD_SCREEN)
			{
				if (strlen(unsavedSysData.unitInfo.wifiPass) < 8)
				{
					// display popup
					if (PasswordTooSmallPopUp() == GUI_ID_OK)
					{
						// Must redisplay the keyboard
						DisplayNewKeyboardScreen(WIFI_PASSWORD_SCREEN, hPersonalizationScreen);
						break;
					}
					else strcpy(unsavedSysData.unitInfo.wifiPass, DEFAULT_WIFI_PASSWORD);
				}
			}
			PersonalizeSaveData(pMsg->Data.p);
			ReturnToPersonalizationScreen(hPersonalizeEntryScreen);
			break;

		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			PersonalizeIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == GEAR_RATIOS)
			{
				DrawSignalStrength();
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}


static const GUI_WIDGET_CREATE_INFO PasswordTooSmall[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Error", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The WiFi Password must be at least", 0, 0, 10, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "8 characters long. Please enter a", 0, 0, 40, 350, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "longer password or select default.",      0, 0, 70, 350, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "EDIT", GUI_ID_OK, 30, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "USE DEFAULT", GUI_ID_CANCEL,   170, 115, 150, 60 },
};


//
// Returns	GUI_ID_OK to go back and edit password again or
//			GUI_ID_CANCEL - to accept the default password
//
int PasswordTooSmallPopUp(void)
{
	SetPopupDefaults(ERROR_POPUP);
	return GUI_ExecDialogBox(PasswordTooSmall, GUI_COUNTOF(PasswordTooSmall), &popupCallback, hPersonalizationScreen, 10, 10);
}


void DisplayNewKeyboardScreen(enum PERSONALIZE_SCREEN_ID id, GUI_HWIN hPrev)
{
	char *pTitle;
	int size, lock;
	Keyboard_Type type;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenType(PERSONALIZE_ENTRY_SCREEN_TYPE);

	hPersonalizeEntryScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, PersonalizeEntryCB, 0);
		
	if (pDataEntryKeyboard)
		DestroyKeyboard(pDataEntryKeyboard);

	switch (screenID = id)
	{
	default:
	case OWNER_SCREEN:
		pTitle = "Enter Owner Name";
		pDest = unsavedSysData.unitInfo.owner;
		size = OWNER_LEN-1;
		type = ALPHA;
		lock = false;
		break;
	case CONTACTINFO_SCREEN:
		pTitle = "Enter Contact Info";
		pDest = unsavedSysData.unitInfo.phone;
		size = PHONE_LEN-1;
		type = NUMERIC;
		lock = false;
		break;
	case WIFI_NAME_SCREEN:
		pTitle = "Enter Wifi Name";
		pDest = unsavedSysData.unitInfo.wifiName;
		size = WIFI_LEN-1;
		type = ALPHA;
		lock = false;
		break;
	case WIFI_PASSWORD_SCREEN:
		pTitle = "Enter Wifi Password";
		pDest = unsavedSysData.unitInfo.wifiPass;
		size = WIFI_LEN-1;
		type = ALPHA;
		lock = false;
		break;
	case SERIALNUMBER_SCREEN:
		pTitle = "Enter Serial Number";
		pDest = workString;
		sprintf(workString, "%u", unsavedSysData.unitInfo.serno);
		size = 10;
		type = NUMERIC;
		lock = true;
		break;
	case BUILDDATE_SCREEN:
		pTitle = "Enter Build Date wwyy";
		pDest = workString;
		sprintf(workString, "%02d%02d", unsavedSysData.unitInfo.week, unsavedSysData.unitInfo.year);
		trim(workString);
		size = 4;
		type = NUMERIC;
		lock = true;
		break;
	case TEMP_SCREEN:
		{
			int temp;

			temp = sysData.calibrationTemperature;
			if (sysData.units == METRIC)
			{
				pTitle = "Enter current temp in C (integer only)";
			}
			else
			{
				pTitle = "Enter current temp in F (integer only)";
				temp = (int)c_to_f((float)temp);	// Temp stored in C
			}
			sprintf(workString, "%d", temp);
			pDest = workString;

		//	trim(workString);		// DO NOT trim - truncates 100 to 1!!!
			size = 3;
			type = NUMERIC;
			lock = true;
		}
		break;
	}

	pDataEntryKeyboard = CreateKeyboard(pTitle, pDest, type, lock, size, id, hPersonalizeEntryScreen, true);

}



void PersonalizeIgnoreNextKeyRelease(void)
{
}

void DeletePersonalizeKeyboards(void)
{
	if (pDataEntryKeyboard)
	{
		DestroyKeyboard(pDataEntryKeyboard);
		pDataEntryKeyboard = 0;
	}
}

/*************************** End of file ****************************/

