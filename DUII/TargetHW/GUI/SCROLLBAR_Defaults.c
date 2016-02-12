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
File        : SCROLLBAR_Defaults.c
Purpose     : SCROLLBAR, optional routine
---------------------------END-OF-HEADER------------------------------
*/

#include "SCROLLBAR_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR_GetDefaultWidth
*/
int SCROLLBAR_GetDefaultWidth(void) {
  return SCROLLBAR__DefaultWidth;
}

/*********************************************************************
*
*       SCROLLBAR_SetDefaultWidth
*/
int  SCROLLBAR_SetDefaultWidth(int DefaultWidth) {
  int OldWidth = SCROLLBAR__DefaultWidth;
  SCROLLBAR__DefaultWidth = DefaultWidth;
  return OldWidth;
}

/*********************************************************************
*
*       SCROLLBAR_SetDefaultColor
*/
GUI_COLOR SCROLLBAR_SetDefaultColor(GUI_COLOR Color, unsigned int Index) {
  GUI_COLOR OldColor = 0;
  if (Index < GUI_COUNTOF(SCROLLBAR__DefaultProps.aColor)) {
    OldColor = SCROLLBAR__DefaultProps.aColor[Index];
    SCROLLBAR__DefaultProps.aColor[Index] = Color;
  }
  return OldColor; 
}

#else
  void SCROLLBAR_Defaults_c(void) {}    /* Avoid empty object files */
#endif  /* #if GUI_WINSUPPORT */



