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
File        : FAT_Format.c
Purpose     : Implementation format routines
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FAT.h"
#include "FAT_Intern.h"

/*********************************************************************
*
*       #define constants
*
**********************************************************************
*/
#define FAT_SIGNATURE   0xAA55

#define PART_OFF_PARTITION0         0x01BE     /* Offset of start of partition table   */
#define PARTENTRY_OFF_TYPE          0x04

#define PART_TYPE_FAT12             0x01
#define PART_TYPE_FAT16             0x04
#define PART_TYPE_FAT16_HUGE        0x06
#define PART_TYPE_FAT32             0x0B

#define NUM_DEFAULT_DIR_ENTRIES     0x100

/*********************************************************************
*
*       Local data types
*
**********************************************************************
*/
typedef struct {
  I32  MediaID;
  U32  NumSectors;
  U16  SectorsPerTrack;
  U16  NumHeads;
  U8   Media; 
} FORMAT_MEDIA_TYPE;


typedef struct {
  U32 SecNum;
  U8  SectorsPerCluster;
  U16 NumRootDirEntries;
} FORMAT_INFO;

typedef struct {
  U8  FATType;
  U32 MinClusters;
} FAT_TYPE_INFO;

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/

static const FAT_TYPE_INFO _aFATTypeInfo[] = {
    {  FS_FAT_TYPE_FAT12, 0x00000000},
    {  FS_FAT_TYPE_FAT16, 0x00000FF5}
#if (FS_FAT_SUPPORT_FAT32)
    ,{ FS_FAT_TYPE_FAT32, 0x0000FFF5 }
#endif
  };

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/* Default volume label. Format will use this string as default label */
const char _acVolumeLabel[] = "NO NAME    "; /* 11 characters */

/*********************************************************************
*
*       _aFormatInfo
*
* Purpose
*   Table with format info
*
* Notes
*   (1) Why this table ?
*       It is not necessary to put information on how to format media of
*       a certain size into a table, but it sure is a lot easier and
*       also a lot more compact in terms of code size than to put this
*       into the source code and also proves to be the most flexible
*       method
*   (2) How is the table searched ?
*       Simple ... The first entry, number of sectors is compared.
*       The first entry with less more or equal sectors as available
*       on the medium (partition) is used
*   (3) Does this table work the same way on any medium ?
*       Yes. It is used by all format code for any medium.
*/
static const FORMAT_INFO _aFormatInfo[] = {
/*    NumSectors, SectorsPerCluster, NumRootDirEntries */
  {        256UL, 0x0001,            0x0020},    /* <= 128kB */
  {        512UL, 0x0001,            0x0040},    /* <= 256kB */
  {   0x000800UL, 0x0001,            0x0080},    /* <= 1  MB */
  {   0x001000UL, 0x0001,            0x0100},    /* <= 2  MB */
  {   0x004000UL, 0x0002,            0x0100},    /* <=  16 MB */
  {   0x008000UL, 0x0002,            0x0100},    /* <=  32 MB */
  {   0x040000UL, 0x0004,            0x0200},    /* <= 128 MB */
  {   0x080000UL, 0x0008,            0x0200},    /* <= 256 MB */
  {   0x100000UL, 0x0010,            0x0200},    /* <= 512 MB */
#if (FS_FAT_SUPPORT_FAT32)
  /* media bigger than 512MB are formatted with FAT32 */
  {  0x1000000UL, 0x0008,            0 },   /* <=  8GB */
  {   33554432UL, 0x0010,            0 },   /* <= 16GB */
  {   67108864UL, 0x0020,            0 },   /* <= 32GB */
  { 0xFFFFFFFFUL, 0x0040,            0 }
#else
  {   0x1FFEA0UL, 0x0020,            0x0200},   /* <= 1GB */
  {   0x3FFD40UL, 0x0040,            0x0200},   /* <= 2GB */
/*  {   0x7FFA80, 0x0080,            0x0200},   <= 4GB */
  {   0xFFF500UL, 0x0080,            0x0200},   /* <= 8GB */
#endif
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _UpdatePartTable
*
*  Function description
*    Updates the partition table. This is necessary because different FAT types
*    have different Ids in the partition table.
*    In other words: If a medium was FAT32 and is now formatted as FAT16 (or the other way round),
*    the partion type in the partition table needs to be changed.
*/
static void _UpdatePartTable(FS_VOLUME * pVolume, U32 NumSectors, U8 FATType, U8 * pBuffer) {
  int   Off;
  U8 CurrentPartType;
  U8 PartType;

  PartType = PART_TYPE_FAT32;
  if (FS_LB_ReadDevice(&pVolume->Partition.Device, 0, pBuffer, FS_SECTOR_TYPE_DATA)) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_UpdatePartTable: Could not read sector 0 to update partition 0.\n"));
    return;
  }
  Off  = PART_OFF_PARTITION0 + PARTENTRY_OFF_TYPE;
  CurrentPartType = *(pBuffer + Off);
  /* */
  switch (FATType) {
  case FS_FAT_TYPE_FAT12:
    PartType = PART_TYPE_FAT12;
    break;
  case FS_FAT_TYPE_FAT16:
    if (NumSectors < 65536UL) {
      PartType = PART_TYPE_FAT16;
    } else {
      PartType = PART_TYPE_FAT16_HUGE;
    }
    break;
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL
  case FS_FAT_TYPE_FAT32:
    break;
  default:
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_UpdatePartTable: Unknown FAT Type.\n"));
    return;
#endif
  }
  if (CurrentPartType != PartType) {
    *(pBuffer + Off) = PartType;
    if (FS_LB_WriteDevice(&pVolume->Partition.Device, 0, pBuffer, FS_SECTOR_TYPE_DATA)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_UpdatePartTable: Could not write sector 0 to update partition 0.\n"));
    }
  }
}



