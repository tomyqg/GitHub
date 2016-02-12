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
File        : FAT_Write.c
Purpose     : FAT filesystem file write routines
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
*       Data structures
*
**********************************************************************
*/
typedef struct {
  U32          FirstSector;
  U32          NumSectors;
  FS_SB      * pSBData;
  const void * pData;
} BURST_INFO_W;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _WriteBurst
*
*/
static int _WriteBurst(BURST_INFO_W * pBurstInfo) {
  if (pBurstInfo->NumSectors) {
    if (FS_LB_WriteBurstPart(pBurstInfo->pSBData->pPart,
                             pBurstInfo->FirstSector,
                             pBurstInfo->NumSectors,
                             pBurstInfo->pData, FS_SECTOR_TYPE_DATA))
    {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_WriteBurst (FAT): Burst write error.\n"));
      return 1;     // Write error
    }
  }
  return 0;         // No problem !
}

/*********************************************************************
*
*       _WriteData
*
*  Return value
*    Number of bytes written
*/
static U32 _WriteData(const U8 * pData, U32 NumBytes2Write, FS_FILE *pFile, FS_SB * pSBData, FS_SB * pSBfat) {
  U32           NumBytesWritten;
  U32           NumBytesCluster;
  U32           BytesPerCluster;
  int           SectorOff;
  U32           SectorNo;
  FS_FILE_OBJ * pFileObj;
  FS_FAT_INFO * pFATInfo;
  char          DirUpdateRequired;
  char          ZeroCopyAllowed;
  U32           LastByteInCluster;
  BURST_INFO_W  BurstInfo;
#if FS_USE_FILE_BUFFER
  FS_FILE_BUFFER * pFileBuffer;
#endif
  //
  // Init / Compute some values used throughout the routine
  //
  DirUpdateRequired     = 0;
  pFileObj              = pFile->pFileObj;
  pFATInfo              = &pFileObj->pVolume->FSInfo.FATInfo;
  BytesPerCluster       = pFATInfo->BytesPerCluster;
  NumBytesWritten       = 0;
  BurstInfo.NumSectors  = 0;
  BurstInfo.FirstSector = 0xFFFFFFFFUL;
  BurstInfo.pSBData     = pSBData;
  ZeroCopyAllowed = 1;
#if FS_SUPPORT_CHECK_MEMORY
  {
    FS_MEMORY_IS_ACCESSIBLE_CALLBACK * pfMemoryIsAccessible;

    pfMemoryIsAccessible = pFileObj->pVolume->Partition.Device.Data.pfMemoryIsAccessible;
    if (pfMemoryIsAccessible) {
      if (pfMemoryIsAccessible((void *)pData, NumBytes2Write) == 0) {
        ZeroCopyAllowed = 0;
      }
    }
  }
#endif
  //
  // Main loop
  // We determine the cluster (allocate as necessary using the FAT buffer)
  // and write data into the cluster
  //
  do {
    //
    // Locate current cluster.
    //
    if (FS_FAT_GotoClusterAllocIfReq(pFile, pSBfat)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_WriteData: Could not alloc cluster to file.\n"));
      if (_WriteBurst(&BurstInfo)) {
        NumBytesWritten = 0;               /* We do not know how many bytes have been written o.k., so reporting 0 is on the safe side */
      }
      FS_FAT_UpdateDirEntry(pFileObj, pSBData);
      return NumBytesWritten;           /* File truncated (too few clusters) */
    }
    LastByteInCluster = BytesPerCluster * (pFileObj->Data.Fat.CurClusterFile + 1);
    NumBytesCluster   = LastByteInCluster - pFile->FilePos;
    SectorOff         = pFile->FilePos & (pFATInfo->BytesPerSec - 1);
    if (NumBytesCluster > NumBytes2Write) {
      NumBytesCluster = NumBytes2Write;
    }
    SectorNo  = FS_FAT_ClusterId2SectorNo(pFATInfo, pFileObj->Data.Fat.CurClusterAbs);
    SectorNo += (pFile->FilePos / pFATInfo->BytesPerSec) & (pFATInfo->SecPerClus -1);
    //
    // If the file buffer has been filled by previous calls of _WriteData
    // we have a special case here:
    // Example: 2 previous calls of _WriteData filled the file buffer and it holds data for a complete sector.
    // In this case we can not follow our standard procedure: "Write content of file buffer into smart buffer and append user data"
    // since the smart buffer can only hold data for 1 sector.
    // Moreover, the user data which shall be written now, shall be written to another sector
    // Let us simply clean the write buffer, if the current sector and the sector the file buffer shall be written to, differs.
    //
#if FS_USE_FILE_BUFFER
    if (pFileObj->pBuffer) {
      if (SectorNo != pFileObj->pBuffer->SectorNo) {
        FS_FAT_CleanWriteBuffer(pFile, NULL, 0, pSBData);
      }
    }
#endif
    //
    // Write data into the cluster, iterating over sectors
    //
    do {
      unsigned NumBytesSector;
      NumBytesSector = pFATInfo->BytesPerSec - SectorOff;
      if ((U32)NumBytesSector > NumBytesCluster) {
        NumBytesSector = (unsigned)NumBytesCluster;
      }
      //
      // Check if we can write an entire sector
      //
      if   ((ZeroCopyAllowed == 0)
#if FS_DRIVER_ALIGNMENT > 1      // Not required, just to avoid warnings
        || (((U32)pData & (FS_DRIVER_ALIGNMENT - 1)))
#endif
        || (NumBytesSector != pFATInfo->BytesPerSec))
      {
        //
        // If we can not write an entire sector or zero copy is not allowed, there are two behaviors,
        // depending on if file write buffers are supported or not.
        // 1. If they are not supported we behave as before: Read the sector,
        //    merge the content with the data to be written and write back the sector.
        // 2. If file write buffers are supported we can choose our logic arm.
        //
#if FS_USE_FILE_BUFFER
        pFileBuffer = pFileObj->pBuffer;
        if (pFileBuffer) {
          if (pFileBuffer->Flags & FS_FILE_BUFFER_WRITE) {
            I32 NumBytesAvail;
            //
            // It is only allowed to fill the file buffer under the following circumstances:
            // a) If the data which shall be written, is immediately written after the previous data
            //    (e.g. the previous call of _Write Data wrote 1 byte to file position 1 we can only
            //    use the file buffer if the current call of _WriteData() writes to file position 2.
            // b) If a sector is written partially and the file buffer is large enough to hold all data which should be written.
            //
            //
            // Clean file buffer if it contains unwritten data and if there is a gap between
            // the data in the file buffer and the data that shall be written now.
            //
            if (pFileBuffer->IsDirty) {
              if (pFile->FilePos != (pFileBuffer->FilePos + pFileBuffer->NumBytesInBuffer)) {
                FS_FAT_CleanWriteBuffer(pFile, NULL, 0, pSBData);
              }
            }
            //
            // If the file buffer does not contain unwritten data, initialize it.
            //
            if (pFileBuffer->IsDirty == 0) {
              pFileBuffer->FilePos          = pFile->FilePos;
              pFileBuffer->NumBytesInBuffer = 0;
            }
            //
            // Does data fit into write buffer ?  If so, then copy into buffer and we are done
            //
            NumBytesAvail = pFileBuffer->BufferSize - pFileBuffer->NumBytesInBuffer;
            if ((I32)NumBytes2Write <= (I32)NumBytesAvail) {
              FS_MEMCPY(pFileBuffer->pData + pFileBuffer->NumBytesInBuffer, pData, NumBytes2Write);
              pFileBuffer->SectorNo          = SectorNo;
              pFileBuffer->IsDirty           = 1;
              pFileBuffer->NumBytesInBuffer += NumBytes2Write;
              goto SectorDone;                                     // Updating file pos & size is taken care of at label "SectorDone"
            }
            //
            // Data does not fit into file buffer.
            // We clean the file buffer and append data for the rest of the sector.
            //
            FS_FAT_CleanWriteBuffer(pFile, pData, NumBytesSector, pSBData);
            goto SectorDone;
          } else {
            pFileObj->pBuffer->NumBytesInBuffer = 0;     // Invalidate Read buffer
          }
        }
#endif  // FS_USE_FILE_BUFFER
        //
        // Read the sector if we need to modify an existing one
        //
        if (SectorOff || (pFile->FilePos != pFileObj->Size)) {
          FS__SB_SetSector(pSBData, SectorNo, FS_SB_TYPE_DATA);
          if (FS__SB_Read(pSBData)) {
            FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_WriteData: Read error during write.\n"));
            pFile->Error = FS_ERR_READERROR;       /* read error during write */
            return NumBytesWritten;
          }
        }
        //
        // Merge the written data into the sector
        //
        FS_MEMCPY(pSBData->pBuffer + SectorOff, pData, NumBytesSector);
        //
        // Write sector
        //
        FS__SB_SetSector(pSBData, SectorNo, FS_SB_TYPE_DATA);
        if (FS__SB_Write(pSBData)) {
          return NumBytesWritten;          /* Could not write  sector */
        }
      } else {
        //
        // Write the sector with "Zero-copy"
        //
        if (SectorNo != BurstInfo.FirstSector + BurstInfo.NumSectors) {
          if (_WriteBurst(&BurstInfo)) {
            return NumBytesWritten;
          }
          BurstInfo.FirstSector = SectorNo;
          BurstInfo.NumSectors  = 1;
          BurstInfo.pData       = pData;
        } else {
          BurstInfo.NumSectors++;
        }
      }
      //
      // Update management info
      //
#if FS_USE_FILE_BUFFER
SectorDone:
#endif
      pData           += NumBytesSector;
      NumBytesCluster -= NumBytesSector;
      NumBytes2Write  -= NumBytesSector;
      NumBytesWritten += NumBytesSector;
      pFile->FilePos  += NumBytesSector;
      SectorNo++;
      SectorOff = 0;                /* Next sector will be written from start */
      //
      // Update File size
      //
      if (pFile->FilePos > pFileObj->Size) {
        if (pFileObj->Size == 0) {          // In this case we have allocated a cluster and need to update the dir entry!
          DirUpdateRequired = 1;
        } else {
          if (FS_Global.WriteMode == FS_WRITEMODE_SAFE) {
            DirUpdateRequired = 1;
          }
        }
        pFileObj->Size = pFile->FilePos;
      }
    } while (NumBytesCluster);
  } while (NumBytes2Write);
  //
  // Flush Burst
  //
  if (_WriteBurst(&BurstInfo)) {
    NumBytesWritten = 0;               /* We do not know how many bytes have been written o.k., so reporting 0 is on the safe side */
  }
  //
  // Update directory entry if required
  //
  if (DirUpdateRequired) {
    FS_FAT_UpdateDirEntry(pFileObj, pSBData);
  }
  return NumBytesWritten;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_UpdateDirEntry
*/
void FS_FAT_UpdateDirEntry(FS_FILE_OBJ * pFileObj, FS_SB * pSB) {
  FS_FAT_DENTRY * pDirEntry;
  U32             TimeDate;
  U32             DirSectorNo;
  U16             BytesPerSector;
  U16             SectorOff;

  BytesPerSector = pFileObj->pVolume->FSInfo.FATInfo.BytesPerSec;
  DirSectorNo    = pFileObj->Data.Fat.DirEntrySector;
  FS__SB_SetSector(pSB, DirSectorNo, FS_SB_TYPE_DIRECTORY);
  SectorOff = (pFileObj->Data.Fat.DirEntryIndex  * sizeof(FS_FAT_DENTRY)) & (BytesPerSector - 1);
  pDirEntry = (FS_FAT_DENTRY *) (pSB->pBuffer + SectorOff);

  if (FS__SB_Read(pSB) == 0) {
    //
    // Modify directory entry
    //
    FS_StoreU32LE(&pDirEntry->data[DIR_ENTRY_OFF_SIZE], pFileObj->Size);
    FS_FAT_WriteDirEntryCluster(pDirEntry, pFileObj->FirstCluster);
    TimeDate = FS_X_GetTimeDate();
    FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_TIME], (U16)(TimeDate & 0xffff));
    FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_DATE], (U16)(TimeDate >> 16));
    FS__SB_Flush(pSB);                   /* Write the modified directory entry */
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_UpdateDirEntry: Could not read directory sector to update directory entry"));
  }
}


