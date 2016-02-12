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
File        : FS_Storage.c
Purpose     : Implementation of Storage API functions
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#define FS_STORAGE_C

#include "FS_Int.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       Public data for internal use
*
**********************************************************************
*/

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _AllocVolumeHandle
*
*  Function description:
*    Returns a free volume handle.
*
*  Return value:
*    pVolume     - A valid free volume handle
*
*/
static FS_VOLUME * _AllocVolumeHandle(void) {
  FS_VOLUME * pVolume;

  FS_LOCK_SYS();
  pVolume = &FS_Global.FirstVolume;
  while (1) { /* While no free entry found. */
    if (pVolume->InUse == 0) {
      FS_VOLUME * pNext;
      //
      // Save the pNext pointer to restore it back.
      //
      pNext = pVolume->pNext;
      FS_MEMSET(pVolume, 0, sizeof(FS_VOLUME));
      pVolume->InUse = 1;
      pVolume->pNext = pNext;
      break;
    }
    if (pVolume->pNext == NULL) {
      pVolume->pNext = (FS_VOLUME *)FS_TryAlloc(sizeof(FS_VOLUME));
      if (pVolume->pNext) {
        FS_MEMSET(pVolume->pNext, 0, sizeof(FS_VOLUME));
      }
    }
    pVolume = pVolume->pNext;
    //
    // Neither a free volume handle found 
    // nor enough space to allocate a new one.
    //
    if (pVolume == NULL) {
      break;
    }
  }
  FS_UNLOCK_SYS();
  return pVolume;
}

/*********************************************************************
*
*       Public code, internal API functions
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__IoCtlNL
*
*  Function description:
*    Internal version of FS_IoCtl.
*    Execute device command.
*
*  Parameters:
*    pVolume     - Pointer to the device structure.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value:
*    Command specific. In general a negative value means an error.
*/
int FS__IoCtlNL(FS_VOLUME * pVolume, I32 Cmd, I32 Aux, void *pBuffer) {
  int Status;

  switch (Cmd) {
  case FS_CMD_UNMOUNT:
  case FS_CMD_UNMOUNT_FORCED:
  case FS_CMD_SYNC:
  case FS_CMD_DEINIT:
    return FS_IOCTL(&pVolume->Partition.Device, Cmd, Aux, pBuffer);
  case FS_CMD_FORMAT_LOW_LEVEL:
  case FS_CMD_REQUIRES_FORMAT:
  case FS_CMD_FREE_SECTORS:
  case FS_CMD_GET_DEVINFO:
  case FS_CMD_SET_DELAY:
  default:
    break;
  }
  Status = FS_LB_GetStatus(&pVolume->Partition.Device);
  if (Status >= 0) {
    return FS_IOCTL(&pVolume->Partition.Device, Cmd, Aux, pBuffer);
  }
  return -1;
}

/*********************************************************************
*
*       FS__IoCtl
*
*  Function description:
*    Internal version of FS_IoCtl.
*    Execute device command.
*
*  Parameters:
*    pVolume     - Pointer to the specified volume.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value:
*    Command specific. In general a negative value means an error.
*/
int FS__IoCtl(FS_VOLUME * pVolume, I32 Cmd, I32 Aux, void *pBuffer) {
  int          r;
  r = -1;
  if (pVolume) {
    FS_LOCK_DRIVER(&pVolume->Partition.Device);
    r = FS__IoCtlNL(pVolume, Cmd, Aux, pBuffer);
    FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
  }
  return r;
}

/*********************************************************************
*
*       FS__FormatLow
*
*  Function description:
*    Low-level format the medium
*
*  Parameters:
*    pDevice       Device specifier (string). "" refers to the first device.
*  
*  Return value:
*    ==0         - O.K.: Low level format successful
*    !=0         - ERROR
*/
int FS__FormatLow(FS_VOLUME * pVolume) {
  int          r;

  FS__UnmountForcedNL(pVolume);
  r = FS__IoCtl(pVolume, FS_CMD_FORMAT_LOW_LEVEL, 0, 0);  /* Erase & Low-level  format the flash */
  return r;
}

