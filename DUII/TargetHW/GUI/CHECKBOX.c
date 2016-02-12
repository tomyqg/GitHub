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
File        : CHECKBOX.c
Purpose     : Implementation of checkbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "CHECKBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default fonts */
#ifndef   CHECKBOX_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define CHECKBOX_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define CHECKBOX_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define CHECKBOX_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define default images */
#ifndef   CHECKBOX_IMAGE0_DEFAULT
  #define CHECKBOX_IMAGE0_DEFAULT &CHECKBOX__abmCheck[0]
#endif

#ifndef   CHECKBOX_IMAGE1_DEFAULT
  #define CHECKBOX_IMAGE1_DEFAULT &CHECKBOX__abmCheck[1]
#endif

/* Define widget background color */
#ifndef   CHECKBOX_BKCOLOR_DEFAULT
  #define CHECKBOX_BKCOLOR_DEFAULT 0xC0C0C0           /* Text background color */
#endif

#ifndef   CHECKBOX_SPACING_DEFAULT
  #define CHECKBOX_SPACING_DEFAULT 4
#endif

#ifndef   CHECKBOX_TEXTCOLOR_DEFAULT
  #define CHECKBOX_TEXTCOLOR_DEFAULT GUI_BLACK
#endif

#ifndef   CHECKBOX_FOCUSCOLOR_DEFAULT
  #define CHECKBOX_FOCUSCOLOR_DEFAULT GUI_BLACK
#endif

#ifndef   CHECKBOX_TEXTALIGN_DEFAULT
  #define CHECKBOX_TEXTALIGN_DEFAULT (GUI_TA_LEFT | GUI_TA_VCENTER)
#endif

#ifndef   CHECKBOX_DRAW_SKIN_DEFAULT
  #define CHECKBOX_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
CHECKBOX_PROPS CHECKBOX__DefaultProps = {
  CHECKBOX_FONT_DEFAULT,
  {
    CHECKBOX_BKCOLOR0_DEFAULT,
    CHECKBOX_BKCOLOR1_DEFAULT,
  },
  CHECKBOX_BKCOLOR_DEFAULT,
  CHECKBOX_FOCUSCOLOR_DEFAULT,
  CHECKBOX_TEXTCOLOR_DEFAULT,
  {
    CHECKBOX__GetButtonSize,
    CHECKBOX_DRAW_SKIN_DEFAULT
  },
  CHECKBOX_TEXTALIGN_DEFAULT,
  CHECKBOX_SPACING_DEFAULT,
  {
    NULL,
    NULL,
    CHECKBOX_IMAGE0_DEFAULT, 
    CHECKBOX_IMAGE1_DEFAULT
  }
};

WIDGET_SKIN const * CHECKBOX__pSkinDefault = &CHECKBOX__SkinClassic;

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
static void _Paint(CHECKBOX_Handle hObj) {
  CHECKBOX_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = CHECKBOX_LOCK_H(hObj);
  pfPaint = pObj->pWidgetSkin->pfPaint;
  GUI_UNLOCK_H(pObj);
  if (pfPaint) {
    pfPaint(hObj);
  }
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(CHECKBOX_Handle hObj, WM_MESSAGE * pMsg) {
  CHECKBOX_Obj * pObj;
  int Notification = 0;
  int Hit = 0;
  const GUI_PID_STATE * pState;
  U8 NumStates;
  U8 CurrentState;
  I16 Id;

  pObj = CHECKBOX_LOCK_H(hObj);
  NumStates    = pObj->NumStates;
  CurrentState = pObj->CurrentState;
  Id           = pObj->Widget.Id;
  GUI_UNLOCK_H(pObj);
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (!WM_HasCaptured(hObj)) {
      if (pState->Pressed) {
        WM_SetCapture(hObj, 1);
        CHECKBOX_SetState(hObj, (CurrentState + 1) % NumStates);
        Notification = WM_NOTIFICATION_CLICKED;
      } else {
        Hit =1;
        Notification = WM_NOTIFICATION_RELEASED;
      }
    }
  } else {
    Notification = WM_NOTIFICATION_MOVED_OUT;
  }
  WM_NotifyParent(hObj, Notification);
  if (Hit == 1) {
    GUI_DEBUG_LOG("CHECKBOX: Hit\n");
    GUI_StoreKey(Id);
  }
}

