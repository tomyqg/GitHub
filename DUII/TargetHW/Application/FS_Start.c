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
File    : FS_Start.c
Purpose : Start application for file system.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "FS.h"
#include "FS_Int.h"


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
void MainTask(void);      // Forward declaration to avoid "no prototype" warning
void MainTask(void) {
  U32          v;
  FS_FILE    * pFile;
  char         ac[256];
  const char * sVolName = "";

  //
  // Initialize file system
  //
  FS_Init();
  //
  // Check if low-level format is required
  //
  FS_FormatLLIfRequired("");
  //
  // Check if volume needs to be high level formatted.
  //
  if (FS_IsHLFormatted("") == 0) {
    printf("High level formatting\n");
    FS_Format("", NULL);
  }
  printf("Running sample on \"%s\"\n", sVolName);
  v = FS_GetVolumeFreeSpace(sVolName);
  if (v <= 0x7fff) {
    printf("  Free space: %u bytes\n", v);
  } else if (v <= 0x2000000UL) {
    v >>= 10;
    printf("  Free space: %u kBytes\n", v);
  } else {
    v >>= 20;
    printf("  Free space: %u MBytes\n", v);
  }
  sprintf(ac, "%s\\File.txt", sVolName);
  printf("  Write test data to file %s\n", ac);
  pFile = FS_FOpen(ac, "w");
  if (pFile) {
    FS_Write(pFile, "Test", 4); 
    FS_FClose(pFile);
  } else {
    printf("Could not open file: %s to write.\n", ac);
  }
  v = FS_GetVolumeFreeSpace(sVolName);
  if (v <= 0x7fff) {
    printf("  Free space: %u bytes\n", v);
  } else if (v <= 0x2000000UL) {
    v >>= 10;
    printf("  Free space: %u kBytes\n", v);
  } else {
    v >>= 20;
    printf("  Free space: %u MBytes\n", v);
  }
  printf("  Finished\n");
  while(1);
}
/****** EOF *********************************************************/
