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
Purpose     : Sample program which is designed to take performance measurements, using the emFile API.
              At first, the device is formatted in order to have a blank one.
              After formatting the device three different test scenarios are performed:

              1. Checking the write performance by writing into a file which was created without cluster pre-allocation.
                 This is done in order to get sure that when writing to the file, no further
                 writes to the FAT and to the directory entry are necessary which would have
                 bad influence on the performance values.

              2. Checking the write performance by writing into a file which was created without cluster pre-allocation.

              3. Checking the read performance by reading the contents of the file that was previously created.

              By default, in all three test scenarios the file is written/read in:
                16 chunks (512 KB per chunk)
                1 chunk = 64 blocks = 64 calls of FS_FWrite/FS_FRead
                1 block = 8 KB = 8 KB per write/read transaction.

                a total file size of 8 MB

              Sample output:

              W0: Writing 16 chunks of 524288 Bytes: ................OK
              W1: Writing 16 chunks of 524288 Bytes: ................OK
              R0: Reading 16 chunks of 524288 Bytes: ................OK

              Test: 0 First write (Clusters/file size preallocated)
                (Min/Max/Av)[ms]: 450/479/464;
                Speed: 1103.45 kByte/s
                Counters:  ReadOperationCnt    = 17
                           ReadSectorCnt       = 17
                           ReadSectorCachedCnt = 0
                           WriteOperationCnt   = 1024
                           WriteSectorCnt      = 16384

              Test: 1 Second write Dynamic allocation
                (Min/Max/Av)[ms]: 833/1131/936;
                Speed: 547.01 kByte/s
                Counters:  ReadOperationCnt    = 1072
                           ReadSectorCnt       = 1072
                           ReadSectorCachedCnt = 0
                           WriteOperationCnt   = 2080
                           WriteSectorCnt      = 17440

              Test: 2 Read
                (Min/Max/Av)[ms]: 298/316/302;
                Speed: 1695.36 kByte/s
                Counters:  ReadOperationCnt    = 1041
                           ReadSectorCnt       = 16401
                           ReadSectorCachedCnt = 0
                           WriteOperationCnt   = 0
                           WriteSectorCnt      = 0


              Test 0 Speed: 1103.45 kByte/s
              Test 1 Speed: 547.01 kByte/s
              Test 2 Speed: 1695.36 kByte/s
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
  I32 MinChunk;
  I32 MaxChunk;
  I32 AvChunk;
  I32 SumChunk;
  I32 NumSamplesChunk;
  U32 NumBytesChunk;
  I32 MinBlock;
  I32 MaxBlock;
  I32 AvBlock;
  I32 SumBlock;
  I32 NumSamplesBlock;
  U32 NumBytesBlock;
  FS_STORAGE_COUNTERS StorageCounter;
} RESULT;

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static FS_FILE * _pFile;
static U32       _aBuffer[BLOCK_SIZE / 4];
static RESULT    _aResult[3];
static int       _TestNo = -1;
static char      _ac[512];

/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _StartTest
*/
static void _StartTest(const char * sName, U32 NumBytesChunk, U32 NumBytesBlock) {
  RESULT * pResult;

  if ((_TestNo + 1) < COUNTOF(_aResult)) {
    pResult = &_aResult[++_TestNo];
    pResult->sName           = sName;
    pResult->MinChunk        =  0x7fffffff;
    pResult->MaxChunk        = -0x7fffffff;
    pResult->NumSamplesChunk = 0;
    pResult->SumChunk        = 0;
    pResult->NumBytesChunk   = NumBytesChunk;
    pResult->MinBlock        =  0x7fffffff;
    pResult->MaxBlock        = -0x7fffffff;
    pResult->NumSamplesBlock = 0;
    pResult->SumBlock        = 0;
    pResult->NumBytesBlock   = NumBytesBlock;
  }
}

/*********************************************************************
*
*       _StoreResultChunk
*/
static void _StoreResultChunk(I32 t) {
  RESULT * pResult;

  pResult = &_aResult[_TestNo];
  if (t < 0) {
    FS_X_Panic(110);
  }
  if (t > pResult->MaxChunk) {
    pResult->MaxChunk = t;
  }
  if (t < pResult->MinChunk) {
    pResult->MinChunk = t;
  }
  pResult->NumSamplesChunk++;
  pResult->SumChunk += (I32)t;
  pResult->AvChunk   = pResult->SumChunk / pResult->NumSamplesChunk;
}

