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
File        : RADIO_Default.c
Purpose     : Implementation of radio widget
---------------------------END-OF-HEADER------------------------------
*/

#include "RADIO.h"
#include "RADIO_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_GetDefaultFont
*/
const GUI_FONT GUI_UNI_PTR* RADIO_GetDefaultFont(void) {
  return RADIO__DefaultProps.pFont;
}

/*********************************************************************
*
*       RADIO_GetDefaultTextColor
*/
GUI_COLOR RADIO_GetDefaultTextColor(void) {
  return RADIO__DefaultProps.TextColor;
}

/*********************************************************************
*
*       RADIO_SetDefaultFont
*/
void RADIO_SetDefaultFont(const GUI_FONT GUI_UNI_PTR* pFont) {
  RADIO__DefaultProps.pFont = pFont;
}

/*********************************************************************
*
*       RADIO_SetDefaultTextColor
*/
void RADIO_SetDefaultTextColor(GUI_COLOR TextColor) {
  RADIO__DefaultProps.TextColor = TextColor;
}

/*********************************************************************
*
*       RADIO_SetDefaultFocusColor
*/
GUI_COLOR RADIO_SetDefaultFocusColor(GUI_COLOR Color) {
  GUI_COLOR OldColor;
  OldColor = RADIO__DefaultProps.FocusColor;
  RADIO__DefaultProps.FocusColor = Color;
  return OldColor;
}

#else                            /* Avoid problems with empty object modules */
  void RADIO_Default_C(void);
  void RADIO_Default_C(void) {}
#endif

/************************* end of file ******************************/
