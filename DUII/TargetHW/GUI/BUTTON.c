/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2010  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.06 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : BUTTON.c
Purpose     : Implementation of button widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "BUTTON_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default fonts */
#ifndef   BUTTON_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define BUTTON_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define BUTTON_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define BUTTON_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define colors */
#ifndef   BUTTON_BKCOLOR0_DEFAULT
  #define BUTTON_BKCOLOR0_DEFAULT   0xAAAAAA
#endif

#ifndef   BUTTON_BKCOLOR1_DEFAULT
  #define BUTTON_BKCOLOR1_DEFAULT   GUI_WHITE
#endif

#ifndef   BUTTON_BKCOLOR2_DEFAULT
  #define BUTTON_BKCOLOR2_DEFAULT   GUI_LIGHTGRAY
#endif

#ifndef   BUTTON_TEXTCOLOR0_DEFAULT
  #define BUTTON_TEXTCOLOR0_DEFAULT GUI_BLACK
#endif

#ifndef   BUTTON_TEXTCOLOR1_DEFAULT
  #define BUTTON_TEXTCOLOR1_DEFAULT GUI_BLACK
#endif

#ifndef   BUTTON_TEXTCOLOR2_DEFAULT
  #define BUTTON_TEXTCOLOR2_DEFAULT GUI_DARKGRAY
#endif

#ifndef   BUTTON_FOCUSCOLOR_DEFAULT
  #define BUTTON_FOCUSCOLOR_DEFAULT GUI_BLACK
#endif

#ifndef   BUTTON_FRAMECOLOR_DEFAULT
  #define BUTTON_FRAMECOLOR_DEFAULT GUI_BLACK
#endif

#ifndef   BUTTON_REACT_ON_LEVEL
  #define BUTTON_REACT_ON_LEVEL 0
#endif

#ifndef   BUTTON_ALIGN_DEFAULT
  #define BUTTON_ALIGN_DEFAULT GUI_TA_HCENTER | GUI_TA_VCENTER
#endif

#ifndef   BUTTON_DRAW_SKIN_DEFAULT
  #define BUTTON_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
BUTTON_PROPS BUTTON__DefaultProps = {
  {
    BUTTON_BKCOLOR0_DEFAULT,
    BUTTON_BKCOLOR1_DEFAULT,
    BUTTON_BKCOLOR2_DEFAULT,
  },
  {
    BUTTON_TEXTCOLOR0_DEFAULT,
    BUTTON_TEXTCOLOR1_DEFAULT,
    BUTTON_TEXTCOLOR2_DEFAULT,
  },
  BUTTON_FOCUSCOLOR_DEFAULT,
  BUTTON_FRAMECOLOR_DEFAULT,
  BUTTON_FONT_DEFAULT,
  {
    BUTTON_DRAW_SKIN_DEFAULT
  },
  BUTTON_ALIGN_DEFAULT
};

WIDGET_SKIN const * BUTTON__pSkinDefault = &BUTTON__SkinClassic;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _Paint
*/
static void _Paint(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = BUTTON_LOCK_H(hObj);
  pfPaint = pObj->pWidgetSkin->pfPaint;
  GUI_UNLOCK_H(pObj);
  if (pfPaint) {
    pfPaint(hObj);
  }
}

/*********************************************************************
*
*       _Delete
*
* Delete attached objects (if any)
*/
static void _Delete(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;
  pObj = BUTTON_LOCK_H(hObj);
  GUI_ALLOC_FreePtr(&pObj->hpText);
  GUI_ALLOC_FreePtrArray(pObj->ahDrawObj, GUI_COUNTOF(pObj->ahDrawObj));
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnButtonPressed
*/
static void _OnButtonPressed(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;
  U16 Status;
  WIDGET_OrState(hObj, BUTTON_STATE_PRESSED);
  pObj = BUTTON_LOCK_H(hObj);
  Status = pObj->Widget.Win.Status;
  GUI_UNLOCK_H(pObj);
  if (Status & WM_SF_ISVIS) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
  }
}

