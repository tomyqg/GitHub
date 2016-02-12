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
File        : FS_APIValidation.c
Purpose     : Sample program showing a simple generic API test.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "FS.h"
#include "FS_Int.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define NUM_FILES     40
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8 _aBuffer[4096];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       _Log
*/
static void _Log(const char * s) {
  printf(s);
}

/*********************************************************************
*
*       _ErrorOut
*/
static void _ErrorOut(const char * s) {
  printf(s);
  while(1);
}

/*********************************************************************
*
*       _FillBuffer
*
*/
static void _FillBuffer(void) {
  unsigned i;
  for (i = 0; i < sizeof(_aBuffer); i++) {
    _aBuffer[i] = i & 0xff;
  }
}

/*********************************************************************
*
*       _WriteFile
*
*   Description
*     Creates a file and writes the data to the file.
*
*
*  Return value
*    0   - OK.
*    1   - Error, file could either not be created/written.
*/
static int _WriteFile(const char * sFileName, const char * sType, const void * pData, unsigned NumBytes) {
  FS_FILE * pFile;
  U32       NumBytesWritten;
  int       r;

  r = 1;
  pFile = FS_FOpen(sFileName, sType);
  if (pFile) {
    NumBytesWritten = FS_Write(pFile, pData, NumBytes);
    if (NumBytesWritten == NumBytes) {
      r  = 0;
    }
    FS_FClose(pFile);
  }
  return r;
}

/*********************************************************************
*
*       _WriteFile
*
*   Description
*     opens a file and read the data from the file.
*
*
*  Return value
*    0   - OK.
*    1   - Error, file could not be found or less data read than expected.
*/
static int _ReadFile(const char * sFileName, const char * sType, void * pData, unsigned NumBytes) {
  FS_FILE * pFile;
  U32       NumBytesRead;
  int       r;

  r = 1;
  pFile = FS_FOpen(sFileName, sType);
  if (pFile) {
    NumBytesRead = FS_Read(pFile, pData, NumBytes);
    if (NumBytesRead == NumBytes) {
      r  = 0;
    }
    FS_FClose(pFile);
  }
  return r;
}
/*********************************************************************
*
*       _FileExist
*
*  Description
*    Tries to open a file. When we receive a valid file handle,
*    we know that the file does exist.
*
*  Return value
*    1   - File exists.
*    0   - File does not exist.
*/
static int _FileExist(const char * sFileName) {
  FS_FILE * pFile;
  int       r;

  r = 0;
  //
  // Open the file if it exists or not.
  //
  pFile = FS_FOpen(sFileName, "r");
  if (pFile) {
    //
    // File exists, close the opened file handle.
    //
    FS_FClose(pFile);
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       _FormatTest
*
*  Description
*    Performs a high level format and checks for success.
*
*/
static void _FormatTest(void)  {
  _Log("High level formatting volume...");
  if (FS_Format("", NULL)) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n\n");
}

/*********************************************************************
*
*       _FileOpenModeTest
*
*  Description
*    Test different modes for opening a file.
*
*/
static void _FileOpenModeTest(void) {
  int          r;
  const char * sString;
  unsigned     NumBytes;
  //
  //  Mode "w"
  //
  _Log("Test FS_FOpen modes\n");
  _Log("Mode 'w'...");
  //
  //  Check if "w" can write a file and if we
  //  can read back its contents
  //
  sString = "1234567890";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "w", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Read verification failure\n");
  }
  if (FS_MEMCMP(_aBuffer, "1234567890", NumBytes) !=0) {
    _ErrorOut("Read verification failure\n");
  }
  //
  //  Mode "w" Test overwrite
  //
  sString = "abcde";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "w", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  if (FS_MEMCMP(_aBuffer, "abcde", NumBytes) != 0) {
    _ErrorOut("Failed\n");
  }
  _Log("OK\n");
  //
  //
  // Mode "a"
  //
  _Log("Mode 'a'...");
  //
  // Check if "a" can append to file
  //
  sString = "fghij";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "a", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  if (FS_MEMCMP(_aBuffer, "abcdefghij", NumBytes) != 0) {
    _ErrorOut("Failed\n");
  }
  //
  // Check if "a" can create a file
  //
  r = FS_Remove("test.txt");
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  sString = "12345";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "a", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  if (FS_MEMCMP(_aBuffer, "12345", NumBytes) != 0) {
    _ErrorOut("Failed\n");
  }
  _Log("OK\n");
  //
  // Mode "r+"
  //
  _Log("Mode 'r+'...");
  //
  // Check if partial overwrite is possible.
  //
  sString = "ab";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "r+", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r+", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Too few bytes read\n");
  }
  if (FS_MEMCMP(_aBuffer, "ab345", NumBytes) != 0) {
    _ErrorOut("Failed\n");
  }
  //
  // Verify "r+" cannot create a file.
  //
  r = FS_Remove("test.txt");
  if (r != 0) {
    _ErrorOut("Could not remove file\n");
  }
  sString = "dummytext";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "r+", sString, NumBytes);
  if (r == 0) {
    _ErrorOut("r+ did not report an error (Creating file ?)\n");
  }
  _Log("OK\n");
  //
  // Mode "w+"
  //
  _Log("Mode 'w+'...");
  //
  // Check if file is truncated to zero
  //
  sString = "12345";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "w", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  sString = "12";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "w+", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  if (FS_MEMCMP(_aBuffer, "12", NumBytes) != 0) {
    _ErrorOut("Failed\n");
  }
  _Log("OK\n");
  //
  // Mode "a+"
  //
  _Log("Mode 'a+'...");
  //
  // Check if append works
  //
  sString = "34567890";
  NumBytes = strlen(sString);
  r = _WriteFile("test.txt", "a+", sString, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  r = _ReadFile("test.txt", "r", _aBuffer, NumBytes);
  if (r != 0) {
    _ErrorOut("Failed\n");
  }
  if (FS_MEMCMP(_aBuffer, "1234567890", NumBytes) != 0) {
    _ErrorOut("Failed\n");
  }
  _Log("OK\n");
  if (FS_Remove("test.txt")) {
    _Log("Failed to remove file\n");
  }
}

