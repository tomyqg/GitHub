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
File        : GUIAALib.C
Purpose     : Antialiasing library
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 *  _pAABuffer;
static U8 *  _pBuffer;
static int  _x0, _x1, _y, _x0_InUse, _x1_InUse, _BufferSize;
static GUI_RECT _ClipRect_HL;

static       tLCD_HL_APIList   _DrawAPICopy;    /* Copy of device function ptr list */
static const tLCD_HL_APIList * _pLCD_HLPrev;    /* Copy of device function ptr list */

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _CleanLine
*/
static void _CleanLine(void) {
  GUI_MEMSET(_pBuffer, 0, _x1 - _x0+1);
  _y = -16383;  /* Invalidate */  
  _x0_InUse =  16383;
  _x1_InUse = -16383;
}

/*********************************************************************
*
*       _FlushLine
*/
static void _FlushLine(void) {
  int Intens, i, j, iEnd, IMax;
  iEnd = _x1_InUse - _x0;
  IMax = GUI_Context.AA_Factor * GUI_Context.AA_Factor;
  /* Make sure not to exceed buffer */
  if (iEnd > (_BufferSize - 1)) {
    iEnd = _BufferSize - 1;
  }
  for (i =_x0_InUse - _x0; i <= iEnd; i++) {
    Intens = *(_pBuffer + i);
    if (Intens) {
      /* Check we can use line draw */
      if (Intens == IMax) {
        for (j = i; j < iEnd; ) {
          if (*(_pBuffer + j + 1) != IMax) {
            break;
          }
          j++;
        }
        /* Draw the full pixel(s) */
        if (j != i) {
          _pLCD_HLPrev->pfDrawHLine(_x0 + i, _y, _x0 + j);
          i = j;
        } else {
          LCD_HL_DrawPixel (_x0 + i, _y);
        }
      } else {
        LCD_SetPixelAA(_x0 + i, _y, (15 * Intens + IMax / 2) / IMax);
      }
    }    
  }
  _CleanLine();
}

/*********************************************************************
*
*       _DrawHLine
*
* Purpose:
*   This is the redirected DrawHLine routine which is called
*   instead of the default output routine. Its job is to do
*   antialiasing and then perform the drawing operations.
*/
static void _DrawHLine  (int x0, int y,  int x1) {
  int x0Real, x1Real, x0_Off, x1_Off, iRem;
  U8 * pDest;
/* Make sure there is something to do */
  if (x1<x0)
    return;
/* Flush line if we are in an other pixel (real) line */
  if (y/GUI_Context.AA_Factor != _y) {
    _FlushLine();
    _y = y / GUI_Context.AA_Factor;
  }
  x0Real = x0 / GUI_Context.AA_Factor;
  x1Real = x1 / GUI_Context.AA_Factor;
/* Handle used area (speed optimization for drawing) */
  if (x0Real < _x0_InUse) {
    _x0_InUse = x0Real;
  }
  if (x1Real > _x1_InUse) {
    _x1_InUse = x1Real;
  }
/* Clip (should not be necessary ... Just to be on the safe side ! */
  if (x0Real < _x0) {
    x0 = _x0 * GUI_Context.AA_Factor;
  }
  if (x1Real > _x1) {
    x1 = (_x1 + 1) * GUI_Context.AA_Factor - 1;
  }
/* Make sure there is still something to do (after clipping) */
  if (x1 < x0) {
    return;
  }
/* Inc. hit counters in buffer */
  {
    x0_Off = x0 / GUI_Context.AA_Factor - _x0;
    x1_Off = x1 / GUI_Context.AA_Factor - _x0;
    iRem = x1_Off - x0_Off + 1;
    pDest = _pBuffer + x0_Off;
    if (iRem == 1) {
      *(pDest) += x1 - x0 + 1;
    } else {
      /* First Pixel */
      *pDest++ += ((x0_Off + _x0 + 1) * GUI_Context.AA_Factor - x0);
      /* Middle Pixels */
      for (;--iRem > 1; ) {
        *pDest++ += GUI_Context.AA_Factor;
      }
      /* Last Pixel */
      *pDest += (1 + x1 - (x1_Off + _x0) * GUI_Context.AA_Factor);
    }
  }
}