/*********************************************************************
*
*       FS__WriteSector
*
*  Function description:
*    Internal version of FS_WriteSector
*    Writes a sector to a device
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS__WriteSector(FS_VOLUME * pVolume, const void *pData, U32 SectorIndex) {
  FS_DEVICE  * pDevice;
  int r;

  r = -1;
  if (pVolume) {
    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS_LB_InitMediumIfRequired(pDevice);
    r = FS_LB_WriteDevice(pDevice, SectorIndex, pData, FS_SECTOR_TYPE_DATA);
    FS_UNLOCK_DRIVER(pDevice);
  }
  return r;
}

/*********************************************************************
*
*       FS__ReadSector
*
*  Function description:
*    Internal version of FS_ReadSector
*    Reads a sector to a device
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS__ReadSector(FS_VOLUME * pVolume, void *pData, U32 SectorIndex) {
  FS_DEVICE  * pDevice;
  int r;

  r = -1;
  if (pVolume) {
    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS_LB_InitMediumIfRequired(pDevice);
    r = FS_LB_ReadDevice(pDevice, SectorIndex, pData, FS_SECTOR_TYPE_DATA);
    FS_UNLOCK_DRIVER(pDevice);
  }
  return r;
}

/*********************************************************************
*
*       FS__WriteSectors
*
*  Function description:
*    Writes multiple sectors to a volume.
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS__WriteSectors(FS_VOLUME * pVolume, const void *pData, U32 SectorIndex, U32 NumSectors) {
  FS_DEVICE  * pDevice;
  int r;

  r = -1;
  if (pVolume) {
    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS_LB_InitMediumIfRequired(pDevice);
    r = FS_LB_WriteBurst(pDevice, SectorIndex, NumSectors, pData, FS_SECTOR_TYPE_DATA);
    FS_UNLOCK_DRIVER(pDevice);
  }
  return r;
}

/*********************************************************************
*
*       FS__ReadSectors
*
*  Function description:
*    Reads multiple sectors from a volume.
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS__ReadSectors(FS_VOLUME * pVolume, void *pData, U32 SectorIndex, U32 NumSectors) {
  FS_DEVICE  * pDevice;
  int r;

  r = -1;
  if (pVolume) {
    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS_LB_InitMediumIfRequired(pDevice);
    r = FS_LB_ReadBurst(pDevice, SectorIndex, NumSectors, pData, FS_SECTOR_TYPE_DATA);
    FS_UNLOCK_DRIVER(pDevice);
  }
  return r;
}


/*********************************************************************
*
*       FS__GetVolumeStatus
*
*  Function description:
*    Internal version of FS_GetVolumeStatus
*    Returns the status of a volume.
*
*  Parameters
*    pVolume           - Pointer to a FS_VOLUME structure.
*                        Can be NULL
*
*  Return value:
*     FS_MEDIA_NOT_PRESENT     - Volume is not present.
*     FS_MEDIA_IS_PRESENT      - Volume is present.
*     FS_MEDIA_STATE_UNKNOWN   - Volume state is unknown.
*
*/
int FS__GetVolumeStatus(FS_VOLUME * pVolume) {
  int r;

  r = FS_MEDIA_STATE_UNKNOWN;
  if (pVolume) {
    FS_DEVICE * pDevice;

    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS_LB_InitMediumIfRequired(pDevice);
    r = pDevice->pType->pfGetStatus(pDevice->Data.Unit);
    FS_UNLOCK_DRIVER(pDevice);
  }
  return r;
}

