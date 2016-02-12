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
File        : GUICC_M888.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 888, 4294967296 colors, RRRRRRRRGGGGGGGGBBBBBBBB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M888
*/
static unsigned _Color2Index_M888(LCD_COLOR Color) {
  U32 r, g, b;
  b = Color & 0xff;
  g = (Color & 0xff00) >> 8;
  r = (Color & 0xff0000) >> 16;
  return (unsigned)(b << 16) | (g << 8) | r;
}

/*********************************************************************
*
*       _GetIndexMask_M888
*/
static unsigned _GetIndexMask_M888(void) {
  return (unsigned)0xffffff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M888
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M888 = {
  _Color2Index_M888,
  LCD_Index2Color_M888,
  _GetIndexMask_M888
};

/*************************** End of file ****************************/