/*********************************************************************
*
*       FS_FAT_CleanWriteBuffer
*
*/
#if FS_USE_FILE_BUFFER
void FS_FAT_CleanWriteBuffer(FS_FILE * pFile, const U8 * pData, I32 NumBytesData, FS_SB * pSBData) {
  FS_FILE_OBJ * pFileObj;
  FS_FAT_INFO * pFATInfo;
  FS_FILE_BUFFER * pFileBuffer;
  int  SectorOff;

  pFileObj    = pFile->pFileObj;
  pFileBuffer = pFileObj->pBuffer;
  if (pFileBuffer == NULL) {
    return;
  }
  if (pFileBuffer->IsDirty == 0) {
    return;
  }
  pFATInfo    = &pFileObj->pVolume->FSInfo.FATInfo;

  FS__SB_SetSector(pSBData, pFileBuffer->SectorNo, FS_SB_TYPE_DATA);
  SectorOff = pFileBuffer->FilePos & (pFATInfo->BytesPerSec - 1);
  if (SectorOff || (pFile->FilePos != pFileObj->Size)) {                             // No need to read if we are at the end of file and start of sector
    if ((NumBytesData + pFileBuffer->NumBytesInBuffer) != pFATInfo->BytesPerSec) {     // No need to read if sector is written completely
      if (FS__SB_Read(pSBData)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_WriteData: Read error during write.\n"));
        pFile->Error = FS_ERR_READERROR;       /* read error during write */
        return;
      }
    }
  }
  //
  // Merge the written data into the sector
  //
  if (pFileBuffer->NumBytesInBuffer) {
    FS_MEMCPY(pSBData->pBuffer + SectorOff, pFileBuffer->pData, pFileBuffer->NumBytesInBuffer);
  }
  if (NumBytesData) {
    FS_MEMCPY(pSBData->pBuffer + SectorOff + pFileBuffer->NumBytesInBuffer, pData, NumBytesData);
  }
  pFileBuffer->IsDirty           = 0;
  pFileBuffer->NumBytesInBuffer  = 0;
  //
  // Write sector
  //
  if (FS__SB_Write(pSBData)) {
    pFile->Error = FS_ERR_WRITEERROR;
  }
}
#endif

