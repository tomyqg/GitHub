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
File        : WIDGET_Effect_Simple.c
Purpose     : Effect routines
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "WIDGET.h"
#include "GUI_Debug.h"
#include "GUI.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GUI_COLOR _aColor[] = {
  0x000000
};

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawUpRect
*
*  Function description
*    Draws an "Up" effect arround a given rectangle.
*    This effect is used in different situations, but typically for an unpressed button
*    which is supposed to "stick out".
*/
static void _DrawUpRect(const GUI_RECT* pRect) {
  GUI_CONTEXT Context;
  GUI_SaveContext(&Context);
  LCD_SetColor(_aColor[0]);
  GUI_DrawRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);          /* Draw rectangle around it */
  GUI_RestoreContext(&Context);
}

/*********************************************************************
*
*       _DrawUp
*
* Function description
*   Gets the rectangle of the current window and uses _DrawUpRect()
*   for drawing the effect.
*/
static void _DrawUp(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _DrawUpRect(&r);
}

/*********************************************************************
*
*       _DrawDownRect
*
* Function description
*   Draws an "Down" effect arround a given rectangle.
*   This effect is used in different situations, but typically for an edit widget
*   which is supposed to "sunken".
*/
static void _DrawDownRect(const GUI_RECT* pRect) {
  GUI_CONTEXT Context;
  GUI_SaveContext(&Context);
  LCD_SetColor(_aColor[0]);
  GUI_DrawRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);          /* Draw rectangle around it */
  GUI_RestoreContext(&Context);
}

/*********************************************************************
*
*       _DrawDown
*
* Function description
*   Gets the rectangle of the current window and uses _DrawDownRect()
*   for drawing the effect.
*/
static void _DrawDown(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _DrawDownRect(&r);
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       WIDGET_EFFECT_Simple_SetColor
*/
void WIDGET_EFFECT_Simple_SetColor(unsigned Index, GUI_COLOR Color) {
  if (Index < GUI_COUNTOF(_aColor)) {
    _aColor[Index] = Color;
  }
}

/*********************************************************************
*
*       WIDGET_EFFECT_Simple_GetColor
*/
GUI_COLOR WIDGET_EFFECT_Simple_GetColor(unsigned Index) {
  GUI_COLOR r;
  r = 0;
  if (Index < GUI_COUNTOF(_aColor)) {
    r = _aColor[Index];
  }
  return r;
}

/*********************************************************************
*
*       WIDGET_EFFECT_Simple_GetNumColors
*/
int WIDGET_EFFECT_Simple_GetNumColors(void) {
  return GUI_COUNTOF(_aColor);
}

/*********************************************************************
*
*       WIDGET_Effect_Simple
*
* Purpose:
*   Structure of type WIDGET_EFFECT which contains the function pointers
*   used to draw the effect. It also contains the effect size
*   which specifies the number of pixels used by the effect frame.
*/
const WIDGET_EFFECT WIDGET_Effect_Simple = {
  1,
  _DrawUp,
  _DrawUpRect,
  _DrawDown,
  _DrawDownRect,
};

#else                            /* Avoid problems with empty object modules */
  void WIDGET_Effect_Simple_C(void) {}
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
