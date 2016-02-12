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
File        : FS_Truncate.c
Purpose     : Implementation of FS_Truncate
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


/*********************************************************************
*
*       FS_Truncate
*
*  Function description:
*    Truncates a open file.
*
*  Parameters:
*    pFile        - Pointer to a valid opened file with write access.
*    NewSize      - New size of the file.
*
*  Return value:
*    == 0        - File has been truncated.
*    ==-1        - An error has occurred.
*
*/
int FS_Truncate(FS_FILE * pFile, U32 NewSize) {
  int           r;
  FS_LOCK();
  r = -1;
  if (pFile->pFileObj->Size > NewSize) {
    FS__FSeek(pFile, (I32)NewSize, FS_FILE_BEGIN);
    r = FS__SetEndOfFile(pFile);
  }
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
