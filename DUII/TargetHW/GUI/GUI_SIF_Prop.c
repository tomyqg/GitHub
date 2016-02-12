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
File        : GUI_SIF_Prop.c
Purpose     : Implementation of system independend fonts
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DispChar
*/
static void _DispChar(U16P c) {
  const U8 * pCharInfo, * pData;
  pCharInfo = GUI_SIF__GetpCharInfo(GUI_Context.pAFont, c, sizeof(GUI_SIF_CHARINFO));
  if (pCharInfo) {
    GUI_DRAWMODE DrawMode, OldDrawMode;
    GUI_SIF_CHARINFO CharInfo;
    CharInfo.XSize        = GUI__Read16(&pCharInfo);
    CharInfo.XDist        = GUI__Read16(&pCharInfo);
    CharInfo.BytesPerLine = GUI__Read16(&pCharInfo);
                            GUI__Read16(&pCharInfo); /* Dummy */
    CharInfo.OffData      = GUI__Read32(&pCharInfo);
    pData = (const U8 *)GUI_Context.pAFont->p.pFontData + CharInfo.OffData;
    DrawMode = GUI_Context.TextMode;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    LCD_DrawBitmap(GUI_Context.DispPosX, GUI_Context.DispPosY,
                   CharInfo.XSize,
									 GUI_Context.pAFont->YSize,
                   GUI_Context.pAFont->XMag,
									 GUI_Context.pAFont->YMag,
                   1,
                   CharInfo.BytesPerLine,
                   pData,
                   LCD_pBkColorIndex);
    /* Fill empty pixel lines */
    if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
      int YMag = GUI_Context.pAFont->YMag;
      int YDist = GUI_Context.pAFont->YDist * YMag;
      int YSize = GUI_Context.pAFont->YSize * YMag;
      if (DrawMode != LCD_DRAWMODE_TRANS) {
        LCD_COLOR OldColor = GUI_GetColor();
        GUI_SetColor(GUI_GetBkColor());
        LCD_FillRect(GUI_Context.DispPosX, 
                     GUI_Context.DispPosY + YSize, 
                     GUI_Context.DispPosX + CharInfo.XSize, 
                     GUI_Context.DispPosY + YDist);
        GUI_SetColor(OldColor);
      }
    }
    LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
    GUI_Context.DispPosX += CharInfo.XDist;
  }
}

/*********************************************************************
*
*       _GetCharDistX
*/
static int _GetCharDistX(U16P c) {
  const U8 * pCharInfo;
  U16 DistX = 0;
  pCharInfo = GUI_SIF__GetpCharInfo(GUI_Context.pAFont, c, sizeof(GUI_SIF_CHARINFO)); /* Get pointer to char info data */
  if (pCharInfo) {
    pCharInfo += 2;                                       /* Skip unused data */
    DistX = GUI__Read16(&pCharInfo);
  }
  return DistX;
}

/*********************************************************************
*
*       _GetFontInfo
*/
static void _GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO * pfi) {
  const U8 * pData;
  pData = (const U8 *)pFont->p.pFontData + 4;             /* Skip unused data */
  pfi->Baseline = GUI__Read16(&pData);
  pfi->LHeight  = GUI__Read16(&pData);
  pfi->CHeight  = GUI__Read16(&pData);
  pfi->Flags    = GUI_FONTINFO_FLAG_PROP;
}

/*********************************************************************
*
*       _IsInFont
*/
static char _IsInFont(const GUI_FONT GUI_UNI_PTR * pFont, U16 c) {
  const U8 * pCharInfo;
  GUI_USE_PARA(pFont);
  pCharInfo = GUI_SIF__GetpCharInfo(GUI_Context.pAFont, c, sizeof(GUI_SIF_CHARINFO));
  return (pCharInfo) ? 1 : 0;
}

/*********************************************************************
*
*       GUI_SIF_TYPE_PROP
*/
const tGUI_SIF_APIList GUI_SIF_APIList_Prop = {
  _DispChar,
  _GetCharDistX,
  _GetFontInfo,
  _IsInFont
};

/*************************** End of file ****************************/
