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
File        : GUIDEV_MoveAndFadeWin.c
Purpose     : Routines for fading and moving windows
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include <stddef.h>

#include "GUI_Private.h"

#if (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)

#include "WM_Intern.h"

#if (WM_SUPPORT_STATIC_MEMDEV)

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifdef WIN32
  #define TIME_PER_FRAME 10
#else
  #define TIME_PER_FRAME 40
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int a180;
  int Period;
  int TimeUsed;
  int dx;
  int dy;
} PARA_IN_MOVE;

typedef struct {
  I32 a;
  int m;
  int ddx;
  int ddy;
} PARA_OUT_MOVE;

typedef struct {
  int Period;         // Complete period to be used for shifting
  int TimeDiff;       // Position on timeline
  int xStart, yStart; // Start moving from here
  int dx, dy;         // Complete distance to be moved during the effect
} PARA_SHIFT;

typedef void (* PF_CALC_PARA_MOVE) (PARA_IN_MOVE * pParaIn, PARA_OUT_MOVE * pParaOut);
typedef int  (* PF_CALC_PARA_FADE) (int Period, int TimeUsed);
typedef void (* PF_INIT_PARA_SHIFT)(PARA_SHIFT * pPara, GUI_MEMDEV_Handle hMemWin, int Period, int Direction);

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _CalcParaMoveIn
*
* Purpose:
*   Calculates the parameters for moving in a window
*/
static void _CalcParaMoveIn(PARA_IN_MOVE * pParaIn, PARA_OUT_MOVE * pParaOut) {
  pParaOut->a   = pParaIn->a180 * 1000 - (pParaIn->TimeUsed * pParaIn->a180 * 1000) / pParaIn->Period;
  pParaOut->m   = (pParaIn->TimeUsed * 1000) / pParaIn->Period;
  pParaOut->ddx = pParaIn->dx - (pParaIn->TimeUsed * pParaIn->dx) / pParaIn->Period;
  pParaOut->ddy = pParaIn->dy - (pParaIn->TimeUsed * pParaIn->dy) / pParaIn->Period;
}

/*********************************************************************
*
*       _CalcParaMoveOut
*
* Purpose:
*   Calculates the parameters for moving out a window
*/
static void _CalcParaMoveOut(PARA_IN_MOVE * pParaIn, PARA_OUT_MOVE * pParaOut) {
  pParaOut->a   = (pParaIn->TimeUsed * pParaIn->a180 * 1000) / pParaIn->Period;
  pParaOut->m   = 1000 - (pParaIn->TimeUsed * 1000) / pParaIn->Period;
  pParaOut->ddx = (pParaIn->TimeUsed * pParaIn->dx) / pParaIn->Period;
  pParaOut->ddy = (pParaIn->TimeUsed * pParaIn->dy) / pParaIn->Period;
}

/*********************************************************************
*
*       _RemoveStaticDevice
*
* Purpose:
*   Removes static device from the given window and its children
*/
static void _RemoveStaticDevice(WM_HWIN hWin) {
  WM_Obj * pObj;

  if (hWin) {
    pObj = (WM_Obj *)GUI_LOCK_H(hWin); {
      if (pObj->Status & WM_CF_STATIC) {
        if (pObj->hMem) {
          GUI_MEMDEV_Delete(pObj->hMem);
          pObj->hMem = 0;
        }
        pObj->Status &= ~WM_CF_STATIC;
      }
    } GUI_UNLOCK_H(pObj);
    hWin = WM_GetFirstChild(hWin);
    while (hWin) {
      _RemoveStaticDevice(hWin);
      hWin = WM_GetNextSibling(hWin);
    }
  }
}

