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
File        : HEADER_SkinClassic.c
Purpose     : Implementation of header widget
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawTriangle
*/
static void _DrawTriangle(int x, int y, int Size, int Inc) {
  for (; Size >= 0; Size--, y += Inc) {
    GUI_DrawHLine(y, x - Size, x + Size);
  }
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(HEADER_Handle hObj) {
  HEADER_Obj * pObj;
  GUI_RECT Rect;
  GUI_RECT RectItem;
  GUI_RECT RectNonFixedArea;
  unsigned i;
  int xPos;
  unsigned NumItems;
  int EffectSize;
  int ArrowSize;
  int ArrowPos;
  WM_HMEM hDrawObj;

  pObj = HEADER_LOCK_H(hObj);
  xPos = -pObj->ScrollPos;
  NumItems = GUI_ARRAY_GetNumItems(pObj->Columns);
  EffectSize = pObj->Widget.pEffect->EffectSize;
  LCD_SetBkColor(pObj->Props.BkColor);
  GUI_SetFont(pObj->Props.pFont);
  GUI_Clear();
  GUI_GetClientRect(&RectNonFixedArea);
  for (i = 0; i < NumItems; i++) {
    int Subtract = 0;
    HEADER_COLUMN * pColumn;
    pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, i);
    GUI_GetClientRect(&Rect);
    xPos += (i < pObj->Fixed) ? pObj->ScrollPos : 0;
    Rect.x0 = xPos;
    Rect.x1 = Rect.x0 + pColumn->Width;
    RectItem = Rect;
    ArrowSize = ((RectItem.y1 - RectItem.y0 - EffectSize * 2) / 3) - 1;
    ArrowPos = RectItem.x1 - 4 - ArrowSize;
    WM_SetUserClipRect(&RectNonFixedArea);
    if (pColumn->hDrawObj) {
      int xOff = 0, yOff = 0;
      switch (pColumn->Align & GUI_TA_HORIZONTAL) {
      case GUI_TA_RIGHT:
        xOff = (pColumn->Width - GUI_DRAW__GetXSize(pColumn->hDrawObj));
        break;
      case GUI_TA_HCENTER:
        xOff = (pColumn->Width - GUI_DRAW__GetXSize(pColumn->hDrawObj)) / 2;
        break;
      }
      switch (pColumn->Align & GUI_TA_VERTICAL) {
	    case GUI_TA_BOTTOM:
        yOff = ((Rect.y1 - Rect.y0 + 1) - GUI_DRAW__GetYSize(pColumn->hDrawObj));
        break;
	    case GUI_TA_VCENTER:
        yOff = ((Rect.y1 - Rect.y0 + 1) - GUI_DRAW__GetYSize(pColumn->hDrawObj)) / 2;
        break;
      }
      WM_SetUserClipRect(&Rect);
      hDrawObj = pColumn->hDrawObj;
      GUI_UNLOCK_H(pColumn);
      GUI_UNLOCK_H(pObj);
      GUI_DRAW__Draw(hDrawObj, hObj, xPos + xOff, yOff);
      pObj = HEADER_LOCK_H(hObj);
      pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, i);
      WM_SetUserClipRect(NULL);
    }
    WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &Rect);
    xPos += Rect.x1 - Rect.x0;
    Rect.x0 += EffectSize + HEADER__DefaultBorderH;
    Rect.x1 -= EffectSize + HEADER__DefaultBorderH;
    Rect.y0 += EffectSize + HEADER__DefaultBorderV;
    Rect.y1 -= EffectSize + HEADER__DefaultBorderV;
    LCD_SetColor(pObj->Props.TextColor);
    if ((pObj->DirIndicatorColumn == (int)i) && ((pColumn->Align & GUI_TA_HORIZONTAL) == GUI_TA_RIGHT)) {
      Subtract = (ArrowSize << 1) + 1;
    }
    Rect.x1 -= Subtract;
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_DispStringInRect(pColumn->acText, &Rect, pColumn->Align);
    Rect.x1 += Subtract;
    if (pObj->DirIndicatorColumn == (int)i) {
      LCD_SetColor(pObj->Props.ArrowColor);
      WM_SetUserClipRect(&RectNonFixedArea);
      if (pObj->DirIndicatorReverse == 0) {
        _DrawTriangle(ArrowPos, ((Rect.y1 - Rect.y0) >> 1), ArrowSize, 1);
      } else {
        _DrawTriangle(ArrowPos, ((Rect.y1 - Rect.y0) >> 1) + ArrowSize, ArrowSize, -1);
      }
      WM_SetUserClipRect(NULL);
    }
    if (i < pObj->Fixed) {
      RectNonFixedArea.x0 = RectItem.x1 + 1;
      xPos -= pObj->ScrollPos;
    }
    GUI_UNLOCK_H(pColumn);
  }
  WM_SetUserClipRect(NULL);
  GUI_GetClientRect(&Rect);
  Rect.x0 = xPos;
  Rect.x1 = 0xfff;
  WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &Rect);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(HEADER_Handle hObj) {
  // Nothing special to do...
  GUI_USE_PARA(hObj);
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
const WIDGET_SKIN HEADER__SkinClassic = {
  _Paint,
  _Create
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetSkinClassic
*/
void HEADER_SetSkinClassic(HEADER_Handle hObj) {
  HEADER_Obj * pObj;

  pObj = HEADER_LOCK_H(hObj);
  pObj->pWidgetSkin = &HEADER__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       HEADER_SetDefaultSkinClassic
*/
void HEADER_SetDefaultSkinClassic(void) {
  HEADER__pSkinDefault = &HEADER__SkinClassic;
}


#else                            /* Avoid empty object files */
  void HEADER_SkinClassic_C(void);
  void HEADER_SkinClassic_C(void) {}
#endif /* GUI_WINSUPPORT */
