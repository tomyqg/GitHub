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
File        : RADIO.c
Purpose     : Implementation of radio button widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include "GUI_Private.h"
#include "RADIO_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/
#ifndef   RADIO_DRAW_SKIN_DEFAULT
  #define RADIO_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

RADIO_PROPS RADIO__DefaultProps = {
  RADIO_DEFAULT_BKCOLOR,
  RADIO_DEFAULT_TEXT_COLOR,
  RADIO_FOCUSCOLOR_DEFAULT,
  RADIO_FONT_DEFAULT,
  {
    RADIO_IMAGE0_DEFAULT,
    RADIO_IMAGE1_DEFAULT,
  },
  RADIO_IMAGE_CHECK_DEFAULT,
  {
    RADIO__GetButtonSize,
    RADIO_DRAW_SKIN_DEFAULT
  }
};

tRADIO_SetValue * RADIO__pfHandleSetValue;

WIDGET_SKIN const * RADIO__pSkinDefault = &RADIO__SkinClassic;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/

/*********************************************************************
*
*       _OnPaint
*
* Purpose:
*   Paints the RADIO button.
*   The button can actually consist of multiple buttons (NumItems).
*   The focus rectangle will be drawn on top of the text if any text is set,
*   otherwise around the entire buttons.
*/
static void _OnPaint(RADIO_Handle hObj) {
  RADIO_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = RADIO_LOCK_H(hObj);
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
static void _OnTouch(RADIO_Handle hObj, WM_MESSAGE * pMsg) {
  RADIO_Obj * pObj;
  int Notification, Spacing, Height, Id;
  int Hit = 0;
  GUI_PID_STATE * pState = (GUI_PID_STATE *)pMsg->Data.p;
  pObj = RADIO_LOCK_H(hObj);
  Spacing = pObj->Spacing;
  if (pObj->GroupId) {
    Height = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
  } else {
    Height = ((RADIO_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate)->pfGetButtonSize(hObj) + RADIO_BORDER * 2;
  }
  Id = pObj->Widget.Id;
  GUI_UNLOCK_H(pObj);
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed) {
      int y, Sel;
      y   = pState->y;
      Sel = y   / Spacing;
      y  -= Sel * Spacing;
      if (y <= Height) {
        RADIO_SetValue(hObj, Sel);
      }
      if (WM_IsFocussable(hObj)) {
        WM_SetFocus(hObj);
      }
      Notification = WM_NOTIFICATION_CLICKED;
    } else {
      Hit = 1;
      Notification = WM_NOTIFICATION_RELEASED;
    }
  } else {
    Notification = WM_NOTIFICATION_MOVED_OUT;
  }
  WM_NotifyParent(hObj, Notification);
  if (Hit == 1) {
    GUI_DEBUG_LOG("RADIO: Hit\n");
    GUI_StoreKey(Id);
  }
}

