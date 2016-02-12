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
File        : FAT_CheckDisk.c
Purpose     : Implementation of FS_FAT_CheckDisk
---------------------------END-OF-HEADER------------------------------
*/

#include "FS_Int.h"
#include "FAT_Intern.h"

/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/
typedef struct {
  U32 Value;
  U32 DirFirstCluster;
  U32 DirEntryIndex;
} CLUSTER_INFO;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static int _FileIndex;
static int _LastDirIndex;
static int _UseSameDir;
static int _AbortRequested;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _IsValidEntryChar
*
*/
static int _IsValidEntryChar(U8 Char2Check) {
  if (Char2Check == ' ') {
    return 1;
  }
  return FS_FAT_IsValidShortNameChar(Char2Check);
}

/*********************************************************************
*
*       _CheckFileName
*
*/
static int _CheckName(FS_FAT_DENTRY * pDirEntry) {
  int i;
  for (i = 0; i < 11; i++) {
    U8 Char2Check;
        
    if (pDirEntry->data[i] != 0x05) {
      Char2Check = pDirEntry->data[i];
    } else {
      Char2Check = 0xe5;
    }

    if (_IsValidEntryChar(Char2Check) == 0) {
      return 1;
    }
    if ((Char2Check >= 'a') && (Char2Check < 'z')) {
      return 1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       _CalcCheckSum
*
*  Return value
*    0        Equal
*    1        Not equal
*/
static U8 _CalcCheckSum(const char * sShortName) {
  U8 Sum;
  int i;

  Sum = 0;
  for (i = 0; i < 11; i++) {
    if (Sum & 1) {
      Sum = (Sum >> 1) | 0x80;
    } else {
      Sum >>= 1;
    }
    Sum += *sShortName++;
  }
  return Sum;
}


/*********************************************************************
*
*       _CheckLongDirEntry
*
*/
static int _CheckLongDirEntry(FS_VOLUME * pVolume, FS_DIR_POS * pDirPos, FS_FAT_DENTRY * pDirEntry, FS_SB * pSB) {
  FS_DIR_POS DirPos;
  U32        CurrentIndex;
  U32        NumEntries;
  U8         Checksum;
  U8         CalcedCheckSum;

  DirPos = *pDirPos; // Save old DirPos settings
  //
  //  Ensure that this is the last long file name entry.
  //  (Note 1)
  //
  if ((pDirEntry->data[0]& 0x40) == 0) {
    pDirEntry->data[0] = 0xe5;  // Error, found a long file name entry with no last entry flag.
    FS__SB_MarkDirty(pSB);
    return 1;
  }
  CurrentIndex = pDirEntry->data[0]& 0x3f;
  NumEntries   = CurrentIndex;
  Checksum     = pDirEntry->data[13];
  do {
    //
    // Check entry for validity
    //
    if (pDirEntry->data[13] != Checksum) {
      pDirEntry->data[0] = 0xe5;  // Error, check sum does not match
      FS__SB_MarkDirty(pSB);
      return 1;      
    }
    if (pDirEntry->data[11] != FS_FAT_ATTR_LONGNAME) {
      pDirEntry->data[0] = 0xe5;  // Error, attributes do not match.
      FS__SB_MarkDirty(pSB);
      return 1;      
    }
    if (FS_LoadU16LE(&pDirEntry->data[26]) != 0) {
      pDirEntry->data[0] = 0xe5;  // Error, First cluster information should be zero.
      FS__SB_MarkDirty(pSB);
    }
    //
    // Get the next directory entry.
    //
    FS_FAT_IncDirPos(pDirPos);
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, pDirPos);
    //
    // No more entries available.
    //
    if (pDirEntry->data[0] == 0) {
      return 0;
    }
  } while (--CurrentIndex);
  //
  // Check if calced Check sum of short directory matches with the checksum stored in the LFN entry(ies)
  //
  CalcedCheckSum = _CalcCheckSum((const char *)&pDirEntry->data[0]);
  if (CalcedCheckSum != Checksum) {    
    *pDirPos = DirPos;    
    do {
      pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, pDirPos);
      pDirEntry->data[0] = 0xe5;
      FS_FAT_IncDirPos(pDirPos);
    } while (--NumEntries);
    FS__SB_MarkDirty(pSB);
    return 1;
  }
  //
  //  Move back to the last LFN entry. The short file name entry shall be check the other routine.
  //
  pDirPos->DirEntryIndex--;
  return 0;
}

/*********************************************************************
*
*       _CheckDirEntry
*
*/
static int _CheckDirEntry(FS_FAT_INFO * pFATInfo, FS_FAT_DENTRY * pDirEntry) {
  U8  Attributes;
  U32 FirstCluster;
  U32 FileSize;
  U32 TotalBytesOnDisk;

  Attributes = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
  //
  //  If any bits other than specified by FAT in 
  //  the attributes field are set, mark as invalid
  //
  if (Attributes & ~(FS_FAT_ATTR_MASK)) {
    return 1;
  }
  //
  //  Check the short directory entry
  //
  if (Attributes != FS_FAT_ATTR_LONGNAME) {
    if ((Attributes != FS_FAT_ATTR_DIRECTORY) && (pDirEntry->data[0] != '.')) {
      if (_CheckName(pDirEntry)) {
        return 1;
      }
    }
    FirstCluster = FS_FAT_GetFirstCluster(pDirEntry);
    if (FirstCluster > pFATInfo->NumClusters) {
      return 1;
    }
    TotalBytesOnDisk = FS__CalcSizeInBytes(pFATInfo->NumClusters, pFATInfo->SecPerClus, pFATInfo->BytesPerSec);
    FileSize = FS_LoadU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE]);
    if (FileSize > TotalBytesOnDisk) {
      return 1;
    }

  }
  return 0;
}

