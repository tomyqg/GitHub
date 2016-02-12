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
File        : FRAMEWIN_CreateIndirect.c
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
*       FRAMEWIN_CreateIndirect
*/
FRAMEWIN_Handle FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent,
                                       int x0, int y0, WM_CALLBACK* cb) {
  FRAMEWIN_Handle hObj;
  hObj = FRAMEWIN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
                           hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName, cb);
  return hObj;
}

#else  /* avoid empty object files */
  void FRAMEWIN_CreateIndirect_C(void) {}
#endif