/*********************************************************************
*
*       _FatFormatEx
*
*  Description:
*    Format a media using specified parameters.
*
*  Return value:
*    >=0         - Media has been formatted.
*    <0          - An error has occurred.
*/
static int _FatFormatEx(FS_VOLUME * pVolume,
                        U32 NumSectors,
                        U8  Media,
                        U16 SecPerTrk,
                        U16 NumHeads,
                        U8  SectorsPerCluster,
                        U16 NumRootDirEntriesProp,   /* Proposed, actual value depends on FATType */
                        U16 BytesPerSector,
                        U16 NumReservedSectors,
                        U8  UpdatePartition)
{
  int                  i;
  int                  j;
  int                  n;
  I32               NumClusters;
  U32               FATSize;
  U8                FATType;
  U32               RootDirSectors;
  I32               NumDataSectors;
  U16               NumRootDirEntries;
  U16               NumReservedSectorsTemp;
  FS_PARTITION       * pPart;
  FS_DEVICE          * pDevice;  
  U8              * pBuffer;

  pPart              = &pVolume->Partition;
  pDevice            = &pPart->Device;
  FATType            = 0;
  NumRootDirEntries  = 0;
  FATSize            = 0;
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA
  NumRootDirEntriesProp &= 0xFFF0;   /* Make sure it is a multiple of 16 */
#endif
  /*
   * Is device ready ?
   */
  i = FS_LB_GetStatus(pDevice);
  if (i < 0) {
    return i;
  }
  /*
   * Unmount the volume. Note that all handles should have been closed !
   */
  pVolume->IsMounted = 0;
  pBuffer = FS__AllocSectorBuffer();
  if (pBuffer == 0) {
    return -1;
  }
  /*
   * Check if the a sector fits into the sector buffer
   */
  if ((BytesPerSector > FS_Global.MaxSectorSize) || (BytesPerSector == 0)) {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "_FatFormatEx: Invalid BytesPerSector value: %d\n.", BytesPerSector));
    return -1;
  }
  NumReservedSectorsTemp = NumReservedSectors;
  //
  // Set NumHeads and SectorPerTrack to default value, if necessary
  //
  if (NumHeads == 0) {
    NumHeads = 0xFF;
  }
  if (SecPerTrk == 0) {
    SecPerTrk = 0x3F;
  }
  /*
   * Calculate the number of data sectors, number of clusters.
   * We calculate preliminary values first (Values may be slightly too big),
   * Use these to calc FATSize and then compute the correct values in a second step.
   */

  for (i = COUNTOF(_aFATTypeInfo) - 1; i >= 0; i--) {
    I32 MinClusters;

    MinClusters        = _aFATTypeInfo[i].MinClusters;
    FATType            = _aFATTypeInfo[i].FATType;
    if ((NumReservedSectorsTemp != NumReservedSectors) || (NumReservedSectorsTemp == 0)) {
      NumReservedSectorsTemp = (FATType == FS_FAT_TYPE_FAT32) ? 32 : 1;
    }
    NumRootDirEntries  = (FATType == FS_FAT_TYPE_FAT32) ?  0 : NumRootDirEntriesProp;
    RootDirSectors  = FS__DivideU32Up(NumRootDirEntries * 32, BytesPerSector);
    NumDataSectors  = (NumSectors - NumReservedSectorsTemp - RootDirSectors);
    NumClusters     = NumDataSectors / SectorsPerCluster;
    FATSize         = FS__DivideU32Up(NumClusters * FATType, 8 * BytesPerSector /* Bits per sector */);
    NumDataSectors -= 2 * FATSize;                                           /* Compute precise number of avail. sectors */
    NumClusters     = NumDataSectors / SectorsPerCluster;                    /* Compute precise number of avail. clusters */    
    FATSize         = FS__DivideU32Up(NumClusters * FATType, 8 * BytesPerSector /* Bits per sector */);
    /* Now check if the max. number of clusters is exceeded ... */
    if (i == 0) {
      if (NumClusters <= 4084) {
        break;
      }
      NumClusters = 4084;
    } else {
      if (NumClusters >= MinClusters) {
        break;                                                     /* O.K., this FATType can be used */
      }
    }
  }
  NumReservedSectors = NumReservedSectorsTemp;
  /*
   * Now that the type of FAT has been determined, we can perform add. checks.
   */
  if (NumRootDirEntriesProp == 0) {
    if (FATType != FS_FAT_TYPE_FAT32) {
      FS_DEBUG_WARN((FS_MTYPE_FS, "_FatFormatEx: NumRootDirEntries (%d) is illegal. Defaulting to %d.\n", NumRootDirEntriesProp, NUM_DEFAULT_DIR_ENTRIES));
      NumRootDirEntriesProp = NUM_DEFAULT_DIR_ENTRIES;
    }
  }
  /* 
   *  Prepare buffer. Offset 0 - 35 is same for FAT12/FAT16 and FAT32 
   */
  /* MS specified most frequently used format: 0xEB 0x3C 0x90 */
  FS_MEMSET(pBuffer, 0x00, BytesPerSector);
  pBuffer[0]   = 0xE9; /* jmpBoot */
  FS_STRNCPY((char*)&pBuffer[3], "MSWIN4.1", 8);                /* OEMName = 'MSWIN4.1' */
  FS_StoreU16LE(&pBuffer[11], BytesPerSector);                   /* Sector size     */
  pBuffer[13]  = SectorsPerCluster;                             /* Sectors in each allocation unit */
  FS_StoreU16LE(&pBuffer[14], NumReservedSectors);
  pBuffer[16]  = 2;                                             /* NumFATs */
  FS_StoreU16LE(&pBuffer[17], NumRootDirEntries);

  /* Number of total sectors (512 byte units) of the media
   * This is independent of FAT type (FAT12/FAT16/FAT32)
   */
  if (NumSectors < 0x10000UL) {
    FS_StoreU16LE(&pBuffer[19], (U16)NumSectors);              /* Total Sectors16 */
  } else {
    FS_StoreU32LE(&pBuffer[32], NumSectors);                      /* Total Sectors32 */
  }
  pBuffer[21]  = Media;                                          /* Media */
  FS_StoreU16LE(&pBuffer[24], SecPerTrk);                         /* Sectors per Track */
  FS_StoreU16LE(&pBuffer[26], NumHeads);                          /* Number of Heads */
  FS_StoreU32LE(&pBuffer[28], pVolume->Partition.StartSector);    /* Hidden Sectors */
  /* Offset 36 and above have different meanings for FAT12/FAT16 and FAT32 */
  if (FATType != FS_FAT_TYPE_FAT32) {
    pBuffer[36]  = 0x80;                                         /* Physical drive number */
    pBuffer[38]  = 0x29;                                         /* Extended Boot Signature */
    FS_StoreU32LE(&pBuffer[39], 0x01234567UL);                      /* 32 Bit Volume ID */
    FS_StoreU16LE(&pBuffer[22], (U16)FATSize);                 /* FATSz16 */
    FS_STRNCPY((char*)&pBuffer[43], _acVolumeLabel, 11);         /* VolLab = ' ' */
    FS_STRNCPY((char*)&pBuffer[54], (FATType == FS_FAT_TYPE_FAT12) ? "FAT12   " : "FAT16   ", 8);
  }