/*********************************************************************
*
*       _CreateFileName
*
*/
static void _CreateFileName(char * pFileName) {
  char * p;
  p = pFileName;
  FS_STRCPY(pFileName, "FILE");
  p   += 4;
  *p++ = (_FileIndex  / 1000)        + '0';
  *p++ = ((_FileIndex % 1000) / 100) + '0';
  *p++ = ((_FileIndex % 100)  /  10) + '0';
  *p++ = ((_FileIndex % 10)        ) + '0';
  *p = 0;
  FS_STRCAT(p, ".CHK");
  _FileIndex++;
}

/*********************************************************************
*
*       _CreateDirName
*
*/
static void _CreateDirName(char * pDirName) {
  char * p;
  p = pDirName;
  FS_STRCPY(pDirName, "FOUND");
  p   += 5;
  *p++ = '.';
  *p++ = ((_LastDirIndex % 1000) / 100) + '0';
  *p++ = ((_LastDirIndex % 100)  /  10) + '0';
  *p++ = ((_LastDirIndex % 10)        ) + '0';
  *p = 0;
}
 
/*********************************************************************
*
*       _IsClusterEOC
*
*/
static U32 _IsClusterEOC(FS_VOLUME * pVolume, FS_SB * pSB, U32 Cluster) {
  Cluster = FS_FAT_ReadFATEntry(pVolume, pSB, Cluster);
  switch (pVolume->FSInfo.FATInfo.FATType) {
  case FS_FAT_TYPE_FAT12:
    return ((Cluster & 0xFFF) == 0xFFF);
  case FS_FAT_TYPE_FAT16:
    return ((Cluster & 0xFFFF) == 0xFFFF);
  }
  return ((Cluster & 0xFFFFFFFUL) == 0xFFFFFFFUL);
}

/*********************************************************************
*
*       _SetFileLen
*
*/
static void _SetFileLen(FS_VOLUME * pVolume, FS_DIR_POS * pDirPos, U32 Size, FS_SB * pSB) {
  FS_FAT_DENTRY  * pDirEntry;

  pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, pDirPos);
  FS_StoreU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE], Size);
  if (Size == 0) {
    FS_FAT_WriteDirEntryCluster(pDirEntry, 0);
  }
  FS__SB_MarkDirty(pSB);
}


