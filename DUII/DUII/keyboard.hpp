/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: keyboard.hpp
Description: keyboard header file
-----------------------------------------------------------------
*/
#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "DUII.h"
#include "KeyboardAccess.h"

#define BUTTON_WIDTH 35
#define BUTTON_HEIGHT BUTTON_WIDTH
#define BUTTON_NUM_WIDTH 50
#define BUTTON_NUM_HEIGHT BUTTON_NUM_WIDTH
#define BUTTON_SPACING 10
#define BUTTON_SPACE_WIDTH 227
#define BUTTON_CMD_WIDTH 116

#define BUTTON_LEFT_X 22
#define BUTTON_ROW_ONE_Y 94
#define BUTTON_ROW_TWO_Y BUTTON_ROW_ONE_Y + BUTTON_HEIGHT + BUTTON_SPACING
#define BUTTON_ROW_THREE_Y BUTTON_ROW_TWO_Y + BUTTON_HEIGHT + BUTTON_SPACING + 1

#define NUM_BTN_LEFT_X 179

#define TEXT_MAX_WIDTH 100
#define NUM_ALPHA_BTNS 35

#define DEL_BTN_ID GUI_KEY_BACKSPACE
#define NUM_BTN_ID 2
#define CANCEL_BTN_ID 3
#define SPACE_BTN_ID GUI_KEY_SPACE
#define UPPER_BTN_ID 4
#define SAVE_BTN_ID 5

#define MAX_KEYBOARDS 10

typedef enum
{
	KB_CANCEL = -1,
	KB_SAVE = 1,
} Keyboard_Return;

class Keyboard {
public:
	Keyboard();
	Keyboard(int x, int y, int iWidth, int iHeight, Keyboard_Type Option, char *Text, char *Title, char bLockMode, int iMaxTextLength, int iSource, WM_HWIN _hParent, char scrollable);
	~Keyboard();
	void GetValue(char *pStr, int max);
	void SetupKeyboard(Keyboard_Type Option);
	void ChangeKeyboardCase(bool Capital);
	void CloseKeyboard();
	// public variables
	Keyboard_Type DisplayType;
	Keyboard_Return Action;
	bool Caps;
	WM_HWIN hParent;
	WM_HWIN _hDlg;
	int Source;
	char *OrgText;  // the original value from when the keyboard is created 
	int maxTextLen;
	BUTTON_Handle AlphaBtns[NUM_ALPHA_BTNS], AlphaNumBtn;
	EDIT_Handle Text;
	GUI_PID_STATE StartPos;
	bool touchDetected;
	long hAutoRepeatTimer;
private:
	void ProcessKey(int KeyId);
	void SetBtnStd(BUTTON_Handle btn);
	int xLoc, yLoc;
	int Width, Height;
	char Title[50];
	char LockMode;  // lock the keyboard into the mode it's created with
	char scrollable;	// Keyboard can be scrolled
};



#endif