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
File        : FAT_DirEntry.c
Purpose     : FAT routines for retrieving/setting dir entry info
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"
#include "FAT.h"
#include "FAT_Intern.h"


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _GetDirInfo
*
*/
static void _GetDirInfo(FS_FAT_DENTRY * pDirEntry, void  * p, int Mask) {
  U32 * pTimeStamp;

  pTimeStamp  = (U32 *)p;
  switch (Mask) {
    case FS_DIRENTRY_GET_ATTRIBUTES:
    {
      U8 * pAttributes;
      pAttributes = (U8 *)p;
      *pAttributes = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
      break;
    }
    case FS_DIRENTRY_GET_TIMESTAMP_CREATE:
      *pTimeStamp = FS_LoadU32LE(&pDirEntry->data[DIR_ENTRY_OFF_CREATION_TIME]);
      break;
    case FS_DIRENTRY_GET_TIMESTAMP_MODIFY:
      *pTimeStamp = FS_LoadU32LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_TIME]);
      break;
    case FS_DIRENTRY_GET_TIMESTAMP_ACCESS:
      *pTimeStamp = (U32)FS_LoadU16LE(&pDirEntry->data[DIR_ENTRY_OFF_LAST_ACCESS_DATE]) << 16;
      break;
  }
}

/*********************************************************************
*
*       _SetDirInfo
*
*/
static void _SetDirInfo(FS_FAT_DENTRY * pDirEntry, const void  * p, int Mask) {
  U32 TimeStamp;
  U16 Date;
  U16 Time;

  if (Mask == FS_DIRENTRY_SET_ATTRIBUTES) {
    U8         Attributes;
    U8         AttributesSet;
    AttributesSet = *(const U8 *)p;
    Attributes    = pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES];
    //
    // Check if directory entry is a directory. This attribute needs
    //
    if ((Attributes & FS_FAT_ATTR_DIRECTORY) == FS_FAT_ATTR_DIRECTORY) {
      AttributesSet |= FS_FAT_ATTR_DIRECTORY;
    } else {
      AttributesSet &= ~FS_FAT_ATTR_DIRECTORY;
    }
    pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] = AttributesSet;
  } else {
    TimeStamp  = *(const U32 *)p;
    Date = (U16)(TimeStamp >> 16);
    Time = (U16)(TimeStamp & 0xffff);
    switch (Mask) {
    case FS_DIRENTRY_SET_TIMESTAMP_CREATE:
      FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_CREATION_TIME], Time);
      FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_CREATION_DATE], Date);
      break;
    case FS_DIRENTRY_SET_TIMESTAMP_MODIFY:
      FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_TIME], Time);
      FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_WRITE_DATE], Date);
      break;
    case FS_DIRENTRY_SET_TIMESTAMP_ACCESS:
      FS_StoreU16LE(&pDirEntry->data[DIR_ENTRY_OFF_LAST_ACCESS_DATE], Date);
      break;
    }
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
*       FS_FAT_GetDirEntryInfo
*
*  Description:
*    Retrieves information of a directory entry
*
*  Parameters:
*    pVolume     - Pointer to an FS_VOLUME data structure.
*    sName       - path to the file/directory.
*    p           - pointer to a buffer that receives the information
*    Mask        - Type of the information that need to be retrieved.
*
*  Return value:
*    0         O.K.
*    1         Error
*/
char FS_FAT_GetDirEntryInfo(FS_VOLUME * pVolume, const char * sName,       void * p, int Mask) {
  const char    *  pName;
  char             r;
  FS_SB            SB;
  U32           DirStart;
  FS_FAT_DENTRY  * pDirEntry;
  /*
   * Search directory
   */
  FS__SB_Create(&SB, &pVolume->Partition);
  if (FS_FAT_FindPath(pVolume, &SB, sName, &pName, &DirStart) == 0) {
    FS__SB_Delete(&SB);
    return 1;  /* Directory not found */
  }
  r = 1;       /* No error so far */
  /*
   * check if entry exists
   */
  pDirEntry = FS_FAT_FindDirEntry(pVolume, &SB, pName, 0, DirStart, 0, NULL);
  if (pDirEntry) {
    _GetDirInfo(pDirEntry, p, Mask);
    r = 0;
  }
  FS__SB_Delete(&SB);

  return r;
}

/*********************************************************************
*
*       FS_FAT_SetDirEntryInfo
*
*  Description:
*    Sets the desired information to a directory entry
*
*  Parameters:
*    pVolume     - Pointer to an FS_VOLUME data structure.
*    sName       - path to the file/directory.
*    p           - pointer to a buffer that holds the information
*    Mask        - Type of the information that needs to be updated.
*
*  Return value:
*    0         O.K.
*    1         Error
*/
char FS_FAT_SetDirEntryInfo(FS_VOLUME * pVolume, const char * sName, const void * p, int Mask) {
  const char    *  pName;
  char             r;
  FS_SB            SB;
  U32           DirStart;
  FS_FAT_DENTRY  * pDirEntry;
  /*
   * Search directory
   */
  FS__SB_Create(&SB, &pVolume->Partition);
  if (FS_FAT_FindPath(pVolume, &SB, sName, &pName, &DirStart) == 0) {
    FS__SB_Delete(&SB);
    return 1;  /* Directory not found */
  }
  r = 1;       /* No error so far */
  /*
   * check if entry exists
   */
  pDirEntry = FS_FAT_FindDirEntry(pVolume, &SB, pName, 0, DirStart, 0, NULL);
  if (pDirEntry) {
    _SetDirInfo(pDirEntry, p, Mask);
    FS__SB_MarkDirty(&SB);
    r = 0;
  }
  FS__SB_Delete(&SB);

  return r;
}



/*************************** End of file ****************************/

