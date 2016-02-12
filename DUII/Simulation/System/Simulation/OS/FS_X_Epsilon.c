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
File        : FS_X_Epsilon.c
Purpose     : OSE Epsilon OS Layer for the file system
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

#include "ose.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static SEMAPHORE *FS_Sema[FS_NUM_LOCKS];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


/*********************************************************************
*
*         FS_X_OS_Lock
*
*  Description:
*    Locks FS_Sema[LockIndex] semaphore.
*
*/
void FS_X_OS_Lock(unsigned LockIndex) {
   wait_sem(&FS_Sema[LockIndex]);
}


/*********************************************************************
*
*       FS_X_OS_Unlock
*
*  Description:
*    Unlocks FS_Sema[LockIndex] semaphore.
*
*/
void FS_X_OS_Unlock(unsigned LockIndex) {
   signal_sem(&FS_Sema[LockIndex]);
}

/*********************************************************************
*
*             FS_X_OS_init
*
*  Description:
*    Initializes the OS resources.
*
*  Parameters:
*    None.
*
*  Return value:
*    0    - on success
*    -1   - on failure.
*/
int FS_X_OS_Init(void) {
  unsigned i;

  for (i = 0; i < FS_NUM_LOCKS; i++) {
    FS_Sema[i]= create_sem(1);
  }
  return 0;
}

/*************************** End of file ****************************/
