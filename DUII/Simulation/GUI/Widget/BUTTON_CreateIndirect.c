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
File        : BUTTON_CreateIndirect.c
Purpose     : Implementation of button widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include "BUTTON_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_CreateIndirect
*/
BUTTON_Handle BUTTON_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb) {
  BUTTON_Handle  hThis;
  GUI_USE_PARA(cb);
  hThis = BUTTON_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
                          pCreateInfo->xSize, pCreateInfo->ySize,
                          hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
  BUTTON_SetText(hThis, pCreateInfo->pName);
  return hThis;
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_CreateIndirect_C(void) {}
#endif /* GUI_WINSUPPORT */




