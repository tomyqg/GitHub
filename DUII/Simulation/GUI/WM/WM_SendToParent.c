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
File        : WM_SendToParent.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_SendToParent
*/
void WM_SendToParent(WM_HWIN hChild, WM_MESSAGE* pMsg) {
  if (pMsg) {
    WM_HWIN hParent;
    WM_LOCK();
    hParent = WM_GetParent(hChild);
    if (hParent) {
      pMsg->hWinSrc = hChild;
      WM__SendMessage(hParent, pMsg);
    }
    WM_UNLOCK();
  }
}

#else
  void WM_SendToParent_C(void) {}   /* Avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
