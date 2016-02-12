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
File        : FS_DirOld.c
Purpose     : Obsolete directory functions
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
*       Public data
*
**********************************************************************
*/
FS_DIR FS__aDirHandle[FS_NUM_DIR_HANDLES];

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       _AllocDirHandle
*
*/
static FS_DIR * _AllocDirHandle(void) {
  FS_DIR * pHandle;
  unsigned i;

  pHandle = NULL;
  FS_LOCK_SYS();
  for (i = 0; i < COUNTOF(FS__aDirHandle); i++) {
    if (FS__aDirHandle[i].InUse == 0) {
      pHandle = &FS__aDirHandle[i];
      pHandle->InUse = 1;
      break;
    }
  }
  FS_UNLOCK_SYS();
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL
  if ((void*)pHandle == NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "No directory handle available.\n"));
  }
#endif
  return pHandle;
}

/*********************************************************************
*
*       _FreeDirHandle
*
*/
static void _FreeDirHandle(FS_DIR * pHandle) {
  if (pHandle) {
    FS_LOCK_SYS();
    pHandle->InUse = 0;
    FS_UNLOCK_SYS();
  }
}

/*********************************************************************
*
*       Public code, internal version of API functions
*
**********************************************************************
*/
/*********************************************************************
*
*       FS__OpenDir
*
*  Description:
*    Internal version of FS_OpenDir.
*    Open an existing directory for reading.
*
*  Parameters:
*    pDirName    - Fully qualified directory name.
*
*  Return value:
*    ==0         - Unable to open the directory.
*    !=0         - Address of an FS_DIR data structure.
*/
FS_DIR *FS__OpenDir(const char * pDirName) {
  FS_DIR       * pDirHandle;
  FS_VOLUME    * pVolume;
  const char   *s;

  pDirHandle = (FS_DIR*) NULL;
  /* Find correct FSL (device:unit:name) */
  pVolume = FS__FindVolume(pDirName, &s);
  if (pVolume) {
    if (FS__AutoMount(pVolume) & FS_MOUNT_R)  {
      /*  Find next free entry in FS__aDirHandle */
      pDirHandle = _AllocDirHandle();
      if (pDirHandle) {
        int Error;
        pDirHandle->Dir.pVolume = pVolume;
        FS_LOCK_DRIVER(&pVolume->Partition.Device);
        Error = FS_OPENDIR(s, &pDirHandle->Dir);
        FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
        if (Error) {
          _FreeDirHandle(pDirHandle);
          pDirHandle = (FS_DIR *)NULL;
        }
      }
    }
  }
  return pDirHandle;
}

/*********************************************************************
*
*       FS__ReadDir
*
*  Description:
*    Internal version of FS_ReadDir.
*    Read next directory entry in directory specified by
*    pDir.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*  Return value:
*    ==0         - No more directory entries or error.
*    !=0         - Pointer to a directory entry.
*/
FS_DIRENT * FS__ReadDir(FS_DIR * pDir) {
  FS_DIRENT        * pDirEnt;
  FS_DIRENTRY_INFO   DirEntryInfo;

  FS_MEMSET(&DirEntryInfo, 0, sizeof(FS_DIRENTRY_INFO));
  DirEntryInfo.sFileName      = &pDir->DirEntry.DirName[0];
  DirEntryInfo.SizeofFileName = sizeof(pDir->DirEntry.DirName);
  pDirEnt = (FS_DIRENT *)NULL;
  if (pDir) {
    FS_VOLUME * pVolume;

    pVolume = pDir->Dir.pVolume;
    if (pVolume) {
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      if (FS_READDIR(&pDir->Dir, &DirEntryInfo) == 0) {
        pDirEnt = &pDir->DirEntry;
        pDirEnt->Attributes = DirEntryInfo.Attributes;
        pDirEnt->Size       = DirEntryInfo.FileSize;
        pDirEnt->TimeStamp  = DirEntryInfo.CreationTime;
      }
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    }
  }
  return pDirEnt;
}

/*********************************************************************
*
*       FS__CloseDir
*
*  Description:
*    Internal version of FS_CloseDir.
*    Close a directory referred by pDir.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*  Return value:
*    ==0         - Directory has been closed.
*    ==-1        - Unable to close directory.
*/
int FS__CloseDir(FS_DIR * pDir) {
  int i;

  i = -1;
  if (pDir) {
    FS_VOLUME * pVolume;

    pVolume = pDir->Dir.pVolume;
    if (pVolume) {
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      i = FS_CLOSEDIR(&pDir->Dir);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    }
  }
  _FreeDirHandle(pDir);
  return i;
}

