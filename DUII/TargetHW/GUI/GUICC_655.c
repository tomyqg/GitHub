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
File        : GUICC_655.C
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

#define B_BITS 6
#define G_BITS 5
#define R_BITS 5

#define R_MASK ((1 << R_BITS) -1)
#define G_MASK ((1 << G_BITS) -1)
#define B_MASK ((1 << B_BITS) -1)

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 655, 65536 colors, BBBBBBGGGGGRRRRR
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_655
*/
static unsigned _Color2Index_655(LCD_COLOR Color) {
  int r, g, b;
  r = (Color >> (8  - R_BITS)) & R_MASK;
  g = (Color >> (16 - G_BITS)) & G_MASK;
  b = (Color >> (24 - B_BITS)) & B_MASK;
  return r + (g << R_BITS) + (b << (G_BITS + R_BITS));
}

/*********************************************************************
*
*       _Index2Color_655
*/
static LCD_COLOR _Index2Color_655(unsigned Index) {
  unsigned int r, g, b;
  /* Separate the color masks */
  r = Index                                  & R_MASK;
  g = (Index >> R_BITS)                      & G_MASK;
  b = ((unsigned)Index >> (R_BITS + G_BITS)) & B_MASK;
  /* Convert the color masks */
  r = r * 255 / R_MASK;
  g = g * 255 / G_MASK;
  b = b * 255 / B_MASK;
  return r + (g << 8) + (((U32)b) << 16);
}

/*********************************************************************
*
*       _GetIndexMask_655
*/
static unsigned _GetIndexMask_655(void) {
  return 0xffff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_655
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_655 = {
  _Color2Index_655,
  _Index2Color_655,
  _GetIndexMask_655
};

/*************************** End of file ****************************/
