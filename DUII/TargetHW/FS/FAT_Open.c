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
File        : FAT_Open.c
Purpose     : FAT routines for open/delete files
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FAT_Intern.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CopyDirText
*
*/
static void _CopyDirText(char * pDest, const char  * pSrc, int NumBytes, int EndPos, char Extension) {
  int i;
  FS_MEMSET(pDest, ' ', NumBytes);
  for (i = 0; i < EndPos; i++) {
    U8 c;
    c = *pSrc++;
    if ((Extension == 0) && (c == 0xE5) && (i == 0)) {
      c = 0x05;
    } else {
      c = (U8)FS_TOUPPER(c);
    }
    *pDest++ = c;
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
*       Public code 1
*
**********************************************************************

  Functions in this section are global, but are used inside the FAT
  File System Layer only.

*/

/*********************************************************************
*
*       FS_FAT_WriteDirEntry83
*
*/
void FS_FAT_WriteDirEntry83(FS_FAT_DENTRY * pDirEntry, const FS_83NAME * pFileName, U32 ClusterId, U8 Attributes, U32 Size, U16 Time, U16 Date) {
  FS_MEMSET(pDirEntry, 0, sizeof(FS_FAT_DENTRY));
  FS_MEMCPY((char*)pDirEntry->data, pFileName, 11);
  pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES]                       = Attributes;
  FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_CREATION_TIME],        Time);
  FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_CREATION_DATE],        Date);
  FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_TIME],           Time);
  FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_DATE],           Date);
  FS_FAT_WriteDirEntryCluster(pDirEntry, ClusterId);
  FS_StoreU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE],                 Size);
}


/*********************************************************************
*
*       FS_FAT_WriteDirEntryShort
*
*/
void FS_FAT_WriteDirEntryShort(FS_FAT_DENTRY * pDirEntry, const char * pFileName, U32 ClusterId, U8 Attributes, U32 Size, U16 Time, U16 Date) {
  FS_83NAME Name83;
  FS_FAT_Make83Name(&Name83, pFileName, 0);
  FS_FAT_WriteDirEntry83(pDirEntry, &Name83, ClusterId, Attributes, Size, Time, Date);
}

/*********************************************************************
*
*       FS_FAT_WriteDirEntryCluster
*
*/
void FS_FAT_WriteDirEntryCluster(FS_FAT_DENTRY * pDirEntry, U32 Cluster) {
  FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_FIRSTCLUSTER_LOW],  (U16)Cluster);
  FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_FIRSTCLUSTER_HIGH], (U16)(Cluster >> 16));
}

/*********************************************************************
*
*       FS_FAT_GetFirstCluster
*
*/
U32 FS_FAT_GetFirstCluster(FS_FAT_DENTRY * pDirEntry) {
  U32 r;
  r = FS_LoadU16LE(&pDirEntry->data[DIR_ENTRY_OFF_FIRSTCLUSTER_LOW]) | (((U32)FS_LoadU16LE(&pDirEntry->data[DIR_ENTRY_OFF_FIRSTCLUSTER_HIGH])) << 16);
  return r;
}

/*********************************************************************
*
*       FS_FAT_InitDirEntryScan
*
*  Description:
*
*  Parameters:
*
*  Return value:
*/
void FS_FAT_InitDirEntryScan(FS_FAT_INFO * pFATInfo, FS_DIR_POS * pDirPos, U32 DirCluster) {
  pDirPos->DirEntryIndex = 0;
  if (pFATInfo->FATType == FS_FAT_TYPE_FAT32) {
    if (DirCluster == 0) {
      DirCluster = pFATInfo->RootDirPos;
    }
  }
  pDirPos->FirstCluster  = DirCluster;
  pDirPos->Cluster       = DirCluster;
}

