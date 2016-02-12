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
File        : GUI_GetBitmapPixelColor.c
Purpose     : Gets the color from one pixel of a bitmap
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetBitmapPixelColor
*/
GUI_COLOR GUI_GetBitmapPixelColor(const GUI_BITMAP GUI_UNI_PTR * pBMP, unsigned x, unsigned y) {
  int Index = GUI_GetBitmapPixelIndex(pBMP, x, y);
  if (pBMP->pPal->HasTrans && (Index == 0)) {
    return GUI_INVALID_COLOR;
  } 
  return pBMP->pPal->pPalEntries[Index];
}

/*************************** End of file ****************************/
