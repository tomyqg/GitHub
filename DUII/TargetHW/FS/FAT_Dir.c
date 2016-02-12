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
File        : FAT_Dir.c
Purpose     : FSL Directory functions
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"
#include "FAT_Intern.h"

/*********************************************************************
*
*       Static Data
*
**********************************************************************
*/
static const FS_83NAME NameDirDot    = {{'.', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}};
static const FS_83NAME NameDirDotDot = {{'.', '.',  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       Public Code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_CreateDirEx
*
*  Description:
*    FS internal function. Create a directory in the directory specified
*    with DirStart. Do not call, if you have not checked before for
*    existing directory with name pDirName.
*
*  Parameters:
*    DirStart    - Start of directory, where to create pDirName.
*
*  Return value:
*    >=0         - Directory has been created.
*    <0          - An error has occurred.
*/
int FS_FAT_CreateDirEx(FS_VOLUME *pVolume, const char *pDirName, U32 DirStart, FS_SB * pSB) {
  U32             DirSector;
  I32             Cluster;
  int             j;
  FS_FAT_DENTRY * pDirEntry;
  U32             TimeDate;
  U16             Date;
  U16             Time;
  FS_FAT_INFO   * pFATInfo;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  TimeDate = FS_X_GetTimeDate();
  Cluster  = FS_FAT_AllocCluster(pVolume, pSB, 0, NULL);
  if (Cluster == 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_CreateDirEx: Can not create dir: No free cluster.\n"));
    return -1;         /* Could not alloc cluster */
  }
  Time = (U16)(TimeDate & 0xffff);
  Date = (U16)(TimeDate >> 16);
  pDirEntry = FAT_pDirEntryAPI->pfCreateDirEntry(pVolume, pSB, pDirName, DirStart, Cluster, FS_FAT_ATTR_DIRECTORY, 0, Time, Date);
  if (pDirEntry) {

    /* Free entry found. Make entry */
    /*
     * Write the modified directory entry
     */
    FS__SB_MarkDirty(pSB);
    FS__SB_Clean(pSB);
    
    /*
     * Make the "." and ".." entries
     */
    FS_MEMSET(pSB->pBuffer, 0x00, pFATInfo->BytesPerSec);
    DirSector = FS_FAT_ClusterId2SectorNo(pFATInfo, Cluster); /* Find 1st absolute sector of the new directory */
    FS__SB_MarkValid(pSB, DirSector, FS_SB_TYPE_DIRECTORY);

    pDirEntry = (FS_FAT_DENTRY *)pSB->pBuffer;
    FS_FAT_WriteDirEntry83(pDirEntry++, &NameDirDot,   Cluster,  FS_FAT_ATTR_DIRECTORY, 0, Time, Date);
    FS_FAT_WriteDirEntry83(pDirEntry,   &NameDirDotDot,DirStart, FS_FAT_ATTR_DIRECTORY, 0, Time, Date);
    FS__SB_Clean(pSB);

    /* Clear rest of the directory cluster */
    FS_MEMSET(pSB->pBuffer, 0x00, pFATInfo->BytesPerSec);
    for (j = 1; j < pFATInfo->SecPerClus; j++) {
      DirSector++;
      FS__SB_SetSector(pSB, DirSector, FS_SB_TYPE_DIRECTORY);
      FS__SB_Write(pSB);

    }
    return 0;  /* It worked O.K. ! */
  } else {
    FS_FAT_FreeClusterChain(pVolume, pSB, Cluster, 1);
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_CreateDirEx: Can not create directory: No free directory entry.\n"));
  }
  return -2;                    /* Directory full and can not be increased */
}
/*********************************************************************
*
*       FS_FAT_OpenDir
*             _OpenDir (helper)
*
*  Description:
*    Open an existing directory for reading.
*
*  Parameters:
*    pDirName    - Directory name.
*    pDir        - Pointer to a FS_DIR data structure.
*
*  Return value:
*    ==0         - FS_DIR data structure contains valid information.
*    !=0         - Unable to open the directory..
*/
static int _OpenDir(const char *pDirName, FS__DIR *pDir, FS_SB * pSB) {
  U32          len;
  U32          dstart;
  const char * pFileName;
  FS_VOLUME  * pVolume;
  U32          FirstCluster;

  if (pDir == NULL) {
    return 1;  /* No valid pointer to a FS_DIR structure */
  }
  FirstCluster = 0;
  pVolume      = pDir->pVolume;
  /* Find parent directory on the media and return file name part of the complete path, as well as location and size info */
  if (FS_FAT_FindPath(pDir->pVolume, pSB, pDirName, &pFileName, &dstart) == 0) {
    return 1;  /* Directory not found */
  }
  /* Parent directory found. */

  len = FS_STRLEN(pFileName);
  if (len != 0) {
    FS_FAT_DENTRY * pDirEntry;
    /* There is a name in the complete path (it does not end with a '\') */
    pDirEntry = FS_FAT_FindDirEntry(pVolume, pSB, pFileName, 0, dstart, FS_FAT_ATTR_DIRECTORY, NULL);
    if (pDirEntry) {
      U8 Attrib;
      /*
       * Check if the directory entry has the directory attribute set
       */
      Attrib = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
      if ((Attrib & FS_FAT_ATTR_DIRECTORY) == FS_FAT_ATTR_DIRECTORY) {
        FirstCluster = FS_FAT_GetFirstCluster(pDirEntry);
      } else {
        return 1;   /* specified directory name is not a directory */
      }
    } else {
      return 1;  /* Directory not found */
    }
  } else {
    /*
       There is no name in the complete path (it does end with a '\'). In that
       case, FS_FAT_FindPath returns already start of the directory.
    */
    FirstCluster = dstart;  /* Use 'current' path */
  }
  pDir->FirstCluster = FirstCluster;
  pDir->error        = 0;
  pDir->DirEntryIndex = 0;
  return 0;
}

int FS_FAT_OpenDir(const char *pDirName, FS__DIR *pDir) {
  int r;
  FS_SB SB;
  FS__SB_Create(&SB, &pDir->pVolume->Partition);
  r = _OpenDir(pDirName, pDir, &SB);
  FS__SB_Delete(&SB);
  return r;
}

/*********************************************************************
*
*       FS_FAT_CloseDir
*
*  Description:
*    FS internal function. Close a directory referred by pDir.
*
*  Parameters:
*    pDir        - Pointer to a FS_DIR data structure.
*
*  Return value:
*    ==0         - Directory has been closed.
*    ==-1        - Unable to close directory.
*/
int FS_FAT_CloseDir(FS__DIR * pDir) {
  if (pDir == NULL) {
    return -1;  /* No valid pointer to a FS_DIR structure */
  }
  return 0;
}


/*********************************************************************
*
*       FS_FAT_ReadDir
*
*  Description:
*    Read next directory entry in directory specified by pDir.
*
*  Parameters:
*    pDir          - Pointer to a FS__DIR structure.
*    pDirEntryInfo - Pointer to a FS_DIRENTRY_INFO structure.
*
*  Return value:
*    ==-1     - Error. 
*    == 0     - Success.
*/
int FS_FAT_ReadDir(FS__DIR *pDir, FS_DIRENTRY_INFO * pDirEntryInfo) {
  FS_VOLUME * pVolume;
  int         r;
  FS_SB       SB;

  if (pDir == NULL) {
    return -1;  /* No valid pointer to a FS_DIR structure */
  }
  r        = -1;
  pVolume  = pDir->pVolume;
  FS__SB_Create(&SB, &pVolume->Partition);
  r = FAT_pDirEntryAPI->pfReadDirEntryInfo(pDir, pDirEntryInfo, &SB);
  FS__SB_Delete(&SB);
  return r;
}

/*********************************************************************
*
*       FS_FAT_RemoveDir
*
*  Description:
*    Remove a directory.
*    If you call this function to remove a directory, you must make sure, that
*    it is already empty.
*
*  Parameters:
*    pDirName    - Directory name.
*
*  Return value:
*    ==0         - Directory has been removed.
*    ==-1        - An error has occurred.
*/
int  FS_FAT_RemoveDir(FS_VOLUME * pVolume, const char *pDirName) {
  U32             Len;
  U32             DirStart;
  const char    * pFileName;
  int             r = -1;
  FS_SB           SB;
  FS_FAT_DENTRY * pDirEntry;
  I32             LongDirEntryIndex = -1;

  FS__SB_Create(&SB, &pVolume->Partition);
  if (FS_FAT_FindPath(pVolume, &SB, pDirName, &pFileName, &DirStart)) {
    Len = FS_STRLEN(pFileName);
    if (Len) {
      pDirEntry =  FS_FAT_FindDirEntry(pVolume, &SB, pFileName, 0, DirStart, FS_FAT_ATTR_DIRECTORY, &LongDirEntryIndex);
      if (pDirEntry) {
        if (FS_FAT_DeleteFileOrDir(pVolume, &SB, pDirEntry, 0) != 0) {;  /* Remove the directory */
          r = -1;
        } else {
          if (FAT_pDirEntryAPI->pfDelLongEntry) {
            FAT_pDirEntryAPI->pfDelLongEntry(pVolume, &SB, DirStart, LongDirEntryIndex);
          }
          r = 0;
        }
      }
    }
  }
  FS__SB_Delete(&SB);
  return r;
}

/*********************************************************************
*
*       FS_FAT_CreateDir
*
*  Description:
*    Creates a directory.
*
*  Parameters:
*    pDirName    - Directory name.
*
*  Return value:
*    ==0         - Directory has been created.
*    ==-1        - An error has occurred.
*/
int  FS_FAT_CreateDir(FS_VOLUME * pVolume, const char *pDirName) {
  U32       Len;
  U32          DirStart;
  I32          i;
  const char  *   pFileName;
  int             r = -1;
  FS_SB           SB;
  FS_FAT_DENTRY * pDirEntry;

  FS__SB_Create(&SB, &pVolume->Partition);
  if (FS_FAT_FindPath(pVolume, &SB, pDirName, &pFileName, &DirStart)) {
    Len = FS_STRLEN(pFileName);
    if (Len) {
      pDirEntry =  FS_FAT_FindDirEntry(pVolume, &SB, pFileName, 0, DirStart, 0, NULL);
      if (pDirEntry == NULL) {
        i = FS_FAT_CreateDirEx(pVolume, pFileName, DirStart, &SB);  /* Create the directory */
        if (i >= 0) {
          r = 0;
        }
      }
    }
  }
  FS__SB_Delete(&SB);
  return r;
}

/*************************** End of file ****************************/
