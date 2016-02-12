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
File        : WM__GetOrg_AA.c
Purpose     : Windows manager routine
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if (GUI_WINSUPPORT)    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM__GetOrgX_AA
*/
int WM__GetOrgX_AA(void) {
  int r;
  r = GUI_Context.xOff;
  if (GUI_Context.AA_HiResEnable) {
    r *= GUI_Context.AA_Factor;
  }
  return r;
}

/*********************************************************************
*
*       WM__GetOrgY_AA
*/
int WM__GetOrgY_AA(void) {
  int r;
  r = GUI_Context.yOff;
  if (GUI_Context.AA_HiResEnable) {
    r *= GUI_Context.AA_Factor;
  }
  return r;
}

#else
  void WM__GetOrg_AA_c(void);
  void WM__GetOrg_AA_c(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
