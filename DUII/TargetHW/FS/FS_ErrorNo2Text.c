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
File        : FS_ErrorNo2Text.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

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
*       FS_ErrorNo2Text
*
*  Description:
*    API function. Retrieves text for a given error code.
*
*  Parameters:
*    ErrCode    Error code to retrieve text for
*  
*  Return value:
*    Clear text for error code
*/
const char * FS_ErrorNo2Text(int ErrCode) {
  const char * pText;

  switch (ErrCode) {
  case FS_ERR_OK:
    pText = "No error";
    break;
  case FS_ERR_EOF:
    pText = "End of file reached";
    break;
  case FS_ERR_DISKFULL:
    pText = "Disk full";
    break;
  case FS_ERR_INVALIDPAR:
    pText = "Invalid parameter";
    break;
  case FS_ERR_CMDNOTSUPPORTED:
    pText = "Command not supported";
    break;
  case FS_ERR_WRITEONLY:
    pText = "File is write only";
    break;
  case FS_ERR_READONLY:
    pText = "File is read only";
    break;
  case FS_ERR_READERROR:
    pText = "Read error";
    break;
  case FS_ERR_WRITEERROR:
    pText = "Write error";
    break;
  default:
    pText = "Unknown error";
    break;
  }

  return pText;
}

/*********************************************************************
*
*       FS_CheckDisk_ErrCode2Text
*
*  Function description:
*    Retrieves text for a given checkdisk error code.
*
*  Parameters:
*    ErrCode    Error code to retrieve text for.
*  
*  Return value:
*    Clear text for error code
*/
const char * FS_CheckDisk_ErrCode2Text(int ErrCode) {
  const char * sFormat = NULL;
  switch (ErrCode) {
  case FS_ERRCODE_0FILE:                   sFormat = "Cluster chain starting on cluster %d assigned to file of size zero"; break;
  case FS_ERRCODE_SHORTEN_CLUSTER:         sFormat = "Need to shorten cluster chain on cluster %d"; break;
  case FS_ERRCODE_CROSSLINKED_CLUSTER:     sFormat = "Cluster %d is cross-linked (used for multiple files / directories) FileId: %d:%d"; break;
  case FS_ERRCODE_FEW_CLUSTER:             sFormat = "To few clusters allocated to file."; break;
  case FS_ERRCODE_CLUSTER_UNUSED:          sFormat = "Cluster %d is marked as used, but not assigned to a file or directory."; break;
  case FS_ERRCODE_CLUSTER_NOT_EOC:         sFormat = "Cluster %d is not marked as end-of-chain."; break;
  case FS_ERRCODE_INVALID_CLUSTER:         sFormat = "Cluster %d is not a valid cluster"; break;
  case FS_ERRCODE_INVALID_DIRECTORY_ENTRY: sFormat = "Invalid director entry found"; break;
  default:                                 sFormat = "Unknown error"; break;
  }
  return sFormat;
}



/*************************** End of file ****************************/