/*********************************************************************
*
*       _MoveWindow
*
* Purpose:
*   Moves in/out a window from/to the given coordinate. Optionally
*   the window can be rotated during the effect.
*
* Parameters:
*   hWin       - Window to be moved
*   x          - Screen coordinate in x for moving window from/to
*   y          - Screen coordinate in y for moving window from/to
*   a180       - Angle in degrees to be used for rotating the window
*   Period     - Period of time for the complete effect
*   pfCalcPara - Function pointer to be used for calculating
*
* Return value:
*   0 on success, 1 on error
*/
static int _MoveWindow(WM_HWIN hWin, int x, int y, int a180, int Period, PF_CALC_PARA_MOVE pfCalcPara) {
  GUI_MEMDEV_Handle hMemWin;
  GUI_MEMDEV_Handle hMemDst;
  GUI_MEMDEV_Handle hMemBk;
  int xSize, ySize;
  int mx, my;
  GUI_RECT Rect;
  U32 * pBk;
  U32 * pDst;
  U32 MemSizeBk;
  int TimeStart, TimeNow, TimeDiff, TimeUsed, r;

  PARA_IN_MOVE  ParaIn;
  PARA_OUT_MOVE ParaOut;

  ParaIn.a180   = a180;
  ParaIn.Period = Period;
  
  //
  // Show window
  //
  WM_ShowWindow(hWin);
  //
  // Create window device
  //
  GUI_MEMDEV_CreateStatic(hWin);
  //
  // Get static memory device from window
  //
  hMemWin = GUI_MEMDEV_GetWindowDevice(hWin);
  //
  // Hide window
  //
  WM_HideWindow(hWin);
  if (hMemWin) {
    //
    // Create background device
    //
    GUI_MEMDEV_CreateStatic(WM_HBKWIN);
    //
    // Set clipping rectangle to desktop area
    //
    WM_SelectWindow(WM_HBKWIN);
    //
    // Get static memory device from background
    //
    hMemBk = GUI_MEMDEV_GetWindowDevice(WM_HBKWIN);
    if (hMemBk) {
      //
      // Get window coordinates
      //
      WM_GetWindowRectEx(hWin, &Rect);
      //
      // Create memory device for animation
      //
      xSize   = LCD_GetXSize();
      ySize   = LCD_GetYSize();
      hMemDst = GUI_MEMDEV_CreateFixed(0, 0, 
                                       xSize, ySize, 
                                       GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_888);
      if (hMemDst) {
        //
        // Calculate size of data block
        //
        MemSizeBk = xSize * ySize * sizeof(LCD_PIXELINDEX);
        //
        // Fill with content of background window
        //
        pBk  = (U32 *)GUI_MEMDEV_GetDataPtr(hMemBk);
        pDst = (U32 *)GUI_MEMDEV_GetDataPtr(hMemDst);
        //
        // Calculate center of window to be animated
        //
        mx = (Rect.x0 + Rect.x1) >> 1;
        my = (Rect.y0 + Rect.y1) >> 1;
        //
        // Calculate parameters for animation
        //
        ParaIn.dx = x - mx;
        ParaIn.dy = y - my;
        //
        // Animation
        //
        TimeStart = GUI_GetTime();
        GUI_X_Delay(TIME_PER_FRAME);
        do {
          //
          // Calculate elapsed time
          //
          TimeNow         = GUI_GetTime();
          TimeDiff        = TimeNow - TimeStart;
          ParaIn.TimeUsed = (TimeDiff > Period) ? Period : TimeDiff;
          //
          // Calculate parameters
          //
          pfCalcPara(&ParaIn, &ParaOut);
          //
          // Fill memory device with background
          //
          GUI_MEMCPY(pDst, pBk, MemSizeBk);
          //
          // Rotate and shrink
          //
          GUI_MEMDEV__Rotate(hMemWin, hMemDst, 
                             Rect.x0 + ParaOut.ddx, 
                             Rect.y0 + ParaOut.ddy, 
                             ParaOut.a, ParaOut.m, 0x00FFFFFF); // Important: No alpha value must remain!
          //
          // Draw memory device without window manager
          //
          WM_Deactivate();
          GUI_MEMDEV_Write(hMemDst);
          WM_Activate();
          //
          // Make sure not more than 25 frames per second are drawn
          //
          TimeUsed = GUI_GetTime() - TimeNow;
          if (TimeUsed < TIME_PER_FRAME) {
            GUI_X_Delay(TIME_PER_FRAME - TimeUsed);
          }
        } while (TimeNow < (TimeStart + Period));
        //
        // Remove temporary memory devices
        //
        GUI_MEMDEV_Delete(hMemDst);
        r = 0;
      } else {
        r = 1;
      }
      GUI_MEMDEV_Delete(hMemBk);
    } else {
      r = 1;
    }
    GUI_MEMDEV_Delete(hMemWin);
  } else {
    r = 1;
  }
  //
  // Remove devices
  //
  _RemoveStaticDevice(WM_HBKWIN);
  //
  // This is the end...
  //
  return r;
}

