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
File        : GUICC_M666.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 666, 262144 colors, RRRRRRGGGGGGBBBBBB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M666
*/
static unsigned _Color2Index_M666(LCD_COLOR Color) {
  unsigned r, g, b;
  b = (Color >>  2) & 0x3f;
  g = (Color >> 10) & 0x3f;
  r = (Color >> 18) & 0x3f;
  return r + (g << 6) + (b << 12);
}

/*********************************************************************
*
*       _Index2Color_M666
*/
static LCD_COLOR _Index2Color_M666(unsigned Index) {
  unsigned r, g, b;
  b = (Index & 0x00003f) <<  2;
  g = (Index & 0x000fc0) >>  4; 
  r = (Index & 0x03f000) >> 10; 
  return r + (g << 8) + (((U32)b) << 16);
}

/*********************************************************************
*
*       _GetIndexMask_M666
*/
static unsigned _GetIndexMask_M666(void) {
  return (unsigned)0x3ffff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M666
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M666 = {
  _Color2Index_M666,
  _Index2Color_M666,
  _GetIndexMask_M666
};

/*************************** End of file ****************************/
