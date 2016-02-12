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
File        : GUI_Time.c
Purpose     : Time related routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Delay
*/
static void _Delay(int Period) {
#if GUI_OS
  unsigned LockCnt;
  unsigned i;

  GUI_LOCK();
  LockCnt = GUITASK__EntranceCnt;
  /*
   * Reset lock count
   */
  GUI_ALLOC_Lock();     /* Memory lock */
  for (i = LockCnt; i; i--) {
    GUI_Unlock();       /* Dec task lock */
  }
#endif
  /*
   * Wait a while
   */
  GUI_X_Delay(Period);
#if GUI_OS
  /*
   * Restore lock count
   */
  for (i = LockCnt; i; i--) {
    GUI_Lock();         /* Inc task lock */
  }
  GUI_ALLOC_Unlock()  ; /* Memory unlock */
  GUI_UNLOCK();
#endif
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetTime
*/
int GUI_GetTime(void) {
  return GUI_X_GetTime();
}

/*********************************************************************
*
*       GUI_Delay
*/
void GUI_Delay(int Period) {
  int EndTime;
  int tRem; /* remaining Time */
  EndTime = GUI_GetTime() + Period;
  while (tRem = EndTime - GUI_GetTime(), tRem > 0) {
    GUI_Exec();
    _Delay((tRem > 5) ? 5 : tRem);
  }
}

/*************************** End of file ****************************/
