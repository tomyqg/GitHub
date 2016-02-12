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
File        : GUI_XBF_Prop.c
Purpose     : Implementation of external binary fonts
---------------------------END-OF-HEADER------------------------------
*/

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
  GUI_DRAWMODE         DrawMode;
  GUI_DRAWMODE         OldDrawMode;
  U8                   aBuffer[GUI_MAX_XBF_BYTES];
  const U8           * pBuffer;
  const GUI_XBF_DATA * pXBF_Data;
  U32                  Off, iOff;
  U16                  Size, XSize;
  U16                  BytesPerLine, NumBytes;
  U16                  MaxLines;
  int                  RemLines, NumLines, xPos, yPos;

  pBuffer = aBuffer;
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
  DrawMode     = GUI_Context.TextMode;
  OldDrawMode  = LCD_SetDrawMode(DrawMode);
  if (Size > GUI_MAX_XBF_BYTES) {
    //
    // Get heading data
    //
    pXBF_Data->pfGetData(Off, 4, pXBF_Data->pVoid, aBuffer);
    XSize        = GUI__Read16(&pBuffer);
    BytesPerLine = GUI__Read16(&pBuffer);
    iOff         = Off + 4;
    MaxLines     = (GUI_MAX_XBF_BYTES - 4) / BytesPerLine;
    RemLines     = GUI_Context.pAFont->YSize;
    xPos         = GUI_Context.DispPosX ;
    yPos         = GUI_Context.DispPosY;
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
  } else {
    pXBF_Data->pfGetData(Off, Size, pXBF_Data->pVoid, aBuffer); /* Get character data */
    XSize        = GUI__Read16(&pBuffer);
    BytesPerLine = GUI__Read16(&pBuffer);
    LCD_DrawBitmap(GUI_Context.DispPosX, GUI_Context.DispPosY,
                   XSize,
								   GUI_Context.pAFont->YSize,
                   GUI_Context.pAFont->XMag,
								   GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pBuffer,
                   LCD_pBkColorIndex);
  }
  //
  // Fill empty pixel lines
  //
  if ((DrawMode & LCD_DRAWMODE_TRANS) == 0) {
    if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
      int YMag  = GUI_Context.pAFont->YMag;
      int YDist = GUI_Context.pAFont->YDist * YMag;
      int YSize = GUI_Context.pAFont->YSize * YMag;
      LCD_COLOR OldColor = GUI_GetColor();
      GUI_SetColor(GUI_GetBkColor());
      LCD_FillRect(GUI_Context.DispPosX,
                   GUI_Context.DispPosY + YSize,
                   GUI_Context.DispPosX + XSize,
                   GUI_Context.DispPosY + YDist);
      GUI_SetColor(OldColor);
    }
  }
  //
  // Restore dramode and adjust cursor position
  //
  LCD_SetDrawMode(OldDrawMode);
  GUI_Context.DispPosX += XSize;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_XBF_TYPE_PROP
*/
const tGUI_XBF_APIList GUI_XBF_APIList_Prop = {
  _DispChar,
  GUI_XBF__GetCharDistX,
  GUI_XBF__GetFontInfo,
  GUI_XBF__IsInFont,
};

/*************************** End of file ****************************/
