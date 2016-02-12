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
File        : GUICC_M888_Index2Color.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Public code,
*
*       LCD_FIXEDPALETTE == 888, 4294967296 colors, BBBBBBBBGGGGGGGGRRRRRRRR
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_Index2Color_M888
*/
LCD_COLOR LCD_Index2Color_M888(unsigned Index) {
  unsigned r, g, b;
  b =  Index & 0xff;
  g = (Index & 0xff00) >> 8;
  r = (Index & 0xff0000) >> 16;
  return ((U32)b << 16) | (g << 8) | r;
}

/*************************** End of file ****************************/
