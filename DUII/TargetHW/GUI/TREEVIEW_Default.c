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
File        : TREEVIEW_Default.c
Purpose     : Implementation of TREEVIEW widget
---------------------------END-OF-HEADER------------------------------
*/

#include "TREEVIEW_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetDefaultColor
*/
static void _SetDefaultColor(int Index, GUI_COLOR Color, int ArrayIndex) {
  if (Index < GUI_COUNTOF(TREEVIEW__DefaultProps.aBkColor)) {
    switch (ArrayIndex) {
    case TREEVIEW_COLORS_BK:
      TREEVIEW__DefaultProps.aBkColor[Index] = Color;
      break;
    case TREEVIEW_COLORS_TEXT:
      TREEVIEW__DefaultProps.aTextColor[Index] = Color;
      break;
    case TREEVIEW_COLORS_LINE:
      TREEVIEW__DefaultProps.aLineColor[Index] = Color;
      break;
    }
  }
}

/*********************************************************************
*
*       _GetDefaultColor
*/
static GUI_COLOR _GetDefaultColor(int Index, int ArrayIndex) {
  GUI_COLOR Color = 0;
  if (Index < GUI_COUNTOF(TREEVIEW__DefaultProps.aBkColor)) {
    switch (ArrayIndex) {
    case TREEVIEW_COLORS_BK:
      Color = TREEVIEW__DefaultProps.aBkColor[Index];
      break;
    case TREEVIEW_COLORS_TEXT:
      Color = TREEVIEW__DefaultProps.aTextColor[Index];
      break;
    case TREEVIEW_COLORS_LINE:
      Color = TREEVIEW__DefaultProps.aLineColor[Index];
      break;
    }
  }
  return Color;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       TREEVIEW_SetDefaultFont
*/
void TREEVIEW_SetDefaultFont(const GUI_FONT GUI_UNI_PTR * pFont) {
  TREEVIEW__DefaultProps.pFont = pFont;
}

/*********************************************************************
*
*       TREEVIEW_GetDefaultFont
*/
const GUI_FONT GUI_UNI_PTR * TREEVIEW_GetDefaultFont(void) {
  return TREEVIEW__DefaultProps.pFont;
}

/*********************************************************************
*
*       TREEVIEW_SetDefaultBkColor
*/
void TREEVIEW_SetDefaultBkColor(int Index, GUI_COLOR Color) {
  _SetDefaultColor(Index, Color, TREEVIEW_COLORS_BK);
}

/*********************************************************************
*
*       TREEVIEW_SetDefaultTextColor
*/
void TREEVIEW_SetDefaultTextColor(int Index, GUI_COLOR Color) {
  _SetDefaultColor(Index, Color, TREEVIEW_COLORS_TEXT);
}

/*********************************************************************
*
*       TREEVIEW_SetDefaultLineColor
*/
void TREEVIEW_SetDefaultLineColor(int Index, GUI_COLOR Color) {
  _SetDefaultColor(Index, Color, TREEVIEW_COLORS_LINE);
}

/*********************************************************************
*
*       TREEVIEW_GetDefaultBkColor
*/
GUI_COLOR TREEVIEW_GetDefaultBkColor(int Index) {
  return _GetDefaultColor(Index, TREEVIEW_COLORS_BK);
}

/*********************************************************************
*
*       TREEVIEW_GetDefaultTextColor
*/
GUI_COLOR TREEVIEW_GetDefaultTextColor(int Index) {
  return _GetDefaultColor(Index, TREEVIEW_COLORS_TEXT);
}

/*********************************************************************
*
*       TREEVIEW_GetDefaultLineColor
*/
GUI_COLOR TREEVIEW_GetDefaultLineColor(int Index) {
  return _GetDefaultColor(Index, TREEVIEW_COLORS_LINE);
}

#else  /* avoid empty object files */

void TREEVIEW_Default_c(void);
void TREEVIEW_Default_c(void){}

#endif  /* #if GUI_WINSUPPORT */

/*************************** End of file ****************************/
