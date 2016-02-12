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
File        : PROGBAR_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "PROGBAR_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   PROGBAR_SKINPROPS
  static PROGBAR_SKINFLEX_PROPS _Props = {
    { 0x00CDFFCD, 0x00ACEE9C },
    { 0x0029D400, 0x0033E21C },
    { 0x00FCFCFC, 0x00DADADA },
    { 0x00CBCBCB, 0x00D5D5D5 },
    0x00A0A0A0,
  };
  #define PROGBAR_SKINPROPS &_Props
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static PROGBAR_SKINFLEX_PROPS * const _pProps = PROGBAR_SKINPROPS;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawBackground
*/
static void _DrawBackground(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, int IsVertical) {
  int Middle, a0, a1;
  PROGBAR_SKINFLEX_INFO * pSkinInfo;
  U32 Color0, Color1;

  pSkinInfo = (PROGBAR_SKINFLEX_INFO *)pDrawItemInfo->p;
  //
  // Draw gradients
  //
  if (IsVertical) {
    //
    // Draw vertical
    //
    Middle = (pDrawItemInfo->x1 - pDrawItemInfo->x0 + 1) >> 1;
    if (pSkinInfo->Index == PROGBAR_SKINFLEX_L) {
      Color0 = _pProps->aColorUpperR[0];
      Color1 = _pProps->aColorUpperR[1];
      a0     = 1;
      a1     = 0;
    } else {
      Color0 = _pProps->aColorUpperL[0];
      Color1 = _pProps->aColorUpperL[1];
      a0     = 0;
      a1     = 1;
    }
    GUI_DrawGradientH(pDrawItemInfo->x0 + 1,
                      pDrawItemInfo->y0 + a0,
                      pDrawItemInfo->x0 + Middle,
                      pDrawItemInfo->y1 - a1,
                      Color0, Color1);
    if (pSkinInfo->Index == PROGBAR_SKINFLEX_L) {
      Color0 = _pProps->aColorLowerR[0];
      Color1 = _pProps->aColorLowerR[1];
      a0     = 1;
      a1     = 0;
    } else {
      Color0 = _pProps->aColorLowerL[0];
      Color1 = _pProps->aColorLowerL[1];
      a0     = 0;
      a1     = 1;
    }
    GUI_DrawGradientH(pDrawItemInfo->x0 + Middle + 1,
                      pDrawItemInfo->y0 + a0,
                      pDrawItemInfo->x1 - 1,
                      pDrawItemInfo->y1 - a1,
                      Color0, Color1);
  } else {
    //
    // Draw horizontal
    //
    Middle = (pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1) >> 1;
    if (pSkinInfo->Index == PROGBAR_SKINFLEX_L) {
      Color0 = _pProps->aColorUpperL[0];
      Color1 = _pProps->aColorUpperL[1];
      a0     = 1;
      a1     = 0;
    } else {
      Color0 = _pProps->aColorUpperR[0];
      Color1 = _pProps->aColorUpperR[1];
      a0     = 0;
      a1     = 1;
    }
    GUI_DrawGradientV(pDrawItemInfo->x0 + a0,
                      pDrawItemInfo->y0 + 1,
                      pDrawItemInfo->x1 - a1,
                      pDrawItemInfo->y0 + Middle,
                      Color0, Color1);
    if (pSkinInfo->Index == PROGBAR_SKINFLEX_L) {
      Color0 = _pProps->aColorLowerL[0];
      Color1 = _pProps->aColorLowerL[1];
      a0     = 1;
      a1     = 0;
    } else {
      Color0 = _pProps->aColorLowerR[0];
      Color1 = _pProps->aColorLowerR[1];
      a0     = 0;
      a1     = 1;
    }
    GUI_DrawGradientV(pDrawItemInfo->x0 + a0,
                      pDrawItemInfo->y0 + Middle + 1,
                      pDrawItemInfo->x1 - a1,
                      pDrawItemInfo->y1 - 1,
                      Color0, Color1);
  }
}

/*********************************************************************
*
*       _DrawText
*/
static void _DrawText(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const char * pText) {
  GUI_SetColor(_pProps->ColorText);
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_DispStringAt(pText, pDrawItemInfo->x0, pDrawItemInfo->y0);
}

/*********************************************************************
*
*       _DrawFrame
*/
static void _DrawFrame(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  //
  // Draw frame
  //
  GUI_SetColor(_pProps->ColorFrame);
  GUI_DrawHLine(pDrawItemInfo->y0, pDrawItemInfo->x0 + 0, pDrawItemInfo->x1 - 0);
  GUI_DrawHLine(pDrawItemInfo->y1, pDrawItemInfo->x0 + 0, pDrawItemInfo->x1 - 0);
  GUI_DrawVLine(pDrawItemInfo->x0, pDrawItemInfo->y0 + 1, pDrawItemInfo->y1 - 1);
  GUI_DrawVLine(pDrawItemInfo->x1, pDrawItemInfo->y0 + 1, pDrawItemInfo->y1 - 1);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_DrawSkinFlex
*/
int PROGBAR_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  PROGBAR_SKINFLEX_INFO * pSkinInfo;

  pSkinInfo = (PROGBAR_SKINFLEX_INFO *)pDrawItemInfo->p;
  //
  // Draw items
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_FRAME:
    _DrawFrame(pDrawItemInfo);
    break;
  case WIDGET_ITEM_DRAW_BACKGROUND:
    _DrawBackground(pDrawItemInfo, pSkinInfo->IsVertical);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    if (pSkinInfo->IsVertical == 0) {
      _DrawText(pDrawItemInfo, pSkinInfo->pText);
    }
    break;
  case WIDGET_ITEM_CREATE:
    break;
  }
  return 0;
}

/*********************************************************************
*
*       PROGBAR_SetSkinFlexProps
*/
void PROGBAR_SetSkinFlexProps(const PROGBAR_SKINFLEX_PROPS * pProps, int Index) {
  if (Index == 0) {
    *_pProps = *pProps;
  }
}

/*********************************************************************
*
*       PROGBAR_GetSkinFlexProps
*/
void PROGBAR_GetSkinFlexProps(PROGBAR_SKINFLEX_PROPS * pProps, int Index) {
  if (Index == 0) {
    *pProps = *_pProps;
  }
}

#else
  void PROGBAR_SkinFlex_C(void);
  void PROGBAR_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
