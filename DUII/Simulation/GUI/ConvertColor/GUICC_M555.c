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
File        : GUICC_M555.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define B_BITS 5
#define G_BITS 5
#define R_BITS 5

#define R_MASK ((1 << R_BITS) -1)
#define G_MASK ((1 << G_BITS) -1)
#define B_MASK ((1 << B_BITS) -1)

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 555, 32768 colors, 0RRRRRGGGGGBBBBB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M555
*/
static unsigned _Color2Index_M555(LCD_COLOR Color) {
  int r, g, b;
  r = (Color >> (8  - R_BITS)) & R_MASK;
  g = (Color >> (16 - G_BITS)) & G_MASK;
  b = (Color >> (24 - B_BITS)) & B_MASK;
  return b + (g << B_BITS) + (r << (G_BITS + B_BITS));
}

/*********************************************************************
*
*       _GetIndexMask_M555
*/
static unsigned _GetIndexMask_M555(void) {
  return 0x7fff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M555
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M555 = {
  _Color2Index_M555,
  LCD_Index2Color_M555,
  _GetIndexMask_M555
};

/*************************** End of file ****************************/