/*********************************************************************
*
*       _GetAverageChunk
*/
static float _GetAverageChunk(int Index) {
  RESULT * pResult;
  float v;

  pResult = &_aResult[Index];
  v = (float)pResult->AvChunk;
  if (v == 0) {
    return 0;
  }
  v = (float)1000.0 / v;
  v = v * (pResult->NumBytesChunk / 1024);
  return v;
}

/*********************************************************************
*
*       _StoreResultBlock
*/
static void _StoreResultBlock(I32 t) {
  RESULT * pResult;

  pResult = &_aResult[_TestNo];
  if (t < 0) {
    FS_X_Panic(110);
  }
  if (t > pResult->MaxBlock) {
    pResult->MaxBlock = t;
  }
  if (t < pResult->MinBlock) {
    pResult->MinBlock = t;
  }
  pResult->NumSamplesBlock++;
  pResult->SumBlock += (I32)t;
  pResult->AvBlock   = pResult->SumBlock / pResult->NumSamplesBlock;
}

/*********************************************************************
*
*       _GetAverageBlock
*/
static float _GetAverageBlock(int Index) {
  RESULT * pResult;
  float v;

  pResult = &_aResult[Index];
  v = (float)pResult->AvBlock;
  if (v == 0) {
    return 0;
  }
  v = (float)1000.0 / v;
  v = v * (pResult->NumBytesBlock / 1024);
  return v;
}

/*********************************************************************
*
*             _WriteFile
*
*  Write and measure time
*/
static I32 _WriteFile(const void * pData, U32 NumBytes) {
  I32 t0;
  I32 t1;
  int i;

  t0 = (I32)FS_X_OS_GetTime();
  for (i = 0; i < NUM_BLOCKS_MEASURE; i++) {
    t1 = (I32)FS_X_OS_GetTime();
    FS_Write(_pFile, pData, NumBytes);
    _StoreResultBlock(FS_X_OS_GetTime() - t1);
  }
  return FS_X_OS_GetTime() - t0;
}

/*********************************************************************
*
*             _ReadFile
*
*  Read and measure time
*/
static I32 _ReadFile(void * pData, U32 NumBytes) {
  I32 t0;
  I32 t1;
  int i;
  t0 = (I32)FS_X_OS_GetTime();
  for (i = 0; i < NUM_BLOCKS_MEASURE; i++) {
    t1 = (I32)FS_X_OS_GetTime();
    FS_Read(_pFile, pData, NumBytes);
    _StoreResultBlock(FS_X_OS_GetTime() - t1);
  }
  return FS_X_OS_GetTime() - t0;
}

