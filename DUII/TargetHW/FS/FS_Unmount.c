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
File        : FS_Unmount.c
Purpose     : Implementation of FS_Unmount..
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_Int.h"


/*********************************************************************
*
*       _Unmount
*
*  Description:
*    Internal version of unmounting a volume
*    Closes all open file and directory handles to the volume, marks the volume as unmounted
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*/
static void _Unmount(FS_VOLUME * pVolume) {
  FS_DEVICE  * pDevice;
  
  pDevice = &pVolume->Partition.Device;
  FS_LOCK_DRIVER(pDevice);
  if (pVolume->IsMounted) {
    FS_FILE    * pFile;
    
    FS_JOURNAL_INVALIDATE(pVolume);              // Note: If a transaction on the journal is running, data in journal is purposly discarded!
    //
    // Close all open files on this volume
    //
    pFile = FS_Global.pFirstFilehandle;
    while (pFile){
      FS_FILE_OBJ * pFileObj;
      char FileIsOnThisVolume;
      
      //
      // Check if file is on this volume. SYS-Lock is required when going thru the data structures.
      //
      FileIsOnThisVolume = 0;
      FS_LOCK_SYS();
      if (pFile->InUse) {
        pFileObj = pFile->pFileObj;
        if (pFileObj->pVolume == pVolume) {
          FileIsOnThisVolume = 1;
        }
      }
      FS_UNLOCK_SYS();
      //
      // Close file if it is on this volume
      //
      if (FileIsOnThisVolume) {
        FS__FCloseNL(pFile);
      }
      pFile = pFile->pNext;
    }
  #if FS_SUPPORT_CACHE
    //
    // Clean cache
    //
    FS__CACHE_CommandDeviceNL(pDevice, FS_CMD_CACHE_CLEAN,  NULL);
    //
    // Invalidate cache
    //
    FS__CACHE_CommandDeviceNL(pDevice, FS_CMD_CACHE_INVALIDATE,  pVolume->Partition.Device.Data.pCacheData);
  #endif
    FS_CLEAN_FS(pVolume);
  }
  FS__IoCtlNL(pVolume, FS_CMD_UNMOUNT, 0, NULL);           // Send unmount command to driver
  FS_LOCK_SYS();
  pVolume->IsMounted = 0;                                  // Mark volume as unmounted
  pDevice->Data.IsInited = 0;
  pVolume->Partition.StartSector = 0;
  pVolume->Partition.NumSectors  = 0;
  FS_UNLOCK_SYS();
  FS_UNLOCK_DRIVER(pDevice);

}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__UnmountNL
*
*  Description:
*    Internal version of unmounting a device
*    Closes all open file and directory handles to the volume, marks the volume as unmounted
*    Does not perform locking.
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*/
void FS__UnmountNL(FS_VOLUME * pVolume) {
  FS_DEVICE  * pDevice;
  
  pDevice = &pVolume->Partition.Device;
  FS_LOCK_DRIVER(pDevice);
  if (pVolume->IsMounted) {
    FS_FILE    * pFile;
    
    FS_JOURNAL_INVALIDATE(pVolume);              // Note: If a transaction on the journal is running, data in journal is purposly discarded!
    //
    // Close all open files on this volume
    //
    pFile = FS_Global.pFirstFilehandle;
    while (pFile){
      FS_FILE_OBJ * pFileObj;
      char FileIsOnThisVolume;
      
      //
      // Check if file is on this volume. SYS-Lock is required when going thru the data structures.
      //
      FileIsOnThisVolume = 0;
      FS_LOCK_SYS();
      if (pFile->InUse) {
        pFileObj = pFile->pFileObj;
        if (pFileObj->pVolume == pVolume) {
          FileIsOnThisVolume = 1;
        }
      }
      FS_UNLOCK_SYS();
      //
      // Close file if it is on this volume
      //
      if (FileIsOnThisVolume) {
        FS__FCloseNL(pFile);
      }
      pFile = pFile->pNext;
    }
  #if FS_SUPPORT_CACHE
    //
    // Clean cache
    //
    FS__CACHE_CommandDeviceNL(pDevice, FS_CMD_CACHE_CLEAN,  NULL);
    //
    // Invalidate cache
    //
    FS__CACHE_CommandDeviceNL(pDevice, FS_CMD_CACHE_INVALIDATE,  pVolume->Partition.Device.Data.pCacheData);
  #endif
  }
  FS__IoCtlNL(pVolume, FS_CMD_UNMOUNT, 0, NULL);           // Send unmount command to driver
  FS_LOCK_SYS();
  pVolume->IsMounted = 0;                                  // Mark volume as unmounted
  pDevice->Data.IsInited = 0;
  pVolume->Partition.StartSector = 0;
  pVolume->Partition.NumSectors  = 0;
  FS_UNLOCK_SYS();
  FS_UNLOCK_DRIVER(pDevice);
}

