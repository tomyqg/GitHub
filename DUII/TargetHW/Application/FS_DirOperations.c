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
File        : FS_DirOperations.c
Purpose     : This sample creates 3 directories. In each directory
              32 files are created.
              After creating the directories and files, the contents
              of each directory is shown.
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include <stdio.h>
#include "FS.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef COUNTOF
  #define COUNTOF(a) (sizeof(a) / sizeof(a[0]))
#endif

#define MAX_RECURSION         5
#define NUM_FILES            32
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static char _acBuffer[0x1000];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CreateFiles
*/
static void _CreateFiles(const char * sPath) {
  int       i;
  FS_FILE * pFile;
  char      acFileName[40];

  
  for (i = 0; i < NUM_FILES; i++) {
    sprintf(&acFileName[0], "%s\\file%.4d.txt", sPath, i);
    pFile = FS_FOpen(&acFileName[0], "w");
    FS_Write(pFile, &acFileName, strlen(acFileName));
    FS_FClose(pFile);
    printf(".");
  }
  printf("Ok\n");
}


/*********************************************************************
*
*       _ShowDir
*
*/
static void _ShowDir(const char * sDirName, int MaxRecursion) {
  FS_FIND_DATA fd;
  char         acFileName[20];
  char         acDummy[20];
  unsigned     NumBytes;
  char         r;
  
  NumBytes = MAX_RECURSION - MaxRecursion;
  memset(acDummy, ' ', NumBytes);
  acDummy[NumBytes] = 0;
  sprintf(_acBuffer, "%sContents of %s \n", acDummy, sDirName);
  FS_X_Log(_acBuffer);
  if (MaxRecursion) {
    r = FS_FindFirstFile(&fd, sDirName, acFileName, sizeof(acFileName));
    if (r == 0) {
      do {
        U8 Attr;

        Attr = fd.Attributes;
        sprintf(_acBuffer,"%s %s %s Attributes: %s%s%s%s Size: %lu\n", acDummy, fd.sFileName,
                            (Attr & FS_ATTR_DIRECTORY) ? "(Dir)" : "     ",
                            (Attr & FS_ATTR_ARCHIVE)   ? "A" : "-",
                            (Attr & FS_ATTR_READ_ONLY) ? "R" : "-",
                            (Attr & FS_ATTR_HIDDEN)    ? "H" : "-",
                            (Attr & FS_ATTR_SYSTEM)    ? "S" : "-",
                            fd.FileSize);
        FS_X_Log(_acBuffer);
        if (Attr & FS_ATTR_DIRECTORY) {
          char acDirName[256];
          //
          // Show contents of each directory in the root
          //
          if (*fd.sFileName != '.') {
            sprintf(acDirName, "%s\\%s", sDirName, fd.sFileName);
            _ShowDir(acDirName, MaxRecursion - 1);
          }
        }

      } while (FS_FindNextFile(&fd));
      FS_FindClose(&fd);
    } else if (r == 1) {
      FS_X_Log("Directory is empty");
    } else {
      char acErr[80];
   
      sprintf(acErr, "Unable to open directory %s\n", sDirName);
      FS_X_ErrorOut(acErr);
    }
    FS_X_Log("\n");
  }
 
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
void MainTask(void);      // Forward declaration to avoid "no prototype" warning
void MainTask(void) {
  const char * sVolumeName = "";
  int          i;
  
  FS_Init();
  FS_FormatLLIfRequired(sVolumeName);
  //
  // Check if volume needs to be high level formatted.
  //
  if (FS_IsHLFormatted(sVolumeName) == 0) {
    printf("High level formatting: %s\n", sVolumeName);
    FS_Format(sVolumeName, NULL);
  }
  //
  //  Create 3 folders 
  //
  for (i = 0; i < 3; i++) {
    char acDirName[20];
    int  r;

    sprintf(acDirName, "%s\\Dir%.2d", sVolumeName, i);
    r = FS_MkDir(acDirName);
    //
    // If directory has been succesfully created
    // Create the files in that directory.
    //
    if (r == 0) {
      _CreateFiles(acDirName);
    } else {
      char acErr[80];
   
      sprintf(acErr, "Error: Could not create the directory %s\n", acDirName);
      FS_X_ErrorOut(acErr);
    }

  }
  //
  // Show contents of root directory
  //
  _ShowDir(sVolumeName, MAX_RECURSION);
  while(1);
}


/*************************** End of file ****************************/
