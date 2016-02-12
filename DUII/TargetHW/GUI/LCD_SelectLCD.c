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
File        : LCD_SelectLCD.c
Purpose     : Implementation of said routine
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

#if GUI_WINSUPPORT
#include "WM.h"
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SelectLCD
*/
void LCD_SelectLCD(void) {
  GUI_DEVICE * pDevice;

  //
  // Check if there is a memory device in the device chain
  //
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];
  if (pDevice) {
    do {
      if (pDevice->pDeviceAPI->DeviceClassIndex == DEVICE_CLASS_MEMDEV) {
        break; // Found memory device, so stop here
      }
      pDevice = pDevice->pNext;
    } while (pDevice);
    if (pDevice) {
      GUI_DEVICE_Unlink(pDevice);
    }
  }
  GUI_Context.hDevData = 0;
  GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
  LCD_SetClipRectMax();
}

/*************************** End of file ****************************/
