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
File        : GUICC_5.C
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
*       _Color2Index_5
*/
static unsigned _Color2Index_5(LCD_COLOR Color) {
  unsigned r, g, b;
  r =  (Color        & 0xff) * 31;
  g = ((Color >>  8) & 0xff) * 31;
  b = ((Color >> 16) & 0xff) * 31;
  return (r + g + b) / (255 * 3);
}

/*********************************************************************
*
*       _Index2Color_5
*/
static LCD_COLOR _Index2Color_5(unsigned Index) {
  U32 g;
  g = (Index * 31 * 255) / (31 * 31);
  return g | (g << 8) | (g << 16);
}

/*********************************************************************
*
*       _GetIndexMask_5
*/
static unsigned _GetIndexMask_5(void) {
  return 0x1f;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_5
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_5 = {
  _Color2Index_5,
  _Index2Color_5,
  _GetIndexMask_5
};

/*************************** End of file ****************************/
