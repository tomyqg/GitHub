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
File        : FAT_SetEndOfFile.c
Purpose     : FAT routine for setting the end of file position
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
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _TruncateFile
*
*  Purpose
*    Truncates the file. This means, the file's cluster chain will 
*    be shorten
*
*/
static int _TruncateFile(FS_FILE * pFile, FS_VOLUME * pVolume, FS_SB * pSB) {
  FS_FAT_INFO * pFATInfo;
  FS_FILE_OBJ * pFileObj;
  U32           NumActClusters;
  U32           ClusterAbs;
  U32           NumCluster2Del;
  U32           ClusterStart2Delete;
  U32           NumNewClusters;
  U32           NewSize;
  int           r;

  pFileObj        = pFile->pFileObj;
  NewSize         = pFile->FilePos;
  pFATInfo        = &pVolume->FSInfo.FATInfo;
  //
  //  Calculate the number of clusters allocated to file.
  //
  NumActClusters  = (pFileObj->Size + pFATInfo->BytesPerCluster) / pFATInfo->BytesPerCluster;
  //
  // Calculate the number of cluster for the new file size.
  //
  NumNewClusters  = (NewSize + pFATInfo->BytesPerCluster) / pFATInfo->BytesPerCluster;
  //
  //   Number of clusters to delete
  //
  NumCluster2Del  = NumActClusters - NumNewClusters;
  //
  // Go to the cluster from where we will deallocate the "unused" clusters.
  //
  ClusterAbs      = FS_FAT_WalkCluster(pVolume, pSB, pFileObj->FirstCluster, NumNewClusters - 1);
  ClusterStart2Delete = FS_FAT_WalkCluster(pVolume, pSB, ClusterAbs, 1);
  //
  // Free the cluster chain.
  //
  FS_FAT_FreeClusterChain(pVolume, pSB, ClusterStart2Delete, NumCluster2Del);
  //
  //  Mark the last cluster to be the last in the chain.
  //
  FS_FAT_MarkClusterEOC(pVolume, pSB, ClusterAbs);
  //
  // Update the Cluster information in pFileObj.
  //
  pFileObj->Data.Fat.CurClusterAbs  = ClusterAbs;
  pFileObj->Data.Fat.CurClusterFile = NumNewClusters;
  pFileObj->Data.Fat.NumAdjClusters = 0;
  r = 0;
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
*       FS_FAT_SetEndOfFile
*
*  Description:
*    Updates the pFile handle and adds/remove cluster to the file handle.
*
*  Parameters:
*    pFile       - Pointer to a opened file with write permission.
*
*  Return value:
*    0         O.K.
*    1         Error
*/
int FS_FAT_SetEndOfFile(FS_FILE * pFile) {
  FS_SB            SB;
  FS_VOLUME      * pVolume;
  FS_FILE_OBJ    * pFileObj;
  int              r;

  pFileObj        = pFile->pFileObj;
  pVolume         = pFileObj->pVolume;
  r               = -1;
  FS__SB_Create(&SB, &pVolume->Partition);
  //
  // Do we want to truncate the file ?
  //
  if (pFile->FilePos < pFileObj->Size) {
    r = _TruncateFile(pFile, pVolume, &SB);
  } else if (pFile->FilePos > pFileObj->Size) {
    //
    // To avoid allocating one cluster more than required, temporarily subtract 1 byte from FilePos
    //
    pFile->FilePos--;
    r = FS_FAT_GotoClusterAllocIfReq(pFile, &SB);
    pFile->FilePos++;
  }
  if (r == 0) {
    pFileObj->Size = pFile->FilePos;
  }
  FS_FAT_UpdateDirEntry(pFileObj, &SB);
  FS__SB_Delete(&SB);
  return r;
}

/*************************** End of file ****************************/

