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
File        : GUICC_888.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 888, 4294967296 colors, BBBBBBBBGGGGGGGGRRRRRRRR
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_888
*/
static unsigned _Color2Index_888(LCD_COLOR Color) {
  return Color;
}

/*********************************************************************
*
*       _GetIndexMask_888
*/
static unsigned _GetIndexMask_888(void) {
  return (unsigned)0xffffff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_888
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_888 = {
  _Color2Index_888,
  LCD_Index2Color_888,
  _GetIndexMask_888
};

/*************************** End of file ****************************/
