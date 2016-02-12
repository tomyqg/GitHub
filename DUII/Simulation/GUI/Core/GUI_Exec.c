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
File        : GUI_Exec.c
Purpose     : Implementation of GUI_Exec
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

#if GUI_WINSUPPORT
#include "WM.h"
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_Exec1
*/
int GUI_Exec1(void) {
  int r = 0;
  /* Execute background jobs */
  if (GUI_pfTimerExec) {
    if ((*GUI_pfTimerExec)()) {
      r = 1;                  /* We have done something */
    }
  }
  #if GUI_WINSUPPORT    /* If 0, WM will not generate any code */
  if (WM_Exec())
    r = 1;
  #endif
  return r;
}

/*********************************************************************
*
*       GUI_Exec
*/
int GUI_Exec(void) {
  int r = 0;
  while (GUI_Exec1()) {
    r = 1;                  /* We have done something */
  }
  return r;
}

/*************************** End of file ****************************/
