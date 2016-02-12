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
File        : WM_SendMessageNoPara.c
Purpose     : Implementation of WM_SendMessageNoPara
----------------------------------------------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
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
*       WM_SendMessageNoPara
*/
void WM_SendMessageNoPara(WM_HWIN hWin, int MsgId) {
  WM_LOCK();
  WM__SendMessageNoPara(hWin, MsgId);
  WM_UNLOCK();
}

#else
  void WM_SendMessageNoPara_c(void);
  void WM_SendMessageNoPara_c(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