/*********************************************************************
*
*       CalcClipRectHL
*/
static void CalcClipRectHL(void) {
  _ClipRect_HL.x0 =  GUI_Context.ClipRect.x0      * GUI_Context.AA_Factor;
  _ClipRect_HL.y0 =  GUI_Context.ClipRect.y0      * GUI_Context.AA_Factor;
  _ClipRect_HL.x1 = (GUI_Context.ClipRect.x1 + 1) * GUI_Context.AA_Factor - 1;
  _ClipRect_HL.y1 = (GUI_Context.ClipRect.y1 + 1) * GUI_Context.AA_Factor - 1;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AA_Init
*/
int GUI_AA_Init(int x0, int x1) {
  int r = 0;
  /* Allocate fixed buffer on first use */
  if (_pAABuffer == NULL) {
    _BufferSize = LCD_GetVXSizeMax();
    _pAABuffer = (U8 *)GUI_ALLOC_GetFixedBlock(_BufferSize);
    if (_pAABuffer == NULL) {
      r = 1;
    }
  }
  /* Bounds checking:
     Make sure x0, x1 are in legal range ...
     (The important point is that they span no more than configured as
      buffer size)
  */
  if (x0 < 0) {
    x0 = 0;
  }
  if ((x1 - x0) > _BufferSize - 1) {
    x1 = x0 + _BufferSize - 1;
  }
  /* Is there anything to do at all ??? */
  if (x1 < x0) {
    x1 = x0;   /* Not really ... */
    r = 1;
  }
  _DrawAPICopy = *GUI_Context.pLCD_HL; /* Copy API table */
  _pLCD_HLPrev = GUI_Context.pLCD_HL; /* Remember list ptr (for restore) */
  _DrawAPICopy.pfDrawHLine = _DrawHLine;  /* modify function ptr. for hline */
  GUI_Context.pLCD_HL = &_DrawAPICopy;      /* Use copy of fp-list */
  _pBuffer = _pAABuffer;
  _x0 = x0;
  _x1 = x1;
  _CleanLine();
  CalcClipRectHL();
  GUI_Context.pClipRect_HL = &_ClipRect_HL;
  return r;
}

/*********************************************************************
*
*       GUI_AA_Init_HiRes
*/
int GUI_AA_Init_HiRes(int x0, int x1) {
  x0 /= GUI_Context.AA_Factor;
  x1 /= GUI_Context.AA_Factor;
  return GUI_AA_Init(x0, x1);
}

/*********************************************************************
*
*       GUI_AA_SetFactor
*/
void GUI_AA_SetFactor(int Factor) {
  GUI_Context.AA_Factor = Factor;
  CalcClipRectHL();      /* High level clipping depends on quality factor */
}

/*********************************************************************
*
*       GUI_AA_GetFactor
*/
int GUI_AA_GetFactor(void) {
  return GUI_Context.AA_Factor;
}

/*********************************************************************
*
*       GUI_AA_DisableHiRes
*/
void GUI_AA_DisableHiRes(void) {
  GUI_Context.AA_HiResEnable = 0;
}

/*********************************************************************
*
*       GUI_AA_EnableHiRes
*/
void GUI_AA_EnableHiRes(void) {
  GUI_Context.AA_HiResEnable =1;
}

/*********************************************************************
*
*       GUI_AA_HiRes2Pixel
*/
I16 GUI_AA_HiRes2Pixel(int HiRes) {
  return GUI_Context.AA_Factor ? (HiRes / GUI_Context.AA_Factor) : HiRes;
}

/*********************************************************************
*
*       GUI_AA_Exit
*/
void GUI_AA_Exit(void) {
  _FlushLine();
  /* restore previous settings */
  GUI_Context.pLCD_HL = _pLCD_HLPrev;
  GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
}

/*************************** End of file ****************************/
