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
*       GL_DrawBitmap
*
* Purpose:
*  Translates the external bitmap format into an internal
*  format. This turned out to be necessary as the internal
*  format is easier to create and more flexible for routines
*  that draw text-bitmaps.
*/
void GL_DrawBitmap(const GUI_BITMAP GUI_UNI_PTR * pBitmap, int x0, int y0) {
  GUI_DRAWMODE PrevDraw;
  const GUI_LOGPALETTE GUI_UNI_PTR * pPal;
  pPal = pBitmap->pPal;
  PrevDraw = GUI_SetDrawMode(0);  /* No Get... at this point */
  GUI_SetDrawMode((pPal && pPal->HasTrans) ? (PrevDraw|GUI_DRAWMODE_TRANS) : PrevDraw &(~GUI_DRAWMODE_TRANS));
  if (pBitmap->pMethods) {
    pBitmap->pMethods->pfDraw(x0, y0, pBitmap->XSize ,pBitmap->YSize, (U8 const *)pBitmap->pData, pBitmap->pPal, 1, 1);
  } else {
    const LCD_PIXELINDEX* pTrans;
    pTrans = LCD_GetpPalConvTable(pBitmap->pPal);
    if (!pTrans) {
      pTrans = (pBitmap->BitsPerPixel != 1) ? NULL : LCD_pBkColorIndex;
    }
    LCD_DrawBitmap( x0,y0
                    ,pBitmap->XSize ,pBitmap->YSize
                    ,1,1
                    ,pBitmap->BitsPerPixel
                    ,pBitmap->BytesPerLine
                    ,pBitmap->pData
                    ,pTrans);
  }
  GUI_SetDrawMode(PrevDraw);
}

/*********************************************************************
*
*       GUI_DrawBitmap
*/
void GUI_DrawBitmap(const GUI_BITMAP GUI_UNI_PTR * pBitmap, int x0, int y0) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    r.x1 = (r.x0 = x0) + pBitmap->XSize-1;
    r.y1 = (r.y0 = y0) + pBitmap->YSize-1;
    WM_ITERATE_START(&r) {
  #endif
  GL_DrawBitmap(pBitmap, x0, y0);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
