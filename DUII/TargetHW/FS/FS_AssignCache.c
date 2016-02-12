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
File        : FS_AssignCache.c
Purpose     : Implementation of Cache functions
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

#if (FS_SUPPORT_CACHE)

/*********************************************************************
*
*       Global code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_AssignCache
*
*  Function description:
*    Adds a cache to a device
*
*  Return value
*    Number of sectors which fit in cache
*/
U32 FS_AssignCache(const char * pName, void * pData, I32 NumBytes, FS_INIT_CACHE * pfInit) {
  U32 r;
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(pName, NULL);
  r = 0;
  if (pVolume) {
    FS_DEVICE    * pDevice;

    pDevice   = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    if (pDevice->Data.pCacheAPI) {
      FS__CACHE_CommandVolume(pVolume, FS_CMD_CACHE_CLEAN, NULL);
    }
    
    if (NumBytes == 0) {
      pfInit = NULL;
    }
    if (pData == NULL) {
      pfInit = NULL;
    }
    if (pfInit) {
      r = (*pfInit)(pDevice, pData, NumBytes);
    } else {
      pDevice->Data.pCacheAPI  = NULL;
      pDevice->Data.pCacheData = NULL;
      r = 0;
    }
    FS_UNLOCK_DRIVER(pDevice);
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS__CACHE_CommandDeviceNL
*
*  Function description:
*    Function that sends a command to a cache module, if attached to the specific volume.
*    This function does not lock.
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
int FS__CACHE_CommandDeviceNL(FS_DEVICE * pDevice, int Cmd, void * pData) {
  int r;
  const FS_CACHE_API * pCacheAPI;

  r = -1;
  pCacheAPI = pDevice->Data.pCacheAPI;
  if (pCacheAPI) {
    if (pCacheAPI->pfCommand) {
      r = pCacheAPI->pfCommand(pDevice, Cmd, pData);
    }
  }
  return r;
}


/*********************************************************************
*
*       FS__CACHE_CommandDevice
*
*  Function description:
*    Function that sends a command to a cache module, if attached to the specific volume.
*    This function does a driver lock and simply calls the non-locking function.
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
int FS__CACHE_CommandDevice(FS_DEVICE * pDevice, int Cmd, void * pData) {
  int r;

  r = -1;
  FS_LOCK_DRIVER(pDevice);
  r = FS__CACHE_CommandDeviceNL(pDevice, Cmd, pData);
  FS_UNLOCK_DRIVER(pDevice);
  return r;
}



/*********************************************************************
*
*       FS__CACHE_CommandVolume
*
*  Function description:
*    Function that sends a command to a cache module, if attached to the specific volume.
*    This function extracts the device pointer from pVolume and calls the
*    FS__CACHE_CommandDevice.
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
int FS__CACHE_CommandVolume(FS_VOLUME * pVolume, int Cmd, void * pData) {
  int r;

  r = -1;
  if (pVolume) {
    FS_DEVICE * pDevice;

    pDevice = &pVolume->Partition.Device;
    r = FS__CACHE_CommandDevice(pDevice, Cmd, pData);
  }
  return r;
}

/*********************************************************************
*
*       FS__CACHE_CleanNL
*
*  Function description:
*    Function that sends a command to a cache module, if attached to the specific volume.
*    This function extracts the device pointer from pVolume and calls the
*    FS__CACHE_CommandDevice.
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
void  FS__CACHE_CleanNL(FS_DEVICE * pDevice) {
  FS__CACHE_CommandDeviceNL(pDevice,  FS_CMD_CACHE_CLEAN, NULL);
}

/*********************************************************************
*
*       FS__CACHE_Clean
*
*  Function description:
*    Function that sends a command to a cache module, if attached to the specific volume.
*    This function extracts the device pointer from pVolume and calls the
*    FS__CACHE_CommandDevice.
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
void  FS__CACHE_Clean(FS_VOLUME * pVolume) {
  FS_DEVICE * pDevice;

  pDevice = &pVolume->Partition.Device;
  FS_LOCK_DRIVER(pDevice);
  FS__CACHE_CleanNL(pDevice);
  FS_UNLOCK_DRIVER(pDevice);
}

/*********************************************************************
*
*       FS_CACHE_Command
*
*  Function description:
*    Sends a command to cache module.
*
*  Return value:
*    Unsupported command:    -1
*    Supported commands:     <0: Error. Precise value depends on command
*/
int FS_CACHE_Command(const char * pName, int Cmd, void * pData) {
  int r;
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(pName, NULL);
  r = FS__CACHE_CommandVolume(pVolume, Cmd, pData);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_CACHE_SetMode
*
*  Function description:
*    Sets the mode of a specific cache.
*/
int FS_CACHE_SetMode(const char * pName, int TypeMask, int ModeMask) {
  int r;
  CACHE_MODE CacheMode;

  CacheMode.TypeMask = TypeMask;
  CacheMode.ModeMask = ModeMask;
  r = FS_CACHE_Command(pName, FS_CMD_CACHE_SET_MODE, &CacheMode);
  return r;
}

/*********************************************************************
*
*       FS_CACHE_SetQuota
*
*  Function description:
*    Sets the quotas of a specific drive cache.
*/
int FS_CACHE_SetQuota(const char * pName, int TypeMask, U32 NumSectors) {
  int r;
  CACHE_QUOTA CacheQuota;

  CacheQuota.TypeMask   = TypeMask;
  CacheQuota.NumSectors = NumSectors;
  r = FS_CACHE_Command(pName, FS_CMD_CACHE_SET_QUOTA, &CacheQuota);
  return r;
}

/*********************************************************************
*
*       FS_CACHE_Clean
*
*  Function description:
*    Cleans the cache module if any dirty sectors need to be written.
*
*/
void FS_CACHE_Clean(const char * pName) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(pName, NULL);
  if (pVolume) {
    FS__CACHE_Clean(pVolume);
  }
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_CACHE_Invalidate
*
*  Function description:
*    Invalidates the cache entries.
*
*  Parameters:
*    sVolumeName    - name of the volume to invalidate the cache.
*  
*  Return value:
*    == 0      - Success
*    != 0      - Error
*/
int FS_CACHE_Invalidate(const char * sVolumeName) {
  FS_VOLUME * pVolume;
  int         r = -1;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolumeName, NULL);
  if (pVolume) {
    r = FS__CACHE_CommandVolume(pVolume, FS_CMD_CACHE_INVALIDATE, pVolume->Partition.Device.Data.pCacheData);
  }
  FS_UNLOCK();
  return r;
}


#endif /* FS_SUPPORT_CACHE */

/*************************** End of file ****************************/
