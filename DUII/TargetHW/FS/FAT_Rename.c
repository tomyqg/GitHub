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
File        : FAT_Rename.c
Purpose     : FAT routines for renaming files or directories
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
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_Rename
*
*  Description:
*    Rename a existing file/directory.
*
*  Parameters:
*    sOldName    - path to the file/directory.
*    sNewName    - new file/directory name.
*    pVolume     - Pointer to an FS_VOLUME data structure.
*
*  Return value:
*    0         O.K.
*    1         Error
*/
int FS_FAT_Rename(const char * sOldName, const char * sNewName, FS_VOLUME * pVolume) {
  const char    *  pOldName;
  int              r;
  FS_SB            SB;
  U32              DirStart;

  r = 1;       /* No error so far */
  /*
   * Search directory
   */
  FS__SB_Create(&SB, &pVolume->Partition);
  if (FS_FAT_FindPath(pVolume, &SB, sOldName, &pOldName, &DirStart)) {
    r = FS__FAT_Move(pVolume, DirStart, DirStart, pOldName, sNewName, &SB);
  }
  FS__SB_Delete(&SB);
  return r;
}

/*************************** End of file ****************************/