/*********************************************************************
*
*       FS_FAT_GetDirEntry
*
*  Description:
*
*  Parameters:
*
*  Return value:
*    >  0             Directory sector
*    == 0             Error, no more sectors
*/
FS_FAT_DENTRY * FS_FAT_GetDirEntry(FS_VOLUME * pVolume, FS_SB* pSB, FS_DIR_POS * pDirPos) {
  U32 Cluster;
  U32 DirSector;
  U32 DirEntryIndex;
  U32 DirSectorIndex;
  U32 CurClusterIndex;
  FS_FAT_INFO        *  pFATInfo;
  unsigned ShiftPerEntry;

  pFATInfo      = &pVolume->FSInfo.FATInfo;
  Cluster       = pDirPos->Cluster;
  DirEntryIndex = pDirPos->DirEntryIndex;
  CurClusterIndex = (DirEntryIndex << DIR_ENTRY_SHIFT) / pFATInfo->BytesPerCluster;
  if (CurClusterIndex < pDirPos->ClusterIndex) {
    pDirPos->ClusterIndex = 0;
  }
  /*
   * Walk to the right cluster starting from the last known cluster position.
   */
  ShiftPerEntry = pFATInfo->ldBytesPerSector - DIR_ENTRY_SHIFT;    /* 4 for 512 byte sectors. */
  if (pDirPos->ClusterIndex == 0) {
    Cluster = pDirPos->FirstCluster;
  }
  DirSectorIndex = DirEntryIndex >> ShiftPerEntry;
  if (Cluster) {
    int SectorMask;
    U32 NumClustersToWalk;
    SectorMask =  pFATInfo->SecPerClus - 1;
    NumClustersToWalk = CurClusterIndex - pDirPos->ClusterIndex;
    /* Go to next cluster */
    if (NumClustersToWalk) {
      Cluster = FS_FAT_WalkCluster(pVolume, pSB, Cluster, NumClustersToWalk);
    }
    if (Cluster == 0) {
      return 0;               /* No more clusters */
    }
    DirSector = FS_FAT_ClusterId2SectorNo(pFATInfo, Cluster) + (DirSectorIndex & SectorMask);
  } else {
    U32 MaxDirSectorIndex;
    MaxDirSectorIndex = pFATInfo->RootEntCnt >> ShiftPerEntry;
    if (DirSectorIndex  < MaxDirSectorIndex) {
      DirSector = pFATInfo->RootDirPos + DirSectorIndex;
    } else {
      /*
       *  We reach the limit
       */
       return NULL;
    }
  }
  pDirPos->Cluster      = Cluster;
  pDirPos->ClusterIndex = CurClusterIndex;
  FS__SB_SetSector(pSB, DirSector, FS_SB_TYPE_DIRECTORY);
  if (FS__SB_Read(pSB)) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_FindNextDirEntry: Failed to read sector %d.\n", pSB->SectorNo));
    return NULL;
  }
  return ((FS_FAT_DENTRY *)pSB->pBuffer) + (DirEntryIndex & ((1 << ShiftPerEntry)- 1));
}

/*********************************************************************
*
*       FS_FAT_IncDirPos
*
*  Description:
*    Increments the position in the directory
*/
void FS_FAT_IncDirPos(FS_DIR_POS * pDirPos) {
  pDirPos->DirEntryIndex++;
}


/*********************************************************************
*
*       FS_FAT_FindEmptyDirEntry
*
*  Description:
*    Tries to find an empty directory entry in the specified directory.
*    If there is no free entry, try to increase directory size.
*
*  Parameters:
*    pVolume     - Volume information
*    DirStart    - Start of directory, where to create pDirName.
*    DirSize     - Sector size of the directory starting at DirStart.
*
*  Return value:
*    != NULL     - Free entry found
*    NULL        - An error has occurred.
*/
FS_FAT_DENTRY * FS_FAT_FindEmptyDirEntry(FS_VOLUME * pVolume,  FS_SB* pSB, U32 DirStart) {
  FS_FAT_DENTRY       * pDirEntry;
  int                   i;
  U32                DirSector;
  FS_FAT_INFO         * pFATInfo;
  FS_DIR_POS            DirPos;

  pFATInfo    = &pVolume->FSInfo.FATInfo;

  /* Read directory, trying to find an empty slot */
  FS_FAT_InitDirEntryScan(pFATInfo, &DirPos, DirStart);
  do {
    U8 c;
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
    FS_FAT_IncDirPos(&DirPos);
    if (!pDirEntry) {
      if ((DirStart == 0) && (pFATInfo->RootEntCnt)) {
        /* Root directory of FAT12/16 medium can not be increased */
        FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_FindEmptyDirEntry: Root directory too small.\n"));
        return NULL;                  /* Can not create, directory is full */
      } else {
        U32 NewCluster;
        U32 LastCluster;
        LastCluster = FS_FAT_FindLastCluster(pVolume, pSB, DirPos.Cluster, (U32*)NULL);
        NewCluster  = FS_FAT_AllocCluster   (pVolume, pSB, LastCluster, NULL);
        FS__SB_Flush(pSB);
        if (NewCluster) {
          /*
           * Clean new directory cluster  (Fill with 0)
           */
          FS_MEMSET(pSB->pBuffer, 0x00, pFATInfo->BytesPerSec);
          DirSector = FS_FAT_ClusterId2SectorNo(pFATInfo, NewCluster);
          for (i = pFATInfo->SecPerClus - 1; i >= 0; i--) {
            FS__SB_SetSector(pSB, DirSector + i, FS_SB_TYPE_DIRECTORY);
            FS__SB_Write(pSB);
          }
          pDirEntry = (FS_FAT_DENTRY *)pSB->pBuffer;
        } else {
          FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_FindEmptyDirEntry: Disk is full.\n"));
          return NULL;
        }
      }
      break;
    }
    c = pDirEntry->data[0];
    if ((c == 0x00) || (c == 0xE5)) {        /* A free entry has either 0 or 0xe5 as first byte */
      /* Free entry found. */
      break;
    }
  } while (1);
  return pDirEntry;
}

