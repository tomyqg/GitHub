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
File        : MULTIPAGE_Default.c
Purpose     : Implementation of MULTIPAGE widget
---------------------------END-OF-HEADER------------------------------
*/

#include "MULTIPAGE_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
/*********************************************************************
*
*       MULTIPAGE_GetDefaultAlign
*/
unsigned MULTIPAGE_GetDefaultAlign(void) {
  return MULTIPAGE__DefaultProps.Align;
}

/*********************************************************************
*
*       MULTIPAGE_GetDefaultBkColor
*/
GUI_COLOR MULTIPAGE_GetDefaultBkColor(unsigned Index) {
  GUI_COLOR Color = GUI_INVALID_COLOR;
  if (Index < GUI_COUNTOF(MULTIPAGE__DefaultProps.aBkColor)) {
    Color = MULTIPAGE__DefaultProps.aBkColor[Index];
  }
  return Color;
}

/*********************************************************************
*
*       MULTIPAGE_GetDefaultFont
*/
const GUI_FONT GUI_UNI_PTR * MULTIPAGE_GetDefaultFont(void) {
  return MULTIPAGE__DefaultProps.pFont;
}

/*********************************************************************
*
*       MULTIPAGE_GetDefaultTextColor
*/
GUI_COLOR MULTIPAGE_GetDefaultTextColor(unsigned Index) {
  GUI_COLOR Color = GUI_INVALID_COLOR;
  if (Index < GUI_COUNTOF(MULTIPAGE__DefaultProps.aTextColor)) {
    Color = MULTIPAGE__DefaultProps.aTextColor[Index];
  }
  return Color;
}

/*********************************************************************
*
*       MULTIPAGE_SetDefaultAlign
*/
void MULTIPAGE_SetDefaultAlign(unsigned Align) {
  MULTIPAGE__DefaultProps.Align = Align;
}

/*********************************************************************
*
*       MULTIPAGE_SetDefaultBkColor
*/
void MULTIPAGE_SetDefaultBkColor(GUI_COLOR Color, unsigned Index) {
  if (Index < GUI_COUNTOF(MULTIPAGE__DefaultProps.aBkColor)) {
    MULTIPAGE__DefaultProps.aBkColor[Index] = Color;
  }
}

/*********************************************************************
*
*       MULTIPAGE_SetDefaultFont
*/
void MULTIPAGE_SetDefaultFont(const GUI_FONT GUI_UNI_PTR * pFont) {
  MULTIPAGE__DefaultProps.pFont = pFont;
}

/*********************************************************************
*
*       MULTIPAGE_SetDefaultTextColor
*/
void MULTIPAGE_SetDefaultTextColor(GUI_COLOR Color, unsigned Index) {
  if (Index < GUI_COUNTOF(MULTIPAGE__DefaultProps.aTextColor)) {
    MULTIPAGE__DefaultProps.aTextColor[Index] = Color;
  }
}

#else
  void MULTIPAGE_Default_C(void);
  void MULTIPAGE_Default_C(void) {} /* avoid empty object files */
#endif
