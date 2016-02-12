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
File        : SLIDER_Default.c
Purpose     : Implementation of SLIDER widget
---------------------------END-OF-HEADER------------------------------
*/

#include "SLIDER_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       SLIDER_SetDefaultBkColor
*/
void SLIDER_SetDefaultBkColor(GUI_COLOR Color) {
  SLIDER__DefaultProps.BkColor = Color;
}

/*********************************************************************
*
*       SLIDER_GetDefaultBkColor
*/
GUI_COLOR SLIDER_GetDefaultBkColor(void) {
  return SLIDER__DefaultProps.BkColor;
}

/*********************************************************************
*
*       SLIDER_SetDefaultBarColor
*/
void SLIDER_SetDefaultBarColor(GUI_COLOR Color) {
  SLIDER__DefaultProps.BarColor = Color;
}

/*********************************************************************
*
*       SLIDER_GetDefaultBarColor
*/
GUI_COLOR SLIDER_GetDefaultBarColor(void) {
  return SLIDER__DefaultProps.BarColor;
}

/*********************************************************************
*
*       SLIDER_SetDefaultTickColor
*/
void SLIDER_SetDefaultTickColor(GUI_COLOR Color) {
  SLIDER__DefaultProps.TickColor = Color;
}

/*********************************************************************
*
*       SLIDER_GetDefaultTickColor
*/
GUI_COLOR SLIDER_GetDefaultTickColor(void) {
  return SLIDER__DefaultProps.TickColor;
}

/*********************************************************************
*
*       SLIDER_SetDefaultFocusColor
*/
GUI_COLOR SLIDER_SetDefaultFocusColor(GUI_COLOR Color) {
  GUI_COLOR OldColor = SLIDER__DefaultProps.FocusColor;
  SLIDER__DefaultProps.FocusColor = Color;
  return OldColor;
}

/*********************************************************************
*
*       SLIDER_GetDefaultFocusColor
*/
GUI_COLOR SLIDER_GetDefaultFocusColor(void) {
  return SLIDER__DefaultProps.FocusColor;
}

#else                            /* Avoid problems with empty object modules */
  void SLIDER_Default_C(void);
  void SLIDER_Default_C(void) {}
#endif

