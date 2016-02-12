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
File        : PROGBAR_Create.c
Purpose     : Implementation of progbar widget
---------------------------END-OF-HEADER------------------------------
*/

#include "PROGBAR.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_Create
*/
PROGBAR_Handle PROGBAR_Create(int x0, int y0, int xsize, int ysize, int Flags) {
  return PROGBAR_CreateEx(x0, y0, xsize, ysize, 0, Flags, 0, 0);
}

/*********************************************************************
*
*       PROGBAR_CreateAsChild
*/
PROGBAR_Handle PROGBAR_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags) {
  return PROGBAR_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id);
}

#else  /* avoid empty object files */
  void PROGBAR_Create_C(void) {}
#endif
