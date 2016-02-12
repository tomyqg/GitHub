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
File        : WM_PaintWindowAndDescs.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include <stddef.h>
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbPaintWindowAndDescs
*/
static void _cbPaintWindowAndDescs(WM_HWIN hWin, void* pData) {
  GUI_USE_PARA(pData);
  WM_Paint(hWin);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_PaintWindowAndDescs
*/
void WM_PaintWindowAndDescs(WM_HWIN hWin) {
  WM_Paint(hWin);
  WM_ForEachDesc(hWin, _cbPaintWindowAndDescs, NULL);
}

#else
  void WM_PaintWindowAndDescs(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
