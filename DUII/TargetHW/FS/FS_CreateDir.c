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
File        : FS_CreateDir.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include <stdlib.h>

#include "FS_Int.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/




/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__CreateDir
*
*  Description:
*    Internal version of FS__CreateDir.
*    Creates a directory, directory path - if a directory does not 
*    exist in the directory path, the directory is created.
*
*  Parameters:
*    sDir    - Fully qualified directory path to create.
*
*  Return value:
*    ==  0        - Directory path has been created.
*    ==  1        - Directory path already exists.
*    == -1        - Directory path cannot be created.
*    == -2        - Directory path is too long to parse.
*/
int FS__CreateDir(const char * sDir) {
  int          r;
  FS_VOLUME  * pVolume;
  const char * sDirPath;

  r = -1;
  //
  //  Find correct volume
  //
  pVolume = FS__FindVolume(sDir, &sDirPath);
  if (*sDirPath != 0) {
    if (pVolume) {
      //
      // Mount the volume if necessary
      //
      if (FS__AutoMount(pVolume) == FS_MOUNT_RW) {
        //
        //  Check if creating directory recursive is allowed
        //
        FS_FIND_DATA fd;
        const char * sNextDirName;
        int          NumChars;
        int          DestLen;
        int          SrcLen;
        char         acDestPath[260];  // Allow up to 258 bytes for the path

        acDestPath[0]   = 0;
        DestLen         = 0;
        SrcLen          = FS_STRLEN(sDirPath);
        if (SrcLen > (int)sizeof(acDestPath)) {
          FS_DEBUG_ERROROUT((FS_MTYPE_API, "FS__CreateDirEx, DirPath is too long.\n"));
          return -2;
        }
        //
        // Parse the directory path
        //
        do {
          //
          // For each FS_DIRECTORY_DELIMITER in string
          // check the directory by opening it.
          //
          sNextDirName = FS__strchr(sDirPath, FS_DIRECTORY_DELIMITER);
          if (sNextDirName) {
            NumChars = sNextDirName - sDirPath;
          } else if (SrcLen > 0) {
            NumChars = SrcLen;
          } else {
            break;
          }
          FS_STRNCAT(acDestPath, sDirPath, NumChars);
          //
          // Open a handle to the directory
          //
          if (FS__FindFirstFileEx(&fd, pVolume, acDestPath, NULL, 0) == -1) {
            //
            //  Create the directory
            //
            FS_LOCK_DRIVER(&pVolume->Partition.Device);
            r = FS_CREATEDIR(pVolume, acDestPath);
            FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
            if (r) {
              break;
            }
          } else {
            //
            //  Close the handle to the opened directory
            //
            FS__FindClose(&fd);
            r = 1;
          }
          //
          //  Update length of the strings
          // 
          DestLen += NumChars;
          SrcLen  -= NumChars;
          if (DestLen) {
            //
            // Append the delimiter to the 
            acDestPath[DestLen++] = FS_DIRECTORY_DELIMITER;
            acDestPath[DestLen]   = 0;
          }
          SrcLen--;
          if (sNextDirName) {
            sDirPath = sNextDirName + 1;
          } 
        } while (1);
      }
    }
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
*       FS_CreateDir
*
*  Description:
*    API function.
*    Creates a directory, directory path - if a directory does not 
*    exist in the directory path, the directory is created.
*
*  Parameters:
*    sDirPath    - Fully qualified directory name.
*
*  Return value:
*    ==  0        - Directory path has been created.
*    ==  1        - Directory path already exists.
*    == -1        - Directory path cannot be created.
*    == -2        - Directory path is too long to parse.
*/
int FS_CreateDir(const char * sDirPath) {
  int  r;
  FS_LOCK();
  r = FS__CreateDir(sDirPath);
  FS_UNLOCK();
  return r;
}


/*************************** End of file ****************************/
