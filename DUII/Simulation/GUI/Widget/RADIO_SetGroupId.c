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
File        : RADIO_SetGroupId.c
Purpose     : Implementation of RADIO widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include "RADIO.h"
#include "RADIO_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _IsInGroup
*/
static int _IsInGroup(WM_HWIN hWin, U8 GroupId) {
  if (GroupId) {
    WM_MESSAGE Msg;
    Msg.MsgId = WM_GET_RADIOGROUP;
    WM__SendMessage(hWin, &Msg);
    return (Msg.Data.v == GroupId);
  }
  return 0;
}

/*********************************************************************
*
*       _GetPrevInGroup
*/
static WM_HWIN _GetPrevInGroup(WM_HWIN hWin, U8 GroupId) {
  for (hWin = WM__GetPrevSibling(hWin); hWin; hWin = WM__GetPrevSibling(hWin)) {
    if (_IsInGroup(hWin, GroupId)) {
      if (WM_IsEnabled(hWin)) {
        return hWin;
      }
    }
  }
  return 0;
}

/*********************************************************************
*
*       _GetNextInGroup
*/
static WM_HWIN _GetNextInGroup(WM_HWIN hWin, U8 GroupId) {
  for (; hWin; hWin = WM_GetNextSibling(hWin)) {
    if (_IsInGroup(hWin, GroupId)) {
      if (WM_IsEnabled(hWin)) {
        return hWin;
      }
    }
  }
  return 0;
}

/*********************************************************************
*
*       _ClearSelection
*/
static void _ClearSelection(RADIO_Handle hObj, U8 GroupId) {
  WM_HWIN hWin;
  WM_Obj * pWin;
  for (hWin = WM__GetFirstSibling(hObj); hWin;) {
    if (hWin != hObj) {
      if (_IsInGroup(hWin, GroupId)) {
        RADIO__SetValue(hWin, -1);
      }
    }
    pWin = (WM_Obj *)GUI_LOCK_H(hWin);
    hWin = pWin->hNext;
    GUI_UNLOCK_H(pWin);
  }
}

/*********************************************************************
*
*       _HandleSetValue
*/
static void _HandleSetValue(RADIO_Handle hObj, int v) {
  RADIO_Obj * pObj;
  WM_HWIN hNext;
  U16 NumItems;
  U8 GroupId;
  I16 Sel;
  pObj = RADIO_LOCK_H(hObj);
  hNext    = pObj->Widget.Win.hNext;
  GroupId  = pObj->GroupId;
  Sel      = pObj->Sel;
  NumItems = pObj->NumItems;
  GUI_UNLOCK_H(pObj);
  if (v < 0) {
    WM_HWIN hWin = _GetPrevInGroup(hObj, GroupId);
    if (hWin) {
      WM_SetFocus(hWin);
      RADIO__SetValue(hWin, 0x7FFF);
      RADIO__SetValue(hObj, -1);
    }
  } else if (v >= NumItems) {
    WM_HWIN hWin = _GetNextInGroup(hNext, GroupId);
    if (hWin) {
      WM_SetFocus(hWin);
      RADIO__SetValue(hWin, 0);
      RADIO__SetValue(hObj, -1);
    }
  } else {
    if (Sel != v) {
      _ClearSelection(hObj, GroupId);
      RADIO__SetValue(hObj, v);
    }
  }
}

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_SetGroupId
*/
void RADIO_SetGroupId(RADIO_Handle hObj, U8 NewGroupId) {
  if (hObj) {
    RADIO_Obj* pObj;
    U8 OldGroupId;
    WM_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    OldGroupId = pObj->GroupId;
    if (NewGroupId != OldGroupId) {
      WM_HWIN hFirst;
      hFirst = WM__GetFirstSibling(hObj);
      /* Set function pointer if necessary */
      if (NewGroupId && (RADIO__pfHandleSetValue == NULL)) {
        RADIO__pfHandleSetValue = _HandleSetValue;
      }
      /* Pass our selection, if we have one, to another radio button in */
      /* our old group. So the group have a valid selection when we leave it. */
      if (OldGroupId && (pObj->Sel >= 0)) {
        WM_HWIN hWin;
        pObj->GroupId = 0; /* Leave group first, so _GetNextInGroup() could */
                           /* not find a handle to our own window. */
        hWin = _GetNextInGroup(hFirst, OldGroupId);
        GUI_UNLOCK_H(pObj);
        if (hWin) {
          RADIO__SetValue(hWin, 0);
        }
      } else {
        GUI_UNLOCK_H(pObj);
      }
      /* Make sure we have a valid selection according to our new group */
      pObj = RADIO_LOCK_H(hObj);
      if (_GetNextInGroup(hFirst, NewGroupId) != 0) {
        /* Join an existing group with an already valid selection, so clear our own one */
        GUI_UNLOCK_H(pObj);
        RADIO__SetValue(hObj, -1);
        pObj = RADIO_LOCK_H(hObj);
      } else if (pObj->Sel < 0) {
        /* We are the first window in group, so we must have a valid selection at our own. */
        GUI_UNLOCK_H(pObj);
        RADIO__SetValue(hObj, 0);
        pObj = RADIO_LOCK_H(hObj);
      }
      /* Change the group */
      pObj->GroupId = NewGroupId;
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void RADIO_SetGroupId_C(void);
  void RADIO_SetGroupId_C(void) {}
#endif

/************************* end of file ******************************/
