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
File        : GUICC_M8888.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 8888, 4294967296 colors + 8 bit transparency, TTTTTTTTRRRRRRRRGGGGGGGGBBBBBBBB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Index2Color_M8888
*/
static LCD_COLOR _Index2Color_M8888(unsigned Index) {
  U32 r, g, b;
  b = Index & 0xff;
  g = (Index & 0xff00) >> 8;
  r = (Index & 0xff0000) >> 16;
  return (Index & 0xff000000) | (b << 16) | (g << 8) | r;
}

/*********************************************************************
*
*       _Color2Index_M8888
*/
static unsigned _Color2Index_M8888(LCD_COLOR Color) {
  U32 r, g, b;
  b = Color & 0xff;
  g = (Color & 0xff00) >> 8;
  r = (Color & 0xff0000) >> 16;
  return (Color & 0xff000000) | (b << 16) | (g << 8) | r;
}

/*********************************************************************
*
*       _GetIndexMask_M8888
*/
static unsigned _GetIndexMask_M8888(void) {
  return (unsigned)0xffffffff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M8888
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M8888 = {
  _Color2Index_M8888,
  _Index2Color_M8888,
  _GetIndexMask_M8888
};

/*************************** End of file ****************************/
