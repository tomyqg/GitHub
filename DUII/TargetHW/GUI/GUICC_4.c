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
File        : GUICC_4.C
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
*       _Color2Index_4
*/
static unsigned _Color2Index_4(LCD_COLOR Color) {
  int r, g, b;
  r = (Color >> ( 0 + 4)) & 15;
  g = (Color >> ( 8 + 4)) & 15;
  b = (Color >> (16 + 4)) & 15;
  return (r + g + b + 1) / 3;
}

/*********************************************************************
*
*       _Index2Color_4
*/
static LCD_COLOR _Index2Color_4(unsigned Index) {
  return ((U32)Index) * 0x111111;
}

/*********************************************************************
*
*       _GetIndexMask_4
*/
static unsigned _GetIndexMask_4(void) {
  return 0x0f;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_4
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_4 = {
  _Color2Index_4,
  _Index2Color_4,
  _GetIndexMask_4
};

/*************************** End of file ****************************/
