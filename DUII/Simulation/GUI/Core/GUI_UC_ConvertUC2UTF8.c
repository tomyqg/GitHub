/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2010  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.06 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_UC_ConvertUC2UTF8.c
Purpose     : Converts a UNICODE string to UTF8
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_UC_ConvertUC2UTF8
*/
int GUI_UC_ConvertUC2UTF8(const U16 GUI_UNI_PTR * s, int Len, char * pBuffer, int BufferSize) {
  int LenDest;
  LenDest = 0;
  GUI_LOCK();
  while (Len--) {
    int NumBytes;
    NumBytes = GUI_UC_Encode(pBuffer, *s++);
    if ((LenDest + NumBytes) > BufferSize) {
      break;
    }
    pBuffer += NumBytes;
    LenDest += NumBytes;
  }
  GUI_UNLOCK();
  return LenDest;
}

/*************************** End of file ****************************/
