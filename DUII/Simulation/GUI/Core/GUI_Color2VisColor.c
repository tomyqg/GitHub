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
File        : GUI_Color2VisColor.C
Purpose     : Implementation of GUI_Color2VisColor
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_Color2VisColor
*/
GUI_COLOR GUI_Color2VisColor(GUI_COLOR color) {
  U32 Index;
  GUI_COLOR r;
  GUI_LOCK();
  Index = LCD_Color2Index(color);
  r = LCD_Index2Color(Index);  
  GUI_UNLOCK();
  return r;  
}

/*********************************************************************
*
*       GUI_ColorIsAvailable
*/
char GUI_ColorIsAvailable(GUI_COLOR color) {
  return (GUI_Color2VisColor(color) == color) ? 1 : 0;
}

/*********************************************************************
*
*       GUI_CalcVisColorError
*/
U32 GUI_CalcVisColorError(GUI_COLOR color) {
  return GUI_CalcColorDist(color, GUI_Color2VisColor(color));
}

/*************************** End of file ****************************/
