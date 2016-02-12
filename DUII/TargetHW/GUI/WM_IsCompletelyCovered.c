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
File        : WM_IsCompletelyCovered.c
Purpose     : Windows manager, implementation of said function
----------------------------------------------------------------------
*/

#include <stddef.h>
#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ASSIGN_IF_LESS(v0, v1) if (v1 < v0) v0 = v1

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  WM_HWIN hWin;
  GUI_RECT ClientRect;
  GUI_RECT CurRect;
  int Cnt;
  int EntranceCnt;
} CONTEXT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static CONTEXT _Context;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Findy1
*
*/
static void _Findy1(WM_HWIN iWin, GUI_RECT * pRect, GUI_RECT * pParentRect) {
  WM_Obj * pWin;
  for (; iWin; iWin = pWin->hNext) { 
    int Status;
    Status = (pWin = WM_H2P(iWin))->Status;
    /* Check if this window affects us at all */    
    if (Status & WM_SF_ISVIS) {
      GUI_RECT rWinClipped;               /* Window rect, clipped to part inside of ancestors */
      if (pParentRect) {
        GUI__IntersectRects(&rWinClipped, &pWin->Rect, pParentRect);
      } else {
        rWinClipped = pWin->Rect;
      }
      /* Check if this window affects us at all */    
      if (GUI_RectsIntersect(pRect, &rWinClipped)) {
        if ((Status & WM_SF_HASTRANS) == 0) {
          if (pWin->Rect.y0 > pRect->y0) {
            ASSIGN_IF_LESS(pRect->y1, rWinClipped.y0 - 1);    /* Check upper border of window */
          } else {
            ASSIGN_IF_LESS(pRect->y1, rWinClipped.y1);        /* Check lower border of window */
          }
        } else {
          /* Check all children */ 
          WM_HWIN hChild;
          WM_Obj* pChild;
          for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
            pChild = WM_H2P(hChild);
            _Findy1(hChild, pRect, &rWinClipped);
          }
        }
      }
    }
  }
}

