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
File        : TREEVIEW_CreateIndirect.c
Purpose     : Implementation of TREEVIEW widget
---------------------------END-OF-HEADER------------------------------
*/

#include "TREEVIEW.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       TREEVIEW_CreateIndirect
*/
TREEVIEW_Handle TREEVIEW_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb) {
  TREEVIEW_Handle  hThis;
  GUI_USE_PARA(cb);
  hThis = TREEVIEW_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
                            hWinParent, WM_CF_SHOW, pCreateInfo->Flags, pCreateInfo->Id);
  return hThis;
}

#else  /* avoid empty object files */
  void TREEVIEW_CreateIndirect_C(void) {}
#endif

/*************************** End of file ****************************/
