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
File        : GUICC_1.c
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_Color2Index_1
*/
static unsigned LCD_Color2Index_1(LCD_COLOR Color) {
  int r, g, b;
  r =  Color        & 255;
  g = (Color >>  8) & 255;
  b = (Color >> 16) & 255;
  return (r + g + b + 383) / (3 * 255);
}

/*********************************************************************
*
*       _Index2Color_1
*/
static LCD_COLOR _Index2Color_1(unsigned Index) {
  return Index ? 0xFFFFFF : 0;
}

/*********************************************************************
*
*       _GetIndexMask_1
*/
static unsigned _GetIndexMask_1(void) {
  return 0x01;
}

/*********************************************************************
*
*       LCD_API_ColorConv_1
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_1 = {
  LCD_Color2Index_1,
  _Index2Color_1,
  _GetIndexMask_1
};

/*************************** End of file ****************************/
