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
File        : GUIAA_Char4Ext_XBF.c
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
  GUI_DRAWMODE         DrawMode;
  GUI_DRAWMODE         OldDrawMode;
  U8                   aBuffer[GUI_MAX_XBF_BYTES];
  const U8           * pBuffer;
  const GUI_XBF_DATA * pXBF_Data;
  U32                  Off, iOff;
  U16                  MaxLines;
  U16                  Size;
  U16                  XSize, YSize;
  U16                  BytesPerLine, NumBytes;
  I16                  xDist, xOff, yOff;
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
      #if GUI_SUPPORT_ROTATION
      if (GUI_pLCD_APIList) {
        GUI_pLCD_APIList->pfDrawBitmap(xPos,         // XPos
                                       yPos,         // YPos
                                       XSize,        // XSize
                                       NumLines,     // YSize
                                       1,            // XMag
                                       1,            // YMag
                                       4,            // BitsPerPixel
                                       BytesPerLine, // BytesPerLine
                                       aBuffer,      // Pointer to pixel data
                                       NULL);        // Translation table
      } else {
      #else
      {
      #endif
        GUI_AA__DrawCharAA4(xPos,                    // XPos 
                            yPos,                    // YPos 
                            XSize,                   // XSize
                            NumLines,                // YSize
                            BytesPerLine,            // BytesPerLine
                            (U8 const *)aBuffer);    // Pointer to pixel data
      }
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
    #if GUI_SUPPORT_ROTATION
    if (GUI_pLCD_APIList) {
      GUI_pLCD_APIList->pfDrawBitmap(GUI_Context.DispPosX + xOff, // XPos
                                     GUI_Context.DispPosY + yOff, // YPos
                                     XSize,                       // XSize
                                     YSize,                       // YSize
                                     1,                           // XMag
                                     1,                           // YMag
                                     4,                           // BitsPerPixel
                                     BytesPerLine,                // BytesPerLine
                                     pBuffer,                     // Pointer to pixel data
                                     NULL);                       // Translation table
    } else {
    #else
    {
    #endif
      GUI_AA__DrawCharAA4(GUI_Context.DispPosX + xOff, // XPos 
                          GUI_Context.DispPosY + yOff, // YPos 
                          XSize,                       // XSize
                          YSize,                       // YSize
                          BytesPerLine,                // BytesPerLine
                          (U8 const *)pBuffer);        // Pointer to pixel data
    }
  }
  LCD_SetDrawMode(OldDrawMode); // Restore draw mode
  GUI_Context.DispPosX += xDist;
}

/*********************************************************************
*
*       _DispLine
*/
static void _DispLine(const char GUI_UNI_PTR * s, int Len) {
  GUIPROP_EXT__SetfpClearLine(GUI_XBF__ClearLine);
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
*       GUI_XBF_APIList_Prop_AA4_Ext
*/
const tGUI_XBF_APIList GUI_XBF_APIList_Prop_AA4_Ext = {
  _DispChar,
  GUI_XBF__GetCharDistX,
  GUI_XBF__GetFontInfo,
  GUI_XBF__IsInFont,
  GUI_XBF__GetCharInfo,
  &_APIList
};

/*************************** End of file ****************************/
