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
File        : FS_GetNumOpenFiles.c
Purpose     : Implementation of FS_GetNumOpenFiles
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
#include "FS_OS.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_GetNumOpenFiles
*
*  Function description:
*    Returns how many files are currently opened.
*
*  Parameters:
*    None
*
*
*  Return value:
*      Number of files that are opened.
*
*
*/
int FS_GetNumFilesOpen(void) {
  int r;
  FS_FILE * pFile;

 
  FS_LOCK();
  r = 0;
  pFile = FS_Global.pFirstFilehandle;
  FS_LOCK_SYS();
  while (pFile) { /* While no free entry found. */
    if (pFile->InUse) {
      r++;
    }
    pFile = pFile->pNext;
  }
  FS_UNLOCK_SYS();
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
