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
File        : GUICC_444_12.c
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 444, 4096 colors, 0000BBBBGGGGRRRR
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_444_12
*/
static unsigned _Color2Index_444_12(LCD_COLOR Color) {
  unsigned int r, g, b;
  r = Color         & 255;
  g = (Color >> 8)  & 255;
  b = (Color >> 16) & 255;
  r = (r + 8) / 17;
  g = (g + 8) / 17;
  b = (b + 8) / 17;
  return r + (g << 4) + (b << 8);
}

/*********************************************************************
*
*       _Index2Color_444_12
*/
static LCD_COLOR _Index2Color_444_12(unsigned Index) {
  unsigned int r, g, b;
  /* Seperate the color masks */
  r = Index & 0xf;
  g = (Index >> 4) & 0xf;
  b = ((unsigned)Index >> 8) & 0xf;
  /* Convert the color masks */
  r = r * 17;
  g = g * 17;
  b = b * 17;
  return r + (g<<8) + (((U32)b)<<16);
}

/*********************************************************************
*
*       _GetIndexMask_444_12
*/
static unsigned _GetIndexMask_444_12(void) {
  return 0x0fff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_444_12
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_444_12 = {
  _Color2Index_444_12,
  _Index2Color_444_12,
  _GetIndexMask_444_12
};

/*************************** End of file ****************************/
