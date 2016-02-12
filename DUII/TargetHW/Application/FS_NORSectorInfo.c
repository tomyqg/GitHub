/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2007         SEGGER Microcontroller Systeme GmbH        *
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
File        : NORSectorInfo.c
Purpose     : Check the infos of all phjysical sectors.
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include <stdio.h>
#include "FS.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
static char _acBuffer[0x400];


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ShowDiskInfo
*
*/
static void _ShowDiskInfo(FS_NOR_DISK_INFO* pDiskInfo) {
  char acBuffer[80];

  FS_X_Log("Disk Info: \n");
  FS_NOR_GetDiskInfo(0, pDiskInfo); 
  sprintf(acBuffer," Physical sectors: %d\n"
                   " Logical sectors : %d\n" 
                   " Used sectors: %d\n", pDiskInfo->NumPhysSectors, pDiskInfo->NumLogSectors, pDiskInfo->NumUsedSectors);
  FS_X_Log(acBuffer);
}

/*********************************************************************
*
*       _ShowSectorInfo
*/
static void _ShowSectorInfo(FS_NOR_SECTOR_INFO* pSecInfo, U32 PhysSectorIndex) {
  char acBuffer[400];

  FS_X_Log("Sector Info: \n");
  FS_NOR_GetSectorInfo(0, PhysSectorIndex, pSecInfo);
  sprintf(acBuffer," Physical sector No.     : %d\n"
                   " Offset                  : %d\n"
                   " Size                    : %d\n"
                   " Erase Count             : %d\n" 
                   " Used logical sectors    : %d\n"
                   " Free logical sectors    : %d\n" 
                   " Erasable logical sectors: %d\n", PhysSectorIndex, pSecInfo->Off, pSecInfo->Size, pSecInfo->EraseCnt, pSecInfo->NumUsedSectors, pSecInfo->NumFreeSectors, pSecInfo->NumEraseableSectors);
  FS_X_Log(acBuffer);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTask(void);  // Avoid Prototype warning/error
void MainTask(void) {
  U32                i, j;
  FS_NOR_DISK_INFO   DiskInfo;
  FS_NOR_SECTOR_INFO SecInfo;

  FS_FILE * pFile;  
  FS_Init();
  FS_FormatLLIfRequired("");
  memset(_acBuffer, 'A', sizeof(_acBuffer));
  //
  // Check if volume needs to be high level formatted.
  //
  if (FS_IsHLFormatted("") == 0) {
    printf("High level formatting\n");
    FS_Format("", NULL);
  }
  _ShowDiskInfo(&DiskInfo);
  for (i = 0;  i < 1000; i++) {
    pFile = FS_FOpen("Test.txt","w");
    if(pFile != 0) {
      FS_Write(pFile, &_acBuffer[0], sizeof(_acBuffer));
      FS_FClose(pFile);
      printf("Loop cycle: %d\n", i);
      for(j = 0; j < DiskInfo.NumPhysSectors; j++) {
        _ShowSectorInfo(&SecInfo, j);
      }
    }
  }
  while(1);
}


/*************************** End of file ****************************/