#if (FS_FAT_SUPPORT_FAT32)
  else {
    /* FAT32 */
    pBuffer[64]  = 0x80;                                         /* Physical drive number */
    pBuffer[66]  = 0x29;                                         /* Extended Boot Signature */
    FS_StoreU32LE(&pBuffer[36], FATSize);                         /* FATSize32 */
    FS_StoreU32LE(&pBuffer[44], 2);                               /* RootClus */
    FS_StoreU16LE(&pBuffer[48], 1);                               /* FSInfo */
    pBuffer[50]  = 0x06;                                         /* BkBootSec = 0x0006; */
    FS_StoreU32LE(&pBuffer[67], 0x01234567UL);                      /* 32 Bit Volume ID */
    FS_STRNCPY((char*)&pBuffer[71], _acVolumeLabel, 11);         /* VolLab = ' ' */
    FS_STRNCPY((char*)&pBuffer[82], "FAT32   ", 8);              /* FilSysType = 'FAT32' */
  }
#endif /* FS_FAT_SUPPORT_FAT32 */
  FS_StoreU16LE(&pBuffer[510], FAT_SIGNATURE);                    /* Signature = 0xAA55 */
  /* Write BPB to media */
  i = FS_LB_WritePart(pPart, 0, (void*)pBuffer, FS_SECTOR_TYPE_DATA);
  if (i < 0) {
    FS__FreeSectorBuffer(pBuffer);
    return -1;
  }
  if (FATType == FS_FAT_TYPE_FAT32) {
    /* Write backup BPB */
    i = FS_LB_WritePart(pPart, 6, (void*)pBuffer, FS_SECTOR_TYPE_DATA);
    if (i < 0) {
      FS__FreeSectorBuffer(pBuffer);
      return -1;
    }
  }
  /*
   *  Init FAT 1 & 2. Start by filling all FAT sectors except the first one with 0
   */
  FS_MEMSET(pBuffer, 0x00, BytesPerSector);

  i = FS_LB_WriteMultiplePart(pPart, NumReservedSectors, 2 * FATSize, pBuffer, FS_SECTOR_TYPE_MAN);
  if (i < 0) {
    FS__FreeSectorBuffer(pBuffer);
    return -1;
  }
  /*
   * Init the first FAT sector
   */
  pBuffer[0] = (U8)Media;
  pBuffer[1] = (U8)0xFF;
  pBuffer[2] = (U8)0xFF;
  if (FATType != FS_FAT_TYPE_FAT12) {
    pBuffer[3] = (U8)0xFF;
  }