/*********************************************************************
*
*       _OnKey
*/
static void  _OnKey(CHECKBOX_Handle hObj, WM_MESSAGE * pMsg) {
  WM_KEY_INFO * pKeyInfo;
  CHECKBOX_Obj * pObj;
  U8 NumStates;
  U8 CurrentState;
  pObj = CHECKBOX_LOCK_H(hObj);
  NumStates    = pObj->NumStates;
  CurrentState = pObj->CurrentState;
  GUI_UNLOCK_H(pObj);
  if (WM__IsEnabled(hObj)) {
    pKeyInfo = (WM_KEY_INFO *)(pMsg->Data.p);
    if (pKeyInfo->PressedCnt > 0) {
      switch (pKeyInfo->Key) {
      case GUI_KEY_SPACE:
        CHECKBOX_SetState(hObj, (CurrentState + 1) % NumStates);
        break;                    /* Send to parent by not doing anything */
      }
    }
  }
}

/*********************************************************************
*
*       _Delete
*/
static void _Delete(CHECKBOX_Handle hObj) {
  CHECKBOX_Obj * pObj;
  pObj = CHECKBOX_LOCK_H(hObj);
  /* Delete attached objects (if any) */
  GUI_DEBUG_LOG("CHECKBOX: Delete() Deleting attached items");
  GUI_ALLOC_FreePtr(&pObj->hpText);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX__GetButtonSize
*/
unsigned CHECKBOX__GetButtonSize(void) {
  WIDGET_ITEM_DRAW_INFO ItemInfo = {0};

  ItemInfo.Cmd = WIDGET_ITEM_GET_BUTTONSIZE;
  return ((CHECKBOX_SKIN_PRIVATE *)CHECKBOX__pSkinDefault->pSkinPrivate)->pfDrawSkin(&ItemInfo);
}

/*********************************************************************
*
*       CHECKBOX_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
CHECKBOX_Obj * CHECKBOX_LockH(CHECKBOX_Handle h) {
  CHECKBOX_Obj * p = (CHECKBOX_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != CHECKBOX_ID) {
      GUI_DEBUG_ERROROUT("CHECKBOX.c: Wrong handle type or Object not init'ed");
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
*       CHECKBOX_Callback
*/
void CHECKBOX_Callback (WM_MESSAGE *pMsg) {
  CHECKBOX_Handle hObj;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_KEY:
    _OnKey(hObj, pMsg);
    break;
  case WM_PAINT:
    GUI_DEBUG_LOG("CHECKBOX: _Callback(WM_PAINT)\n");
    _Paint(hObj);
    return;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_DELETE:
    GUI_DEBUG_LOG("CHECKBOX: _Callback(WM_DELETE)\n");
    _Delete(hObj);
    break;       /* No return here ... WM_DefaultProc needs to be called */
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/

/* Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions */

/*********************************************************************
*
*       CHECKBOX_CreateEx
*/
CHECKBOX_Handle CHECKBOX_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id)
{
  CHECKBOX_Handle hObj;
  CHECKBOX_Obj  * pObj;
  GUI_USE_PARA(ExFlags);
  WM_LOCK();
  /* Calculate size if needed */
  if ((xsize == 0) || (ysize == 0)) {
    if (xsize == 0) {
      xsize  = ((CHECKBOX_SKIN_PRIVATE *)CHECKBOX__pSkinDefault->pSkinPrivate)->pfGetButtonSize();
    }
    if (ysize == 0) {
      ysize  = ((CHECKBOX_SKIN_PRIVATE *)CHECKBOX__pSkinDefault->pSkinPrivate)->pfGetButtonSize();
    }
  }
#if WM_SUPPORT_TRANSPARENCY
  if (CHECKBOX__DefaultProps.BkColor == GUI_INVALID_COLOR) {
    WinFlags |= WM_CF_HASTRANS;
  }
#endif
  /* Create the window */
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, CHECKBOX_Callback,
                                sizeof(CHECKBOX_Obj) - sizeof(WM_Obj));
  if (hObj) {
    pObj = (CHECKBOX_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    CHECKBOX_INIT_ID(pObj);
    /* init member variables */
    pObj->Props = CHECKBOX__DefaultProps;
    pObj->pWidgetSkin = CHECKBOX__pSkinDefault;
    pObj->NumStates   = 2; /* Default behaviour is 2 states: checked and unchecked */
    GUI_UNLOCK_H(pObj);
    CHECKBOX__pSkinDefault->pfCreate(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "CHECKBOX_Create failed")
  }
  WM_UNLOCK();
  return hObj;
}

#else                            /* Avoid problems with empty object modules */
  void CHECKBOX_C(void);
  void CHECKBOX_C(void) {}
#endif  /* #if GUI_WINSUPPORT */



