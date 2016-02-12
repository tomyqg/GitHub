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
File        : FS_Read.c
Purpose     : Implementation of FS_Read
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
*       FS__Read
*
*  Function description:
*    Internal version of FS_Read.
*    Reads data from a file.
*
*  Parameters:
*    pData       - Pointer to a buffer to receive the data
*    NumBytes    - Number of bytes to be read
*    pFile       - Pointer to a FS_FILE data structure.
*  
*  Return value:
*    Number of bytes read.
*/
U32 FS__Read(FS_FILE *pFile, void *pData, U32 NumBytes) {
  char InUse;
  U32 NumBytesRead;
  FS_FILE_OBJ * pFileObj;

  if (NumBytes == 0) {
    return 0;
  }
  if (pFile == NULL) {
    return 0;  /* No pointer to a FS_FILE structure */
  }
  NumBytesRead = 0;
  //
  // Load file information
  //
  FS_LOCK_SYS();
  InUse   = pFile->InUse;
  pFileObj = pFile->pFileObj;
  FS_UNLOCK_SYS();
  if ((InUse == 0) || (pFileObj == (FS_FILE_OBJ*)NULL)) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Application error: FS__Read: Invalid file handle.\n"));
    return 0;
  }
  //
  // Lock driver before performing operation
  //
  FS_LOCK_DRIVER(&pFileObj->pVolume->Partition.Device);
  //
  // Multi-tasking environments with per-driver-locking:
  // Make sure that relevant file information has not changed (an other task may have closed the file, unmounted the volume etc.)
  // If it has, no action is performed.
  //
#if FS_OS_LOCK_PER_DRIVER
  FS_LOCK_SYS();
  if (pFileObj != pFile->pFileObj) {
    InUse = 0;
  }
  if (pFile->InUse == 0) {
    InUse = 0;
  }
  FS_UNLOCK_SYS();
  if (InUse == 0) {      // Let's make sure the file is still valid
    FS_DEBUG_ERROROUT((FS_MTYPE_API,"Application error: File handle has been invalidated by other thread during wait"));
  } else
#endif
  //
  // All checks and locking operations completed. Call the File system (FAT/EFS) layer.
  //
  {
    if ((pFile->AccessFlags & FS_FILE_ACCESS_FLAG_R) == 0) {
      pFile->Error = FS_ERR_WRITEONLY; /* File open mode does not allow read ops */
      NumBytesRead = 0;
    } else {
      NumBytesRead = FS_FREAD(pFile, pData, NumBytes);
    }
  }
  FS_UNLOCK_DRIVER(&pFileObj->pVolume->Partition.Device);
  return NumBytesRead;
}

/*********************************************************************
*
*       FS_Read
*
*  Function description:
*    Read data from a file.
*
*  Parameters:
*    pData       - Pointer to a buffer to receive the data
*    NumBytes    - Number of bytes to be read
*    pFile       - Pointer to a FS_FILE data structure.
*  
*  Return value:
*    Number of bytes read.
*/
U32 FS_Read(FS_FILE *pFile, void *pData, U32 NumBytes) {
  U32 i;

  FS_LOCK();
  i = FS__Read(pFile, pData, NumBytes);
  FS_UNLOCK();
  return i;
}


/*************************** End of file ****************************/
