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
File        : GUIAA_SIF4Ext.c
Purpose     : Implementation of system independent fonts
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
*       _SIF_DispChar
*/
static void _DispChar(U16P c) {
  U8 BytesPerLine;
  const U8 * pCharInfo, * pData;
  GUI_DRAWMODE DrawMode, OldDrawMode;
  GUI_SIF_CHARINFO_EXT CharInfo;

  pCharInfo = GUI_SIF__GetpCharInfo(GUI_Context.pAFont, c, sizeof(GUI_SIF_CHARINFO_EXT));
  if (pCharInfo) {
    CharInfo.XSize        = GUI__Read16(&pCharInfo);
    CharInfo.YSize        = GUI__Read16(&pCharInfo);
    CharInfo.XOff         = GUI__Read16(&pCharInfo);
    CharInfo.YOff         = GUI__Read16(&pCharInfo);
    CharInfo.XDist        = GUI__Read16(&pCharInfo);
    BytesPerLine          = (CharInfo.XSize + 1) >> 1;
                            GUI__Read16(&pCharInfo);
    CharInfo.OffData      = GUI__Read32(&pCharInfo);
    pData = (const U8 *)GUI_Context.pAFont->p.pFontData + CharInfo.OffData;
    DrawMode = GUI_Context.TextMode;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    #if GUI_SUPPORT_ROTATION
    if (GUI_pLCD_APIList) {
      GUI_pLCD_APIList->pfDrawBitmap(GUI_Context.DispPosX + CharInfo.XOff, // XPos
                                     GUI_Context.DispPosY + CharInfo.YOff, // YPos
                                     CharInfo.XSize,                       // XSize
                                     CharInfo.YSize,                       // YSize
                                     1,                                    // XMag
                                     1,                                    // YMag
                                     4,                                    // BitsPerPixel
                                     BytesPerLine,                         // BytesPerLine
                                     pData,                                // Pointer to pixel data
                                     NULL);                                // Translation table
    } else {
    #else
    {
    #endif
      GUI_AA__DrawCharAA4(GUI_Context.DispPosX + CharInfo.XOff, // XPos 
                          GUI_Context.DispPosY + CharInfo.YOff, // YPos 
                          CharInfo.XSize,                       // XSize
                          CharInfo.YSize,                       // YSize
                          BytesPerLine,                         // BytesPerLine
                          (U8 const *)pData);                   // Pointer to pixel data
    }
    LCD_SetDrawMode(OldDrawMode);                      // Restore draw mode
    GUI_Context.DispPosX += CharInfo.XDist;
  }
}

/*********************************************************************
*
*       _DispLine
*/
static void _DispLine(const char GUI_UNI_PTR * s, int Len) {
  GUIPROP_EXT__SetfpClearLine(GUI_SIF__ClearLine_ExtFrm);
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
*       Public API table
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SIF_APIList_Prop_AA4_EXT
*/
const tGUI_SIF_APIList GUI_SIF_APIList_Prop_AA4_EXT = {
  _DispChar,
  GUI_SIF__GetCharDistX_ExtFrm,
  GUI_SIF__GetFontInfo_ExtFrm,
  GUI_SIF__IsInFont_ExtFrm,
  GUI_SIF__GetCharInfo_ExtFrm,
  &_APIList
};

/*************************** End of file ****************************/
