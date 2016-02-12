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
File        : LCD_Index2ColorEx.c
Purpose     : Implementation of LCD_Index2ColorEx
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
*       LCD_Index2ColorEx
*
* Purpose:
*   Convert Index to color for the given display
*/
LCD_COLOR LCD_Index2ColorEx(int i, unsigned int LayerIndex) {
  LCD_COLOR r = 0;
  if (LayerIndex < GUI_NUM_LAYERS) {
    r = GUI_Context.apDevice[LayerIndex]->pColorConvAPI->pfIndex2Color(i);
  }
  return r;
}

/*************************** End of file ****************************/
