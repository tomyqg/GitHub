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
File        : GUI_CursorHeaderMPx.c
Purpose     : Defines the pixels of the header cursor, medium
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public (internal) data
*
**********************************************************************
*/

GUI_CONST_STORAGE unsigned char GUI_PixelsHeaderM[5 * 17] = {
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x04, 0x01, 0x90, 0x04, 0x00,
  0x19, 0x55, 0x95, 0x59, 0x00,
  0x6a, 0xaa, 0xaa, 0xaa, 0x40,
  0x19, 0x55, 0x95, 0x59, 0x00,
  0x04, 0x01, 0x90, 0x04, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
};

/*************************** End of file ****************************/
