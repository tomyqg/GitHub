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
File        : GUI_Goto.c
Purpose     : Implementation of GUI_Goto... routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"
 
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GotoY
*/
static char _GotoY(int y) {
  GUI_Context.DispPosY = y;
  return 0;
}

/*********************************************************************
*
*       _GotoX
*/
static char _GotoX(int x) {
  GUI_Context.DispPosX = x;
  return 0;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GotoY
*/
char GUI_GotoY(int y) {
  char r;
  GUI_LOCK();
  r = _GotoY(y);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GotoX
*/
char GUI_GotoX(int x) {
  char r;
  GUI_LOCK();
  r = _GotoX(x);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GotoXY
*/
char GUI_GotoXY(int x, int y) {
  char r;
  GUI_LOCK();
  r  = _GotoX(x);
  r |= _GotoY(y);
  GUI_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
