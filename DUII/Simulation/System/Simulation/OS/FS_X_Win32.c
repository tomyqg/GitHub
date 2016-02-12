/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2009     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
----------------------------------------------------------------------
File        : FS_X_Win32.c
Purpose     : Win32 API OS Layer for the file system
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"
#include "FS_OS.h"
#include "FS_Conf.h"

#include <time.h>
#include <windows.h>
#include <stdio.h>

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/
typedef struct {
  HANDLE hSema;
  char   acName[60];
  int    OpenCnt;
}  LOCK_INST;
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static LOCK_INST * _paInst;
static char        _NumLocks;
static char        _IsInited;
static HANDLE      _hEvent = INVALID_HANDLE_VALUE;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _CheckInit
*
*/
static void  _Exit(void) {
  timeEndPeriod(1);
}

/*********************************************************************
*
*       _CheckInit
*
*/
static void  _CheckInit(void) {
  if (_IsInited == 0) {
    timeBeginPeriod(1);
    atexit(_Exit);
    _IsInited = 1;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_X_OS_Lock
*
*/
void FS_X_OS_Lock(unsigned LockIndex) {
  HANDLE hSema;
  LOCK_INST * pInst;

  pInst = (_paInst + LockIndex);
  if (pInst) {
    hSema = pInst->hSema;
    if (hSema == NULL) {
      return;
    }
    pInst->OpenCnt++;
    FS_DEBUG_LOG((FS_MTYPE_OS, "Locking:   Index: 0x%8x, Name: %s\n", LockIndex, pInst->acName));
    WaitForSingleObject(hSema, INFINITE);
  }
}

/*********************************************************************
*
*       FS_X_OS_Unlock
*
*/
void FS_X_OS_Unlock(unsigned LockIndex) {
  HANDLE hSema;
  LOCK_INST * pInst;

  pInst = (_paInst + LockIndex);
  if (pInst) {
    hSema = pInst->hSema;
    if (hSema == NULL) {
      return;
    }
    FS_DEBUG_LOG((FS_MTYPE_OS, "Unlocking: Index: 0x%8x, Name: %s\n", LockIndex, pInst->acName));
    ReleaseSemaphore(hSema, 1, NULL);
    pInst->OpenCnt--;
  }
}

/*********************************************************************
*
*       FS_X_OS_Init
*
*  Description:
*    Initializes the OS resources. Specifically, you will need to
*    create FS_NUM_LOCKS binary semaphores/mutexs. This function is
*    called by FS_Init(). You should create all resources required
*    by the OS to support multi threading of the file system.
*
*  Parameters:
*    Number of Locks needed.
*
*/
void FS_X_OS_Init(unsigned NumLocks) {
  unsigned i;
  LOCK_INST * pInst;

  _CheckInit();
  _paInst = FS_AllocZeroed(NumLocks * sizeof(LOCK_INST));
  pInst    = &_paInst[0];
  for (i = 0; i < NumLocks; i++) {
    sprintf(pInst->acName, "Filesystem Semaphore %.3d", i);
    pInst->hSema = CreateSemaphore(NULL, 1, 1, pInst->acName);
    if (pInst->hSema == NULL) {
      FS_DEBUG_ERROROUT((FS_MTYPE_OS, "FS_X_OS_Init: Could not create semaphore."));
      return;
    }
    pInst++;
  }
  if (_hEvent == INVALID_HANDLE_VALUE) {
    _hEvent = CreateEvent(NULL, FALSE, FALSE, "FS Event object");
  }
}

/*********************************************************************
*
*       FS_X_OS_DeInit
*
*  Description:
*    Delete all locks that have been created by FS_X_OS_Init().
*    This makes sure that a 
*
*  Parameters:
*    None.
*
*/
void FS_X_OS_DeInit(void) {
  unsigned i;
  LOCK_INST * pInst;
  unsigned    NumLocks;

  NumLocks = _NumLocks;
  pInst    = &_paInst[0];
  for (i = 0; i < NumLocks; i++) {
    CloseHandle(pInst->hSema);
    pInst++;
  }
  FS_Free(_paInst);
  _paInst   = NULL;
  _NumLocks = 0;
}


/*********************************************************************
*
*       FS_X_OS_Delay
*
*/
void FS_X_OS_Delay  (int ms) {
  _CheckInit();
  Sleep(ms);
}

/*********************************************************************
*
*       FS_X_OS_GetTime
*
*/
U32  FS_X_OS_GetTime(void) {
  _CheckInit();
  return timeGetTime();
}

/*********************************************************************
*
*       FS_X_OS_Wait
*
*  Function description:
*    Wait for an event to be signaled.
*
*  Parameters:
*    Timeout    - Time to be wait for the event object.
*  
*  Return value:
*    0       - Event object was signaled within the timeout value
*   -1       - An error or a timeout occurred.
*/
int FS_X_OS_Wait(int Timeout) {
  int r;

  r = -1;
  _CheckInit();
  if (WaitForSingleObject(_hEvent, Timeout) == WAIT_OBJECT_0) {
    r = 0;
  }
  return r;
}

/*********************************************************************
*
*       FS_X_OS_Signal
*
*  Function description:
*    Signals a event
*
*/
void FS_X_OS_Signal(void) {
  _CheckInit();
  SetEvent(_hEvent);
}


/*************************** End of file ****************************/
