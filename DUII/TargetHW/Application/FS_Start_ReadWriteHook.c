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
File    : FS_Start_ReadWriteHook.c
Purpose : Start application for file system.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "FS.h"
#include "FS_Int.h"


/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int          Type;
  const char * s;
} TYPE_DESC;


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static char ac[511];

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
static const TYPE_DESC _aSectorType[] = {
  { FS_SECTOR_TYPE_DATA, "DATA" },
  { FS_SECTOR_TYPE_MAN,  "MAN " },
  { FS_SECTOR_TYPE_DIR,  "DIR " },
};

static const TYPE_DESC _aOperationType[] = {
  { FS_OPERATION_READ,   "Read  " },
  { FS_OPERATION_WRITE,  "Write " },
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _Num2Name
*/
static const char * _Num2Name(int Type, const TYPE_DESC * pDesc, unsigned NumItems) {
  unsigned i;
  for (i = 0; i < NumItems; i++) {
    if (pDesc->Type == Type) {
      return pDesc->s;
    }
    pDesc++;
  }
  return "Unknown Type";
}

/*********************************************************************
*
*       _OnDeviceActivity
*/
static void _OnDeviceActivity(FS_DEVICE * pDevice, unsigned Operation, U32 StartSector, U32 NumSectors, int SectorType) {
  const char * sOperation;
  const char * sSectorType;

  sOperation  = _Num2Name(Operation, &_aOperationType[0], COUNTOF(_aOperationType));
  sSectorType = _Num2Name(SectorType, &_aSectorType[0], COUNTOF(_aSectorType));

  printf("  %s: StartSector: 0x%08x, NumSectors: 0x%08x, SectorType: %s \n", sOperation, StartSector, NumSectors, sSectorType);

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
  FS_FILE    * pFile;
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
  FS_SetOnDeviceActivityHook(sVolName, _OnDeviceActivity);
  printf("Running sample on %s\n", sVolName);
  sprintf(ac, "%s\\File.txt", sVolName);
  //
  // Open the file
  //
  printf("Open/create file\n");
  pFile = FS_FOpen(ac, "w");
  if (pFile) {
    //
    // 1st write to file.
    //
    printf("1st Write (4 bytes)to file\n");
    FS_Write(pFile, "Test", 4); 
    //
    // 2nd write to file 511 bytes.
    //
    printf("2nd write (511 bytes) to file.\n");
    FS_Write(pFile, ac, sizeof(ac)); 
    printf("Close file\n");
    //
    // Close the file.
    //
    FS_FClose(pFile);
  } else {
    printf("Could not open file: %s to write.\n", ac);
  }
  printf("Finished\n");
  while(1);
}
/****** EOF *********************************************************/
