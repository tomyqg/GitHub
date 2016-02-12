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
File        : GUICC_M233.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == -233, 256 colors, RRGGGBBB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M233
*/
static unsigned _Color2Index_M233(LCD_COLOR Color) {
  unsigned r, g, b;
  r = Color & 255;
  g = (Color >> 8 ) & 255;
  b = Color >> 16;
  r = (r + 42 ) / 85;
  g = (g * 7 + 127) / 255;
  b = (b * 7 + 127) / 255;
  return b + (g << 3) + (r << 6);
}

/*********************************************************************
*
*       _Index2Color_M233
*/
static LCD_COLOR _Index2Color_M233(unsigned Index) {
  unsigned r, g, b;
  b = (Index & 7) * 255 / 7;
  g = ((Index >> 3) & 7) * 255 / 7;
  r = ((Index >> 6) & 3) * 85;
  return r + (g << 8) + (((U32)b) << 16);
}

/*********************************************************************
*
*       _GetIndexMask_M233
*/
static unsigned _GetIndexMask_M233(void) {
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
*       LCD_API_ColorConv_M233
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M233 = {
  _Color2Index_M233,
  _Index2Color_M233,
  _GetIndexMask_M233
};

/*************************** End of file ****************************/