/*********************************************************************
*
*       FS_FAT_DeleteFileOrDir
*
*  Description:
*    Deletes a directory entry and frees all clusters allocated to it
*
*  Parameters:
*    pVolume     - Volume information
*    pSB         - Smart buffer
*    pDirEntry   - Pointer to directory entry to be deleted (in smart buffer)
*    IsFile      - 1 => remove a file
*                  0 => remove a directory
*
*/
char FS_FAT_DeleteFileOrDir(FS_VOLUME * pVolume, FS_SB * pSB, FS_FAT_DENTRY  * pDirEntry, U8 IsFile) {
  I32           FirstCluster;
  I32           NumClusters;
  FS_FAT_INFO    * pFATInfo;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  if (pDirEntry) {
    /* Entry has been found, delete directory entry */
    pDirEntry->data[0]  = 0xe5;
    /* Free blocks in FAT */
    //
    //  For normal files, there are no more clusters freed than the entry's file size
    //  does indicate. That avoids corruption of the complete media in case there is
    //  no EOF mark found for the file (FAT is corrupt!!!).
    //  If the function should remove a directory, file size if always 0 and cannot
    //  be used for that purpose. To avoid running into endless loop,
    //  NumClusters is set to a reasonable limit.
    //
    FS__SB_MarkDirty(pSB);
    if (IsFile) {
      U32 FileSize;
      FileSize  = FS_LoadU32LE(&pDirEntry->data[28]);
      NumClusters      = (FileSize + pFATInfo->BytesPerCluster - 1) / pFATInfo->BytesPerCluster;
    } else {
      NumClusters = 1000;     /* Max. number of clusters for directory */
    }
    FirstCluster = FS_FAT_GetFirstCluster(pDirEntry);
    FS_FAT_FreeClusterChain(pVolume, pSB, FirstCluster, NumClusters);
  }
  return pSB->HasError;
}

