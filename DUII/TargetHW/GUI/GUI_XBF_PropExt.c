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
File        : GUI_XBF_PropExt.c
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
  LCD_COLOR            OldColor;
  GUI_DRAWMODE         DrawMode, OldDrawMode;
  U8                   aBuffer[GUI_MAX_XBF_BYTES];
  const U8           * pBuffer;
  const GUI_XBF_DATA * pXBF_Data;
  int                  YMag, YDist;
  U32                  Off, iOff;
  I16                  xDist, xOff, yOff;
  U16                  Size, XSize, YSize;
  U16                  MaxLines;
  U16                  BytesPerLine, NumBytes;
  int                  RemLines, NumLines, xPos, yPos;
  
  pBuffer        = aBuffer;
  pXBF_Data      = (const GUI_XBF_DATA *)GUI_Context.pAFont->p.pFontData;
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
  DrawMode       = GUI_Context.TextMode;
  if (GUI__CharHasTrans) {
    DrawMode    |= GUI_DM_TRANS;
  }
  OldDrawMode    = LCD_SetDrawMode(DrawMode);
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
      LCD_DrawBitmap(xPos,
                     yPos,
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
    //
    // Get character data
    //
    pXBF_Data->pfGetData(Off, Size, pXBF_Data->pVoid, aBuffer);
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
  }
  //
  // Fill empty pixel lines
  //
  if ((DrawMode & LCD_DRAWMODE_TRANS) == 0) {
    if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
      YMag  = GUI_Context.pAFont->YMag;
      YDist = GUI_Context.pAFont->YDist * YMag;
      YSize = GUI_Context.pAFont->YSize * YMag;
      OldColor = GUI_GetColor();
      GUI_SetColor(GUI_GetBkColor());
      LCD_FillRect(GUI_Context.DispPosX,
                   GUI_Context.DispPosY + YSize,
                   GUI_Context.DispPosX + XSize,
                   GUI_Context.DispPosY + YDist);
      GUI_SetColor(OldColor);
    }
  }
  //
  // Restore drawmode and adjust cursor position
  //
  LCD_SetDrawMode(OldDrawMode);
  GUI_Context.DispPosX += xDist;
}

/*********************************************************************
*
*       _ClearLine
*/
static void _ClearLine(const char GUI_UNI_PTR * s, int Len) {
  U16 c;
  I16 XDist;
  int yDist, DispPosX, DispPosY;
  LCD_COLOR OldColor;
  OldColor = GUI_GetColor();
  GUI_SetColor((GUI_Context.TextMode & GUI_TM_REV) ? GUI_GetColor() : GUI_GetBkColor());
  yDist    = GUI_Context.pAFont->YDist * GUI_Context.pAFont->YMag;
  DispPosX = GUI_Context.DispPosX;
  DispPosY = GUI_Context.DispPosY;
  XDist    = 0;
  while (--Len >= 0) {
    c = GUI_UC__GetCharCodeInc(&s);
    XDist += GUI_XBF__GetCharDistX(c);
  }
  XDist += GUI__GetOverlap(c);
  LCD_FillRect(DispPosX, 
               DispPosY, 
               DispPosX + XDist, 
               DispPosY + yDist);
  GUI_SetColor(OldColor);
  //
  // Avoid background pixels to be drawn twice
  //
  GUI__CharHasTrans = 1;
}

/*********************************************************************
*
*       _DispLine
*/
static void _DispLine(const char GUI_UNI_PTR * s, int Len) {
  GUIPROP_EXT__SetfpClearLine(_ClearLine);
  GUI_ENC_APIList_EXT.pfDispLine(s, Len);
  GUIPROP_EXT__SetfpClearLine(GUIPROP_EXT__ClearLine);
}

/*********************************************************************
*
*       _APIList
*/
static const tGUI_ENC_APIList _APIList = {
  NULL,
  NULL,
  _DispLine
};

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_XBF_TYPE_PROP_EXT
*/
const tGUI_XBF_APIList GUI_XBF_APIList_Prop_Ext = {
  _DispChar,
  GUI_XBF__GetCharDistX,
  GUI_XBF__GetFontInfo,
  GUI_XBF__IsInFont,
  GUI_XBF__GetCharInfo,
  &_APIList
};

/*************************** End of file ****************************/
