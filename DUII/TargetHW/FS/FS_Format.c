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
File        : FS_Format.c
Purpose     : Implementation of the FS_Format API function.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/


#include "FS_Int.h"

/*********************************************************************
*
*       Public code, internal
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__Format
*
*  Function description:
*    Internal version of FS_Format.
*    Format the medium
*
*  Parameters:
*    pVolume       Volume to format. NULL is permitted, but returns an error.
*    pFormatInfo   Add. optional format information.
*  
*  Return value:
*    ==0         - File system has been started.
*    !=0         - An error has occurred.
*/
int FS__Format(FS_VOLUME  * pVolume, FS_FORMAT_INFO * pFormatInfo) {
  int          r;
  int          Status;
  FS_DEVICE  * pDevice;

  r = -1;
  if (pVolume) {
    pDevice = &pVolume->Partition.Device;
    FS_LOCK_DRIVER(pDevice);
    FS__UnmountForcedNL(pVolume);
    Status = FS_LB_GetStatus(pDevice);
    if (Status >= 0) {
      FS_LB_InitMediumIfRequired(pDevice);
      r = FS_FORMAT(pVolume, pFormatInfo);
    }
    FS_UNLOCK_DRIVER(pDevice);

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
*       FS_Format
*
*  Function description:
*    Format the medium
*
*  Parameters:
*    pDevice       Device specifier (string). "" refers to the first device.
*    pFormatInfo   Add. optional format information.
*  
*  Return value:
*    ==0         - O.K., format successful
*    !=0         - An error has occurred.
*/
int FS_Format(const char *sVolumeName, FS_FORMAT_INFO * pFormatInfo) {
  int r;
  FS_VOLUME  * pVolume;

  FS_LOCK();
  pVolume = FS__FindVolume(sVolumeName, NULL);
  r = FS__Format(pVolume, pFormatInfo);
  FS_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
