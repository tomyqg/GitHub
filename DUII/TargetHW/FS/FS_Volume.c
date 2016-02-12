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
File        : FS_Volume.c
Purpose     : API functions for handling Volumes
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _GetVolumeInfoEx
*
*  Description:
*    Internal function. Get volume information
*
*  Parameters:
*    sVolume            - The volume name
*    pInfo              - A pointer to FS_DISK_INFO. Volume information
*                         will be filled in
*
*  Return value:
*    0                  - OK
*   -1                  - Error. Failed to get volume information
*/
static int _GetVolumeInfoEx(FS_VOLUME  * pVolume, FS_DISK_INFO * pInfo, int Flags) {
  int r;

  r = -1;
  if ((FS__AutoMount(pVolume) & FS_MOUNT_R) == FS_MOUNT_R) {
    FS_LOCK_DRIVER(&pVolume->Partition.Device);
    r = FS_GET_DISKINFO(pVolume, pInfo, Flags);
    FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    if (r != -1) {
      r = 0;      /* OK - volume info retrieved successfully */
    }
  }
  return r;
}

/*********************************************************************
*
*       Public code, internal
*
**********************************************************************
*/



/*********************************************************************
*
*       FS__GetVolumeInfoEx
*
*  Description:
*    Internal function. Get volume information
*
*  Parameters:
*    sVolume            - The volume name
*    pInfo              - A pointer to FS_DISK_INFO. Volume information
*                         will be filled in
*
*  Return value:
*    0                  - OK
*   -1                  - Error. Failed to get volume information
*/
int FS__GetVolumeInfoEx(const char  * sVolume, FS_DISK_INFO * pInfo, int Flags) {
  FS_VOLUME*  pVolume;
  int r;
  r = -1;

  if (sVolume == NULL || pInfo == NULL) {
    return -1;   /* Error */
  }
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    r = _GetVolumeInfoEx(pVolume, pInfo, Flags);
  }
  return r;
}

/*********************************************************************
*
*       FS__GetVolumeInfo
*
*  Description:
*    Internal function. Get volume information
*
*  Parameters:
*    sVolume            - The volume name
*    pInfo              - A pointer to FS_DISK_INFO. Volume information
*                         will be filled in
*
*  Return value:
*    0                  - OK
*   -1                  - Error. Failed to get volume information
*/
int FS__GetVolumeInfo(const char * sVolume, FS_DISK_INFO * pInfo) {
  int r;

  r = FS__GetVolumeInfoEx(sVolume, pInfo, FS_DISKINFO_FLAG_USE_FREE_SPACE);
  return r;
}


