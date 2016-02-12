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
File        : SLIDER_SkinClassic.c
Purpose     : Implementation of scrollbar widget
---------------------------END-OF-HEADER------------------------------
*/

#include "SLIDER_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Static code
*
**********************************************************************
*/

// KMC
extern GUI_CONST_STORAGE GUI_BITMAP bmSliderDotTransparent;

/*********************************************************************
*
*       _Paint
*/
static void _Paint(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;
  GUI_RECT r, rFocus, rSlider, rSlot;
  int x, x0, xsize, i, Range, NumTicks;

  pObj = SLIDER_LOCK_H(hObj);
  WIDGET__GetClientRect(&pObj->Widget, &rFocus);
  GUI__ReduceRect(&r, &rFocus, 1);
  NumTicks = pObj->NumTicks;
  xsize    = r.x1 - r.x0  + 1 - pObj->Width;
  x0       = r.x0 + pObj->Width / 2;
  Range    = pObj->Max - pObj->Min;
  if (Range == 0) {
    Range = 1;
  }
  //
  // Fill with parents background color
  //
  #if !SLIDER_SUPPORT_TRANSPARENCY   // Not needed any more, since window is transparent
    if (pObj->Props.BkColor == GUI_INVALID_COLOR) {
      LCD_SetBkColor(WIDGET__GetBkColor(hObj));
    } else {
      LCD_SetBkColor(pObj->Props.BkColor);
    }
    GUI_Clear();
  #else
    if (!WM_GetHasTrans(hObj)) {
      LCD_SetBkColor(pObj->Props.BkColor);
      GUI_Clear();
    }
  #endif
  //
  // Calculate Slider position
  //
  rSlider    = r;
  rSlider.y0 = 5;
  rSlider.x0 = x0 + (U32)xsize * (U32)(pObj->v - pObj->Min) / Range - pObj->Width / 2;
  rSlider.x1 = rSlider.x0 + pObj->Width;
  //
  // Calculate Slot position
  //
  rSlot.x0 = x0;
  rSlot.x1 = x0 + xsize;
  rSlot.y0 = (rSlider.y0 + rSlider.y1) / 2 - 1;
  rSlot.y1 = rSlot.y0 + 3;
  WIDGET__EFFECT_DrawDownRect(&pObj->Widget, &rSlot);        // Draw slot
  //
  // Draw the ticks
  //
  if (NumTicks < 0) {
    NumTicks = Range + 1;
    if (NumTicks > (xsize / 5)) {
      NumTicks = 11;
    }
  }
  if (NumTicks > 1) {
    LCD_SetColor(pObj->Props.TickColor);
    for (i = 0; i < NumTicks; i++) {
      x = x0 + xsize * i / (NumTicks - 1);
      WIDGET__DrawVLine(&pObj->Widget, x, 1, 3);
    }
  }
  //
  // Draw the slider itself
  //
  LCD_SetColor(pObj->Props.BarColor);

// KMC - start
  //WIDGET__FillRectEx(&pObj->Widget, &rSlider);
  //WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &rSlider);
  ////
  //// Draw focus
  ////
  //if (pObj->Widget.State & WIDGET_STATE_FOCUS) {
  //  LCD_SetColor(pObj->Props.FocusColor);
  //  WIDGET__DrawFocusRect(&pObj->Widget, &rFocus, 0);
  //}

  GUI_DrawBitmap(&bmSliderDotTransparent, rSlider.x0 - 11, 0);
// KMC - end

  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(SLIDER_Handle hObj) {
  // Nothing special to do
  GUI_USE_PARA(hObj);
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
const WIDGET_SKIN SLIDER__SkinClassic = {
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
*       SLIDER_SetSkinClassic
*/
void SLIDER_SetSkinClassic(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;

  pObj = SLIDER_LOCK_H(hObj);
  pObj->pWidgetSkin = &SLIDER__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       SLIDER_SetDefaultSkinClassic
*/
void SLIDER_SetDefaultSkinClassic(void) {
  SLIDER__pSkinDefault = &SLIDER__SkinClassic;
}

#else
  void SLIDER_SkinClassic_C(void);
  void SLIDER_SkinClassic_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
