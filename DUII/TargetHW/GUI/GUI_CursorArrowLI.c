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
File        : GUI_CursorArrowLI.c
Purpose     : Defines the arrow cursor, large inverted
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapArrowLI = {
 18,                  /* XSize */
 30,                  /* YSize */
 5,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_ArrowL,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorArrowLI = {
  &GUI_BitmapArrowLI, 0, 0
};

/*************************** End of file ****************************/
