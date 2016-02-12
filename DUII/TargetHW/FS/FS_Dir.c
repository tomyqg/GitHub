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
File        : FS_Dir.c
Purpose     : Directory support functions
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
*       Public code, internal version of API functions
*
**********************************************************************
*/
/*********************************************************************
*
*       FS__MkDir
*
*  Function description
*    Internal version of FS_MkDir.
*    Creates a directory.
*
*  Parameters
*    pDirName    - Fully qualified directory name.
*
*  Return value
*    ==0         - Directory has been created.
*    ==-1        - An error has occurred.
*/
int FS__MkDir(const char * pDirName) {
  int          r;
  FS_VOLUME  * pVolume;
  const char * s;
  r = -1;
  /* Find correct FSL (device:unit:name) */
  pVolume = FS__FindVolume(pDirName, &s);
  if (pVolume) {
    if (FS__AutoMount(pVolume) == FS_MOUNT_RW) {
      /* Execute the FSL function */
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      FS_JOURNAL_BEGIN(pVolume);
      r = FS_CREATEDIR(pVolume, s);
      FS_JOURNAL_END  (pVolume);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    }
  }
  return r;
}


/*********************************************************************
*
*       FS__RmDir
*
*  Function description
*    Internal version of FS_RmDir.
*    Removes a directory. if it is an empty directory.
*
*  Parameters
*    pDirName    - Fully qualified directory name.
*
*  Return value
*    ==0         - Directory has been removed.
*    ==-1        - An error has occurred.
*/
int FS__RmDir(const char * sDirName) {
  FS_FIND_DATA   fd;
  FS_VOLUME    * pVolume;
  int            i;
  const char   * s;
  char           c;

  //
  // Check if the directory exists.
  //
  i = FS__FindFirstFile(&fd, sDirName, &c, 1);
  FS_USE_PARA(c);  // 'c' is not really used.
  if (i == 0) {
    //
    // Check if directory is empty
    //
    i = 0;
    do {
      i++;
      if (i > 2) { /* if is more than '..' and '.' */
        //    There is more than '..' and '.' in the directory, so you
        //    must not delete it.
        FS__FindClose(&fd);
        return -1;
      }
    } while (FS__FindNextFile(&fd));
    FS__FindClose(&fd);
    //
    // Get the correct pVolume instance
    //
    pVolume = FS__FindVolume(sDirName, &s);
    if (pVolume) {
      //
      //  Remove the directory
      //
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      FS_JOURNAL_BEGIN(pVolume);
      i = FS_REMOVEDIR(pVolume, s);
      FS_JOURNAL_END(pVolume);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
    } else {
      i = -1;
    }
  } else {
    i = -1;  // Error, directory not found
  }
  return i;
}

/*********************************************************************
*
*       FS__FindFirstFileEx
*
*  Function description
*    This function initializes the pfd structure with the information
*    necessary to open a directory for listing.
*    It also retrieves the first entry in the given directory.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure.
*    pVolume        - Pointer to a FS_VOLUME structure.
*    sPath          - relative path to the directory on the volume.
*    sFilename      - Pointer to a buffer for storing the file name.
*    sizeofFilename - size in bytes of the sFilename buffer.
*
*  Return value
*    ==0   - O.K., first file found
*    ==1   - No entries available in directory
*    < 0   - Error
*/
int FS__FindFirstFileEx(FS_FIND_DATA * pfd, FS_VOLUME * pVolume, const char * sPath, char * sFilename, int sizeofFilename) {
  int          r;

  r       = -1;  // Set as error so far
  //
  // Mount the volume if necessary
  //
  if (FS__AutoMount(pVolume) & FS_MOUNT_R) {
    FS__DIR          * pDir;
    FS_DIRENTRY_INFO   DirEntryInfo;
  
    FS_MEMSET(pfd, 0, sizeof(FS_FIND_DATA));
    FS_MEMSET(&DirEntryInfo, 0, sizeof(FS_DIRENTRY_INFO));
    DirEntryInfo.sFileName      = sFilename;
    DirEntryInfo.SizeofFileName = sizeofFilename;
    pfd->Dir.pVolume            = pVolume;
    pDir                        = &pfd->Dir;
    FS_LOCK_DRIVER(&pVolume->Partition.Device);
    if (pVolume->IsMounted) {
      if (FS_OPENDIR(sPath, pDir) == 0) {
        if (FS_READDIR(pDir, &DirEntryInfo) == 0) {
          pfd->Attributes      = DirEntryInfo.Attributes;
          pfd->CreationTime    = DirEntryInfo.CreationTime;
          pfd->FileSize        = DirEntryInfo.FileSize;
          pfd->LastAccessTime  = DirEntryInfo.LastAccessTime;
          pfd->LastWriteTime   = DirEntryInfo.LastWriteTime;
          pfd->sFileName       = DirEntryInfo.sFileName;
          pfd->SizeofFileName  = DirEntryInfo.SizeofFileName;
          r = 0;
        } else {
          r = 1;
        }
      }
    } else {
      FS_DEBUG_ERROROUT((FS_MTYPE_API, "Application error: Volume has been unmounted by another thread.\n"));
    }
    FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
  }
  return r;
}

