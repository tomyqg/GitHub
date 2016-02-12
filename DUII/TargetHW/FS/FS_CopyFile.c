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
File        : FS_CopyFile.c
Purpose     : Implementation of FS_CopyFile
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       Public code, internal
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__CopyFile
*
*  Function description:
*    Internal version of FS_CopyFile.
*    Copies a file
*
*  Parameters:
*    sSource    - Fully qualified file name to source file.
*    sDest      - Fully qualified file name to destination file.
*
*  Return value:
*    ==0         - File has been copied.
*    ==-1        - An error has occurred.
*/
int FS__CopyFile(const char * sSource, const char * sDest) {
  int           r;
  FS_FILE     * pFileSrc;
  FS_FILE     * pFileDest;

  U8         aBuffer[512];
  U32        NumBytes;
  U32        TimeStamp;

  // 
  // Open source file
  //
  pFileSrc  = FS__FOpenEx(sSource, FS_FILE_ACCESS_FLAG_R, 0, 0, 1);
  if ((void *)pFileSrc == NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__CopyFile: Source file could not be opened.\n"));
    return -1;
  }
  // 
  // Open destination file
  // 
  pFileDest = FS__FOpenEx(sDest, FS_FILE_ACCESS_FLAGS_CW, 1, 1, 0);
  if ((void *)pFileDest == NULL) {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__CopyFile: Destination file could not be created.\n"));
    FS__FClose(pFileSrc);
    return -1;
  }
  r = -1;
  //
  //  retrieve the source file size
  // 
  NumBytes = FS__GetFileSize(pFileSrc);
  //
  //  Preallocate destination file to optimize copy
  //
  FS__FSeek(pFileDest, NumBytes, FS_FILE_BEGIN);
  FS__SetEndOfFile(pFileDest);
  FS__FSeek(pFileDest, 0, FS_FILE_BEGIN);
  //
  //
  //  Now copy the data to the destination file
  // 
  do {
    U32 NumBytesRead;
    U32 NumBytesWritten;
    NumBytesRead = FS__Read(pFileSrc, aBuffer, sizeof(aBuffer));
    if (NumBytesRead == 0) {
      r = 0;
      break;
    }
    NumBytesWritten = FS__Write(pFileDest, aBuffer, NumBytesRead);
    NumBytes -= NumBytesRead;
    if (NumBytesWritten != NumBytesRead) {
      r = -1;   // Not all bytes have been written, maybe the volume is full
      break;
    }
    if (NumBytes == 0) {
      r = 0;
      break;
    }
  } while (1);
  //
  //  Close source and destination file
  //  and update the directory entry for destination file
  // 
  FS__FClose(pFileSrc);
  FS__FClose(pFileDest);
  //
  // Since we have copied the file, we need to set the attributes 
  // and timestamp of destination file to the same as source file.
  // 
  if (r == 0) {
    U8 Attrib;
    FS__GetFileTimeEx(sSource, &TimeStamp, FS_FILETIME_CREATE);
    FS__SetFileTimeEx(sDest,   TimeStamp, FS_FILETIME_CREATE);
    Attrib = FS__GetFileAttributes(sSource);
    FS__SetFileAttributes(sDest, Attrib);
  } else {
    //
    // Error occurred, delete the destination file.
    // 
    FS__Remove(sDest);
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
*       FS_CopyFileEx
*
*  Function description:
*    Copies a file
*
*  Parameters:
*    sSource    - Fully qualified file name.
*    sDest      - Fully qualified file name
*
*  Return value:
*    ==0         - File has been copied.
*    ==-1        - An error has occurred.
*/
int FS_CopyFileEx(const char * sSource, const char * sDest, ...);
int FS_CopyFileEx(const char * sSource, const char * sDest, ...) {
  int r;

  FS_LOCK();
  r = FS__CopyFile(sSource, sDest);
  FS_UNLOCK();
  return r;
}


/*********************************************************************
*
*       FS_CopyFile
*
*  Function description:
*    Copies a file
*
*  Parameters:
*    sSource    - Fully qualified file name.
*    sDest      - Fully qualified file name
*
*  Return value:
*    ==0         - File has been copied.
*    ==-1        - An error has occurred.
*/
int FS_CopyFile(const char * sSource, const char * sDest) {
  int r;

  FS_LOCK();
  r = FS__CopyFile(sSource, sDest);
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
