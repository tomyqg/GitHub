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
File        : GUI__strlen.c
Purpose     : Implementation of GUI__strlen
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__strlen
*
* Purpose:
*  Replacement for the strlen function. The advantage is that it can
*  be called with a NULL pointer, in which case -1 is returned.
*/
int GUI__strlen(const char GUI_UNI_PTR * s) {
  int r = -1;
  if (s) {
    do {
      r++;
    } while (*s++);
  }
  return r;
}

/*************************** End of file ****************************/
