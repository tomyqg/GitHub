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
File        : LISTWHEEL.c
Purpose     : Implementation of listwheel widget
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_ARRAY.h"
#include "LISTWHEEL_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/
/* Define default fonts */
#ifndef LISTWHEEL_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define LISTWHEEL_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define LISTWHEEL_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define LISTWHEEL_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define colors */
#ifndef   LISTWHEEL_BKCOLOR0_DEFAULT
  #define LISTWHEEL_BKCOLOR0_DEFAULT GUI_WHITE      /* Not selected */
#endif
#ifndef   LISTWHEEL_BKCOLOR1_DEFAULT
  #define LISTWHEEL_BKCOLOR1_DEFAULT GUI_WHITE      /* Selected */
#endif
#ifndef   LISTWHEEL_TEXTCOLOR0_DEFAULT
  #define LISTWHEEL_TEXTCOLOR0_DEFAULT GUI_BLACK    /* Not selected */
#endif
#ifndef   LISTWHEEL_TEXTCOLOR1_DEFAULT
  #define LISTWHEEL_TEXTCOLOR1_DEFAULT GUI_BLUE     /* Selected */
#endif
#ifndef   LISTWHEEL_TEXTALIGN_DEFAULT
  #define LISTWHEEL_TEXTALIGN_DEFAULT GUI_TA_LEFT   /* Left aligned */
#endif

#define TIMER_PERIOD 25

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
LISTWHEEL_PROPS LISTWHEEL_DefaultProps = {
  LISTWHEEL_FONT_DEFAULT,
  {
    LISTWHEEL_BKCOLOR0_DEFAULT,
    LISTWHEEL_BKCOLOR1_DEFAULT,
  },
  {
    LISTWHEEL_TEXTCOLOR0_DEFAULT,
    LISTWHEEL_TEXTCOLOR1_DEFAULT,
  },
  LISTWHEEL_TEXTALIGN_DEFAULT
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _RecalcDatasizeY
*
* Purpose:
*   Recalculates the data size in Y direction
*/
static void _RecalcDatasizeY(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  int NumItems, ySizeData = 0, ySizeItem;
  WIDGET_ITEM_DRAW_INFO ItemInfo = {0};
  LISTWHEEL_ITEM * pItem;
  GUI_ARRAY ItemArray;

  pObj = LISTWHEEL_LOCK_H(hObj); {
    pfOwnerDraw = pObj->pfOwnerDraw;
    NumItems    = GUI_ARRAY_GetNumItems(pObj->ItemArray);
    ItemArray   = pObj->ItemArray;
  } GUI_UNLOCK_H(pObj);
  ItemInfo.Cmd  = WIDGET_ITEM_GET_YSIZE;
  ItemInfo.hWin = hObj;
  while (NumItems--) {
    pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, NumItems); {
      ItemInfo.p = pItem->pData;
      //
      // If there is no custom data use ItemInfo.p for passing the item handle
      //
      if (ItemInfo.p == NULL) {
        ItemInfo.p = (void *)GUI_ARRAY_GethItem(ItemArray, NumItems);
      }
    } GUI_UNLOCK_H(pItem);
    //
    // Get size of item
    //
    ySizeItem = pfOwnerDraw(&ItemInfo);
    ySizeData += ySizeItem;
  }
  //
  // Store data size
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    pObj->ySizeData = ySizeData;
  } GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnPaint
*
* Purpose:
*   Draws the widget
*/
static void _OnPaint(LISTWHEEL_Handle hObj) {
  const GUI_FONT GUI_UNI_PTR * pFont;
  int i, j, NumItems, xSizeItem, ySizeItem, xPos = 0, yPos = 0, Pos, ySizeData;
  
  LISTWHEEL_OBJ * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  LISTWHEEL_ITEM * pItem;
  WIDGET_ITEM_DRAW_INFO ItemInfo = {0};
  GUI_ARRAY ItemArray;
  GUI_RECT RectInside;

  //
  // Get widget properties
  //
  WM_GetInsideRectEx(hObj, &RectInside);
  pObj = LISTWHEEL_LOCK_H(hObj); {
    pfOwnerDraw = pObj->pfOwnerDraw;
    NumItems    = GUI_ARRAY_GetNumItems(pObj->ItemArray);
    ItemArray   = pObj->ItemArray;
    Pos         = pObj->Pos;
    pFont       = pObj->Props.pFont;
    ySizeData   = pObj->ySizeData;
  } GUI_UNLOCK_H(pObj);
  if (ySizeData == 0) {
    return; // Nothing to do
  }
  GUI_SetFont(pFont);
  if (pfOwnerDraw) {
    ItemInfo.hWin = hObj;
    //
    // Draw background
    //
    ItemInfo.Cmd       = WIDGET_ITEM_DRAW_BACKGROUND;
    ItemInfo.hWin      = hObj;
    ItemInfo.x0        = 0;
    ItemInfo.y0        = 0;
    ItemInfo.x1        = RectInside.x1;
    ItemInfo.y1        = RectInside.y1;
    pfOwnerDraw(&ItemInfo);
    for (j = 0; j < 2; j++) {
      xPos = 0;
      yPos = Pos;
      i = j ? NumItems - 1 : 0;
      while ((j == 0) || ((j > 0) && (Pos > 0))) {
        //
        // Get data pointer of item
        //
        pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, i); {
          ItemInfo.p = pItem->pData;
          //
          // If there is no custom data use ItemInfo.p for passing the item handle
          //
          if (ItemInfo.p == NULL) {
            ItemInfo.p = (void *)GUI_ARRAY_GethItem(ItemArray, i);
          }
        } GUI_UNLOCK_H(pItem);
        //
        // Get size of item
        //
        ItemInfo.Cmd = WIDGET_ITEM_GET_YSIZE;
        ySizeItem    = pfOwnerDraw(&ItemInfo);
        xSizeItem    = 0;
        //
        // Adjust position
        //
        if (j) {
          xPos -= xSizeItem;
          yPos -= ySizeItem;
        }
        //
        // Use callback function for drawing operation
        //
        if (((xPos + xSizeItem) >= 0) || ((yPos + ySizeItem) >= 0)) {
          ItemInfo.Cmd       = WIDGET_ITEM_DRAW;
          ItemInfo.hWin      = hObj;
          ItemInfo.ItemIndex = i;
          ItemInfo.x0        = xPos;
          ItemInfo.y0        = yPos;
          ItemInfo.x1        = RectInside.x1;
          ItemInfo.y1        = yPos + ySizeItem - 1;
          pfOwnerDraw(&ItemInfo);
        }
        //
        // Adjust position
        //
        if (j == 0) {
          xPos += xSizeItem;
          yPos += ySizeItem;
        }
        //
        // Check if we are done
        //
        if (j) {
          if ((xPos < 0) || (yPos < 0)) {
            break;
          }
          if (i == 0) {
            i = NumItems;
          }
          i--;
        } else {
          if (i == (NumItems - 1)) {
            i = -1;
          }
          if ((xPos > RectInside.x1) || (yPos > RectInside.y1)) {
            break;
          }
          i++;
        }
      };
    }
    //
    // Draw overlay
    //
    ItemInfo.Cmd       = WIDGET_ITEM_DRAW_OVERLAY;
    ItemInfo.hWin      = hObj;
    ItemInfo.x0        = 0;
    ItemInfo.y0        = 0;
    ItemInfo.x1        = RectInside.x1;
    ItemInfo.y1        = RectInside.y1;
    pfOwnerDraw(&ItemInfo);
  }
}

