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
File        : FS_X_embOS.c
Purpose     : embOS OS Layer for the file system
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include <stdio.h>
#include "FS_Int.h"
#include "FS_OS.h"

#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_RSEMA * _paSema;
static char       _NumLocks;
static OS_EVENT   _Event;
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
  OS_RSEMA * pSema;

  pSema = _paSema + LockIndex;
  if (pSema) {
    OS_Use(pSema);
  }
}

/*********************************************************************
*
*       FS_X_OS_Unlock
*
*/
void FS_X_OS_Unlock(unsigned LockIndex) {
  OS_RSEMA * pSema;

  pSema = _paSema + LockIndex;
  if (pSema) {
    OS_Unuse(pSema);
  }
}

/*********************************************************************
*
*       FS_X_OS_Init
*
*  Description:
*    Initializes the OS resources. Specifically, you will need to
*    create four binary semaphores. This function is called by
*    FS_Init(). You should create all resources required by the
*    OS to support multi threading of the file system.
*
*  Parameters:
*    None.
*
*  Return value:
*    0    - on success
*    -1   - on failure.
*/
void FS_X_OS_Init(unsigned NumLocks) {
  unsigned i;
  OS_RSEMA * pSema;

  _paSema = (OS_RSEMA *)FS_AllocZeroed(NumLocks* sizeof(OS_RSEMA));
  pSema =_paSema;
  for (i = 0; i < NumLocks; i++) {
    OS_CREATERSEMA(pSema++);
  }
  OS_EVENT_Create(&_Event);
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
  OS_RSEMA  * pSema;
  unsigned    NumLocks;

  NumLocks = _NumLocks;
  pSema   = &_paSema[0];
  for (i = 0; i < NumLocks; i++) {
    OS_DeleteRSema(pSema);
    pSema++;
  }
  OS_EVENT_Delete(&_Event);
  FS_Free(_paSema);
  _paSema  = NULL;
  _NumLocks = 0;
}

/*********************************************************************
*
*       FS_X_OS_GetTime
*
*/
U32  FS_X_OS_GetTime(void) {
  return (U32)OS_GetTime32();
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

	if (0 == OS_EVENT_WaitTimed(&_Event, Timeout))
		return 0;
	else
		return -1;

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
  OS_EVENT_Pulse(&_Event);
}


/*************************** End of file ****************************/
