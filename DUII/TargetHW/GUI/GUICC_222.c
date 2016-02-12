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
File        : GUICC_222.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code, LCD_FIXEDPALETTE == 222, 64 colors
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_222
*/
static unsigned _Color2Index_222(LCD_COLOR Color) {
  int r, g, b;
  r = ( (Color        & 255) + 0x2a) / 0x55;
  g = (((Color >>  8) & 255) + 0x2a) / 0x55;
  b = (((Color >> 16) & 255) + 0x2a) / 0x55;
  return r + (g << 2) + (b << 4);
}

/*********************************************************************
*
*       _Index2Color_222
*/
static LCD_COLOR _Index2Color_222(unsigned Index) {
  U16P r, g, b;
  r = ((Index >> 0) & 3) * 0x55;
  g = ((Index >> 2) & 3) * 0x55;
  b =  (Index >> 4)      * 0x55;
  return (((U32)b) << 16) | (g << 8) | r;
}

/*********************************************************************
*
*       _GetIndexMask_222
*/
static unsigned _GetIndexMask_222(void) {
  return 0x0003f;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_222
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_222 = {
  _Color2Index_222,
  _Index2Color_222,
  _GetIndexMask_222
};

/*********************************************************************
*
*       Static code, LCD_FIXEDPALETTE == 222, 64 colors, SWAP_RB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M222
*/
static unsigned _Color2Index_M222(LCD_COLOR Color) {
  int r, g, b;
  r = ( (Color        & 255) + 0x2a) / 0x55;
  g = (((Color >>  8) & 255) + 0x2a) / 0x55;
  b = (((Color >> 16) & 255) + 0x2a) / 0x55;
  return b + (g << 2) + (r << 4);
}

/*********************************************************************
*
*       LCD_Index2Color_M222
*/
static LCD_COLOR _Index2Color_M222(unsigned Index) {
  U16P r, g, b;
  r = ((Index >> 0) & 3) * 0x55;
  g = ((Index >> 2) & 3) * 0x55;
  b =  (Index >> 4)      * 0x55;
  return (((U32)r) << 16) | (g << 8) | b;
}

/*********************************************************************
*
*       _GetIndexMask_M222
*/
static unsigned _GetIndexMask_M222(void) {
  return 0x003f;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M222
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M222 = {
  _Color2Index_M222,
  _Index2Color_M222,
  _GetIndexMask_M222
};

/*************************** End of file ****************************/
