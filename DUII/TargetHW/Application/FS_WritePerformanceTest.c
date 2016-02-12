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
File        : FS_WritePerformance.c
Purpose     : Sample program for measure the performance
---------------------------END-OF-HEADER------------------------------
*/

#include "FS_Int.h"
#include "FS_OS.h"
#include <string.h>
#include <stdio.h>

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define NUM_LOOPS                   2048                     // 
#define BLOCK_SIZE                  1024                     // Block size for write operation [byte]
#define DEVICE                      ""                       // Defines the volume that should be used
#define FILE_NAME                   DEVICE"\\default.txt"    // Defines the name of the file to write to

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static U32  _aBuffer[BLOCK_SIZE / 4];
static I32  _tMax;
static I32  _tMin;
static I32  _tAvg;
static I32  _tSum;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/


/*********************************************************************
*
*             MainTask
*/
void MainTask(void);
void MainTask(void) {
  unsigned  i;
  char      ac[200];
  FS_FILE * pFile;
  I32       t;
  int       iLoop;

  FS_Init();
  //
  // Do a low-level format if necessary
  //
  if (FS_IsLLFormatted("") == 0) {
    FS_X_Log("Low level formatting\n");
    FS_FormatLow("");  
  }
  //
  // Perform a high level format
  //
  FS_X_Log("High level formatting\n");
  FS_FormatSD(DEVICE);
  FS_MEMSET((void*)_aBuffer, 'a', sizeof(_aBuffer));
  for (iLoop = 0; iLoop < 2; iLoop++) {
    //
    // Initialize the time values
    //
    _tMin = 0x7fffffff;
    _tMax = 0;
    _tSum = 0;
    //
    // Open file for writing
    //
    if (iLoop == 0) {
      pFile = FS_FOpen(FILE_NAME, "w");
    } else {
      FS_FSeek(pFile, 0, FS_SEEK_SET);
    }
    sprintf(ac, "Writing chunks of %d x %d Bytes \n", NUM_LOOPS, BLOCK_SIZE);
    FS_X_Log(ac);
    for (i = 0; i < NUM_LOOPS ; i++) {
      //
      //  Get time
      //
      t = (I32)FS_X_OS_GetTime();
      FS_Write(pFile, _aBuffer, BLOCK_SIZE);
      t = FS_X_OS_GetTime() - t;
      if (t > _tMax) {
        _tMax = t;
      }
      if (t < _tMin) {
        _tMin = t;
      }
      _tSum        += t;
  //    sprintf(ac, "Write cycle %d: %d ms\n", i, t);
  //    FS_X_Log(ac);
    }
    //
    // Show result
    //
    _tAvg = _tSum / NUM_LOOPS;
    sprintf(ac, "(Min/Avg/Max/Total): %d, %d, %d, %d\n", _tMin, _tAvg, _tMax, _tSum);
    FS_X_Log(ac);
  }
  FS_FClose(pFile);
  FS_X_Log("Finished...\n");
  FS_Unmount("");
  while (1) {
  }
}
/*************************** End of file ****************************/
