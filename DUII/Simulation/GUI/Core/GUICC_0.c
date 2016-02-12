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
File        : GUICC_0.C
Purpose     : Color conversion routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code, 1/2/4/8 bpp color, LUT
*
**********************************************************************
*/
/*********************************************************************
*
*       _Color2Index_0
*/
static unsigned _Color2Index_0(LCD_COLOR Color) {
  const LCD_COLOR * pPalEntry;
  int i;
  int NumEntries;
  int BestIndex;
  U32 Diff;
  U32 BestDiff;
  
  NumEntries = LCD_GetNumColors();
  BestIndex  = 0;
  BestDiff   = 0xffffff;  // Initialize to worst match
  //
  // Try to find perfect match#
  //
  i = 0;
  pPalEntry = LCD_GetPalette();
  if (pPalEntry) {
    do {
	    if (Color == (*(pPalEntry + i))) {
	      return i;
      }
    } while (++i < NumEntries);
    //
    // Find best match
    //
    i = 0;
    do {
      Diff = GUI_CalcColorDist(Color, *(pPalEntry + i));
      if (Diff < BestDiff) {
        BestDiff  = Diff;
        BestIndex = i;
      }
    } while (++i < NumEntries);
  }
  return BestIndex;
}

/*********************************************************************
*
*       _Index2Color_0
*/
static LCD_COLOR _Index2Color_0(unsigned Index) {
  const LCD_COLOR * pPalEntry;
  unsigned NumEntries;
  
  NumEntries = LCD_GetNumColors();
  pPalEntry  = LCD_GetPalette();
  if (Index >= NumEntries) {
    return 0;     // Illegal index
  }
  if (pPalEntry == NULL) {
    return 0;
  }
  return *(pPalEntry + Index);
}

/*********************************************************************
*
*       _GetIndexMask_0
*/
static unsigned _GetIndexMask_0(void) {
  unsigned NumEntries, Shift;
  
  NumEntries = LCD_GetNumColors();
  if        (NumEntries == 256) {
    Shift = 8;
  } else if (NumEntries >= 128) {
    Shift = 7;
  } else if (NumEntries >= 64) {
    Shift = 6;
  } else if (NumEntries >= 32) {
    Shift = 5;
  } else if (NumEntries >= 16) {
    Shift = 4;
  } else if (NumEntries >= 8) {
    Shift = 3;
  } else if (NumEntries >= 4) {
    Shift = 2;
  } else if (NumEntries >= 2) {
    Shift = 1;
  } else {
    Shift = 0;
  }
  return (1 << Shift) - 1;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_API_ColorConv_0
*/
const LCD_API_COLOR_CONV LCD_API_ColorConv_0 = {
  _Color2Index_0,
  _Index2Color_0,
  _GetIndexMask_0
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_SetLUTEx
*/
void LCD_SetLUTEx(int LayerIndex, const LCD_PHYSPALETTE * pPalette) {
  int i;

  for (i = 0; i < pPalette->NumEntries; i++) {
    LCD_SetLUTEntryEx(LayerIndex, i, *(pPalette->pPalEntries + i));
  }
}

/*********************************************************************
*
*       LCD_SetLUT
*/
void LCD_SetLUT(const LCD_PHYSPALETTE * pPalette) {
  LCD_SetLUTEx(GUI_Context.SelLayer, pPalette);
}

/*************************** End of file ****************************/
