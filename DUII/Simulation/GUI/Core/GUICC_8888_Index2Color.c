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
File        : GUICC_8888_Index2Color.c
Purpose     : Color conversion routine for 8888 mode
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Protected.h"

/*********************************************************************
*
*       Public code,
*
*       LCD_FIXEDPALETTE == 8888, 4294967296 colors + 8 bit transparency, TTTTTTTTBBBBBBBBGGGGGGGGRRRRRRRR
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_Index2Color_8888
*/
LCD_COLOR LCD_Index2Color_8888(unsigned Index) {
  return Index;
}

/*************************** End of file ****************************/