/*********************************************************************
*
*       FS__GetDeviceInfo
*
*  Function description:
*    Internal version of FS_GetDeviceInfo
*    Retrieves device information of a volume.
*
*  Return value:
*      0     - O.K.
*     -1     - Device is not ready or general error.
*/
int FS__GetDeviceInfo(FS_VOLUME * pVolume, FS_DEV_INFO * pDevInfo) {
  int         r;

  r =  -1;  // Set as error so far
  if (pVolume) {
    FS_DEVICE * pDevice;

    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS_MEMSET(pDevInfo, 0, sizeof(FS_DEV_INFO));
    FS_LB_InitMediumIfRequired(pDevice);
    r = FS_LB_GetDeviceInfo(pDevice, pDevInfo);
    FS_UNLOCK_DRIVER(pDevice);
  }
  return r;
}


/*********************************************************************
*
*       FS__AddPhysDevice
*
*  Description:
*    Adds a device driver to the file system.
*
*  Parameters:
*
*  Return value:
*     >= 0    Unit no of the device.
*     <= 0    Error.
*/
int FS__AddPhysDevice(const FS_DEVICE_TYPE * pDevType) {
  int Unit;

  Unit = -1;
  if (pDevType->pfAddDevice) {
    Unit = pDevType->pfAddDevice();
    if (Unit < 0) {
      FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__AddPhysDevice: Could not add device.\n"));
    } else {
      FS_OS_ADD_DRIVER(pDevType);
    }
  }
  return Unit;
}

/*********************************************************************
*
*       FS__AddDevice
*
*  Description:
*    Internal version of FS_AddDevice.
*    Adds a device driver to the file system.
*
*  Parameters:
*
*  Return value:
*/
FS_VOLUME * FS__AddDevice(const FS_DEVICE_TYPE * pDevType) {
  FS_VOLUME * pVolume;
  int Unit;

  pVolume = _AllocVolumeHandle();
  if (pVolume) {
    Unit = FS__AddPhysDevice(pDevType);
    if (Unit >= 0) {
      FS_DEVICE * pDevice;

      FS_Global.NumVolumes++;
      pDevice = &pVolume->Partition.Device;
      pDevice->pType     = pDevType;
      pDevice->Data.Unit = (U8)Unit;
      pVolume->InUse     = 1;
      pVolume->AllowAutoMount = FS_MOUNT_RW;
    } else {
      FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__AddDevice: FS__AddPhysDevice failed.\n"));
    }
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__AddDevice: Add. driver could not be added.\n"));
  }
  return pVolume;
}

/*********************************************************************
*
*       FS__IsLLFormatted
*
*  Function description:
*    Internal version of FS_IsLLFormatted
*    Returns if a volume is low-level formatted or not.
*
*  Return value:
*      1     - Volume is low-level formatted.
*      0     - Volume is not low-level formatted.
*     -1     - Low level format not supported by volume.
*/
int FS__IsLLFormatted(FS_VOLUME * pVolume) {
  int r;

  r = FS__IoCtl(pVolume, FS_CMD_REQUIRES_FORMAT, 0, NULL);
  if (r == 0) {
    r = 1;
  } else if (r == 1) {
    r = 0;
  }
  return r;
}

/*********************************************************************
*
*       FS__FindVolume
*
*  Description:
*    Finds a volume based on the fully qualified filename.
*
*  Parameters:
*    pFullName   - Fully qualified name.
*    pFilename   - Address of a pointer, which is modified to point to
*                  the file name part of pFullName.
*
*  Return value:
*    NULL        - No matching volume found
*    else        - pointer to volume containing the file
*
*  Add. info:
*    pFullname can be as follows:
*    - "filename.ext"           e.g. "file.txt"
*    - "dev:filename.ext"       e.g. "mmc:file.txt"
*    - "dev:unit:filename.ext"  e.g. "mmc:0:file.txt"
*/
FS_VOLUME * FS__FindVolume(const char *pFullName, const char * * ppFileName) {
  const char * s;
  FS_VOLUME  * pVolume;
  unsigned     m;
  U8           Unit;

  pVolume = &FS_Global.FirstVolume;
  /* Find correct FSL (device:unit:name) */
  s = FS__strchr(pFullName, ':');
  if (s != NULL) {
    m    = (int)((U32)(s) - (U32)(pFullName));      /* Compute length of specified device name */
    Unit = 0;
    if (*s != '\0') {
      //
      // Find the correct unit
      //
      s++;
      if ((*s != '\0') && (*(s+1) == ':')) {
        Unit = *s - '0';
        s += 2;        
      } 
    }
    //
    // Scan for device name
    //
    do {
      const FS_DEVICE_TYPE * pDevice;
      FS_DEVICE_DATA       * pDevData;
      const char           * sVolName;

      pDevice  = pVolume->Partition.Device.pType;
      pDevData = &pVolume->Partition.Device.Data;
      sVolName = pDevice->pfGetName(pDevData->Unit);
      if (FS_STRLEN(sVolName) == m) {
        if (FS_STRNCMP(sVolName, pFullName, m) == 0) {
          if (Unit == pDevData->Unit) {
            break;
          }
        }
      }
      pVolume = pVolume->pNext;
    } while (pVolume);
  } else {
    //
    // use 1st FSL as default
    //
    s = pFullName;
  }
  if (ppFileName) {
    *ppFileName = s;
  }
  return pVolume;
}


