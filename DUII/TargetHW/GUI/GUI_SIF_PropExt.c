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
File        : GUI_SIF_PropExt.c
Purpose     : Implementation of system independend fonts
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
  U8 BytesPerLine;
  const U8 * pCharInfo, * pData;
  pCharInfo = GUI_SIF__GetpCharInfo(GUI_Context.pAFont, c, sizeof(GUI_SIF_CHARINFO_EXT));
  if (pCharInfo) {
    GUI_DRAWMODE DrawMode, OldDrawMode;
    GUI_SIF_CHARINFO_EXT CharInfo;
    CharInfo.XSize        = GUI__Read16(&pCharInfo);
    CharInfo.YSize        = GUI__Read16(&pCharInfo);
    CharInfo.XOff         = GUI__Read16(&pCharInfo);
    CharInfo.YOff         = GUI__Read16(&pCharInfo);
    CharInfo.XDist        = GUI__Read16(&pCharInfo);
    BytesPerLine          = (CharInfo.XSize + 7) >> 3;
                            GUI__Read16(&pCharInfo);
    CharInfo.OffData      = GUI__Read32(&pCharInfo);
    pData = (const U8 *)GUI_Context.pAFont->p.pFontData + CharInfo.OffData;
    DrawMode = GUI_Context.TextMode;
    if (GUI__CharHasTrans) {
      DrawMode |= GUI_DM_TRANS;
    }
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    LCD_DrawBitmap(GUI_Context.DispPosX + CharInfo.XOff, GUI_Context.DispPosY + CharInfo.YOff,
                   CharInfo.XSize,
									 CharInfo.YSize,
                   GUI_Context.pAFont->XMag,
									 GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pData,
                   LCD_pBkColorIndex);
    LCD_SetDrawMode(OldDrawMode);                         /* Restore draw mode */
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
*       Exported data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SIF_TYPE_PROP_EXT
*/
const tGUI_SIF_APIList GUI_SIF_APIList_Prop_Ext = {
  _DispChar,
  GUI_SIF__GetCharDistX_ExtFrm,
  GUI_SIF__GetFontInfo_ExtFrm,
  GUI_SIF__IsInFont_ExtFrm,
  GUI_SIF__GetCharInfo_ExtFrm,
  &_APIList
};

/*************************** End of file ****************************/
