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
File        : WM_PID__GetPrevState.c
Purpose     : Touch support
----------------------------------------------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "WM_Intern.h"

#if (GUI_WINSUPPORT)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GUI_PID_STATE _aPrevState[GUI_NUM_LAYERS];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_PID__GetPrevState
*/
void WM_PID__GetPrevState(GUI_PID_STATE * pPrevState, int Layer) {
  *pPrevState = _aPrevState[Layer];
}

/*********************************************************************
*
*       WM_PID__SetPrevState
*/
void WM_PID__SetPrevState(GUI_PID_STATE * pPrevState, int Layer) {
  _aPrevState[Layer] = *pPrevState;
}

#else
  void WM_PID__GetPrevState_c(void) {} /* avoid empty object files */
#endif  /* (GUI_WINSUPPORT) */

/*************************** End of file ****************************/

