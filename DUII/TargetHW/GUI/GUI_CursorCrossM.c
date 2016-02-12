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
File        : GUI_CursorCrossM.C
Purpose     : Defines the cross cursor, medium
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapCrossM = {
 21,                   /* XSize */
 21,                   /* YSize */
 6,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};


GUI_CONST_STORAGE GUI_CURSOR GUI_CursorCrossM = {
  &GUI_BitmapCrossM, 10, 10
};


/*************************** End of file ****************************/
