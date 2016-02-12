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
File        : WM_Paint.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include <stddef.h>
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_Update
*/
void WM_Update(WM_HWIN hWin) {
  GUI_CONTEXT Context;

  WM_ASSERT_NOT_IN_PAINT();
  if (hWin) {
    WM_LOCK();
    GUI_SaveContext(&Context);
    WM__Paint(hWin);
    GUI_RestoreContext(&Context);
    WM_UNLOCK();
    //
    // Make sure that alpha device settings depends on current context
    //
    LCD_SetColor(Context.Color);
    LCD_SetBkColor(Context.BkColor);
  }
}

/*********************************************************************
*
*       WM_Paint
*/
void WM_Paint(WM_HWIN hWin) {
  WM_InvalidateWindow(hWin);
  WM_Update(hWin);
}

#else
  void WM_Paint(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
