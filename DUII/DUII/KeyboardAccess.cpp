/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: KeyboardAccess.cpp
Description: Access wrapper for the keyboard object.  Needed to
	access the C++ object from C
-----------------------------------------------------------------
*/

#include "keyboard.hpp"
#include "commondefs.h"
#include "KeyboardAccess.h"
#include "GeneralConfig.h"

static char iKeyboardCount = 0;

void *CreateKeyboard(char *Title, char *ExistingValue, Keyboard_Type DisplayType, char LockKeyboard, int iMaxTextLength, int iSource, WM_HWIN parent, char scrollable)
{
	if (++iKeyboardCount == MAX_KEYBOARDS)
	{
		iKeyboardCount--;
		SystemError();
		return 0;
	}
	else return new Keyboard(0, 0, XSIZE_PHYS, YSIZE_PHYS, DisplayType, ExistingValue, Title, LockKeyboard, iMaxTextLength, iSource, parent, scrollable);
}

void DestroyKeyboard(void *pKeyboard)
{
	Keyboard *board;

	board = (Keyboard *)pKeyboard;
	delete board;

	iKeyboardCount--;
}

