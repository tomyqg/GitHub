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
File        : HEADER_SetFixed.c
Purpose     : Implementation of HEADER_SetFixed
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetFixed
*/
unsigned HEADER_SetFixed(HEADER_Handle hObj, unsigned Fixed) {
  unsigned FixedOld = 0;
  if (hObj) {
    HEADER_Obj* pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    FixedOld = pObj->Fixed;
    if (Fixed != FixedOld) {
      pObj->Fixed = Fixed;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return FixedOld;
}

#else                            /* Avoid problems with empty object modules */
  void HEADER_SetFixed_C(void);
  void HEADER_SetFixed_C(void) {}
#endif

/*************************** End of file ****************************/
