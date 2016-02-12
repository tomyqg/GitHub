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
File        : RADIO_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "RADIO_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   RADIO_SKINFLEX_SIZE
  #define RADIO_SKINFLEX_SIZE 12
#endif

#ifndef   RADIO_SKINPROPS_CHECKED
  static RADIO_SKINFLEX_PROPS _PropsChecked = {
    { 0x00707070, 0x00FCFCFC, 0x00B9B3AE, 0x00404040 },
    RADIO_SKINFLEX_SIZE,
  };
  #define RADIO_SKINPROPS_CHECKED     &_PropsChecked
#endif

#ifndef   RADIO_SKINPROPS_UNCHECKED
  static RADIO_SKINFLEX_PROPS _PropsUnchecked = {
    { 0x00707070, 0x00FCFCFC, 0x00B9B3AE, 0x00F3F3F3 },
    RADIO_SKINFLEX_SIZE,
  };
  #define RADIO_SKINPROPS_UNCHECKED   &_PropsUnchecked
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static RADIO_SKINFLEX_PROPS * _apProps[] = {
  RADIO_SKINPROPS_CHECKED,
  RADIO_SKINPROPS_UNCHECKED,
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawText
*/
static void _DrawText(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const RADIO_SKINFLEX_PROPS * pProps) {
  RADIO_Obj * pObj;
  GUI_COLOR TextColor;
  const char * pText;

  GUI_USE_PARA(pProps);
  pObj = RADIO_LOCK_H(pDrawItemInfo->hWin);
  TextColor = pObj->Props.TextColor;
  GUI_UNLOCK_H(pObj);
  LCD_SetColor(TextColor);
  pText = (const char *)pDrawItemInfo->p;
  GUI_DispStringAt(pText, pDrawItemInfo->x0, pDrawItemInfo->y0);
}

/*********************************************************************
*
*       _DrawFocus
*/
static void _DrawFocus(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const RADIO_SKINFLEX_PROPS * pProps) {
  RADIO_Obj * pObj;
  GUI_RECT Rect;
  GUI_COLOR FocusColor;

  GUI_USE_PARA(pProps);
  pObj = RADIO_LOCK_H(pDrawItemInfo->hWin);
  FocusColor = pObj->Props.FocusColor;
  GUI_UNLOCK_H(pObj);
  Rect.x0 = pDrawItemInfo->x0;
  Rect.y0 = pDrawItemInfo->y0;
  Rect.x1 = pDrawItemInfo->x1;
  Rect.y1 = pDrawItemInfo->y1;
  LCD_SetColor(FocusColor);
  GUI_DrawFocusRect(&Rect, 0);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_DrawSkinFlex
*/
int RADIO_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  RADIO_Obj * pObj;
  const RADIO_SKINFLEX_PROPS * pProps;
  int Sel;

  //
  // Get object properties
  //
  pObj = RADIO_LOCK_H(pDrawItemInfo->hWin);
  Sel = pObj->Sel;
  GUI_UNLOCK_H(pObj);
  //
  // Get right property pointer
  //
  pProps = (pDrawItemInfo->ItemIndex == Sel) ? _apProps[0] : _apProps[1];
  //
  // Draw
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BUTTON:
    GUI__FillTrippleArc(pDrawItemInfo->x0, 
                        pDrawItemInfo->y0, 
                        pProps->ButtonSize, 
                        pProps->aColorButton[0], 
                        pProps->aColorButton[1], 
                        pProps->aColorButton[2], 
                        pProps->aColorButton[3]);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    _DrawText(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_DRAW_FOCUS:
    _DrawFocus(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_GET_BUTTONSIZE:
    return pProps->ButtonSize;
  }
  return 0;
}

/*********************************************************************
*
*       RADIO_SetSkinFlexProps
*/
void RADIO_SetSkinFlexProps(const RADIO_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
  }
}

/*********************************************************************
*
*       RADIO_GetSkinFlexProps
*/
void RADIO_GetSkinFlexProps(RADIO_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void RADIO_SkinFlex_C(void);
  void RADIO_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
