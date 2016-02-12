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
File        : GUI_XBF_PropFrm.c
Purpose     : Implementation of external binary fonts
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DispChar
*/
static void _DispChar(U16P c) {
  const U8           * pBuffer;
  U8                   aBuffer[GUI_MAX_XBF_BYTES];
  const GUI_XBF_DATA * pXBF_Data;
  U32                  Off, iOff, i;
  U16                  Size;
  I16                  xDist, xOff, yOff;
  U16                  XSize, YSize;
  U16                  MaxLines;
  U16                  BytesPerLine, NumBytes;
  int                  RemLines, NumLines, xPos, yPos;
  GUI_DRAWMODE         OldDrawMode;
  LCD_PIXELINDEX       OldIndex;
  pBuffer   = aBuffer;
  pXBF_Data = (const GUI_XBF_DATA *)GUI_Context.pAFont->p.pFontData;
  if ((c < pXBF_Data->First) || (c > pXBF_Data->Last)) {
    return;
  }
  //
  // Get offset and size of character data
  //
  GUI_XBF__GetOffAndSize(pXBF_Data, c, &Off, &Size);
  if (!Off) {
    return;
  }
  //
  // Set drawmode to transparent and draw the character using the foreground color
  //
  OldDrawMode = LCD_SetDrawMode(GUI_TM_TRANS | (GUI_Context.TextMode & GUI_TM_REV));
  OldIndex    = LCD__GetColorIndex();
  //
  // Use banding mode if size is too large
  //
  if (Size > GUI_MAX_XBF_BYTES) {
    //
    // Get the heading 12 Bytes of the character data
    //
    pXBF_Data->pfGetData(Off, 12, pXBF_Data->pVoid, aBuffer);
    xDist        = GUI__Read16(&pBuffer);
    XSize        = GUI__Read16(&pBuffer);
    YSize        = GUI__Read16(&pBuffer);
    xOff         = GUI__Read16(&pBuffer);
    yOff         = GUI__Read16(&pBuffer);
    BytesPerLine = GUI__Read16(&pBuffer);
    iOff         = Off + 12;
    MaxLines     = (GUI_MAX_XBF_BYTES - 12) / BytesPerLine;
    RemLines     = YSize;
    xPos         = GUI_Context.DispPosX + xOff;
    yPos         = GUI_Context.DispPosY + yOff;
    //
    // Display bands while RemLines are left
    //
    for (i = 0; i < 2; i++) {
      do {
        //
        // Use MaxLines as YSize for a band until RemLines are less than MaxLines
        //
        NumLines   = MaxLines;
        if (NumLines > RemLines) {
          NumLines = RemLines;
        }
        NumBytes   = NumLines * BytesPerLine;
        pXBF_Data->pfGetData(iOff, NumBytes, pXBF_Data->pVoid, aBuffer);
        LCD_DrawBitmap(xPos, yPos,
                       XSize,
								       NumLines,
                       GUI_Context.pAFont->XMag,
								       GUI_Context.pAFont->YMag,
                       1,
                       BytesPerLine,
                       aBuffer,
                       LCD_pBkColorIndex);
        iOff      += NumBytes;
        RemLines  -= NumLines;
        yPos      += NumLines;
      } while (RemLines);
      if (i == 0) {
        RemLines     = YSize;
        yPos         = GUI_Context.DispPosY + yOff;
        LCD__SetColorIndex(LCD__GetBkColorIndex());
      }
    }
  } else {
    pXBF_Data->pfGetData(Off, Size, pXBF_Data->pVoid, aBuffer); /* Get character data */
    xDist        = GUI__Read16(&pBuffer);
    XSize        = GUI__Read16(&pBuffer);
    YSize        = GUI__Read16(&pBuffer);
    xOff         = GUI__Read16(&pBuffer);
    yOff         = GUI__Read16(&pBuffer);
    BytesPerLine = GUI__Read16(&pBuffer);
    LCD_DrawBitmap(GUI_Context.DispPosX + xOff, GUI_Context.DispPosY + yOff,
                   XSize,
								   YSize,
                   GUI_Context.pAFont->XMag,
								   GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pBuffer,
                   LCD_pBkColorIndex);
    //
    // Draw the frame using the background color
    //
    LCD__SetColorIndex(LCD__GetBkColorIndex());
    LCD_DrawBitmap(GUI_Context.DispPosX + xOff, GUI_Context.DispPosY + yOff,
                   XSize,
								   YSize,
                   GUI_Context.pAFont->XMag,
								   GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pBuffer + BytesPerLine * YSize,
                   LCD_pBkColorIndex);
    //
    // Restore colors and dramode and adjust cursor position
    //
  }
  LCD__SetColorIndex(OldIndex);
  LCD_SetDrawMode(OldDrawMode);
  GUI_Context.DispPosX += xDist;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_XBF_TYPE_PROP_FRM
*/
const tGUI_XBF_APIList GUI_XBF_APIList_Prop_Frm = {
  _DispChar,
  GUI_XBF__GetCharDistX,
  GUI_XBF__GetFontInfo,
  GUI_XBF__IsInFont,
};

/*************************** End of file ****************************/