/*********************************************************************
*
*       FS_FAT_Make83Name
*
*  Description:
*    FS internal function. Convert a given name to the format, which is
*    used in the FAT directory.
*
*  Parameters:
*    pOrgName    - Pointer to name to be translated
*    pEntryName  - Pointer to a buffer for storing the real name used
*                  in a directory.
*    Len         - Len of the name. 0 means no defined length.
*
*  Return value:
*    0             Success: Name could be converted
*    1             Error:   Name did not comply with 8.3 criteria
*
*  Notes
*    (1) Allowed file names
*        The filename must conform to 8.3 standards.
*        The extension is optional, the name may be 8 characters at most.
*/
char FS_FAT_Make83Name(FS_83NAME * pOutName, const char *pOrgName, int Len) {
  int          i;
  int          ExtPos;

  ExtPos = -1;
  if (Len == 0) {
    Len = -1;        /* Change to a safe value, which can never occur in comparison */
  }
  //
  //  In special case of ".." we manually generate the 8.3 entry
  //
  if (FS_STRCMP(pOrgName, "..") == 0) {    
    FS_MEMCPY(&pOutName->ac[0], pOrgName, 2);
    FS_MEMSET(&pOutName->ac[2], ' ', sizeof(FS_83NAME) - 2);
    return 0;    
  }
  for (i = 0; ; i++) {
    char c;
    if (i == 13) {
      return 1;                     /* Error, file name too long*/
    }
    c = *(pOrgName + i);
    if ((c == 0) || (i == Len)) {    /* End of name ? */
      if (ExtPos == -1) {
        ExtPos = i;
      }
      break;
    }
    if (FS_FAT_IsValidShortNameChar(c) == 0) {
      return 1;                      // Invalid character used in string
    }
    if (c == '.') {
      ExtPos = i;
    }
  }
  /* Perform some checks */
  if (ExtPos == 0) {
    return 1;                     /* Error, no file name*/
  }
  if (ExtPos > 8) {
    return 1;                     /* Error, file name too long */
  }
  if ((i - ExtPos) > 4) {
    return 1;                     /* Error, extension too long */
  }
  /* All checks passed, copy filename and extension */
  _CopyDirText(&pOutName->ac[0], pOrgName,              8, ExtPos,         0);
  _CopyDirText(&pOutName->ac[8], pOrgName + ExtPos + 1, 3, i - ExtPos - 1, 1);
  return 0;                     /* O.K., file name successfully converted */
}


/*********************************************************************
*
*       FS_FAT_FindDirEntryShortEx
*
*  Description:
*    Tries to locate the short directory entry in the specified directory
*
*  Return value:
*    != NULL     - pointer to directory entry (in the smart buffer)
*    NULL        - Entry not found
*/
FS_FAT_DENTRY * FS_FAT_FindDirEntryShortEx(FS_VOLUME * pVolume, FS_SB * pSB, const char *pEntryName, int Len, FS_DIR_POS * pDirPos, U8 AttributeReq) {
  FS_FAT_DENTRY       * pDirEntry;
  FS_83NAME             FATEntryName;

  if (FS_FAT_Make83Name(&FATEntryName, pEntryName, Len)) {
    return NULL;  /* Entry name could not be converted */
  }
  /* Read directory */
  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, pDirPos);
    if (!pDirEntry) {
      break;
    }
    if (pDirEntry->data[0] == 0) {
      pDirEntry = (FS_FAT_DENTRY*)NULL;
      break;  /* No more entries. Not found. */
    }
    if (FS_MEMCMP(pDirEntry->data, &FATEntryName, 11) == 0) { /* Name does match */
      U8 Attrib;
      //
      // Do the attribute match ?
      //
      Attrib = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
      if (((Attrib & AttributeReq) == AttributeReq) && (Attrib != FS_FAT_ATTR_VOLUME_ID)) {
        break;
      }
    }
    FS_FAT_IncDirPos(pDirPos);
  } while (1);
  return pDirEntry;
}

/*********************************************************************
*
*       FS_FAT_FindDirEntryShort
*
*  Description:
*    Tries to locate the short directory entry in the specified directory
*
*  Parameters:
*    pVolume     - Volume information
*    pEntryName  - Directory entry name
*    MaxLen      - Maximum number of characters in EntryName (everything after that is ignored). E.g.: "Dir\File", 3: -> "Dir"
*
*  Return value:
*    != NULL     - pointer to directory entry (in the smart buffer)
*    NULL        - Entry not found
*/
FS_FAT_DENTRY * FS_FAT_FindDirEntryShort(FS_VOLUME * pVolume, FS_SB * pSB, const char *pEntryName, int Len, U32 DirStart, U8 AttributeReq) {
  FS_DIR_POS            DirPos;
  FS_FAT_INFO         * pFATInfo;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  FS_FAT_InitDirEntryScan(pFATInfo, &DirPos, DirStart);
  return FS_FAT_FindDirEntryShortEx(pVolume, pSB, pEntryName, Len, &DirPos, AttributeReq);
}

