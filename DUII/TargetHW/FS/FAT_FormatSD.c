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
File        : FAT_FormatSD.c
Purpose     : Implementation of the SD FS Format spec. V2.00.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/


#include "FS_Int.h"
#include "FAT_Intern.h"

#if FS_SUPPORT_FAT
/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/
#define PART_OFF_PARTITION0         0x01BE     /* Offset of start of partition table   */
#define SIZEOF_PARTITIONENTRY       0x10       /* Size of one entry in partition table */

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U32 NumSectors;
  U16 SectorsPerCluster;
  U32 BoundaryUnit;
} SIZE_INFO;

typedef struct {
  U32 NumSectors;
  U8  NumHeads;
  U8  SectorsPerTrack;
} CHS_INFO;

typedef struct {
  U32 NumSectors4Part;
  U32 PartStartSector;
  U8  FSType;
  U8  StartHead;
  U8  StartSector;
  U16 StartCylinder;
  U8  EndHead;
  U8  EndSector;
  U16 EndCylinder;

} PART_INFO;

typedef struct {
  FS_DEV_INFO       DevInfo;
  FS_FORMAT_INFO_EX FormatInfoEx; 
  PART_INFO         PartInfo;
  const SIZE_INFO * pSizeInfo;
} VOLUME_FORMAT_INFO;

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
static const SIZE_INFO _aSizeInfo[] = {
  {0x0003FFFUL, 16,   16},  // Up to     8 MBytes
  {0x001FFFFUL, 32,   32},  // Up to    64 MBytes
  {0x007FFFFUL, 32,   64},  // Up to   256 MBytes
  {0x01FFFFFUL, 32,  128},  // Up to  1024 MBytes
  {0x03FFFFFUL, 64,  128},  // Up to  2048 MBytes
  {0x3FFFFFFUL, 64, 8192}   // Up to 32768 MBytes
};