/*********************************************************************
*
*       _Mount
*
*  Description:
*    If volume is not yet mounted, try to mount it.
*
*  Parameters:
*    MountType
*    pVolume       Volume to mount. Must be valid, may not be NULL.
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
static int _Mount(FS_VOLUME * pVolume, U8 MountType) {
  int Status;
  FS_DEVICE * pDevice;

  pDevice = &pVolume->Partition.Device;
  //
  //  Check if the media is accessible.
  //
  Status = FS_LB_GetStatus(pDevice);
  if (Status != FS_MEDIA_NOT_PRESENT) {
    FS_LB_InitMediumIfRequired(pDevice);
    //
    // Check first if there is a partition on the volume.
    //
    FS__LocatePartition(pVolume);
    //
    //  Mount the file system
    //
    if (FS_CHECK_INFOSECTOR(pVolume)) {
      pVolume->IsMounted = MountType;
    }
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Error: _Mount could not mount volume.\n"));
    return -1;
  }
  //
  // Mount the journal if necessary.
  //
  FS_JOURNAL_MOUNT(pVolume);
  return pVolume->IsMounted;
}

/*********************************************************************
*
*       FS__MountNL
*
*  Description:
*    FS internal function.
*    If volume is not yet mounted, try to mount it.
*
*  Parameters:
*    pVolume       Volume to mount. Must be valid, may not be NULL.
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
int FS__MountNL(FS_VOLUME * pVolume, U8 MountType) {
  if (pVolume->IsMounted == 0) {
    //
    //  Shall we auto mount?
    //
    if (MountType != 0) {
      if (_Mount(pVolume, MountType) <= 0) {
        return -1;
      }
    }
  }
  return pVolume->IsMounted;
}

/*********************************************************************
*
*       FS__Mount
*
*  Description:
*    FS internal function.
*    If volume is not yet mounted, try to mount it.
*
*  Parameters:
*    pVolume       Volume to mount. Must be valid, may not be NULL.
*    MountType     FS_MOUNT_R (1) or FS_MOUNT_RW (3)
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
int FS__Mount(FS_VOLUME * pVolume, U8 MountType) {
  int r;
  FS_DEVICE * pDevice;

  FS_LOCK_SYS();
  pDevice = &pVolume->Partition.Device;
  FS_UNLOCK_SYS();
  FS_USE_PARA(pDevice);
  FS_LOCK_DRIVER(pDevice);
  r = FS__MountNL(pVolume, MountType);
  FS_UNLOCK_DRIVER(pDevice);
  return r;
}


/*********************************************************************
*
*       FS__AutoMount
*
*  Description:
*    If volume is not yet mounted, try to mount it if allowed.
*
*  Parameters:
*    pVolume       Volume to mount. Must be valid, may not be NULL.
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
int FS__AutoMount(FS_VOLUME * pVolume) {
  int r;
  FS_DEVICE * pDevice;

  r = pVolume->IsMounted;
  if (r) {
    return r;
  }
  if (pVolume->AllowAutoMount == 0) {
    return 0;
  }
  //
  // Not yet mounted, automount allowed. Let's try to mount.
  //
  pDevice = &pVolume->Partition.Device;
  FS_USE_PARA(pDevice);
  FS_LOCK_DRIVER(pDevice);
  r = _Mount(pVolume, pVolume->AllowAutoMount);
  FS_UNLOCK_DRIVER(pDevice);
  return r;
}

/*********************************************************************
*
*       FS__AutoMountNL
*
*  Description:
*    If volume is not yet mounted, try to mount it if allowed.
*    This function does not lock.
*
*  Parameters:
*    pVolume       Volume to mount. Must be valid, may not be NULL.
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
int FS__AutoMountNL(FS_VOLUME * pVolume) {
  int r;

  r = pVolume->IsMounted;
  if (r) {
    return r;
  }
  if (pVolume->AllowAutoMount == 0) {
    return 0;
  }
  //
  // Not yet mounted, automount allowed. Let's try to mount.
  //
  r = _Mount(pVolume, pVolume->AllowAutoMount);
  return r;
}


/*********************************************************************
*
*       FS__Sync
*
*  Function Description
*    Internal version of FS_Sync
*/
void FS__Sync(FS_VOLUME * pVolume) {
  FS_DEVICE * pDevice;

  FS_LOCK_SYS();
  pDevice = &pVolume->Partition.Device;
  FS_UNLOCK_SYS();
  FS_LOCK_DRIVER(pDevice);
  FS_USE_PARA(pDevice);
  if (pVolume->IsMounted) {
    FS_FILE   * pFile;
    
    //
    // Clean the journal
    //
    FS_JOURNAL_CLEAN(pVolume);
    //
    // All information of each file handle is updated on the volume FS.
    //
    FS_LOCK_SYS();
    pFile    = FS_Global.pFirstFilehandle;
    while (pFile) {
      //
      // Check if file is on this volume. SYS-Lock is required when going thru the data structures.
      //
      if (pFile->InUse && pFile->pFileObj->pVolume == pVolume) {
        FS_UPDATE_FILE(pFile);
      }
      pFile = pFile->pNext;
    }
    FS_UNLOCK_SYS();
    //
    // Updates all relevant FS information
    //
    FS_CLEAN_FS(pVolume);
    //
    // As last operation, tell storage layer to sync.
    // This should flush the cache (if active) and clean any buffers the driver may have.
    //
    FS__STORAGE_Sync(pVolume);
  }
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
*       FS_IsVolumeMounted
*
*  Description:
*    Returns if a volume is mounted and correctly formatted.
*
*  Return value:
*       1         - Volume is mounted
*       0         - Volume is not mounted or does not exist
*/
int FS_IsVolumeMounted(const char * sVolumeName) {
  int r;
  FS_VOLUME * pVolume;
  FS_LOCK();
  r = 0;
  pVolume = FS__FindVolume(sVolumeName, NULL);
  if (pVolume) {
    r = (pVolume->IsMounted == 0) ? 0 : 1;
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeInfo
*
*  Description:
*    Get volume information
*
*  Parameters:
*    sVolume            - The volume name
*    pInfo              - A pointer to FS_DISK_INFO. Volume information
*                         will be filled in
*
*  Return value:
*    0                  - OK
*   -1                  - Error. Failed to get volume information
*/
int FS_GetVolumeInfo(const char * sVolume, FS_DISK_INFO * pInfo) {
  int r;

  FS_LOCK();
  r = FS__GetVolumeInfo(sVolume, pInfo);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeFreeSpace
*
*  Description:
*    Returns a volume's free space in bytes
*
*  Parameters:
*    sVolume            - The volume name
*
*  Return value:
*    Number of bytes available on the volume.
*    If the volume can not be found, 0 is returned.
*
*  Notes
*    (1) Max. value:
*        Since the return value is a 32 bit value, the maximum that can
*        be return is 0xFFFFFFFF = 2^32 - 1.
*        If there is more space available than 0xFFFFFFFF, the return value is 0xFFFFFFFF.
*/
U32 FS_GetVolumeFreeSpace(const char * sVolume) {
  FS_DISK_INFO Info;
  U32       r;

  FS_LOCK();
  r = 0;
  if (sVolume) {
    if (FS__GetVolumeInfo(sVolume, &Info) != -1) {
      r = FS__CalcSizeInBytes(Info.NumFreeClusters, Info.SectorsPerCluster, Info.BytesPerSector);
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeFreeSpaceKB
*
*  Description:
*    Returns a volume's free space in KBytes
*
*  Parameters:
*    sVolume            - The volume name
*
*  Return value:
*    Number of bytes available on the volume.
*    If the volume can not be found, 0 is returned.
*
*  Notes
*    (1) Max. value:
*        Since the return value is a 32 bit value, the maximum that can
*        be return is 0xFFFFFFFF = 2^32 - 1.
*        If there is more space available than 0xFFFFFFFF, the return value is 0xFFFFFFFF.
*/
U32 FS_GetVolumeFreeSpaceKB(const char * sVolume) {
  FS_DISK_INFO Info;
  U32       r;

  FS_LOCK();
  r = 0;
  if (sVolume) {
    if (FS__GetVolumeInfo(sVolume, &Info) != -1) {
      r = FS__CalcSizeInKB(Info.NumFreeClusters, Info.SectorsPerCluster, Info.BytesPerSector);
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeSize
*
*  Description:
*    Returns a volume's total size in bytes
*
*  Parameters:
*    sVolume            - The volume name
*
*  Return value:
*    The number of total bytes available on this volume, or
*    0 if the volume could not be found.
*
*  Notes
*    (1) Max. value:
*        Since the return value is a 32 bit value, the maximum that can
*        be return is 0xFFFFFFFF = 2^32 - 1.
*        If there is more space available than 0xFFFFFFFF, the return value is 0xFFFFFFFF.
*/
U32 FS_GetVolumeSize(const char * sVolume) {
  FS_DISK_INFO Info;
  U32       r;

  r = 0;          /* Error - Failed to get volume information */
  FS_LOCK();
  if (sVolume) {
    if (FS__GetVolumeInfoEx(sVolume, &Info, 0) != -1) {
      r = FS__CalcSizeInBytes(Info.NumTotalClusters, Info.SectorsPerCluster, Info.BytesPerSector);
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeSizeKB
*
*  Description:
*    Returns a volume's total size in bytes
*
*  Parameters:
*    sVolume            - The volume name
*
*  Return value:
*    The number of total bytes available on this volume, or
*    0 if the volume could not be found.
*
*  Notes
*    (1) Max. value:
*        Since the return value is a 32 bit value, the maximum that can
*        be return is 0xFFFFFFFF = 2^32 - 1.
*        If there is more space available than 0xFFFFFFFF, the return value is 0xFFFFFFFF.
*/
U32 FS_GetVolumeSizeKB(const char * sVolume) {
  FS_DISK_INFO Info;
  U32       r;

  r = 0;          /* Error - Failed to get volume information */
  FS_LOCK();
  if (sVolume) {
    if (FS__GetVolumeInfoEx(sVolume, &Info, 0) != -1) {
      r = FS__CalcSizeInKB(Info.NumTotalClusters, Info.SectorsPerCluster, Info.BytesPerSector);
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_GetVolumeLabel
*
*  Description:
*    Returns a volume label name if one exists.
*
*  Parameters:
*    sVolume            - The volume name
*    pVolumeLabel       - Pointer to a buffer to receive the volume label.
*    VolumeLabelSize    - length of pVolumeName
*
*  Return value:
*    0     - Success.
*   -1     - Error.
*/
int FS_GetVolumeLabel(const char * sVolume, char * pVolumeLabel, unsigned VolumeLabelSize) {
  int  r;
  FS_VOLUME * pVolume;
  r = -1;          /* Error - Failed to get volume information */
  FS_LOCK();
  if (sVolume) {
    if (pVolumeLabel) {
      pVolume = FS__FindVolume(sVolume, NULL);
      if (pVolume) {
        if ((FS__AutoMount(pVolume) & FS_MOUNT_R) == FS_MOUNT_R)  {
          FS_LOCK_DRIVER(&pVolume->Partition.Device);
          r = FS_GET_VOLUME_LABEL(pVolume, pVolumeLabel, VolumeLabelSize);
          FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
        }
      }
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_SetVolumeLabel
*
*  Description:
*    Returns a volume label name if one exists.
*
*  Parameters:
*    sVolume            - The volume name
*    pVolumeLabel       - Pointer to a buffer with the new volume label.
*                       - NULL indicates, that the volume label should
*                         be deleted.
*
*  Return value:
*    0     - Success.
*   -1     - Error.
*/
int FS_SetVolumeLabel(const char * sVolume, const char * pVolumeLabel) {
  int  r;
  FS_VOLUME * pVolume;
  r = -1;          /* Error - Failed to get volume information */
  FS_LOCK();
  if (sVolume) {
    pVolume = FS__FindVolume(sVolume, NULL);
    if (pVolume) {
      if (FS__AutoMount(pVolume) == FS_MOUNT_RW)  {
        FS_LOCK_DRIVER(&pVolume->Partition.Device);
        FS_JOURNAL_BEGIN(pVolume);
        r = FS_SET_VOLUME_LABEL(pVolume, pVolumeLabel);
        FS_JOURNAL_END(pVolume);
        FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
      }
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_Mount
*
*  Function description:
*    Mounts a volume, if necessary.
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
int FS_Mount(const char * sVolume) {
  FS_VOLUME * pVolume;
  int         r;

  FS_LOCK();
  r       = 1;  // Set as error so far
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    r = FS__Mount(pVolume, FS_MOUNT_RW);
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_MountEx
*
*  Function description:
*    Mounts a volume, if necessary.
*
*  Parameters:
*    sVolume            - The volume name
*    MountType          - FS_MOUNT_R (1) or FS_MOUNT_RW (3)
*
*  Return value:
*    == 0               - Volume is not mounted.
*    == 1 (FS_MOUNT_R)  - Volume is mounted read only.
*    == 3 (FS_MOUNT_RW) - Volume is mounted read/write.
*    == -1              - Error, Volume can not be mounted.
*/
int FS_MountEx(const char * sVolume, U8 MountType) {
  FS_VOLUME * pVolume;
  int         r;

  FS_LOCK();
  r       = -1;  // Set as error so far
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    r = FS__Mount(pVolume, MountType);
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_IsHLFormatted
*
*  Function description:
*    Returns if a volume is high-level formatted or not.
*
*  Return value:
*      1     - Volume is     high-level formatted.
*      0     - Volume is not high-level formatted.
*     -1     - Device is not ready or general error.
*/
int FS_IsHLFormatted(const char * sVolume) {
  FS_VOLUME * pVolume;
  int         r;

  FS_LOCK();
  r       = -1;  // Set as error so far
  pVolume = FS__FindVolume(sVolume, NULL);
  if (pVolume) {
    r = FS__AutoMount(pVolume);
    if (r > 0) {
      r = 1;
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_CheckDisk
*
*  Function description
*     This function checks the file system for corruption:
*     The following corruption are detected/fixed:
*       * Invalid directory entries.
*       * Lost clusters/cluster chains.
*       * Cross linked clusters.
*       * Clusters are associated to a file with size of 0.
*       * Too few clusters are allocated to a file.
*       * Cluster is not marked as end-of-chain, although it should be.
*
*  Parameters
*    sVolume           - Pointer to a string containing the name of the volume.
*    pBuffer           - Pointer to a buffer that shall be used for checking the cluster entries.
*    BufferSize        - Size of the buffer in bytes.
*    MaxRecursionLevel - The max recursion depth checkdisk shall go.
*    pfOnError         - Pointer to a callback function that shall report the user of the error. NULL is not permitted, but returns an error.
*
*  Return value
*    0    O.K.  - File system is not in a corrupted state.
*    1    Error -> an error has be found and repaired, retry is required.
*    2    User specified an abort of checkdisk operation thru callback or volume not mountable.
*/
int FS_CheckDisk(const char * sVolumeName, void *pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_QUERY_F_TYPE * pfOnError) {
  FS_VOLUME    * pVolume;
  FS_DISK_INFO   DiskInfo;
  int            r = 2;

  if (pfOnError == NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "No callback for error reporting is specified, returning.\n"));
    return 2;
  }
  FS_LOCK();
  pVolume = FS__FindVolume(sVolumeName, NULL);
  if (pVolume) {
    FS__Unmount(pVolume);
    r = _GetVolumeInfoEx(pVolume, &DiskInfo, FS_DISKINFO_FLAG_USE_FREE_SPACE);
    if (r == 0) {
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      FS_JOURNAL_INVALIDATE(pVolume);
      r = FS_CHECKDISK(pVolume, &DiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError);
      if (r == 0) {
        FS_JOURNAL_MOUNT(pVolume);
      }
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    } else {
      FS_DEBUG_ERROROUT((FS_MTYPE_API, "Medium does not contain a valid EFS allocation table structure.\n"));
      r = 2;
    }
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_SetAutoMount
*
*  Function description:
*    Sets the mount behavior of the specified volume.
*
*  Parameters
*    sVolume           - Pointer to a string containing the name of the volume.
*    MountType         - 3 values are allowed:
*                          FS_MOUNT_R    - Allows to auto mount the volume read only.
*                          FS_MOUNT_RW   - Allows to auto mount the volume read/write.
*                          0             - Disables auto mount for the volume.
*
*/
void FS_SetAutoMount(const char  * sVolume, U8 MountType) {
 FS_VOLUME * pVolume;

 FS_LOCK();
 pVolume = FS__FindVolume(sVolume, NULL);
 if (pVolume) {
   FS_LOCK_SYS();
   pVolume->AllowAutoMount = MountType;
   FS_UNLOCK_SYS();
 }
 FS_UNLOCK();

}

/*********************************************************************
*
*       FS_Sync
*
*  Function Description
*    Synchronize the volume, which includes
*      Flushes the cache
*      All information of each file handle is updated on the volume FS.
*      Clean the journal
*      Updates all relevant FS information
*      
*/
int FS_Sync(const char * sVolume) {
  FS_VOLUME * pVolume;
  int r = -1;

  FS_LOCK();
  if (sVolume) {
    pVolume = FS__FindVolume(sVolume, NULL);
    if (pVolume) {
      FS__Sync(pVolume);
    }
  }
  FS_UNLOCK();
  return r;
}


/*************************** End of file ****************************/

