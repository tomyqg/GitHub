//
//	AnalogInputScale.c
//
//	Gathers user input for analog input scale settings
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DataHandler.h"
#include "InputScreens.h"
#include "KeyboardAccess.h"
#include "GearRatios.h"
#include "Utilities.h"
#include "GeneralConfig.h"


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
void DisplayFirst(WM_HWIN);
void DisplaySecond(WM_HWIN);
void DisplayThird(WM_HWIN);
void DisplayFourth(WM_HWIN);
void DisplayFifth(WM_HWIN);
void DisplaySixth(WM_HWIN);
void DisplaySeventh(WM_HWIN);
void DisplayEighth(WM_HWIN);

void FirstPaint(WM_HWIN);
void SecondPaint(WM_HWIN);
void ThirdPaint(WM_HWIN);
void FourthPaint(WM_HWIN);
void FifthPaint(WM_HWIN);
void SixthPaint(WM_HWIN);
void SeventhPaint(WM_HWIN);
void EighthPaint(WM_HWIN);

void GearRatiosSaveData(const void *type);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST GearRatiosScreenList[NUM_GEAR_RATIOS_SCREENS+1] = {
	{ FIRST_GEAR_SCREEN, DisplayFirst, FirstPaint },
	{ SECOND_GEAR_SCREEN, DisplaySecond, SecondPaint },
	{ THIRD_GEAR_SCREEN, DisplayThird, ThirdPaint },
	{ FOURTH_GEAR_SCREEN, DisplayFourth, FourthPaint },
	{ FIFTH_GEAR_SCREEN, DisplayFifth, FifthPaint },
	{ SIXTH_GEAR_SCREEN, DisplaySixth, SixthPaint },
	{ SEVENTH_GEAR_SCREEN, DisplaySeventh, SeventhPaint },
	{ EIGHTH_GEAR_SCREEN, DisplayEighth, EighthPaint },
	{ -1, NULL, NULL }
};

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hGearRatios;
static void *pFirst = 0;
static void *pSecond = 0;
static void *pThird = 0;
static void *pFourth = 0;
static void *pFifth = 0;
static void *pSixth = 0;
static void *pSeventh = 0;
static void *pEighth = 0;

char gearRatioString[NUMBER_LENGTH+1];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetGearRatiosWindowHandle(void)
{
	return hGearRatios;
}

void SetGearRatiosWindowHandle(WM_HWIN hWin)
{
	hGearRatios = hWin;
}

void GearRatiosSaveData(const void *type)
{
	float temp;

	// KMC 2/11/12 - ensure no dashes ('-') in the string
	EnsurePositive(gearRatioString);
	if ((temp = atof(gearRatioString)) >= 100.0)
		temp = 99.0;

	if (type == pFirst)
		vehicleData.gearRatios[0] = temp;
	else if (type == pSecond)
		vehicleData.gearRatios[1] = temp;
	else if (type == pThird)
		vehicleData.gearRatios[2] = temp;
	else if (type == pFourth)
		vehicleData.gearRatios[3] = temp;
	else if (type == pFifth)
		vehicleData.gearRatios[4] = temp;
	else if (type == pSixth)
		vehicleData.gearRatios[5] = temp;
	else if (type == pSeventh)
		vehicleData.gearRatios[6] = temp;
	else if (type == pEighth)
		vehicleData.gearRatios[7] = temp;
}

void GearRatiosCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
			DisplayNewScreenList(GEARING_SCREENS_TYPE, GetLastScreen(GEARING_SCREENS_TYPE), hGearRatios);
			break;

		case KB_SAVE_MSG:
			GearRatiosSaveData(pMsg->Data.p);
			break;

		case KB_SAVE_AND_EXIT_MSG:
			GearRatiosSaveData(pMsg->Data.p);
			DisplayNewScreenList(GEARING_SCREENS_TYPE, GetLastScreen(GEARING_SCREENS_TYPE), hGearRatios);
			break;

		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			GearRatiosIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == GEAR_RATIOS)
			{
				// Call paint method of center screen
				GearRatiosScreenList[GetLastScreen(GEAR_RATIOS)].paint(hGearRatios);
				DrawSignalStrength();
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void DisplayFirst(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pFirst)
	//	DestroyKeyboard(pFirst);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[0]);
	trim(gearRatioString);			// Trim off trailing 0s
	pFirst = CreateKeyboard("First Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, FIRST_GEAR_SCREEN, hParent, true);
}

void DisplaySecond(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pSecond)
	//	DestroyKeyboard(pSecond);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[1]);
	trim(gearRatioString);
	pSecond = CreateKeyboard("Second Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, SECOND_GEAR_SCREEN, hParent, true);
}

void DisplayThird(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pThird)
	//	DestroyKeyboard(pThird);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[2]);
	trim(gearRatioString);
	pThird = CreateKeyboard("Third Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, THIRD_GEAR_SCREEN, hParent, true);
}

void DisplayFourth(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pFourth)
	//	DestroyKeyboard(pFourth);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[3]);
	trim(gearRatioString);
	pFourth = CreateKeyboard("Fourth Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, FOURTH_GEAR_SCREEN, hParent, true);
}

void DisplayFifth(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pFifth)
	//	DestroyKeyboard(pFifth);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[4]);
	trim(gearRatioString);
	pFifth = CreateKeyboard("Fifth Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, FIFTH_GEAR_SCREEN, hParent, true);
}

void DisplaySixth(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pSixth)
	//	DestroyKeyboard(pSixth);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[5]);
	trim(gearRatioString);
	pSixth = CreateKeyboard("Sixth Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, SIXTH_GEAR_SCREEN, hParent, true);
}

void DisplaySeventh(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pSeventh)
	//	DestroyKeyboard(pSeventh);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[6]);
	trim(gearRatioString);
	pSeventh = CreateKeyboard("Seventh Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, SEVENTH_GEAR_SCREEN, hParent, true);
}

void DisplayEighth(WM_HWIN hParent)
{
	DeleteGearRatiosKeyboards();
	//if (pEighth)
	//	DestroyKeyboard(pEighth);

	sprintf(gearRatioString, "%f", vehicleData.gearRatios[7]);
	trim(gearRatioString);
	pEighth = CreateKeyboard("Eighth Gear", gearRatioString, NUMERIC, true, NUMBER_LENGTH, EIGHTH_GEAR_SCREEN, hParent, true);
}

void FirstPaint(WM_HWIN hParent)
{
}


void SecondPaint(WM_HWIN hParent)
{
}

void ThirdPaint(WM_HWIN hParent)
{
}

void FourthPaint(WM_HWIN hParent)
{
}

void FifthPaint(WM_HWIN hParent)
{
}

void SixthPaint(WM_HWIN hParent)
{
}

void SeventhPaint(WM_HWIN hParent)
{
}

void EighthPaint(WM_HWIN hParent)
{
}

void GearRatiosIgnoreNextKeyRelease(void)
{
}

void DeleteGearRatiosKeyboards(void)
{
	if (pFirst)
	{
		DestroyKeyboard(pFirst);
		pFirst = 0;
	}
	if (pSecond)
	{
		DestroyKeyboard(pSecond);
		pSecond = 0;
	}
	if (pThird)
	{
		DestroyKeyboard(pThird);
		pThird = 0;
	}
	if (pFourth)
	{
		DestroyKeyboard(pFourth);
		pFourth = 0;
	}
	if (pFifth)
	{
		DestroyKeyboard(pFifth);
		pFifth = 0;
	}
	if (pSixth)
	{
		DestroyKeyboard(pSixth);
		pSixth = 0;
	}
	if (pSeventh)
	{
		DestroyKeyboard(pSeventh);
		pSeventh = 0;
	}
	if (pEighth)
	{
		DestroyKeyboard(pEighth);
		pEighth = 0;
	}
}

/*************************** End of file ****************************/

