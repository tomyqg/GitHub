/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2007         SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : FS_ConfigIO.c
Purpose     : I/O Configuration routines for Filesystem
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include "FS.h"

/*********************************************************************
*
*       FS_X_Panic
*
*  Function description
*    Referred in debug builds of the file system only and
*    called only in case of fatal, unrecoverable errors.
*/
void FS_X_Panic(int ErrorCode) {
  FS_USE_PARA(ErrorCode);
  while (1);
}

/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefore not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/
void FS_X_Log(const char *s) {
  printf("%s", s);
}

void FS_X_Warn(const char *s) {
  printf("FS warning: %s\n", s);
}

void FS_X_ErrorOut(const char *s) {
  printf("FS error: %s\n", s);
}

/*************************** End of file ****************************/