/*********************************************************************
*
*       _OnButtonReleased
*/
static void _OnButtonReleased(BUTTON_Handle hObj, int Notification) {
  BUTTON_Obj * pObj;
  U16 Status, Id;
  WIDGET_AndState(hObj, BUTTON_STATE_PRESSED);
  pObj = BUTTON_LOCK_H(hObj);
  Status = pObj->Widget.Win.Status;
  Id     = pObj->Widget.Id;
  GUI_UNLOCK_H(pObj);
  if (Notification == WM_NOTIFICATION_RELEASED) {
    GUI_DEBUG_LOG("BUTTON: Hit\n");
    GUI_StoreKey(Id);
  }
  if (Status & WM_SF_ISVIS) {
    WM_NotifyParent(hObj, Notification);
  }
}

/*********************************************************************
*
*       _OnTouchDefault
*/
static void _OnTouchDefault(BUTTON_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState;
  BUTTON_Obj * pObj;
  U16 State;

  pObj = BUTTON_LOCK_H(hObj);
  State = pObj->Widget.State;
  GUI_UNLOCK_H(pObj);
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed) {
      if ((State & BUTTON_STATE_PRESSED) == 0){   
        _OnButtonPressed(hObj);
      }
    } else {
      /* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
      if (State & BUTTON_STATE_PRESSED) {   
        _OnButtonReleased(hObj, WM_NOTIFICATION_RELEASED);
      }
    }
  } else {
    _OnButtonReleased(hObj, WM_NOTIFICATION_MOVED_OUT);
  }
}

/*********************************************************************
*
*       _OnTouchReactOnLevel
*/
static void _OnTouchReactOnLevel(BUTTON_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState;
  BUTTON_Obj * pObj;
  U16 State;

  pObj = BUTTON_LOCK_H(hObj);
  State = pObj->Widget.State;
  GUI_UNLOCK_H(pObj);
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (!pMsg->Data.p) {  /* Mouse moved out */
    _OnButtonReleased(hObj, WM_NOTIFICATION_MOVED_OUT);
  } else if ((!pState->Pressed) && (State & BUTTON_STATE_PRESSED)) {
    _OnButtonReleased(hObj, WM_NOTIFICATION_MOVED_OUT);
  }
}

/*********************************************************************
*
*       _OnPidStateChange
*/
static void _OnPidStateChange(BUTTON_Handle hObj, WM_MESSAGE * pMsg) {
  const WM_PID_STATE_CHANGED_INFO * pState;
  BUTTON_Obj * pObj;
  U16 State;
  pObj = BUTTON_LOCK_H(hObj);
  State = pObj->Widget.State;
  GUI_UNLOCK_H(pObj);
  pState = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
  if ((pState->StatePrev == 0) && pState->State) {
    if ((State & BUTTON_STATE_PRESSED) == 0){   
      _OnButtonPressed(hObj);
    }
  } else if ((pState->StatePrev == 1) && (pState->State == 0)) {
    if (State & BUTTON_STATE_PRESSED) {   
      _OnButtonReleased(hObj, WM_NOTIFICATION_RELEASED);
    }
  }
}

/*********************************************************************
*
*       Function pointers for BUTTON_REACT_ON_LEVEL
*/
#if (BUTTON_REACT_ON_LEVEL == 1)
  static void (* _pfOnTouch)         (BUTTON_Handle hObj, WM_MESSAGE * pMsg) = _OnTouchReactOnLevel;
  static void (* _pfOnPidStateChange)(BUTTON_Handle hObj, WM_MESSAGE * pMsg) = _OnPidStateChange;
#else
  static void (* _pfOnTouch)         (BUTTON_Handle hObj, WM_MESSAGE * pMsg) = _OnTouchDefault;
  static void (* _pfOnPidStateChange)(BUTTON_Handle hObj, WM_MESSAGE * pMsg);
#endif

/*********************************************************************
*
*       BUTTON_SetReactOnLevel
*/
void BUTTON_SetReactOnLevel(void) {
  _pfOnTouch          = _OnTouchReactOnLevel;
  _pfOnPidStateChange = _OnPidStateChange;
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
BUTTON_Obj * BUTTON_LockH(BUTTON_Handle h) {
  BUTTON_Obj * p = (BUTTON_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != BUTTON_ID) {
      GUI_DEBUG_ERROROUT("BUTTON.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_Callback
*/
void BUTTON_Callback(WM_MESSAGE *pMsg) {
  int PressedCnt, Key;
  BUTTON_Handle hObj;
  
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_PID_STATE_CHANGED:
    if (_pfOnPidStateChange) {
      _pfOnPidStateChange(hObj, pMsg);
    }
    return;
  case WM_TOUCH:
    _pfOnTouch(hObj, pMsg);
    return;      /* Message handled. Do not call WM_DefaultProc, because the window may have been destroyed */
  case WM_PAINT:
    GUI_DEBUG_LOG("BUTTON: _BUTTON_Callback(WM_PAINT)\n");
    _Paint(hObj);
    return;
  case WM_DELETE:
    GUI_DEBUG_LOG("BUTTON: _BUTTON_Callback(WM_DELETE)\n");
    _Delete(hObj);
    break;       /* No return here ... WM_DefaultProc needs to be called */
  case WM_KEY:
    PressedCnt = ((WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt;
    Key        = ((WM_KEY_INFO*)(pMsg->Data.p))->Key;
    switch (Key) {
    case GUI_KEY_ENTER:
      if (PressedCnt > 0) {
        _OnButtonPressed(hObj);
        _OnButtonReleased(hObj, WM_NOTIFICATION_RELEASED);
        return;
      }
      break;
    case GUI_KEY_SPACE:
      if (PressedCnt > 0) {
        _OnButtonPressed(hObj);
      } else {
        _OnButtonReleased(hObj, WM_NOTIFICATION_RELEASED);
      }
      return;
    }
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_CreateEx
*/
BUTTON_Handle BUTTON_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id) {
  BUTTON_Handle hObj;
  BUTTON_Obj * pObj;

  GUI_USE_PARA(ExFlags);
  /* Create the window */
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, BUTTON_Callback,
                                sizeof(BUTTON_Obj) - sizeof(WM_Obj));
  if (hObj) {
    pObj = (BUTTON_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    /* init member variables */
    BUTTON_INIT_ID(pObj);
    pObj->Props = BUTTON__DefaultProps;
    pObj->pWidgetSkin = BUTTON__pSkinDefault;
    GUI_UNLOCK_H(pObj);
    BUTTON__pSkinDefault->pfCreate(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "BUTTON_Create failed")
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/

/*********************************************************************
*
*       BUTTON_SetText
*/
void BUTTON_SetText(BUTTON_Handle hObj, const char* s) {
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_HMEM hpText;
    WM_HMEM hpTextOld;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    hpText = hpTextOld = pObj->hpText;
    GUI_UNLOCK_H(pObj);
    if (GUI__SetText(&hpText, s)) {
      if (hpText != hpTextOld) {
        pObj = BUTTON_LOCK_H(hObj);
        pObj->hpText = hpText;
        GUI_UNLOCK_H(pObj);
      }
      BUTTON_Invalidate(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       BUTTON_SetFont
*/
void BUTTON_SetFont(BUTTON_Handle hObj, const GUI_FONT GUI_UNI_PTR * pfont) {
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    pObj->Props.pFont = pfont;
    GUI_UNLOCK_H(pObj);
    BUTTON_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       BUTTON_SetState
*/
void BUTTON_SetState(BUTTON_Handle hObj, int State) {
  WIDGET_SetState(hObj, State);
}

/*********************************************************************
*
*       BUTTON_SetPressed
*/
void BUTTON_SetPressed(BUTTON_Handle hObj, int State) {
  if (State) {
    WIDGET_OrState(hObj, BUTTON_STATE_PRESSED);
  } else {
    WIDGET_AndState(hObj, BUTTON_STATE_PRESSED);
  }
}

/*********************************************************************
*
*       BUTTON_SetFocussable
*/
void BUTTON_SetFocussable(BUTTON_Handle hObj, int State) {
  if (State) {
    WIDGET_OrState(hObj, WIDGET_STATE_FOCUSSABLE);
  } else {
    WIDGET_AndState(hObj, WIDGET_STATE_FOCUSSABLE);
  }
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_C(void) {}
#endif /* GUI_WINSUPPORT */




