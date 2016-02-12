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
File        : FRAMEWIN_Create.c
Purpose     : Implementation of framewin widget
---------------------------END-OF-HEADER------------------------------
*/

#include "FRAMEWIN.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_Create
*/
FRAMEWIN_Handle FRAMEWIN_Create(const char* pText, WM_CALLBACK* cb, int Flags,
                                int x0, int y0, int xsize, int ysize) {
  return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, WM_HWIN_NULL, Flags, 0, 0, pText, cb);
}

/*********************************************************************
*
*       FRAMEWIN_CreateAsChild
*/
FRAMEWIN_Handle FRAMEWIN_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                       const char* pText, WM_CALLBACK* cb, int Flags) {
  return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, 0, pText, cb);
}

#else  /* avoid empty object files */
  void FRAMEWIN_Create_C(void) {}
#endif
