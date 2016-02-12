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
File        : FS_MAP.c
Purpose     : Wrapper to call the correct FS function.
---------------------------END-OF-HEADER------------------------------
*/

#include "FS_Int.h"

#if FS_SUPPORT_MULTIPLE_FS
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
typedef struct {
  int               FSNumber;
  const FS_FS_API * pAPI;
} FS_API_TABLE;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
static const FS_API_TABLE  _aAPI[] = {
  {FS_FAT, &FS_FAT_API},
  {FS_EFS, &FS_EFS_API}
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_MAP_Close
*
*/
void FS_MAP_Close(FS_FILE * pFile) {
  FS_VOLUME * pVolume;

  pVolume = pFile->pFileObj->pVolume;
  if (pVolume->pFS_API) {
    pVolume->pFS_API->pfClose(pFile);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
}


/*********************************************************************
*
*       FS_MAP_Close
*
*/

int FS_MAP_CheckFS_API(FS_VOLUME * pVolume) {
  unsigned i;

  for (i = 0; i < COUNTOF(_aAPI); i++) {
    if (_aAPI[i].pAPI->pfCheckBootSector(pVolume) != 0) {
      pVolume->pFS_API = _aAPI[i].pAPI;
      return 1;
    }
  }
  FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain an recognizable file system.\n"));
  return 0;
}

/*********************************************************************
*
*       FS_MAP_Read
*
*/
U32 FS_MAP_Read(FS_FILE * pFile, void  * pData, U32 NumBytes) {
  FS_VOLUME * pVolume;

  pVolume = pFile->pFileObj->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfRead(pFile, pData, NumBytes);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return 0;
}


/*********************************************************************
*
*       FS_MAP_Write
*
*/
U32 FS_MAP_Write(FS_FILE * pFile, const void  * pData, U32 NumBytes) {
  FS_VOLUME * pVolume;

  pVolume = pFile->pFileObj->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfWrite(pFile, pData, NumBytes);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return 0;
}

/*********************************************************************
*
*       FS_MAP_Open
*
*/
char FS_MAP_Open(const char * pFileName, FS_FILE * pFile, char DoDel, char DoOpen, char DoCreate) {
  FS_VOLUME * pVolume;

  pVolume = pFile->pFileObj->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfOpen(pFileName, pFile, DoDel, DoOpen, DoCreate);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return 0;
}

/*********************************************************************
*
*       FS_MAP_Format
*
*/
int FS_MAP_Format(FS_VOLUME * pVolume,   FS_FORMAT_INFO * pFormatInfo) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfFormat(pVolume, pFormatInfo);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_OpenDir
*
*/
int FS_MAP_OpenDir(const char * pDirName,  FS__DIR * pDir) {
  FS_VOLUME * pVolume;

  pVolume = pDir->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfOpenDir(pDirName, pDir);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_CloseDir
*
*/
int FS_MAP_CloseDir(FS__DIR * pDir) {
  FS_VOLUME * pVolume;

  pVolume = pDir->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfCloseDir(pDir);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_ReadDir
*
*/
int FS_MAP_ReadDir(FS__DIR * pDir, FS_DIRENTRY_INFO * pDirEntryInfo) {
  FS_VOLUME * pVolume;

  pVolume = pDir->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfReadDir(pDir, pDirEntryInfo);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_RemoveDir
*
*/
int FS_MAP_RemoveDir(FS_VOLUME * pVolume, const char * pDirName) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfRemoveDir(pVolume, pDirName);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_CreateDir
*
*/
int FS_MAP_CreateDir(FS_VOLUME * pVolume, const char * pDirName) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfCreateDir(pVolume, pDirName);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_Rename
*
*/
int FS_MAP_Rename(const char * sOldName, const char * sNewName, FS_VOLUME * pVolume) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfRename(sOldName, sNewName, pVolume);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_Move
*
*/
int FS_MAP_Move(const char * sOldName, const char * sNewName, FS_VOLUME * pVolume) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfMove(sOldName, sNewName, pVolume);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_SetDirEntryInfo
*
*/
char FS_MAP_SetDirEntryInfo(FS_VOLUME * pVolume, const char * sName, const void * p, int Mask) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfSetDirEntryInfo(pVolume, sName, p, Mask);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return (char)0xff;
}

/*********************************************************************
*
*       FS_MAP_GetDirEntryInfo
*
*/
char FS_MAP_GetDirEntryInfo(FS_VOLUME * pVolume, const char * sName, void * p, int Mask) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfGetDirEntryInfo(pVolume, sName, p, Mask);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return (char)0xff;
}

/*********************************************************************
*
*       FS_MAP_SetEndOfFile
*
*/
int FS_MAP_SetEndOfFile(FS_FILE * pFile) {
  FS_VOLUME * pVolume;

  pVolume = pFile->pFileObj->pVolume;
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfSetEndOfFile(pFile);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return 0xff;
}

/*********************************************************************
*
*       FS_MAP_Clean
*
*/
void FS_MAP_Clean(FS_VOLUME * pVolume) {
  if (pVolume->pFS_API) {
    pVolume->pFS_API->pfUnmount(pVolume);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
}

/*********************************************************************
*
*       FS_MAP_GetDiskInfo
*
*/
int FS_MAP_GetDiskInfo(FS_VOLUME * pVolume, FS_DISK_INFO * pDiskData, int Flags) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfGetDiskInfo(pVolume, pDiskData, Flags);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_GetVolumeLabel
*
*/
int FS_MAP_GetVolumeLabel(FS_VOLUME * pVolume, char * pVolumeLabel, unsigned VolumeLabelSize) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfGetVolumeLabel(pVolume, pVolumeLabel, VolumeLabelSize);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_SetVolumeLabel
*
*/
int FS_MAP_SetVolumeLabel(FS_VOLUME * pVolume, const char * pVolumeLabel) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfSetVolumeLabel(pVolume, pVolumeLabel);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_CreateJournalFile
*
*/
int FS_MAP_CreateJournalFile(FS_VOLUME * pVolume, U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfCreateJournalFile(pVolume, NumBytes, pFirstSector, pNumSectors);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_OpenJournalFile
*
*/
int FS_MAP_OpenJournalFile(FS_VOLUME * pVolume) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfOpenJournalFile(pVolume);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_GetIndexOfLastSector
*
*/
U32 FS_MAP_GetIndexOfLastSector(FS_VOLUME * pVolume) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfGetIndexOfLastSector(pVolume);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return 0;
}

/*********************************************************************
*
*       FS_MAP_CheckDisk
*
*/
int FS_MAP_CheckDisk(FS_VOLUME * pVolume, FS_DISK_INFO * pDiskInfo, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_QUERY_F_TYPE * pfOnError) {
  if (pVolume->pFS_API) {
    return pVolume->pFS_API->pfCheckDisk(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
  return -1;
}

/*********************************************************************
*
*       FS_MAP_UpdateFile
*
*/
void FS_MAP_UpdateFile(FS_FILE * pFile) {
  FS_VOLUME * pVolume;

  pVolume = pFile->pFileObj->pVolume;
  if (pVolume->pFS_API) {
    pVolume->pFS_API->pfUpdateFile(pFile);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Volume does not contain a valid FS.\n"));
  }
}


/*********************************************************************
*
*       FS_SetFSType
*
*/
int FS_SetFSType(const char * sVolume, int FSType) {
  FS_VOLUME * pVolume;
  int         r;
  FS_LOCK();
  r       = -1;
  pVolume = FS_FindVolume(sVolume);
  if (pVolume) { 
    if (pVolume->pFS_API != NULL) {
      FS_DEVICE * pDevice;

      pDevice = &pVolume->Partition.Device;
      FS_DEBUG_WARN((FS_MTYPE_FS, "A file system is already assigned to Volume %s:%d:.\n",  pDevice->pType->pfGetName(pDevice->Data.Unit), pDevice->Data.Unit));
    }
    if (FSType >= COUNTOF(_aAPI)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "Invalid wrong file sytem type.\n", FSType));
    } else {
      pVolume->pFS_API = _aAPI[FSType].pAPI;
      r = 0;
    }
  }
  FS_UNLOCK();
  return r;
}
#endif

/*************************** End of file ****************************/