/*********************************************************************
*
*       FS_FindVolume
*
*  Description:
*    Finds a volume based on the fully qualified filename.
*    Format needs to be "device:<unit>", e.g. "nand:0" or "nand:"
*
*/
FS_VOLUME * FS_FindVolume(const char * sVolume) {
  const char * s;
  const char * sDevice;
  FS_VOLUME  * pVolume;
  unsigned     DeviceNameLen;
  unsigned     Unit;

  pVolume = &FS_Global.FirstVolume;
  if (sVolume) {
    s = FS__strchr(sVolume, ':');
    DeviceNameLen = s - sVolume;
    if (s) {
      Unit = *(s + 1) - '0';
      if (Unit > 9) {
        Unit = 0;
      }
      do {
        const FS_DEVICE_TYPE * pDevice;
        FS_DEVICE_DATA       * pDevData;

        if (pVolume == (FS_VOLUME *)NULL) {
          break;                                         /* No matching device found */
        }
        pDevice  = pVolume->Partition.Device.pType;
        pDevData = &pVolume->Partition.Device.Data;
        if (pDevData->Unit == Unit) {
          sDevice = pDevice->pfGetName(pDevData->Unit);
          if (strlen(sDevice) == DeviceNameLen) {
            if (FS_MEMCMP(sDevice, sVolume, DeviceNameLen) == 0) {
              break;                                       // Found device
            }
          }
        }
        pVolume = pVolume->pNext;
      } while (1);
    }
  }
  return pVolume;
}

/*********************************************************************
*
*       FS_GetNumVolumes
*
*  Description:
*    Returns the number of available volumes.
*/
int FS__GetNumVolumes(void) {
  return FS_Global.NumVolumes;
}


/*********************************************************************
*
*       FS__UnmountLL
*
*  Description:
*    Internal version of unmounting a volume at driver layer.
*    Sends an unmount command to the driver, marks the volume 
*    as unmounted and uninitialized.
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*/
void FS__UnmountLL(FS_VOLUME * pVolume) {
  FS_DEVICE * pDevice;

  pDevice = &pVolume->Partition.Device;
  //
  // Check if we need to low-level-unmount
  //
  if ((pDevice->Data.IsInited) == 0 && (pVolume->IsMounted == 0)) {
    return;
  }
  FS_LOCK_DRIVER(pDevice);
  FS__IoCtlNL(pVolume, FS_CMD_UNMOUNT, 0, NULL);    // Send unmount command to driver
  FS_LOCK_SYS();
  pDevice->Data.IsInited = 0;
  FS_UNLOCK_SYS();
  FS_UNLOCK_DRIVER(pDevice);
}

