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
File        : TEXT_Create.c
Purpose     : Implementation of text widget
---------------------------END-OF-HEADER------------------------------
*/

#include "TEXT.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       TEXT_Create
*/
TEXT_Handle TEXT_Create(int x0, int y0, int xsize, int ysize, int Id, int Flags, const char * s, int Align) {
  return TEXT_CreateEx(x0, y0, xsize, ysize, WM_HMEM_NULL, Flags, Align, Id, s);
}

/*********************************************************************
*
*       TEXT_CreateAsChild
*/
TEXT_Handle TEXT_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, const char * s, int Align) {
  return TEXT_CreateEx(x0, y0, xsize, ysize, hParent, Flags, Align, Id, s);
}

#else  /* avoid empty object files */
  void TEXT_Create_C(void) {}
#endif