/*********************************************************************
*
*       _ConvertLostClusters2File
*
*/
static int _ConvertLostClusterChain2File(FS_VOLUME * pVolume, U32 DirStart, U32 FirstCluster, char * sFileName, FS_SB * pSB) {
  U32             NumClusters;
  U32             FileSize;
  U32             LastCluster;
  FS_FAT_INFO   * pFATInfo;
  FS_FAT_DENTRY * pDirEntry;
  U32             DateTime;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  LastCluster = FS_FAT_FindLastCluster(pVolume, pSB, FirstCluster, &NumClusters);
  /*
   * Check if the last cluster in the chain otherwise it will be corrected.
   */
  if (LastCluster == 0) {
    FS_FAT_MarkClusterEOC(pVolume, pSB, LastCluster);
  }
  if (NumClusters == 0) {
    FS_FAT_MarkClusterEOC(pVolume, pSB, LastCluster);
  }
  FileSize = (NumClusters + 1) * (pFATInfo->BytesPerCluster);
  do {
    pDirEntry = FAT_pDirEntryAPI->pfFindDirEntry(pVolume, pSB, sFileName, strlen(sFileName), DirStart, 0, NULL);
    if (pDirEntry == (FS_FAT_DENTRY *)NULL) {
      break;
    }
    _CreateFileName(sFileName);
  } while(1);
  /*
   * Create the directory entry for the lost cluster chain
   */
  DateTime = FS_X_GetTimeDate();
  pDirEntry = FAT_pDirEntryAPI->pfCreateDirEntry(pVolume, pSB, sFileName, DirStart, FirstCluster, FS_FAT_ATTR_ARCHIVE, FileSize, (U16)(DateTime & 0xffff), (U16)(DateTime >> 16));
  if (pDirEntry == (FS_FAT_DENTRY *)NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "Failed to create directory entry, no space available"));
    return 2;
  }
  return 0;
}

/*********************************************************************
*
*       _CheckFileName
*
*/
static void _CheckFileName(FS_VOLUME * pVolume, FS_SB * pSB, FS_DIR_POS * pDirPos, U32 DirCluster) {
  FS_FAT_DENTRY  * pDirEntry;

  pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, pDirPos);
  if (_CheckName(pDirEntry)) {
    char       acFileName[13];
    FS_83NAME  Name;
    FS_SB      SB;

    FS__SB_Create(&SB, &pVolume->Partition);
    do {
      FS_FAT_DENTRY  * pDirEntryNew;

      _CreateFileName(acFileName);
      pDirEntryNew = FS_FAT_FindDirEntry(pVolume, &SB, acFileName, strlen(acFileName), DirCluster, 0, NULL);
      if (pDirEntryNew == (FS_FAT_DENTRY *)NULL) {
        break;
      }
    } while(1);
    FS__SB_Delete(&SB);
    FS_FAT_Make83Name(&Name, acFileName, strlen(acFileName));
    FS_MEMCPY(pDirEntry->data, Name.ac, sizeof(Name.ac));
    FS__SB_MarkDirty(pSB);
  }
}

/*********************************************************************
*
*       _GetFixDir
*
*/
static U32 _GetFixDir(FS_VOLUME * pVolume, U32 DirStart, FS_SB * pSB, int UseSameDir) {
  char             acDirName[13];
  FS_FAT_DENTRY  * pDirEntry;
  U32              r;

  _CreateDirName(acDirName);
  if (UseSameDir == 0) {
    do {
      _LastDirIndex++;
      _CreateDirName(acDirName);
      pDirEntry = FS_FAT_FindDirEntry(pVolume, pSB, acDirName, strlen(acDirName), DirStart, 0, NULL);
      if (pDirEntry == (FS_FAT_DENTRY *)NULL) {
        if (FS_FAT_CreateDirEx(pVolume, acDirName, DirStart, pSB)) {
          return 0xFFFFFFUL;
        }
        break;
      }
    } while (1);
  }
  pDirEntry = FS_FAT_FindDirEntry(pVolume, pSB, acDirName, strlen(acDirName), DirStart, FS_FAT_ATTR_DIRECTORY, NULL);
  if (pDirEntry) {
    r = FS_FAT_GetFirstCluster(pDirEntry);
  } else {
    r = 0xFFFFFFUL;
  }
  return r;
}