/*********************************************************************
*
*       FS__UnmountForcedNL
*
*  Description:
*    Unmounts a devices.
*    Invalidates all open file and directory handles to the volume, marks the volume as unmounted
*    Does not perform locking.
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*
*/
void FS__UnmountForcedNL(FS_VOLUME * pVolume) {
  FS_DEVICE * pDevice;
  pDevice = &pVolume->Partition.Device;
  if (pVolume->IsMounted) {
    FS_FILE   * pFile;
    
    FS_JOURNAL_INVALIDATE(pVolume);
    //
    // Invalidate all open handles on this volume
    //
    FS_LOCK_SYS();
    pFile    = FS_Global.pFirstFilehandle;
    while (pFile) {
      FS_FILE_OBJ * pFileObj;
      //
      // Check if file is on this volume. SYS-Lock is required when going thru the data structures.
      //
      if (pFile->InUse) {
        pFileObj = pFile->pFileObj;
        if (pFileObj) {
          if (pFileObj->pVolume == pVolume) {
            pFile->InUse    = 0;
            pFile->pFileObj = (FS_FILE_OBJ*)NULL;
            if (pFileObj->UseCnt) {
              pFileObj->UseCnt--;   // Could also be cleared to 0
            }
          }
        }
      }
      pFile = pFile->pNext;
    }
    FS_UNLOCK_SYS();
  }
#if FS_SUPPORT_CACHE
  //
  // Invalidate cache
  //
  FS__CACHE_CommandDeviceNL(pDevice, FS_CMD_CACHE_INVALIDATE,  pVolume->Partition.Device.Data.pCacheData);
#endif
  FS__IoCtlNL(pVolume, FS_CMD_UNMOUNT_FORCED, 0, NULL);           // Send unmount command to driver
  FS_LOCK_SYS();
  pVolume->IsMounted = 0;                                  // Mark volume as unmounted
  pDevice->Data.IsInited = 0;
  pVolume->Partition.StartSector = 0;
  pVolume->Partition.NumSectors  = 0;
  FS_UNLOCK_SYS();
  FS_JOURNAL_INVALIDATE(pVolume);
}

/*********************************************************************
*
*       FS__UnmountForced
*
*  Description:
*    Unmounts a devices.
*    Invalidates all open file and directory handles to the volume, marks the volume as unmounted
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*
*/
void FS__UnmountForced(FS_VOLUME * pVolume) {
  FS_LOCK_DRIVER(&pVolume->Partition.Device);
  FS__UnmountForcedNL(pVolume);
  FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
}

/*********************************************************************
*
*       FS__Unmount
*
*  Description:
*    Internal version of unmounting a device
*    Closes all open file and directory handles to the volume, marks the volume as unmounted
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*/
void FS__Unmount(FS_VOLUME * pVolume) {
  int Status;
  FS_LOCK_DRIVER(&pVolume->Partition.Device);
  Status = FS_LB_GetStatus(&pVolume->Partition.Device);
  FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
  if (Status == FS_MEDIA_NOT_PRESENT) {
    FS__UnmountForced(pVolume);
  } else {
    _Unmount(pVolume);
  }
}

/*********************************************************************
*
*       FS_Unmount
*
*  Description:
*    Unmounts a device
*    Closes all open file and directory handles to the volume, marks the volume as unmounted
*
*  Parameters:
*    sVolume            - The volume name.
*/
void FS_Unmount(const char * sVolume) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    FS__Unmount(pVolume);
  }
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_UnmountForced
*
*  Description:
*    Unmounts a device
*    Closes all open file and directory handles to the volume, marks the volume as unmounted
*
*  Parameters:
*    sVolume            - The volume name.
*/
void FS_UnmountForced(const char * sVolume) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    FS__UnmountForced(pVolume);
  }
  FS_UNLOCK();
}

/*************************** End of file ****************************/
