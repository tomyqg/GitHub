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
File        : GUI_CharLine.c
Purpose     : Implementation of character and string services
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef   GUI_SUPPORT_BIDI
  #define GUI_SUPPORT_BIDI 0
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Log2VisBuffered
*/
static const char GUI_UNI_PTR * _Log2VisBuffered(const char GUI_UNI_PTR * s, int * pMaxNumChars) {
  GUI_USE_PARA(pMaxNumChars);
  return s;
}

/*********************************************************************
*
*       Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       Function pointer for rearranging strings, default is static code
*/
#if GUI_SUPPORT_BIDI
  const char GUI_UNI_PTR * (* GUI_CharLine_pfLog2Vis)(const char GUI_UNI_PTR * s, int * pMaxNumChars) = GUI__BIDI_Log2VisBuffered;
#else
  const char GUI_UNI_PTR * (* GUI_CharLine_pfLog2Vis)(const char GUI_UNI_PTR * s, int * pMaxNumChars) = _Log2VisBuffered;
#endif

/*********************************************************************
*
*       _DispLine
*/
static void _DispLine(const char GUI_UNI_PTR * s, int MaxNumChars, const GUI_RECT * pRect) {
  //
  // Conversion if required
  //
  s = GUI_CharLine_pfLog2Vis((char *)s, &MaxNumChars);
  //
  // Check if we have anything to do at all ...
  //
  if (GUI_Context.pClipRect_HL) {
    if (GUI_RectsIntersect(GUI_Context.pClipRect_HL, pRect) == 0)
      return;
  }
  if (GUI_Context.pAFont->pafEncode) {
    GUI_Context.pAFont->pafEncode->pfDispLine(s, MaxNumChars);
  } else {
    U16 Char;
    while (--MaxNumChars >= 0) {
      Char = GUI_UC__GetCharCodeInc(&s);
      GUI_Context.pAFont->pfDispChar(Char);
      if (GUI_pfDispCharStyle) {
        GUI_pfDispCharStyle(Char);
      }
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__GetLineNumChars
*/
int GUI__GetLineNumChars(const char GUI_UNI_PTR * s, int MaxNumChars) {
  int NumChars = 0;
  if (s) {
    if (GUI_Context.pAFont->pafEncode) {
      if (GUI_Context.pAFont->pafEncode->pfGetLineLen) {
        return GUI_Context.pAFont->pafEncode->pfGetLineLen(s, MaxNumChars);
      }
    }
    for (; NumChars < MaxNumChars; NumChars++) {
      U16 Data = GUI_UC__GetCharCodeInc(&s);
      if ((Data == 0) || (Data == '\n')) {
        break;
      }
    }
  }
  return NumChars;
}

/*********************************************************************
*
*       GUI__GetOverlap
*/
int GUI__GetOverlap(U16 Char) {
  GUI_CHARINFO_EXT CharInfo;
  int Size;
  int r = 0;
  if (GUI_Context.pAFont->pfGetCharInfo) {
    if (GUI_Context.pAFont->pfGetCharInfo(Char, &CharInfo) == 0) {
      Size = CharInfo.XSize + CharInfo.XPos;
      if (Size > CharInfo.XDist) {
        r = Size - CharInfo.XDist;
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       GUI__GetLineDistX
*
*  This routine is used to calculate the length of a line in pixels.
*/
int GUI__GetLineDistX(const char GUI_UNI_PTR * s, int MaxNumChars) {
  int Dist;
  //
  // Conversion if required
  //
  s = GUI_CharLine_pfLog2Vis((char *)s, &MaxNumChars);
  //
  // Calculate legth of line
  //
  Dist = 0;
  if (s) {
    U16 Char = 0;
    if (GUI_Context.pAFont->pafEncode) {
      if (GUI_Context.pAFont->pafEncode->pfGetLineDistX) {
        return GUI_Context.pAFont->pafEncode->pfGetLineDistX(s, MaxNumChars);
      }
    }
    while (--MaxNumChars >= 0) {
      Char  = GUI_UC__GetCharCodeInc(&s);
      Dist += GUI_GetCharDistX(Char);
    }
    Dist += GUI__GetOverlap(Char);
  }
  return Dist;
}

/*********************************************************************
*
*       GUI__DispLine
*/
void GUI__DispLine(const char GUI_UNI_PTR * s, int MaxNumChars, const GUI_RECT * pr) {
  GUI_RECT r;
  #if GUI_SUPPORT_ROTATION
  if (GUI_pLCD_APIList) {
    #if GUI_WINSUPPORT
    WM_ITERATE_START(NULL) {
    #endif
      //
      // Do the actual drawing via routine call.
      //
      _DispLine(s, MaxNumChars, &r);
    #if GUI_WINSUPPORT
    } WM_ITERATE_END();
    #endif
  } else
  #endif
  {
    r = *pr;
    #if GUI_WINSUPPORT
    WM_ADDORG(r.x0, r.y0);
    WM_ADDORG(r.x1, r.y1);
    WM_ITERATE_START(&r) {
    #endif
      GUI_Context.DispPosX = r.x0;
      GUI_Context.DispPosY = r.y0;
      //
      // Do the actual drawing via routine call.
      //
      _DispLine(s, MaxNumChars, &r);
    #if GUI_WINSUPPORT
    } WM_ITERATE_END();
    WM_SUBORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
    #endif
  }
}

/*************************** End of file ****************************/
