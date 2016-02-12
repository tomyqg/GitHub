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
File        : FS_ErrorOut.C
Purpose     : Logging (used only at higher debug levels)
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "FS_Int.h"

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
static U32  _ErrorFilter = 0xFFFFFFFFUL;    // Per default all errors enabled


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_ErrorOutf
*/
void FS_ErrorOutf(U32 Type, const char * sFormat, ...) {
  va_list ParamList;
  char acBuffer[100];

  //
  // Filter message. If logging for this type of message is not enabled, do  nothing.
  //
  if ((Type & _ErrorFilter) == 0) {
    return;
  }
  //
  // Replace place holders (%d, %x etc) by values and call output routine.
  //
  va_start(ParamList, sFormat);
  FS_PrintfSafe(acBuffer, sFormat, sizeof(acBuffer), &ParamList);
  FS_X_ErrorOut(acBuffer);
}


/*********************************************************************
*
*       FS_SetErrorFilter
*/
void FS_SetErrorFilter(U32 FilterMask) {
  _ErrorFilter = FilterMask;
}

/*********************************************************************
*
*       FS_AddErrorFilter
*/
void FS_AddErrorFilter(U32 FilterMask) {
  _ErrorFilter |= FilterMask;
}

/*************************** End of file ****************************/
