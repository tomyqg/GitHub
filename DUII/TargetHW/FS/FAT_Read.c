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
File        : FAT_Read.c
Purpose     : FAT read routines
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
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
  U32  FirstSector;
  U32  NumSectors;
  FS_SB * pSBData;
  void  * pData;
} BURST_INFO_R;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ReadBurst
*
*/
static int _ReadBurst(BURST_INFO_R * pBurstInfo) {
  if (pBurstInfo->NumSectors) {
    if (FS_LB_ReadBurstPart(pBurstInfo->pSBData->pPart,
                            pBurstInfo->FirstSector,
                            pBurstInfo->NumSectors,
                            pBurstInfo->pData, FS_SECTOR_TYPE_DATA))
    {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_ReadBurst (FAT): Burst read error.\n"));
      return 1;     // read error
    }
  }
  return 0;         // No problem !
}

/*********************************************************************
*
*       _ReadData
*
*  Function description
*    Read data from a file.
*    Does most of the actual work and contains optimizations such as file buffer & burst support.
*
*  Return value:
*    Number of bytes read
*/
static U32 _ReadData(U8 *pData, U32 NumBytesReq, FS_FILE * pFile, FS_SB * pSBData, FS_SB * pSBfat) {
  U32           NumBytesRead;
  U32           BytesPerCluster;
  FS_VOLUME *   pVolume;
  U32           NumBytesCluster;
  U32           FirstByteAfterCluster;
  int           SectorOff;
  U32           SectorNo;
  FS_FILE_OBJ * pFileObj;
  FS_FAT_INFO * pFATInfo;
  char          ZeroCopyAllowed;
  BURST_INFO_R  BurstInfo;
#if FS_USE_FILE_BUFFER
  FS_FILE_BUFFER * pFileBuffer;
#endif

  //
  // Init / Compute some values used thruout the routine
  //
  pFileObj        = pFile->pFileObj;
  pVolume         = pFileObj->pVolume;
  pFATInfo        = &pVolume->FSInfo.FATInfo;
  BytesPerCluster = pFATInfo->BytesPerCluster;
  NumBytesRead    = 0;
  BurstInfo.NumSectors  = 0;
  BurstInfo.FirstSector = 0xFFFFFFFFUL;
  BurstInfo.pSBData     = pSBData;
  //
  // Check if "Zero copy" is possible.
  // Per default, it is, but some systems may not allow the driver in some situations to read data directly into the application buffer.
  // Possible reasons can be misaligned destination (DMA requires 4-byte alignment, but application buffer is not) or caching issues.
  // On most systems, this does not need to be considered since it is not an issue; ideally this is taken care of by the driver anyhow;
  // meaning that if copying is required, it is already done by the driver itself
  //
  ZeroCopyAllowed = 1;
#if FS_SUPPORT_CHECK_MEMORY
  {
    FS_MEMORY_IS_ACCESSIBLE_CALLBACK * pfMemoryIsAccessible;

    pfMemoryIsAccessible = pFileObj->pVolume->Partition.Device.Data.pfMemoryIsAccessible;
    if (pfMemoryIsAccessible) {
      if (pfMemoryIsAccessible(pData, NumBytesReq) == 0) {
        ZeroCopyAllowed = 0;
      }
    }
  }
#endif
  //
  // Check if we can serve application  from file buffer. If so, copy from file buffer and return.
  //
#if FS_USE_FILE_BUFFER
  {
    I32 Off;
    I32 NumBytesAvail;

    pFileBuffer = pFileObj->pBuffer;
    if (pFileBuffer) {
      if (pFileBuffer->IsDirty) {  // Dirty will only be set if file buffer is also used for write operations
        FS_FAT_CleanWriteBuffer(pFile, NULL, 0, pSBData);
      }
      Off = pFile->FilePos - pFileBuffer->FilePos;
      //
      // Check if data for the current file position is present in our buffer and if the file buffer can serve data for the whole read.
      //
      if (Off >= 0) {
        NumBytesAvail = pFileBuffer->NumBytesInBuffer - Off;
        if (NumBytesAvail > 0) {
          I32 NumBytesToCopy;

          NumBytesToCopy = MIN(NumBytesAvail, (I32)NumBytesReq);
          FS_MEMCPY(pData, pFileBuffer->pData + Off, NumBytesToCopy);
          pFile->FilePos   += NumBytesToCopy;
          NumBytesReq -= NumBytesToCopy;
          if (NumBytesReq == 0) {
            return NumBytesToCopy;
          }
          pData        += NumBytesToCopy;
          NumBytesRead += NumBytesToCopy;
        }
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
    if (FS_FAT_GotoCluster(pFile, pSBfat)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_ReadData: Too few cluster allocated to file.\n"));
      return NumBytesRead;           /* File truncated (to few clusters) */
    }

    FirstByteAfterCluster = BytesPerCluster * (pFileObj->Data.Fat.CurClusterFile + 1);
    NumBytesCluster   = FirstByteAfterCluster - pFile->FilePos;
    if (NumBytesCluster > NumBytesReq) {
      NumBytesCluster = NumBytesReq;
    }
    SectorOff = pFile->FilePos & (pFATInfo->BytesPerSec - 1);
    SectorNo  = FS_FAT_ClusterId2SectorNo(pFATInfo, pFileObj->Data.Fat.CurClusterAbs);
    SectorNo += (pFile->FilePos >> pFATInfo->ldBytesPerSector) & (pFATInfo->SecPerClus -1);
    //
    // Read data from the cluster, iterating over sectors
    // 
    do {
      unsigned NumBytesSector;
      NumBytesSector = pFATInfo->BytesPerSec - SectorOff;
      if ((U32)NumBytesSector > NumBytesCluster) {
        NumBytesSector = (unsigned)NumBytesCluster;
      }
      //
      // Do we have to read one sector into intermediate buffer ?
      //
      if   ((ZeroCopyAllowed == 0)
#if FS_DRIVER_ALIGNMENT > 1      // Not required, just to avoid warnings
        || (((U32)pData & (FS_DRIVER_ALIGNMENT - 1)))
#endif
        || (NumBytesSector != pFATInfo->BytesPerSec))    // Do we read the sector only partially ?
      {
        //
        // Safe, but slow: Read one sector using memory of a smart buffer and copy data to destination
        //
        FS__SB_SetSector(pSBData, SectorNo, FS_SB_TYPE_DATA);
        if (FS__SB_Read(pSBData)) {
          return NumBytesRead;
        }
        FS_MEMCPY(pData, pSBData->pBuffer + SectorOff, NumBytesSector);
#if FS_USE_FILE_BUFFER
        //
        // Copy data which has been read from storage, but not yet requested by application to file buffer
        //
        pFileBuffer = pFileObj->pBuffer;
        if (pFileBuffer) {
          int NumBytesToCopy;

          NumBytesToCopy = pFATInfo->BytesPerSec - NumBytesSector - SectorOff;
          NumBytesToCopy = MIN(NumBytesToCopy, (int)pFileBuffer->BufferSize);
          if (NumBytesToCopy) {    // Do we read the sector only partially ?
            FS_MEMCPY(pFileBuffer->pData, pSBData->pBuffer + SectorOff + NumBytesSector, NumBytesToCopy);
            pFileBuffer->FilePos          = pFile->FilePos + NumBytesSector;
            pFileBuffer->NumBytesInBuffer = NumBytesToCopy;
          }
        }
#endif
      } else {
        //
        // Zero copy variant. Check if we need to read the previous burst data
        //
        if (SectorNo != BurstInfo.FirstSector + BurstInfo.NumSectors) {
          if (_ReadBurst(&BurstInfo)) {
            FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_ReadData: Burst read error.\n"));
            return 0;               // We do not know how many bytes have been read o.k., so reporting 0 is on the safe side
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
      pData           += NumBytesSector;
      NumBytesCluster -= NumBytesSector;
      NumBytesReq     -= NumBytesSector;
      NumBytesRead    += NumBytesSector;
      pFile->FilePos  += NumBytesSector;
      SectorNo++;
      SectorOff = 0;                // Next sector will be written from start
    } while (NumBytesCluster);
  } while (NumBytesReq);
  if (_ReadBurst(&BurstInfo)) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_ReadData: Burst read error.\n"));
    NumBytesRead = 0;               // We do not know how many bytes have been read o.k., so reporting 0 is on the safe side
  }
  return NumBytesRead;
}


/*********************************************************************
*
*       FS_FAT_Read
*
*  Function description
*    FS internal function. Read data from a file.
*
*  Return value:
*    Number of elements read.
*/
U32 FS_FAT_Read(FS_FILE *pFile, void * pData, U32 NumBytesReq) {
  U32           NumBytesRead;
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
  if (pFile->Error) {
    return 0;                 // Error
  }

  if (pFile->FilePos >= pFileObj->Size) {
    pFile->Error = FS_ERR_EOF;
    return 0;
    
  }

  //
  // Make sure we do not try to read beyond the end of the file
  // 
  {
    U32 NumBytesAvail;
    NumBytesAvail = pFileObj->Size - pFile->FilePos;
    if (NumBytesReq > NumBytesAvail) {
      NumBytesReq = NumBytesAvail;
    }
  }
  if (NumBytesReq == 0) {
    pFile->Error = FS_ERR_EOF;
    return 0;
  }

  if (pFileObj->FirstCluster == 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_Read: Can not read: No cluster in directory entry.\n"));
    return 0;
  }
  //
  // Allocate sector buffers.
  // 

  FS__SB_Create(&sbfat,  &pVolume->Partition);
  FS__SB_Create(&sbData, &pVolume->Partition);
  //
  // Do the work in a static subroutine
  // 
  NumBytesRead = _ReadData((U8 *)pData, NumBytesReq, pFile, &sbData, &sbfat);
  //
  // If less bytes have been read than intended
  //   - Set error code in file structure (unless already set)
  //   - Invalidate the Current cluster Id to make sure we read allocation list from start next time we read
  // 
  if (NumBytesRead != NumBytesReq) {
    if (pFile->Error == 0) {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_Read: General read error.\n"));
      pFile->Error = FS_ERR_READERROR;
    }
  }
  //
  // Cleanup
  // 
  FS__SB_Delete(&sbfat);
  FS__SB_Delete(&sbData);
  return NumBytesRead;
}

/*************************** End of file ****************************/
