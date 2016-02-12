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
File        : PROGBAR_SkinClassic.c
Purpose     : Implementation of PROGBAR widget
---------------------------END-OF-HEADER------------------------------
*/

#include "PROGBAR_Private.h"
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
*       _DrawPart
*/
static void _DrawPart(const PROGBAR_Obj * pObj, int Index,
										  int xText, int yText, const char* pText) {
  LCD_SetBkColor(pObj->Props.aBarColor[Index]);
  LCD_SetColor(pObj->Props.aTextColor[Index]);
  GUI_Clear();
  if (pText) {
    GUI_GotoXY(xText, yText);
    GUI_DispString(pText);
  }
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(PROGBAR_Handle hObj) {
  PROGBAR_Obj * pObj;
  GUI_RECT r, rInside, rClient, rText;
  const char* pText;
  int tm, Pos;
  
  pObj = PROGBAR_LOCK_H(hObj);
  WM_GetClientRect(&rClient);
  GUI__ReduceRect(&rInside, &rClient, pObj->Widget.pEffect->EffectSize);
  Pos  = PROGBAR__Value2Pos(pObj, pObj->v);
  if (pObj->Flags & PROGBAR_CF_VERTICAL) {
    //
    // Draw lower part
    //
    r    = rInside;
    r.y0 = Pos;
    WM_SetUserClipArea(&r);
    _DrawPart(pObj, 0, 0, 0, 0);
    //
    // Draw upper part
    //
    r    = rInside;
    r.y1 = Pos - 1;
    WM_SetUserClipArea(&r);
    _DrawPart(pObj, 1, 0, 0, 0);
  } else {
    pText = PROGBAR__GetTextLocked(pObj);
    if (pText) {
      GUI_SetFont(pObj->Props.pFont);
      PROGBAR__GetTextRect(pObj, &rText, pText);
    }
    tm = GUI_SetTextMode(GUI_TM_TRANS);
    //
    // Draw left bar
    //
    r    = rInside;
    r.x1 = Pos - 1;
    WM_SetUserClipArea(&r);
    _DrawPart(pObj, 0, rText.x0, rText.y0, pText);
    //
    // Draw right bar
    //
    r    = rInside;
    r.x0 = Pos;
    WM_SetUserClipArea(&r);
    _DrawPart(pObj, 1, rText.x0, rText.y0, pText);
    GUI_SetTextMode(tm);
    GUI_UNLOCK_H(pText);
  }
  WM_SetUserClipArea(NULL);
  WIDGET__EFFECT_DrawDownRect(&pObj->Widget, &rClient);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(PROGBAR_Handle hObj) {
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
*       PROGBAR__SkinClassic
*/
const WIDGET_SKIN PROGBAR__SkinClassic = {
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
*       PROGBAR_SetSkinClassic
*/
void PROGBAR_SetSkinClassic(PROGBAR_Handle hObj) {
  PROGBAR_Obj * pObj;

  pObj = PROGBAR_LOCK_H(hObj);
  pObj->pWidgetSkin = &PROGBAR__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       PROGBAR_SetDefaultSkinClassic
*/
void PROGBAR_SetDefaultSkinClassic(void) {
  PROGBAR__pSkinDefault = &PROGBAR__SkinClassic;
}


#else                            /* Avoid empty object files */
  void PROGBAR_SkinClassic_C(void);
  void PROGBAR_SkinClassic_C(void) {}
#endif /* GUI_WINSUPPORT */
