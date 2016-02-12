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
File        : WM_SetCallback.C
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include <stddef.h>
#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */
#include "GUI_Debug.h"

/*********************************************************************
*
*        Exported routines:  Set callback
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_SetCallback
*/
WM_CALLBACK* WM_SetCallback (WM_HWIN hWin, WM_CALLBACK* cb) {
  WM_CALLBACK* r = NULL;  
  if (hWin) {
    WM_Obj* pWin;
    WM_LOCK();
    pWin = WM_H2P(hWin);
    r = pWin->cb;
    pWin->cb = cb; 
    WM_InvalidateWindow(hWin);
    WM_UNLOCK();
  }
  return r;
}

#else
  void WM_SetCallBack(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