/*********************************************************************
*
*       FS_FAT_FindPath
*
*  Description:
*    FS internal function. Return start cluster and size of the directory
*    of the file name in pFileName.
*
*  Parameters:
*    pVolume     - Volume information
*    pFullName   - Fully qualified file name w/o device name.
*    ppFileName  - Pointer to a pointer, which is modified to point to the
*                  file name part of pFullName.
*    pDirStart   - Pointer to an U32 for returning the start cluster of the directory.
*
*  Return value:
*    >0          - Path exists
*    ==0         - An error has occurred.
*
*  Add. information
*    The function opens the path of the highest level directory.
*    subdir               -> Opens \
*    subdir\              -> Opens \subdir\
*    subdir\subdir1       -> Opens \subdir\
*    subdir\subdir1\      -> Opens \subdir\subdir1\
*/
char FS_FAT_FindPath(FS_VOLUME * pVolume, FS_SB * pSB,  const char *pFullName, const char * *ppFileName, U32 *pDirStart) {
  const char *        pDirNameStart;
  const char *        pDirNameStop;
  I32        i;
  U32        DirStart;
  FS_FAT_DENTRY * pDirEntry;

  *ppFileName = pFullName;
  // Setup pDirStart/dsize for root directory
  DirStart = 0;
  // descend into sub directory for every \ found
  pDirNameStart = pFullName;
  pDirNameStop  = FS__strchr(*ppFileName, FS_DIRECTORY_DELIMITER);
  do {
    if (pDirNameStart == pDirNameStop) {
      pDirNameStart++;
      *ppFileName  = pDirNameStart;
      pDirNameStop  = FS__strchr(*ppFileName, FS_DIRECTORY_DELIMITER);
    }
    if (pDirNameStop) {
      i = pDirNameStop-pDirNameStart;
      if (i > 0) {
        pDirEntry = FS_FAT_FindDirEntry(pVolume, pSB, pDirNameStart, i, DirStart, FS_FAT_ATTR_DIRECTORY, NULL);
        if (pDirEntry == NULL) {
          return 0;
        } else {
          DirStart = FS_FAT_GetFirstCluster(pDirEntry);
        }
      }
    }
    pDirNameStart = pDirNameStop;
    pDirNameStop  = FS__strchr(*ppFileName, FS_DIRECTORY_DELIMITER);
  } while (pDirNameStop);
  *pDirStart = DirStart;
  return 1;
}


/*********************************************************************
*
*       Public code 2
*
**********************************************************************

  These are real global functions, which are used by the API Layer
  of the file system.

*/

