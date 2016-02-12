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
File        : LCDColor.C
Purpose     : Color conversion routines for LCD-drivers
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines, config defaults
*
**********************************************************************
*/

#ifndef  LCD_SIZEOF_COLORCACHE
  #define LCD_SIZEOF_COLORCACHE 0
#endif

#if  LCD_SIZEOF_COLORCACHE
  static  const LCD_LOGPALETTE * pLogPalCache;
#endif

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
LCD_PIXELINDEX LCD__aConvTable[LCD_MAX_LOG_COLORS];

LCD_PIXELINDEX * LCD_pBkColorIndex = &GUI_Context.LCD.aColorIndex32[0];
LCD_PIXELINDEX * LCD_pColorIndex   = &GUI_Context.LCD.aColorIndex32[1];
LCD_PIXELINDEX * LCD_pAColorIndex  =  GUI_Context.LCD.aColorIndex32;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_GetpPalConvTableUncached
*/
LCD_PIXELINDEX * LCD_GetpPalConvTableUncached(const LCD_LOGPALETTE GUI_UNI_PTR *  pLogPal) {
  if (pLogPal == NULL) {
    return NULL;
  }
  if (!pLogPal->pPalEntries) {
    return NULL;
  }
  //
  // Check if sufficient space is available
  //
  if (pLogPal->NumEntries > LCD_MAX_LOG_COLORS) {
    return NULL;
  }
  //
  // Build conversion table
  //
  {
    int i;
    int NumEntries = pLogPal->NumEntries;
    const LCD_COLOR GUI_UNI_PTR * pPalEntry = &pLogPal->pPalEntries[0];
    for (i=0; i< NumEntries; i++) {
      LCD__aConvTable[i] = (unsigned)LCD_Color2Index(*(pPalEntry+i));
    }
  }
  return &LCD__aConvTable[0];
}

/*********************************************************************
*
*       LCD_GetpPalConvTable
*/
LCD_PIXELINDEX * LCD_GetpPalConvTable(const LCD_LOGPALETTE GUI_UNI_PTR *  pLogPal) {
//
// Check cache
//
  #if  LCD_SIZEOF_COLORCACHE
    if (pLogPalCache == pLogPal) {
      return &LCD__aConvTable[0];
    }
    pLogPalCache = pLogPal;
  #endif
  return LCD_GetpPalConvTableUncached(pLogPal);
}

/*********************************************************************
*
*       LCD_InitLUT
*/
void LCD_InitLUT(void) {
  GUI_DEVICE * pDevice;
  int DisplayOld, NumLayers, i, j;
  I32 NumColors;
  LCD_COLOR Color;

  NumLayers = LCD_GetNumLayers();
  //
  // Iterate over all layers
  //
  for (i = 0; i < NumLayers; i++) {
    pDevice = GUI_DEVICE__GetpDriver(i);
    if (pDevice) {
      //
      // Initialize LUT only if color depth is <= 8
      //
      DisplayOld = GUI_SelectLayer(i);
      if (LCD_GetBitsPerPixelEx(i) <= 8) {
        if (GUI_Context.apDevice[i]->pColorConvAPI != GUICC_0) {
          //
          // Iterate over colors
          //
          NumColors = LCD_GetNumColorsEx(i);
          for (j = 0; j < NumColors; j++) {
            Color = LCD_Index2Color((U8)j);
            LCD_SetLUTEntryEx(i, (U8)j, Color);
          }
        }
      }
      GUI_SelectLayer(DisplayOld);
    }
  }
}

/*************************** End of file ****************************/
