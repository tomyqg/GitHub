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
File        : DROPDOWN_Default.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_SetDefaultFont
*/
void DROPDOWN_SetDefaultFont(const GUI_FONT GUI_UNI_PTR * pFont) {
  DROPDOWN__DefaultProps.pFont = pFont;
}

/*********************************************************************
*
*       DROPDOWN_GetDefaultFont
*/
const GUI_FONT GUI_UNI_PTR * DROPDOWN_GetDefaultFont(void) {
  return DROPDOWN__DefaultProps.pFont;
}

/*********************************************************************
*
*       DROPDOWN_SetDefaultColor
*/
GUI_COLOR DROPDOWN_SetDefaultColor(int Index, GUI_COLOR Color) {
  GUI_COLOR OldColor = 0;
  if (Index < GUI_COUNTOF(DROPDOWN__DefaultProps.aColor)) {
    OldColor = DROPDOWN__DefaultProps.aColor[Index];
    DROPDOWN__DefaultProps.aColor[Index] = Color;
  }
  return OldColor;
}

/*********************************************************************
*
*       DROPDOWN_SetDefaultBkColor
*/
GUI_COLOR DROPDOWN_SetDefaultBkColor(int Index, GUI_COLOR Color) {
  GUI_COLOR OldColor = 0;
  if (Index < GUI_COUNTOF(DROPDOWN__DefaultProps.aBackColor)) {
    OldColor = DROPDOWN__DefaultProps.aBackColor[Index];
    DROPDOWN__DefaultProps.aBackColor[Index] = Color;
  }
  return OldColor;
}

/*********************************************************************
*
*       DROPDOWN_GetDefaultBkColor
*/
GUI_COLOR DROPDOWN_GetDefaultBkColor(int Index) {
  GUI_COLOR Color = 0;
  if (Index < GUI_COUNTOF(DROPDOWN__DefaultProps.aBackColor)) {
    Color = DROPDOWN__DefaultProps.aBackColor[Index];
  }
  return Color;
}

/*********************************************************************
*
*       DROPDOWN_SetDefaultScrollbarColor
*/
GUI_COLOR DROPDOWN_SetDefaultScrollbarColor(int Index, GUI_COLOR Color) {
  GUI_COLOR OldColor = 0;
  if (Index < GUI_COUNTOF(DROPDOWN__DefaultProps.aScrollbarColor)) {
    OldColor = DROPDOWN__DefaultProps.aScrollbarColor[Index];
    DROPDOWN__DefaultProps.aScrollbarColor[Index] = Color;
  }
  return OldColor;
}

/*********************************************************************
*
*       DROPDOWN_GetDefaultColor
*/
GUI_COLOR DROPDOWN_GetDefaultColor(int Index) {
  GUI_COLOR Color = 0;
  if (Index < GUI_COUNTOF(DROPDOWN__DefaultProps.aColor)) {
    Color = DROPDOWN__DefaultProps.aColor[Index];
  }
  return Color;
}

/*********************************************************************
*
*       DROPDOWN_GetDefaultScrollbarColor
*/
GUI_COLOR DROPDOWN_GetDefaultScrollbarColor(int Index) {
  GUI_COLOR Color = 0;
  if (Index < GUI_COUNTOF(DROPDOWN__DefaultProps.aScrollbarColor)) {
    Color = DROPDOWN__DefaultProps.aScrollbarColor[Index];
  }
  return Color;
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_Default_C(void);
  void DROPDOWN_Default_C(void) {}
#endif
