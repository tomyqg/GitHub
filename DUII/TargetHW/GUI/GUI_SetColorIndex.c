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
File        : GUI_DrawBitmap.C
Purpose     : Implementation of GUI_DrawBitmap
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
*       GUI_SetBkColorIndex
*/
void GUI_SetBkColorIndex(int Index) {
  GUI_LOCK(); {
    GUI_Context.BkColor = GUI_INVALID_COLOR;
    LCD_SetBkColorIndex(Index);
  } GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_SetColorIndex
*/
void GUI_SetColorIndex(int Index) {
  GUI_LOCK(); {
    GUI_Context.Color = GUI_INVALID_COLOR;
    LCD_SetColorIndex(Index);
  } GUI_UNLOCK();
}

/*************************** End of file ****************************/