#if (FS_FAT_SUPPORT_FAT32)
  if (FATType == FS_FAT_TYPE_FAT32) {
    FS_StoreU32LE(&pBuffer[4],   0x0FFFFFFFUL);
    FS_StoreU32LE(&pBuffer[8],   0x0FFFFFFFUL);
  }
#endif /* FS_FAT_SUPPORT_FAT32 */
  for (i = 0; i < 2; i++) {
    j = FS_LB_WritePart(pPart, (U32)NumReservedSectors + i * FATSize, (void*)pBuffer, FS_SECTOR_TYPE_MAN);
    if (j < 0) {
      FS__FreeSectorBuffer(pBuffer);
      return -1;
    }
  }
  /* Init root directory area */
  FS_MEMSET(pBuffer, 0x00, BytesPerSector);
  if (NumRootDirEntries != 0) {
    /* FAT12/FAT16 */
    n = (((U32)NumRootDirEntries * 32) / (U32)BytesPerSector);
    j = FS_LB_WriteMultiplePart(pPart, NumReservedSectors + 2 * FATSize, n, pBuffer, FS_SECTOR_TYPE_DIR);
    if (j < 0) {
      FS__FreeSectorBuffer(pBuffer);
      return -1;
    }
  }
#if (FS_FAT_SUPPORT_FAT32)
  else {
    /* FAT32 */
    n = SectorsPerCluster;
    j = FS_LB_WriteMultiplePart(pPart, NumReservedSectors + 2 * FATSize, n, pBuffer, FS_SECTOR_TYPE_DIR);
    if (j < 0) {
      FS__FreeSectorBuffer(pBuffer);
      return -1;
    }
  }
