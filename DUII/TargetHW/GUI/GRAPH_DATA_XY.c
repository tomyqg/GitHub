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
File        : GRAPH_DATA_XY.c
Purpose     : Implementation of YT graph
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GRAPH_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define _LINES_VIS  (1 << 0)
#define _POINTS_VIS (1 << 1)

#ifndef   GRAPH_DATA_XY_POINTSIZE_DEFAULT
  #define GRAPH_DATA_XY_POINTSIZE_DEFAULT 5
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  GRAPH_DATA_OBJ GraphDataObj;
  WM_HMEM        hData;
  unsigned       PointSize;
  U8             Flags;
  U8             PenSize;
  U8             LineStyle;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
} GRAPH_DATA_XY_OBJ;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawPolyPoints
*/
static void _DrawPolyPoints(GRAPH_DATA_Handle hDataObj, int x0, int y0) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  const GUI_POINT * pPoints;
  int NumPoints;
  int x, y, Off;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
  pPoints    = (GUI_POINT *)GUI_LOCK_H(pDataXYObj->hData);
  NumPoints  = pDataXYObj->GraphDataObj.NumItems;
  Off        = (pDataXYObj->PointSize - 1) / 2;
  if (pDataXYObj->pfOwnerDraw) {
    ItemInfo.Cmd  = WIDGET_ITEM_DRAW;
    ItemInfo.p    = (void *)&hDataObj;
    do {
      ItemInfo.x0 = x0 + pPoints->x;
      ItemInfo.y0 = y0 - pPoints->y;
      pDataXYObj->pfOwnerDraw(&ItemInfo);
      pPoints++;
    } while (--NumPoints > 0);
  } else {
    WM_ADDORG(x0,y0);
    WM_ITERATE_START(NULL); {
      do {
        x = x0 + pPoints->x;
        y = y0 - pPoints->y;
        GL_DrawLine(x - Off, y - Off, x + Off, y + Off);
        GL_DrawLine(x - Off, y + Off, x + Off, y - Off);
        pPoints++;
      } while (--NumPoints > 0);
    } WM_ITERATE_END();
  }
  GUI_UNLOCK_H(pPoints);
  GUI_UNLOCK_H(pDataXYObj);
}

/*********************************************************************
*
*       _DrawPolyLine
*/
static void _DrawPolyLine(const GUI_POINT * pPoints, int NumPoints, int x0, int y0) {
  unsigned PixelCnt;
  PixelCnt = 0;
  WM_ADDORG(x0,y0);
  WM_ITERATE_START(NULL); {
    GL_MoveTo(pPoints->x + x0, y0 - pPoints->y);
    while (--NumPoints >0) {
      pPoints++;
      GL_DrawLineToEx(pPoints->x + x0, y0 - pPoints->y, &PixelCnt);
    }
  } WM_ITERATE_END();
}

