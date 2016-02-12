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
File        : FRAMEWIN_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "FRAMEWIN_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   FRAMEWIN_SKINFLEX_BORDERSIZE_L
  #define FRAMEWIN_SKINFLEX_BORDERSIZE_L 5
#endif
#ifndef   FRAMEWIN_SKINFLEX_BORDERSIZE_R
  #define FRAMEWIN_SKINFLEX_BORDERSIZE_R 5
#endif
#ifndef   FRAMEWIN_SKINFLEX_BORDERSIZE_T
  #define FRAMEWIN_SKINFLEX_BORDERSIZE_T 2
#endif
#ifndef   FRAMEWIN_SKINFLEX_BORDERSIZE_B
  #define FRAMEWIN_SKINFLEX_BORDERSIZE_B 4
#endif
#ifndef   FRAMEWIN_SKINFLEX_RADIUS
  #define FRAMEWIN_SKINFLEX_RADIUS       5
#endif
#ifndef   FRAMEWIN_SKINFLEX_SPACE_X
  #define FRAMEWIN_SKINFLEX_SPACE_X      0
#endif

#ifndef   FRAMEWIN_SKINPROPS_ACTIVE
  static FRAMEWIN_SKINFLEX_PROPS _PropsActive = {
    { 0x00000000, 0x00FFFFFF, 0x00D2B69A },
    { 0x00D2B69A, 0x00EAD1B9 },
    FRAMEWIN_SKINFLEX_RADIUS,   
    FRAMEWIN_SKINFLEX_SPACE_X,   
    FRAMEWIN_SKINFLEX_BORDERSIZE_L,
    FRAMEWIN_SKINFLEX_BORDERSIZE_R,
    FRAMEWIN_SKINFLEX_BORDERSIZE_T,
    FRAMEWIN_SKINFLEX_BORDERSIZE_B
  };
  #define FRAMEWIN_SKINPROPS_ACTIVE      &_PropsActive
#endif

