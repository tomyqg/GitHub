/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2006         SEGGER Microcontroller Systeme GmbH        *
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
File        : FS_Performance.c
Purpose     : Sample program for measure the performance
---------------------------END-OF-HEADER------------------------------
*/

#include "FS_Int.h"
#include "FS_OS.h"
#include <string.h>
#include <stdio.h>

#define FILE_SIZE                   (8192L * 1024L)            // Defines the file size that should be used
#define BLOCK_SIZE                  (8L    * 1024L)            // Block size for individual read / write operation [byte]
#define NUM_BLOCKS_MEASURE          ( 64)                    // Number of blocks for individual measurement
#define DEVICE                      ""                       // Defines the volume that should be used
#define FILE_NAME                   DEVICE"\\default.txt"    // Defines the name of the file to write to

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  const char * sName;
  I32 Min;
  I32 Max;
  I32 Av;
  I32 Sum;
  I32 NumSamples;
  U32 NumBytes;
} RESULT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32       _aBuffer[BLOCK_SIZE / 4];
static RESULT    _aResult[2];
static int       _TestNo = -1;
static char      _ac[512];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*             _WriteFile
*
*  Write and measure time
*/
static I32 _WriteFile(FS_FILE * pFile, const void * pData, U32 NumBytes) {
  I32 t;
  int i;
  t = (I32)FS_X_OS_GetTime();
  for (i = 0; i < NUM_BLOCKS_MEASURE; i++) {
    FS_Write(pFile, pData, NumBytes);
  }
  return FS_X_OS_GetTime() - t;
}


/*********************************************************************
*
*             _ReadFile
*
*  Read and measure time
*/
static I32 _ReadFile(FS_FILE * pFile, void * pData, U32 NumBytes) {
  I32 t;
  int i;
  t = (I32)FS_X_OS_GetTime();
  for (i = 0; i < NUM_BLOCKS_MEASURE; i++) {
    FS_Read(pFile, pData, NumBytes);
  }
  return FS_X_OS_GetTime() - t;
}
/*********************************************************************
*
*       _StartTest
*/
static void _StartTest(const char * sName, U32 NumBytes) {
  RESULT * pResult;
 
  if ((_TestNo + 1) < COUNTOF(_aResult)) {
    pResult = &_aResult[++_TestNo];
    pResult->sName = sName;
    pResult->Min =  0x7fffffff;
    pResult->Max = -0x7fffffff;
    pResult->NumSamples = 0;
    pResult->Sum = 0;
    pResult->NumBytes = NumBytes;
  }
}

/*********************************************************************
*
*       _StoreResult
*/
static void _StoreResult(I32 t) {
  RESULT * pResult;
  
  pResult = &_aResult[_TestNo];
  if (t < 0) {
    FS_X_Panic(110);
  }
  if (t > pResult->Max) {
    pResult->Max = t;
  }
  if (t < pResult->Min) {
    pResult->Min = t;
  }
  pResult->NumSamples++;
  pResult->Sum += (I32)t;
  pResult->Av   = pResult->Sum / pResult->NumSamples;
}

/*********************************************************************
*
*       _StoreResult
*/
static double _GetAverage(int Index) {
  RESULT * pResult;
  double v;

  pResult = &_aResult[Index];
  v = (double)pResult->Av;
  if (v == 0) {
    return 0;
  }
  v = (double)1000.0 / v;
  v = v * (pResult->NumBytes / 1024);
  return v;
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
void MainTask(void);
void MainTask(void) {
  unsigned i;
  U32 Space;
  U32 NumLoops;
  U32 NumBytes;
  U32 NumBytesAtOnce;
  FS_FILE * pFile;
  I32 t;

  FS_Init();
  while (1) {
    _TestNo = -1;
    //
    // Check if we need to low-level format the volume
    //
    if (FS_IsLLFormatted("") == 0) {
      FS_X_Log("Low level formatting\n");
      FS_FormatLow("");  /* Erase & Low-level  format the flash */
    }
    //
    // Volume is always high level formatted
    // before doing any performance tests.
    //
    FS_X_Log("High level formatting\n");
    if (FS_FormatSD(DEVICE) == 0) {
  //    FS_Format(DEVICE, NULL);
      //
      // Disable that the directory entry is every time 
      // updated after a write operation
      //
      FS_WriteUpdateDir(0);
      //
      // Fill the buffer with data
      //
      FS_MEMSET((void*)_aBuffer, 'a', sizeof(_aBuffer));
      //
      // Get some general info
      //
      Space          = FS_GetVolumeFreeSpace(DEVICE);
      Space          = MIN(Space, FILE_SIZE);
      NumBytes       = BLOCK_SIZE * NUM_BLOCKS_MEASURE;
      NumBytesAtOnce = BLOCK_SIZE;
      NumLoops       = Space / NumBytes;
      //
      // Create file of full size
      //
      _StartTest("W", NumBytes);
      pFile = FS_FOpen(FILE_NAME, "w");
      //
      // Preallocate the file, setting the file pointer to the highest position
      // and declare it as the end of the file.
      //
      FS_SetFilePos(pFile, Space, FS_FILE_BEGIN);
      FS_SetEndOfFile(pFile);
      //
      // Set file position to the beginning
      //
      FS_SetFilePos(pFile, 0, FS_FILE_BEGIN);
      //
      // Check write performance with clusters/file size preallocated
      //
      sprintf(_ac, "Writing %ld chunks of %ld Bytes: ", NumLoops, NumBytes);
      FS_X_Log(_ac);
      for (i = 0; i < NumLoops ; i++) {
        t = _WriteFile(pFile, &_aBuffer[0], NumBytesAtOnce);
        _StoreResult(t);
        FS_X_Log(".");
      }
      FS_X_Log("OK\n");
      FS_FClose(pFile);
      //
      // Check read performance
      //
      _StartTest("R", NumBytes);
      sprintf(_ac, "Reading %ld chunks of %ld Bytes: " , NumLoops, NumBytes);
      FS_X_Log(_ac);
      pFile = FS_FOpen(FILE_NAME, "r");
      for (i = 0; i < NumLoops; i++) {
        t = _ReadFile(pFile, _aBuffer, NumBytesAtOnce);
        _StoreResult(t);
        FS_X_Log(".");
      }
      FS_X_Log("OK\n\n");
      FS_FClose(pFile);
      //
      // Show results for performance list
      //
      for (i = 0; i <= (unsigned)_TestNo; i++) {
        sprintf(_ac, "%s Speed: %f kByte/s\n", _aResult[i].sName, _GetAverage(i));
        FS_X_Log(_ac);
      }
      FS_X_Log("Finished...\n");
      FS_Unmount("");
    } else {
      FS_X_Log("Volume could not be formatted!\n");
    }
    while (1) {
    }
  }
}

/*************************** End of file ****************************/
