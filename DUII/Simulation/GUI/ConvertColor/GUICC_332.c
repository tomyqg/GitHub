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
File        : GUICC_332.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 332, 256 colors, BBBGGGRR
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_332
*/
static unsigned _Color2Index_332(LCD_COLOR Color) {
  int r, g, b;
  r = Color & 255;
  g = (Color >> 8 ) & 255;
  b = Color >> 16;
  r = (r + 42) / 85;
  g = (g * 7 + 127) / 255;
  b = (b * 7 + 127) / 255;
  return r + (g << 2) + (b << 5);
}

/*********************************************************************
*
*       _Index2Color_332
*/
static LCD_COLOR _Index2Color_332(unsigned Index) {
  unsigned r, g, b;
  r =  (Index & 3)       * 85;
  g = ((Index >> 2) & 7) * 255 / 7;
  b = ((Index >> 5) & 7) * 255 / 7;
  return r + (g << 8) + (((U32)b) << 16);
}

/*********************************************************************
*
*       _GetIndexMask_332
*/
static unsigned _GetIndexMask_332(void) {
  return 0xff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_332
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_332 = {
  _Color2Index_332,
  _Index2Color_332,
  _GetIndexMask_332
};

/*************************** End of file ****************************/