#endif /* FS_FAT_SUPPORT_FAT32 */
#if (FS_FAT_SUPPORT_FAT32)
  if (FATType == FS_FAT_TYPE_FAT32) {
    /* Init FSInfo */
    FS_MEMSET(pBuffer, 0x00, BytesPerSector);
    FS_StoreU32LE(&pBuffer[FSINFO_OFF_SIGNATURE_1],       0x41615252UL);       /* LeadSig = 0x41615252 */
    FS_StoreU32LE(&pBuffer[FSINFO_OFF_SIGNATURE_2],       0x61417272UL);       /* StructSig = 0x61417272 */
    FS_StoreU32LE(&pBuffer[FSINFO_OFF_FREE_CLUSTERS],     NumClusters - 1);  /* Invalidate last known free cluster count */
    FS_StoreU32LE(&pBuffer[FSINFO_OFF_NEXT_FREE_CLUSTER], 0x00000003UL);       /* Give hint for free cluster search */
    FS_StoreU16LE(&pBuffer[510], FAT_SIGNATURE);                             /* Signature = 0xAA55 */
    i = FS_LB_WritePart(pPart, 1, (void*)pBuffer, FS_SECTOR_TYPE_DATA);
    if (i >= 0) {
      /* Write Backup of FSInfo sector */
      i  = FS_LB_WritePart(pPart, 7, (void*)pBuffer, FS_SECTOR_TYPE_DATA);
    }
    if (i < 0) {
      FS__FreeSectorBuffer(pBuffer);
      return -1;
    }
  }
#endif /* FS_FAT_SUPPORT_FAT32 */
  // 
  // Update partition table if necessary 
  //
  if (pVolume->Partition.StartSector) {
    if (UpdatePartition) {
      _UpdatePartTable(pVolume, NumSectors, FATType, pBuffer);
    }
  }
  FS__FreeSectorBuffer(pBuffer);
  return 0;
}

/*********************************************************************
*
*       _FatFormat
*
*  Description:
*    Format a media using specified parameters.
*
*  Return value:
*    >=0         - Media has been formatted.
*    <0          - An error has occurred.
*/
static int _FatFormat(FS_VOLUME * pVolume, I32 NumSectors, U8 Media, U16 SecPerTrk, U16 NumHeads, U16 BytesPerSector) {
  int i;
  U8  SectorsPerCluster;
  U16 NumRootDirEntries;
  /*
   * Find properties to use in table
   */
  for (i = 0; (U32)NumSectors > _aFormatInfo[i].SecNum; i++);
  SectorsPerCluster  = _aFormatInfo[i].SectorsPerCluster;
  NumRootDirEntries  = _aFormatInfo[i].NumRootDirEntries;
  return _FatFormatEx(pVolume, NumSectors, Media, SecPerTrk, NumHeads, SectorsPerCluster, NumRootDirEntries, BytesPerSector, 0, 1);
}

/*********************************************************************
*
*       _AutoFormat
*
*  Description:
*    FS internal function. Get information about the media from the 
*    device driver. Based on that informaton, calculate parameters for
*    formatting that media and call the format routine.
*
*  Parameters:
*    Idx         - Index of device in the device information table referred by FS__pDevInfo.
*    Unit        - Unit number.
*  
*  Return value:
*    >=0         - Media has been formatted.
*    <0          - An error has occurred.
*/
static int _AutoFormat(FS_VOLUME * pVolume) {
  FS_DEV_INFO DevInfo;
  FS_MEMSET(&DevInfo, 0, sizeof(FS_DEV_INFO));
  /*
   * Check if there is a partition table
   */
  FS__LocatePartition(pVolume);
  FS_LB_GetDeviceInfo(&pVolume->Partition.Device, &DevInfo);   /* Get info from device */
  if (pVolume->Partition.StartSector) {
    DevInfo.NumSectors = pVolume->Partition.NumSectors;
  }
  /* Check if we have gotten the information */
  if (DevInfo.NumSectors == 0) {
    return -1;     /* Error, we could not get the required info or device is not ready */
  }
  
  /* Format media using calculated values */
  return _FatFormat(pVolume, DevInfo.NumSectors, 0xF8, DevInfo.SectorsPerTrack, DevInfo.NumHeads, DevInfo.BytesPerSector);
}


