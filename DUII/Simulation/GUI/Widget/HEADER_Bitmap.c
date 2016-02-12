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
File        : HEADER_Bitmap.c
Purpose     : HEADER bitmap support
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetBitmapEx
*/
void HEADER_SetBitmapEx(HEADER_Handle hObj, unsigned Index, const GUI_BITMAP * pBitmap, int x, int y) {
  HEADER__SetDrawObj(hObj, Index, GUI_DRAW_BITMAP_Create(pBitmap, x, y));
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       HEADER_SetBitmap
*/
void HEADER_SetBitmap(HEADER_Handle hObj, unsigned Index, const GUI_BITMAP* pBitmap) {
  HEADER_SetBitmapEx(hObj, Index, pBitmap, 0, 0);
}

#else                            /* Avoid problems with empty object modules */
  void HEADER_Bitmap_C(void) {}
#endif
