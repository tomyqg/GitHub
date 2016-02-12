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
File        : GUI_CursorPalI.C
Purpose     : Defines the colors for most inverted cursors
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static GUI_CONST_STORAGE GUI_COLOR _aColor[] = {
  0x0000FF, 0xFFFFFF, 0x000000
};

GUI_CONST_STORAGE GUI_LOGPALETTE GUI_CursorPalI = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_aColor[0]
};

/*************************** End of file ****************************/
