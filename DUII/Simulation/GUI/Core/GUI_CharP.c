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
File        : GUICharP.C
Purpose     : Implementation of Proportional fonts
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"
 
/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP__FindChar
*/
const GUI_FONT_PROP * GUIPROP__FindChar(const GUI_FONT_PROP * pProp, U16P c) {
  for (; pProp; pProp = (const GUI_FONT_PROP *) pProp->pNext) {
    if ((c >= pProp->First) && (c <= pProp->Last))
      break;
  }
  return pProp;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_DispChar(U16P c) {
  int BytesPerLine;
  GUI_DRAWMODE DrawMode, OldDrawMode;
  const GUI_FONT_PROP * pProp;
  const GUI_CHARINFO  * pCharInfo;

  DrawMode = GUI_Context.TextMode;
  pProp    = GUIPROP__FindChar(GUI_Context.pAFont->p.pProp, c);
  if (pProp) {
    pCharInfo = pProp->paCharInfo + (c - pProp->First);
    BytesPerLine = pCharInfo->BytesPerLine;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    LCD_DrawBitmap( GUI_Context.DispPosX, GUI_Context.DispPosY,
                       pCharInfo->XSize,
                       GUI_Context.pAFont->YSize,
                       GUI_Context.pAFont->XMag,
                       GUI_Context.pAFont->YMag,
                       1,     /* Bits per Pixel */
                       BytesPerLine,
                       pCharInfo->pData,
                       LCD_pBkColorIndex
                       );
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
                     GUI_Context.DispPosX + pCharInfo->XSize * GUI_Context.pAFont->YMag, 
                     GUI_Context.DispPosY + YDist);
        GUI_SetColor(OldColor);
      }
    }
    LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
    GUI_Context.DispPosX += pCharInfo->XDist * GUI_Context.pAFont->XMag;
  }
}

/*********************************************************************
*
*       GUIPROP_GetCharDistX
*/
int GUIPROP_GetCharDistX(U16P c) {
  const GUI_FONT_PROP GUI_UNI_PTR * pProp;
  
  pProp = GUIPROP__FindChar(GUI_Context.pAFont->p.pProp, c);
  return (pProp) ? (pProp->paCharInfo + (c - pProp->First))->XSize * GUI_Context.pAFont->XMag : 0;
}

/*********************************************************************
*
*       GUIPROP_GetFontInfo
*/
void GUIPROP_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO* pfi) {
  GUI_USE_PARA(pFont);
  pfi->Flags = GUI_FONTINFO_FLAG_PROP;
}

/*********************************************************************
*
*       GUIPROP_IsInFont
*/
char GUIPROP_IsInFont(const GUI_FONT GUI_UNI_PTR * pFont, U16 c) {
  const GUI_FONT_PROP GUI_UNI_PTR * pProp;

  pProp = GUIPROP__FindChar(pFont->p.pProp, c);
  return (pProp == NULL) ? 0 : 1;
}

/*************************** End of file ****************************/
