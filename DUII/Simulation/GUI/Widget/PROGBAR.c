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
File        : ProgBar.c
Purpose     : Implementation of progress bar
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include "PROGBAR_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

#ifndef PROGBAR_DEFAULT_FONT
  #if   WIDGET_USE_SCHEME_SMALL
    #define PROGBAR_DEFAULT_FONT &GUI_Font6x8
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define PROGBAR_DEFAULT_FONT &GUI_Font8x12_ASCII
  #elif WIDGET_USE_SCHEME_LARGE
    #define PROGBAR_DEFAULT_FONT &GUI_Font8x16
  #endif
#endif

#ifndef PROGBAR_DEFAULT_BARCOLOR0
  #define PROGBAR_DEFAULT_BARCOLOR0 0x555555
#endif

#ifndef PROGBAR_DEFAULT_BARCOLOR1
  #define PROGBAR_DEFAULT_BARCOLOR1 0xAAAAAA
#endif

#ifndef PROGBAR_DEFAULT_TEXTCOLOR0
  #define PROGBAR_DEFAULT_TEXTCOLOR0 0xFFFFFF
#endif

#ifndef PROGBAR_DEFAULT_TEXTCOLOR1
  #define PROGBAR_DEFAULT_TEXTCOLOR1 0x000000
#endif

#ifndef   PROGBAR_DRAW_SKIN_DEFAULT
  #define PROGBAR_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
PROGBAR_PROPS PROGBAR__DefaultProps = {
  PROGBAR_DEFAULT_FONT,
  {
    PROGBAR_DEFAULT_BARCOLOR0,
    PROGBAR_DEFAULT_BARCOLOR1,
  },
  {
    PROGBAR_DEFAULT_TEXTCOLOR0,
    PROGBAR_DEFAULT_TEXTCOLOR1
  }
};

WIDGET_SKIN const * PROGBAR__pSkinDefault = &PROGBAR__SkinClassic;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _Paint
*/
static void _Paint(PROGBAR_Handle hObj) {
  PROGBAR_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = PROGBAR_LOCK_H(hObj);
  pfPaint = pObj->pWidgetSkin->pfPaint;
  GUI_UNLOCK_H(pObj);
  if (pfPaint) {
    pfPaint(hObj);
  }
}

