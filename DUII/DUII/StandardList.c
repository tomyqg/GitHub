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
#include <stdarg.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "StandardList.h"


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
// Note that the button bitmaps must be encoded for the appropriate color
// conversion selected in LCDConf.c
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmOrangeButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTransparentButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlackBlankButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTopFakeButton;


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
#define	POSITION_ONE	topPosition + (2 * (gap + BUTTON_HEIGHT))
#define	POSITION_TWO	topPosition + (3 * (gap + BUTTON_HEIGHT))
#define	POSITION_THREE	topPosition + (4 * (gap + BUTTON_HEIGHT))
#define	POSITION_FOUR	topPosition + gap + BUTTON_HEIGHT
#define	POSITION_FIVE	topPosition = 51

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
BUTTON_Handle hBtn1, hBtn2, hBtn3, hBtn4, hBtn5;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
void DisplayStdListScreen(WM_HWIN hParent,...)
{
	int top = 0;
	int topPosition, gap;
	const GUI_BITMAP *pGraphic;
	BUTTON_Handle hBtn;

	char **pList;
	int listWheelID;
	LISTWHEEL_Handle *pWheel;
	int selection;
	enum LIST_TYPE lType;
	char *pText;
	char *pButtonOneText;
	int buttonOneID;
	char *pButtonTwoText;
	int buttonTwoID;
	char *pButtonThreeText;
	int buttonThreeID;
	char *pButtonFourText;
	int buttonFourID;
	char *pButtonFiveText;
	int buttonFiveID;
	va_list ap;

	va_start(ap, hParent);
	pList = va_arg(ap, char **);
	listWheelID = va_arg(ap, int);
	pWheel = va_arg(ap, long *);
	selection = va_arg(ap, int);
	lType = va_arg(ap, enum LIST_TYPE);
	pText = va_arg(ap, char *);
	pButtonOneText = va_arg(ap, char *);
	buttonOneID = va_arg(ap, int);
	pButtonTwoText = va_arg(ap, char *);
	buttonTwoID = va_arg(ap, int);
	pButtonThreeText = va_arg(ap, char *);
	buttonThreeID = va_arg(ap, int);
	pButtonFourText = va_arg(ap, char *);
	buttonFourID = va_arg(ap, int);
	pButtonFiveText = va_arg(ap, char *);
	buttonFiveID = va_arg(ap, int);
	va_end(ap);

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	if (listWheelID)
	{
		*pWheel = LISTWHEEL_CreateEx(SPINNER_START + 16, 50, 205, 205, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, listWheelID, pList);
		// Show 5 entries
		LISTWHEEL_SetLineHeight(*pWheel, 41);
		LISTWHEEL_SetSnapPosition(*pWheel, 87);
		LISTWHEEL_SetFont(*pWheel, &GUI_Font24_ASCII);
		LISTWHEEL_SetTextAlign(*pWheel, GUI_TA_LEFT);

		if (selection < 0)	// Safety check!
			selection = 0;

		LISTWHEEL_SetPos(*pWheel, selection);
		LISTWHEEL_SetSel(*pWheel, selection);
	}

	topPosition = 38;
	gap = 6;

	switch (lType)
	{
		case FIVE_BOTTON_NO_TEXT:
			hBtn5 = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, POSITION_FIVE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, buttonFiveID);
			BUTTON_SetBitmap(hBtn5, BUTTON_BI_UNPRESSED, &bmOrangeButton);
			BUTTON_SetText(hBtn5, pButtonFiveText);
			BUTTON_SetTextColor(hBtn5, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hBtn5, BUTTON_BI_PRESSED, GUI_BLACK);
			BUTTON_SetFocussable(hBtn5, 0);
			gap = 3;

		case FOUR_BUTTON:
			hBtn4 = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, POSITION_FOUR, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, buttonFourID);
			BUTTON_SetBitmap(hBtn4, BUTTON_BI_UNPRESSED, &bmBlueButton);
			BUTTON_SetText(hBtn4, pButtonFourText);
			BUTTON_SetTextColor(hBtn4, BUTTON_BI_UNPRESSED, GUI_WHITE);
			BUTTON_SetFocussable(hBtn4, 0);

		case THREE_BUTTON:
			hBtn3 = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, POSITION_THREE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, buttonThreeID);
			BUTTON_SetBitmap(hBtn3, BUTTON_BI_UNPRESSED, &bmRedButton);
			BUTTON_SetText(hBtn3, pButtonThreeText);
			BUTTON_SetTextColor(hBtn3, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hBtn3, BUTTON_BI_PRESSED, GUI_WHITE);
			BUTTON_SetFocussable(hBtn3, 0);

		case TWO_BUTTON:
			hBtn2 = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, POSITION_TWO, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, buttonTwoID);
			BUTTON_SetBitmap(hBtn2, BUTTON_BI_UNPRESSED, &bmYellowButton);
			BUTTON_SetText(hBtn2, pButtonTwoText);
			BUTTON_SetTextColor(hBtn2, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hBtn2, BUTTON_BI_PRESSED, GUI_WHITE);
			BUTTON_SetFocussable(hBtn2, 0);

		case ONE_BUTTON:
			hBtn1 = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS, POSITION_ONE, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, buttonOneID);
			BUTTON_SetBitmap(hBtn1, BUTTON_BI_UNPRESSED, &bmGreenButton);
			BUTTON_SetText(hBtn1, pButtonOneText);
			BUTTON_SetTextColor(hBtn1, BUTTON_BI_UNPRESSED, GUI_BLACK);
			BUTTON_SetTextColor(hBtn1, BUTTON_BI_PRESSED, GUI_WHITE);
			BUTTON_SetFocussable(hBtn1, 0);
			break;
	}

	// Display static text
	switch (lType)
	{
	case TEXT_ONLY:
		top = 129;
		pGraphic = &bmTransparentButton;
		break;

	case FOUR_BUTTON:
		top = 45;
		pGraphic = &bmTopFakeButton;
		break;

	case THREE_BUTTON:
	case TWO_BUTTON:
	case ONE_BUTTON:
		top = 83;
		pGraphic = &bmBlackBlankButton;
		break;

	case FIVE_BOTTON_NO_TEXT:
	case FOUR_BUTTON_NO_TEXT:
		return;
	}

	hBtn = BUTTON_CreateEx(SPINNER_SCREEN_BUTTON_POS,  top, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, 0);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, pGraphic);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetText(hBtn, pText);
}


