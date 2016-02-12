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
File        : LCD_GetNumDisplays.c
Purpose     : Routines returning info at runtime
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
*       LCD_GetNumLayers
*/
int LCD_GetNumLayers(void) {
  int i, NumLayers;

  NumLayers = 0;
  for (i = 0; i < GUI_NUM_LAYERS; i++) {
    if (GUI_DEVICE__GetpDriver(i)) {
      NumLayers++;
    }
  }
  return NumLayers;
}

/*************************** End of file ****************************/