/*********************************************************************
*
*       _GetDistToNextItem
*
* Purpose:
*   It returns the distance to the next item relative to the given position.
*
* Parameters:
*   yPos   - Position from which the distance is calculated
*   pIndex - Pointer for storing the index of the next element (could be NULL) 
*/
static int _GetDistToNextItem(LISTWHEEL_Handle hObj, int yPos, int * pIndex) {
  LISTWHEEL_OBJ * pObj;
  GUI_ARRAY ItemArray;
  int NumItems, Pos, PosNew, ySizeItem, ySizeData, i = 0;
  WIDGET_ITEM_DRAW_INFO ItemInfo = {0};
  LISTWHEEL_ITEM * pItem;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  
  //
  // Get widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    pfOwnerDraw = pObj->pfOwnerDraw;
    ItemArray   = pObj->ItemArray;
    ySizeData   = pObj->ySizeData;
    Pos         = pObj->Pos;
  } GUI_UNLOCK_H(pObj);
  NumItems = GUI_ARRAY_GetNumItems(ItemArray);
  while (Pos > yPos) {
    Pos -= ySizeData;
  }
  ItemInfo.Cmd  = WIDGET_ITEM_GET_YSIZE;
  ItemInfo.hWin = hObj;
  do {
    //
    // Get data pointer of item
    //
    pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, i); {
      ItemInfo.p = pItem->pData;
      //
      // If there is no custom data use ItemInfo.p for passing the item handle
      //
      if (ItemInfo.p == NULL) {
        ItemInfo.p = (void *)GUI_ARRAY_GethItem(ItemArray, i);
      }
    } GUI_UNLOCK_H(pItem);
    //
    // Get size of item
    //
    ySizeItem = pfOwnerDraw(&ItemInfo);
    //
    // Check if done
    //
    PosNew = Pos + ySizeItem;
    if ((yPos >= Pos) && (yPos < PosNew)) {
      if (pIndex) {
        *pIndex = i;
      }
      return yPos - Pos;
    }
    if (++i == NumItems) {
      i = 0;
    }
    Pos = PosNew;
  } while (1);
}

/*********************************************************************
*
*       _CreateTimer
*
* Purpose:
*   Creates a new timer. If there is already an existing timer it will
*   be removed
*   after releasing the PID.
*/
static WM_HTIMER _CreateTimer(WM_HWIN hWin, int UserId, int Period, int Mode) {
  LISTWHEEL_OBJ * pObj;
  WM_HTIMER hTimer;

  pObj   = (LISTWHEEL_OBJ *)WM_H2P(hWin);
  hTimer = pObj->hTimer;
  if (hTimer) {
    WM_RestartTimer(hTimer, Period);
  } else {
    hTimer = WM_CreateTimer(hWin, UserId, Period, Mode);
  }
  return hTimer;
}