/*********************************************************************
*
*       _CheckFile
*
*  Return value
*    0    O.K.
*    1    Error -> an error has be found and repaired, retry is required.
*    2    User specified an abort of checkdisk operation thru callback.
*/
static int _CheckFile(FS_VOLUME * pVolume,
               U32                FirstFileCluster,
               U32                FileSize,
               CLUSTER_INFO     * paClusterInfo,
               U32                FirstClusterInArray,
               I32                NumClustersInArray,
               FS_SB            * pSB,
               U32                DirCluster,
               FS_DIR_POS       * pDirPos,
               FS_QUERY_F_TYPE  * pfOnError)
{
  FS_FAT_INFO    * pFATInfo;
  FS_SB            SBFat;
  FS_SB          * pSBFat;
  CLUSTER_INFO   * pClusterInfo;
  U32           NumClustersUsed;
  U32           Cluster;
  U32           LastCluster;
  int           r;
  U32           i;

  
  _CheckFileName(pVolume, pSB, pDirPos, DirCluster);

  if (FS__SB_Create(&SBFat, &pVolume->Partition) == 0) {
    pSBFat = &SBFat;
  } else {
    pSBFat = pSB;
  }
  if (FileSize == 0) {
    if (FirstFileCluster == 0) {
      r = 0;
      goto OnExit;
    } else {
      r = pfOnError(FS_ERRCODE_0FILE, FirstFileCluster);
      if (r > 0) {
        if (r == 1) {
          _SetFileLen(pVolume, pDirPos, 0, pSB);
          FS_FAT_FreeClusterChain(pVolume, pSBFat, FirstFileCluster, 0xFFFFFFFFUL);
        } 
        goto OnExit;
      }
    }
  }
  r = 0;
  LastCluster = 0;
  pFATInfo = &pVolume->FSInfo.FATInfo;
  NumClustersUsed = (FileSize + pFATInfo->BytesPerCluster - 1) / pFATInfo->BytesPerCluster;
  Cluster = FirstFileCluster;
  for (i = 0; Cluster; i++) {
    I32 Off;
    /* Check if max. size has been exceeded */
    if (i > (NumClustersUsed - 1)) {
      r = pfOnError(FS_ERRCODE_SHORTEN_CLUSTER, Cluster);
      if (r > 0) {
        if (r == 1) {
          FS_FAT_MarkClusterEOC(pVolume, pSBFat, LastCluster);
          FS_FAT_FreeClusterChain(pVolume, pSBFat, Cluster, 0xFFFFFFFFUL);
          // Need to shorten cluster chain
        }
        goto OnExit;
      }
    }
    /* Add it to the cluster info array */
    Off = (I32)Cluster - (I32)FirstClusterInArray;
    if ((Off >= 0) && (Off < NumClustersInArray)) {
      pClusterInfo = paClusterInfo + Off;
      if (pClusterInfo->Value == 0xFFFFFFFFUL) {
        pClusterInfo->Value           = Cluster;
        pClusterInfo->DirEntryIndex   = pDirPos->DirEntryIndex;
        pClusterInfo->DirFirstCluster = pDirPos->FirstCluster;
      } else {
        r = pfOnError(FS_ERRCODE_CROSSLINKED_CLUSTER, Cluster, pDirPos->FirstCluster, pDirPos->DirEntryIndex);
        if (r > 0) {
          if (r == 1) {
            /* Fix it ! */
            _SetFileLen(pVolume, pDirPos, i * pFATInfo->BytesPerCluster, pSB);
          }
          goto OnExit;
        }
      }
    }
    //
    //  Check if cluster is beyond the number of clusters
    // 
    if ((Cluster - 2) >= pVolume->FSInfo.FATInfo.NumClusters) {
      r = pfOnError(FS_ERRCODE_INVALID_CLUSTER, Cluster);
      if (r > 0) {
        if (r == 1) {
          if (FS_FAT_MarkClusterEOC(pVolume, pSBFat, LastCluster)) {
            _SetFileLen(pVolume, pDirPos, 0, pSB);
          }
        }
        goto OnExit;
      }
    }
    LastCluster = Cluster;
    Cluster = FS_FAT_WalkCluster(pVolume, pSBFat, Cluster, 1);
    /* Check if the last cluster is marked as end-of-chain */
    if (i == (NumClustersUsed - 1)) {
      if ((_IsClusterEOC(pVolume, pSB, LastCluster) == 0)) {
        r = pfOnError(FS_ERRCODE_CLUSTER_NOT_EOC, LastCluster);
        if (r > 0) {
          if (r == 1) {
            if (FS_FAT_MarkClusterEOC(pVolume, pSBFat, LastCluster)) {
              _SetFileLen(pVolume, pDirPos, 0, pSB);
            }
          }
          goto OnExit;
        }
      }
    }
  }
  if (i != NumClustersUsed) {
    r = pfOnError(FS_ERRCODE_FEW_CLUSTER);
    if (r > 0) {
      if (r == 1) {
        _SetFileLen(pVolume, pDirPos, i * pFATInfo->BytesPerCluster, pSB);
      }
      goto OnExit;
    }
  }
OnExit:
  FS__SB_Delete(&SBFat);
  return r;
}