/*********************************************************************
*
*       FS__UnmountForcedLL
*
*  Description:
*    Internal version of force-unmounting a volume at driver layer.
*    Sends a forced unmount command to the driver, marks the volume 
*    as unmounted and uninitialized.
*
*  Parameters:
*    pVolume       Volume to unmount. Must be valid, may not be NULL.
*/
void FS__UnmountForcedLL(FS_VOLUME * pVolume) {
  FS_DEVICE * pDevice;

  pDevice = &pVolume->Partition.Device;
  //
  // Check if we need to low-level-unmount
  //
  if ((pDevice->Data.IsInited) == 0 && (pVolume->IsMounted == 0)) {
    return;
  }
  FS_LOCK_DRIVER(pDevice);
  FS__IoCtlNL(pVolume, FS_CMD_UNMOUNT_FORCED, 0, NULL);    // Send forced unmount command to driver
  FS_LOCK_SYS();
  pDevice->Data.IsInited = 0;
  FS_UNLOCK_SYS();
  FS_UNLOCK_DRIVER(pDevice);
}

/*********************************************************************
*
*       FS__STORAGE_Sync
*
*  Function description:
*    Cleans all caches related to the volume.
*    Informs the driver driver about the sync operation
*    thru IOCTL command FS_CMD_SYNC_VOLUME
*
*  Notes
*    There can be 2 types of caches related to the volume:
*      - High level (above driver)
*      - Low level (inside of driver)
*/
void FS__STORAGE_Sync(FS_VOLUME * pVolume) {
  if (pVolume) {
#if FS_SUPPORT_CACHE
    FS__CACHE_CommandVolume(pVolume, FS_CMD_CACHE_CLEAN, NULL);
#endif
    FS__IoCtl              (pVolume, FS_CMD_SYNC, 0, NULL);
  } else {
    FS_DEBUG_WARN((FS_MTYPE_STORAGE, "FS_STORAGE_Sync: pVolume is invalid"));
  }
}

/*********************************************************************
*
*       Public code, API functions
*
**********************************************************************
*/
/*********************************************************************
*
*       FS_STORAGE_WriteSector
*
*  Function description:
*    Writes a sector to a device
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS_STORAGE_WriteSector(const char *sVolume, const void *pData, U32 SectorIndex) {
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  r = FS__WriteSector(pVolume, pData, SectorIndex);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_STORAGE_ReadSector
*
*  Function description:
*    Reads a sector from a device
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS_STORAGE_ReadSector(const char * sVolume, void *pData, U32 SectorIndex) {
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  r = FS__ReadSector(pVolume, pData, SectorIndex);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_STORAGE_WriteSectors
*
*  Function description:
*    Writes a sector to a device
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS_STORAGE_WriteSectors(const char *sVolume, const void * pData, U32 FirstSector, U32 NumSectors) {
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  r = FS__WriteSectors(pVolume, pData, FirstSector, NumSectors);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_STORAGE_ReadSectors
*
*  Function description:
*    Reads a sector from a device
*
*  Return value:
*       0             O.K.
*    != 0             Error
*/
int FS_STORAGE_ReadSectors(const char *sVolume, void * pData, U32 FirstSector, U32 NumSectors) {
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  r = FS__ReadSectors(pVolume, pData, FirstSector, NumSectors);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_STORAGE_Unmount
*
*  Description:
*    Unmountis a given volume at driver layer.
*    Sends an unmount command to the driver, marks the volume as unmounted
*    and uninitialized.
*
*  Parameters:
*    sVolume            - The volume name
*
*/
void FS_STORAGE_Unmount(const char * sVolume) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    FS__UnmountLL(pVolume);
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
void FS_STORAGE_UnmountForced(const char * sVolume) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    FS__UnmountForcedLL(pVolume);
  }
  FS_UNLOCK();
}


