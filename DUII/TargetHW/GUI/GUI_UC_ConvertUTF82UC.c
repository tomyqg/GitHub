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
File        : GUI_UC_ConvertUTF82UC.c
Purpose     : Converts a UTF8 string to UNICODE
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
*       GUI_UC_ConvertUTF82UC
*/
int GUI_UC_ConvertUTF82UC(const char GUI_UNI_PTR * s, int Len, U16 * pBuffer, int BufferSize) {
  int LenDest;
  LenDest = 0;
  GUI_LOCK();
  while (Len) {
    int NumBytes;
    NumBytes = GUI_UC_GetCharSize(s);
    *(pBuffer++) = GUI_UC_GetCharCode(s);
    s += NumBytes;
    LenDest++;
    if (LenDest >= BufferSize) {
      break;
    }
    Len -= NumBytes;
  }
  GUI_UNLOCK();
  return LenDest;
}

/*************************** End of file ****************************/