/*********************************************************************
*
*       _IsDotFolderEntryValid
*
*  Return value:
*    0       -  Entry is O.K.
*    1       -  Error.
*
*/
static int _IsDotFolderEntryValid(FS_FAT_DENTRY * pDirEntry, FS_DIR_POS * pDirPos, const char * sDirEntryName, U32 ClusterId) {
  FS_USE_PARA(pDirPos);
  if (!pDirEntry) {
    goto OnError;
  }
  if (memcmp(&pDirEntry->data[0], sDirEntryName, 11) != 0) {
    goto OnError;
  }
  if ((pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] & FS_FAT_ATTR_DIRECTORY) == 0) {
    goto OnError;
  }
  if (FS_FAT_GetFirstCluster(pDirEntry) != ClusterId) {
    goto OnError;
  }
  return 0;
OnError:
  return 1;
}

/*********************************************************************
*
*       _CheckDir
*
*  Return value
*    0    O.K.
*    1    Error -> an error has be found and repaired, retry is required.
*    2    User specified an abort of checkdisk operation thru callback.
*/
static int _CheckDir(FS_VOLUME * pVolume, U32 DirCluster, CLUSTER_INFO * paClusterInfo, U32 FirstClusterInArray, I32 NumClustersInArray, int MaxRecursionLevel, FS_SB * pSB, FS_QUERY_F_TYPE * pfOnError) {
  FS_DIR_POS       DirPos;
  FS_FAT_INFO    * pFATInfo;
  FS_FAT_DENTRY  * pDirEntry;
  int              r;
  U8               Attributes;
  FS_DIR_POS       DirPos2Check;
  FS_FAT_DENTRY  * pDirEntry2Check;
  U32              RootDirCluster;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  /* Iterate over directory entries */
  RootDirCluster = DirCluster;
  FS_FAT_InitDirEntryScan(pFATInfo, &DirPos, DirCluster);
  r = 1;
  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
    if (!pDirEntry) {
      break;
    }
    if (pDirEntry->data[0] == 0x00) {         /* Last entry found ? */
      break;
    }
    if (pDirEntry->data[0] != (U8)0xE5) {     /* not a deleted file */
      //
      // Check the entry for validity
      //
      if (_CheckDirEntry(pFATInfo, pDirEntry)) {
        //
        // invalid entry, mark it as deleted
        //
        pDirEntry->data[0] = 0xe5;
        FS__SB_MarkDirty(pSB);
        return 1;
      }
      //
      // Check if the directory entry has the directory attribute set
      //
      Attributes = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
      if ((Attributes & FS_FAT_ATTR_LONGNAME) != FS_FAT_ATTR_LONGNAME) {
        if ((Attributes & FS_FAT_ATTR_DIRECTORY) == FS_FAT_ATTR_DIRECTORY) {
          if (pDirEntry->data[0] != '.') {
            if (MaxRecursionLevel) {
              I32            Off;
              CLUSTER_INFO * pClusterInfo;

              DirCluster     = FS_FAT_GetFirstCluster(pDirEntry);
              //
              // Check if cluster FAT entry is not empty.
              //
              if (FS_FAT_ReadFATEntry(pVolume, pSB, DirCluster) == 0) {
                FS_FAT_InitDirEntryScan(pFATInfo, &DirPos2Check, DirCluster);
                pDirEntry2Check = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos2Check);
                
                //
                //  Check if directory match to directory entry
                //
                if ((pDirEntry2Check->data[0] != '.') || (FS_FAT_GetFirstCluster(pDirEntry) != DirCluster)) {
                  //
                  //  Retrieve the directory entry and mark it as deleted
                  //  since the directory entry does not point to a valid directory.
                  //
                  pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
                  pDirEntry->data[0] = 0xe5;
                  FS__SB_MarkDirty(pSB);
                  return 1;
                } else {
                  //
                  // Repair  the FAT entry, since the directory is valid.
                  r = pfOnError(FS_ERRCODE_CLUSTER_NOT_EOC);
                  if (r > 0) {
                    if (r == 1) {
                      FS_FAT_MarkClusterEOC(pVolume, pSB, DirCluster);
                    } else {
                      return 2;
                    }
                  }
                }
              }
              pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
              if (FS_LoadU32LE(&pDirEntry ->data[DIR_ENTRY_OFF_SIZE]) != 0) {
                //
                // Directory is not a valid directory
                // we convert it into a file.
                //
                pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] &= ~(FS_FAT_ATTR_DIRECTORY);
                FS__SB_MarkDirty(pSB);
                return 1;
              }
              //
              // If we have a directory and it is not a root directory.
              // We need to check whether there are '..' and '.' directory entries
              // otherwise we convert the directory into a file.
              //
              FS_FAT_InitDirEntryScan(pFATInfo, &DirPos2Check, DirCluster);
              pDirEntry2Check = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos2Check);
              //
              // Check the "." Entry
              //
              r = _IsDotFolderEntryValid(pDirEntry2Check, &DirPos, ".          ", DirPos2Check.FirstCluster);
              if (r != 0) {
                pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
                pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] &= ~(FS_FAT_ATTR_DIRECTORY);
                FS__SB_MarkDirty(pSB);
                return r;
              }
              //
              // Check the ".." Entry
              //
              FS_FAT_IncDirPos(&DirPos2Check);
              pDirEntry2Check = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos2Check);
              r = _IsDotFolderEntryValid(pDirEntry2Check, &DirPos2Check, "..         ", RootDirCluster);
              if (r != 0) {
                pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
                pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] &= ~(FS_FAT_ATTR_DIRECTORY);
                FS__SB_MarkDirty(pSB);
                return r;
              }
              /* Add it to the cluster info array */
              Off = (I32)DirCluster - (I32)FirstClusterInArray;
              if ((Off >= 0) && (Off < NumClustersInArray)) {
                pClusterInfo = paClusterInfo + Off;
                if (pClusterInfo->Value == 0xFFFFFFFFUL) {
                  pClusterInfo->Value           = DirCluster;
                  pClusterInfo->DirEntryIndex   = DirPos.DirEntryIndex;
                  pClusterInfo->DirFirstCluster = DirPos.FirstCluster;
                }
              }
              r = _CheckDir(pVolume, DirCluster, paClusterInfo, FirstClusterInArray, NumClustersInArray, MaxRecursionLevel - 1, pSB, pfOnError);
              if (r) {
                return r;                 // Error
              }
            } else {
              return 2;  // Abort -> Max recursion level limit reached.
            }
          }
        } else {
          U32 FileSize;
          U32 FirstFileCluster;

          FirstFileCluster = FS_FAT_GetFirstCluster(pDirEntry);
          FileSize         = FS_LoadU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE]);
          r = _CheckFile(pVolume, FirstFileCluster, FileSize, paClusterInfo, FirstClusterInArray, NumClustersInArray, pSB, DirCluster, &DirPos, pfOnError);
          if (r) {
            return r;
          }
        }
      } else {
        //
        // Check Long file name entry
        //
        if (_CheckLongDirEntry(pVolume, &DirPos, pDirEntry, pSB)) {
          return 1;
        }
      }

    }
    FS_FAT_IncDirPos(&DirPos);
    /* Moved to another cluster to check the directory entries ? */
    if (DirPos.Cluster != DirCluster) {
      I32            Off;
      CLUSTER_INFO * pClusterInfo;

      if (FS_FAT_ReadFATEntry(pVolume, pSB, DirPos.Cluster) == 0) {
        // Repair  the FAT entry, since the directory is valid.
        r = pfOnError(FS_ERRCODE_CLUSTER_NOT_EOC);
        if (r > 0) {
          if (r == 1) {
            FS_FAT_MarkClusterEOC(pVolume, pSB, DirCluster);
            return 1;
          } else {
            return 2;
          }
        }
      }
      /* Add it to the cluster info array */
      Off = (I32)DirPos.Cluster  - (I32)FirstClusterInArray;
      if ((Off >= 0) && (Off < NumClustersInArray)) {
        pClusterInfo = paClusterInfo + Off;
        if (pClusterInfo->Value == 0xFFFFFFFFUL) {
          pClusterInfo->Value           = DirPos.Cluster;
          pClusterInfo->DirEntryIndex   = DirPos.DirEntryIndex;
          pClusterInfo->DirFirstCluster = DirPos.FirstCluster;
        }
      }    
    }

  } while (1);
  return 0;
}


