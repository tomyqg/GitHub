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
File        : GUIPolyAA.C
Purpose     : Draw Polygon routines with Antialiasing
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _FillPolygonAA
*/
static void _FillPolygonAA(GUI_POINT* paPoint, int NumPoints, int x, int y) {
  int i;
  int Stat;
  int x0, x1, paPoint_x;
  int PosFactor;

  PosFactor = GUI_Context.AA_HiResEnable ? 1 : GUI_Context.AA_Factor;
  //
  // Calc horizontal limits and init GUI_AA
  //
  x0 = x1 = paPoint[0].x;
  for (i = 1; i < NumPoints; i++) {
    paPoint_x =  paPoint[i].x;
    if (paPoint_x < x0) {
      x0 = paPoint_x;
    }
    if (paPoint_x > x1) {
      x1 = paPoint_x;
    }
  }
  if (GUI_Context.AA_HiResEnable == 0) {
    Stat = GUI_AA_Init(x0 + x, x1 + x);
  } else {
    Stat = GUI_AA_Init((x0 + x) / GUI_Context.AA_Factor, (x1 + x) / GUI_Context.AA_Factor);
  }
  if (Stat == 0) {
    //
    // Modify pointlist
    //
    if (GUI_Context.AA_HiResEnable == 0) {
      for (i = 0; i < NumPoints; i++) {
        paPoint[i].x *= GUI_Context.AA_Factor;
        paPoint[i].y *= GUI_Context.AA_Factor;
      }
    }
    GL_FillPolygon(paPoint, NumPoints, x * PosFactor, y * PosFactor);
    //
    // Restore pointlist
    //
    if (GUI_Context.AA_HiResEnable == 0) {
      for (i = 0; i < NumPoints; i++) {
        paPoint[i].x /= GUI_Context.AA_Factor;
        paPoint[i].y /= GUI_Context.AA_Factor;
      }
    }
  }
  //
  // Cleanup
  //
  GUI_AA_Exit();
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AA_FillPolygon
*/
void GUI_AA_FillPolygon(GUI_POINT * pPoints, int NumPoints, int x0, int y0) {
  GUI_DEVICE * pDevice;
  void (* pfFillPolygonAA)(GUI_DEVICE *, const GUI_POINT *, int, int, int);
  
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG_AA(x0,y0);
    WM_ITERATE_START(NULL); {
  #endif
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];
  pfFillPolygonAA = (void (*)(GUI_DEVICE *, const GUI_POINT *, int, int, int))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_FILLPOLYAA);
  if (pfFillPolygonAA) {
    pfFillPolygonAA(pDevice, pPoints, NumPoints, x0, y0);
  } else {
    _FillPolygonAA (pPoints, NumPoints, x0, y0);
  }
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
