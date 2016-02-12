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
File        : GUICC_111.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code, LCD_FIXEDPALETTE == 111, 8 basic colors
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_111
*/
static unsigned _Color2Index_111(LCD_COLOR Color) {
  int r, g, b;
  r = (Color >> (0  + 7)) & 1;
  g = (Color >> (8  + 7)) & 1;
  b = (Color >> (16 + 7)) & 1;
  return r + (g << 1) + (b << 2);
}

/*********************************************************************
*
*       _Index2Color_111
*/
static LCD_COLOR _Index2Color_111(unsigned Index) {
  U16 r, g, b;
  r = (((Index >> 0) & 1) * 0xff);
  g = (((Index >> 1) & 1) * 0xff);
  b =   (Index >> 2)      * 0xff;
  return r | (g << 8) | ((U32)b << 16);
}

/*********************************************************************
*
*       _GetIndexMask_111
*/
static unsigned _GetIndexMask_111(void) {
  return 0x0007;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_111
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_111 = {
  _Color2Index_111,
  _Index2Color_111,
  _GetIndexMask_111
};

/*********************************************************************
*
*       Static code, LCD_FIXEDPALETTE == 111, 8 basic colors, SWAP_RB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M111
*/
static unsigned _Color2Index_M111(LCD_COLOR Color) {
  int r,g,b;
  r = (Color >> ( 0 + 7)) & 1;
  g = (Color >> ( 8 + 7)) & 1;
  b = (Color >> (16 + 7)) & 1;
  return b + (g << 1) + (r << 2);
}

/*********************************************************************
*
*       _Index2Color_M111
*/
static LCD_COLOR _Index2Color_M111(unsigned Index) {
  U16 r, g, b;
  r = (((Index >> 0) & 1) * 0xff);
  g = (((Index >> 1) & 1) * 0xff);
  b =   (Index >> 2)      * 0xff;
  return b | (g << 8) | ((U32)r << 16);
}

/*********************************************************************
*
*       _GetIndexMask_M111
*/
static unsigned _GetIndexMask_M111(void) {
  return 0x0007;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M111
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M111 = {
  _Color2Index_M111,
  _Index2Color_M111,
  _GetIndexMask_M111
};

/*************************** End of file ****************************/
