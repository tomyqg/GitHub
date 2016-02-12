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
File        : CHECKBOX_SkinClassic.c
Purpose     : Implementation of CHECKBOX widget
---------------------------END-OF-HEADER------------------------------
*/

#include "CHECKBOX_Private.h"
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
static void _Paint(CHECKBOX_Handle hObj) {
  CHECKBOX_Obj * pObj;
  GUI_RECT RectBox = {0};
  int ColorIndex, EffectSize, Index;
  const GUI_BITMAP * pBm;

  pObj = CHECKBOX_LOCK_H(hObj);
  EffectSize = pObj->Widget.pEffect->EffectSize;
  ColorIndex = WM__IsEnabled(hObj);
  /* Clear inside ... Just in case      */
  /* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
  if (!WM_GetHasTrans(hObj))
#endif
  {
    if (pObj->Props.BkColor == GUI_INVALID_COLOR) {
      LCD_SetBkColor(WIDGET__GetBkColor(hObj));
    } else {
      LCD_SetBkColor(pObj->Props.BkColor);
    }
    GUI_Clear();
  }
  /* Get size from bitmap */
  RectBox.x1 = pObj->Props.apBm[CHECKBOX_BI_ACTIV]->XSize - 1 + 2 * EffectSize;
  RectBox.y1 = pObj->Props.apBm[CHECKBOX_BI_ACTIV]->YSize - 1 + 2 * EffectSize;
  WM_SetUserClipRect(&RectBox);
  /* Clear inside  ... Just in case */
  LCD_SetBkColor(pObj->Props.aBkColorBox[ColorIndex]);
  GUI_Clear();
  Index = pObj->CurrentState * 2 + ColorIndex;
  pBm = pObj->Props.apBm[Index];
  if (pBm) {
    GUI_UNLOCK_H(pObj);
    GUI_DrawBitmap(pBm, EffectSize, EffectSize);
    pObj = CHECKBOX_LOCK_H(hObj);
  }
  /* Draw the effect around the box */
  WIDGET__EFFECT_DrawDownRect(&pObj->Widget, &RectBox);
  WM_SetUserClipRect(NULL);
  /* Draw text if needed */
  if (pObj->hpText) {
    const char * s;
    GUI_RECT RectText;
    /* Draw the text */
    WM_GetClientRect(&RectText);
    RectText.x0 += RectBox.x1 + 1 + pObj->Props.Spacing;
    GUI_SetTextMode(GUI_TM_TRANS);
    LCD_SetColor(pObj->Props.TextColor);
    GUI_SetFont(pObj->Props.pFont);
    s = (const char *)GUI_LOCK_H(pObj->hpText);
    GUI_DispStringInRect(s, &RectText, pObj->Props.Align);
    /* Draw focus rectangle */
    if (pObj->Widget.State & WIDGET_STATE_FOCUS) {
      int xSizeText = GUI_GetStringDistX(s);
      int ySizeText = GUI_GetFontSizeY();
      GUI_RECT RectFocus = RectText;
      switch (pObj->Props.Align & ~(GUI_TA_HORIZONTAL)) {
      case GUI_TA_VCENTER:
        RectFocus.y0 = (RectText.y1 - ySizeText + 1) / 2;
        break;
      case GUI_TA_BOTTOM:
        RectFocus.y0 = RectText.y1 - ySizeText;
        break;
      }
      switch (pObj->Props.Align & ~(GUI_TA_VERTICAL)) {
      case GUI_TA_HCENTER:
        RectFocus.x0 += ((RectText.x1 - RectText.x0) - xSizeText) / 2;
        break;
      case GUI_TA_RIGHT:
        RectFocus.x0 += (RectText.x1 - RectText.x0) - xSizeText;
        break;
      }
      RectFocus.x1 = RectFocus.x0 + xSizeText - 1;
      RectFocus.y1 = RectFocus.y0 + ySizeText - 1;
      LCD_SetColor(pObj->Props.FocusColor);
      GUI_DrawFocusRect(&RectFocus, -1);
    }
    GUI_UNLOCK_H(s);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(CHECKBOX_Handle hObj) {
  // Nothing special to do...
  GUI_USE_PARA(hObj);
}

/*********************************************************************
*
*       _GetButtonSize
*/
static unsigned _GetButtonSize(void) {
  int EffectSize;

  EffectSize = WIDGET_GetDefaultEffect()->EffectSize;
  return CHECKBOX__DefaultProps.apBm[CHECKBOX_BI_ACTIV]->XSize + 2 * EffectSize;
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       _SkinPrivate
*/
static CHECKBOX_SKIN_PRIVATE _SkinPrivate = {
  _GetButtonSize
};

/*********************************************************************
*
*       CHECKBOX__SkinClassic
*/
const WIDGET_SKIN CHECKBOX__SkinClassic = {
  _Paint,
  _Create,
  &_SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX_SetSkinClassic
*/
void CHECKBOX_SetSkinClassic(CHECKBOX_Handle hObj) {
  CHECKBOX_Obj * pObj;

  pObj = CHECKBOX_LOCK_H(hObj);
  pObj->pWidgetSkin = &CHECKBOX__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       CHECKBOX_SetDefaultSkinClassic
*/
void CHECKBOX_SetDefaultSkinClassic(void) {
  CHECKBOX__pSkinDefault = &CHECKBOX__SkinClassic;
}


#else                            /* Avoid empty object files */
  void CHECKBOX_SkinClassic_C(void);
  void CHECKBOX_SkinClassic_C(void) {}
#endif /* GUI_WINSUPPORT */
