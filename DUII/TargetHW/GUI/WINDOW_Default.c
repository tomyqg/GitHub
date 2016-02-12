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
File        : WINDOW.c
Purpose     : Window routines
---------------------------END-OF-HEADER------------------------------
*/

#include "DIALOG.h"
#include "WINDOW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       WINDOW_SetDefaultBkColor
*/
void WINDOW_SetDefaultBkColor(GUI_COLOR Color) {
  WINDOW__DefaultBkColor = Color;
}

/*********************************************************************
*
*       WINDOW_GetDefaultBkColor
*/
GUI_COLOR WINDOW_GetDefaultBkColor(void) {
  return WINDOW__DefaultBkColor;
}

#else
  void WINDOW_Default_C(void);
  void WINDOW_Default_C(void) {} /* avoid empty object files */
#endif
