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
File        : LCDGetP.c
Purpose     : Get Pixel routines
              Note: These routines are in a module of their own
                    because they are mostly not required to link
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
*       LCD_GetPixelIndex
*
* NOTE:
*   We can not use the standard clipping which we use for drawing
*   operations as it is perfectly legal to read pixels outside of
*   the clipping area. We therefor get the bounding rectangle of the
*   device and use it for clipping.
*/
unsigned LCD_GetPixelIndex(int x, int y)  {
  LCD_RECT r;  
  GUI_ASSERT_LOCK();
  LCDDEV_L0_GetRect(GUI_Context.apDevice[GUI_Context.SelLayer], &r);
  if (x < r.x0) {
    return 0;
  }
  if (x > r.x1) {
    return 0;
  }
  if (y < r.y0) {
    return 0;
  }
  if (y > r.y1) {
    return 0;
  }
  return LCDDEV_L0_GetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y);
}

/*********************************************************************
*
*       GUI_GetPixelIndex
*/
unsigned GUI_GetPixelIndex(int x, int y)  {
  unsigned r;

  GUI_LOCK();
  r = LCD_GetPixelIndex(x, y);
  GUI_UNLOCK();
  return r;
}


/*************************** End of file ****************************/
