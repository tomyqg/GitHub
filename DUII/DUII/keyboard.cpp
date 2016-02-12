/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: keyboard.cpp
Description: display a keyboard and a text box
-----------------------------------------------------------------
*/
#include "keyboard.hpp"
#include "GeneralConfig.h"
#include "GUI.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

extern "C" GUI_CONST_STORAGE GUI_BITMAP bmKeyboardBlank;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmNumPadBlank;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmCapital;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmNotCapital;
//extern "C" GUI_CONST_STORAGE GUI_BITMAP bmSave;
//extern "C" GUI_CONST_STORAGE GUI_BITMAP bmCancel;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmDeleteKey;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;

char * TopKeysChar [] = { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" };
char * MidKeysChar[] = { "A", "S", "D", "F", "G", "H", "J", "K", "L" };
char * LowKeysChar[] = { "Z", "X", "C", "V", "B", "N", "M" };
char * BottomKeysChar[] = { "-", "Space", ".", "Bksp"};

char * NumKeysChar[] = { "7", "8", "9", "4", "5", "6", "1", "2", "3" };
char * NumOneKeys[] = { "7", "8", "9"};
char * NumTwoKeys[] = { "4", "5", "6", "-"};
char * NumThreeKeys[] = { "0", "1", "2", "3", "."};
char * NumFourKeys[] = { "0", ".", "<-"};
char * NumKeys[] = {"7", "8", "9", "4", "5", "6", "1", "2", "3", "0", ".", "<-"};
void _cbKeyboardCallback(WM_MESSAGE * pMsg);
void SaveData(int message, Keyboard *key);

static Keyboard *kb[MAX_KEYBOARDS];
static WM_HWIN kbId[MAX_KEYBOARDS];

#define	AUTO_REPEAT_TIME	350

#define	MOVE_LIMIT	20

Keyboard::Keyboard(int x, int y, int iWidth, int iHeight, Keyboard_Type Option, char *DispText,
				   char *cTitle, char bLockMode, int iMaxTextLength, int iSource, WM_HWIN _hParent, char canScroll)
{
	int i;

// KMC FIX FOR PAINTING BUTTON TEXT
//	bInitializeClass = true;
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	xLoc = x;
	yLoc = y;
	hParent = _hParent;
	Width = iWidth;
	Height = iHeight;
	LockMode = bLockMode;
	strcpy(Title, cTitle);
	Source = iSource;
	scrollable = canScroll;
	touchDetected = false;
	hAutoRepeatTimer = 0;

	if ((_hDlg = WM_CreateWindowAsChild(x, y, Width, Height, _hParent, WM_CF_SHOW, _cbKeyboardCallback, 0)) <= 0)
	{
		SystemError();
		return;
	}
	// Find next free slot
	for (i = 0; i < MAX_KEYBOARDS; i++)
	{
		if (kb[i] == 0)
		{
			kb[i] = this;
			kbId[i] = _hDlg;
			break;
		}
	}

	WM_ShowWindow(_hDlg);

	xLoc = x;
	yLoc = y;

	Text = EDIT_CreateEx(22, 43, 353, 38, _hDlg, WM_CF_SHOW, 0, GUI_ID_TEXT0, iMaxTextLength);
	EDIT_SetFont(Text, &GUI_Font24B_ASCII);

	// Save pointer to and display original text
	if (strcmp(DispText, "0") == 0)		// If input string is "0"
		DispText[0] = 0;				// then display nothing at all
	EDIT_SetText(Text, DispText);
	maxTextLen = iMaxTextLength;
	OrgText = DispText;

	for(i = 0; i < NUM_ALPHA_BTNS; i++)
	{
		AlphaBtns[i] = NULL;
	}
	Caps = true;

	SetupKeyboard(Option);

	// KMC - do not allow use of the back key on ANY keyboard as it is too close to the CANCEL key and the keyboards already SAVE/CANCEL keys
	//GUI_DrawBitmap(&bmleftarrow, 0, 229);

// KMC FIX FOR PAINTING BUTTON TEXT
//	bInitializeClass = false;
}

void Keyboard::SetupKeyboard(Keyboard_Type Option)
{
	int i, x, y, index, Id;
	BUTTON_Handle btn;
	char c[2];

	// if we have existing buttons then we need to remove them
	if(AlphaBtns[0] != NULL)
	{
		for(i = 0; i < NUM_ALPHA_BTNS; i++)
		{
			if(AlphaBtns[i] != NULL)
			{
				WM_DeleteWindow(AlphaBtns[i]);
				AlphaBtns[i] = NULL;
			}
		}
	}

	WM_SelectWindow(_hDlg);

	BUTTON_SetDefaultBkColor(GUI_WHITE, BUTTON_CI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_CI_UNPRESSED);
	BUTTON_SetDefaultFont(&GUI_Font24B_ASCII);

	switch(Option)
	{
	case ALPHA:
		
		GUI_DrawBitmap(&bmKeyboardBlank, 0, 0);
		y = BUTTON_ROW_ONE_Y;
		for(i = 0; i < 10; i++)
		{
			x = BUTTON_LEFT_X + i * (BUTTON_WIDTH + BUTTON_SPACING);
			Id = GUI_ID_USER + (int)*TopKeysChar[i];
			btn = BUTTON_CreateEx(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, Id);

			if(Caps == false)
			{
				c[0] = tolower((int)*TopKeysChar[i]);
				c[1] = '\0';
				BUTTON_SetText(btn, c);
			}
			else
				BUTTON_SetText(btn, TopKeysChar[i]);

			AlphaBtns[i] = btn;
		}
		
		y = BUTTON_ROW_TWO_Y;
		for(i = 0; i < 9; i++)
		{
			index = i + 10;
			x = BUTTON_LEFT_X + i * (BUTTON_WIDTH + BUTTON_SPACING) + (BUTTON_WIDTH / 2) + 3;
			Id = GUI_ID_USER + (int)*MidKeysChar[i];
			btn = BUTTON_CreateEx(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, Id);
			//BUTTON_SetText(btn, MidKeysChar[i]);
			if(Caps == false)
			{
				c[0] = tolower((int)*MidKeysChar[i]);
				c[1] = '\0';
				BUTTON_SetText(btn, c);
			}
			else
				BUTTON_SetText(btn, MidKeysChar[i]);
			//BUTTON_SetBkColor(btn, BUTTON_CI_UNPRESSED, GUI_WHITE);
			//BUTTON_SetTextColor(btn, BUTTON_CI_UNPRESSED, GUI_BLACK);
			AlphaBtns[index] = btn;
		}
		
		y = BUTTON_ROW_THREE_Y;
		for(i = 0; i < 7; i++)
		{
			index = i + 19;
			x = BUTTON_LEFT_X + i * (BUTTON_WIDTH + BUTTON_SPACING) + BUTTON_WIDTH + BUTTON_WIDTH / 4 - 1;
			Id = GUI_ID_USER + (int)*LowKeysChar[i];
			btn = BUTTON_CreateEx(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, Id);
			//BUTTON_SetText(btn, LowKeysChar[i]);
			if(Caps == false)
			{
				c[0] = tolower((int)*LowKeysChar[i]);
				c[1] = '\0';
				BUTTON_SetText(btn, c);
			}
			else
				BUTTON_SetText(btn, LowKeysChar[i]);
			//BUTTON_SetBkColor(btn, BUTTON_CI_UNPRESSED, GUI_WHITE);
			//BUTTON_SetTextColor(btn, BUTTON_CI_UNPRESSED, GUI_BLACK);
			AlphaBtns[index] = btn;
		}

		// create the uppercase button
		x = BUTTON_LEFT_X - 3;
		Id =  GUI_ID_USER + UPPER_BTN_ID;
		btn = BUTTON_CreateEx(x - 3, y - 4, 43, 42, _hDlg, WM_CF_SHOW, 0, Id);
		//BUTTON_SetBitmap(btn, BUTTON_BI_UNPRESSED, &bmNotCapital);
		if(Caps == true)
		{
			BUTTON_SetBitmap(btn, BUTTON_BI_UNPRESSED, &bmCapital);
		}
		else
		{
			BUTTON_SetBitmap(btn, BUTTON_BI_UNPRESSED, &bmNotCapital);
		}

		index++;
		AlphaBtns[index++] = btn;
		y += BUTTON_HEIGHT + 5;
		x = 5;
		//width = BUTTON_WIDTH;

		index = 26;
		break;
	case NUMERIC:
		GUI_DrawBitmap(&bmNumPadBlank, 0, 0);

		y = BUTTON_ROW_ONE_Y;
		for(i = 0; i < 3; i++)
		{
			x = NUM_BTN_LEFT_X + i * (BUTTON_WIDTH + BUTTON_SPACING);
			Id = GUI_ID_USER + (int)*NumOneKeys[i];
			btn = BUTTON_CreateEx(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, Id);
			
			BUTTON_SetText(btn, NumOneKeys[i]);
			AlphaBtns[i] = btn;

		}
		y = BUTTON_ROW_TWO_Y;
		for(i = 0; i < 4; i++)
		{
			index = i + 3;
			x = NUM_BTN_LEFT_X + i * (BUTTON_WIDTH + BUTTON_SPACING);
			Id = GUI_ID_USER + (int)*NumTwoKeys[i];
			btn = BUTTON_CreateEx(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, Id);
			
			BUTTON_SetText(btn, NumTwoKeys[i]);
			AlphaBtns[index] = btn;
		}
		y = BUTTON_ROW_THREE_Y;
		for(i = 0; i < 5; i++)
		{
			index = i + 7;
			x = NUM_BTN_LEFT_X + i * (BUTTON_WIDTH + BUTTON_SPACING) - (BUTTON_WIDTH + BUTTON_SPACING);
			Id = GUI_ID_USER + (int)*NumThreeKeys[i];
			btn = BUTTON_CreateEx(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, Id);
			
			BUTTON_SetText(btn, NumThreeKeys[i]);
			AlphaBtns[index] = btn;

		}

		//index = 12;
		break;
	};

	if(LockMode == false)
	{
		AlphaNumBtn = BUTTON_CreateEx(382, BUTTON_ROW_THREE_Y, 80, BUTTON_HEIGHT, _hDlg, WM_CF_SHOW, 0, GUI_ID_USER + NUM_BTN_ID);
		if(Option == ALPHA)
			BUTTON_SetText(AlphaNumBtn, ".-123");
		else
			BUTTON_SetText(AlphaNumBtn, "ABC");
	}
	else
	{
		// ALPH/NUM button not active - cover over blank button
		GUI_SetColor(GUI_BLACK);
		GUI_FillRect(352, 180, 470, 225);
	}

	btn = BUTTON_CreateEx(380, 42, 80, 35, _hDlg, WM_CF_SHOW, 0, GUI_ID_USER + DEL_BTN_ID);
	BUTTON_SetText(btn, "Del");
	BUTTON_SetTextAlign(btn, GUI_TA_RIGHT | GUI_TA_BOTTOM);
	BUTTON_SetBitmap(btn, BUTTON_BI_UNPRESSED, &bmDeleteKey);
	SetBtnStd(btn);

	btn = BUTTON_CreateEx(BUTTON_LEFT_X - 5, BUTTON_ROW_THREE_Y + BUTTON_HEIGHT + BUTTON_SPACING - 1, 106, BUTTON_HEIGHT + 1, _hDlg, WM_CF_SHOW, 0, GUI_ID_USER + CANCEL_BTN_ID);
	BUTTON_SetText(btn, "Cancel");
	BUTTON_SetBkColor(btn, BUTTON_CI_UNPRESSED, GUI_YELLOW);
	SetBtnStd(btn);

	btn = BUTTON_CreateEx(134, BUTTON_ROW_THREE_Y + BUTTON_HEIGHT + BUTTON_SPACING - 1, 215, BUTTON_HEIGHT + 1, _hDlg, WM_CF_SHOW, 0, GUI_ID_USER + SPACE_BTN_ID);
	BUTTON_SetText(btn, "Space");
	SetBtnStd(btn);

	btn = BUTTON_CreateEx(359, BUTTON_ROW_THREE_Y + BUTTON_HEIGHT + BUTTON_SPACING - 1, 105, BUTTON_HEIGHT + 1, _hDlg, WM_CF_SHOW, 0, GUI_ID_USER + SAVE_BTN_ID);
	BUTTON_SetText(btn, "Save");
	BUTTON_SetBkColor(btn, BUTTON_CI_UNPRESSED, GUI_GREEN);
	BUTTON_SetBkColor(btn, BUTTON_CI_PRESSED, GUI_RED);
	SetBtnStd(btn);

	DisplayType = Option;

	// draw the title
	GUI_SetFont(&GUI_Font24B_ASCII);
	x = xLoc + Width / 2 - GUI_GetStringDistX(Title) / 2;
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringAt(Title, x, 5);
	GUI_SetColor(GUI_BLACK);
}


void Keyboard::SetBtnStd(BUTTON_Handle btn)
{
	BUTTON_SetTextColor(btn, BUTTON_CI_UNPRESSED, GUI_BLACK);
	BUTTON_SetTextColor(btn, BUTTON_CI_PRESSED, GUI_WHITE);
	BUTTON_SetFont(btn, &GUI_Font32B_ASCII);
	BUTTON_SetSkinClassic(btn);
	WM_BringToTop(btn);
}

void Keyboard::ChangeKeyboardCase(bool Capital)
{
	char c[2];
	int i;

	if(AlphaBtns[0] != NULL)
	{
		for(i = 0; i < NUM_ALPHA_BTNS; i++)
		{
			if(AlphaBtns[i] != NULL)
			{
				BUTTON_GetText(AlphaBtns[i], c, 2);
				if (c[0] != 0)
				{
					if(Capital == true)
						c[0] = toupper(c[0]);
					else
						c[0] = tolower(c[0]);
					BUTTON_SetText(AlphaBtns[i], c);
				}
			}
		}
	}

}

void Keyboard::GetValue(char *str, int max)
{
	EDIT_GetText(Text, str, max+1);
}


Keyboard::~Keyboard()
{
	// Search for this keyboard and remove it.
	// Since buttons are owned by the window then deleting the window should get rid of the children buttons
	for (int i = 0; i < MAX_KEYBOARDS; i++)
	{
		if (kb[i] == this)
		{
			kb[i] = 0;
			kbId[i] = 0;

			WM_DeleteWindow(this->_hDlg);

			return;
		}
	}
}

void _cbKeyboardCallback(WM_MESSAGE * pMsg) {
	int NCode, Id;
	int i;
	WM_MESSAGE msg;
	Keyboard *key = 0;

	//if (pMsg->hWinSrc < 0)
	//{
	//	WM_DefaultProc(pMsg);
	//	return;
	//}
	for(i=0; i < MAX_KEYBOARDS; i++)
	{
		if (pMsg->hWin == kbId[i])
		{
			key = kb[i];
			break;
		}
	}
	if ((i == MAX_KEYBOARDS) || (key == 0))
	{
		return;
	}

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == key->hAutoRepeatTimer)
			{
				EDIT_AddKey(key->Text, DEL_BTN_ID);
				WM_RestartTimer(key->hAutoRepeatTimer, AUTO_REPEAT_TIME);
			}
			break;

//
// KMC 9/9/12 - removed old code involved in screen scrolling.  Was causing issues of misinterpreting presses
// off the working screen area as valid keys.  Since we no longer support screen scrolling this code can
// simlpy be removed.
//
//		case WM_TOUCH:
//			{
//				// If the non-keyboard area is touched and moves more than MOVE_LIMIT pixels then we may be scrolling off this screen -
//				// so save the data.
//				GUI_PID_STATE TouchState;
//
//				GUI_TOUCH_GetState(&TouchState);
//				if (TouchState.Pressed)
//				{
//					if (key->touchDetected)
//					{
//						if ((abs(key->StartPos.y - TouchState.y) > MOVE_LIMIT) || (abs(key->StartPos.x - TouchState.x) > MOVE_LIMIT))		// If screen moving
//						{
//							SaveData(KB_SAVE_MSG, key);
//						}
//					}
//					else
//					{
//						GUI_TOUCH_GetState(&key->StartPos);
//						key->touchDetected = true;
//					}
//				}
//				else key->touchDetected = false;
//			}
//			break;

		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
			NCode = pMsg->Data.v;                 // Notification code

		    switch(NCode)
		    {
				case WM_NOTIFICATION_MOVED_OUT:
				case WM_NOTIFICATION_RELEASED:
					if (Id == GUI_ID_USER + DEL_BTN_ID)
					{
						WM_DeleteTimer(key->hAutoRepeatTimer);
						key->hAutoRepeatTimer = 0;
					}
					else
					{
						int x, y;
					// If a is released and we are no longer
					// at the 'home' position then we may be scrolling off this screen -
					// remove the last key added and save the data.
						GUI_GetOrg(&x, &y);
						if ((y != YSIZE_PHYS) && (y != 0))
						{
							EDIT_AddKey(key->Text, (int)GUI_KEY_BACKSPACE);
							SaveData(KB_SAVE_MSG, key);
						}
					}
					break;

				case WM_NOTIFICATION_CLICKED:
					switch(Id - GUI_ID_USER)
					{
					case NUM_BTN_ID:
						if(key->DisplayType == ALPHA)
							key->SetupKeyboard(NUMERIC);
						else
							key->SetupKeyboard(ALPHA);
						break;

					case UPPER_BTN_ID:
						if(key->Caps == false)
						{
							BUTTON_SetBitmap(pMsg->hWinSrc, BUTTON_BI_UNPRESSED, &bmCapital);
							key->Caps = true;
						}
						else
						{
							BUTTON_SetBitmap(pMsg->hWinSrc, BUTTON_BI_UNPRESSED, &bmNotCapital);
							key->Caps = false;
						}
						key->ChangeKeyboardCase(key->Caps);
						break;

					case CANCEL_BTN_ID:
						msg.MsgId = KB_CANCEL_MSG;
						msg.hWin = key->hParent;
						msg.hWinSrc = key->_hDlg;
						key->Action = KB_CANCEL;
						msg.Data.v = key->Source;
						msg.Data.p = key;
						if (TouchPosIsBackKey())
							GaugeScreensIgnoreNextKeyRelease();
						WM_SendMessage(key->hParent, &msg);
						break;

					case SAVE_BTN_ID:
						SaveData(KB_SAVE_AND_EXIT_MSG, key);
						break;

					default:
						char c;
						
						if (Id < GUI_ID_USER)
							break;

						if (Id == GUI_ID_USER + DEL_BTN_ID)
							key->hAutoRepeatTimer = WM_CreateTimer(key->_hDlg, 0, AUTO_REPEAT_TIME, 0);

						c = (char)(Id - GUI_ID_USER);
						if(key->Caps == false)
							c = tolower(c);
						EDIT_AddKey(key->Text, (int)c);
						break;
					};

			}
			break;

		default:
			WM_DefaultProc(pMsg);
	};
}

void SaveData(int message, Keyboard *key)
{
	WM_MESSAGE msg;

	msg.MsgId = message;
	key->GetValue(key->OrgText, key->maxTextLen);
	msg.hWin = key->hParent;
	msg.hWinSrc = key->_hDlg;
	msg.Data.v = key->Source;
	key->Action = KB_SAVE;
	msg.Data.p = key;
	WM_SendMessage(key->hParent, &msg);
}
