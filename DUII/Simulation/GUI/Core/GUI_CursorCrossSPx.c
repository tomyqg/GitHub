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
File        : GUI_CursorCrossSPx.C
Purpose     : Defines the pixel offset of the cross cursor, small
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

GUI_CONST_STORAGE unsigned char GUI_Pixels_CrossS[33] = {
  0x00, 0x54, 0x00,
  0x00, 0x64, 0x00,
  0x00, 0x64, 0x00,
  0x00, 0x64, 0x00,
  0x55, 0x65, 0x54,
  0x6A, 0x9A, 0xA4,
  0x55, 0x65, 0x54,
  0x00, 0x64, 0x00,
  0x00, 0x64, 0x00,
  0x00, 0x64, 0x00,
  0x00, 0x54, 0x00
};

/*************************** End of file ****************************/
