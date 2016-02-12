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
File        : MULTIEDIT_Create.c
Purpose     : Implementation of multiedit widget
---------------------------END-OF-HEADER------------------------------
*/

#include "MULTIEDIT.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       MULTIEDIT_Create
*/
MULTIEDIT_HANDLE MULTIEDIT_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int ExFlags, const char * pText, int MaxLen) {
  return MULTIEDIT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, ExFlags, Id, MaxLen, pText);
}

#else  /* avoid empty object files */
  void MULTIEDIT_Create_C(void) {}
#endif