/*********************************************************************
*
*       FS__FindFirstFile
*
*  Function description
*    This function calls the FS__FindFirstFileEx function to
*    open a directory for directory listing.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure
*    sPath          - Path to the directory that 
*    sFilename      - Pointer to a buffer for storing the file name
*    sizeofFilename - size in bytes of the sFilename buffer.
*
*  Return value
*    ==0   - O.K.
*    ==1   - No entries available in directory
*    !=0   - Error
*/
int FS__FindFirstFile(FS_FIND_DATA * pfd, const char * sPath, char * sFilename, int sizeofFilename) {
  int          r;
  FS_VOLUME  * pVolume;
  const char * s;


  if (sFilename == (char * )NULL || (sizeofFilename == 0)) {
    return -1;   // Error, no valid buffer specified for the 
  }
  r       = -1;  // Set as error so far
  pVolume = FS__FindVolume(sPath, &s);
  if (pVolume) {
    r = FS__FindFirstFileEx(pfd, pVolume, s, sFilename, sizeofFilename);
  }
  return r;
}

/*********************************************************************
*
*       FS__FindNextFile
*
*  Function description
*    Finds the next directory entry in the directory specified by pfd.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure
*
*  Return value
*    == 1   - O.K.
*    == 0   - Error or no more items found.
*/
int FS__FindNextFile(FS_FIND_DATA * pfd) {
  int                r;
  FS__DIR          * pDir;
  FS_DIRENTRY_INFO   DirEntryInfo;


  r = 0;
  FS_MEMSET(&DirEntryInfo, 0, sizeof(FS_DIRENTRY_INFO));
  DirEntryInfo.sFileName      = pfd->sFileName;
  DirEntryInfo.SizeofFileName = pfd->SizeofFileName;

  FS_LOCK_SYS();
  pDir = &pfd->Dir;
  FS_UNLOCK_SYS();
  FS_LOCK_DRIVER(&pDir->pVolume->Partition.Device);
  if (pDir->pVolume->IsMounted) {
    r    = FS_READDIR(pDir, &DirEntryInfo);
    if (r == 0) {
      pfd->Attributes      = DirEntryInfo.Attributes;
      pfd->CreationTime    = DirEntryInfo.CreationTime;
      pfd->FileSize        = DirEntryInfo.FileSize;
      pfd->LastAccessTime  = DirEntryInfo.LastAccessTime;
      pfd->LastWriteTime   = DirEntryInfo.LastWriteTime;
      pfd->sFileName       = DirEntryInfo.sFileName;
      r = 1;
    } else {
      r = 0;
    }
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_API, "Application error: Volume has been unmounted by another thread.\n"));
  }
  FS_UNLOCK_DRIVER(&pDir->pVolume->Partition.Device);
  return r;
}


/*********************************************************************
*
*       FS__FindClose
*
*  Function description
*    Closes the FS_FIND_DATA structure.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure
*
*/
void FS__FindClose(FS_FIND_DATA * pfd) {
  FS_USE_PARA(pfd);
  FS_LOCK_SYS();
  pfd->Dir.pVolume = NULL;
  FS_UNLOCK_SYS();
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FS_MkDir
*
*  Function description
*   Create a directory.
*
*  Parameters
*    pDirName    - Fully qualified directory name.
*
*  Return value
*    ==0         - Directory has been created.
*    ==-1        - An error has occurred.
*/
int FS_MkDir(const char * pDirName) {
  int  r;
  FS_LOCK();
  r = FS__MkDir(pDirName);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_RmDir
*
*  Function description
*    API function. Remove a directory.
*
*  Parameters
*    pDirName    - Fully qualified directory name.
*
*  Return value
*    ==0         - Directory has been removed.
*    ==-1        - An error has occurred.
*/
int FS_RmDir(const char * pDirName) {
  int    r;

  FS_LOCK();
  r = FS__RmDir(pDirName);
  FS_UNLOCK();
  return r;
}

/*********************************************************************
*
*       FS_FindFirstFile
*
*  Function description
*    This function will call the internal function, 
*    which will do the following:
*    * Initialize the pfd structure with the information necessary to open a directory for listing.
*    * Retrieves the first entry in the given directory if available.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure
*    sPath          - Path to the directory that 
*    sFilename      - Pointer to a buffer for storing the file name
*    sizeofFilename - size in bytes of the sFilename buffer.
*
*  Return value
*    ==0   - O.K.
*    ==1   - No entries available in directory
*    !=0   - Error
*/
char FS_FindFirstFile(FS_FIND_DATA * pfd, const char * sPath, char * sFilename, int sizeofFilename) {
  int r;
  FS_LOCK();
  r = FS__FindFirstFile(pfd, sPath, sFilename, sizeofFilename);
  FS_UNLOCK();
  return r;
}


/*********************************************************************
*
*       FS_FindNextFile
*
*  Function description
*    Finds the next directory entry in the directory specified by pfd.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure
*
*  Return value
*    ==1   - O.K.
*    ==0   - Error
*/
char FS_FindNextFile(FS_FIND_DATA * pfd) {
  int r;
  FS_LOCK();
  r = FS__FindNextFile(pfd);
  FS_UNLOCK();
  return r;
}


/*********************************************************************
*
*       FS_FindClose
*
*  Function description
*    Closes the FS_FIND_DATA structure.
*
*  Parameters
*    pfd            - Pointer to a FS_FIND_DATA structure
*
*/
void FS_FindClose(FS_FIND_DATA * pfd) {
  FS_LOCK();
  FS__FindClose(pfd);
  FS_UNLOCK();
}

/*************************** End of file ****************************/
