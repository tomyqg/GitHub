/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2004  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_Dialog.c
Purpose     : Automotive samples
----------------------------------------------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI.h"
#include "GUIDEMO.h"
#include "LCD_ConfDefaults.h"

#if GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT

#include "WM.h"
#include "DIALOG.h"
#include "LISTBOX.h"
#include "EDIT.h"
#include "SLIDER.h"
#include "FRAMEWIN.h"

/*********************************************************************
*
*       types
*
**********************************************************************
*/

typedef struct {
  unsigned char Id;
  int Time;
} KEYSIM;

/*********************************************************************
*
*       static data
*
**********************************************************************
*/

static const GUI_WIDGET_CREATE_INFO aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Dialog", 0,               10,   10, 162, 214, FRAMEWIN_CF_MOVEABLE},
  { BUTTON_CreateIndirect,   "OK",     GUI_ID_OK,       90,    5,  60,  20 },
  { BUTTON_CreateIndirect,   "Cancel", GUI_ID_CANCEL,   90,   30,  60,  20 },
  { TEXT_CreateIndirect,     "LText",  0,                5,   59,  43,  15, TEXT_CF_LEFT  },
  { TEXT_CreateIndirect,     "RText",  0,                5,   79,  43,  15, TEXT_CF_RIGHT },
  { TEXT_CreateIndirect,     "Hex",    0,                5,   99,  48,  15, TEXT_CF_LEFT  },
  { TEXT_CreateIndirect,     "Bin",    0,                5,  119,  48,  15, TEXT_CF_LEFT  },
  { EDIT_CreateIndirect,     "",       GUI_ID_EDIT0,    50,   59, 100,  15, EDIT_CF_LEFT,  50 },
  { EDIT_CreateIndirect,     "",       GUI_ID_EDIT1,    50,   79, 100,  15, EDIT_CF_RIGHT, 50 },
  { EDIT_CreateIndirect,     "",       GUI_ID_EDIT2,    50,   99, 100,  15, EDIT_CF_RIGHT, 6  },
  { EDIT_CreateIndirect,     "",       GUI_ID_EDIT3,    50,  119, 100,  15, EDIT_CF_LEFT,  0  },
  { LISTBOX_CreateIndirect,  "",       GUI_ID_LISTBOX0,  5,    5,  75,  45, 0, 0 },
  { SLIDER_CreateIndirect,   NULL,     GUI_ID_SLIDER1,   5,  140, 100,  20, 0, 0 },
  { SLIDER_CreateIndirect,   NULL,     GUI_ID_SLIDER0,   5,  165, 145,  26, 0, 0 }
};

static const KEYSIM _aID[] = {
  { GUI_KEY_TAB,  400 },
  { GUI_KEY_TAB,  400 },
  { GUI_KEY_TAB,  400 },
  { GUI_KEY_TAB,  400 },
  { GUI_KEY_TAB,  400 },
  { GUI_KEY_TAB,  400 },
  { GUI_KEY_DOWN, 250 },
  { GUI_KEY_DOWN, 250 },
  { GUI_KEY_DOWN, 250 },
  { GUI_KEY_DOWN, 250 },
  { GUI_KEY_UP,   250 },
  { GUI_KEY_UP,   250 }
};

static const GUI_ConstString _apListBox[] = {
  "English", "Deutsch", "Français", "Japanese", "Italiano", NULL
};

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbCallback
*/
static void _cbCallback(WM_MESSAGE * pMsg) {
  WM_HWIN hWin = pMsg->hWin;
  int NCode, Id;
  switch (pMsg->MsgId) {
    case WM_INIT_DIALOG:
      /* Initialize all widgets */
      EDIT_SetText(WM_GetDialogItem(hWin, GUI_ID_EDIT0), "EDIT widget 0");
      EDIT_SetText(WM_GetDialogItem(hWin, GUI_ID_EDIT1), "EDIT widget 1");
      {
        WM_HWIN hListBox;
        EDIT_SetHexMode(WM_GetDialogItem(hWin, GUI_ID_EDIT2), 0x1234, 0, 0xffffff);
        EDIT_SetBinMode(WM_GetDialogItem(hWin, GUI_ID_EDIT3), 0x1234, 0, 0xffff);
        hListBox = WM_GetDialogItem(hWin, GUI_ID_LISTBOX0);
        LISTBOX_SetText(hListBox, _apListBox);
        SCROLLBAR_CreateAttached(hListBox, SCROLLBAR_CF_VERTICAL);
      }
      break;
    case WM_KEY:
      switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) {
      case GUI_KEY_ESCAPE:
        GUI_EndDialog(hWin, 1);
        break;
      case GUI_KEY_ENTER:
        GUI_EndDialog(hWin, 0);
        break;
      }
      break;
    case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);    /* Id of widget */
      NCode = pMsg->Data.v;               /* Notification code */
      switch (NCode) {
        case WM_NOTIFICATION_RELEASED:    /* React only if released */
          if (Id == GUI_ID_OK) {          /* OK Button */
            GUI_EndDialog(hWin, 0);
          }
          if (Id == GUI_ID_CANCEL) {      /* Cancel Button */
            GUI_EndDialog(hWin, 1);
          }
          break;
        case WM_NOTIFICATION_SEL_CHANGED: /* Selection changed */
          FRAMEWIN_SetText(hWin, "Dialog - sel changed");
          break;
        default:
          FRAMEWIN_SetText(hWin, "Dialog - notification received");
      }
      break;
    default:
      WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_Dialog
*/
void GUIDEMO_Dialog(void) {
  int i;
  WM_HWIN hWin;
  GUI_COLOR DesktopColorOld;
  GUIDEMO_ShowIntro("Dialog boxes", "\nDialog boxes\ncan easily be created");
  GUIDEMO_HideInfoWin();
  DesktopColorOld = GUIDEMO_SetBkColor(GUI_RED);
  DesktopColorOld = WM_SetDesktopColor(DesktopColorOld);  /* Automatically update desktop window */
  hWin = GUI_CreateDialogBox(aDialogCreate, GUI_COUNTOF(aDialogCreate), _cbCallback, 0, 0, 0);
  for (i = 0; i < GUI_COUNTOF(_aID); i++) {
    GUIDEMO_Delay(_aID[i].Time);
    GUI_SendKeyMsg(_aID[i].Id, 1);
  }
  GUIDEMO_Delay(1000);
  for (i = 0; i <= 100; i += 10) {
    if (WM_IsWindow(hWin)) {
      SLIDER_SetValue(WM_GetDialogItem(hWin, GUI_ID_SLIDER0), i);
    }
    GUIDEMO_Delay(100);
  }
  GUIDEMO_Delay(800);
  if (WM_IsWindow(hWin)) {
    WM_MoveTo(hWin, 20, 20);
    GUIDEMO_Delay(500);
  }
  if (WM_IsWindow(hWin)) {
    WM_MoveTo(hWin, 60, 20);
    GUIDEMO_Delay(800);
  }
  /* Cleanup ... */
  if (WM_IsWindow(hWin)) {
    WM_DeleteWindow(hWin);
  }
  GUIDEMO_NotifyStartNext();
  WM_SetDesktopColor(DesktopColorOld);   /* Restore old settings */
}

#else

void GUIDEMO_Dialog(void) {}

#endif /* GUI_SUPPORT_MEMDEV */
