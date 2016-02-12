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
File        : GUI_AddBin.c
Purpose     : Converts binary value 2 string
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AddBin
*/
void GUI_AddBin(U32 v, U8 Len, char** ps) {
  char *s = *ps;
#if GUI_DEBUG_LEVEL >1
  if (Len > 32) {
    GUI_DEBUG_WARN("Can not display more than 32 bin. digits");
    Len = 32;
  }
#endif
  (*ps) += Len;
  **ps   = '\0';     /* Make sure string is 0-terminated */
  while(Len--) {
    *(s + Len) = (char)('0' + (v & 1));
    v >>= 1;
  }
}

/*************************** End of file ****************************/
