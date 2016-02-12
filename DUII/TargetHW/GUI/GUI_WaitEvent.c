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
File        : GUI_WaitEvent.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_WaitEvent
*
*  Function description
*    This function is called from a task which needs to wait for an event.
*    The GUI-Lock will be freed when the task is suspended; the lock state
*    is restored when execution continues.
*/
void GUI_WaitEvent(void) {
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
#ifdef GUI_X_WAIT_EVENT_TIMED
  if (GUI_TIMER__IsActive()) {
    GUI_TIMER_TIME Period;
    Period = GUI_TIMER__GetPeriod();
    if (Period) {
      GUI_X_WAIT_EVENT_TIMED(Period);
    } else {
      GUI_X_WAIT_EVENT();
    }
  } else {
    GUI_X_WAIT_EVENT();
  }
#else
  GUI_X_WAIT_EVENT();
#endif
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

/*************************** End of file ****************************/
