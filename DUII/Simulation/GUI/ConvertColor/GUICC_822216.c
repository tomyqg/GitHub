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
File        : GUICC_822216.c
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
*       _Color2Index_822216
*/
static unsigned _Color2Index_822216(LCD_COLOR Color) {
  U32 r, g, b, Alpha;

  /* Calculate color components (8 bit info) */
  r = (Color & 0xff)     >>  0;
  g = (Color & 0xff00)   >>  8;
  b = (Color & 0xff0000) >> 16;
  /* Calculate alpha component */
  Alpha = (((Color & 0xff000000) >> 24) ^ 0xff) / 0x11;
  if ((r == g) && (g == b)) {
    /* Return gray */
    return (r >> 5) | (Alpha << 4);
  }
  /* Reduce color information to 1 bit */
  r >>= 7;
  g >>= 7;
  b >>= 7;
  /* Return color */
  return (r << 0) | (g << 1) | (b << 2) | (1 << 3) | (Alpha << 4);
}

/*********************************************************************
*
*       _Index2Color_822216
*/
static LCD_COLOR _Index2Color_822216(unsigned Index) {
  U32 r, g, b, Alpha;

  /* Calculate alpha factor */
  Alpha = (U32)((((Index & 0xf0) >> 4) ^ 0xf) * 0x11) << 24;
  if ((Index & (1 << 3)) == 0) {
    /* Return gray */
    Index &= 7;
    return Alpha | ((Index * (255 / 7) + (Index >> 1)) * 0x010101);
  }
  /* Calculate color components */
  r = (Index & (1 << 0)) ? 0x0000ff : 0;
  g = (Index & (1 << 1)) ? 0x00ff00 : 0;
  b = (Index & (1 << 2)) ? 0xff0000 : 0;
  /* Return color */
  return Alpha | r | g | b;
}

/*********************************************************************
*
*       _GetIndexMask_822216
*/
static unsigned _GetIndexMask_822216(void) {
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
*       LCD_API_ColorConv_822216
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_822216 = {
  _Color2Index_822216,
  _Index2Color_822216,
  _GetIndexMask_822216
};

/*************************** End of file ****************************/
