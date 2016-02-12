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
File        : GUITASK.C
Purpose     : Saves/Restores task context with supported OSs.
              It also uses a resource semaphore.

  The following externals are used and should typically be defined
  in GUI_X.c:
  
    U32  GUI_X_GetTaskId();
    void GUI_X_Unlock();
    void GUI_X_Lock();

---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

/*********************************************************************
*
*       Configuration defaults
*
**********************************************************************
*/

#ifndef GUI_MAXTASK
  #define GUI_MAXTASK (4)
#endif

#ifndef GUI_MAX_LOCK_NESTING
  #define GUI_MAX_LOCK_NESTING 48   /* If a lot of nested dialogs are used, this may not be sufficient. */
#endif

#if GUI_OS

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

typedef struct {
  U32  TaskID;
  GUI_CONTEXT Context;
} SAVE_CONTEXT;

static SAVE_CONTEXT * _aSave;

static int _CurrentTaskNo = -1;
static U32 _TaskIDLock    =  0;

static int _MaxTask       = GUI_MAXTASK;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
int GUITASK__EntranceCnt = 0;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetTaskNo
*/
static int _GetTaskNo(void) {
  int i;
  U32 TaskId;

  for (i = 0; i < _MaxTask; i++) {
    TaskId = GUI_X_GetTaskId();
    if (_aSave[i].TaskID == TaskId) {
      return i;
    }
    if (_aSave[i].TaskID == 0) {
      _aSave[i].TaskID = TaskId;
      return i;
    }
  }
  GUI_DEBUG_ERROROUT("No Context available for task. Increase number of maximum tasks!");
  return 0;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_Unlock
*/
void GUI_Unlock(void) {
  if (--GUITASK__EntranceCnt == 0) {
    GUI_X_Unlock();
  }
  /* Test if GUITASK__EntranceCnt is in reasonable range ... Not required in release builds */
  GUI_DEBUG_ERROROUT_IF((GUITASK__EntranceCnt < 0), "GUITASK.c: GUI_Unlock() GUITASK__EntranceCnt underflow ");
}

/*********************************************************************
*
*       GUI_Lock
*/
void GUI_Lock(void) {
  if (GUITASK__EntranceCnt == 0) {
    GUI_X_Lock();
    _TaskIDLock = GUI_X_GetTaskId();         /* Save task ID */
  } else {
    if (_TaskIDLock != GUI_X_GetTaskId()) {
      GUI_X_Lock();
      _TaskIDLock = GUI_X_GetTaskId();         /* Save task ID */
    }
  }
  if (++GUITASK__EntranceCnt == 1) {
    int TaskNo = _GetTaskNo();
    if (TaskNo != _CurrentTaskNo) {
      /* Save data of current task */
      if (_CurrentTaskNo>=0) {  /* Make sure _CurrentTaskNo is valid */
        _aSave[_CurrentTaskNo].Context = GUI_Context;
        /* Load data of this task */
        GUI_Context = _aSave[TaskNo].Context;
      }
      _CurrentTaskNo = TaskNo;
    }
  }
  /* Test if GUITASK__EntranceCnt is in reasonable range ... Not required in release builds */
  GUI_DEBUG_ERROROUT_IF((GUITASK__EntranceCnt > GUI_MAX_LOCK_NESTING), "GUITASK.c: GUI_Lock() GUITASK__EntranceCnt overflow ");
}

/*********************************************************************
*
*       GUITASK_Init
*/
void GUITASK_Init(void) {
  _aSave = (SAVE_CONTEXT *)GUI_ALLOC_GetFixedBlock(sizeof(SAVE_CONTEXT) * _MaxTask);
  while (_aSave == NULL); // Error: Not enough memory. Stop execution here.
  _CurrentTaskNo =-1;   /* Invalidate */
  GUI_X_InitOS();
}


/*********************************************************************
*
*       GUITASK_CopyContext
*/
void GUITASK_CopyContext(void) {
  int i;
  for (i = 0; i < _MaxTask; i++) {
    _aSave[i].Context = GUI_Context;
    _aSave[i].TaskID  = 0;
  }
}

/*********************************************************************
*
*       GUITASK_GetpContext
*/
GUI_CONTEXT * GUITASK_GetpContext(int Index) {
  GUI_CONTEXT * pContext;
  pContext = NULL;
  GUI_Lock();
  if (Index < _MaxTask) {
    pContext = &_aSave[Index].Context;
  }
  GUI_Unlock();
  return pContext;
}

/*********************************************************************
*
*       GUITASK_SetMaxTask
*/
void GUITASK_SetMaxTask(int MaxTask) {
  while (_aSave); // Error: This function needs to be called from GUI_X_Config()!
  _MaxTask = MaxTask;
}

#if defined(WIN32) && defined(_DEBUG)

/*********************************************************************
*
*       GUITASK_AssertLock
*/
void GUITASK_AssertLock(void) {  
  if (!GUITASK__EntranceCnt) {
    GUI_DEBUG_ERROROUT("GUITASK.c: GUITASK_AssertLock failed!");
  }
}

/*********************************************************************
*
*       GUITASK_AssertNoLock
*/
void GUITASK_AssertNoLock(void) {  
  GUI_Lock();
  if (GUITASK__EntranceCnt > 1) {
    GUI_DEBUG_ERROROUT("GUITASK.c: GUITASK_AssertNoLock failed!");
  }
  GUI_Unlock();
}

#endif

#else

/*********************************************************************
*
*       Dummy Kernel routines
*
* The routines below are dummies in case configuration tells us not
* to use any kernel. In this case the routines below should
* not be required, but it can not hurt to have them. The linker
* will eliminate them anyhow.
*/
void GUI_Unlock(void);
void GUI_Unlock(void) {}
void GUI_Lock(void);
void GUI_Lock(void) {}
void GUITASK_Init(void);
void GUITASK_Init(void) {}
void GUITASK_StoreDefaultContext(void);
void GUITASK_StoreDefaultContext(void) {}

#endif

/*************************** End of file ****************************/
