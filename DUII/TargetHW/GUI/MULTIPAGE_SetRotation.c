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
File        : MULTIPAGE_SetRotation.c
Purpose     : Implementation of MULTIPAGE widget
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "MULTIPAGE_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawTextItemV
*/
static void _DrawTextItemV(MULTIPAGE_Obj* pObj, const char* pText, unsigned Index,
                          const GUI_RECT* pRect, int y0, int w, int ColorIndex) {
  GUI_RECT r;
  int Len;
  r = *pRect;
  r.y0 += y0;
  r.y1  = r.y0 + w;
  pObj->Widget.pEffect->pfDrawUpRect(&r);
  GUI__ReduceRect(&r, &r, pObj->Widget.pEffect->EffectSize);
  if (pObj->Selection == Index) {
    if (pObj->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
      r.x1 += pObj->Widget.pEffect->EffectSize + 1;
	    if (pObj->Widget.pEffect->EffectSize > 1) {
		    LCD_SetColor(MULTIPAGE__aEffectColor[0]);
		    GUI_DrawHLine(r.y0 - 1, r.x1 - 2, r.x1 - 1);
		    LCD_SetColor(MULTIPAGE__aEffectColor[1]);
		    GUI_DrawHLine(r.y1 + 1, r.x1 - 2, r.x1 - 1);
	    }
    } else {
      r.x0 -= pObj->Widget.pEffect->EffectSize + 1;
	    if (pObj->Widget.pEffect->EffectSize > 1) {
		    LCD_SetColor(MULTIPAGE__aEffectColor[0]);
		    GUI_DrawHLine(r.y0 - 1, r.x0, r.x0 + 1);
		    LCD_SetColor(MULTIPAGE__aEffectColor[1]);
		    GUI_DrawHLine(r.y1 + 1, r.x0, r.x0 + 1);
	    }
    }
  }
  LCD_SetColor(pObj->Props.aBkColor[ColorIndex]);
  GUI_FillRectEx(&r);
  LCD_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
  LCD_SetColor(pObj->Props.aTextColor[ColorIndex]);
  r.y0 += 4;
  r.x1 -= 1;
  if ((pObj->Selection == Index) && (pObj->Props.Align & MULTIPAGE_ALIGN_BOTTOM)) {
    r.x1 -= pObj->Widget.pEffect->EffectSize + 1;
  }
  Len = strlen(pText);
  #if GUI_SUPPORT_ROTATION
    GUI_DispStringInRectEx(pText, &r, GUI_TA_LEFT, Len, GUI_ROTATE_CW);
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
*       MULTIPAGE_SetRotation
*/
void MULTIPAGE_SetRotation(MULTIPAGE_Handle hObj, unsigned Rotation) {
  MULTIPAGE_Obj* pObj;
  GUI_RECT rClient;
  if (hObj) {
    WM_LOCK();
    pObj = MULTIPAGE_LOCK_H(hObj);
    Rotation &= MULTIPAGE_CF_ROTATE_CW;
    if (( (pObj->Widget.State & WIDGET_STATE_VERTICAL) && (Rotation == 0)) ||
        (!(pObj->Widget.State & WIDGET_STATE_VERTICAL) && (Rotation == MULTIPAGE_CF_ROTATE_CW))) {
      if (Rotation == MULTIPAGE_CF_ROTATE_CW) {
        pObj->Widget.State |= WIDGET_STATE_VERTICAL;
        pObj->pfDrawTextItem = _DrawTextItemV;
      } else {
        pObj->Widget.State &= ~WIDGET_STATE_VERTICAL;
        pObj->pfDrawTextItem = MULTIPAGE__DrawTextItemH;
      }
    }
    MULTIPAGE__DeleteScrollbar(hObj);
    MULTIPAGE__CalcClientRect(hObj, &rClient);
    WM_MoveTo(pObj->hClient, rClient.x0 + pObj->Widget.Win.Rect.x0,
                             rClient.y0 + pObj->Widget.Win.Rect.y0);
    GUI_UNLOCK_H(pObj);
    MULTIPAGE__UpdatePositions(hObj);
    WM_UNLOCK();
  }
}

#else
  void MULTIPAGE_SetRotation_C(void);
  void MULTIPAGE_SetRotation_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/