/*********************************************************************
*
*       FS_FAT_FOpen
*
*  Description:
*    FS internal function. Open an existing file or create a new one.
*
*  Parameters:
*    pFileName   - File name.
*    pMode       - Mode for opening the file.
*    pFile       - Pointer to an FS_FILE data structure.
*
*  Return value:
*    0         O.K.
*    1         Error
*/
char FS_FAT_Open(const char * pFileName, FS_FILE * pFile, char DoDel, char DoOpen, char DoCreate) {
  const char    * pFName;
  U32             DirStart;
  U8              AccessFlags;
  I32             DirEntryIndex;
  U32             DirEntrySector;
  U32             FirstCluster;
  U32             FileSize;
  char            r;
  FS_FILE_OBJ   * pFileObj;
  FS_VOLUME     * pVolume;
  FS_SB           SB;
  FS_FAT_DENTRY * pDirEntry;
  I32             LongDirEntryIndex = -1;

  //
  // Search directory
  //
  pFileObj       = pFile->pFileObj;
  pVolume        = pFileObj->pVolume;
  DirEntryIndex  = 0;
  DirEntrySector = 0;
  FirstCluster   = 0;
  FileSize       = 0;
  FS__SB_Create(&SB, &pVolume->Partition);
  pDirEntry = NULL;

  if (FS_FAT_FindPath(pVolume, &SB, pFileName, &pFName, &DirStart) == 0) {
    FS__SB_Delete(&SB);
    return 1;  // Directory not found
  }
  r = 0;       // No error so far
  AccessFlags = pFile->AccessFlags;
  pDirEntry = FS_FAT_FindDirEntry(pVolume, &SB, pFName, 0, DirStart, 0, &LongDirEntryIndex);
  if (pDirEntry) {
    U8 Attrib;
    //
    // Check if the directory entry is not a directory
    //
    Attrib = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
    if ((Attrib & FS_FAT_ATTR_DIRECTORY) != FS_FAT_ATTR_DIRECTORY) {
      DirEntryIndex = pDirEntry - (FS_FAT_DENTRY *)SB.pBuffer;
      DirEntrySector = SB.SectorNo;
    } else {
      FS__SB_Delete(&SB);
      return 1;  // Specified file name is a directory
    }
  }
  //
  // Delete the file if requested
  //
  if (DoDel) {              // Do we need to delete the file ?
    if (pDirEntry) {        // Does file exist ?
      FS_FILE * pFile2Check;
      pFile2Check = FS_Global.pFirstFilehandle;
      while (pFile2Check) {
        FS_INT_DATA_FAT * pData;
        
        if (pFile2Check->pFileObj && pFile2Check->InUse) {
          pData = &pFile2Check->pFileObj->Data.Fat;
          if ((pData->DirEntryIndex == DirEntryIndex) && (pData->DirEntrySector == DirEntrySector) && (pFile2Check->pFileObj->pVolume == pVolume)){
            pFile->Error = FS_ERR_FILE_ALREADY_OPENED;
            goto OnError;
          }
        }
        pFile2Check = pFile2Check->pNext;
      }
      if (FS_FAT_DeleteFileOrDir(pVolume, &SB, pDirEntry, 1) != 0) {
        pFile->Error = -1;
        goto OnError;
      } else {
        if (FAT_pDirEntryAPI->pfDelLongEntry) {
          FAT_pDirEntryAPI->pfDelLongEntry(pVolume, &SB, DirStart, LongDirEntryIndex);
        }
      }
      pDirEntry = NULL;          // File does not exist any more
    } else {
      if ((DoOpen | DoCreate) == 0) {
        r = 1;                   // This is an error unless some other command is executed
      }
    }
  }
  //
  // Open file if requested
  //
  if (DoOpen) {
    if (pDirEntry) {       // Does file exist ?
      //
      // Check read only
      //
      if ((((pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] & FS_FAT_ATTR_READ_ONLY) != 0)) &&
               (AccessFlags & (FS_FILE_ACCESS_FLAG_W | FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAG_C)))
      {
        // Files is RO and we try to create, write or append
        pFile->Error = FS_ERR_READONLY;
        if (DoCreate == 0) {
          r = 1;                       // This is an error unless some other command is executed
        }
      }
      FirstCluster = FS_FAT_GetFirstCluster(pDirEntry);
      FileSize     = FS_LoadU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE]);
      DoCreate = 0;      // Do not create, since it could be opened
    } else {
      if (DoCreate == 0) {
        r = 1;                       // This is an error unless some other command is executed
      }
    }
  }
  //
  // Do we need to create the file ?
  //
  if (DoCreate == 1 ) {
    if (pDirEntry == (FS_FAT_DENTRY *)NULL) {
      U32 TimeDate;

      TimeDate = FS_X_GetTimeDate();
      //
      // Create new file
      //
      pDirEntry = FAT_pDirEntryAPI->pfCreateDirEntry(pVolume, &SB, pFName, DirStart, 0, FS_FAT_ATTR_ARCHIVE, 0, (U16)(TimeDate & 0xffff), (U16)(TimeDate >> 16));
      if (pDirEntry) {
        /* Free entry found. */
        DirEntryIndex = pDirEntry - (FS_FAT_DENTRY*)SB.pBuffer;
        DirEntrySector = SB.SectorNo;
      } else {
        r = 1;                       // Error, could not create file
      }
      FileSize = 0;
      FirstCluster = 0;
    } else {
      r = 1;                        // Error, file already exists, we can recreate am additional directory entry
    }
  }
  pFileObj->Data.Fat.DirEntrySector = DirEntrySector;
  pFileObj->Data.Fat.DirEntryIndex  = (U16)DirEntryIndex;
  pFileObj->Data.Fat.CurClusterFile = 0xFFFFFFFF;   // Invalidate
  pFileObj->FirstCluster            = FirstCluster;
  pFileObj->Size                    = FileSize;
  pFile->FilePos                    = (AccessFlags & FS_FILE_ACCESS_FLAG_A) ? pFileObj->Size : 0;
OnError:
  if (pFile->Error < 0) {
    r = 1;
  }
  FS__SB_Delete(&SB);
  return r;
}

/*************************** End of file ****************************/

