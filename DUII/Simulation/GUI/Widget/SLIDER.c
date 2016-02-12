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
File        : SLIDER.c
Purpose     : Implementation of slider widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "GUI_Private.h"
#include "SLIDER_Private.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define colors */
#ifndef   SLIDER_BKCOLOR0_DEFAULT
  #define SLIDER_BKCOLOR0_DEFAULT   0xc0c0c0
#endif

#ifndef   SLIDER_COLOR0_DEFAULT
  #define SLIDER_COLOR0_DEFAULT     0xc0c0c0
#endif

#ifndef   SLIDER_FOCUSCOLOR_DEFAULT
  #define SLIDER_FOCUSCOLOR_DEFAULT GUI_BLACK
#endif

#ifndef   SLIDER_TICKCOLOR_DEFAULT
  #define SLIDER_TICKCOLOR_DEFAULT GUI_BLACK
#endif

#ifndef   SLIDER_DRAW_SKIN_DEFAULT
  #define SLIDER_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

SLIDER_PROPS SLIDER__DefaultProps = {
  SLIDER_BKCOLOR0_DEFAULT,
  SLIDER_COLOR0_DEFAULT,
  SLIDER_FOCUSCOLOR_DEFAULT,
  SLIDER_TICKCOLOR_DEFAULT,
  {
    SLIDER_DRAW_SKIN_DEFAULT
  }
};

WIDGET_SKIN const * SLIDER__pSkinDefault = &SLIDER__SkinClassic;

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
static void _Paint(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = SLIDER_LOCK_H(hObj);
  pfPaint = pObj->pWidgetSkin->pfPaint;
  GUI_UNLOCK_H(pObj);
  if (pfPaint) {
    pfPaint(hObj);
  }
}

/*********************************************************************
*
*       _SliderPressed
*/
static void _SliderPressed(SLIDER_Handle hObj, U16 Status) {
  WIDGET_OrState(hObj, SLIDER_STATE_PRESSED);
  if (Status & WM_SF_ISVIS) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
  }
}

