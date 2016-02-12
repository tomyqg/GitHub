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
File        : BUTTON_SkinClassic.c
Purpose     : Implementation of button widget
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON_Private.h"
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
*       _Paint
*/
static void _Paint(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;
  int EffectSize;
  const char * s;
  unsigned int Index;
  int State, PressedState, ColorIndex;
  GUI_RECT RectClient, RectInside, RectText;
  WM_HMEM hDrawObj;
  
  pObj = BUTTON_LOCK_H(hObj);
  State = pObj->Widget.State;
  PressedState = (State & BUTTON_STATE_PRESSED) ? 1 : 0;
  ColorIndex   = (WM__IsEnabled(hObj)) ? PressedState : 2;
  GUI_SetFont(pObj->Props.pFont);
  GUI_DEBUG_LOG("BUTTON: Paint(..)\n");
  GUI_GetClientRect(&RectClient);
  //
  // Start drawing
  //
  RectInside = RectClient;
  //
  // Draw the 3D effect (if configured)
  //
  if ((PressedState) == 0) {
    pObj->Widget.pEffect->pfDrawUp();
    EffectSize = pObj->Widget.pEffect->EffectSize;
  } else {
    if (pObj->Widget.pEffect->pfDrawFlat) {
      pObj->Widget.pEffect->pfDrawFlat();
      EffectSize = pObj->Widget.pEffect->EffectSize;
    } else {
      LCD_SetColor(pObj->Props.FrameColor);
      GUI_DrawRect(RectClient.y0, RectClient.x0, RectClient.x1, RectClient.y1);
      EffectSize = 1;
    }
  }
  GUI__ReduceRect(&RectInside, &RectInside, EffectSize);
  //
  // Draw background
  //
  LCD_SetBkColor (pObj->Props.aBkColor[ColorIndex]);
  LCD_SetColor   (pObj->Props.aTextColor[ColorIndex]);
  WM_SetUserClipRect(&RectInside);
  GUI_Clear();
  //
  // Draw bitmap.
  // If we have only one, we will use it.
  // If we have to we will use the second one (Index 1) for the pressed state
  //
  if (ColorIndex < 2) {
    Index = (pObj->ahDrawObj[BUTTON_BI_PRESSED] && PressedState) ? BUTTON_BI_PRESSED : BUTTON_BI_UNPRESSED;
  } else {
    Index = pObj->ahDrawObj[BUTTON_BI_DISABLED] ? BUTTON_BI_DISABLED : BUTTON_BI_UNPRESSED;
  }
  hDrawObj = pObj->ahDrawObj[Index];
  GUI_UNLOCK_H(pObj);
  GUI_DRAW__Draw(hDrawObj, hObj, 0, 0);
  pObj = BUTTON_LOCK_H(hObj);
  //
  // Draw the actual button (background and text) *
  //
  RectText = RectInside;
  if (PressedState) {
    GUI_MoveRect(&RectText, BUTTON_3D_MOVE_X,BUTTON_3D_MOVE_Y);
  }
  if (pObj->hpText) {
    GUI_SetTextMode(GUI_TM_TRANS);
    s = (const char *)GUI_LOCK_H(pObj->hpText);
    GUI_DispStringInRect(s, &RectText, pObj->Props.Align);
    GUI_UNLOCK_H(s);
  }
//
// KMC - we do not want to draw the focus rectangle, ever
//
  ////
  //// Draw focus
  ////
  //if (State & BUTTON_STATE_FOCUS) {
  //  LCD_SetColor(pObj->Props.FocusColor);
  //  GUI_DrawFocusRect(&RectClient, pObj->Widget.pEffect->EffectSize > 2 ? pObj->Widget.pEffect->EffectSize : 2);
  //}
  WM_SetUserClipRect(NULL);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(BUTTON_Handle hObj) {
  // Nothing special to do...
  GUI_USE_PARA(hObj);
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
const WIDGET_SKIN BUTTON__SkinClassic = {
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
*       BUTTON_SetSkinClassic
*/
void BUTTON_SetSkinClassic(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;

  pObj = BUTTON_LOCK_H(hObj);
  pObj->pWidgetSkin = &BUTTON__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       BUTTON_SetDefaultSkinClassic
*/
void BUTTON_SetDefaultSkinClassic(void) {
  BUTTON__pSkinDefault = &BUTTON__SkinClassic;
}


#else                            /* Avoid empty object files */
  void BUTTON_SkinClassic_C(void);
  void BUTTON_SkinClassic_C(void) {}
#endif /* GUI_WINSUPPORT */