/*********************************************************************
*
*       FS_STORAGE_Sync
*
*  Function description:
*    Cleans all caches related to the volume.
*    Informs the driver driver about the sync operation
*    thru IOCTL command FS_CMD_SYNC_VOLUME
*
*  Notes
*    There can be 2 types of caches related to the volume:
*      - High level (above driver)
*      - Low level (inside of driver)
*/
void FS_STORAGE_Sync(const char * sVolume) {
  FS_VOLUME * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  FS__STORAGE_Sync(pVolume);
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_STORAGE_GetDeviceInfo
*
*  Function description:
*    Retrieves device information of a volume.
*
*  Return value:
*      0     - O.K.
*     -1     - Device is not ready or general error.
*/
int FS_STORAGE_GetDeviceInfo(const char * sVolume, FS_DEV_INFO * pDevInfo) {
  FS_VOLUME * pVolume;
  int         r;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolume, NULL);
  r = FS__GetDeviceInfo(pVolume, pDevInfo);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_IoCtl
*
*  Function description:
*    Execute device command.
*
*  Parameters:
*    pDevName    - Fully qualified directory name.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value:
*    Command specific. In general a negative value means an error.
*/
int FS_IoCtl(const char *pDevName, I32 Cmd, I32 Aux, void *pBuffer) {
  const char * s;
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(pDevName, &s);
  r = FS__IoCtl(pVolume, Cmd, Aux, pBuffer);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_FormatLow
*
*  Function description:
*    Low-level format the medium
*
*  Parameters:
*    pDevice       Device specifier (string). "" refers to the first device.
*  
*  Return value:
*    ==0         - O.K.: Low level format successful
*    !=0         - ERROR
*/
int FS_FormatLow(const char *pDevice) {
  int         r;
  FS_VOLUME * pVolume;

  FS_LOCK();
  r = -1;
  pVolume = FS__FindVolume(pDevice, NULL);
  if (pVolume) { 
    r = FS__FormatLow(pVolume);
  }
  FS_UNLOCK();
  return r;
}


/*********************************************************************
*
*       FS_FormatLLIfRequired
*
*  Function description:
*    Low-level format the medium
*
*  Parameters:
*    pDevice       Device specifier (string). "" refers to the first device.
*  
*  Return value:
*    == 0         - O.K.: Low level format successful.
*    == 1         - low-level format not required.
*    ==-1         - ERROR, low-level format not supported.
*/
int FS_FormatLLIfRequired(const char * sVolName) {
  int r;

  r = FS_IsLLFormatted(sVolName);
  if (r == 0) {
    FS_DEBUG_LOG((FS_MTYPE_API, "FS_FormatLLIfRequired: Low-level-formatting volume"));
    r = FS_FormatLow(sVolName);
  }
  return r;
}

/*********************************************************************
*
*       FS_AddDevice
*
*  Description:
*    Adds a device driver to the file system.
*
*  Parameters:
*
*  Return value:
*/
FS_VOLUME * FS_AddDevice(const FS_DEVICE_TYPE * pDevType) {
  FS_VOLUME * pVolume;
  pVolume = FS__AddDevice(pDevType);
  return pVolume;
}

/*********************************************************************
*
*       FS_AddDevice
*
*  Description:
*    Adds a device driver to the file system.
*
*  Parameters:
*
*  Return value:
*/
int FS_AddPhysDevice(const FS_DEVICE_TYPE * pDevType) {
  int r;
  r = FS__AddPhysDevice(pDevType);
  return r;
}

/*********************************************************************
*
*       FS_GetNumVolumes
*
*  Description:
*    Returns the number of available volumes.
*/
int FS_GetNumVolumes(void) {
  int r;
  FS_LOCK();
  r =  FS_Global.NumVolumes;
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_IsLLFormatted
*
*  Function description:
*    Returns if a volume is low-level formatted or not.
*
*  Return value:
*      1     - Volume is low-level formatted.
*      0     - Volume is not low-level formatted.
*     -1     - Low level format not supported by volume.
*/
int FS_IsLLFormatted(const char * sVolume) {
  FS_VOLUME * pVolume;
  int         r;

  FS_LOCK();
  r       = -1;  // Set as error so far
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    r = FS__IsLLFormatted(pVolume);
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeName
*
*  Description:
*    Returns the name of a volume
*/
int FS_GetVolumeName(int Index, char * pBuffer, int MaxSize) {
  FS_VOLUME            * pVolume;
  const FS_DEVICE_TYPE * pType;
  FS_DEVICE_DATA       * pDevData;
  const char           * pDevName;
  int                    r;
  int                    i;

  FS_LOCK();
  pVolume = &FS_Global.FirstVolume;
  for (i = 0; i < Index; i++) {
    pVolume = pVolume->pNext;
  }   
  pType    = pVolume->Partition.Device.pType;
  pDevData = &pVolume->Partition.Device.Data;
  pDevName = pType->pfGetName(pDevData->Unit);
  r        = 0;
  if (pBuffer)  {
    int LenReq;
    LenReq = FS_STRLEN(pDevName) + 5;
    if ((LenReq) > MaxSize) {
      FS_UNLOCK();
      return LenReq;
    }
    /*
     * Copy the device name
     */
    do {
      *pBuffer++ = *pDevName++;
      r++;
    } while (*pDevName);
    /*
     * Add ':'
     */
    *pBuffer++ = ':';
    /*
     * Add Unit number
     */
    *pBuffer++ = (U8) ('0' + pDevData->Unit);
    /*
     * Add ':'
     */
    *pBuffer++ = ':';
    r += 3;
    /*
     * Add '\0'
     */
    *pBuffer = 0;
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeStatus
*
*  Function description:
*    Returns the status of a volume.
*
*  Parameters
*    sVolume           - Pointer to a string containing the name of the volume.
*
*  Return value:
*     FS_MEDIA_NOT_PRESENT     - Volume is not present.
*     FS_MEDIA_IS_PRESENT      - Volume is present.
*     FS_MEDIA_STATE_UNKNOWN   - Volume state is unknown.
*
*/
int FS_GetVolumeStatus(const char  * sVolume) {
 FS_VOLUME * pVolume;
 int         r;

 FS_LOCK();
 pVolume = FS__FindVolume(sVolume, NULL);
 r = FS__GetVolumeStatus(pVolume);
 FS_UNLOCK();
 return r;

}

/*********************************************************************
*
*       FS_SetOnDeviceActivityHook
*
*  Function description:
*    Allows to setup a hook function in order to see which sectors are
*    actually written/read.
*    The hook function prototype:
*      void (FS_ONDEVICEACTIVITYHOOK)(FS_DEVICE * pDevice, 
*                                     unsigned Operation, 
*                                     U32 StartSector, 
*                                     U32 NumSectors, 
*                                     int Sectortype);
*    where:  Operation is either  FS_OPERATION_READ or FS_OPERATION_WRITE
*            Sectortype is  either FS_SECTOR_TYPE_DATA
*                               or FS_SECTOR_TYPE_MAN
*                               or FS_SECTOR_TYPE_DIR
*
*
*/
void FS_SetOnDeviceActivityHook(const char * sVolume, FS_ONDEVICEACTIVITYHOOK * pfOnDeviceActivityHook) {
 FS_VOLUME * pVolume;

 FS_LOCK();
 pVolume = FS__FindVolume(sVolume, NULL);
 if (pVolume) {
    FS_LOCK_SYS();
    pVolume->Partition.Device.Data.pfOnDeviceActivityHook = pfOnDeviceActivityHook;
    FS_UNLOCK_SYS(); 
 }
 FS_UNLOCK();
  
}

/*********************************************************************
*
*       FS_STORAGE_GetCounters
*
*/
void FS_STORAGE_GetCounters(FS_STORAGE_COUNTERS * pStat) {
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
  *pStat = FS_STORAGE_Counters;
#else
  FS_MEMSET(pStat, 0, sizeof(FS_STORAGE_COUNTERS));
#endif
}

/*********************************************************************
*
*       FS_STORAGE_ResetCounters
*
*/
void FS_STORAGE_ResetCounters(void) {
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
  FS_MEMSET(&FS_STORAGE_Counters, 0, sizeof(FS_STORAGE_COUNTERS));
#endif
}

/*************************** End of file ****************************/
