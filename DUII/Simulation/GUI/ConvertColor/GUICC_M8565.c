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
File        : GUICC_M8565.C
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
#define G_BITS 6
#define R_BITS 5

#define R_MASK ((1 << R_BITS) -1)
#define G_MASK ((1 << G_BITS) -1)
#define B_MASK ((1 << B_BITS) -1)

/*********************************************************************
*
*       Static code,
*
*       LCD_FIXEDPALETTE == 8565, 65536 colors + 8 bit transparency, TTTTTTTTxxxxxxxxRRRRRGGGGGGBBBBB
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_M8565
*/
static unsigned _Color2Index_M8565(LCD_COLOR Color) {
  unsigned r, g, b;
  U32 Alpha;
  Alpha = ((U32)Color & 0xFF000000) ^ 0xFF000000;
  r = (Color >> (8  - R_BITS)) & R_MASK;
  g = (Color >> (16 - G_BITS)) & G_MASK;
  b = (Color >> (24 - B_BITS)) & B_MASK;
  return (U32)(Alpha | (b + (g << B_BITS) + (r << (G_BITS + B_BITS))));
}

/*********************************************************************
*
*       Index2Color_M8565
*/
static LCD_COLOR Index2Color_M8565(unsigned Index) {
  unsigned r, g, b;
  U32 Alpha;
  Alpha = ((U32)Index & 0xFF000000) ^ 0xFF000000;
  /* Separate the color masks */
  b = Index                        & B_MASK;
  g = (Index >> B_BITS)            & G_MASK;
  r = (Index >> (B_BITS + G_BITS)) & R_MASK;
  /* Convert the color masks */
  r = r * 255 / R_MASK;
  g = g * 255 / G_MASK;
  b = b * 255 / B_MASK;
  return (U32)(Alpha | (r + (g << 8) + (((U32)b) << 16)));
}

/*********************************************************************
*
*       _GetIndexMask_M8565
*/
static unsigned _GetIndexMask_M8565(void) {
  return (unsigned)0xffffffff;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_M8565
*
* Purpose
*   API table for this color conversion mode. Only used by memory
*   devices in this mode.
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_M8565 = {
  _Color2Index_M8565,
  Index2Color_M8565,
  _GetIndexMask_M8565
};

/*************************** End of file ****************************/
