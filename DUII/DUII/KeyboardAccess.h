/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: KeyboardAccess.h
Description: Access wrapper for the keyboard object.  Needed to
	access the C++ object from C
-----------------------------------------------------------------
*/


#ifndef KEYBOARDACCESS_HPP
#define KEYBOARDACCESS_HPP

#define KB_SAVE_MSG				WM_NOTIFICATION_USER + 10
#define KB_CANCEL_MSG			WM_NOTIFICATION_USER + 11
#define KB_SAVE_AND_EXIT_MSG	WM_NOTIFICATION_USER + 12


typedef enum 
{
	ALPHA = 1,
	NUMERIC,
} Keyboard_Type;

#ifdef __cplusplus
extern "C" {
#endif

void *CreateKeyboard(char *Title, char *ExistingValue, Keyboard_Type DisplayType, char LockKeyboard, int iMaxTextLength, int iSource, WM_HWIN parent, char scrollable);
void DestroyKeyboard(void *pKeyboard);

#ifdef __cplusplus
}
#endif


#endif