/*********************************************************************
*
*       FS_FAT_GotoClusterAllocIfReq
*
*  Purpose
*    Allocates clusters to the file.
*
*  Return value
*    0     if cluster has been located
*    1     error
*/
int FS_FAT_GotoClusterAllocIfReq(FS_FILE *pFile, FS_SB * pSBfat) {
  FS_FILE_OBJ * pFileObj;
  FS_VOLUME   * pVolume;
  U32           NumClustersToGo;
  int           r;

  r               = 0;
  pFileObj        = pFile->pFileObj;
  pVolume         = pFileObj->pVolume;
  NumClustersToGo = FS_FAT_GotoCluster(pFile, pSBfat);
  if (NumClustersToGo > 0) {
    //
    //
    // Make sure at least one cluster is allocated, so that FirstCluster is valid.
    // If no cluster has yet been allocated, allocate one
    //
    if (pFileObj->FirstCluster == 0) {
      U32 CurClusterId;    /* FAT Id of the current cluster */
      CurClusterId    = FS_FAT_FindFreeCluster(pVolume, pSBfat, 0, pFile);
      if (CurClusterId == 0) {
        pFile->Error = FS_ERR_DISKFULL;
        return 1;                // No free cluster
      }
      pFileObj->FirstCluster = CurClusterId;
      NumClustersToGo--;
      pFileObj->Data.Fat.CurClusterAbs  = CurClusterId;
      pFileObj->Data.Fat.CurClusterFile = 0;
      if (FS_Global.WriteMode != FS_WRITEMODE_FAST) {
        if (FS_FAT_MarkClusterEOC(pVolume, pSBfat, pFileObj->Data.Fat.CurClusterAbs)) {
          FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_GotoClusterAllocIfReq: Could not write FAT entry.\n"));
          return 1;                // Error
        }
      }
    }
    if (NumClustersToGo) {
      do {
        U32 NewCluster;
        //
        // Check if we have an other cluster in the chain or if we need to alloc an other one
        //
        NewCluster = FS_FAT_FindFreeCluster(pVolume, pSBfat, pFileObj->Data.Fat.CurClusterAbs, pFile);
        if (NewCluster == 0) {
          pFile->Error = FS_ERR_DISKFULL;
          r =  1;                                /* Error, disk full */
          break;
        }
        if (FS_Global.WriteMode != FS_WRITEMODE_FAST) {
          FS_FAT_LinkCluster(pVolume, pSBfat, pFileObj->Data.Fat.CurClusterAbs, NewCluster);
        }
        pFileObj->Data.Fat.CurClusterAbs = NewCluster;
        pFileObj->Data.Fat.CurClusterFile++;
      } while (--NumClustersToGo);
      //
      // Mark the last allocated cluster as the last in the chain.
      //
      if (FS_Global.WriteMode != FS_WRITEMODE_FAST) {
        if (FS_FAT_MarkClusterEOC(pVolume, pSBfat, pFileObj->Data.Fat.CurClusterAbs)) {
          FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_GotoClusterAllocIfReq: Could not write FAT entry.\n"));
          return 1;                // Error
        }
      }
    }
  }
  return r;               /* O.K. */
}


