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
File        : FS_Warn.c
Purpose     : Logging (used only at higher debug levels)
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "FS_Int.h"
#include "FS_Debug.h"


/*********************************************************************
*
*       Defines
*
**********************************************************************
*/


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32  _WarnFilter = 0xFFFFFFFF;    // Per default all warning enabled

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
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_Warnf
*/
void FS_Warnf(U32 Type, const char * sFormat, ...) {
  va_list ParamList;
  char acBuffer[100];

  //
  // Filter message. If logging for this type of message is not enabled, do  nothing.
  //
  if ((Type & _WarnFilter) == 0) {
    return;
  }
  //
  // Replace place holders (%d, %x etc) by values and call output routine.
  //
  va_start(ParamList, sFormat);
  FS_PrintfSafe(acBuffer, sFormat, sizeof(acBuffer), &ParamList);
  FS_X_Warn(acBuffer);
}


/*********************************************************************
*
*       FS_SetWarnFilter
*/
void FS_SetWarnFilter(U32 FilterMask) {
  _WarnFilter = FilterMask;
}

/*********************************************************************
*
*       FS_AddWarnFilter
*/
void FS_AddWarnFilter(U32 FilterMask) {
  _WarnFilter |= FilterMask;
}

/*************************** End of file ****************************/
