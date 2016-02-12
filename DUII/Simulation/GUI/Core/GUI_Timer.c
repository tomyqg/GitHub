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
File        : GUITimer.c
Purpose     : Supplies timers
----------------------------------------------------------------------
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define GUI_TIMER_H2P(h) (GUI_TIMER_Obj*)GUI_ALLOC_h2p(h)

#ifdef GUI_TIMER_MAXVALUE
  #define CHECK_LIMIT(v) if (v > GUI_TIMER_MAXVALUE) v = 0
#else
  #define CHECK_LIMIT(v)
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  GUI_TIMER_CALLBACK* cb;
  GUI_TIMER_HANDLE hNext;
  int Flags;
	U32 Context;
	GUI_TIMER_TIME t0;
	GUI_TIMER_TIME Period;
} GUI_TIMER_Obj;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

GUI_TIMER_HANDLE _hFirstTimer;
GUI_TIMER_HANDLE _hActiveTimer;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Unlink
*/
static void _Unlink(GUI_TIMER_HANDLE hTimer) {
  GUI_TIMER_Obj * pTimer;
  GUI_TIMER_Obj * pi;
  GUI_TIMER_HANDLE hi;
  pTimer = GUI_TIMER_H2P(hTimer);
/* Check if it is the first element */
  if (_hFirstTimer == hTimer) {
    _hFirstTimer = pTimer->hNext;
    return;
	}
  hi = _hFirstTimer;
/* Try to find it in the list ... */
  while(hi) {
    /* GUI_ASSERT(hi<1000,0); */
    pi = GUI_TIMER_H2P(hi);
    if (pi->hNext == hTimer) {
      pi->hNext = pTimer->hNext;
      break;
		}        
    hi = pi->hNext;
  }  
}