/*********************************************************************
*
*       _CheckFAT
*
*  Return value
*    0    O.K.
*    1    Error -> an error has be found and repaired, retry is required.
*    2    User specified an abort of checkdisk operation thru callback.
*/
static int _CheckFAT(FS_VOLUME * pVolume, void * pBuffer, U32 FirstClusterInArray, I32 NumClustersInArray, int MaxRecursionLevel, FS_SB * pSB, FS_QUERY_F_TYPE * pfOnError) {
  I32              i;
  U32              DirStart;
  CLUSTER_INFO   * paClusterInfo;
  const char     * sFileName;
  FS_FAT_INFO    * pFATInfo;
  int              r;


  /*
   * Search directory
   */
  if (FS_FAT_FindPath(pVolume, pSB, "", &sFileName, &DirStart) == 0) {
    return 2;  /* Directory not found */
  }
  pFATInfo = &pVolume->FSInfo.FATInfo;
  paClusterInfo = (CLUSTER_INFO *) pBuffer;
  FS_MEMSET(paClusterInfo, 0xFF, NumClustersInArray * sizeof(CLUSTER_INFO));
  if (pFATInfo->FATType == FS_FAT_TYPE_FAT32) {
    if (DirStart == 0) {
      I32            Off;
      U32            DirCluster;
      CLUSTER_INFO * pClusterInfo;

      DirCluster = pFATInfo->RootDirPos;
      /* Add it to the cluster info array */
      Off = (I32)DirCluster - (I32)FirstClusterInArray;
      if ((Off >= 0) && (Off < NumClustersInArray)) {
        pClusterInfo = paClusterInfo + Off;
        if (pClusterInfo->Value == 0xFFFFFFFFUL) {
          pClusterInfo->Value           = DirCluster;
          pClusterInfo->DirEntryIndex   = 0;
          pClusterInfo->DirFirstCluster = 0;
        }
      }
    }
  }
  r = _CheckDir(pVolume, DirStart, paClusterInfo, FirstClusterInArray, NumClustersInArray, MaxRecursionLevel, pSB, pfOnError);
  if (r) {
    return r;
  }
  //
  // Check Fat entries
  //
  for (i = 0; i < NumClustersInArray; i++) {
    CLUSTER_INFO * pClusterInfo;
    int            UseSameDir;

    UseSameDir   = _UseSameDir;
    pClusterInfo = paClusterInfo + i;
    if (pClusterInfo->Value == 0xFFFFFFFFUL) {
      U32 FATEntry;

      FATEntry = FS_FAT_ReadFATEntry(pVolume, pSB, i + FirstClusterInArray);
      if ((FATEntry)) {
        pClusterInfo->Value = FATEntry;
      }
    }
    if ((pClusterInfo->Value           != 0xFFFFFFFFUL) &&
        (pClusterInfo->DirFirstCluster == 0xFFFFFFFFUL) &&
        (pClusterInfo->DirEntryIndex   == 0xFFFFFFFFUL)) {
      r = pfOnError(FS_ERRCODE_CLUSTER_UNUSED, i + FirstClusterInArray);
      if (r > 0) {
        if (r == 1) {
          FS_DIR_POS       DirPos;
          char             acFileName[13];
          FS_FAT_DENTRY  * pDirEntry;
          U32              FileSize;
          U32              FirstCluster;
          U32              FixDirStart;

          FixDirStart = _GetFixDir(pVolume, 0, pSB, UseSameDir);
          UseSameDir  = 1;
          pClusterInfo->Value = 0xFFFFFFFFUL;
          /* Fix it ! */
          _CreateFileName(acFileName);
          r = _ConvertLostClusterChain2File(pVolume, FixDirStart, i + FirstClusterInArray, acFileName, pSB);
          if (r) {
            return r;
          }
          //
          //  Check the newly created file for validity
          //
          FS_FAT_InitDirEntryScan(&pVolume->FSInfo.FATInfo, &DirPos, FixDirStart);
          pDirEntry    = FS_FAT_FindDirEntryShortEx(pVolume, pSB, acFileName, sizeof(acFileName), &DirPos, 0);
          FileSize     = FS_LoadU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE]);
          FirstCluster =  i + FirstClusterInArray;
          r = _CheckFile(pVolume, FirstCluster, FileSize, paClusterInfo, FirstClusterInArray, NumClustersInArray, pSB, DirStart, &DirPos, pfOnError);
          if (r) {
            return r;
          }
        } else if (r == 3) {
            U32 FirstCluster;
            U32 LastCluster;
            U32 NumClusters;

            FirstCluster = i + FirstClusterInArray;
            LastCluster = FS_FAT_FindLastCluster(pVolume, pSB, FirstCluster, &NumClusters);
            /*
             * Check if the last cluster in the chain otherwise it will be corrected.
             */
            if (LastCluster == 0) {
              FS_FAT_MarkClusterEOC(pVolume, pSB, LastCluster);
            }
            if (NumClusters == 0) {
              FS_FAT_MarkClusterEOC(pVolume, pSB, LastCluster);
            }
            if (_IsClusterEOC(pVolume, pSB, LastCluster) == 0) {
              FS_FAT_MarkClusterEOC(pVolume, pSB, LastCluster);
            }
            FS_FAT_FreeClusterChain(pVolume, pSB, FirstCluster, NumClusters + 1);
            return 1;

        } else {
          return r;
        }

      }
    }
  }
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
*       FS_FAT__CheckDisk
*
*  Return value
*    0    O.K.
*    1    Error -> an error has be found and repaired, retry is required.
*    2    User specified an abort of checkdisk operation thru callback.
*/
int FS_FAT__CheckDisk(FS_VOLUME * pVolume, FS_DISK_INFO * pDiskInfo, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_QUERY_F_TYPE * pfOnError) {
  U32            NumClusters;
  U32            iCluster;
  U32            NumClustersAtOnce;
  U32            NumClustersPerSector;
  FS_SB          SB;
  int            r = 2;

  FS__SB_Create(&SB, &pVolume->Partition);
  NumClusters           = pDiskInfo->NumTotalClusters;
  NumClustersAtOnce     = BufferSize / sizeof(CLUSTER_INFO);
  NumClustersPerSector  = pVolume->FSInfo.FATInfo.BytesPerSec / pVolume->FSInfo.FATInfo.FATType;
  NumClustersAtOnce    &= ~(NumClustersPerSector - 1);
  NumClustersAtOnce     = MIN(NumClustersAtOnce, NumClusters);
  for (iCluster = FAT_FIRST_CLUSTER; NumClusters;) {
    r = _CheckFAT(pVolume, pBuffer, iCluster, NumClustersAtOnce, MaxRecursionLevel, &SB, pfOnError);
    if (_AbortRequested) {
      //
      // Reset the internal variable.
      //
      _AbortRequested = 0;
      //
      // Return 2 in order to abort checkdisk.
      //
      r = 2;
    }
    if (r) {
      break;
    }
    iCluster         += NumClustersAtOnce;
    NumClusters      -= NumClustersAtOnce;
    NumClustersAtOnce = MIN(NumClustersAtOnce, NumClusters);
  }
  FS__SB_Delete(&SB);
  if (r == 0) {
    _UseSameDir = 0;
  } else {
    _UseSameDir = 1;
  }
  return r;
}

/*********************************************************************
*
*       FS_FAT_AbortCheckDisk
*
*/
void FS_FAT_AbortCheckDisk(void) {
  _AbortRequested = 1;
}

/*************************** End of file ****************************/

