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
File        : WIDGET_Effect_None.c
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
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawDown
*/
static void _DrawDown(void) {
}

/*********************************************************************
*
*       _DrawUp
*/
static void _DrawUp(void) {
}

/*********************************************************************
*
*       _DrawDownRect
*/
static void _DrawDownRect(const GUI_RECT* pRect) {
  GUI_USE_PARA(pRect);
}

/*********************************************************************
*
*       _DrawUpRect
*/
static void _DrawUpRect(const GUI_RECT* pRect) {
  GUI_USE_PARA(pRect);
}

/*********************************************************************
*
*       _DrawFlat
*/
static void _DrawFlat(void) {
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       WIDGET_Effect_None
*
* Purpose:
*   Structure of type WIDGET_EFFECT which contains the function pointers
*   used to draw the effect. It also contains the effect size
*   which specifies the number of pixels used by the effect frame.
*/
const WIDGET_EFFECT WIDGET_Effect_None = {
  0,
  _DrawUp,
  _DrawUpRect,
  _DrawDown,
  _DrawDownRect,
  _DrawFlat,
};

#else                            /* Avoid problems with empty object modules */
  void WIDGET_Effect_None_C(void) {}
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
