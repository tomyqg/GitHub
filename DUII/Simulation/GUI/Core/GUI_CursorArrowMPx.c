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
File        : GUI_CursorArrowMPx.C
Purpose     : Defines the pixel offset of the arrow cursor, medium
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

GUI_CONST_STORAGE unsigned char GUI_Pixels_ArrowM[60] = {
  0x40, 0x00, 0x00,
  0x50, 0x00, 0x00,
  0x64, 0x00, 0x00,
  0x69, 0x00, 0x00,
  0x6A, 0x40, 0x00,
  0x6A, 0x90, 0x00,
  0x6A, 0xA4, 0x00,
  0x6A, 0xA9, 0x00,
  0x6A, 0xAA, 0x40,
  0x6A, 0xAA, 0x90,
  0x6A, 0xAA, 0xA4,
  0x6A, 0xA9, 0x55,
  0x69, 0xA9, 0x00,
  0x64, 0x6A, 0x40,
  0x50, 0x6A, 0x40,
  0x40, 0x1A, 0x90,
  0x00, 0x1A, 0x90,
  0x00, 0x06, 0xA4,
  0x00, 0x06, 0xA4,
  0x00, 0x01, 0x50
};

/*************************** End of file ****************************/