/*********************************************************************
*
*       _Format
*
*/
static int _Format(FS_VOLUME * pVolume, FS_FORMAT_INFO * pFormatInfo) {
  int Status;
  FS_DEV_INFO DevInfo;
  FS_DEVICE * pDevice;
  
  pDevice = &pVolume->Partition.Device;
  Status = FS_LB_GetStatus(pDevice);
  if (Status >= 0) {
    FS__LocatePartition(pVolume);            /* Check if there is a partition table */
    if ((void*)pFormatInfo->pDevInfo == NULL) {
      FS_LB_GetDeviceInfo(pDevice, &DevInfo);   /* Get info from device */
    } else {
      DevInfo = *pFormatInfo->pDevInfo;
    }
    /* If there is a partition table, then use this information to clip the size (NumSectors) */
    if (pVolume->Partition.StartSector) {
      if (DevInfo.NumSectors > pVolume->Partition.NumSectors) {
        DevInfo.NumSectors = pVolume->Partition.NumSectors;
      }
    }
    Status = _FatFormatEx(pVolume, DevInfo.NumSectors, (U8)0xF8, DevInfo.SectorsPerTrack, DevInfo.NumHeads,
                         (U8)pFormatInfo->SectorsPerCluster, pFormatInfo->NumRootDirEntries, DevInfo.BytesPerSector, 0, 1);
  } else {
    Status = -1;
  }
  return Status;
}

/*********************************************************************
*
*       _FormatEx
*
*/
static int _FormatEx(FS_VOLUME * pVolume, FS_FORMAT_INFO_EX * pFormatInfoEx) {
  int Status;
  FS_DEV_INFO DevInfo;
  FS_DEVICE * pDevice;
  
  pDevice = &pVolume->Partition.Device;
  Status = FS_LB_GetStatus(pDevice);
  if (Status >= 0) {
    FS__LocatePartition(pVolume);            /* Check if there is a partition table */
    if ((void*)pFormatInfoEx->pDevInfo == NULL) {
      FS_LB_GetDeviceInfo(pDevice, &DevInfo);   /* Get info from device */
    } else {
      DevInfo = *pFormatInfoEx->pDevInfo;
    }
    /* If there is a partition table, then use this information to clip the size (NumSectors) */
    if (pVolume->Partition.StartSector) {
      if (DevInfo.NumSectors > pVolume->Partition.NumSectors) {
        DevInfo.NumSectors = pVolume->Partition.NumSectors;
      }
    }
    Status = _FatFormatEx(pVolume, DevInfo.NumSectors, (U8)0xF8, DevInfo.SectorsPerTrack, DevInfo.NumHeads,
                         (U8)pFormatInfoEx->SectorsPerCluster, pFormatInfoEx->NumRootDirEntries, DevInfo.BytesPerSector,
                          pFormatInfoEx->NumReservedSectors, pFormatInfoEx->UpdatePartition);
  } else {
    Status = -1;
  }
  return Status;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_Format
*
*  Description:
*    This functions formats the volume
*
*  Return value:
*    -1     Error, formatting the volume was not successful.
*     0     Success.
*/
int FS_FAT_Format(FS_VOLUME  * pVolume, FS_FORMAT_INFO * pFormatInfo) {
  int r;
  if (pFormatInfo) {
    r = _Format(pVolume, pFormatInfo);
  } else {
    r = _AutoFormat(pVolume);
  }
  return r;
}

/*********************************************************************
*
*       FS_FAT_FormatEx
*
*  Description:
*    This functions formats the volume
*
*  Return value:
*    -1     Error, formatting the volume was not successful.
*     0     Success.
*/
int FS_FAT_FormatEx(FS_VOLUME  * pVolume, FS_FORMAT_INFO_EX * pFormatInfoEx) {
  int r;
  if (pFormatInfoEx) {
    r = _FormatEx(pVolume, pFormatInfoEx);
  } else {
    r = _AutoFormat(pVolume);
  }
  return r;
  
}
/*************************** End of file ****************************/
