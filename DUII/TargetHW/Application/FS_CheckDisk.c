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
File        : FS_CheckDisk.c
Purpose     : Sample program demonstrating FS_CheckDisk functionality
---------------------------END-OF-HEADER------------------------------
*/

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "FS_API.h"
#include "Global.h"

static U32 _aBuffer[2000];   // The more space is used the faster CheckDisk can run.

/*********************************************************************
*
*       _OnError
*/
static int _OnError(int ErrCode, ...) {
  va_list ParamList;
  const char * sFormat;
  char  c;
  char ac[1000];

  sFormat = FS_FAT_CheckDisk_ErrCode2Text(ErrCode);
  if (sFormat) {
    va_start(ParamList, ErrCode);
    vsprintf(ac, sFormat, ParamList);
    printf("%s\n", ac);
  }
  if (ErrCode != FS_ERRCODE_CLUSTER_UNUSED) {
    printf("  Do you want to repair this? (y/n/a) ");
  } else {
    printf("  * Convert lost cluster chain into file (y)\n"
           "  * Delete cluster chain                 (d)\n"
           "  * Do not repair                        (n)\n"
           "  * Abort                                (a) ");
    printf("\n");
  }
  c = getchar();
  printf("\n");
  if ((c == 'y') || (c == 'Y')) {
    return 1;
  } else if ((c == 'a') || (c == 'A')) {
    return 2;
  } else if ((c == 'd') || (c == 'D')) {
    return 3;
  }
  return 0;     // Do not fix.
}



/*********************************************************************
*
*       MainTask
*/
void MainTask(void);      // Forward declaration to avoid "no prototype" warning
void MainTask(void) {
  FS_Init();
  while (FS_CheckDisk("", &_aBuffer[0], sizeof(_aBuffer), 5, _OnError) == 1) {
  }
  printf("Finished\n");
  while (1);
}

/*************************** End of file ****************************/

