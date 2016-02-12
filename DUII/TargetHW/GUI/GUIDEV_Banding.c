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
File        : GUIDEV_Banding.c
Purpose     : Implementation of banding memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled. */

#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Min
*/
static int _Min(int v0, int v1) {
  if (v0 <= v1) {
    return v0;
  }
  return v1;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_Draw
*
*  This routine uses a banding memory device to draw the
*  given area flicker free. It not only draws, but also
*  automatically calculates the size of, creates, moves
*  and then destroys the memory device.
*/
int GUI_MEMDEV_Draw(GUI_RECT * pRect, GUI_CALLBACK_VOID_P * pfDraw, void * pData, int NumLines, int Flags) {
  int x0, y0, x1, y1, xsize, ysize, i, xOrg, yOrg;
  GUI_MEMDEV_Handle hMD;

  if (pRect) {
    GUI_GetOrg(&xOrg, &yOrg);
    x0 = (pRect->x0 < 0) ? 0 : pRect->x0;
    y0 = (pRect->y0 < 0) ? 0 : pRect->y0;
    x1 = _Min(pRect->x1, xOrg + LCD_GetVXSizeEx(GUI_Context.SelLayer) - 1);
    y1 = _Min(pRect->y1, yOrg + LCD_GetVYSizeEx(GUI_Context.SelLayer) - 1);
    xsize = x1 - x0 + 1;
    ysize = y1 - y0 + 1;
  } else {
    x0 = 0;
    y0 = 0;
    xsize = LCD_GetVXSizeEx(GUI_Context.SelLayer);
    ysize = LCD_GetVYSizeEx(GUI_Context.SelLayer);
  }
  if (NumLines == 0) {
    NumLines = -ysize;   /* Request <ysize> lines ... Less is o.k. */
  }
  if ((xsize <= 0) || (ysize <= 0)) {
    return 0;           /* Nothing to do ... */
  }
  //
  // Create memory device
  //
  hMD = GUI_MEMDEV_CreateEx(x0, y0, xsize, NumLines, Flags);
  if (!hMD) {
    GUI_DEBUG_ERROROUT("GUI_MEMDEV_Draw() Not enough memory ...");
    pfDraw(pData);
    return 1;
  }
  NumLines = GUI_MEMDEV_GetYSize(hMD);
  GUI_MEMDEV_Select(hMD);
  //
  // Start drawing
  //
  for (i = 0; i < ysize; i += NumLines) {
    int RemLines = ysize - i;
    if (RemLines < NumLines) {
      GUI_MEMDEV_ReduceYSize(hMD, RemLines);
    }
    if (i) {
      GUI_MEMDEV_SetOrg(hMD, x0, y0 + i);
      GUI_MEMDEV_Clear(hMD);
    }
    pfDraw(pData);
    GUI_MEMDEV_CopyToLCD(hMD);
  }
  GUI_MEMDEV_Delete(hMD);
  GUI_MEMDEV_Select(0);
  return 0;             /* Success ! */
}

#else

void GUIDEV_Banding(void);
void GUIDEV_Banding(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

/*************************** end of file ****************************/
