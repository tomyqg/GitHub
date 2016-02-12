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
File        : FS_GetNumFiles.c
Purpose     : Implementation of FS_GetNumFiles
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include <stdio.h>

#include "FS_Int.h"

/*********************************************************************
*
*       Public code, internal
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__GetNumFiles
*
*  Function description:
*    API function. Returns the size of a file
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*                  The file must have been opened with read or write access.
*
*  Return value:
*    0xFFFFFFFF     - Indicates failure
*    0 - 0xFFFFFFFE - File size of the given file
*
*/
U32 FS__GetNumFiles(FS_DIR *pDir) {
  U32 r;

  if (pDir) {
    U16         EntryIndexOld;
    FS_DIR_POS  SectorPosOld;
    /* Save the old position in pDir structure */
    EntryIndexOld = pDir->Dir.DirEntryIndex;
    SectorPosOld = pDir->Dir.DirPos;
    FS__RewindDir(pDir);
    r = 0;
    do {
      U8         Attr;
      if (FS__ReadDir(pDir) == (FS_DIRENT *)NULL) {
        break; /* No more files */
      }
      FS__DirEnt2Attr(&pDir->DirEntry, &Attr);
      if (!(Attr & FS_ATTR_DIRECTORY)) { /* Is directory entry the volume ID  or a directory, ignore them */
        r++;
      }
    } while (1);
    /* Restore the old position in pDir structure */
    pDir->Dir.DirEntryIndex = EntryIndexOld;
    pDir->Dir.DirPos        = SectorPosOld;
  } else {
    r = 0xffffffffUL;
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
*       FS_GetNumFiles
*
*  Function description:
*    API function. Returns the size of a file
*
*  Parameters:
*    pFile       - Pointer to a FS_FILE data structure.
*                  The file must have been opened with read or write access.
*
*  Return value:
*    0xFFFFFFFF     - Indicates failure
*    0 - 0xFFFFFFFE - File size of the given file
*
*/
U32 FS_GetNumFiles(FS_DIR *pDir) {
  U32 r;

  FS_LOCK();
  r = FS__GetNumFiles(pDir);
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
