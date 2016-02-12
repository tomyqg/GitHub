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
File        : GUIDEV_WriteAlpha.C
Purpose     : Implementation of memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       GUI_MEMDEV__ReadLine
*/
void GUI_MEMDEV__ReadLine(int x0, int y, int x1, LCD_PIXELINDEX * pBuffer) {
  LCD_RECT r;
  GUI_DEVICE * pDevice;
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];

  pDevice->pDeviceAPI->pfGetRect(pDevice, &r);
  if (x0 > r.x1) {
    return;
  }
  if (x1 < r.x0) {
    return;
  }
  if (y > r.y1) {
    return;
  }
  if (y < r.y0) {
    return;
  }
  if (x0 < r.x0) {
    pBuffer += r.x0 - x0;
    x0 = r.x0;
  }
  if (x1 > r.x1) {
    x1 = r.x1;
  }
  GUI_ReadRect(x0, y, x1, y, pBuffer, pDevice);
}

#else

void GUI_MEMDEV__ReadLine_C(void);
void GUI_MEMDEV__ReadLine_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
