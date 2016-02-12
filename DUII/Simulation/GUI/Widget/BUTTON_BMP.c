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
File        : BUTTON_StreamedBitmap.c
Purpose     : Button streamed bitmap support
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_SetBMPEx
*/
void BUTTON_SetBMPEx(BUTTON_Handle hObj,unsigned int Index, const void * pBitmap, int x, int y) {
  BUTTON__SetDrawObj(hObj, Index, GUI_DRAW_BMP_Create(pBitmap, x, y));
}

/*********************************************************************
*
*       BUTTON_SetBMP
*/
void BUTTON_SetBMP(BUTTON_Handle hObj,unsigned int Index, const void * pBitmap) {
  BUTTON_SetBMPEx(hObj, Index, pBitmap, 0, 0);
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_BMP_C(void) {}
#endif
