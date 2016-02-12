/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
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
File        : FS_OS_Interface.c
Purpose     : File system OS interface
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include <stdlib.h>

#include "FS_Int.h"
#include "FS_OS.h"
#include "FS_Lbl.h"
#include "FS_CLib.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static char _IsInited = 0;

#if ((FS_OS) && (FS_OS_LOCK_PER_DRIVER))              /* One lock per driver */

/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/
typedef struct DRIVER_LOCK DRIVER_LOCK;


struct DRIVER_LOCK  {
  DRIVER_LOCK          * pNext;
  U8                     Id;
  const FS_DEVICE_TYPE * pDriver;
  U8                     References;
};

/*********************************************************************
*
*       Additional static data
*
**********************************************************************
*/
static int           _NumDriverLocks;
static DRIVER_LOCK * _pDriverLock;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _AddDriver
*
* Function description
*   Adds a driver to the lock list.
*   If the driver is already in the list, its reference count is incremented;
*   if not, a memory block is allocated and added to the lock list as last element.
*/
static void _AddDriver(const FS_DEVICE_TYPE * pDriver) {
  DRIVER_LOCK * pDriverLock;
  DRIVER_LOCK ** ppPrev;

  pDriverLock = _pDriverLock;
  ppPrev       = &_pDriverLock;
  do {
    if (pDriverLock == NULL) {
      pDriverLock = (DRIVER_LOCK * )FS_AllocZeroed(sizeof(DRIVER_LOCK));
      pDriverLock->Id       = _NumDriverLocks++;
      pDriverLock->pDriver  = pDriver;
      pDriverLock->References++;
      *ppPrev = pDriverLock;
      break;
    }

    if (pDriverLock->pDriver == pDriver) {
      pDriverLock->References++;
      break;
    }
    ppPrev      = &pDriverLock->pNext;
    pDriverLock = pDriverLock->pNext;
    
  } while (1);

}


/*********************************************************************
*
*       _RemoveDriver
*
* Function description
*   Removes a driver from the lock list, in case the reference count is zero;
*
*/
static void _RemoveDriver(const FS_DEVICE_TYPE * pDriver) {
  DRIVER_LOCK * pDriverLock;
  DRIVER_LOCK ** ppPrev;

  pDriverLock = _pDriverLock;
  ppPrev       = &_pDriverLock;

  do {
    if (pDriver == pDriverLock->pDriver) {
      if (--pDriverLock->References == 0) {
        (*ppPrev)= pDriverLock->pNext;
        FS_Free(pDriverLock);
        _NumDriverLocks--;
        break;
      }
    }
    ppPrev      = &pDriverLock;
    pDriverLock = pDriverLock->pNext;
  } while (pDriverLock);

}


/*********************************************************************
*
*       _Driver2Id
*
* Function description
*   Retrieves the lock Id of the device driver.
*   The lock Id is unique for every device driver.
*/
static unsigned _Driver2Id(const FS_DEVICE_TYPE * pDriver) {
  DRIVER_LOCK * pDriverLock;

  pDriverLock = _pDriverLock;
  do {
    if (pDriverLock->pDriver == pDriver) {
      return pDriverLock->Id;
    }
    pDriverLock = pDriverLock->pNext;
  }  while (pDriverLock);
  FS_DEBUG_ERROROUT((FS_MTYPE_OS, "_Driver2Id: Driver was not in driver lock list."));
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
*       FS_OS_AddDriver
*
*/
void FS_OS_AddDriver(const FS_DEVICE_TYPE * pDriver) {
  _AddDriver(pDriver);
}

/*********************************************************************
*
*       FS_OS_RemoveDriver
*
*/
void FS_OS_RemoveDriver(const FS_DEVICE_TYPE * pDriver) {
  _RemoveDriver(pDriver);
}

/*********************************************************************
*
*       FS_OS_LockDriver
*
*/
void  FS_OS_LockDriver(const FS_DEVICE * pDevice) {
  unsigned int LockIndex;

  LockIndex = 0;
  if (pDevice) {
    LockIndex = _Driver2Id(pDevice->pType);
  }
  LockIndex += FS_LOCK_ID_DEVICE;
  FS_X_OS_Lock(LockIndex);    
}

/*********************************************************************
*
*       FS_OS_UnlockDriver
*
*/
void  FS_OS_UnlockDriver(const FS_DEVICE * pDevice) {
  unsigned int LockIndex;

  LockIndex = 0;
  if (pDevice) {
    LockIndex = _Driver2Id(pDevice->pType);
  }
  LockIndex += FS_LOCK_ID_DEVICE;
  FS_X_OS_Unlock(LockIndex);    
}

/*********************************************************************
*
*       FS_OS_GetNumDriverLocks
*
*/
unsigned FS_OS_GetNumDriverLocks(void) {
  return _NumDriverLocks;
}

#endif

/*********************************************************************
*
*       FS_OS_Lock
*
*/
void FS_OS_Lock(unsigned LockIndex) {
  if (_IsInited) {
    FS_X_OS_Lock(LockIndex);
  }
}

/*********************************************************************
*
*       FS_OS_Unlock
*
*/
void FS_OS_Unlock(unsigned LockIndex) {
  if (_IsInited) {
    FS_X_OS_Unlock(LockIndex);
  }
}

/*********************************************************************
*
*       FS_OS_Init
*
*/
void FS_OS_Init(unsigned NumLocks) {
  _IsInited = 1;
  FS_X_OS_Init(NumLocks);
}

/*********************************************************************
*
*       FS_OS_DeInit
*
*/
void FS_OS_DeInit(void) {
  _IsInited = 0;
  FS_X_OS_DeInit();
}

/*************************** End of file ****************************/