/*********************************************************************
*
*       _OnKey
*/
static void  _OnKey(RADIO_Handle hObj, WM_MESSAGE * pMsg) {
  WM_KEY_INFO * pKeyInfo;
  pKeyInfo = (WM_KEY_INFO *)(pMsg->Data.p);
  if (pKeyInfo->PressedCnt > 0) {
    switch (pKeyInfo->Key) {
    case GUI_KEY_RIGHT:
    case GUI_KEY_DOWN:
      RADIO_Inc(hObj);
      break;                    /* Send to parent by not doing anything */
    case GUI_KEY_LEFT:
    case GUI_KEY_UP:
      RADIO_Dec(hObj);
      break;                    /* Send to parent by not doing anything */
    default:
      return;
    }
  }
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
RADIO_Obj * RADIO_LockH(RADIO_Handle h) {
  RADIO_Obj * p = (RADIO_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != RADIO_ID) {
      GUI_DEBUG_ERROROUT("RADIO.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       RADIO__GetButtonSize
*/
unsigned RADIO__GetButtonSize(RADIO_Handle hObj) {
  RADIO_Obj * pObj;
  RADIO_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  
  pObj = RADIO_LOCK_H(hObj);
  pSkinPrivate = (RADIO_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  GUI_UNLOCK_H(pObj);
  ItemInfo.hWin = hObj;
  ItemInfo.Cmd = WIDGET_ITEM_GET_BUTTONSIZE;
  return pSkinPrivate->pfDrawSkin(&ItemInfo);
}

/*********************************************************************
*
*       Exported routines, modul internal
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO__SetValue
*/
void RADIO__SetValue(RADIO_Handle hObj, int v) {
  RADIO_Obj * pObj;
  pObj = RADIO_LOCK_H(hObj);
  if (v >= (int)pObj->NumItems) {
    v = pObj->NumItems - 1;
  }
  if (v != pObj->Sel) {
    pObj->Sel = v;
    WM_InvalidateWindow(hObj);
    GUI_UNLOCK_H(pObj);
    WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_Callback
*/
void RADIO_Callback (WM_MESSAGE * pMsg) {
  RADIO_Handle hObj;
  RADIO_Obj *   pObj;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_DEBUG_LOG("RADIO: _Callback(WM_PAINT)\n");
    _OnPaint(hObj);
    return;
  case WM_GET_RADIOGROUP:
    pObj = RADIO_LOCK_H(hObj);
    pMsg->Data.v = pObj->GroupId;
    GUI_UNLOCK_H(pObj);
    return;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_KEY:
    _OnKey(hObj, pMsg);
    break;
  case WM_DELETE:
    pObj = RADIO_LOCK_H(hObj);
    GUI_ARRAY_Delete(pObj->TextArray);
    GUI_UNLOCK_H(pObj);
    break;
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
*       RADIO_CreateEx
*/
RADIO_Handle RADIO_CreateEx(int x0, int y0, int xSize, int ySize, WM_HWIN hParent,
                            int WinFlags, int ExFlags, int Id, int NumItems, int Spacing)
{
  RADIO_Handle hObj;
  int Height, i;
  WM_LOCK();
  /* Calculate helper variables */
  Height   = RADIO__DefaultProps.apBmRadio[0]->YSize + RADIO_BORDER * 2;
  Spacing  = (Spacing  <= 0) ? RADIO_SPACING_DEFAULT : Spacing;
  NumItems = (NumItems <= 0) ? 2                     : NumItems;
  if (ySize == 0) {
    ySize  = Height + ((NumItems - 1) * Spacing);
  }
  if (xSize == 0) {
    xSize  = RADIO__DefaultProps.apBmRadio[0]->XSize + RADIO_BORDER * 2;
  }
#if WM_SUPPORT_TRANSPARENCY
  WinFlags |= WM_CF_HASTRANS;
#endif
  /* Create the window */
  hObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WinFlags, RADIO_Callback, sizeof(RADIO_Obj) - sizeof(WM_Obj));
  if (hObj) {
    GUI_ARRAY TextArray;
    RADIO_Obj * pObj;
    TextArray = GUI_ARRAY_Create();
    for (i = 0; i < NumItems; i++) {
      GUI_ARRAY_AddItem(TextArray, NULL, 0);
    }
    pObj = (RADIO_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* Init sub-classes */
    pObj->TextArray = TextArray;
    /* Init widget specific variables */
    ExFlags &= RADIO_TEXTPOS_LEFT;
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE | ExFlags);
    /* Init member variables */
    RADIO_INIT_ID(pObj);
    pObj->pWidgetSkin = RADIO__pSkinDefault;
    pObj->Props = RADIO__DefaultProps;
    pObj->Props.BkColor= WM_GetBkColor(hParent);
    pObj->NumItems     = NumItems;
    pObj->Spacing      = Spacing;
    GUI_UNLOCK_H(pObj);
    RADIO__pSkinDefault->pfCreate(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "RADIO_Create failed")
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
*       RADIO_AddValue
*/
void RADIO_AddValue(RADIO_Handle hObj, int Add) {
  if (hObj) {
    RADIO_Obj * pObj;
    int Sel;
    WM_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    Sel = pObj->Sel;
    GUI_UNLOCK_H(pObj);
    RADIO_SetValue(hObj, Sel + Add);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       RADIO_Dec
*/
void RADIO_Dec(RADIO_Handle hObj) {
  RADIO_AddValue(hObj, -1);
}

/*********************************************************************
*
*       RADIO_Inc
*/
void RADIO_Inc(RADIO_Handle hObj) {
  RADIO_AddValue(hObj,  1);
}

/*********************************************************************
*
*       RADIO_SetValue
*/
void RADIO_SetValue(RADIO_Handle hObj, int v) {
  if (hObj) {
    RADIO_Obj * pObj;
    U8 GroupId;
    WM_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    GroupId = pObj->GroupId;
    GUI_UNLOCK_H(pObj);
    if (GroupId && RADIO__pfHandleSetValue) {
      (*RADIO__pfHandleSetValue)(hObj, v);
    } else {
      if (v < 0) {
        v = 0;
      }
      RADIO__SetValue(hObj, v);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       Exported routines:  Query state
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_GetValue
*/
int RADIO_GetValue(RADIO_Handle hObj) {
  int r = 0;
  if (hObj) {
    RADIO_Obj * pObj;
    WM_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    r = pObj->Sel;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

#else /* avoid empty object files */

void RADIO_C(void);
void RADIO_C(void){}

#endif  /* #if GUI_WINSUPPORT */

/************************* end of file ******************************/
