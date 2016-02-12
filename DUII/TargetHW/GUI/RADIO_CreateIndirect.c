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
File        : RADIO_Create.c
Purpose     : Implementation of radio widget
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "RADIO.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_CreateIndirect
*/
RADIO_Handle RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb) {
  RADIO_Handle  hThis;
  int NumItems = (pCreateInfo->Para)       & 0xFF;
  int Spacing  = (pCreateInfo->Para >>  8) & 0xFF;
  GUI_USE_PARA(cb);
  hThis = RADIO_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
                         hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id, NumItems, Spacing);
  return hThis;
}

#else  /* avoid empty object files */
  void RADIO_CreateIndirect_C(void) {}
#endif

/************************* end of file ******************************/
