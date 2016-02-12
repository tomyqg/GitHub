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
File        : GUICharPExt.c
Purpose     : Implementation of extended proportional fonts
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
*       GUIPROP_EXT__FindChar
*/
const GUI_FONT_PROP_EXT GUI_UNI_PTR * GUIPROP_EXT__FindChar(const GUI_FONT_PROP_EXT GUI_UNI_PTR* pPropExt, U16P c) {
  for (; pPropExt; pPropExt = pPropExt->pNext) {
    if ((c>=pPropExt->First) && (c<=pPropExt->Last))
      break;
  }
  return pPropExt;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP_EXT_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_EXT_DispChar(U16P c) {
  int BytesPerLine;
  GUI_DRAWMODE DrawMode = GUI_Context.TextMode;
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt = GUIPROP_EXT__FindChar(GUI_Context.pAFont->p.pPropExt, c);
  if (GUI__CharHasTrans) {
    DrawMode |= GUI_DM_TRANS;
  }
  if (pPropExt) {
    GUI_DRAWMODE OldDrawMode;
    const GUI_CHARINFO_EXT GUI_UNI_PTR * pCharInfo = pPropExt->paCharInfo+(c-pPropExt->First);
    BytesPerLine = (pCharInfo->XSize + 7) >> 3;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    LCD_DrawBitmap(GUI_Context.DispPosX + pCharInfo->XPos, GUI_Context.DispPosY + pCharInfo->YPos,
                   pCharInfo->XSize,
                   pCharInfo->YSize,
                   GUI_Context.pAFont->XMag,
                   GUI_Context.pAFont->YMag,
                   1,     /* Bits per Pixel */
                   BytesPerLine,
                   pCharInfo->pData,
                   LCD_pBkColorIndex);
    LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
    GUI_Context.DispPosX += pCharInfo->XDist * GUI_Context.pAFont->XMag;
  }
}

/*********************************************************************
*
*       GUIPROP_EXT_GetCharDistX
*/
int GUIPROP_EXT_GetCharDistX(U16P c) {
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt = GUIPROP_EXT__FindChar(GUI_Context.pAFont->p.pPropExt, c);
  return (pPropExt) ? (pPropExt->paCharInfo+(c-pPropExt->First))->XDist * GUI_Context.pAFont->XMag : 0;
}

/*********************************************************************
*
*       GUIPROP_EXT_GetFontInfo
*/
void GUIPROP_EXT_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO* pfi) {
  GUI_USE_PARA(pFont);
  pfi->Flags = GUI_FONTINFO_FLAG_PROP;
}

/*********************************************************************
*
*       GUIPROP_EXT_IsInFont
*/
char GUIPROP_EXT_IsInFont(const GUI_FONT GUI_UNI_PTR * pFont, U16 c) {
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt = GUIPROP_EXT__FindChar(pFont->p.pPropExt, c);
  return (pPropExt==NULL) ? 0 : 1;
}

/*********************************************************************
*
*       GUIPROP_EXT_GetCharInfo
*/
int GUIPROP_EXT_GetCharInfo(U16P c, GUI_CHARINFO_EXT * pInfo) {
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt;
  pPropExt = GUIPROP_EXT__FindChar(GUI_Context.pAFont->p.pPropExt, c);
  if (pPropExt) {
    *pInfo = *(pPropExt->paCharInfo + (c - pPropExt->First));
    return 0;
  }
  return 1;
}

/*************************** End of file ****************************/