/*********************************************************************
*
*       _Delete
*/
static void _Delete(PROGBAR_Handle hObj) {
  PROGBAR_Obj * pObj;
  pObj = PROGBAR_LOCK_H(hObj);
  GUI_ALLOC_FreePtr(&pObj->hpText);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _SetValue
*/
static void _SetValue(PROGBAR_Obj * pObj, char * pText, int v) {
  char * p;
  pObj->v = v;
  p = pText;
  GUI_AddDecMin((100 * (I32)(pObj->v - pObj->Min)) / (pObj->Max - pObj->Min), &p);
  *p++ = '%';
  *p   = 0;
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
PROGBAR_Obj * PROGBAR_LockH(PROGBAR_Handle h) {
  PROGBAR_Obj * p = (PROGBAR_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != PROGBAR_ID) {
      GUI_DEBUG_ERROROUT("PROGBAR.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       PROGBAR__GetTextLocked
*/
char * PROGBAR__GetTextLocked(const PROGBAR_Obj * pObj) {
  char * pText;
  pText = (char *)GUI_LOCK_H(pObj->hpText);
  return pText;
}

/*********************************************************************
*
*       PROGBAR__GetTextRect
*/
void PROGBAR__GetTextRect(const PROGBAR_Obj * pObj, GUI_RECT* pRect, const char* pText) {
  int xSize      = pObj->Widget.Win.Rect.x1 - pObj->Widget.Win.Rect.x0 + 1;
  int ySize      = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
  int TextWidth  = GUI_GetStringDistX(pText);
  int TextHeight = GUI_GetFontSizeY();
  int EffectSize = pObj->Widget.pEffect->EffectSize;
  switch (pObj->TextAlign & GUI_TA_HORIZONTAL) {
  case GUI_TA_CENTER:
    pRect->x0 = (xSize - TextWidth) / 2;
		break;
  case GUI_TA_RIGHT:
    pRect->x0 = xSize - TextWidth - 1 - EffectSize;
		break;
  default:
    pRect->x0 = EffectSize;
	}
  pRect->y0  = (ySize - TextHeight) / 2;
  pRect->x0 += pObj->XOff;
  pRect->y0 += pObj->YOff;
  pRect->x1  = pRect->x0 + TextWidth  - 1;
  pRect->y1  = pRect->y0 + TextHeight - 1;
}

/*********************************************************************
*
*       PROGBAR__Value2Pos
*/
int PROGBAR__Value2Pos(const PROGBAR_Obj * pObj, int v) {
  int IsVertical, EffectSize, Size, Min, Max;

  IsVertical = pObj->Flags & PROGBAR_CF_VERTICAL;
  EffectSize = pObj->Widget.pEffect->EffectSize;
  Size = IsVertical ? pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1 : pObj->Widget.Win.Rect.x1 - pObj->Widget.Win.Rect.x0 + 1;
  Min  = pObj->Min;
  Max  = pObj->Max;
  if (v < Min) {
	  v = Min;
  }
  if (v > Max) {
	  v = Max;
  }
  return EffectSize + ((Size - 2 * EffectSize) * (I32)(IsVertical ? Max - v : v - Min)) / (Max - Min);
}

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_Callback
*/
void PROGBAR_Callback(WM_MESSAGE * pMsg) {
  PROGBAR_Handle hObj;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_PAINT:
    _Paint(hObj);
    return;
  case WM_DELETE:
    _Delete(hObj);
    break;
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_CreateEx
*/
PROGBAR_Handle PROGBAR_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                int WinFlags, int ExFlags, int Id)
{
  PROGBAR_Handle hObj;
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, PROGBAR_Callback,
                                sizeof(PROGBAR_Obj) - sizeof(WM_Obj));
  if (hObj) {
    PROGBAR_Obj * pObj;
    char * pText;
    WM_HMEM hpText;
    hpText = GUI_ALLOC_AllocZero(5);
    if (hpText) {
      pObj = (PROGBAR_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
      /* init widget specific variables */
      WIDGET__Init(&pObj->Widget, Id, 0);
      WIDGET_SetEffect(hObj, &WIDGET_Effect_None); /* Standard effect for progbar: None */
      PROGBAR_INIT_ID(pObj);
      /* init member variables */
      pObj->pWidgetSkin = PROGBAR__pSkinDefault;
      pObj->Props        = PROGBAR__DefaultProps;
      pObj->TextAlign    = GUI_TA_CENTER;
      pObj->Max          = 100;
      pObj->Min          = 0;
      pObj->Flags        = ExFlags;
      pObj->hpText       = hpText;
      pText = (char *)GUI_LOCK_H(pObj->hpText);
      _SetValue(pObj, pText, 0);
      GUI_UNLOCK_H(pText);
      GUI_UNLOCK_H(pObj);
      PROGBAR__pSkinDefault->pfCreate(hObj);
    }
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_SetValue
*/
void PROGBAR_SetValue(PROGBAR_Handle hObj, int v) {
  if (hObj) {
    PROGBAR_Obj * pObj;
    WM_LOCK();
    pObj = PROGBAR_LOCK_H(hObj);
    /* Put v into legal range */
    if (v < pObj->Min) {
	    v = pObj->Min;
    }
    if (v > pObj->Max) {
	    v = pObj->Max;
    }
    if (pObj->v != v) {
      GUI_RECT r;
      if (pObj->Flags & PROGBAR_CF_VERTICAL) {
        /* Get y values */
        if (v < pObj->v) {
          r.y1 = PROGBAR__Value2Pos(pObj, v);
          r.y0 = PROGBAR__Value2Pos(pObj, pObj->v);
        } else {
          r.y1 = PROGBAR__Value2Pos(pObj, pObj->v);
          r.y0 = PROGBAR__Value2Pos(pObj, v);
        }
		    r.x0    = 0;
		    r.x1    = 4095;
        pObj->v = v;
      } else {
        /* Get x values */
        if (v < pObj->v) {
          r.x0 = PROGBAR__Value2Pos(pObj, v);
          r.x1 = PROGBAR__Value2Pos(pObj, pObj->v);
        } else {
          r.x0 = PROGBAR__Value2Pos(pObj, pObj->v);
          r.x1 = PROGBAR__Value2Pos(pObj, v);
        }
		    r.y0    = 0;
		    r.y1    = 4095;
        if ((pObj->Flags & PROGBAR_SF_USER) == 0) {
          const GUI_FONT GUI_UNI_PTR * pOldFont;
          char * pText;
          GUI_RECT rText = {0};
          if (GUI_ALLOC_GetSize(pObj->hpText) < 5) {
            WM_HMEM hpText;
            hpText = pObj->hpText;
            GUI_UNLOCK_H(pObj);
            hpText = GUI_ALLOC_Realloc(hpText, 5);
            pObj = PROGBAR_LOCK_H(hObj);
            pObj->hpText = hpText;
          }
          pText = PROGBAR__GetTextLocked(pObj);
          if (pText) {
            pOldFont = GUI_SetFont(pObj->Props.pFont);
            PROGBAR__GetTextRect(pObj, &rText, pText);
            GUI_MergeRect(&r, &r, &rText);
            _SetValue(pObj, pText, v);
            PROGBAR__GetTextRect(pObj, &rText, pText);
            GUI_MergeRect(&r, &r, &rText);
            GUI_SetFont(pOldFont);
            GUI_UNLOCK_H(pText);
          }
        } else {
          pObj->v = v;
        }
      }
      WM_InvalidateRect(hObj, &r);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else

void WIDGET_Progbar(void) {} /* avoid empty object files */

#endif /* GUI_WINSUPPORT */
