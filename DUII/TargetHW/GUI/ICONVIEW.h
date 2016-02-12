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
File        : ICONVIEW.h
Purpose     : ICONVIEW include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef ICONVIEW_H
#define ICONVIEW_H

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
/* Status- and create flags */
#define ICONVIEW_CF_AUTOSCROLLBAR_V (1 << 1)
#define ICONVIEW_SF_AUTOSCROLLBAR_V ICONVIEW_CF_AUTOSCROLLBAR_V

/* Color indices */
#define ICONVIEW_CI_BK       0
#define ICONVIEW_CI_UNSEL    0
#define ICONVIEW_CI_SEL      1
#define ICONVIEW_CI_DISABLED 2

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef WM_HMEM ICONVIEW_Handle;

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/
ICONVIEW_Handle ICONVIEW_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int xSizeItem, int ySizeItem);

int  ICONVIEW_AddBitmapItem(ICONVIEW_Handle hObj, const GUI_BITMAP * pBitmap, const char * pText);
int  ICONVIEW_GetSel       (ICONVIEW_Handle hObj);
void ICONVIEW_SetSel       (ICONVIEW_Handle hObj, int Sel);
void ICONVIEW_SetBkColor   (ICONVIEW_Handle hObj, int Index, GUI_COLOR Color);
void ICONVIEW_SetFont      (ICONVIEW_Handle hObj, const GUI_FONT GUI_UNI_PTR * pFont);
void ICONVIEW_SetTextColor (ICONVIEW_Handle hObj, int Index, GUI_COLOR Color);

void ICONVIEW_Callback(WM_MESSAGE * pMsg);

#if defined(__cplusplus)
  }
#endif

#endif   /* if GUI_WINSUPPORT */
#endif   /* ICONVIEW_H */

/*************************** End of file ****************************/