/*********************************************************************
*
*       _OnPaint
*/
static void _OnPaint(GRAPH_DATA_Handle hDataObj, GUI_RECT * pRect) {
  GUI_POINT         * pData;
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  GUI_USE_PARA(pRect);
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
  if (pDataXYObj->hData) {
    GUI_RECT Rect;
    GRAPH_OBJ * pObj;
    int xPos, yPos;
    int BorderL, BorderT, BorderR, BorderB;
    pData = (GUI_POINT *)GUI_LOCK_H(pDataXYObj->hData);
    pObj = GRAPH_LOCK_H(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    BorderL = pObj->Props.BorderL;
    BorderT = pObj->Props.BorderT;
    BorderR = pObj->Props.BorderR;
    BorderB = pObj->Props.BorderB;
    WM_GetInsideRectExScrollbar(pDataXYObj->GraphDataObj.PaintObj.hGraph, &Rect);
    Rect.x0 += BorderL;
    Rect.y0 += BorderT;
    Rect.x1 -= BorderR;
    Rect.y1 -= BorderB;
    xPos     = Rect.x0 + pDataXYObj->GraphDataObj.OffX;
    yPos     = Rect.y1 - pDataXYObj->GraphDataObj.OffY;
    GUI_SetColor(pDataXYObj->GraphDataObj.Color);
    GUI_SetPenSize(pDataXYObj->PenSize);
    GUI_SetLineStyle(pDataXYObj->LineStyle);
    if (pDataXYObj->Flags & _POINTS_VIS) {
      _DrawPolyPoints(hDataObj, xPos + pObj->ScrollStateH.v, yPos + pObj->ScrollStateV.v);
    }
    if (pDataXYObj->Flags & _LINES_VIS) {
      _DrawPolyLine(pData, pDataXYObj->GraphDataObj.NumItems, xPos + pObj->ScrollStateH.v, yPos + pObj->ScrollStateV.v);
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK_H(pData);
  }
  GUI_UNLOCK_H(pDataXYObj);
}

/*********************************************************************
*
*       _OnDelete
*/
static void _OnDelete(GRAPH_DATA_Handle hDataObj) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
  if (pDataXYObj->hData) {
    GUI_ALLOC_Free(pDataXYObj->hData);
    pDataXYObj->hData = 0;
  }
  GUI_UNLOCK_H(pDataXYObj);
  GUI_ALLOC_Free(hDataObj);
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GRAPH_DATA_XY_Create
*/
GRAPH_DATA_Handle GRAPH_DATA_XY_Create(GUI_COLOR Color, unsigned MaxNumItems, GUI_POINT * pItems, unsigned NumItems) {
  GRAPH_DATA_Handle  hDataObj;
  WM_LOCK();
  hDataObj = GUI_ALLOC_AllocZero(sizeof(GRAPH_DATA_XY_OBJ));
  if (hDataObj) {
    GRAPH_DATA_XY_OBJ * pDataXYObj;
    WM_HMEM             hData;
    int                 BufferSize;
    BufferSize = sizeof(GUI_POINT) * MaxNumItems;
    hData = GUI_ALLOC_AllocZero(BufferSize);
    if (hData && pItems && NumItems) {
      void * p;
      p = GUI_LOCK_H(hData);
      GUI_MEMCPY(p, pItems, sizeof(GUI_POINT) * NumItems);
      GUI_UNLOCK_H(p);
    }
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    pDataXYObj->hData     = hData;
    pDataXYObj->PenSize   = 1;
    pDataXYObj->PointSize = GRAPH_DATA_XY_POINTSIZE_DEFAULT;
    pDataXYObj->Flags     = _LINES_VIS;
    pDataXYObj->GraphDataObj.Color               = Color;
    pDataXYObj->GraphDataObj.MaxNumItems         = MaxNumItems;
    pDataXYObj->GraphDataObj.NumItems            = NumItems;
    pDataXYObj->GraphDataObj.PaintObj.pfOnPaint  = _OnPaint;
    pDataXYObj->GraphDataObj.PaintObj.pfOnDelete = _OnDelete;
    GUI_UNLOCK_H(pDataXYObj);
  }
  WM_UNLOCK();
  return hDataObj;
}

/*********************************************************************
*
*       GRAPH_DATA_XY_Delete
*/
void GRAPH_DATA_XY_Delete(GRAPH_DATA_Handle hDataObj) {
  WM_LOCK();
  _OnDelete(hDataObj);
  WM_UNLOCK();
}

/*********************************************************************
*
*       GRAPH_DATA_XY_AddPoint
*/
void GRAPH_DATA_XY_AddPoint(GRAPH_DATA_Handle hDataObj, GUI_POINT * pPoint) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    GUI_POINT * pData;
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    pData = (GUI_POINT *)GUI_LOCK_H(pDataXYObj->hData);
    GRAPH__AddValue(&pDataXYObj->GraphDataObj, pData, pPoint, sizeof(GUI_POINT));
    GUI_UNLOCK_H(pData);
    GUI_UNLOCK_H(pDataXYObj);
  }
  WM_UNLOCK();
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetOffX
*/
void GRAPH_DATA_XY_SetOffX(GRAPH_DATA_Handle hDataObj, int Off) {
  GRAPH_DATA_OBJ * pDataObj;
  if (hDataObj) {
    WM_LOCK();
    pDataObj = (GRAPH_DATA_OBJ *)GUI_LOCK_H(hDataObj);
    if (pDataObj->OffX != Off) {
      pDataObj->OffX = Off;
      WM_Invalidate(pDataObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pDataObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetOffY
*/
void GRAPH_DATA_XY_SetOffY(GRAPH_DATA_Handle hDataObj, int Off) {
  GRAPH_DATA_OBJ * pDataObj;
  if (hDataObj) {
    WM_LOCK();
    pDataObj = (GRAPH_DATA_OBJ *)GUI_LOCK_H(hDataObj);
    if (pDataObj->OffY != Off) {
      pDataObj->OffY = Off;
      WM_Invalidate(pDataObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pDataObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetPenSize
*/
void GRAPH_DATA_XY_SetPenSize(GRAPH_DATA_Handle hDataObj, U8 PenSize) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    if (pDataXYObj->PenSize != PenSize) {
      pDataXYObj->PenSize = PenSize;
      WM_InvalidateWindow(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetLineStyle
*/
void GRAPH_DATA_XY_SetLineStyle(GRAPH_DATA_Handle hDataObj, U8 LineStyle) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    if (pDataXYObj->LineStyle != LineStyle) {
      pDataXYObj->LineStyle = LineStyle;
      WM_InvalidateWindow(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       GRAPH_DATA_XY_GetLineVis
*/
unsigned  GRAPH_DATA_XY_GetLineVis(GRAPH_DATA_Handle hDataObj) {
  unsigned Old = 0;
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    Old = (pDataXYObj->Flags & _LINES_VIS) ? 1 : 0;
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
  return Old;
}

/*********************************************************************
*
*       GRAPH_DATA_XY_GetPointVis
*/
unsigned  GRAPH_DATA_XY_GetPointVis(GRAPH_DATA_Handle hDataObj) {
  unsigned Old = 0;
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    Old = (pDataXYObj->Flags & _POINTS_VIS) ? 1 : 0;
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
  return Old;
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetLineVis
*/
unsigned  GRAPH_DATA_XY_SetLineVis(GRAPH_DATA_Handle hDataObj, unsigned OnOff) {
  unsigned Old = 0;
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    Old = (pDataXYObj->Flags & _LINES_VIS) ? 1 : 0;
    if        ((OnOff == 1) && ((pDataXYObj->Flags & _LINES_VIS) == 0)) {
      pDataXYObj->Flags |= _LINES_VIS;
      WM_Invalidate(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    } else if ((OnOff == 0) && ((pDataXYObj->Flags & _LINES_VIS) != 0)) {
      pDataXYObj->Flags &= ~_LINES_VIS;
      WM_Invalidate(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
  return Old;
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetPointVis
*/
unsigned  GRAPH_DATA_XY_SetPointVis(GRAPH_DATA_Handle hDataObj, unsigned OnOff) {
  unsigned Old = 0;
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    Old = (pDataXYObj->Flags & _POINTS_VIS) ? 1 : 0;
    if        ((OnOff == 1) && ((pDataXYObj->Flags & _POINTS_VIS) == 0)) {
      pDataXYObj->Flags |= _POINTS_VIS;
      WM_Invalidate(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    } else if ((OnOff == 0) && ((pDataXYObj->Flags & _POINTS_VIS) != 0)) {
      pDataXYObj->Flags &= ~_POINTS_VIS;
      WM_Invalidate(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
  return Old;
}

/*********************************************************************
*
*       GRAPH_DATA_XY_Clear
*/
void GRAPH_DATA_XY_Clear(GRAPH_DATA_Handle hDataObj) {
  GRAPH_DATA_OBJ * pDataObj;
  if (hDataObj) {
    WM_LOCK();
    pDataObj = (GRAPH_DATA_OBJ *)GUI_LOCK_H(hDataObj);
    pDataObj->NumItems = 0;
    WM_Invalidate(pDataObj->PaintObj.hGraph);
    GUI_UNLOCK_H(pDataObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetOwnerDraw
*/
void GRAPH_DATA_XY_SetOwnerDraw(GRAPH_DATA_Handle hDataObj, WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    pDataXYObj->pfOwnerDraw = pfOwnerDraw;
    WM_Invalidate(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       GRAPH_DATA_XY_SetPointSize
*/
void GRAPH_DATA_XY_SetPointSize(GRAPH_DATA_Handle hDataObj, unsigned PointSize) {
  GRAPH_DATA_XY_OBJ * pDataXYObj;
  if (hDataObj) {
    WM_LOCK();
    pDataXYObj = (GRAPH_DATA_XY_OBJ *)GUI_LOCK_H(hDataObj);
    pDataXYObj->PointSize = PointSize;
    WM_Invalidate(pDataXYObj->GraphDataObj.PaintObj.hGraph);
    GUI_UNLOCK_H(pDataXYObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void GRAPH_DATA_XY_C(void);
  void GRAPH_DATA_XY_C(void) {}
#endif  /* #if GUI_WINSUPPORT */
