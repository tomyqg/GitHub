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
File        : WMTouch.c
Purpose     : Windows manager, touch support
----------------------------------------------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "WM_Intern.h"
#include "GUI_Debug.h"

#if (GUI_WINSUPPORT)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GUI_PID_STATE _aStateLast[GUI_NUM_LAYERS];

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

WM_CRITICAL_HANDLE  WM__aCHWinModal[GUI_NUM_LAYERS];
WM_CRITICAL_HANDLE  WM__aCHWinLast[GUI_NUM_LAYERS];
#if GUI_SUPPORT_MOUSE
  WM_CRITICAL_HANDLE  WM__aCHWinMouseOver[GUI_NUM_LAYERS];
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Screen2Win
*/
static WM_HWIN _Screen2Win(GUI_PID_STATE* pState) {
  if (WM__ahCapture[WM__TOUCHED_LAYER] == 0) {
    return WM_Screen2hWin(pState->x, pState->y);
  } 
  return WM__ahCapture[WM__TOUCHED_LAYER];
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM__IsInModalArea
*/
int WM__IsInModalArea(WM_HWIN hWin) {
  if ((WM__aCHWinModal[WM__TOUCHED_LAYER].hWin == 0) || WM__IsAncestor(hWin, WM__aCHWinModal[WM__TOUCHED_LAYER].hWin) || (hWin == WM__aCHWinModal[WM__TOUCHED_LAYER].hWin)) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       WM__SendPIDMessage
*
* Purpose:
*   Sends a PID -type message to the affected window.
*   All ancestors are notified of this by sending a WM_TOUCH_CHILD
*   message.
*
*/
void WM__SendPIDMessage(WM_HWIN hWin, WM_MESSAGE* pMsg) {
  #if 1
  WM_HWIN iWin;
  WM_MESSAGE Msg;
  Msg = *pMsg;                 /* Save message as it may be modified in callback (as return value) */
  /* Send notification to all ancestors.
     We need to check if the window which has received the last message still exists,
     since it may have deleted itself and its parent as result of the message.
  */
  Msg.hWinSrc = hWin;
  Msg.MsgId   = WM_TOUCH_CHILD;
  iWin = hWin;
  while (WM_IsWindow(iWin)) {
    iWin = WM_GetParent(iWin);
    if (iWin) {
      Msg.Data.p  = pMsg;            /* Needs to be set for each window, as callback is allowed to modify it */
      WM__SendMessageIfEnabled(iWin, &Msg);    /* Send message to the ancestors */
    }
  }
  /* Send message to the affected window */
  WM__SendMessageIfEnabled(hWin, pMsg);
  #else
  WM_MESSAGE Msg;
  /* Send message to the affected window */
  Msg = *pMsg;                 /* Save message as it may be modified in callback (as return value) */
  WM__SendMessageIfEnabled(hWin, &Msg);
  /* Send notification to all ancestors.
     We need to check if the window which has received the last message still exists,
     since it may have deleted itself and its parent as result of the message.
  */
  Msg.hWinSrc = hWin;
  Msg.MsgId   = WM_TOUCH_CHILD;
  while (WM_IsWindow(hWin)) {
    hWin = WM_GetParent(hWin);
    if (hWin) {
      Msg.Data.p  = pMsg;            /* Needs to be set for each window, as callback is allowed to modify it */
      WM__SendMessageIfEnabled(hWin, &Msg);    /* Send message to the ancestors */
    }
  }
  #endif
}

/*********************************************************************
*
*       WM__SendTouchMessage
*/
void WM__SendTouchMessage(WM_HWIN hWin, WM_MESSAGE* pMsg) {
  GUI_PID_STATE* pState;
  pState     = (GUI_PID_STATE*)pMsg->Data.p;
  if (pState) {
    WM_Obj* pWin;
    pWin       = WM_H2P(hWin);
    pState->x -= pWin->Rect.x0;
    pState->y -= pWin->Rect.y0;
  }
  WM__SendPIDMessage(hWin, pMsg);
}

/*********************************************************************
*
*       WM__SetLastTouched
*/
void WM__SetLastTouched(WM_HWIN hWin) {
#if GUI_SUPPORT_MOUSE
  WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin = WM__aCHWinLast[WM__TOUCHED_LAYER].hWin = hWin;
#else
  GUI_USE_PARA(hWin);
#endif
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_HandlePID       
*
* Polls the touch screen. If something has changed,
* sends a message to the concerned window.
*
* Return value:
*   0 if nothing has been done
*   1 if touch message has been sent
*/
int WM_HandlePID(void) {
  int r = 0;
  int xPosLayer, yPosLayer;
  WM_MESSAGE Msg;
  WM_CRITICAL_HANDLE CHWin;
  GUI_PID_STATE State = {0}, StateNew;
  WM_HWIN hWin;
  
  GUI_PID_GetState(&StateNew);
  if ((StateNew.x == -1) && (StateNew.y == -1) && (_aStateLast[WM__TOUCHED_LAYER].Pressed == StateNew.Pressed)) {
    return r;
  }
  WM_LOCK();
  /*
  if ((StateNew.x >= 0) && (StateNew.y >= 0)) {
    StateNew.x += GUI_OrgX;
    StateNew.y += GUI_OrgY;
  }
  */
  if ((StateNew.x >= 0) && (StateNew.y >= 0)) {
    StateNew.x += GUI_OrgX;
    StateNew.y += GUI_OrgY;
  } else if ((StateNew.x == -1) && (StateNew.y == -1)) {
    StateNew.x = _aStateLast[WM__TOUCHED_LAYER].x;
    StateNew.y = _aStateLast[WM__TOUCHED_LAYER].y;
  }
  WM__AddCriticalHandle(&CHWin);
  #if (GUI_NUM_LAYERS > 1)
    if (StateNew.Layer < GUI_NUM_LAYERS) {
      WM__TouchedLayer = StateNew.Layer;
    }
  #endif
  GUI_GetLayerPosEx(WM__TOUCHED_LAYER, &xPosLayer, &yPosLayer);
  StateNew.x -= xPosLayer;
  StateNew.y -= yPosLayer;
  if (((_aStateLast[WM__TOUCHED_LAYER].x != StateNew.x) || 
       (_aStateLast[WM__TOUCHED_LAYER].y != StateNew.y) || 
       (_aStateLast[WM__TOUCHED_LAYER].Pressed != StateNew.Pressed)
      )) {
    #if GUI_SUPPORT_CURSOR
      GUI_CURSOR_SetPositionEx(StateNew.x, StateNew.y, StateNew.Layer);
      /*
      if ((StateNew.x >= 0) && (StateNew.y >= 0)) {
        GUI_CURSOR_SetPositionEx(StateNew.x, StateNew.y, StateNew.Layer);
      }
      */
    #endif
    CHWin.hWin = _Screen2Win(&StateNew);
    #if GUI_SUPPORT_MOUSE
    /* Send WM_MOUSEOVER_END Message */
    hWin = WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin; // Suppress warning: 'undefined behavior: the order of volatile accesses is undefined'
    if (hWin && (hWin != CHWin.hWin)) {
      if (WM__IsInModalArea(WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin)) {
        /* Do not send messages to disabled windows */
        if (WM__IsEnabled(WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin)) {
          State      = StateNew;
          Msg.MsgId  = WM_MOUSEOVER_END;
          Msg.Data.p = (void*)&State;
          WM__SendTouchMessage(WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin, &Msg);
          WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin = 0;
        }
      }
    }
    #endif
    if (WM__IsInModalArea(CHWin.hWin)) {
      #if GUI_SUPPORT_MOUSE
        WM__aCHWinMouseOver[WM__TOUCHED_LAYER].hWin = CHWin.hWin;
      #endif
      /*
       * Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released)
       */
      if ((_aStateLast[WM__TOUCHED_LAYER].Pressed != StateNew.Pressed) && CHWin.hWin) {
        GUI_PID_STATE PID_StateOld;
        WM_HWIN hWinOld, hWinLast;
        WM_PID_STATE_CHANGED_INFO Info;
        WM_Obj* pWin;
        pWin = WM_H2P(CHWin.hWin);
        Info.State     = StateNew.Pressed;
        Info.StatePrev = _aStateLast[WM__TOUCHED_LAYER].Pressed;
        Info.x         = StateNew.x - pWin->Rect.x0;
        Info.y         = StateNew.y - pWin->Rect.y0;
        Msg.Data.p = &Info;
        Msg.MsgId  = WM_PID_STATE_CHANGED;
        /* Make sure that this routine does not react a second time on the same press event.
         * This could happen if for example a dialog is executed during the WM_PID_STATE_CHANGED message processing
         */
        PID_StateOld = _aStateLast[WM__TOUCHED_LAYER];
        /* Make sure that in case of creating a new window over the current PID position
         * during the WM_PID_STATE_CHANGED message processing the old window
         * will be informed that it is no longer pressed.
         */
        _aStateLast[WM__TOUCHED_LAYER].Pressed = StateNew.Pressed;
        if (State.Pressed) {
          hWinOld            = WM__aCHWinLast[WM__TOUCHED_LAYER].hWin;
          WM__aCHWinLast[WM__TOUCHED_LAYER].hWin = CHWin.hWin;
        } else {
          hWinOld            = 0;
        }
        hWinLast = WM__aCHWinLast[WM__TOUCHED_LAYER].hWin;
        WM__SendMessageIfEnabled(CHWin.hWin, &Msg);
        if (hWinLast != WM__aCHWinLast[WM__TOUCHED_LAYER].hWin) {
          CHWin.hWin = WM__aCHWinLast[WM__TOUCHED_LAYER].hWin;
        }
        /* Restore state for further processing */
        _aStateLast[WM__TOUCHED_LAYER] = PID_StateOld;
        if (hWinOld) {
          /* hWinOld could be deleted during message processing. So check if it is a Window
           * before using it.
           */
          if (WM_IsWindow(hWinOld)) {
            WM__aCHWinLast[WM__TOUCHED_LAYER].hWin        = hWinOld;
          }
        }
      }
      /*
       * Send WM_TOUCH message(s)
       * Note that we may have to send 2 touch messages.
       */
      if (_aStateLast[WM__TOUCHED_LAYER].Pressed | StateNew.Pressed) {    /* Only if pressed or just released */
        Msg.MsgId = WM_TOUCH;
        r = 1;
        /*
         * Tell window that it is no longer pressed.
         * This happens for 2 possible reasons:
         * a) PID is released
         * b) PID is moved out
         */
        hWin = WM__aCHWinLast[WM__TOUCHED_LAYER].hWin; // Suppress warning: 'undefined behavior: the order of volatile accesses is undefined'
        if (hWin != CHWin.hWin) {
          if (WM__aCHWinLast[WM__TOUCHED_LAYER].hWin != 0) {
            if (StateNew.Pressed) {
              /* Moved out -> no longer in this window
               * Send a NULL pointer as data
               */
              Msg.Data.p = NULL;
            } else {
              /* Last window needs to know that it has been "Released"
               * Send last coordinates 
               */
              State.x       = _aStateLast[WM__TOUCHED_LAYER].x;
              State.y       = _aStateLast[WM__TOUCHED_LAYER].y;
              State.Pressed = 0;
              Msg.Data.p = (void*)&State;
            }
            GUI_DEBUG_LOG1 ("\nSending WM_Touch to LastWindow %d (out of area)", WM__aCHWinLast[WM__TOUCHED_LAYER].hWin);
            WM__SendTouchMessage(WM__aCHWinLast[WM__TOUCHED_LAYER].hWin, &Msg);
            WM__aCHWinLast[WM__TOUCHED_LAYER].hWin = 0;
          }
        }
        /* Make sure that this routine does not react a second time on the same event.
         * This could happen if for example a dialog is executed during the WM_TOUCH message processing
         */
        _aStateLast[WM__TOUCHED_LAYER].Pressed = StateNew.Pressed;
        /* Sending WM_Touch to current window */
        if (CHWin.hWin) {
          /* convert screen into window coordinates */
          State = StateNew;
          /* Remember window */
          if (State.Pressed) {
            WM__aCHWinLast[WM__TOUCHED_LAYER].hWin = CHWin.hWin;
          } else {
            /* Handle automatic capture release */
            if (WM__CaptureReleaseAuto) {
              WM_ReleaseCapture();
            }
            WM__aCHWinLast[WM__TOUCHED_LAYER].hWin = 0;
          }
          Msg.Data.p = (void*)&State;
          WM__SendTouchMessage(CHWin.hWin, &Msg);
        }
      }
      /*
       * Send WM_MOUSEOVER message
       */
      #if GUI_SUPPORT_MOUSE
      else {
        /* Send WM_MOUSEOVER Message */
        if (CHWin.hWin) {
          /* Do not send messages to disabled windows */
          if (WM__IsEnabled(CHWin.hWin)) {
            State      = StateNew;
            Msg.MsgId  = WM_MOUSEOVER;
            Msg.Data.p = (void*)&State;
            WM__SendTouchMessage(CHWin.hWin, &Msg);
          }
        }
      }
      #endif
    }
    /* Store the new state */
    _aStateLast[WM__TOUCHED_LAYER] = StateNew;
    WM_PID__SetPrevState(&StateNew, WM__TOUCHED_LAYER);
  }
  WM__RemoveCriticalHandle(&CHWin);
  WM_UNLOCK();
  return r;
}

#else
  void WM_Touch_c(void) {} /* avoid empty object files */
#endif  /* (GUI_WINSUPPORT & GUI_SUPPORT_TOUCH) */

/*************************** End of file ****************************/

