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
File        : GUI_AddHex.c
Purpose     : Converts hex value 2 string
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static const char acHex[16] =  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AddHex
*/
void GUI_AddHex(U32 v, U8 Len, char**ps) {
  char *s = *ps;
  if (Len > 8) {
    return;
  }
  (*ps) += Len;
  **ps   = '\0';     /* Make sure string is 0-terminated */
  while(Len--) {
    *(s + Len) = acHex[v & 15];
    v >>= 4;
  }
}

/*************************** End of file ****************************/
