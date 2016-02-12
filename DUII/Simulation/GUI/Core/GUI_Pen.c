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
File        : GUI_Pen.C
Purpose     : Getting / Setting pen attributes
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
*       GUI_GetPenSize
*/
U8 GUI_GetPenSize(void) {
  U8 r;
  GUI_LOCK();
  r = GUI_Context.PenSize;
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GetPenShape
*/
U8 GUI_GetPenShape(void) {
  U8 r;
  GUI_LOCK();
  r = GUI_Context.PenShape;
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_SetPenSize
*/
U8 GUI_SetPenSize(U8 PenSize) {
  U8 r;
  GUI_LOCK();
    r = GUI_Context.PenSize;
    GUI_Context.PenSize = PenSize;
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_SetPenShape
*/
U8 GUI_SetPenShape(U8 PenShape) {
  U8 r;
  GUI_LOCK();
    r = GUI_Context.PenShape;
    GUI_Context.PenShape = PenShape;
  GUI_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