/*********************************************************************
*
*       _FileAPITest
*
*  Description
*    Test different modes for opening a file.
*/
static void _FileAPITest(void)  {
  FS_FILE    * pFile;
  U32          NumBytes;
  int          i;

  //
  //  1. Create a file and write some bytes to it.
  //
  _Log("Simple file test\n");
  _Log("Creating file...");
  pFile = FS_FOpen("File.txt","w+");
  if (pFile == (FS_FILE *)NULL) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  _Log("Write some data...");
  NumBytes = FS_Write(pFile, "File.txt", 8);
  if (NumBytes != 8) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  2. Verify that all data have been written and are correct.
  //
  _Log("Check file pos...");
  //
  // Seek to Pos. 0
  //
  FS_FSeek(pFile, 0, FS_SEEK_SET);
  if (FS_FTell(pFile)) {
    _ErrorOut("Failed\n");
  }
  _Log(".");
  //
  // Seek to Pos. 4 from beginning of the file
  //
  FS_FSeek(pFile, 4, FS_SEEK_SET);
  if (FS_FTell(pFile) != 4) {
    _ErrorOut("Failed\n");
  }
  _Log(".");
  //
  // Seek to Pos. 200 from beginning of the file
  //
  FS_FSeek(pFile, 200, FS_SEEK_SET);
  if (FS_FTell(pFile) != 200) {
    _ErrorOut("Failed\n");
  }
  //
  // Seek to end of file
  //
  FS_FSeek(pFile, 0, FS_SEEK_END);
  if (FS_FTell(pFile) != 8) {
    _ErrorOut("Failed\n");
  }
  _Log(".");
  //
  //  Check FS_FEof reports that we have reached end of file.
  //
  if (FS_FEof(pFile) == 0) {
    _ErrorOut("Failed\n");
  }
  _Log(".");
  _Log("Ok\n");
  //
  //  2. Verify that all data have been written and are correct.
  //
  _Log("Read written data back...");
  FS_FSeek(pFile, 0, FS_SEEK_SET);
  NumBytes = FS_Read(pFile, _aBuffer, 8);
  if ((NumBytes != 8) || (FS_MEMCMP(_aBuffer, "File.txt", 8))) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  3. Write data to file using burst
  //
  _Log("Write Burst test...");
  _FillBuffer();
  NumBytes = FS_Write(pFile, _aBuffer, sizeof(_aBuffer));
  if (NumBytes != sizeof(_aBuffer)) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  4. Read data from file using burst
  //
  _Log("Read Burst test...");
  FS_FSeek(pFile, 0, FS_SEEK_SET);
  NumBytes = FS_Read(pFile, _aBuffer, sizeof(_aBuffer));
  if (NumBytes != sizeof(_aBuffer)) {
    _ErrorOut("Failed\n");
  }
  if (FS_Remove("File.txt")) {
    _Log("Failed to remove file\n");
  }
  _Log("Ok\n");
  //
  //  4. Create a bunch of files
  //
  _Log("Creating a lot of file in root directory");
  for (i = 0; i < NUM_FILES; i++) {
    char acFileName[20];

    sprintf(acFileName, "file%d.txt", i);
    if (_WriteFile(acFileName, "w", _aBuffer, sizeof(_aBuffer)) == 0) {
      _Log(".");
    } else {
      _ErrorOut("Failed!\n");
    }
  }
  //
  // Remove all files that have been created
  //
  for (i = 0; i < NUM_FILES; i++) {
    char acFileName[20];

    sprintf(acFileName, "file%d.txt", i);
    if (FS_Remove(acFileName)) {
      _Log("Failed to remove file\n");
    }
  }
  _Log("Ok\n");
  _FileOpenModeTest();
  _Log("\n");
}


