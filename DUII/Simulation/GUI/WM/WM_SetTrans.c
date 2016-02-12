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
File        : WM_SetTrans.c
Purpose     : Windows manager, optional routines
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT
#if WM_SUPPORT_TRANSPARENCY   /* If 0, WM will not generate any code */

#include "GUI_Debug.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_SetHasTrans
*/
void WM_SetHasTrans(WM_HWIN hWin) {
  WM_Obj *pWin;
  WM_LOCK();
  if (hWin) {
    pWin = WM_H2P(hWin);  
    //
    // First check if this is necessary at all
    //
    if ((pWin->Status & WM_SF_HASTRANS) == 0) {
      //
      // Now make sure that the number of invalid windows is correct
      //
      WM_ValidateWindow(hWin);
      //
      // Make window transparent
      //
      pWin->Status |= WM_SF_HASTRANS; // Set Transparency flag
      WM__TransWindowCnt++;           // Increment counter for transparency windows
      //
      // Invalidate window area
      //
      WM_InvalidateWindow(hWin);      // Mark content as invalid
    }
  }
  WM_UNLOCK();
}

/*********************************************************************
*
*       WM_ClrHasTrans
*/
void WM_ClrHasTrans(WM_HWIN hWin) {
  WM_Obj *pWin;
  WM_LOCK();
  if (hWin) {
    pWin = WM_H2P(hWin);  
    //
    // First check if this is necessary at all
    //
    if (pWin->Status & WM_SF_HASTRANS) {
      pWin->Status &= ~WM_SF_HASTRANS;
      WM__TransWindowCnt--;           // Decrement counter for transparency windows 
      WM_InvalidateWindow(hWin);      // Mark content as invalid 
    }
  }
  WM_UNLOCK();
}

/*********************************************************************
*
*       WM_GetHasTrans
*/
int WM_GetHasTrans(WM_HWIN hWin) {
  int r = 0;
  WM_Obj *pWin;
  WM_LOCK();
  if (hWin) {
    pWin = WM_H2P(hWin);  
    r = pWin->Status & WM_SF_HASTRANS;
  }
  WM_UNLOCK();
  return r;
}

#endif /*WM_SUPPORT_TRANSPARENCY*/

#else
  void WM_SetTrans_c(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