#ifndef   FRAMEWIN_SKINPROPS_INACTIVE
  static FRAMEWIN_SKINFLEX_PROPS _PropsInactive = {
    { 0x004C4C4C, 0x00FFFFFF, 0x00DCCEC0 },
    { 0x00DCCEC0, 0x00F2E4D7 },
    FRAMEWIN_SKINFLEX_RADIUS,   
    FRAMEWIN_SKINFLEX_SPACE_X,   
    FRAMEWIN_SKINFLEX_BORDERSIZE_L,
    FRAMEWIN_SKINFLEX_BORDERSIZE_R,
    FRAMEWIN_SKINFLEX_BORDERSIZE_T,
    FRAMEWIN_SKINFLEX_BORDERSIZE_B
  };
  #define FRAMEWIN_SKINPROPS_INACTIVE    &_PropsInactive
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static FRAMEWIN_SKINFLEX_PROPS * const _apProps[] = {
  FRAMEWIN_SKINPROPS_ACTIVE,
  FRAMEWIN_SKINPROPS_INACTIVE
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_DrawSkinFlex
*/
int FRAMEWIN_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  const FRAMEWIN_SKINFLEX_PROPS * pProps;
  GUI_RECT Rect;
  FRAMEWIN_Obj * pObj;
  int ColorIndex;
  const char * pText;
  int IsActive, x0, y0, x1, y1;

  IsActive = FRAMEWIN_GetActive(pDrawItemInfo->hWin);
  pProps = (IsActive) ? _apProps[FRAMEWIN_SKINFLEX_PI_ACTIVE] : _apProps[FRAMEWIN_SKINFLEX_PI_INACTIVE];
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_GET_BORDERSIZE_L:
    return pProps->BorderSizeL;
  case WIDGET_ITEM_GET_BORDERSIZE_R:
    return pProps->BorderSizeR;
  case WIDGET_ITEM_GET_BORDERSIZE_T:
    return pProps->BorderSizeT;
  case WIDGET_ITEM_GET_BORDERSIZE_B:
    return pProps->BorderSizeB;
  case WIDGET_ITEM_DRAW_FRAME:
    //
    // Rounded corners
    //
    if (pProps->Radius >= 2) {
      GUI__DrawTwinArc2(pDrawItemInfo->x0, pDrawItemInfo->x1, pDrawItemInfo->y0, pProps->Radius, pProps->aColorFrame[0], pProps->aColorFrame[1], pProps->aColorFrame[2]);
    } else {
      GUI_SetColor(pProps->aColorFrame[0]);
      GUI_DrawHLine(pDrawItemInfo->y0, pDrawItemInfo->x0, pDrawItemInfo->x0 + pProps->BorderSizeL - 1);
      GUI_DrawHLine(pDrawItemInfo->y0, pDrawItemInfo->x1 - pProps->BorderSizeL + 1, pDrawItemInfo->x1);
      y0 = pDrawItemInfo->y0 + 1;
      GUI_DrawPixel(pDrawItemInfo->x0, y0);
      GUI_DrawPixel(pDrawItemInfo->x1, y0);
      GUI_SetColor(pProps->aColorFrame[1]);
      GUI_DrawPixel(pDrawItemInfo->x0 + 1, y0);
      GUI_DrawPixel(pDrawItemInfo->x1 - 1, y0);
    }
    //
    // Border top
    //
    if (pProps->Radius >= 2) {
      x0 = pDrawItemInfo->x0 + pProps->Radius;
      x1 = pDrawItemInfo->x1 - pProps->Radius;
    } else {
      x0 = pDrawItemInfo->x0 + 2;
      x1 = pDrawItemInfo->x1 - 2;
    }
    GUI_SetColor(pProps->aColorFrame[0]);
    GUI_DrawHLine(pDrawItemInfo->y0, x0, x1);
    GUI_SetColor(pProps->aColorFrame[1]);
    GUI_DrawHLine(pDrawItemInfo->y0 + 1, x0, x1);
    if (pProps->BorderSizeT > 2) {
      GUI_SetColor(pProps->aColorFrame[2]);
      y0 = pDrawItemInfo->y0 + 2;
      y1 = pDrawItemInfo->y0 + pProps->BorderSizeT - 1;
      GUI_FillRect(x0, y0, x1, y1);
    } else {
      if (pProps->BorderSizeR > pProps->Radius) {
        y0 = pDrawItemInfo->y0 + 2;
        y1 = (pProps->Radius >= 2) ? pDrawItemInfo->y0 + pProps->Radius : pDrawItemInfo->y0 + 2;
        x0 = (pProps->Radius >= 2) ? pDrawItemInfo->x0 + pProps->Radius : pDrawItemInfo->x0 + 2;
        x1 = pDrawItemInfo->x0 + pProps->BorderSizeR - 1;
        GUI_FillRect(x0, y0, x1, y1);
        x1 = (pProps->Radius >= 2) ? pDrawItemInfo->x1 - pProps->Radius : pDrawItemInfo->x1 - 2;
        x0 = pDrawItemInfo->x1 - pProps->BorderSizeL + 1;
        GUI_FillRect(x0, y0, x1, y1);
      }
    }
    //
    // Border left & right
    //
    y0 = (pProps->Radius >= 2) ? pDrawItemInfo->y0 + pProps->Radius : pDrawItemInfo->y0 + 2;
    y1 = pDrawItemInfo->y1 - 1;
    GUI_SetColor(pProps->aColorFrame[0]);
    GUI_DrawVLine(pDrawItemInfo->x0, y0, y1);
    GUI_DrawVLine(pDrawItemInfo->x1, y0, y1);
    y1 = pDrawItemInfo->y1 - 2;
    GUI_SetColor(pProps->aColorFrame[1]);
    GUI_DrawVLine(pDrawItemInfo->x0 + 1, y0, y1);
    GUI_DrawVLine(pDrawItemInfo->x1 - 1, y0, y1);
    if ((pProps->BorderSizeL > 2) || (pProps->BorderSizeR > 2)) {
      y1 = (pProps->BorderSizeB > 2) ? pDrawItemInfo->y1 - pProps->BorderSizeB : pDrawItemInfo->y1 - 2;
      GUI_SetColor(pProps->aColorFrame[2]);
      if (pProps->BorderSizeL > 2) {
        x0 = pDrawItemInfo->x0 + 2;
        x1 = pDrawItemInfo->x0 + pProps->BorderSizeL - 1;
        GUI_FillRect(x0, y0, x1, y1);
      }
      if (pProps->BorderSizeR > 2) {
        x0 = pDrawItemInfo->x1 - pProps->BorderSizeL + 1;
        x1 = pDrawItemInfo->x1 - 2;
        GUI_FillRect(x0, y0, x1, y1);
      }
    }
    //
    // Border bottom
    //
    GUI_SetColor(pProps->aColorFrame[0]);
    GUI_DrawHLine(pDrawItemInfo->y1, pDrawItemInfo->x0, pDrawItemInfo->x1);
    GUI_SetColor(pProps->aColorFrame[1]);
    GUI_DrawHLine(pDrawItemInfo->y1 - 1, pDrawItemInfo->x0 + 1, pDrawItemInfo->x1 - 1);
    if (pProps->BorderSizeB > 2) {
      x0 = pDrawItemInfo->x0 + 2;
      x1 = pDrawItemInfo->x1 - 2;
      y0 = pDrawItemInfo->y1 - pProps->BorderSizeB + 1;
      y1 = pDrawItemInfo->y1 - 2;
      GUI_SetColor(pProps->aColorFrame[2]);
      GUI_FillRect(x0, y0, x1, y1);
    }
    break;
  case WIDGET_ITEM_DRAW_BACKGROUND:
    Rect.x0 = pDrawItemInfo->x0;
    Rect.y0 = pDrawItemInfo->y0;
    Rect.x1 = pDrawItemInfo->x1;
    Rect.y1 = pDrawItemInfo->y1;
    GUI_DrawGradientV(Rect.x0, Rect.y0, Rect.x1, Rect.y1, pProps->aColorTitle[0], pProps->aColorTitle[1]);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    Rect.x0 = pDrawItemInfo->x0 + pProps->SpaceX;
    Rect.y0 = pDrawItemInfo->y0;
    Rect.x1 = pDrawItemInfo->x1 - pProps->SpaceX;
    Rect.y1 = pDrawItemInfo->y1;
    pObj = FRAMEWIN_LOCK_H(pDrawItemInfo->hWin); {
      if (pObj->hText) {
        GUI_SetTextMode(GUI_TM_TRANS);
        ColorIndex = (pObj->Flags & FRAMEWIN_SF_ACTIVE) ? 1 : 0;
        GUI_SetColor(pObj->Props.aTextColor[ColorIndex]);
        GUI_SetFont(pObj->Props.pFont);
        pText = (const char *)GUI_LOCK_H(pObj->hText); {
          GUI_DispStringInRect(pText, &Rect, pObj->Props.TextAlign);
        } GUI_UNLOCK_H(pText);
      }
    } GUI_UNLOCK_H(pObj);
    break;
  case WIDGET_ITEM_DRAW_SEP:
    Rect.x0 = pDrawItemInfo->x0;
    Rect.y0 = pDrawItemInfo->y0;
    Rect.x1 = pDrawItemInfo->x1;
    Rect.y1 = pDrawItemInfo->y1;
    GUI_SetColor(pProps->aColorFrame[2]);
    GUI_FillRectEx(&Rect);
    break;
  case WIDGET_ITEM_CREATE:
    pObj = FRAMEWIN_LOCK_H(pDrawItemInfo->hWin); {
      pObj->Props.TextAlign |= GUI_TA_VCENTER;
    } GUI_UNLOCK_H(pObj);
    WM_SetHasTrans(pDrawItemInfo->hWin);
    break;
  }
  return 0;
}

/*********************************************************************
*
*       FRAMEWIN_SetSkinFlexProps
*/
void FRAMEWIN_SetSkinFlexProps(const FRAMEWIN_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
    //
    // The size of the border has to be at least 2 pixels
    //
    if (_apProps[Index]->BorderSizeL < 2) {
      _apProps[Index]->BorderSizeL = 2;
    }
    if (_apProps[Index]->BorderSizeR < 2) {
      _apProps[Index]->BorderSizeR = 2;
    }
    if (_apProps[Index]->BorderSizeT < 2) {
      _apProps[Index]->BorderSizeT = 2;
    }
    if (_apProps[Index]->BorderSizeB < 2) {
      _apProps[Index]->BorderSizeB = 2;
    }
  }
}

/*********************************************************************
*
*       FRAMEWIN_GetSkinFlexProps
*/
void FRAMEWIN_GetSkinFlexProps(FRAMEWIN_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void FRAMEWIN_SkinFlex_C(void);
  void FRAMEWIN_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
