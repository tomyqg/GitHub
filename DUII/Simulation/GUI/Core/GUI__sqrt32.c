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
File        : GUI__sqrt32.c
Purpose     : Calculates the square root
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
*       GUI__sqrt32
*/
I32 GUI__sqrt32(I32 Square) {
  U32 Mask, Value, NewRoot, Root;

  Root  = 0;
  Mask  = (U32)0x40000000;
  Value = (U32)Square;
  do {
    NewRoot = Root + Mask;
    if (NewRoot <= Value) {
      Value -= NewRoot;
      Root   = NewRoot + Mask;
    }
    Root >>= 1;
    Mask >>= 2;
  } while (Mask != 0);
  return (I32)Root;
}

/*************************** End of file ****************************/
