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
File        : RADIO_SkinClassic.c
Purpose     : Implementation of RADIO widget
---------------------------END-OF-HEADER------------------------------
*/

#include "RADIO_Private.h"
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
static void _Paint(RADIO_Handle hObj) {
  RADIO_Obj * pObj;
  const GUI_BITMAP * pBmRadio;
  const GUI_BITMAP * pBmCheck;
  const char * pText;
  GUI_FONTINFO FontInfo;
  GUI_RECT Rect, r, rFocus = {0};
  int i, y, HasFocus, FontDistY, Sel, Height;
  U8 SpaceAbove, CHeight, FocusBorder;

  pObj = RADIO_LOCK_H(hObj);
  Sel = pObj->Sel;
  Height = pObj->Props.apBmRadio[RADIO_BI_ACTIV]->YSize + RADIO_BORDER * 2;
  /* Init some data */
  WIDGET__GetClientRect(&pObj->Widget, &rFocus);
  HasFocus  = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? 1 : 0;
  pBmRadio  = pObj->Props.apBmRadio[WM__IsEnabled(hObj)];
  pBmCheck  = pObj->Props.pBmCheck;
  rFocus.x1 = pBmRadio->XSize + RADIO_BORDER * 2 - 1;
  rFocus.y1 = Height + ((pObj->NumItems - 1) * pObj->Spacing) - 1;

  /* Select font and text color */
  LCD_SetColor(pObj->Props.TextColor);
  GUI_SetFont(pObj->Props.pFont);
  GUI_SetTextMode(GUI_TM_TRANS);

  /* Get font infos */
  GUI_GetFontInfo(pObj->Props.pFont, &FontInfo);
  FontDistY   = GUI_GetFontDistY();
  CHeight     = FontInfo.CHeight;
  SpaceAbove  = FontInfo.Baseline - CHeight;
  Rect.x0     = pBmRadio->XSize + RADIO_BORDER * 2 + 2;
  Rect.y0     = (CHeight <= Height) ? ((Height - CHeight) / 2) : 0;
  Rect.y1     = Rect.y0 + CHeight - 1;
  FocusBorder = (FontDistY <= 12) ? 2 : 3;
  if (Rect.y0 < FocusBorder) {
    FocusBorder = Rect.y0;
  }

  /* Clear inside ... Just in case      */
  /* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
  if (!WM_GetHasTrans(hObj))
#endif
  {
    if (pObj->Props.BkColor != GUI_INVALID_COLOR) {
      LCD_SetBkColor(pObj->Props.BkColor);
    } else {
      LCD_SetBkColor(RADIO_DEFAULT_BKCOLOR);
    }
    GUI_Clear();
  }

  /* Iterate over all items */
  for (i = 0; i < pObj->NumItems; i++) {
    y = i * pObj->Spacing;
    /* Draw the radio button bitmap */
    GUI_UNLOCK_H(pObj);
    GUI_DrawBitmap(pBmRadio, RADIO_BORDER, RADIO_BORDER + y);
    /* Draw the check bitmap */
    if (Sel == i) {
      GUI_DrawBitmap(pBmCheck, RADIO_BORDER +  (pBmRadio->XSize - pBmCheck->XSize) / 2, 
                               RADIO_BORDER + ((pBmRadio->YSize - pBmCheck->YSize) / 2) + y);
    }
    pObj = RADIO_LOCK_H(hObj);
    /* Draw text if available */
    pText = (const char *)GUI_ARRAY_GetpItemLocked(pObj->TextArray, i);
    if (pText) {
      if (*pText) {
        r = Rect;
        r.x1 = r.x0 + GUI_GetStringDistX(pText) - 2;
        GUI_MoveRect(&r, 0, y);
        GUI_DispStringAt(pText, r.x0, r.y0 - SpaceAbove);
        /* Calculate focus rect */
        if (HasFocus && ((pObj->Sel == i) || (pObj->GroupId))) {
          GUI__ReduceRect(&rFocus, &r, -FocusBorder);
        }
      }
      GUI_UNLOCK_H(pText);
    }
  }

  /* Draw the focus rect */
  if (HasFocus) {
    LCD_SetColor(pObj->Props.FocusColor);
    GUI_DrawFocusRect(&rFocus, 0);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(RADIO_Handle hObj) {
  // Nothing special to do...
  GUI_USE_PARA(hObj);
}

/*********************************************************************
*
*       _GetButtonSize
*/
static unsigned _GetButtonSize(RADIO_Handle hObj) {
  RADIO_Obj * pObj;
  unsigned ButtonSize;
  
  pObj = RADIO_LOCK_H(hObj);
  ButtonSize = pObj->Props.apBmRadio[RADIO_BI_ACTIV]->YSize;
  GUI_UNLOCK_H(pObj);
  return ButtonSize;
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
static RADIO_SKIN_PRIVATE _SkinPrivate = {
  _GetButtonSize
};

/*********************************************************************
*
*       RADIO__SkinClassic
*/
const WIDGET_SKIN RADIO__SkinClassic = {
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
*       RADIO_SetSkinClassic
*/
void RADIO_SetSkinClassic(RADIO_Handle hObj) {
  RADIO_Obj * pObj;

  pObj = RADIO_LOCK_H(hObj);
  pObj->pWidgetSkin = &RADIO__SkinClassic;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       RADIO_SetDefaultSkinClassic
*/
void RADIO_SetDefaultSkinClassic(void) {
  RADIO__pSkinDefault = &RADIO__SkinClassic;
}


#else                            /* Avoid empty object files */
  void RADIO_SkinClassic_C(void);
  void RADIO_SkinClassic_C(void) {}
#endif /* GUI_WINSUPPORT */