/*********************************************************************
*
*       _Findx0
*/
static int _Findx0(WM_HWIN hWin, GUI_RECT * pRect, GUI_RECT * pParentRect) {
  WM_Obj * pWin;
  int r = 0;
  for (; hWin; hWin = pWin->hNext) { 
    int Status;
    Status = (pWin = WM_H2P(hWin))->Status;
    if (Status & WM_SF_ISVIS) {           /* If window is not visible, it can be safely ignored */
      GUI_RECT rWinClipped;               /* Window rect, clipped to part inside of ancestors */
      if (pParentRect) {
        GUI__IntersectRects(&rWinClipped, &pWin->Rect, pParentRect);
      } else {
        rWinClipped = pWin->Rect;
      }
      /* Check if this window affects us at all */    
      if (GUI_RectsIntersect(pRect, &rWinClipped)) {
        if ((Status & WM_SF_HASTRANS) == 0) {
          pRect->x0 = rWinClipped.x1 + 1;
          r = 1;
        } else {
          /* Check all children */
          WM_HWIN hChild;
          WM_Obj * pChild;
          for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
            pChild = WM_H2P(hChild);
            if (_Findx0(hChild, pRect, &rWinClipped)) {
              r = 1;
            }
          }
        }
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       _Findx1
*/
static void _Findx1(WM_HWIN hWin, GUI_RECT * pRect, GUI_RECT * pParentRect) {
  WM_Obj* pWin;
  for (; hWin; hWin = pWin->hNext) { 
    int Status;
    Status = (pWin = WM_H2P(hWin))->Status;
    if (Status & WM_SF_ISVIS) {           /* If window is not visible, it can be safely ignored */
      GUI_RECT rWinClipped;               /* Window rect, clipped to part inside of ancestors */
      if (pParentRect) {
        GUI__IntersectRects(&rWinClipped, &pWin->Rect, pParentRect);
      } else {
        rWinClipped = pWin->Rect;
      }
      /* Check if this window affects us at all */    
      if (GUI_RectsIntersect(pRect, &rWinClipped)) {
        if ((Status & WM_SF_HASTRANS) == 0) {
          pRect->x1 = rWinClipped.x0-1;
        } else {
          /* Check all children */
          WM_HWIN hChild;
          WM_Obj * pChild;
          for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
            pChild = WM_H2P(hChild);
            _Findx1(hChild, pRect, &rWinClipped);
          }
        }
      }
    }
  }
}

/*********************************************************************
*
*       _FindNextRect
*/
static int _FindNextRect(void) {
  WM_HMEM hParent;
  GUI_RECT r;
  WM_Obj * pAWin;
  WM_Obj * pParent;
  r = _Context.CurRect;  /* temps  so we do not have to work with pointers too much */
  /*
     STEP 1:
       Set the next position which could be part of the next rectangle.
       This will be the first unhandle pixel in reading order, i.e. next one to the right
       or next one down if we are at the right border.
  */
  if (_Context.Cnt == 0) {       /* First rectangle starts in upper left */
    r.x0 = _Context.ClientRect.x0;
    r.y0 = _Context.ClientRect.y0;
  } else {
    r.x0 = _Context.CurRect.x1 + 1;
    r.y0 = _Context.CurRect.y0;
    if (r.x0 > _Context.ClientRect.x1) {
NextStripe:  /* go down to next stripe */
      r.x0 = _Context.ClientRect.x0;
      r.y0 = _Context.CurRect.y1 + 1;
    }
  }
  /*
     STEP 2:
       Check if we are done completely.
  */
  if (r.y0 >_Context.ClientRect.y1) {
    return 0;
  }
  /* STEP 3:
       Find out the max. height (r.y1) if we are at the left border.
       Since we are using the same height for all rectangles at the same y0,
       we do this only for the leftmost one.
  */
  pAWin = WM_H2P(_Context.hWin);
  if (r.x0 == _Context.ClientRect.x0) {
    r.y1 = _Context.ClientRect.y1;
    r.x1 = _Context.ClientRect.x1;
    /* Iterate over all windows which are above */
    /* Check all siblings above (Iterate over Parents and top siblings (hNext) */
    for (hParent = _Context.hWin; hParent; hParent = pParent->hParent) {
      pParent = WM_H2P(hParent);
      _Findy1(pParent->hNext, &r, NULL);
    }
    /* Check all children */
    _Findy1(pAWin->hFirstChild, &r, NULL);
  }
  /* 
    STEP 4
      Find out x0 for the given y0, y1 by iterating over windows above.
      if we find one that intersects, adjust x0 to the right.
  */
Find_x0:
  r.x1 = r.x0;
  /* Iterate over all windows which are above */
  /* Check all siblings above (siblings of window, siblings of parents, etc ...) */
  hParent = _Context.hWin;
  for (; hParent; hParent = pParent->hParent) {
    pParent = WM_H2P(hParent);
    if (_Findx0(pParent->hNext, &r, NULL)) {
      goto Find_x0;
    }
  }
  /* Check all children */
  if (_Findx0(pAWin->hFirstChild, &r, NULL)) {
    goto Find_x0;
  }
  /* 
   STEP 5:
     If r.x0 out of right border, this stripe is done. Set next stripe and goto STEP 2
     Find out x1 for the given x0, y0, y1
  */
  r.x1 = _Context.ClientRect.x1;
  if (r.x1 < r.x0) {/* horizontal border reached ? */
    _Context.CurRect = r;
    goto NextStripe;
  }    
  /* 
   STEP 6:
     Find r.x1. We have to Iterate over all windows which are above
  */
  hParent = _Context.hWin;
  for (; hParent; hParent = pParent->hParent) {
    pParent = WM_H2P(hParent);
    _Findx1(pParent->hNext, &r, NULL);
  }
  /* Check all children */
  _Findx1(pAWin->hFirstChild, &r, NULL);
  /* We are done. Return the rectangle we found in the _Context. */
  if (_Context.Cnt > 200) {
    return 0;  /* error !!! This should not happen !*/
  }
  _Context.CurRect = r;
  return 1;
}

/*********************************************************************
*
*       _GetNextRect
*/
static int  _GetNextRect(void) {
  if (_Context.EntranceCnt > 1) {
    _Context.EntranceCnt--;
    return 0;
  }
  ++_Context.Cnt;
  /* Find next rectangle and use it as ClipRect */
  if (!_FindNextRect()) {
    _Context.EntranceCnt--;  /* This search is over ! */
    return 0;        /* Could not find an other one ! */
  }
  return 1;
}

/*********************************************************************
*
*       _InitRectSearch
*/
static int _InitRectSearch(const GUI_RECT * pMaxRect) {
  GUI_RECT r;
  WM_Obj * pAWin;
  /* If we entered multiple times, leave Cliprect alone */
  if (++_Context.EntranceCnt > 1) {
    return 1;
  }
  pAWin = WM_H2P(_Context.hWin);
  _Context.Cnt        = -1;
  if (pAWin->Status & WM_SF_ISVIS) {
    r = pAWin->Rect;
  } else {
    --_Context.EntranceCnt;
    return 0;  /* window is not even visible ! */
  }
  /* If the drawing routine has specified a rectangle, use it to reduce the rectangle */
  if (pMaxRect) {
    GUI__IntersectRect(&r, pMaxRect);
  }
  /* Iterate over all ancestors and clip at their borders. If there is no visible part, we are done */
  if (WM__ClipAtParentBorders(&r, _Context.hWin) == 0) {
    --_Context.EntranceCnt;
    return 0;           /* Nothing to draw */
  }
  /* Store the rectangle and find the first rectangle of the area */
  _Context.ClientRect = r;
  return _GetNextRect();
}

/*********************************************************************
*
*       _IsCompletelyCovered
*
*/
static char _IsCompletelyCovered(WM_HWIN hWin) {
  GUI_RECT Rect = {0};
  Rect.x1 = LCD_GetVXSizeEx(GUI_Context.SelLayer);
  Rect.y1 = LCD_GetVYSizeEx(GUI_Context.SelLayer);
  _Context.hWin        = hWin;
  _Context.EntranceCnt = 0;
  if (_InitRectSearch(&Rect)) {
    return 0;                 /* Is not completely covered */
  }
  return 1;                   /* Is completely covered */
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_IsCompletelyCovered
*
*/
char WM_IsCompletelyCovered(WM_HWIN hWin) {
  int r = 0;
  if (hWin) {
    WM_LOCK();
    r = _IsCompletelyCovered(hWin);
    WM_UNLOCK();
  }
  return r;
}

#else
  void WM_IsCompletelyCovered_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
