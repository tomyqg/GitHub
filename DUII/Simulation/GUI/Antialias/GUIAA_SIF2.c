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
File        : GUIAA_SIF2.c
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
*       _GUI_SIF_DispChar
*/
static void _GUI_SIF_DispChar(U16P c) {
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
    GUI_AA__DrawCharAA2(GUI_Context.DispPosX,      // XPos                 
                        GUI_Context.DispPosY,      // YPos                 
                        CharInfo.XSize,            // XSize                
                        GUI_Context.pAFont->YSize, // YSize                
                        CharInfo.BytesPerLine,     // BytesPerLine         
                        pData);                    // Pointer to pixel data
    GUI_Context.DispPosX += CharInfo.XDist;
    LCD_SetDrawMode(OldDrawMode);
  }
}

/*********************************************************************
*
*       Public API table
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SIF_TYPE_PROP_AA2
*/
const tGUI_SIF_APIList GUI_SIF_APIList_Prop_AA2 = {
  _GUI_SIF_DispChar,
  GUI_SIF__GetCharDistX,
  GUI_SIF__GetFontInfo,
  GUI_SIF__IsInFont
};

/*************************** End of file ****************************/
