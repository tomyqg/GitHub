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
File        : DROPDOWN_SkinClassic.c
Purpose     : Implementation of DROPDOWN widget
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"
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
*       _DrawTriangleDown
*/
static void _DrawTriangleDown(int x, int y, int Size) {
  for (; Size >= 0; Size--, y++ ) {
    GUI_DrawHLine(y, x - Size, x + Size);
  }
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(DROPDOWN_Handle hObj) {
  int Border;
  GUI_RECT r;
  const char * s;
  int InnerSize, ColorIndex;
  DROPDOWN_Obj * pObj;
  int TextBorderSize;

  //
  // Do some initial calculations
  //
  pObj = DROPDOWN_LOCK_H(hObj);
  Border = pObj->Widget.pEffect->EffectSize;
  TextBorderSize = pObj->Props.TextBorderSize;
  GUI_SetFont(pObj->Props.pFont);
  ColorIndex = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? 2 : 1;
  WM_GetClientRect(&r);
  GUI__ReduceRect(&r, &r, Border);
  InnerSize = r.y1 - r.y0 + 1;
  //
  // Draw the 3D effect (if configured)
  //
  WIDGET__EFFECT_DrawDown(&pObj->Widget);
  //
  // Draw the outer text frames
  //
  r.x1 -= InnerSize;     // Spare square area to the right
  LCD_SetColor(pObj->Props.aBackColor[ColorIndex]);
  //
  // Draw the text
  //
  LCD_SetBkColor(pObj->Props.aBackColor[ColorIndex]);
  GUI_FillRectEx(&r);
  r.x0 += TextBorderSize;
  r.x1 -= TextBorderSize;
  LCD_SetColor(pObj->Props.aTextColor[ColorIndex]);
  s = DROPDOWN__GetpItemLocked(hObj, pObj->Sel);
  GUI_DispStringInRect(s, &r, pObj->Props.Align);
  if (s) {
    GUI_UNLOCK_H(s);
  }
  //
  // Draw arrow
  //
  WM_GetClientRect(&r);
  GUI__ReduceRect(&r, &r, Border);
  r.x0 = r.x1 + 1 - InnerSize;
  LCD_SetColor(pObj->Props.aColor[DROPDOWN_CI_BUTTON]);
  GUI_FillRectEx(&r);
  LCD_SetColor(pObj->Props.aColor[DROPDOWN_CI_ARROW]);
  _DrawTriangleDown((r.x1 + r.x0) / 2, r.y0 + 5, (r.y1 - r.y0 - 8) / 2);
  WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &r);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(DROPDOWN_Handle hObj) {
  // Nothing special to do...
  GUI_USE_PARA(hObj);
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN__SkinClassic
*/
const WIDGET_SKIN DROPDOWN__SkinClassic = {
  _Paint,
  _Create,
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_SetSkinClassic
*/
void DROPDOWN_SetSkinClassic(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj * pObj;

  pObj = DROPDOWN_LOCK_H(hObj);
  pObj->pWidgetSkin = &DROPDOWN__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       DROPDOWN_SetDefaultSkinClassic
*/
void DROPDOWN_SetDefaultSkinClassic(void) {
  DROPDOWN__pSkinDefault = &DROPDOWN__SkinClassic;
}


#else                            /* Avoid empty object files */
  void DROPDOWN_SkinClassic_C(void);
  void DROPDOWN_SkinClassic_C(void) {}
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