/*********************************************************************
*
*       FS__RewindDir
*
*  Description:
*    Internal version of FS_RewindDir.
*    Set pointer for reading the next directory entry to
*    the first entry in the directory.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*    Return value:
*    None.
*/
void FS__RewindDir(FS_DIR * pDir) {
  if (pDir) {
    FS_LOCK_SYS();
    pDir->Dir.DirEntryIndex = 0; /* Only rewind, if we have a valid pointer */
    FS_UNLOCK_SYS();
  }
}


/*********************************************************************
*
*       FS__DirEnt2Attr
*
*  Description:
*    Internal version of FS_DirEnt2Attr.
*    Gets the directory entry attributes.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIRENT data structure.
*    pString     - Pointer to a buffer to be copied to.
*
*/
void FS__DirEnt2Attr(FS_DIRENT * pDirEnt, U8 * pAttr) {
  if (pDirEnt != NULL) {
    if (pAttr) {
      *pAttr = pDirEnt->Attributes;
    }
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
*       FS_OpenDir
*
*  Description:
*    API function. Open an existing directory for reading.
*
*  Parameters:
*    pDirName    - Fully qualified directory name.
*
*  Return value:
*    ==0         - Unable to open the directory.
*    !=0         - Address of an FS_DIR data structure.
*/
FS_DIR * FS_OpenDir(const char * pDirName) {
  FS_DIR * pHandle;
  FS_LOCK();
  pHandle = FS__OpenDir(pDirName);
  FS_UNLOCK();
  return pHandle;
}


/*********************************************************************
*
*       FS_CloseDir
*
*  Description:
*    API function. Close a directory referred by pDir.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*  Return value:
*    ==0         - Directory has been closed.
*    ==-1        - Unable to close directory.
*/
int FS_CloseDir(FS_DIR * pDir) {
  int i;

  FS_LOCK();
  i = FS__CloseDir(pDir);
  FS_UNLOCK();
  return i;
}

/*********************************************************************
*
*       FS_ReadDir
*
*  Description:
*    API function. Read next directory entry in directory specified by
*    pDir.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*  Return value:
*    ==0         - No more directory entries or error.
*    !=0         - Pointer to a directory entry.
*/
FS_DIRENT * FS_ReadDir(FS_DIR * pDir) {
  FS_DIRENT * pDirEnt;
  FS_LOCK();
  pDirEnt = FS__ReadDir(pDir);
  FS_UNLOCK();
  return pDirEnt;
}

/*********************************************************************
*
*       FS_RewindDir
*
*  Description:
*    API function. Set pointer for reading the next directory entry to
*    the first entry in the directory.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*    Return value:
*    None.
*/
void FS_RewindDir(FS_DIR * pDir) {
  FS_LOCK();
  FS__RewindDir(pDir);
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_DirEnt2Attr
*
*  Description:
*    API function. Get the directory entry attributes.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIRENT data structure.
*    pString     - Pointer to a buffer to be copied to.
*
*/
void FS_DirEnt2Attr(FS_DIRENT * pDirEnt, U8 * pAttr) {
  FS_LOCK();
  FS__DirEnt2Attr(pDirEnt, pAttr);
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_DirEnt2Name
*
*  Description:
*    API function. Get the Get the directory entry name.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIRENT data structure.
*    pBuffer     - Pointer to a buffer to be copied to.
*
*/
void FS_DirEnt2Name(FS_DIRENT * pDirEnt, char * pBuffer) {
  FS_LOCK();
  if (pDirEnt) {
    if (pBuffer) {
      FS_STRCPY(pBuffer, pDirEnt->DirName);
    }
  }
  FS_UNLOCK();
}

/*********************************************************************
*
*       FS_DirEnt2Size
*
*  Description:
*    API function. Get the directory entry file size.
*
*  Parameters:
*    pDirEnt     - Pointer to a FS_DIRENT data structure.
*
*/
U32 FS_DirEnt2Size(FS_DIRENT * pDirEnt) {
  U32 r;

  FS_LOCK();
  r = 0;
  if (pDirEnt) {
    r = pDirEnt->Size;
  }
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_DirEnt2Time
*
*  Description:
*    API function. Get the directory entry timestamp.
*
*  Parameters:
*    pDirEnt      - Pointer to a FS_DIRENT data structure.
*
*/
U32 FS_DirEnt2Time(FS_DIRENT * pDirEnt) {
  U32 r;
  r = 0;
  FS_LOCK();
  if (pDirEnt) {
    r = pDirEnt->TimeStamp;
  }
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