/*********************************************************************
*
*       _CalcParaFadeIn
*
* Purpose:
*   Calculates the parameters for fading in a window
*/
static int _CalcParaFadeIn(int Period, int TimeUsed) {
  int Intens;

  Intens = (TimeUsed * 255) / Period;
  Intens = (Intens > 255) ? 255 : Intens;
  return Intens;
}

/*********************************************************************
*
*       _CalcParaFadeOut
*
* Purpose:
*   Calculates the parameters for fading out a window
*/
static int _CalcParaFadeOut(int Period, int TimeUsed) {
  int Intens;

  Intens = 255 - (TimeUsed * 255) / Period;
  Intens = (Intens < 0) ? 0 : Intens;
  return Intens;
}

/*********************************************************************
*
*       _FadeWindow
*
* Purpose:
*   Fades in/out a window.
*
* Parameters:
*   hWin       - Window to be faded
*   Period     - Period of time for the complete effect
*   pfCalcPara - Function pointer to be used for calculating
*
* Return value:
*   0 on success, 1 on error
*/
static int _FadeWindow(WM_HWIN hWin, int Period, PF_CALC_PARA_FADE pfCalcPara) {
  GUI_MEMDEV_Handle hMemWin;
  GUI_MEMDEV_Handle hMemDst;
  GUI_MEMDEV_Handle hMemBk;
  int xSize, ySize;
  GUI_RECT Rect;
  int TimeStart, TimeNow, TimeDiff, TimeUsed, Intens, r, xPosWin, yPosWin;

  //
  // Show window
  //
  WM_ShowWindow(hWin);
  //
  // Create window device
  //
  GUI_MEMDEV_CreateStatic(hWin);
  //
  // Get static memory device from window
  //
  hMemWin = GUI_MEMDEV_GetWindowDevice(hWin);
  //
  // Hide window
  //
  WM_HideWindow(hWin);
  if (hMemWin) {
    xPosWin = GUI_MEMDEV_GetXPos(hMemWin);
    yPosWin = GUI_MEMDEV_GetYPos(hMemWin);
    //
    // Create background device
    //
    GUI_MEMDEV_CreateStatic(WM_HBKWIN);
    //
    // Set clipping rectangle to desktop area
    //
    WM_SelectWindow(WM_HBKWIN);
    //
    // Get static memory device from background
    //
    hMemBk = GUI_MEMDEV_GetWindowDevice(WM_HBKWIN);
    if (hMemBk) {
      //
      // Get window coordinates
      //
      WM_GetWindowRectEx(hWin, &Rect);
      //
      // Create memory device for animation
      //
      xSize   = Rect.x1 - Rect.x0 + 1;
      ySize   = Rect.y1 - Rect.y0 + 1;
      hMemDst = GUI_MEMDEV_CreateFixed(Rect.x0, Rect.y0, 
                                       xSize, ySize, 
                                       GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_888);
      if (hMemDst) {
        //
        // Animation
        //
        TimeStart = GUI_GetTime();
        GUI_X_Delay(TIME_PER_FRAME);
        do {
          //
          // Calculate elapsed time
          //
          TimeNow  = GUI_GetTime();
          TimeDiff = TimeNow - TimeStart;
          TimeDiff = (TimeDiff > Period) ? Period : TimeDiff;
          //
          // Calculate parameters
          //
          Intens = pfCalcPara(Period, TimeDiff);
          //
          // Fade device
          //
          GUI_MEMDEV__FadeDeviceEx(hMemWin, hMemBk, hMemDst, Intens, xPosWin, yPosWin);
          //
          // Draw memory device without window manager
          //
          WM_Deactivate();
          GUI_MEMDEV_Write(hMemDst);
          WM_Activate();
          //
          // Make sure not more than 25 frames per second are drawn
          //
          TimeUsed = GUI_GetTime() - TimeNow;
          if (TimeUsed < TIME_PER_FRAME) {
            GUI_X_Delay(TIME_PER_FRAME - TimeUsed);
          }
        } while (TimeNow < (TimeStart + Period));
        //
        // Remove temporary memory devices
        //
        GUI_MEMDEV_Delete(hMemDst);
        r = 0;
      } else {
        r = 1;
      }
      GUI_MEMDEV_Delete(hMemBk);
    } else {
      r = 1;
    }
    GUI_MEMDEV_Delete(hMemWin);
  } else {
    r = 1;
  }
  //
  // Remove devices
  //
  _RemoveStaticDevice(WM_HBKWIN);
  //
  // This is the end...
  //
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_MoveInWindow
*/
int GUI_MEMDEV_MoveInWindow(WM_HWIN hWin, int x, int y, int a180, int Period) {
  int r;

  GUI_LOCK();
  //
  // Move window
  //
  r = _MoveWindow(hWin, x, y, a180, Period, _CalcParaMoveIn);
  //
  // Make visible
  //
  WM_ShowWindow(hWin);
  WM_ValidateWindow(WM_HBKWIN);
  WM_ValidateWindow(hWin);
  //
  // That's all
  //
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_MEMDEV_MoveOutWindow
*/
int GUI_MEMDEV_MoveOutWindow(WM_HWIN hWin, int x, int y, int a180, int Period) {
  int r;

  GUI_LOCK();
  //
  // Move window
  //
  r = _MoveWindow(hWin, x, y, a180, Period, _CalcParaMoveOut);
  //
  // Hide window
  //
  WM_HideWindow(hWin);
  WM_ValidateWindow(WM_HBKWIN);
  //
  // That's all
  //
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_MEMDEV_FadeInWindow
*/
int GUI_MEMDEV_FadeInWindow(WM_HWIN hWin, int Period) {
  int r;

  GUI_LOCK();
  //
  // Fade window
  //
  r = _FadeWindow(hWin, Period, _CalcParaFadeIn);
  //
  // Make visible
  //
  WM_ShowWindow(hWin);
  WM_ValidateWindow(WM_HBKWIN);
  //
  // That's all
  //
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_MEMDEV_FadeOutWindow
*/
int GUI_MEMDEV_FadeOutWindow(WM_HWIN hWin, int Period) {
  int r;

  GUI_LOCK();
  //
  // Fade window
  //
  r = _FadeWindow(hWin, Period, _CalcParaFadeOut);
  //
  // Hide window
  //
  WM_HideWindow(hWin);
  WM_ValidateWindow(WM_HBKWIN);
  //
  // That's all
  //
  GUI_UNLOCK();
  return r;
}

#endif /* WM_SUPPORT_STATIC_MEMDEV */

void GUIDEV_MoveAndFadeWin_C(void);
void GUIDEV_MoveAndFadeWin_C(void) {} /* avoid empty object files */

#else

void GUIDEV_MoveAndFadeWin_C(void);
void GUIDEV_MoveAndFadeWin_C(void) {} /* avoid empty object files */

#endif /* (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT) */

/*************************** end of file ****************************/


