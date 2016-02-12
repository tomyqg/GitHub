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
File        : GUI__HandleEOLine.C
Purpose     : Implementation of optional routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       GUI__HandleEOLine
*
* Is called when processing strings which may consist of
* multiple lines after a line has been processed. It will
* a) Swall the line feed character (if it is there)
* b) Return 1 if end of string, otherwise 0
*/
int GUI__HandleEOLine(const char GUI_UNI_PTR **ps) {
  const char GUI_UNI_PTR *s = *ps;
  char c = *s++;
  if (c == 0) {
    return 1;
  }
  if (c == '\n') {
    *ps = s;
  }
  return 0;
}

/*************************** End of file ****************************/