void DisplayTripleListScreen(WM_HWIN hParent,
			char *pList1[], int listWheelID1, LISTWHEEL_Handle *pWheel1, int selection1,
			char *pList2[], int listWheelID2, LISTWHEEL_Handle *pWheel2, int selection2,
			char *pList3[], int listWheelID3, LISTWHEEL_Handle *pWheel3, int selection3,
			char *pText,
			//char *pOptionText1, char *pOptionText2,
			char *pButtonText, int buttonID)
{
	BUTTON_Handle hBtn;

	BUTTON_SetDefaultFont(&GUI_Font24_ASCII);

	// Display first (left) wheel
	*pWheel1 = LISTWHEEL_CreateEx(TRIPLE_SPINNER_START_1, 50, 69, 208, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, listWheelID1, pList1);
	LISTWHEEL_SetLineHeight(*pWheel1, 41);
	LISTWHEEL_SetSnapPosition(*pWheel1, 87);
	LISTWHEEL_SetFont(*pWheel1, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(*pWheel1, GUI_TA_CENTER);
	if (selection1 < 0)	// Safety check!
		selection1 = 0;
	LISTWHEEL_SetPos(*pWheel1, selection1);
	LISTWHEEL_SetSel(*pWheel1, selection1);

	// Display second (middle) wheel
	*pWheel2 = LISTWHEEL_CreateEx(TRIPLE_SPINNER_START_2, 50, 69, 208, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, listWheelID2, pList2);
	LISTWHEEL_SetLineHeight(*pWheel2, 41);
	LISTWHEEL_SetSnapPosition(*pWheel2, 87);
	LISTWHEEL_SetFont(*pWheel2, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(*pWheel2, GUI_TA_CENTER);
	if (selection2 < 0)	// Safety check!
		selection2 = 0;
	LISTWHEEL_SetPos(*pWheel2, selection2);
	LISTWHEEL_SetSel(*pWheel2, selection2);

	// Display third (right) wheel
	*pWheel3 = LISTWHEEL_CreateEx(TRIPLE_SPINNER_START_3, 50, 69, 208, hParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, listWheelID3, pList3);
	LISTWHEEL_SetLineHeight(*pWheel3, 41);
	LISTWHEEL_SetSnapPosition(*pWheel3, 87);
	LISTWHEEL_SetFont(*pWheel3, &GUI_Font24_ASCII);
	LISTWHEEL_SetTextAlign(*pWheel3, GUI_TA_CENTER);
	if (selection3 < 0)	// Safety check!
		selection3 = 0;
	LISTWHEEL_SetPos(*pWheel3, selection3);
	LISTWHEEL_SetSel(*pWheel3, selection3);

	// Display first (standard) line of text
	hBtn = BUTTON_CreateEx(TRIPLE_SPINNER_TEXT_START,  129, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, 0);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmTransparentButton);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetText(hBtn, pText);

	//if (pOptionText1)
	//{
	//	// Display second (optional) line of text
	//	hBtn = BUTTON_CreateEx(BUTTON_LEFT,  160, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, 0);
	//	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlackBlankButton);
	//	BUTTON_SetFont(hBtn, &GUI_Font20_ASCII);
	//	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_GREEN);
	//	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_GREEN);
	//	BUTTON_SetText(hBtn, pOptionText1);
	//}

	//if (pOptionText2)
	//{
	//	// Display third (standard) line of text
	//	hBtn = BUTTON_CreateEx(BUTTON_LEFT,  185, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, 0);
	//	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlackBlankButton);
	//	BUTTON_SetFont(hBtn, &GUI_Font16_ASCII);
	//	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
	//	BUTTON_SetTextColor(hBtn, BUTTON_BI_PRESSED, GUI_WHITE);
	//	BUTTON_SetText(hBtn, pOptionText2);
	//}

	// Display optional bottom button
	if (pButtonText)
	{
		hBtn = BUTTON_CreateEx(TRIPLE_SPINNER_TEXT_START, 218, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, buttonID);
		BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
		BUTTON_SetText(hBtn, pButtonText);
		BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetFocussable(hBtn, 0);
	}
}

GUI_HWIN GetButtonID(int button)
{
	switch (button)
	{
	default:
	case ONE_BUTTON:			return hBtn1;
	case TWO_BUTTON:			return hBtn2;
	case THREE_BUTTON:			return hBtn3;
	case FOUR_BUTTON:			return hBtn4;
	case FIVE_BOTTON_NO_TEXT:
	case FOUR_BUTTON_NO_TEXT:	return hBtn5;
	}
}


/*************************** End of file ****************************/

