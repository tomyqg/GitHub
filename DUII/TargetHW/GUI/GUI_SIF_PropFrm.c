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
File        : GUI_SIF_PropFrm.c
Purpose     : Implementation of system independend fonts
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
  U8 BytesPerLine;
  const U8 * pCharInfo, * pData;
  pCharInfo = GUI_SIF__GetpCharInfo(GUI_Context.pAFont, c, sizeof(GUI_SIF_CHARINFO_EXT));
  if (pCharInfo) {
    GUI_DRAWMODE OldDrawMode;
    LCD_PIXELINDEX OldIndex;
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
    OldDrawMode  = LCD_SetDrawMode(GUI_TM_TRANS | (GUI_Context.TextMode & GUI_TM_REV));
    LCD_DrawBitmap(GUI_Context.DispPosX + CharInfo.XOff, GUI_Context.DispPosY + CharInfo.YOff,
                   CharInfo.XSize,
									 CharInfo.YSize,
                   GUI_Context.pAFont->XMag,
									 GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pData,
                   LCD_pBkColorIndex);
    OldIndex = LCD__GetColorIndex();
    LCD__SetColorIndex(LCD__GetBkColorIndex());
    LCD_DrawBitmap(GUI_Context.DispPosX + CharInfo.XOff, GUI_Context.DispPosY + CharInfo.YOff,
                   CharInfo.XSize,
									 CharInfo.YSize,
                   GUI_Context.pAFont->XMag,
									 GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pData + BytesPerLine * CharInfo.YSize,
                   LCD_pBkColorIndex);
    LCD__SetColorIndex(OldIndex);
    LCD_SetDrawMode(OldDrawMode);
    GUI_Context.DispPosX += CharInfo.XDist;
  }
}

/*********************************************************************
*
*       Exported data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SIF_TYPE_PROP
*/
const tGUI_SIF_APIList GUI_SIF_APIList_Prop_Frm = {
  _DispChar,
  GUI_SIF__GetCharDistX_ExtFrm,
  GUI_SIF__GetFontInfo_ExtFrm,
  GUI_SIF__IsInFont_ExtFrm,
};

/*************************** End of file ****************************/
