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
File        : GRAPH_SCALE.c
Purpose     : Implementation of GRAPH_SCALE object
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GRAPH_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef   GRAPH_SCALE_TEXTCOLOR_DEFAULT
  #define GRAPH_SCALE_TEXTCOLOR_DEFAULT GUI_WHITE
#endif
#ifndef   GRAPH_SCALE_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define GRAPH_SCALE_FONT_DEFAULT &GUI_Font6x8
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define GRAPH_SCALE_FONT_DEFAULT &GUI_Font8x12_ASCII
  #elif WIDGET_USE_SCHEME_LARGE
    #define GRAPH_SCALE_FONT_DEFAULT &GUI_Font8x16
  #endif
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
GRAPH_SCALE_PROPS _Props = {
  GRAPH_SCALE_TEXTCOLOR_DEFAULT,
  GRAPH_SCALE_FONT_DEFAULT,
};

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _OnPaintVert
*/
static void _OnPaintVert(GRAPH_SCALE_OBJ * pScaleObj, GUI_RECT * pRectInvalid) {
  int BorderT, BorderB, EffectSize;
  GRAPH_OBJ * pObj;
  int xPos, xPosScreen, y0, y1, i, yStart, FontSizeY, TextOffY, Skip;
  GUI_RECT Rect;
  if (!pScaleObj->TickDist) {
    return;
  }
  pObj = GRAPH_LOCK_H(pScaleObj->PaintObj.hGraph);
  EffectSize = pObj->Widget.pEffect->EffectSize;
  xPos       = pScaleObj->Pos + EffectSize;
  xPosScreen = xPos + WM_GetOrgX();
  Skip = 0;
  switch (pScaleObj->TextAlign) {
  case GUI_TA_RIGHT:
    if (pRectInvalid->x0 > xPosScreen) {
      Skip = 1;
    }
    break;
  case GUI_TA_LEFT:
    if (pRectInvalid->x1 < xPosScreen) {
      Skip = 1;
    }
    break;
  }
  if (Skip == 0) {
    WM_GetInsideRectExScrollbar(pScaleObj->PaintObj.hGraph, &Rect);
    BorderT = pObj->Props.BorderT;
    BorderB = pObj->Props.BorderB;
    FontSizeY = GUI_GetFontSizeY();
    TextOffY  = FontSizeY / 2;
    y0     = Rect.y0 + BorderT;
    y1     = Rect.y1 - BorderB;
    yStart = y1 - pScaleObj->Off + pObj->ScrollStateV.v;
    while (yStart < y1) {
      yStart += pScaleObj->TickDist;
    }
    for (i = yStart; i >= y0; i -= pScaleObj->TickDist) {
      if ((i >= y0) && (i <= y1)) {
        float Value;
        GUI_SetTextAlign(pScaleObj->TextAlign);
        Value = (y1 - i - pScaleObj->Off + pObj->ScrollStateV.v) * pScaleObj->Factor;
        GUI_GotoXY(xPos, i - TextOffY);
        GUI_DispFloatMin(Value, pScaleObj->NumDecs);
      }
    }
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnPaintHorz
*/
static void _OnPaintHorz(GRAPH_SCALE_OBJ * pScaleObj, GUI_RECT * pRectInvalid) {
  int BorderL, BorderR, EffectSize;
  GRAPH_OBJ * pObj;
  int yPos, yPosScreen, x0, x1, i, xStart, FontSizeY, xAdd, Skip;
  GUI_RECT Rect;
  if (!pScaleObj->TickDist) {
    return;
  }
  pObj = GRAPH_LOCK_H(pScaleObj->PaintObj.hGraph);
  EffectSize = pObj->Widget.pEffect->EffectSize;
  FontSizeY  = GUI_GetFontSizeY();
  yPos       = pScaleObj->Pos - EffectSize;
  yPosScreen = yPos + WM_GetOrgY();
  Skip = 0;
  if (yPosScreen > pRectInvalid->y1) {
    Skip = 1;
  }
  if ((yPosScreen + FontSizeY) < pRectInvalid->y0) {
    Skip = 1;
  }
  if (Skip == 0) {
    WM_GetInsideRectExScrollbar(pScaleObj->PaintObj.hGraph, &Rect);
    BorderL = pObj->Props.BorderL;
    BorderR = pObj->Props.BorderR;
    x0     = Rect.x0 + BorderL;
    x1     = Rect.x1 - BorderR;
    xAdd   = (pObj->Flags & GRAPH_CF_GRID_FIXED_X) ? 0 : pObj->ScrollStateH.v;
    xStart = x0 + pScaleObj->Off + xAdd;
    while (xStart > x0) {
      xStart -= pScaleObj->TickDist;
    }
    for (i = xStart; i <= x1; i += pScaleObj->TickDist) {
      if ((i > x0) && (i <= x1)) {
        float Value;
        GUI_SetTextAlign(pScaleObj->TextAlign);
        Value = (i - pScaleObj->Off - BorderL - EffectSize - xAdd) * pScaleObj->Factor;
        GUI_GotoXY(i, yPos);
        GUI_DispFloatMin(Value, pScaleObj->NumDecs);
      }
    }
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnPaint
*/
static void _OnPaint(WM_HMEM hScaleObj, GUI_RECT * pRectInvalid) {
  GRAPH_SCALE_OBJ * pScaleObj;
  pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
  GUI_SetFont(pScaleObj->Props.pFont);
  GUI_SetColor(pScaleObj->Props.TextColor);
  GUI_SetTextMode(GUI_TM_TRANS);
  if (pScaleObj->Flags & GRAPH_SCALE_CF_VERTICAL) {
    _OnPaintVert(pScaleObj, pRectInvalid);
  } else {
    _OnPaintHorz(pScaleObj, pRectInvalid);
  }
  GUI_UNLOCK_H(pScaleObj);
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GRAPH_SCALE_Create
*/
GRAPH_SCALE_Handle GRAPH_SCALE_Create(int Pos, int TextAlign, unsigned Flags, unsigned TickDist) {
  GRAPH_SCALE_Handle hScaleObj;
  WM_LOCK();
  hScaleObj = GUI_ALLOC_AllocZero(sizeof(GRAPH_SCALE_OBJ));
  if (hScaleObj) {
    GRAPH_SCALE_OBJ * pScaleObj;
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    pScaleObj->Pos        = Pos;
    pScaleObj->TextAlign  = TextAlign;
    pScaleObj->Flags      = Flags;
    pScaleObj->TickDist   = TickDist;
    pScaleObj->Factor     = 1;
    pScaleObj->Props      = _Props;
    pScaleObj->PaintObj.pfOnPaint = _OnPaint;
    GUI_UNLOCK_H(pScaleObj);
  }
  WM_UNLOCK();
  return hScaleObj;
}

/*********************************************************************
*
*       GRAPH_SCALE_Delete
*/
void GRAPH_SCALE_Delete(GRAPH_SCALE_Handle hScaleObj) {
  GUI_ALLOC_Free(hScaleObj);
}

/*********************************************************************
*
*       GRAPH_SCALE_SetOff
*/
int GRAPH_SCALE_SetOff(GRAPH_SCALE_Handle hScaleObj, int Off) {
  int OldValue = 0;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    OldValue = pScaleObj->Off;
    if (pScaleObj->Off != Off) {
      pScaleObj->Off = Off;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return OldValue;
}

/*********************************************************************
*
*       GRAPH_SCALE_SetTickDist
*/
unsigned GRAPH_SCALE_SetTickDist(GRAPH_SCALE_Handle hScaleObj, unsigned Value) {
  unsigned OldValue = 0;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    OldValue = pScaleObj->TickDist;
    if (pScaleObj->TickDist != Value) {
      pScaleObj->TickDist = Value;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return OldValue;
}

/*********************************************************************
*
*       GRAPH_SCALE_SetFont
*/
const GUI_FONT GUI_UNI_PTR * GRAPH_SCALE_SetFont(GRAPH_SCALE_Handle hScaleObj, const GUI_FONT GUI_UNI_PTR * pFont) {
  const GUI_FONT GUI_UNI_PTR * pOldFont = NULL;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    pOldFont = pScaleObj->Props.pFont;
    if (pScaleObj->Props.pFont != pFont) {
      pScaleObj->Props.pFont = pFont;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return pOldFont;
}

/*********************************************************************
*
*       GRAPH_SCALE_SetTextColor
*/
GUI_COLOR GRAPH_SCALE_SetTextColor(GRAPH_SCALE_Handle hScaleObj, GUI_COLOR Color) {
  GUI_COLOR OldColor = 0;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    OldColor = pScaleObj->Props.TextColor;
    if (pScaleObj->Props.TextColor != Color) {
      pScaleObj->Props.TextColor = Color;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return OldColor;
}

/*********************************************************************
*
*       GRAPH_SCALE_SetPos
*/
int GRAPH_SCALE_SetPos(GRAPH_SCALE_Handle hScaleObj, int Pos) {
  int OldPos = 0;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    OldPos = pScaleObj->Pos;
    if (pScaleObj->Pos != Pos) {
      pScaleObj->Pos = Pos;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return OldPos;
}

/*********************************************************************
*
*       GRAPH_SCALE_SetFactor
*/
float GRAPH_SCALE_SetFactor(GRAPH_SCALE_Handle hScaleObj, float Factor) {
  float OldFactor = 0;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    OldFactor = pScaleObj->Factor;
    if (pScaleObj->Factor != Factor) {
      pScaleObj->Factor = Factor;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return OldFactor;
}

/*********************************************************************
*
*       GRAPH_SCALE_SetNumDecs
*/
int GRAPH_SCALE_SetNumDecs(GRAPH_SCALE_Handle hScaleObj, int NumDecs) {
  int OldValue = 0;
  GRAPH_SCALE_OBJ * pScaleObj;
  if (hScaleObj) {
    WM_LOCK();
    pScaleObj = (GRAPH_SCALE_OBJ *)GUI_LOCK_H(hScaleObj);
    OldValue = pScaleObj->NumDecs;
    if (pScaleObj->NumDecs != NumDecs) {
      pScaleObj->NumDecs = NumDecs;
      WM_InvalidateWindow(pScaleObj->PaintObj.hGraph);
    }
    GUI_UNLOCK_H(pScaleObj);
    WM_UNLOCK();
  }
  return OldValue;
}

#else                            /* Avoid problems with empty object modules */
  void GRAPH_SCALE_C(void);
  void GRAPH_SCALE_C(void) {}
#endif  /* #if GUI_WINSUPPORT */
