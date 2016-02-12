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
File        : GUIDEV_CmpWithLCD.c
Purpose     : Implementation of Checking for memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled. */ 

#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_CompareWithLCD
*/
int GUI_MEMDEV_CompareWithLCD(GUI_MEMDEV_Handle hMem, int*px, int*py, int *pExp, int*pAct) {
  GUI_DEVICE * pDevice;
  GUI_MEMDEV * pDevData;
  int x, y, x0, XMax, YMax, Max, MagX, MagY, Ret, Exp, Act;
  int BitsPerPixel;

  Ret = 0;
  //
  // Make sure memory handle is valid
  //
  if (!hMem) {
    hMem = GUI_Context.hDevData;
  }
  if (!hMem) {
    return 1;
  }
  GUI_LOCK();
  pDevData = (GUI_MEMDEV*) GUI_ALLOC_h2p(hMem);  // Convert to pointer
  y    = pDevData->y0;
  x0   = pDevData->x0;
  XMax = pDevData->XSize + x0;
  YMax = pDevData->YSize + y;
  MagX = LCD_GetXMag();
  MagY = LCD_GetYMag();
  //
  // Calculate limits
  //
  if (y < 0) {
		y = 0;
  }
  if (x0 < 0) {
		x0 = 0;
  }
  Max = LCD_GetYSize();
	if (YMax > Max) {
		YMax = Max;
  }
  Max = LCD_GetXSize();
	if (XMax > Max) {
		XMax = Max;
  }
  #if (GUI_NUM_LAYERS > 1)
    BitsPerPixel = LCD_GetBitsPerPixelEx(GUI_Context.SelLayer);
  #else
    BitsPerPixel = LCD_GetBitsPerPixel();
  #endif
  pDevice = GUI_DEVICE__GetpDriver(GUI_Context.SelLayer);
  if (BitsPerPixel == 1) {
    for (; y < YMax; y++) {
      x = x0;
      for (x = x0; x < XMax; x++) {
        Exp = LCD_GetPixelIndex(x * MagX, y * MagY);
        Act = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x * MagX, y * MagY);
        if (Act != Exp) {
          *px    = x;
          *py    = y;
          *pAct  = Act;
          *pExp  = Exp;
          Ret = 1;
          goto Skip;
        }
      }
    }
  } else {
    if (BitsPerPixel <= 8) {
      U8 * pData;
      for (; y < YMax; y++) {
        x = x0;
        pData = (U8 *)GUI_MEMDEV__XY2PTR(x, y);
        for (x = x0; x < XMax; x++) {
          Exp = *pData++;
          Act = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x * MagX, y * MagY);
          if (Act != Exp) {
            *px    = x;
            *py    = y;
            *pAct  = Act;
            *pExp  = Exp;
            Ret = 1;
            goto Skip;
          }
        }
      }
    } else if (BitsPerPixel <= 16) {
      U16 * pData;
      for (; y < YMax; y++) {
        x = x0;
        pData = (U16 *)GUI_MEMDEV__XY2PTR(x, y);
        for (x = x0; x < XMax; x++) {
          Exp = *pData++;
          Act = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x * MagX, y * MagY);
          if (Act != Exp) {
            *px    = x;
            *py    = y;
            *pAct  = Act;
            *pExp  = Exp;
            Ret = 1;
            goto Skip;
          }
        }
      }
    } else {
      U32 * pData;
      for (; y < YMax; y++) {
        x = x0;
        pData = (U32 *)GUI_MEMDEV__XY2PTR(x, y);
        for (x = x0; x < XMax; x++) {
          Exp = *pData++;
          Act = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x * MagX, y * MagY);
          if (Act != Exp) {
            *px    = x;
            *py    = y;
            *pAct  = Act;
            *pExp  = Exp;
            Ret = 1;
            goto Skip;
          }
        }
      }
    }
  }
Skip:
  GUI_UNLOCK();
  return Ret;
}

#else

void GUIDEV_CmpWithLCD(void);
void GUIDEV_CmpWithLCD(void) {} // avoid empty object files

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
