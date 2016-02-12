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
File        : FS_Verify.c
Purpose     : Implementation of FS_Verify
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
*       Defines, configurable
*
**********************************************************************
*/
#ifndef FS_VERIFY_BUFFER_SIZE
  #define FS_VERIFY_BUFFER_SIZE 128
#endif

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
*       FS__Verify
*
*  Function description:
*    Internal version of FS_Verify
*    Verifies a file with a given data buffer
*
*  Parameters:
*    pFile       - Pointer to an open file.
*    pData       - Pointer to the data source of verification
*    NumBytes    - Number of bytes to be verified
*
*  Return value:
*    ==0         - Verification was successful.
*    !=0         - Verification failed.
*/
int FS__Verify(FS_FILE * pFile, const void *pData, U32 NumBytes) {
  U8         acVerifyBuffer[FS_VERIFY_BUFFER_SIZE];
  U32        NumBytesInFile;
  U32        NumBytesAtOnce;
  U32        NumBytesRead;
  int           r;
  const U8 * p;

  r = 1;
  if (pData == NULL) {
    return 1;                  /* Failure */
  }
  p = (const U8 *)pData;
  if (pFile) {
    NumBytesInFile = FS__GetFileSize(pFile);
    if (NumBytesInFile < NumBytes) {
      NumBytes = NumBytesInFile;
    }
    do {
      /* Request only as much bytes as are available. */
      NumBytesAtOnce = (FS_VERIFY_BUFFER_SIZE > NumBytes) ? NumBytes : FS_VERIFY_BUFFER_SIZE;
      NumBytesRead   = FS__Read(pFile, acVerifyBuffer, NumBytesAtOnce);
      if (NumBytesRead < NumBytesAtOnce) {
        break;                   /* Could not read sufficient data -> Failure */
      }
      r = FS_MEMCMP(p, acVerifyBuffer, NumBytesRead);
      if (r != 0) {
        break;                   /* Verification failed */
      }
      NumBytes -= NumBytesRead;
      p        += NumBytesRead;
      if (NumBytes == 0) {
        r = 0;
        break;                   /* Data successfully verified */
      }
    } while (1);
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
*       FS_Verify
*
*  Function description:
*    Verifies a file with a given data buffer
*
*  Parameters:
*    pFile       - Pointer to an open file.
*    pData       - Pointer to the data source of verification
*    NumBytes    - Number of bytes to be verified
*
*  Return value:
*    ==0         - Verification was successful.
*    !=0         - Verification failed.
*/
int FS_Verify(FS_FILE * pFile, const void *pData, U32 NumBytes) {
  int r;
  FS_LOCK();
  r = FS__Verify(pFile, pData, NumBytes);
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
