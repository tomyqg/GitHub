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
File        : FS_Rename.c
Purpose     : Implementation of FS_Rename
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Int.h"
#include "FS_OS.h"
#include "FS_CLib.h"

/*********************************************************************
*
*       Static code
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
*       FS_Rename
*
*  Function description:
*    Rename a file/directory.
*
*  Parameters:
*    sOldName   - Fully qualified file name.
*    sNewName   - new file name.
*
*  Return value:
*    ==0         - File has been renamed.
*    ==-1        - An error has occurred.
*/
int FS_Rename(const char * sOldName, const char * sNewName) {
  const char * s;
  int          r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  r = -1;
  /* Find correct FSL  (device:unit:name) */
  pVolume = FS__FindVolume(sOldName, &s);
  if (pVolume) {
    if (FS__AutoMount(pVolume) == FS_MOUNT_RW)  {    
      /* Call the FSL function to do the actual work */
      FS_LOCK_DRIVER(&pVolume->Partition.Device);
      FS_JOURNAL_BEGIN(pVolume);
      r = FS_RENAME(s, sNewName, pVolume);
      FS_JOURNAL_END  (pVolume);
      FS_UNLOCK_DRIVER(&pVolume->Partition.Device);
      if (r) {
        r = -1;
      }
    }
  }
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
