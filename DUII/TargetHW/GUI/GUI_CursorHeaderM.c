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
File        : GUI_CursorHeaderM.c
Purpose     : Defines the header cursor, medium size
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       static data, cursor bitmap
*
**********************************************************************
*/

static GUI_CONST_STORAGE GUI_BITMAP _BitmapHeaderM = {
 17,                  /* XSize */
 17,                  /* YSize */
 5,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_PixelsHeaderM,      /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};


/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
GUI_CONST_STORAGE GUI_CURSOR GUI_CursorHeaderM = {
  &_BitmapHeaderM, 8, 8
};

/*************************** End of file ****************************/