/*********************************************************************
*
*       _SliderReleased
*/
static void _SliderReleased(SLIDER_Handle hObj, U16 Status) {
  WIDGET_AndState(hObj, SLIDER_STATE_PRESSED);
  if (Status & WM_SF_ISVIS) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_RELEASED);
  }
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(SLIDER_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    SLIDER_Obj * pObj;
    I16 Width, State;
    U16 Status;
    int Min, Max;
    pObj = SLIDER_LOCK_H(hObj);
    Min = pObj->Min;
    Max = pObj->Max;
    Width = pObj->Width;
    State = pObj->Widget.State;
    Status = pObj->Widget.Win.Status;
    GUI_UNLOCK_H(pObj);
    if (pState->Pressed) {
      int x0, xsize, x, Sel, Range;
      Range = (Max - Min);
      x0    = 1 + Width / 2;  /* 1 pixel focus rectangle + width of actual slider */
      x     = (State & WIDGET_STATE_VERTICAL) ? pState->y : pState->x;
      x    -= x0;
      xsize = WIDGET__GetWindowSizeX(hObj) - 2 * x0;
      if (x <= 0) {
        Sel = Min;
      } else if (x >= xsize) {
        Sel = Max;
      } else {
        int Div;
        Div = xsize ? xsize : 1;     /* Make sure we do not divide by 0, even though xsize should never be 0 in this case anyhow */
        Sel = Min + ((U32)Range * (U32)x + Div / 2) / Div;
      }
      if (WM_IsFocussable(hObj)) {
        WM_SetFocus(hObj);
      }
      WM_SetCapture(hObj, 1);
      SLIDER_SetValue(hObj, Sel);
      if ((State & SLIDER_STATE_PRESSED) == 0){   
        _SliderPressed(hObj, Status);
      }
    } else {
      /* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
      if (State & SLIDER_STATE_PRESSED) {   
        _SliderReleased(hObj, Status);
      }
    }
  }
}

/*********************************************************************
*
*       _OnKey
*/
static void  _OnKey(SLIDER_Handle hObj, WM_MESSAGE * pMsg) {
  const WM_KEY_INFO * pKeyInfo;
  int Key;
  pKeyInfo = (const WM_KEY_INFO *)(pMsg->Data.p);
  Key = pKeyInfo->Key;
  if (pKeyInfo->PressedCnt > 0) {
    switch (Key) {
      case GUI_KEY_RIGHT:
        SLIDER_Inc(hObj);
        break;                    /* Send to parent by not doing anything */
      case GUI_KEY_LEFT:
        SLIDER_Dec(hObj);
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
*       SLIDER_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
SLIDER_Obj * SLIDER_LockH(SLIDER_Handle h) {
  SLIDER_Obj * p = (SLIDER_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != SLIDER_ID) {
      GUI_DEBUG_ERROROUT("SLIDER.c: Wrong handle type or Object not init'ed");
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
*       SLIDER_Callback
*/
void SLIDER_Callback (WM_MESSAGE *pMsg) {
  SLIDER_Handle hObj;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_DEBUG_LOG("SLIDER: _Callback(WM_PAINT)\n");
    _Paint(hObj);
    return;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_KEY:
    _OnKey(hObj, pMsg);
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
*       SLIDER_CreateEx
*/
SLIDER_Handle SLIDER_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                              int WinFlags, int ExFlags, int Id)
{
  SLIDER_Handle hObj;
  /* Create the window */
  WM_LOCK();
  #if SLIDER_SUPPORT_TRANSPARENCY
    WinFlags |= WM_CF_HASTRANS;
  #endif
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, SLIDER_Callback, sizeof(SLIDER_Obj) - sizeof(WM_Obj));
  if (hObj) {
    SLIDER_Obj * pObj;
    U16 InitState;
    pObj = (SLIDER_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* Handle SpecialFlags */
    InitState = WIDGET_STATE_FOCUSSABLE;
    if (ExFlags & SLIDER_CF_VERTICAL) {
      InitState |= WIDGET_CF_VERTICAL;
    }
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, InitState);
    /* init member variables */
    SLIDER_INIT_ID(pObj);
    pObj->Props = SLIDER__DefaultProps;
    pObj->pWidgetSkin = SLIDER__pSkinDefault;
    pObj->Width       = 8;
    pObj->Max         = 100;
    pObj->Min         = 0;
    pObj->NumTicks    = -1;
    GUI_UNLOCK_H(pObj);
    SLIDER__pSkinDefault->pfCreate(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "SLIDER_Create failed")
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
*       SLIDER_Dec
*/
void SLIDER_Dec(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    if (pObj->v > pObj->Min) {
      pObj->v--;
      WM_InvalidateWindow(hObj);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_Inc
*/
void SLIDER_Inc(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    if (pObj->v < pObj->Max) {
      pObj->v++;
      WM_InvalidateWindow(hObj);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_SetWidth
*/
void SLIDER_SetWidth(SLIDER_Handle hObj, int Width) {
  SLIDER_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    if (pObj->Width != Width) {
      pObj->Width = Width;
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_SetValue
*/
void SLIDER_SetValue(SLIDER_Handle hObj, int v) {
  SLIDER_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    /* Put in min/max range */
    if (v < pObj->Min) {
      v = pObj->Min;
    }
    if (v > pObj->Max) {
      v = pObj->Max;
    }
    if (pObj->v != v) {
      pObj->v = v;
      WM_InvalidateWindow(hObj);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_SetRange
*/
void SLIDER_SetRange(SLIDER_Handle hObj, int Min, int Max) {
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    if (Max < Min) {
      Max = Min;
    }
    pObj->Min = Min;
    pObj->Max = Max;
    if (pObj->v < Min) {
      pObj->v = Min;
    }
    if (pObj->v > Max) {
      pObj->v = Max;
    }
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_SetNumTicks
*/
void SLIDER_SetNumTicks(SLIDER_Handle hObj, int NumTicks) {
  if (hObj && (NumTicks >= 0)) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    pObj->NumTicks = NumTicks;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       Query state
*
**********************************************************************
*/
/*********************************************************************
*
*       SLIDER_GetValue
*/
int SLIDER_GetValue(SLIDER_Handle hObj) {
  int r = 0;
  SLIDER_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    r = pObj->v;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}


#else /* avoid empty object files */

void SLIDER_C(void);
void SLIDER_C(void){}

#endif  /* #if GUI_WINSUPPORT */