/*********************************************************************
*
*       _Link
*
* Purpose:
*   This routine inserts the new timer (referenced by its handle) into
*	  the linked list. The linked list is sorted according to timestamps.
*	  The first element is the timer which expires first.
*/
static void _Link(GUI_TIMER_HANDLE hNew) {
  GUI_TIMER_Obj * pNew;
  GUI_TIMER_Obj * pTimer;
  GUI_TIMER_Obj * pNext;
  GUI_TIMER_HANDLE hNext;
  pNew = GUI_TIMER_H2P(hNew);
  if (_hFirstTimer ==0) { /* List is empty, make it the only element */
    _hFirstTimer = hNew;
	  pNew->hNext = 0;
  } else {
    GUI_TIMER_Obj* pFirstTimer      = GUI_TIMER_H2P(_hFirstTimer);
/* Check if we have to make it the first element */
    if ((pNew->t0 - pFirstTimer->t0) <=0) {
      pNew->hNext  = _hFirstTimer;
      _hFirstTimer = hNew;
			return;
		} else {
      GUI_TIMER_HANDLE hTimer = _hFirstTimer;
/* Put it into the list */
      do {
        pTimer = GUI_TIMER_H2P(hTimer);
        hNext  = pTimer->hNext;
        if (hNext == 0) {
					goto Append;
        }
        pNext  = GUI_TIMER_H2P(hNext);
				if ((pNew->t0 - pNext->t0) <= 0) {
          pNew->hNext   = hNext;
          pTimer->hNext = hNew;
          return;
				}
        hTimer = pTimer->hNext; 
			} while(1);
/* Put it at the end of the list */
Append:
      pNew->hNext   = hNext;
      pTimer->hNext = hNew;
      return;
		}
  }
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_TIMER__IsActive
*/
int GUI_TIMER__IsActive(void) {
  int r;
  r = 0;
  GUI_LOCK(); {
    if (_hFirstTimer) {
      r = 1;
    }
  } GUI_UNLOCK(); 
  return r;
}

/*********************************************************************
*
*       GUI_TIMER__GetPeriod
*/
GUI_TIMER_TIME GUI_TIMER__GetPeriod(void) {
  GUI_TIMER_TIME r;
  r = 0;
  GUI_LOCK(); {
    if (_hFirstTimer) {
      GUI_TIMER_TIME t;
     	GUI_TIMER_Obj * pTimer = GUI_TIMER_H2P(_hFirstTimer);
      t = GUI_GetTime();
      r = pTimer->t0 - t;
      if (r < 0) {
        r = 0;
      }
    }
  } GUI_UNLOCK(); 
  return r;
}

/*********************************************************************
*
*       GUI_TIMER__GetNextTimer
*/
GUI_TIMER_HANDLE GUI_TIMER__GetNextTimer(GUI_TIMER_HANDLE hTimer, U32 * pContext) {
  GUI_TIMER_Obj  * pTimer;

  *pContext = 0;
  if (hTimer == 0) {
    hTimer = _hFirstTimer;
    if (hTimer) {
   	  pTimer = GUI_TIMER_H2P(hTimer);
      *pContext = pTimer->Context;
    }
  } else {
   	pTimer = GUI_TIMER_H2P(hTimer);
    hTimer = pTimer->hNext;
    if (hTimer) {
   	  pTimer = GUI_TIMER_H2P(hTimer);
      *pContext = pTimer->Context;
    }
  }
  return hTimer;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_TIMER_Exec
*/
int GUI_TIMER_Exec(void) {
  int r;
  r = 0;
  GUI_LOCK(); {
    while (_hFirstTimer) {
      GUI_TIMER_TIME t;
     	GUI_TIMER_Obj * pTimer = GUI_TIMER_H2P(_hFirstTimer);
      t = GUI_GetTime();
      if ((pTimer->t0 - t) <= 0) {
        GUI_TIMER_MESSAGE tm;
        tm.Time       = t;
				tm.Context    = pTimer->Context;
        _hActiveTimer = _hFirstTimer;
        _hFirstTimer  = pTimer->hNext;
        pTimer->cb(&tm);
        r = 1;
			} else {
			  break;
      }
    }
  } GUI_UNLOCK(); 
  return r;
}

/*********************************************************************
*
*       GUI_TIMER_Create
*/
GUI_TIMER_HANDLE GUI_TIMER_Create(GUI_TIMER_CALLBACK* cb, int Time, U32 Context, int Flags) {
  GUI_TIMER_HANDLE hObj;
  GUI_TIMER_Obj* pObj;
  GUI_LOCK(); {
    GUI_USE_PARA(Flags);
    /* Alloc memory for obj */
    hObj = GUI_ALLOC_AllocZero(sizeof(GUI_TIMER_Obj));
    if (hObj) {
      GUI_pfTimerExec = GUI_TIMER_Exec;
      pObj = GUI_TIMER_H2P(hObj);
      /* init member variables */
      CHECK_LIMIT(Time);
      pObj->t0 = Time;
      pObj->cb = cb;
	    pObj->Context = Context;
      /* Link it */
	    _Link(hObj);
    }
  } GUI_UNLOCK(); 
  return hObj;
}

/*********************************************************************
*
*       GUI_TIMER_Delete
*/
void GUI_TIMER_Delete(GUI_TIMER_HANDLE hObj) {
/* Unlink Timer */
  GUI_LOCK();
  _Unlink(hObj);
  GUI_ALLOC_Free(hObj);
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_TIMER_SetPeriod
*/
void GUI_TIMER_SetPeriod(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Period) {
  GUI_LOCK(); {
    GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->Period = Period;
  } GUI_UNLOCK(); 
}

/*********************************************************************
*
*       GUI_TIMER_SetTime
*/
void GUI_TIMER_SetTime(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Time) {
  GUI_LOCK(); {
   	GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = Time;
    CHECK_LIMIT(pObj->t0);
  } GUI_UNLOCK(); 
}

/*********************************************************************
*
*       GUI_TIMER_SetDelay
*/
void GUI_TIMER_SetDelay(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Delay) {
  GUI_LOCK(); {
   	GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = Delay;
		_Unlink(hObj);
		_Link(hObj);
  } GUI_UNLOCK(); 
}

/*********************************************************************
*
*       GUI_TIMER_Restart
*/
void GUI_TIMER_Restart(GUI_TIMER_HANDLE hObj) {
  GUI_TIMER_Obj* pObj;
  GUI_LOCK(); {
    if (hObj == 0) {
      hObj = _hActiveTimer;
    }
   	pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = GUI_GetTime() +pObj->Period;
    CHECK_LIMIT(pObj->t0);
    _Unlink(hObj);
    _Link(hObj);
  } GUI_UNLOCK(); 
}

/*************************** End of file ****************************/
