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
File        : FAT_DiskInfo.c
Purpose     : FAT File System Layer for handling disk information
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

/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/


/*********************************************************************
*
*       Static const
*
**********************************************************************
*/


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_GetDiskInfo
*
*  Description:
*    Store information about used/unused clusters
*    in a FS_DISK_INFO data structure.
*
*  Parameters:
*    pDiskData   - Pointer to a FS_DISK_INFO data structure.
*
*  Return value:
*    ==0         - Information is stored in pDiskData.
*    <0          - An error has occurred.
*/
int FS_FAT_GetDiskInfo(FS_VOLUME * pVolume, FS_DISK_INFO * pDiskData, int Flags) {
  FS_FAT_INFO* pFATInfo;
  U32 iCluster;
  U32 LastCluster;
  U32 NumFreeClusters;
  FS_SB  sb;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  if (pDiskData == NULL) {
    return -1;  /* No pointer to a FS_DISK_INFO structure */
  }
  FS__SB_Create(&sb, &pVolume->Partition);
  if (Flags & FS_DISKINFO_FLAG_USE_FREE_SPACE) {
    LastCluster = pFATInfo->NumClusters + 1;
    if ((pFATInfo->NumFreeClusters != INVALID_NUM_FREE_CLUSTERS_VALUE) && (pFATInfo->NumFreeClusters <= pFATInfo->NumClusters)) {
      NumFreeClusters = pFATInfo->NumFreeClusters;
    } else {
      //
      // Start to count the empty clusters
      //
      NumFreeClusters = 0;
      for (iCluster = 2; iCluster <= LastCluster; iCluster++) {
        if (FS_FAT_ReadFATEntry(pVolume, &sb, iCluster) == 0) {
          NumFreeClusters++;
        }
        if (sb.HasError) {
          FS_MEMSET(pDiskData, 0, sizeof(FS_DISK_INFO));
          FS__SB_Delete(&sb);
          return -1;
        }
      }
      pFATInfo->NumFreeClusters = NumFreeClusters;         /* Update FATInfo */
    }
  } else {
    NumFreeClusters = 0;
  }
  pDiskData->NumTotalClusters  = pFATInfo->NumClusters;
  pDiskData->NumFreeClusters   = NumFreeClusters;
  pDiskData->SectorsPerCluster = pFATInfo->SecPerClus;
  pDiskData->BytesPerSector    = pFATInfo->BytesPerSec;
  FS__SB_Delete(&sb);
  return 0;
}

/*********************************************************************
*
*       FS_FAT_GetDiskSpace
*
*  Description:
*    Return the available disk space on volume
*
*  Return value:
*    !=0          - Size of disk in bytes.
*    ==0          - An error has occurred.
*    ==0xFFFFFFFF - Disk size > 4GBytes.
*/
U32 FS_FAT_GetDiskSpace(FS_VOLUME * pVolume) {
  U32 r;
  FS_FAT_INFO * pFATInfo;

  pFATInfo = &pVolume->FSInfo.FATInfo;
  r = FS__CalcSizeInBytes(pFATInfo->NumClusters, pFATInfo->SecPerClus, pFATInfo->BytesPerSec);
  return r;
}
/*************************** End of file ****************************/