static const CHS_INFO _aCHSInfo[] = {
  {0x0000FFFUL,   2,   16},  // Up to     2 MBytes
  {0x0007FFFUL,   2,   32},  // Up to    16 MBytes
  {0x000FFFFUL,   4,   32},  // Up to    32 MBytes
  {0x003FFFFUL,   8,   32},  // Up to   128 MBytes
  {0x007FFFFUL,  16,   32},  // Up to   256 MBytes
  {0x00FBFFFUL,  16,   63},  // Up to   504 MBytes
  {0x01F7FFFUL,  32,   63},  // Up to  1008 MBytes
  {0x03EFFFFUL,  64,   63},  // Up to  2016 MBytes
  {0x07DFFFFUL, 128,   63},  // Up to  4032 MBytes
  {0x07DFFFFUL, 255,   63},  // Up to 32768 MBytes
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/



static int _CreatePartition(FS_VOLUME * pVolume, int PartIndex, PART_INFO * pPartInfo) {
  U8 * pBuffer;
  U8 * pPart;
  FS_DEVICE * pDevice;
  int r;
  unsigned Off;

  pBuffer = FS__AllocSectorBuffer();

  r = -1;
  if (pBuffer == (U8 *)NULL) {
    return -1;
  }
  pDevice = &pVolume->Partition.Device;
  FS_MEMSET(pBuffer, 0x00, FS_Global.MaxSectorSize);
  Off   = PART_OFF_PARTITION0 + (PartIndex * SIZEOF_PARTITIONENTRY);
  pPart = pBuffer + Off;
  *pPart++ = 0x00;                                       // Boot Indicator
  *pPart++ = pPartInfo->StartHead;                       // Starting head
  *pPart++ =  (pPartInfo->StartSector   & 0x3f)          // Starting sector
           | ((pPartInfo->StartCylinder & 0x300) >> 2);  // Higher bits of Starting cylinder
  *pPart++ = pPartInfo->StartCylinder & 0xff;            // Lower bits of starting cylinder
  *pPart++ = pPartInfo->FSType;                          // File system id
  *pPart++ = pPartInfo->EndHead;                         // Ending head
  *pPart++ =  (pPartInfo->EndSector   & 0x3f)            // Ending sector 
           | ((pPartInfo->EndCylinder & 0x300) >> 2);    // Higher bits of ending cylinder
  *pPart++ = pPartInfo->EndCylinder & 0xff;              // Lower  bits of ending cylinder
  FS_StoreU32LE(pPart, pPartInfo->PartStartSector);       // Relative sector
  pPart += 4;
  FS_StoreU32LE(pPart, pPartInfo->NumSectors4Part);       // Total sectors
  FS_StoreU16LE(pBuffer + 0x1fe, 0xAA55);                 // Boot signature
  if (FS_LB_WriteDevice(pDevice, 0, pBuffer, FS_SECTOR_TYPE_DATA) == 0) {
    r = 0;
  }
  FS__FreeSectorBuffer(pBuffer);
  return r;
}

/*********************************************************************
*
*       _CalcCHSInfo
*/
static void _CalcCHSInfo(PART_INFO * pPartInfo, FS_DEV_INFO * pDevInfo) {
  unsigned          i;
  const CHS_INFO  * pCHSInfo;
  U32               PartFirstSector;
  U32               PartLastSector;
  U32		            Data;
  
  //
  //  Get CHS info
  //
  for (i = 0; i <  COUNTOF(_aCHSInfo); i++) {
    pCHSInfo = &_aCHSInfo[i];
    if (pCHSInfo->NumSectors > pDevInfo->NumSectors) {
      break;
    }
  }
  PartFirstSector            = pPartInfo->PartStartSector;
  PartLastSector             = pPartInfo->PartStartSector + pPartInfo->NumSectors4Part - 1;
  Data                       = PartFirstSector % (pCHSInfo->NumHeads * pCHSInfo->SectorsPerTrack);
  Data                      /= pCHSInfo->SectorsPerTrack;
  pPartInfo->StartHead       = (U8)Data;

  Data                       = (PartFirstSector % pCHSInfo->SectorsPerTrack) + 1;
  pPartInfo->StartSector     = (U8)Data;

  Data                       = PartFirstSector / (pCHSInfo->NumHeads * pCHSInfo->SectorsPerTrack);
  pPartInfo->StartCylinder   = (U16)Data;

  Data                       = PartLastSector % (pCHSInfo->NumHeads * pCHSInfo->SectorsPerTrack);
  Data                      /= pCHSInfo->SectorsPerTrack;
  pPartInfo->EndHead         = (U8)Data;

  Data                       = (PartLastSector % pCHSInfo->SectorsPerTrack) + 1;
  pPartInfo->EndSector       = (U8)Data;

  Data                       = PartLastSector / (pCHSInfo->NumHeads * pCHSInfo->SectorsPerTrack);
  pPartInfo->EndCylinder     = (U16)Data;

  pDevInfo->SectorsPerTrack  = pCHSInfo->SectorsPerTrack;
  pDevInfo->NumHeads         = pCHSInfo->NumHeads;

}

/*********************************************************************
*
*       _CalcSizeInfo
*/
static int _CalcFormatInfo(VOLUME_FORMAT_INFO * pVolumeFormatInfo) {
  unsigned          i;
  U32               NumClusters;
  U32               NumSectorsPerFAT;
  U32               NumSectors4SystemArea;
  U32               PartStartSector;
  U32               ReservedSectorCount;
  char              FATType;
  U8                FSType;
  const SIZE_INFO * pSizeInfo;
  FS_DEV_INFO     * pDevInfo;
  PART_INFO       * pPartInfo;
  
  pDevInfo  = &pVolumeFormatInfo->DevInfo;
  pPartInfo = &pVolumeFormatInfo->PartInfo;
  for (i = 0; i <  COUNTOF(_aSizeInfo); i++) {
    pSizeInfo = &_aSizeInfo[i];
    if (pSizeInfo->NumSectors > pDevInfo->NumSectors) {
      break;
    }
  }
  pVolumeFormatInfo->pSizeInfo = pSizeInfo;
  NumClusters                  = pDevInfo->NumSectors / pSizeInfo->SectorsPerCluster;
  FATType                      = FS_FAT_GetFATType(NumClusters);
  NumSectorsPerFAT             = FS__DivideU32Up(NumClusters * FATType, pDevInfo->BytesPerSector * 8);
  //
  //  Calc partition start sector
  //
  if (FATType != FS_FAT_TYPE_FAT32) {
    NumSectors4SystemArea        = 2 * NumSectorsPerFAT + 33;  // 33 =  1 Sector for boot parameter block +
                                                               //      32 sectors for root directory [512 entries]
    //
    //  Find the correct multiplicator
    //
    i = 1;
    do {
      if (i * pSizeInfo->BoundaryUnit > NumSectors4SystemArea) {
        break;
      }
      i++;
    } while(1);
    PartStartSector = i * pSizeInfo->BoundaryUnit - NumSectors4SystemArea;
    if (PartStartSector & (pSizeInfo->BoundaryUnit -1)) {
      PartStartSector += pSizeInfo->BoundaryUnit;
    }
  } else {    
    PartStartSector     = pSizeInfo->BoundaryUnit;
    ReservedSectorCount = 2 * NumSectorsPerFAT + 1;
    //
    //  Find the correct multiplicator
    //
    i = 1;
    do {
      if (i * pSizeInfo->BoundaryUnit > ReservedSectorCount) {
        break;
      }
      i++;
    } while(1);
    if (ReservedSectorCount < 9) {
      ReservedSectorCount += pSizeInfo->BoundaryUnit;
    }
    //
    // Values need to be recalculated
    //    
    NumClusters                  = (pDevInfo->NumSectors - PartStartSector - (i * pSizeInfo->BoundaryUnit)) / pSizeInfo->SectorsPerCluster;
    NumSectorsPerFAT             = FS__DivideU32Up(NumClusters * FATType, pDevInfo->BytesPerSector * 8);
    ReservedSectorCount          = (i * pSizeInfo->BoundaryUnit) - 2 * NumSectorsPerFAT;
    pVolumeFormatInfo->FormatInfoEx.NumReservedSectors = (U16)ReservedSectorCount;

  }

  
  //
  //  Store info into PartInfo
  //
  pPartInfo->NumSectors4Part = pDevInfo->NumSectors - PartStartSector;
  pPartInfo->PartStartSector = PartStartSector;
  if (pPartInfo->NumSectors4Part < 0x7FA8) {
    FSType = 0x01;
  } else if (pPartInfo->NumSectors4Part < 0x010000UL) {
    FSType = 0x04;
  } else if (pPartInfo->NumSectors4Part < 0x400000UL) {
    FSType = 0x06;
  } else if (pPartInfo->NumSectors4Part < 0xFB0400UL) {
    FSType = 0x0B;
  } else {
    FSType = 0x0C;
  }    
  pPartInfo->FSType = FSType;
  _CalcCHSInfo(pPartInfo, pDevInfo);
  //
  // Create partition on device
  //
  return 0;
}


/*********************************************************************
*
*       _CalcSizeInfo
*/
static void _InitFormatInfo(VOLUME_FORMAT_INFO * pVolFormatInfo) {
  FS_MEMSET(&pVolFormatInfo->DevInfo,      0, sizeof(FS_DEV_INFO));
  FS_MEMSET(&pVolFormatInfo->FormatInfoEx, 0, sizeof(FS_FORMAT_INFO_EX));
  FS_MEMSET(&pVolFormatInfo->PartInfo,     0, sizeof(PART_INFO));
  pVolFormatInfo->pSizeInfo = NULL;
}


/*********************************************************************
*
*       Public code, internal
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__SD_Format
*
*  Function description:
*    Internal version of FS_Format.
*    Format the medium as specified in FS specification.
*
*  Parameters:
*    pVolume       Volume to format. NULL is permitted, but returns an error.
*  
*  Return value:
*    ==0         - File system has been started.
*    !=0         - An error has occurred.
*/
int FS__SD_Format(FS_VOLUME  * pVolume) {
  int                 r;
  int                 Status;
  FS_DEVICE         * pDevice;
  VOLUME_FORMAT_INFO  VolFormatInfo;

  r = -1;
  if (pVolume) {
    _InitFormatInfo(&VolFormatInfo);
    pDevice = &pVolume->Partition.Device;
    Status = FS_LB_GetStatus(pDevice);
    if (Status >= 0) {
      FS_LB_InitMediumIfRequired(pDevice);
      //
      //  Retrieve the information from card
      //
      if (FS_LB_GetDeviceInfo(pDevice, &VolFormatInfo.DevInfo) < 0) {
        return -1;  // Error, device information can not be retrieved.
      }
      if (_CalcFormatInfo(&VolFormatInfo)) {
        return -1;
      }
      r = _CreatePartition(pVolume, 0, &VolFormatInfo.PartInfo);
      if (r == 0) {
        FS_FORMAT_INFO_EX * pFormatInfoEx;

        pFormatInfoEx                     = &VolFormatInfo.FormatInfoEx;
        pFormatInfoEx->NumRootDirEntries  = 512;
        pFormatInfoEx->pDevInfo           = &VolFormatInfo.DevInfo;
        pFormatInfoEx->SectorsPerCluster  = VolFormatInfo.pSizeInfo->SectorsPerCluster;
        r = FS_FAT_FormatEx(pVolume, pFormatInfoEx);
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FormatSD
*
*  Function description:
*    Format the medium according to specification published by the SD-Card
*    organisation. This includes the following steps:
*    - Writes partion entry into the MBR
*    - Formats the partition
*
*  Parameters:
*    pDevice       Device specifier (string). "" refers to the first device.
*  
*  Return value:
*    ==0         - O.K., format successful
*    !=0         - An error has occurred.
*/
int FS_FormatSD(const char *sVolumeName) {
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolumeName, NULL);
  r = FS__SD_Format(pVolume);
  FS_UNLOCK();
  return r;
}

#endif

/*************************** End of file ****************************/