/*********************************************************************
*
*       _OnReleased
*
* Purpose:
*   This routine makes sure, that the widget moves to the nearest item
*   after releasing the PID.
*/
static void _OnReleased(LISTWHEEL_Handle hObj, int Notification) {
  GUI_ARRAY ItemArray;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  LISTWHEEL_ITEM * pItem;
  LISTWHEEL_OBJ * pObj;
  WM_HMEM hTimer;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  int Velocity, TimeTouched, TimeNow, TimeUsed, Dist, SnapPosition, Index, ySizeItem, Destination;

  //
  // Get widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    pfOwnerDraw  = pObj->pfOwnerDraw;
    Velocity     = pObj->Velocity;
    hTimer       = pObj->hTimer;
    TimeTouched  = pObj->TimeTouched;
    ItemArray    = pObj->ItemArray;
    SnapPosition = pObj->SnapPosition;
    Destination  = pObj->Destination;
  } GUI_UNLOCK_H(pObj);
  if (Destination >= 0) {
    return;
  }
  TimeNow = GUI_GetTime();
  TimeUsed = TimeNow - TimeTouched;
  Dist = 0;
  if (TimeUsed > 100) {
    Velocity = 0;
  }
  //
  // Set timer if required
  //
  if (Velocity && (hTimer == 0)) {
    //
    // If velocity is already available create the timer if required
    //
    hTimer = _CreateTimer(hObj, 0, TIMER_PERIOD, 0);
  } else if (Velocity == 0) {
    //
    // Get the distance to the nearest item
    //
    Dist = _GetDistToNextItem(hObj, SnapPosition, &Index);
    if (Dist) {
      //
      // Get data pointer of item
      //
      pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index); {
        ItemInfo.p = pItem->pData;
        //
        // If there is no custom data use ItemInfo.p for passing the item handle
        //
        if (ItemInfo.p == NULL) {
          ItemInfo.p = (void *)GUI_ARRAY_GethItem(ItemArray, Index);
        }
      } GUI_UNLOCK_H(pItem);
      //
      // Get size of item
      //
      ItemInfo.Cmd  = WIDGET_ITEM_GET_YSIZE;
      ItemInfo.hWin = hObj;
      ySizeItem = pfOwnerDraw(&ItemInfo);
      //
      //
      //
      Dist += ySizeItem >> 1;
      if (Dist > ySizeItem) {
        Dist -= ySizeItem;
      }
      if (Dist > (ySizeItem >> 1)) {
        Velocity = 1;
      } else {
        Velocity = -1;
      }
      hTimer = _CreateTimer(hObj, 0, TIMER_PERIOD, 0);
    }
  }
  //
  // Set widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    pObj->Velocity    = Velocity;
    pObj->TimeTouched = 0;
    pObj->TouchPos    = 0;
    pObj->hTimer      = hTimer;
  } GUI_UNLOCK_H(pObj);
  //
  // Notify parent
  //
  WM_NotifyParent(hObj, Notification);
  if (Dist == 0) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_SEL_CHANGED);
  }
}

/*********************************************************************
*
*       _OnPressed
*
* Purpose:
*   Calculates the velocity in dependence of the timestamp of the last
*   pressed event and the distance to the current position.
*/
static void _OnPressed(LISTWHEEL_Handle hObj, int y) {
  LISTWHEEL_OBJ * pObj;
  int TimeTouched, TimeNow, TimeUsed, Diff = 0, TouchPos, Velocity = 0, Destination;

  //
  // Get widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    TouchPos    = pObj->TouchPos;
    TimeTouched = pObj->TimeTouched;
    Destination = pObj->Destination;
  } GUI_UNLOCK_H(pObj);
  if (Destination >= 0) {
    return;
  }
  //
  // Calculate velocity
  //
  TimeNow = GUI_GetTime();
  if (TimeTouched > 0) {
    Diff = y - TouchPos;
    if (Diff != 0) {
      WM_InvalidateWindow(hObj);
      TimeUsed = TimeNow - TimeTouched + 1;
      Velocity = (Diff * TIMER_PERIOD / TimeUsed);
    }
  }
  //
  // Set widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    pObj->TimeTouched = TimeNow;
    pObj->TouchPos    = y;
    pObj->Velocity    = Velocity;
    pObj->Pos        += Diff;
    while (pObj->Pos < 0) {
      pObj->Pos += pObj->ySizeData;
    }
    while (pObj->Pos > pObj->ySizeData) {
      pObj->Pos -= pObj->ySizeData;
    }
  } GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _CheckTimer
*/
static void _CheckTimer(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int TimeNow, TimeTouched;
  WM_HMEM hTimer;
  
  //
  // Get widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    TimeTouched = pObj->TimeTouched;
    hTimer      = pObj->hTimer;
  } GUI_UNLOCK_H(pObj);
  TimeNow = GUI_GetTime();
  if ((TimeTouched > TimeNow) && (hTimer)) {
    WM_RestartTimer(hTimer, TIMER_PERIOD);
  }
}

/*********************************************************************
*
*       _StopAutoMove
*
* Purpose:
*   Stops moving based on pObj->Velocity and pObj->hTimer
*/
static void _StopAutoMove(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  WM_HMEM hTimer;

  pObj = LISTWHEEL_LOCK_H(hObj); {
    hTimer = pObj->hTimer;
    pObj->Velocity = 0;
    pObj->hTimer   = 0;
  } GUI_UNLOCK_H(pObj);
  if (hTimer) {
    WM_DeleteTimer(hTimer);
  }
}

/*********************************************************************
*
*       _OnTouch
*
* Purpose:
*   Manages PID events if the widget has already captured the input.
*/
static void _OnTouch(LISTWHEEL_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState;
  GUI_RECT Rect;
  int Notification;
  int HasCaptured;

  _CheckTimer(hObj);
  //
  // Get message state
  //
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  HasCaptured = WM_HasCaptured(pMsg->hWin);
  //
  //
  //
  if ((HasCaptured == 0) && (pState)) {
    if (pState->Pressed) {
      _StopAutoMove(hObj);
      WM_SetCapture(hObj, 0);
    }
  }
  //
  // Process message
  //
  if (HasCaptured) {
    if (pMsg->Data.p) {
      WM_GetClientRectEx(pMsg->hWin, &Rect);
      if (pState->Pressed && (pState->y >= 0) && (pState->y <= Rect.y1)) {
        _OnPressed(hObj, pState->y);
      } else {
        if (pState->Pressed) {
          Notification = WM_NOTIFICATION_MOVED_OUT;
        } else {
          Notification = WM_NOTIFICATION_RELEASED;
        }
        _OnReleased(hObj, Notification);
        WM_ReleaseCapture();
      }
    }
  }
}

