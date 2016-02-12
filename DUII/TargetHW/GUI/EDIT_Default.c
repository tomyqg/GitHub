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
File        : EDIT_Default.c
Purpose     : Implementation of edit widget
---------------------------END-OF-HEADER------------------------------
*/

#include "EDIT.h"
#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       EDIT_SetDefaultFont
*/
void EDIT_SetDefaultFont(const GUI_FONT* pFont) {
  EDIT__DefaultProps.pFont = pFont;
}

/*********************************************************************
*
*       EDIT_GetDefaultFont
*/
const GUI_FONT GUI_UNI_PTR * EDIT_GetDefaultFont(void) {
  return EDIT__DefaultProps.pFont;
}

/*********************************************************************
*
*       EDIT_SetDefaultTextAlign
*/
void EDIT_SetDefaultTextAlign(int Align) {
  EDIT__DefaultProps.Align = Align;
}

/*********************************************************************
*
*       EDIT_GetDefaultTextAlign
*/
int EDIT_GetDefaultTextAlign(void) {
  return EDIT__DefaultProps.Align;
}

/*********************************************************************
*
*       EDIT_SetDefaultTextColor
*/
void EDIT_SetDefaultTextColor(unsigned int Index, GUI_COLOR Color) {
  if (Index <= GUI_COUNTOF(EDIT__DefaultProps.aTextColor)) {
    EDIT__DefaultProps.aTextColor[Index] = Color;
  }
}

/*********************************************************************
*
*       EDIT_SetDefaultBkColor
*/
void EDIT_SetDefaultBkColor(unsigned int Index, GUI_COLOR Color) {
  if (Index <= GUI_COUNTOF(EDIT__DefaultProps.aBkColor)) {
    EDIT__DefaultProps.aBkColor[Index] = Color;
  }
}

/*********************************************************************
*
*       EDIT_GetDefaultTextColor
*/
GUI_COLOR EDIT_GetDefaultTextColor(unsigned int Index) {
  GUI_COLOR Color = 0;
  if (Index <= GUI_COUNTOF(EDIT__DefaultProps.aTextColor)) {
    Color = EDIT__DefaultProps.aTextColor[Index];
  }
  return Color;
}

/*********************************************************************
*
*       EDIT_GetDefaultBkColor
*/
GUI_COLOR EDIT_GetDefaultBkColor(unsigned int Index) {
  GUI_COLOR Color = 0;
  if (Index <= GUI_COUNTOF(EDIT__DefaultProps.aBkColor)) {
    Color = EDIT__DefaultProps.aBkColor[Index];
  }
  return Color;
}

#else  /* avoid empty object files */

void EDIT_Default_C(void) {}

#endif