/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       FSTask
*/
void MainTask(void);
void MainTask(void) {
  unsigned i;
  U32 Space;
  U32 NumLoops;
  U32 NumBytes;
  U32 NumBytesAtOnce;
  I32 t;

  FS_Init();
  while (1) {
    _TestNo = -1;
    if (FS_IsLLFormatted("") == 0) {
      FS_X_Log("Low level formatting\n");
      FS_FormatLow("");  /* Erase & Low-level  format the flash */
    }
    FS_X_Log("High level formatting\n");
//    FS_FormatSD(DEVICE);
    FS_Format(DEVICE, NULL);
    FS_WriteUpdateDir(0);
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
    _StartTest("First write (Clusters/file size preallocated)", NumBytes, BLOCK_SIZE);
    _pFile = FS_FOpen(FILE_NAME, "w");
    FS_SetFilePos(_pFile, Space, FS_FILE_BEGIN);
    FS_SetEndOfFile(_pFile);
    FS_SetFilePos(_pFile, 0, FS_FILE_BEGIN);
    //
    // Check write performance with clusters/file size preallocated
    //
    sprintf(_ac, "W0: Writing %ld chunks of %ld Bytes: ", NumLoops, NumBytes);
    FS_X_Log(_ac);
    FS_STORAGE_ResetCounters();
    for (i = 0; i < NumLoops ; i++) {
      t = _WriteFile(&_aBuffer[0], NumBytesAtOnce);
      _StoreResultChunk(t);
      FS_X_Log(".");
    }
    FS_STORAGE_GetCounters(&_aResult[_TestNo].StorageCounter);
    FS_X_Log("OK\n");
    FS_FClose(_pFile);
    //
    // Check write performance with dynamic allocation of clusters
    //
    _StartTest("Second write Dynamic allocation", NumBytes, BLOCK_SIZE);
    _pFile = FS_FOpen(FILE_NAME, "w");
    sprintf(_ac, "W1: Writing %ld chunks of %ld Bytes: ", NumLoops, NumBytes);
    FS_X_Log(_ac);
    FS_STORAGE_ResetCounters();
    for (i = 0; i < NumLoops ; i++) {
      t = _WriteFile(&_aBuffer[0], NumBytesAtOnce);
      _StoreResultChunk(t);
      FS_X_Log(".");
    }
    FS_STORAGE_GetCounters(&_aResult[_TestNo].StorageCounter);
    FS_X_Log("OK\n");
    FS_FClose(_pFile);
    //
    // Check read performance
    //
    _StartTest("Read", NumBytes, BLOCK_SIZE);
    sprintf(_ac, "R0: Reading %ld chunks of %ld Bytes: " , NumLoops, NumBytes);
    FS_X_Log(_ac);
    _pFile = FS_FOpen(FILE_NAME, "r");
    FS_STORAGE_ResetCounters();
    for (i = 0; i < NumLoops; i++) {
      t = _ReadFile(_aBuffer, NumBytesAtOnce);
      _StoreResultChunk(t);
      sprintf(_ac, "Read cycle %d: %lu ms\n", i, t);
      FS_X_Log(".");
    }
    FS_STORAGE_GetCounters(&_aResult[_TestNo].StorageCounter);
    FS_X_Log("OK\n\n");
    FS_FClose(_pFile);
    //
    // Show results
    //
    for (i = 0; i <= (unsigned)_TestNo; i++) {
      sprintf(_ac, "Test: %d %s\n"
                   "(Min/Max/Av)[ms] per chunk (%d KB): %ld/%ld/%ld;\n  Speed: %4.2f kByte/s\n"
                   "(Min/Max/Av)[ms] per block (%d KB): %ld/%ld/%ld;\n  Speed: %4.2f kByte/s\n"
                   "  Counters:  ReadOperationCnt    = %d\n"
                   "             ReadSectorCnt       = %d\n"
                   "             ReadSectorCachedCnt = %d\n"
                   "             WriteOperationCnt   = %d\n"
                   "             WriteSectorCnt      = %d\n\n", i, _aResult[i].sName,
                                                                ((Space / NumLoops) >> 10), _aResult[i].MinChunk, _aResult[i].MaxChunk, _aResult[i].AvChunk, _GetAverageChunk(i),
                                                                BLOCK_SIZE, _aResult[i].MinBlock, _aResult[i].MaxBlock, _aResult[i].AvBlock, _GetAverageBlock(i),
                                                                _aResult[i].StorageCounter.ReadOperationCnt,
                                                                _aResult[i].StorageCounter.ReadSectorCnt,
                                                                _aResult[i].StorageCounter.ReadSectorCachedCnt,
                                                                _aResult[i].StorageCounter.WriteOperationCnt,
                                                                _aResult[i].StorageCounter.WriteSectorCnt);
      FS_X_Log(_ac);
    }
    FS_X_Log("\n");
    //
    // Show results for performance list
    //
    for (i = 0; i <= (unsigned)_TestNo; i++) {
      sprintf(_ac, "Test %d Speed per chunk (%d KB): %4.2f kByte/s\n"
                   "        Speed per block (%d KB); %4.2f kByte/s\n", i, ((Space / NumLoops) >> 10), _GetAverageChunk(i),
                                                                       BLOCK_SIZE, _GetAverageBlock(i));
      FS_X_Log(_ac);
    }
    FS_X_Log("Finished...\n");
    FS_Unmount("");
    while (1) {
    }
  }
}

/*************************** End of file ****************************/
