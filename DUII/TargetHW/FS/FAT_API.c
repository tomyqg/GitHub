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
File        : FAT_API.c
Purpose     : FAT File System Layer function table
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

#if FS_SUPPORT_MULTIPLE_FS
/*********************************************************************
*
*       Public const
*
**********************************************************************
*/
/*********************************************************************
*
*       FS_FAT_API
*
*  Description:
*    This structure is used to allow to use different file system types
*    such as FAT and EFS.
*    It contains all API functions that are used by the public API.
*/
const FS_FS_API FS_FAT_API = {
  &FS_FAT_CheckBPB,
  &FS_FAT_Open,
  &FS_FAT_Close,
  &FS_FAT_Read,
  &FS_FAT_Write,
  &FS_FAT_Format,
  &FS_FAT_OpenDir,
  &FS_FAT_CloseDir,
  &FS_FAT_ReadDir,
  &FS_FAT_RemoveDir,
  &FS_FAT_CreateDir,
  &FS_FAT_Rename,
  &FS_FAT_Move,
  &FS_FAT_SetDirEntryInfo,
  &FS_FAT_GetDirEntryInfo,
  &FS_FAT_SetEndOfFile,
  &FS_FAT_Clean,
  &FS_FAT_GetDiskInfo,
  &FS_FAT_GetVolumeLabel,
  &FS_FAT_SetVolumeLabel,
  &FS_FAT_CreateJournalFile,
  &FS_FAT_OpenJournalFile,
  &FS_FAT_GetIndexOfLastSector,
  &FS_FAT__CheckDisk

};
#else
  void FAT_API_c(void);
  void FAT_API_c(void){}
#endif

/*************************** End of file ****************************/