/*********************************************************************
*
*       _DirAPITestTest
*/
static void _DirAPITestTest(void)  {
  FS_FIND_DATA fd;
  int          r;
  unsigned     i;

  _Log("Directory API test\n");
  _Log("Checking FS_MkDir()...");
  if (FS_MkDir("SubDir")) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  _Log("Checking FS_CreateDir()...");
  if (FS_CreateDir("SubDir1\\SubDir2\\SubDir3") < 0) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  // Verify that FS_FindFirstFile()//FS_FindNextFile()
  // gets the correct number of entries available.
  //
  _Log("Checking FS_FindFirstFile()/FS_FindNextFile...");
  r = FS_FindFirstFile(&fd, "", (char*)_aBuffer, sizeof(_aBuffer));
  if (r == 0) {
    i = 1;
    do {
      i++;
    } while (FS_FindNextFile(&fd));
    if (i > 3) {
     _ErrorOut("Failed\n");
    }
  } else {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  _Log("Checking FS_RmDir()...");
  //
  // Remove an empty directory
  //
  if (FS_RmDir("SubDir")) {
    _ErrorOut("Failed\n");
  }
  _Log(".");
  //
  // Remove a directory that is not empty, FS_RmDir should indicate an error
  //
  if (FS_RmDir("SubDir1") == 0) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n\n");
}

/*********************************************************************
*
*       _PrepareExtTest
*
*  Return value
*    0   - OK.
*    1   - Error, preparation Failed.
*/
static int _PrepareExtTest(void)  {
  int r;

  _FillBuffer();
  //
  // Create a file in root directory
  //
  r = _WriteFile("file.txt", "w", _aBuffer, sizeof(_aBuffer));
  if (r == 0) {
    //
    //  Create a directory in root directory
    //
    r = FS_MkDir("SubDir");
  }
  return r;
}

/*********************************************************************
*
*       _ExtendedAPITest
*/
static void _ExtendedAPITest(void)  {
  FS_FILE * pFile;

  _Log("Extended API test\n");
  _Log("Preparing test...");
  if (_PrepareExtTest()) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  Extended file operations
  //
  _Log("Checking FS_CopyFile()...");
  if (FS_CopyFile("file.txt", "copy.txt")) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  _Log("Checking FS_Move()...");
  FS_Move("File.txt", "SubDir\\File.txt");
  if ((_FileExist("file.txt")) && (_FileExist("SubDir\\File.txt")) == 0) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  Check deleting a file
  //
  _Log("Checking FS_Remove()...");
  FS_Remove     ("copy.txt");
  if (_FileExist("copy.txt")) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  Renaming a file
  //
  _Log("Checking FS_Rename()...");
  FS_Rename("SubDir\\File.txt", "Data.txt");
  if ((_FileExist("SubDir\\File.txt")) && ((_FileExist("SubDir\\data.txt")) == 0)) {
    _ErrorOut("Failed\n");
  }
  _Log("Ok\n");
  //
  //  Verify file
  //
  pFile = FS_FOpen ("SubDir\\Data.txt", "r");
  if (FS_Verify(pFile, _aBuffer, sizeof(_aBuffer))) {
    _ErrorOut("Failed\n");
  }
  FS_FClose(pFile);
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
  //
  // Initialize file system
  //
  FS_Init();
  //
  // Check if low-level format is required
  //
  FS_FormatLLIfRequired("");
  _FormatTest();
  //
  //
  //
  _FileAPITest();
  //
  //
  //
  _DirAPITestTest();
  //
  //
  //
  _ExtendedAPITest();
  printf("Finished\n");
  while(1);
}

/****** EOF *********************************************************/
