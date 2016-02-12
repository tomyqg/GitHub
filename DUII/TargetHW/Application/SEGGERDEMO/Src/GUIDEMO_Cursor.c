/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2007  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V4.14 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_Cursor.c
Purpose     : shows Cursor-API
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "GUIDEMO.h"

#if GUI_SUPPORT_CURSOR

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

#define countof(Obj) (sizeof(Obj)/sizeof(Obj[0]))

#define NUM_CURSORS   3
#define XMAX         28

/*********************************************************************
*
*       types
*
**********************************************************************
*/

typedef struct {
  const GUI_CURSOR* pCursor;
  char              Size;
} CURSOR_INFO;

typedef struct {
  const CURSOR_INFO aCursor[NUM_CURSORS];
  const char*       pType;
} CURSORTYPE_INFO;

/*********************************************************************
*
*       static data
*
**********************************************************************
*/

static const CURSORTYPE_INFO _CursorArrow = {
  &GUI_CursorArrowS, 'S',
  &GUI_CursorArrowM, 'M',
  #if (NUM_CURSORS == 3)
    &GUI_CursorArrowL, 'L',
    "arrow cursors"
  #else
    "arrow\ncursors"
  #endif
};


static const CURSORTYPE_INFO _CursorArrowI = {
  &GUI_CursorArrowSI, 'S',
  &GUI_CursorArrowMI, 'M',
  #if (NUM_CURSORS == 3)
    &GUI_CursorArrowLI, 'L',
    "inverted arrow cursors"
  #else
    "inverted\narrow cursors"
  #endif
};

static const CURSORTYPE_INFO _CursorCross = {
  &GUI_CursorCrossS, 'S',
  &GUI_CursorCrossM, 'M',
  #if (NUM_CURSORS == 3)
    &GUI_CursorCrossL, 'L',
    "cross cursors"
  #else
    "cross\ncursors"
  #endif
};

static const CURSORTYPE_INFO _CursorCrossI = {
  &GUI_CursorCrossSI, 'S',
  &GUI_CursorCrossMI, 'M',
  #if (NUM_CURSORS == 3)
    &GUI_CursorCrossLI, 'L',
    "inverted cross cursors"
  #else
    "inverted\ncross cursors"
  #endif
};

static const GUI_CURSOR* _aCursor[] = {
  &GUI_CursorArrowM,
  #if !(GUIDEMO_TINY)
    &GUI_CursorArrowMI,
  #endif
  &GUI_CursorCrossM,
  #if !(GUIDEMO_TINY)
    &GUI_CursorCrossMI,
  #endif
};

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _ShowCursorType
*/
static void _ShowCursorType(const CURSORTYPE_INFO* pCursorType, int x0, int y0) {
  const GUI_CURSOR* pCursor;
  char Char;
  int i, x, y;
  int yMax = 0;
  int yHot = 0;
  /* Calculate height and width of biggest cursor */
  for (i = 0; i < NUM_CURSORS; i++) {
    pCursor = pCursorType->aCursor[i].pCursor;
    if (pCursor->pBitmap->YSize > yMax) {
      yMax = pCursor->pBitmap->YSize;
      yHot = pCursor->yHot;
    }
  }
  GUI_SetFont(&GUI_FontComic18B_ASCII);
  #if (NUM_CURSORS != 3)
    GUI_SetLBorder(x0);
  #endif
  GUI_DispStringAt(pCursorType->pType, x0, y0);
  y0 += GUI_GetFontDistY() + 1;
  GUI_SetFont(&GUI_Font13B_ASCII);
  for (i = 0; i < NUM_CURSORS; i++) {
    pCursor = pCursorType->aCursor[i].pCursor;
    Char    = pCursorType->aCursor[i].Size;
    y = y0 + yHot - pCursor->yHot;
    x = ((pCursor->pBitmap->XSize - GUI_GetCharDistX(Char)) >> 1);
    GUI_DrawBitmap(pCursor->pBitmap, x0 + XMAX * i + 5,     y);
    GUI_DispCharAt(Char,             x0 + XMAX * i + 5 + x, y0 + yMax + 2);
  }
}

/*********************************************************************
*
*       _MoveCursor
*/
static void _MoveCursor(const GUI_CURSOR* Cursor){
  int x, y;
  int xMax = LCD_GetXSize() >> 1;
  int yMax = LCD_GetYSize() >> 1;
  GUI_CURSOR_Select(Cursor);
  for(x = y = 0; ((x < xMax) && (y < yMax) && !GUIDEMO_CheckCancel()); x += 2, y += 2) {
    if ( x == xMax) {
      x = xMax;
    }
    if ( y == yMax) {
      y = yMax;
    }
    GUI_CURSOR_SetPosition(x, y);
    GUI_Delay(25);
  }
}

/*********************************************************************
*
*       _DispCursor
*/
static void _DispCursor(void) {
  int y0, dx, dy;
  GUI_CURSOR_Hide();
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetFont(&GUI_FontComic24B_ASCII);
  GUI_DispStringAt("Available\n  cursors:", 18, 12);
  /* Display the cursors */
  y0 = 68;
  dx = (int)(LCD_GetXSize() / 2.5);
  dy = 75;
  _ShowCursorType(&_CursorArrow,    0, y0);
  _ShowCursorType(&_CursorCross,    0, y0 + dy);
  _ShowCursorType(&_CursorArrowI,  dx, y0);
  _ShowCursorType(&_CursorCrossI,  dx, y0 + dy);
}

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_Cursor
*/
void GUIDEMO_Cursor(void) {
  int i = 0;
  GUIDEMO_ShowIntro("Cursor",
                    "\nemWin supports"
                    "\nsoftware cursor");
  GUIDEMO_SetBkColor(GUI_BLUE);
  GUI_Clear();
  _DispCursor();
  GUIDEMO_Delay(1500);
  GUIDEMO_NotifyStartNext();
  GUI_CURSOR_Show();
  for ( i = 0;(i < countof(_aCursor) && !GUIDEMO_CheckCancel()); i++) {
      _MoveCursor(_aCursor[i]);
  }
  GUI_CURSOR_SetPosition(0,0);
  GUI_CURSOR_Select(&GUI_CursorArrowM);
}

#else

void GUIDEMO_Cursor(void) {}

#endif
