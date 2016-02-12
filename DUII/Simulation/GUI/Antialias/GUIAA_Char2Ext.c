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
File        : GUIAA_Char2Ext.c
Purpose     : Draw antialiased characters with extended font information
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP_AA2_EXT_DispChar
*/
void GUIPROP_AA2_EXT_DispChar(U16P c) {
  int BytesPerLine;
  GUI_DRAWMODE DrawMode, OldDrawMode;
  const GUI_FONT_PROP_EXT GUI_UNI_PTR * pPropExt;
  const GUI_CHARINFO_EXT  GUI_UNI_PTR * pCharInfo;

  DrawMode = GUI_Context.TextMode;
  pPropExt = GUIPROP_EXT__FindChar(GUI_Context.pAFont->p.pPropExt, c);
  if (pPropExt) {
    pCharInfo    = pPropExt->paCharInfo + (c - pPropExt->First);
    BytesPerLine = (pCharInfo->XSize + 3) >> 2;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    #if GUI_SUPPORT_ROTATION
    if (GUI_pLCD_APIList) {
      GUI_pLCD_APIList->pfDrawBitmap(GUI_Context.DispPosX + pCharInfo->XPos, // XPos
                                     GUI_Context.DispPosY + pCharInfo->YPos, // YPos
                                     pCharInfo->XSize,                       // XSize
                                     pCharInfo->YSize,                       // YSize
                                     1,                                      // XMag
                                     1,                                      // YMag
                                     2,                                      // BitsPerPixel
                                     BytesPerLine,                           // BytesPerLine
                                     pCharInfo->pData,                       // Pointer to pixel data
                                     NULL);                                  // Translation table
    } else {
    #else
    {
    #endif
      GUI_AA__DrawCharAA2(GUI_Context.DispPosX + pCharInfo->XPos, // XPos                 
                          GUI_Context.DispPosY + pCharInfo->YPos, // YPos                 
                          pCharInfo->XSize,                       // XSize                
                          pCharInfo->YSize,                       // YSize                
                          BytesPerLine,                           // BytesPerLine         
                          (U8 const *)pCharInfo->pData);          // Pointer to pixel data
    }
    LCD_SetDrawMode(OldDrawMode); // Restore draw mode
    GUI_Context.DispPosX += pCharInfo->XDist;
  }
}

/*************************** End of file ****************************/
