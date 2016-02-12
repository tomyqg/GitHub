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
File        : GUICC_2.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_2
*/
static unsigned _Color2Index_2(LCD_COLOR Color) {
  int r, g, b;
  r =  Color       & 255;
  g = (Color >> 8) & 255;
  b =  Color >> 16;
  return (r + g + b + 127) / 255;
}

/*********************************************************************
*
*       _Index2Color_2
*/
static LCD_COLOR _Index2Color_2(unsigned Index) {
  return ((U32)Index) * 0x555555;
}

/*********************************************************************
*
*       _GetIndexMask_2
*/
static unsigned _GetIndexMask_2(void) {
  return 0x03;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_2
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_2 = {
  _Color2Index_2,
  _Index2Color_2,
  _GetIndexMask_2
};

/*************************** End of file ****************************/
