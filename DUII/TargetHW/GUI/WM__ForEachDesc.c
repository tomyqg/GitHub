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
File        : WM__ForEachDesc.c
Purpose     : Implementation of WM__ForEachDesc
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*         Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM__ForEachDesc
*/
void WM__ForEachDesc(WM_HWIN hWin, WM_tfForEach * pcb, void * pData) {
  WM_HWIN hChild;
  WM_Obj* pChild;
  WM_Obj* pWin;
  pWin = WM_H2P(hWin);
  for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
    pChild = WM_H2P(hChild);
    pcb(hChild, pData);
    WM__ForEachDesc(hChild, pcb, pData);
  }
}

#else
  void WM__ForEachDesc_C(void);
  void WM__ForEachDesc_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
