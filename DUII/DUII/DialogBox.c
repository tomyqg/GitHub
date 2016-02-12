/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: DialogBox.c
Description: Display a dialog box with the specified text
-----------------------------------------------------------------
*/

#include "GUI.h"
#include "DUII.h"
#include "BUTTON.h"

WM_HWIN _hDlg, _hPrevWin;
const char *Title, *Text;

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect, "", 0, 90, 36, 300, 200, FRAMEWIN_CF_MOVEABLE, 0 },
	{ TEXT_CreateIndirect, "text", GUI_ID_TEXT1, 10, 10, 280, 110, 0},
	{ BUTTON_CreateIndirect, "YES", GUI_ID_BUTTON1, 50, 120, 75, 50 },
	{ BUTTON_CreateIndirect, "NO", GUI_ID_BUTTON2, 170, 120, 75, 50 },
};

static void _cbCallback(WM_MESSAGE * pMsg) {
	int NCode, Id;
	WM_HWIN hTxt1;

	switch(pMsg->MsgId)
	{
		case WM_INIT_DIALOG:
			hTxt1 = WM_GetDialogItem(_hDlg, GUI_ID_TEXT1);
			TEXT_SetTextAlign(hTxt1, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
			TEXT_SetWrapMode(hTxt1, GUI_WRAPMODE_WORD);
			TEXT_SetText(hTxt1, Text);

			FRAMEWIN_SetText(_hDlg, Title);

			break;
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
			NCode = pMsg->Data.v;                 // Notification code
			switch(NCode)
			{
				case WM_NOTIFICATION_RELEASED:	// Note: must be 'released' not 'clicked' so we don't "move" the parent
												// when this window goes away and the parent finds the pointer pressed
												// in a new location
			  // button clicked
			  switch (Id) {
				case GUI_ID_BUTTON2:
						GUI_EndDialog(_hDlg, NO);
					break;
				case GUI_ID_BUTTON1:
						GUI_EndDialog(_hDlg, YES);
					break;
			  };

			};
			break;
	 	default:
		  WM_DefaultProc(pMsg);
};
}

int ShowDialog(const char *sTitle, const char *sText, int x_org, int y_org)
{
	WM_HWIN hTxt1;
	//Title = sTitle;
	//Text = sText;

	_hPrevWin = WM_GetFocussedWindow();
	//_hDlg = GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
	_hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, x_org, y_org);

	FRAMEWIN_SetText(_hDlg, sTitle);

	hTxt1 = TEXT_CreateEx(10, 10, 280, 110, _hDlg, WM_CF_SHOW, 0, GUI_ID_TEXT1, sText);
	TEXT_SetTextAlign(hTxt1, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
	TEXT_SetWrapMode(hTxt1, GUI_WRAPMODE_WORD);

	return GUI_ExecCreatedDialog(_hDlg);
}