/*********************************************************************
*
*       FS_FAT_Write
*
*  Description:
*    FS internal function. Write data to a file.
*
*  Parameters:
*    pData           - Pointer to data, which will be written to the file.
*    NumBytes        - Size of an element to be transferred to a file.
*    pFile           - Pointer to a FS_FILE data structure.
*
*  Note:
*    pFile is not checked if it is valid
*
*  Return value:
*    Number of bytes written.
*/
U32 FS_FAT_Write(FS_FILE *pFile, const void *pData, U32 NumBytes) {
  U32        NumBytesWritten;
  FS_SB         sbData;          /* Sector buffer for Data */
  FS_SB         sbfat;           /* Sector buffer for FAT handling */
  FS_FILE_OBJ * pFileObj;
  FS_VOLUME   * pVolume;

  pFileObj = pFile->pFileObj;
  pVolume  = pFileObj->pVolume;
  //
  // Check if file status is O.K..
  // If not, return.
  //
  if ((pFile->Error != FS_ERR_EOF) && (pFile->Error != FS_ERR_OK)) {
    return 0;                 // Error
  }
  //
  // Allocate sector buffers.
  //
  FS__SB_Create(&sbfat,  &pVolume->Partition);
  FS__SB_Create(&sbData, &pVolume->Partition);
  //
  // Do the work in a static subroutine
  //
  NumBytesWritten = _WriteData((const U8 *)pData, NumBytes, pFile, &sbData, &sbfat);
  //
  // If less bytes have been written than intended
  //   - Set error code in file structure (unless already set)
  //   - Invalidate the Current cluster Id to make sure we read allocation list from start next time we read
  //
  if (NumBytesWritten != NumBytes) {
    if (pFile->Error == 0) {
      pFile->Error = FS_ERR_WRITEERROR;
    }
  }
  //
  // Cleanup
  //
  FS__SB_Delete(&sbfat);
  FS__SB_Delete(&sbData);
  return NumBytesWritten;
}

