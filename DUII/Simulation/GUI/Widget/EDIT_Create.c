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
File        : EDIT_Create.c
Purpose     : Implementation of edit widget
---------------------------END-OF-HEADER------------------------------
*/

#include "EDIT.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_Create
*/
EDIT_Handle EDIT_Create(int x0, int y0, int xsize, int ysize, int Id, int MaxLen, int Flags) {
  return EDIT_CreateEx(x0, y0, xsize, ysize, WM_HMEM_NULL, Flags, 0, Id, MaxLen);
}

/*********************************************************************
*
*       EDIT_CreateAsChild
*/
EDIT_Handle EDIT_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int MaxLen) {
  return EDIT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id, MaxLen);
}

#else  /* avoid empty object files */
  void EDIT_Create_C(void) {}
#endif