/*********************************************************************
*
*       _OnPidStateChange
*
* Purpose:
*   Captures the input on the first press.
*/
static void _OnPidStateChange(LISTWHEEL_Handle hObj, WM_MESSAGE * pMsg) {
  LISTWHEEL_OBJ * pObj;
  U16 State;
  const WM_PID_STATE_CHANGED_INFO * pState;

  _CheckTimer(hObj);
  //
  // Get widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    State = pObj->Widget.State;
  } GUI_UNLOCK_H(pObj);
  //
  // Get message state
  //
  pState = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
  //
  // Process message
  //
  if ((pState->StatePrev == 0) && (pState->State == 1)) {
    _StopAutoMove(hObj);
    if ((State & LISTWHEEL_STATE_PRESSED) == 0) {   
      pObj = LISTWHEEL_LOCK_H(hObj); {
        pObj->TimeTouched = GUI_GetTime();
        pObj->TouchPos    = pState->y;
      } GUI_UNLOCK_H(pObj);
      WM_SetCapture(hObj, 0);
      _OnPressed(hObj, pState->y);
      WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
    }
  }
}

/*********************************************************************
*
*       _OnTimer
*
* Purpose:
*   If a destination has been set it sets the velocity to the right direction
*   and accelerates the motion to a maximum value, which depends on the
*   complete data size.
*   If no destination has been set it slows down the motion. It keeps moving
*   with a minimum of speed until any item has reached the sanp position.
*/
static void _OnTimer(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  WM_HMEM hTimer;
  int Pos, Velocity, Swap, ySizeData, SnapPosition, Destination, Dist, MaxVel;

  //
  // Get widget properties
  //
  pObj = LISTWHEEL_LOCK_H(hObj); {
    Pos          = pObj->Pos;
    Velocity     = pObj->Velocity;
    hTimer       = pObj->hTimer;
    ySizeData    = pObj->ySizeData;
    SnapPosition = pObj->SnapPosition;
    Destination  = pObj->Destination;
  } GUI_UNLOCK_H(pObj);
  if (Destination >= 0) {
    //
    // Automatic movement
    //
    if (Pos != Destination) {
      if (Velocity == 0) {
        //
        // Initial velocity
        //
        Dist = Destination - Pos;
        if (Dist > 0) {
          Swap = 0;
        } else {
          Dist = -Dist;
          Swap = 1;
        }
        if (Dist > (ySizeData >> 1)) {
          Swap ^= 1;
        }
        Velocity = -(Swap << 1) + 1;
      } else {
        //
        // Adjust position
        //
        Pos += Velocity;
        while (Pos < 0) {
          Pos += ySizeData;
        }
        while (Pos > ySizeData) {
          Pos -= ySizeData;
        }
        WM_InvalidateWindow(hObj);
        //
        // Acceleration
        //
        if (Velocity > 0) {
          Swap = 0;
          Dist = Destination - Pos;
          while (Dist < 0) {
            Dist += ySizeData;
          }
        } else {
          Swap = 1;
          Velocity = -Velocity;
          Dist = Pos - Destination;
          while (Dist < 0) {
            Dist += ySizeData;
          }
        }
        MaxVel = ySizeData >> 6;
        if (MaxVel < 5) {
          MaxVel = 5;
        }
        if (Velocity < MaxVel) {
          Velocity++;
        }
        if (Velocity > Dist) {
          Velocity = Dist;
        }
        if (Swap) {
          Velocity = -Velocity;
        }
      }
      WM_RestartTimer(hTimer, TIMER_PERIOD);
    } else {
      Destination = -1;
      WM_DeleteTimer(hTimer);
      hTimer = 0;
    }
    pObj = LISTWHEEL_LOCK_H(hObj); {
      pObj->Pos         = Pos;
      pObj->hTimer      = hTimer;
      pObj->Velocity    = Velocity;
      pObj->Destination = Destination;
      pObj->TimeTouched = GUI_GetTime();
    } GUI_UNLOCK_H(pObj);
  } else {
    //
    // Adjust position on current velocity
    //
    while (Pos > ySizeData) {
      Pos -= ySizeData;
    }
    WM_InvalidateWindow(hObj);
    //
    // Decrease velocity
    //
    if (Velocity > 0) {
      Swap = 0;
    } else {
      Swap = 1;
      Velocity = -Velocity;
    }
    if (Velocity > 200) {
      Velocity = 200;
    } else if (Velocity > 100) {
      Velocity -= 10;
    } else if (Velocity > 10) {
      Velocity -= 5;
    } else {
      Velocity -= 1;
    }
    //
    // Keep moving until snap position has been reached
    //
    if (Velocity == 0) {
      if (_GetDistToNextItem(hObj, SnapPosition, NULL)) {
        Velocity = 1;
      }
    }
    //
    // Manage timer
    //
    if (Velocity != 0) {
      WM_RestartTimer(hTimer, TIMER_PERIOD);
    } else {
      WM_DeleteTimer(hTimer);
      hTimer = 0;
      WM_NotifyParent(hObj, WM_NOTIFICATION_SEL_CHANGED);
    }
    //
    // Set widget properties
    //
    if (Swap) {
      Velocity = -Velocity;
    }
    Pos += Velocity;
    pObj = LISTWHEEL_LOCK_H(hObj); {
      pObj->Pos      = Pos;
      pObj->hTimer   = hTimer;
      pObj->Velocity = Velocity;
    } GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _RemoveItems
*/
static void _RemoveItems(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  GUI_ARRAY ItemArray;
  int Index;

  pObj = LISTWHEEL_LOCK_H(hObj); {
    ItemArray = pObj->ItemArray;
  } GUI_UNLOCK_H(pObj);
  Index = GUI_ARRAY_GetNumItems(ItemArray);
  while (Index--) {
    GUI_ARRAY_DeleteItem(ItemArray, Index);
  }
}

/*********************************************************************
*
*       _FreeAttached
*/
static void _FreeAttached(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  GUI_ARRAY ItemArray;

  pObj = LISTWHEEL_LOCK_H(hObj); {
    ItemArray = pObj->ItemArray;
    pObj->ItemArray = 0;
    pObj->hTimer    = 0;
  } GUI_UNLOCK_H(pObj);
  GUI_ARRAY_Delete(ItemArray);
}

/*********************************************************************
*
*       _GetItemHeight
*/
static int _GetItemHeight(LISTWHEEL_Handle hObj, unsigned int ItemIndex) {
  LISTWHEEL_OBJ * pObj;
  WIDGET_ITEM_DRAW_INFO ItemInfo = {0};
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  LISTWHEEL_ITEM * pItem;
  GUI_ARRAY ItemArray;
  int Height;

  pObj = LISTWHEEL_LOCK_H(hObj); {
    pfOwnerDraw = pObj->pfOwnerDraw;
    ItemArray   = pObj->ItemArray;
  } GUI_UNLOCK_H(pObj);
  pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex); {
    ItemInfo.p = pItem->pData;
    //
    // If there is no custom data use ItemInfo.p for passing the item handle
    //
    if (ItemInfo.p == NULL) {
      ItemInfo.p = (void *)GUI_ARRAY_GethItem(ItemArray, ItemIndex);
    }
  } GUI_UNLOCK_H(pItem);
  ItemInfo.Cmd = WIDGET_ITEM_GET_YSIZE;
  ItemInfo.hWin = hObj;
  ItemInfo.ItemIndex = ItemIndex;
  Height = pfOwnerDraw(&ItemInfo);
  return Height;
}

/*********************************************************************
*
*       _GetDestination
*/
static int _GetDestination(LISTWHEEL_Handle hObj, unsigned int Index) {
  LISTWHEEL_OBJ * pObj;
  int SnapPosition, Destination, ySizeData;

  pObj = LISTWHEEL_LOCK_H(hObj); {
    SnapPosition = pObj->SnapPosition;
    ySizeData    = pObj->ySizeData;
  } GUI_UNLOCK_H(pObj);

  for (Destination = 0; Index > 0; Index--) {
    Destination += _GetItemHeight(hObj, Index - 1);
  }
  Destination = ySizeData - Destination + SnapPosition;
  while (Destination > ySizeData) {
    Destination -= ySizeData;
  }

  return Destination;
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTWHEEL_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
LISTWHEEL_OBJ * LISTWHEEL_LockH(LISTWHEEL_Handle h) {
  LISTWHEEL_OBJ * p;
  p = (LISTWHEEL_OBJ *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != LISTWHEEL_ID) {
      GUI_DEBUG_ERROROUT("LISTWHEEL.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTWHEEL_Callback
*/
void LISTWHEEL_Callback(WM_MESSAGE * pMsg) {
  LISTWHEEL_Handle hObj;

  hObj = pMsg->hWin;
  //
  // Let widget handle the standard messages
  //
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  //
  // Process widget specific messages
  //
  switch (pMsg->MsgId) {
  case WM_DELETE:
    _FreeAttached(hObj);
    break;
  case WM_TIMER:
    _OnTimer(hObj);
    break;
  case WM_PID_STATE_CHANGED:
    _OnPidStateChange(hObj, pMsg);
    break;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_PAINT:
    _OnPaint(hObj);
    return;
  }
  //
  // Process remaining messages
  //
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       _FillStringInRect
*/
static void _FillStringInRect(char * s, GUI_RECT * pRect, int Align, int HasTrans) {
  int Len;

  Len = GUI__strlen(s);
  if (HasTrans == 0) {
    GUI_ClearRectEx(pRect);
    GUI_SetTextMode(GUI_TM_NORMAL);
  } else {
    GUI_SetTextMode(GUI_TM_TRANS);
  }
  GUI_DispStringInRectMax(s, pRect, Align, Len);
}

/*********************************************************************
*
*       LISTWHEEL_OwnerDraw
*/
int LISTWHEEL_OwnerDraw(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  LISTWHEEL_OBJ * pObj;
  LISTWHEEL_ITEM * pItem;
  int xSize, ySizeItem, ySizeFont, LineHeight, Index, Align, HasTrans;
  char * pText;
  GUI_RECT Rect;
  unsigned LBorder;
  unsigned RBorder;
  GUI_COLOR Color, BkColor;

  if (pDrawItemInfo->p == NULL) {
    return 0;
  }
  //
  // Get object properties
  //
  pObj = LISTWHEEL_LOCK_H(pDrawItemInfo->hWin); {
    ySizeFont  = GUI_GetYSizeOfFont(pObj->Props.pFont);
    xSize      = pObj->Widget.Win.Rect.x1 - pObj->Widget.Win.Rect.x0 + 1;
    LBorder    = pObj->LBorder;
    RBorder    = pObj->RBorder;
    LineHeight = pObj->LineHeight;
    Index      = (pObj->Sel == pDrawItemInfo->ItemIndex) ? LISTWHEEL_CI_SEL : LISTWHEEL_CI_UNSEL;
    Color      = pObj->Props.aTextColor[Index];
    BkColor    = pObj->Props.aBackColor[Index];
    Align      = pObj->Props.Align;
  } GUI_UNLOCK_H(pObj);
  if (LineHeight) {
    ySizeItem = LineHeight;
  } else {
    ySizeItem = ySizeFont;
  }
  //
  // Get item properties
  //
  pItem = (LISTWHEEL_ITEM *)GUI_LOCK_H((WM_HMEM)pDrawItemInfo->p); {
    pText = pItem->acText;
  } GUI_UNLOCK_H(pItem);
  //
  // Process messages
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_GET_XSIZE:
    return xSize;
  case WIDGET_ITEM_GET_YSIZE:
    if (LineHeight) {
      return LineHeight;
    } else {
      return ySizeItem;
    }
  case WIDGET_ITEM_DRAW:
    Rect.x0 = pDrawItemInfo->x0 + LBorder;
    Rect.y0 = pDrawItemInfo->y0;
    Rect.x1 = pDrawItemInfo->x1 - RBorder;
    Rect.y1 = pDrawItemInfo->y1;
    #if WM_SUPPORT_TRANSPARENCY
      HasTrans = WM_GetHasTrans(pDrawItemInfo->hWin);
    #else
      HasTrans = 0;
    #endif
    GUI_SetBkColor(BkColor);
    GUI_SetColor(Color);
    _FillStringInRect(pText, &Rect, Align, HasTrans);
    if (HasTrans == 0) {
      if (LBorder) {
        GUI_ClearRect(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->x0 + LBorder - 1, pDrawItemInfo->y1);
      }
      if (RBorder) {
        GUI_ClearRect(pDrawItemInfo->x1 - RBorder, pDrawItemInfo->y0, pDrawItemInfo->x1, pDrawItemInfo->y1);
      }
    }
    break;
  }
  return 0;
}

/*********************************************************************
*
*       LISTWHEEL_CreateEx
*/
LISTWHEEL_Handle LISTWHEEL_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                int WinFlags, int ExFlags, int Id, const GUI_ConstString * ppText)
{
  LISTWHEEL_Handle hObj;
  GUI_ARRAY ItemArray;
  LISTWHEEL_OBJ * pObj;

  GUI_USE_PARA(ExFlags);
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, LISTWHEEL_Callback,
                                sizeof(LISTWHEEL_OBJ) - sizeof(WM_Obj));
  if (hObj) {
    ItemArray = GUI_ARRAY_Create();
    if (ItemArray) {
      pObj = (LISTWHEEL_OBJ *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
      //
      // Default initialization
      //
      WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
      LISTWHEEL_INIT_ID(pObj);
      //
      // Widget specific initialization
      //
      pObj->Widget.pEffect = &WIDGET_Effect_None;
      pObj->ItemArray   = ItemArray;
      pObj->Props       = LISTWHEEL_DefaultProps;
      pObj->pfOwnerDraw = LISTWHEEL_OwnerDraw;
      pObj->Destination = -1;
      GUI_UNLOCK_H(pObj);
      if (ppText) {
        LISTWHEEL_SetText(hObj, ppText);
      }
    } else {
      WM_DeleteWindow(hObj);
    }
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       LISTWHEEL_AddString
*/
void LISTWHEEL_AddString(LISTWHEEL_Handle hObj, const char * s) {
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  LISTWHEEL_ITEM * pItem;
  LISTWHEEL_OBJ * pObj;
  LISTWHEEL_ITEM Item = {0};
  GUI_RECT RectInside;
  GUI_ARRAY ItemArray;
  unsigned ItemIndex;
  int ySizeData;

  if (hObj && s) {
    WM_LOCK();
    //
    // Get properties of widget
    //
    WM_GetInsideRect(&RectInside);
    pObj = LISTWHEEL_LOCK_H(hObj); {
      GUI_SetFont(pObj->Props.pFont);
      ItemArray   = pObj->ItemArray;
      pfOwnerDraw = pObj->pfOwnerDraw;
    } GUI_UNLOCK_H(pObj);
    ItemIndex = GUI_ARRAY_GetNumItems(ItemArray);
    //
    // Set item properties
    //
    if (GUI_ARRAY_AddItem(ItemArray, &Item, sizeof(LISTWHEEL_ITEM) + strlen(s)) == 0) {
      pItem     = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex); {
        strcpy(pItem->acText, s);
        ItemInfo.p = pItem->pData;
        //
        // If there is no custom data use ItemInfo.p for passing the item handle
        //
        if (ItemInfo.p == NULL) {
          ItemInfo.p = (void *)GUI_ARRAY_GethItem(ItemArray, ItemIndex);
        }
      } GUI_UNLOCK_H(pItem);
      //
      // Get item size
      //
      ItemInfo.hWin      = hObj;
      ItemInfo.ItemIndex = ItemIndex;
      ItemInfo.Cmd       = WIDGET_ITEM_GET_YSIZE;
      ySizeData          = pfOwnerDraw(&ItemInfo);
      //
      // Set widget properties
      //
      pObj = LISTWHEEL_LOCK_H(hObj); {
        pObj->ySizeData += ySizeData;
      } GUI_UNLOCK_H(pObj);
      //
      // Invalidate object
      //
      WM_Invalidate(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_SetFont
*/
void LISTWHEEL_SetFont(LISTWHEEL_Handle hObj, const GUI_FONT GUI_UNI_PTR * pFont) {
  LISTWHEEL_OBJ * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    pObj->Props.pFont = pFont;
    GUI_UNLOCK_H(pObj);
    _RecalcDatasizeY(hObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetFont
*/
const GUI_FONT GUI_UNI_PTR * LISTWHEEL_GetFont(LISTWHEEL_Handle hObj) {
  const GUI_FONT GUI_UNI_PTR * pFont = NULL;
  LISTWHEEL_OBJ * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    pFont = pObj->Props.pFont;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return pFont;
}

/*********************************************************************
*
*       LISTWHEEL_SetOwnerDraw
*/
void LISTWHEEL_SetOwnerDraw(LISTWHEEL_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pfOwnerDraw) {
      pObj->pfOwnerDraw = pfOwnerDraw;
    }else {
      pObj->pfOwnerDraw = LISTWHEEL_OwnerDraw;
    }
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL__GetpStringLocked
*
*  Returns:
*    Pointer to the specified item
*/
const char * LISTWHEEL__GetpStringLocked(LISTWHEEL_Handle hObj, int Index) {
  LISTWHEEL_OBJ * pObj;
  const char * s = NULL;
  LISTWHEEL_ITEM * pItem;
  pObj = LISTWHEEL_LOCK_H(hObj);
  pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
  GUI_UNLOCK_H(pObj);
  if (pItem) {
    s = pItem->acText;
  }
  return s;
}

/*********************************************************************
*
*       LISTWHEEL_GetNumItems
*/
int LISTWHEEL_GetNumItems(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int r = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    r = GUI_ARRAY_GetNumItems(pObj->ItemArray);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       LISTWHEEL_GetItemText
*/
void LISTWHEEL_GetItemText(LISTWHEEL_Handle hObj, unsigned Index, char * pBuffer, int MaxSize) {
  unsigned NumItems;

  if (hObj) {
    WM_LOCK();
    NumItems = LISTWHEEL_GetNumItems(hObj);
    if (Index < NumItems) {
      const char * pString;
      int CopyLen;
      pString = LISTWHEEL__GetpStringLocked(hObj, Index);
      CopyLen = strlen(pString);
      if (CopyLen > (MaxSize - 1)) {
        CopyLen = MaxSize - 1;
      }
      GUI_MEMCPY(pBuffer, pString, CopyLen);
      GUI_UNLOCK_H(pString);
      pBuffer[CopyLen] = 0;
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_SetItemData
*/
void LISTWHEEL_SetItemData(LISTWHEEL_Handle hObj, unsigned Index, void * pData) {
  LISTWHEEL_OBJ * pObj;
  LISTWHEEL_ITEM * pItem;
  unsigned NumItems;
  GUI_ARRAY ItemArray;

  if (hObj) {
    WM_LOCK();
    //
    // Get widget properties
    //
    pObj = LISTWHEEL_LOCK_H(hObj); {
      ItemArray   = pObj->ItemArray;
    } GUI_UNLOCK_H(pObj);
    NumItems = GUI_ARRAY_GetNumItems(ItemArray);
    if (Index < NumItems) {
      //
      // Get data pointer of item
      //
      pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index); {
        pItem->pData = pData;
      } GUI_UNLOCK_H(pItem);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetItemData
*/
void * LISTWHEEL_GetItemData(LISTWHEEL_Handle hObj, unsigned Index) {
  LISTWHEEL_OBJ * pObj;
  LISTWHEEL_ITEM * pItem;
  unsigned NumItems;
  GUI_ARRAY ItemArray;
  void * pData = NULL;

  if (hObj) {
    WM_LOCK();
    //
    // Get widget properties
    //
    pObj = LISTWHEEL_LOCK_H(hObj); {
      ItemArray   = pObj->ItemArray;
    } GUI_UNLOCK_H(pObj);
    NumItems = GUI_ARRAY_GetNumItems(ItemArray);
    if (Index < NumItems) {
      //
      // Get data pointer of item
      //
      pItem = (LISTWHEEL_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index); {
        pData = pItem->pData;
      } GUI_UNLOCK_H(pItem);
    }
    WM_UNLOCK();
  }
  return pData;
}

/*********************************************************************
*
*       LISTWHEEL_SetUserData
*/
void LISTWHEEL_SetUserData(LISTWHEEL_Handle hObj, void * pData) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    //
    // Get widget properties
    //
    pObj = LISTWHEEL_LOCK_H(hObj); {
      pObj->pData = pData;
    } GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetUserData
*/
void * LISTWHEEL_GetUserData(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  void * pData = NULL;

  if (hObj) {
    WM_LOCK();
    //
    // Get widget properties
    //
    pObj = LISTWHEEL_LOCK_H(hObj); {
      pData = pObj->pData;
    } GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return pData;
}

/*********************************************************************
*
*       LISTWHEEL_SetSnapPosition
*/
void LISTWHEEL_SetSnapPosition(LISTWHEEL_Handle hObj, int SnapPosition) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    pObj->SnapPosition = pObj->Pos = SnapPosition;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetSnapPosition
*/
int LISTWHEEL_GetSnapPosition(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int r = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    r = pObj->SnapPosition;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       LISTWHEEL_SetTextAlign
*/
void LISTWHEEL_SetTextAlign(LISTWHEEL_Handle hObj, int Align) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    pObj->Props.Align = Align;
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetTextAlign
*/
int LISTWHEEL_GetTextAlign(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int Align = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    Align = pObj->Props.Align;
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
  return Align;
}

/*********************************************************************
*
*       LISTWHEEL_SetRBorder
*/
void LISTWHEEL_SetRBorder(LISTWHEEL_Handle hObj, unsigned BorderSize) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pObj->RBorder != BorderSize) {
      pObj->RBorder = BorderSize;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_SetLBorder
*/
void LISTWHEEL_SetLBorder(LISTWHEEL_Handle hObj, unsigned BorderSize) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pObj->LBorder != BorderSize) {
      pObj->LBorder = BorderSize;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetRBorder
*/
int LISTWHEEL_GetRBorder(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int RBorder = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    RBorder = pObj->RBorder;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return RBorder;
}

/*********************************************************************
*
*       LISTWHEEL_GetLBorder
*/
int LISTWHEEL_GetLBorder(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int LBorder = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    LBorder = pObj->LBorder;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return LBorder;
}

/*********************************************************************
*
*       LISTWHEEL_SetLineHeight
*/
void LISTWHEEL_SetLineHeight(LISTWHEEL_Handle hObj, unsigned LineHeight) {
  LISTWHEEL_OBJ * pObj;
  int Recalc = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pObj->LineHeight != LineHeight) {
      pObj->LineHeight = LineHeight;
      WM_Invalidate(hObj);
      Recalc = 1;
    }
    GUI_UNLOCK_H(pObj);
    if (Recalc) {
      _RecalcDatasizeY(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_SetSel
*/
void LISTWHEEL_SetSel(LISTWHEEL_Handle hObj, int Sel) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pObj->Sel != Sel) {
      pObj->Sel = Sel;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetSel
*/
int LISTWHEEL_GetSel(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int Sel = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pObj->Sel != Sel) {
      Sel = pObj->Sel;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Sel;
}

/*********************************************************************
*
*       LISTWHEEL_GetLineHeight
*/
unsigned LISTWHEEL_GetLineHeight(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  unsigned LineHeight = 0;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj);
    if (pObj->LineHeight != LineHeight) {
      LineHeight = pObj->LineHeight;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return LineHeight;
}

/*********************************************************************
*
*       LISTWHEEL_SetTextColor
*/
void LISTWHEEL_SetTextColor(LISTWHEEL_Handle hObj, unsigned int Index, GUI_COLOR Color) {
  LISTWHEEL_OBJ * pObj;

  if (hObj) {
    if (Index < GUI_COUNTOF(pObj->Props.aBackColor)) {
      WM_LOCK();
      pObj = LISTWHEEL_LOCK_H(hObj);
      pObj->Props.aTextColor[Index] = Color;
      GUI_UNLOCK_H(pObj);
      WM_Invalidate(hObj);
      WM_UNLOCK();
    }
  }
}

/*********************************************************************
*
*       LISTWHEEL_GetPos
*/
int LISTWHEEL_GetPos(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int SnapPosition, Index = -1;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj); {
      SnapPosition = pObj->SnapPosition;
    } GUI_UNLOCK_H(pObj);
    _GetDistToNextItem(hObj, SnapPosition, &Index);
    WM_UNLOCK();
  }
  return Index;
}

/*********************************************************************
*
*       LISTWHEEL_GetVelocity
*/
int LISTWHEEL_GetVelocity(LISTWHEEL_Handle hObj) {
  LISTWHEEL_OBJ * pObj;
  int Velocity;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj); {
		Velocity = pObj->Velocity;
    } GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Velocity;
}

/*********************************************************************
*
*       LISTWHEEL_SetText
*/
void LISTWHEEL_SetText(LISTWHEEL_Handle hObj, const GUI_ConstString * ppText) {
  int i;
  const char * s;

  if (hObj) {
    WM_LOCK();
    if (ppText) {
      //
      // Remove previous items
      //
      _RemoveItems(hObj);
      //
      // Add new items
      //
      for (i = 0; (s = *(ppText + i)) != 0; i++) {
        LISTWHEEL_AddString(hObj, s);
      }
    }
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_MoveToPos
*/
void LISTWHEEL_MoveToPos(LISTWHEEL_Handle hObj, unsigned int Index) {
  LISTWHEEL_OBJ * pObj;
  WM_HMEM hTimer;
  int Pos, Destination;

  if (hObj) {
    WM_LOCK();
    _StopAutoMove(hObj);
    pObj = LISTWHEEL_LOCK_H(hObj); {
      Pos          = pObj->Pos;
      Destination  = pObj->Destination;
      hTimer       = pObj->hTimer;
    } GUI_UNLOCK_H(pObj);
    Destination = _GetDestination(hObj, Index);
    if (Destination != Pos) {
      if (hTimer == 0) {
        hTimer = _CreateTimer(hObj, 0, TIMER_PERIOD, 0);
      }
      pObj = LISTWHEEL_LOCK_H(hObj); {
        pObj->Destination = Destination;
        pObj->hTimer      = hTimer;
        pObj->TimeTouched = GUI_GetTime();
      } GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_SetVelocity
*/
void LISTWHEEL_SetVelocity(LISTWHEEL_Handle hObj, int Velocity) {
  LISTWHEEL_OBJ * pObj;
  WM_HMEM hTimer;
  int Destination;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj); {
      Destination  = pObj->Destination;
      hTimer       = pObj->hTimer;
    } GUI_UNLOCK_H(pObj);
    if (Destination == -1) {
      if (hTimer == 0) {
        hTimer = _CreateTimer(hObj, 0, TIMER_PERIOD, 0);
      }
      pObj = LISTWHEEL_LOCK_H(hObj); {
        pObj->Velocity    = Velocity;
        pObj->hTimer      = hTimer;
        pObj->TimeTouched = GUI_GetTime();
      } GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTWHEEL_SetPos
*/
void LISTWHEEL_SetPos(LISTWHEEL_Handle hObj, unsigned int Index) {
  LISTWHEEL_OBJ * pObj;
  int Pos, Destination, NumItems;

  if (hObj) {
    WM_LOCK();
    pObj = LISTWHEEL_LOCK_H(hObj); {
      NumItems     = GUI_ARRAY_GetNumItems(pObj->ItemArray);
      Pos          = pObj->Pos;
    } GUI_UNLOCK_H(pObj);
    if (Index < (unsigned)NumItems) {
      Destination = _GetDestination(hObj, Index);
      if (Destination != Pos) {
        pObj = LISTWHEEL_LOCK_H(hObj); {
          pObj->Pos         = Destination;
        } GUI_UNLOCK_H(pObj);
        WM_Invalidate(hObj);
      }
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTWHEEL_C(void) {}
#endif

/*************************** End of file ****************************/