/*********************************************************************
*
*       FS_FAT_Close
*
*  Description:
*    FS internal function. Close a file referred by a file pointer.
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*
*  Return value:
*    None.
*/
void FS_FAT_Close(FS_FILE *pFile) {
  FS_FILE_OBJ * pFileObj;
  FS_VOLUME   * pVolume;

  pFileObj = pFile->pFileObj;
  /* Check if media is OK */
  pVolume  = pFileObj->pVolume;
  if (pFile->Error == 0) {
    //
    // Update directory if necessary
    //
    if (pFile->AccessFlags & FS_FILE_ACCESS_FLAGS_AW) {
      FS_SB     SB;          /* Sector buffer for Data */
      FS__SB_Create(&SB, &pVolume->Partition);
      FS_FAT_UpdateDirEntry(pFileObj, &SB);
      if (FS_Global.WriteMode == FS_WRITEMODE_FAST) {
        FS_FAT_SyncFAT(pVolume, &SB);
      }
#if FS_USE_FILE_BUFFER
      FS_FAT_CleanWriteBuffer(pFile, NULL, 0, &SB);
#endif
      FS__SB_Delete(&SB);
    }
  }
}

/*********************************************************************
*
*       FS_FAT_Clean
*
*  Description:
*    Cleans the fs of a volume. If any pending operations need to be done to
*    the FAT FS (eg. Updating the FSInfo on FAT32 media), this is done
*    in this function.
*
*  Parameters:
*    pVolume       - Pointer to a mounted volume.
*
*  Return value:
*    None.
*/
void FS_FAT_Clean(FS_VOLUME * pVolume) {
#if FS_FAT_USE_FSINFO_SECTOR
  FS_FAT_INFO * pFATInfo;
  pFATInfo = &pVolume->FSInfo.FATInfo;
  if (pFATInfo->FATType == FS_FAT_TYPE_FAT32 && pFATInfo->FSInfoSectorIsInvalidated) {
    FS_SB         SB;
    //
    // Update the FSInfo Sector on FAT32 medium.
    //
    FS__SB_Create(&SB, &pVolume->Partition);
    FS__SB_SetSector(&SB, pFATInfo->FSInfoSector, FS_SB_TYPE_DATA);
    if (FS__SB_Read(&SB) == 0) {
      U8 * pBuffer;

      pBuffer = SB.pBuffer;
      FS_StoreU32LE(&pBuffer[FSINFO_OFF_FREE_CLUSTERS],     pFATInfo->NumFreeClusters);
      FS_StoreU32LE(&pBuffer[FSINFO_OFF_NEXT_FREE_CLUSTER], pFATInfo->NextFreeCluster);
      FS__SB_MarkDirty(&SB);
    }
    FS__SB_Delete(&SB);
  }
#else
  FS_USE_PARA(pVolume);
#endif
}

/*************************** End of file ****************************/

