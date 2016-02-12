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
*       GUIPROP_FRM__FindChar
*/
static const GUI_FONT_PROP_EXT GUI_UNI_PTR * GUIPROP_FRM__FindChar(const GUI_FONT_PROP_EXT GUI_UNI_PTR* pPropExt, U16P c) {
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
*       GUIPROP_FRM_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_FRM_DispChar(U16P c) {
  int BytesPerLine;
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt = GUIPROP_FRM__FindChar(GUI_Context.pAFont->p.pPropExt, c);
  if (pPropExt) {
    GUI_DRAWMODE OldDrawMode;
    LCD_PIXELINDEX OldIndex;
    const GUI_CHARINFO_EXT GUI_UNI_PTR * pCharInfo = pPropExt->paCharInfo+(c-pPropExt->First);
    BytesPerLine = (pCharInfo->XSize + 7) >> 3;
    OldDrawMode  = LCD_SetDrawMode(GUI_TM_TRANS | (GUI_Context.TextMode & GUI_TM_REV));
    LCD_DrawBitmap(GUI_Context.DispPosX + pCharInfo->XPos, GUI_Context.DispPosY + pCharInfo->YPos,
                   pCharInfo->XSize,
                   pCharInfo->YSize,
                   GUI_Context.pAFont->XMag,
                   GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pCharInfo->pData,
                   LCD_pBkColorIndex);
    OldIndex = LCD__GetColorIndex();
    LCD__SetColorIndex(LCD__GetBkColorIndex());
    LCD_DrawBitmap(GUI_Context.DispPosX + pCharInfo->XPos, GUI_Context.DispPosY + pCharInfo->YPos,
                   pCharInfo->XSize,
                   pCharInfo->YSize,
                   GUI_Context.pAFont->XMag,
                   GUI_Context.pAFont->YMag,
                   1,
                   BytesPerLine,
                   pCharInfo->pData + BytesPerLine * pCharInfo->YSize,
                   LCD_pBkColorIndex);
    LCD__SetColorIndex(OldIndex);
    LCD_SetDrawMode(OldDrawMode);
    GUI_Context.DispPosX += pCharInfo->XDist * GUI_Context.pAFont->XMag;
  }
}

/*********************************************************************
*
*       GUIPROP_FRM_GetCharDistX
*/
int GUIPROP_FRM_GetCharDistX(U16P c) {
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt = GUIPROP_FRM__FindChar(GUI_Context.pAFont->p.pPropExt, c);
  return (pPropExt) ? (pPropExt->paCharInfo+(c-pPropExt->First))->XDist * GUI_Context.pAFont->XMag : 0;
}

/*********************************************************************
*
*       GUIPROP_FRM_GetFontInfo
*/
void GUIPROP_FRM_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO* pfi) {
  GUI_USE_PARA(pFont);
  pfi->Flags = GUI_FONTINFO_FLAG_PROPFRM;
}

/*********************************************************************
*
*       GUIPROP_FRM_IsInFont
*/
char GUIPROP_FRM_IsInFont(const GUI_FONT GUI_UNI_PTR * pFont, U16 c) {
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt = GUIPROP_FRM__FindChar(pFont->p.pPropExt, c);
  return (pPropExt==NULL) ? 0 : 1;
}

/*************************** End of file ****************************/
