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
File        : GUI_TOUCH_DriverAnalog.c
Purpose     : Touch screen manager
----------------------------------------------------------------------
This module handles the touch screen. It is configured in the file
GUITouch.conf.h (Should be located in the Config\ directory).
----------------------------------------------------------------------
*/

#include <stdlib.h>

#include "GUI_Private.h"

/* Generate code only if configuration says so ! */
#if GUI_SUPPORT_TOUCH

#if (GUI_SUPPORT_TOUCH == 2)
  #include "GUITouchConf.h"
#endif

/*********************************************************************
*
*       Defines, config defaults
*
**********************************************************************
*/

#ifndef GUI_TOUCH_AD_LEFT         /* max value returned by AD-converter */
  #define GUI_TOUCH_AD_LEFT 0
#endif

#ifndef GUI_TOUCH_AD_RIGHT        /* min value returned by AD-converter */
  #define GUI_TOUCH_AD_RIGHT 0
#endif

#ifndef GUI_TOUCH_AD_TOP          /* max value returned by AD-converter */
  #define GUI_TOUCH_AD_TOP 0
#endif

#ifndef GUI_TOUCH_AD_BOTTOM       /* min value returned by AD-converter */
  #define GUI_TOUCH_AD_BOTTOM 0
#endif

#ifndef GUI_TOUCH_SWAP_XY         /* Is XY of touch swapped ? */
  #define GUI_TOUCH_SWAP_XY 0
#endif

#ifndef GUI_TOUCH_MIRROR_X
  #define GUI_TOUCH_MIRROR_X 0
#endif

#ifndef GUI_TOUCH_MIRROR_Y
  #define GUI_TOUCH_MIRROR_Y 0
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct { int Min; int Max; } MIN_MAX_T;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static int _xPhys, _yPhys;

static unsigned _OrientationPhys = (GUI_TOUCH_MIRROR_X << 0) | (GUI_TOUCH_MIRROR_X << 1) | (GUI_TOUCH_SWAP_XY << 2);
static unsigned _OrientationLog;

static MIN_MAX_T _xyMinMax[2];
static int       _CalibrationDone;

#ifndef WIN32

static int _xMin;
static int _xMax;
static int _yMin;
static int _yMax;

#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _AD2X
*
* Purpose:
*   Convert physical value into (logical) coordinates.
*/
#ifndef WIN32
static int _AD2X(int adx) {
  I32 r;
  int xSize;
  r = adx - _xyMinMax[GUI_COORD_X].Min;
  if ((_OrientationLog & GUI_SWAP_XY) == 0) {
    xSize = LCD_GetXSizeEx(GUI_Context.SelLayer);
  } else {
    xSize = LCD_GetYSizeEx(GUI_Context.SelLayer);
  }
  r *= xSize - 1;
  return r / (_xyMinMax[GUI_COORD_X].Max - _xyMinMax[GUI_COORD_X].Min);    
}

/*********************************************************************
*
*       _AD2Y
*
* Purpose:
*   Convert physical value into (logical) coordinates.
*/
static int _AD2Y(int ady) {
  I32 r = ady - _xyMinMax[GUI_COORD_Y].Min;
  int ySize;
  if ((_OrientationLog & GUI_SWAP_XY) == 0) {
    ySize = LCD_GetYSizeEx(GUI_Context.SelLayer);
  } else {
    ySize = LCD_GetXSizeEx(GUI_Context.SelLayer);
  }
  r *= ySize - 1;
  return r/(_xyMinMax[GUI_COORD_Y].Max - _xyMinMax[GUI_COORD_Y].Min);    
}
#endif

/*********************************************************************
*
*       _Log2Phys
*/
static int _Log2Phys(int l, I32 l0, I32 l1, I32 p0, I32 p1) {
  return p0 + ((p1 - p0) * (I32)(l - l0)) / (l1 - l0);
}

/*********************************************************************
*
*       _StoreUnstable
*/
static void _StoreUnstable(int x, int y) {
  static int _xLast = -1;
  static int _yLast = -1;
  int xOut, yOut;

  if ((x != -1) && (y != -1) && (_xLast != -1) && (_yLast != -1)) {
    xOut = _xLast;    
    yOut = _yLast;    
  } else {
    xOut = -1;
    yOut = -1;    
  }
  _xLast = x;
  _yLast = y;
  GUI_TOUCH_StoreUnstable(xOut, yOut);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_TOUCH_SetOrientationPhys
*/
void GUI_TOUCH_SetOrientationPhys(unsigned Orientation) {
  _OrientationPhys = Orientation;
}

/*********************************************************************
*
*       GUI_TOUCH_SetOrientation
*/
void GUI_TOUCH_SetOrientation(unsigned Orientation) {
  _OrientationLog = Orientation;
}

/*********************************************************************
*
*       GUI_TOUCH_GetxPhys
*/
int  GUI_TOUCH_GetxPhys(void) {
  return _xPhys;
}

/*********************************************************************
*
*       GUI_TOUCH_GetyPhys
*/
int  GUI_TOUCH_GetyPhys(void) {
  return _yPhys;
}

/*********************************************************************
*
*       GUI_TOUCH_SetDefaultCalibration
*/
void GUI_TOUCH_SetDefaultCalibration(void) {
  _xyMinMax[0].Min = GUI_TOUCH_AD_LEFT;
  _xyMinMax[0].Max = GUI_TOUCH_AD_RIGHT;
  _xyMinMax[1].Min = GUI_TOUCH_AD_TOP;
  _xyMinMax[1].Max = GUI_TOUCH_AD_BOTTOM;
}

/*********************************************************************
*
*       GUI_TOUCH_Calibrate
*/
int GUI_TOUCH_Calibrate(int Coord, int Log0, int Log1, int Phys0, int Phys1) {
  int l0, l1, xSize, ySize;
  if ((_OrientationLog & GUI_SWAP_XY) == 0) {
    xSize = LCD_GetXSizeEx(GUI_Context.SelLayer);
    ySize = LCD_GetYSizeEx(GUI_Context.SelLayer);
  } else {
    xSize = LCD_GetYSizeEx(GUI_Context.SelLayer);
    ySize = LCD_GetXSizeEx(GUI_Context.SelLayer);
  }
  l0 = 0;
  l1 = (Coord == GUI_COORD_X) ? xSize - 1 : ySize - 1;
  if (labs(Phys0 - Phys1) < 20) {
    return 1;
  }
  if (labs(Log0 - Log1) < 20) {
    return 1;
  }
  _xyMinMax[Coord].Min = _Log2Phys(l0, Log0, Log1, Phys0, Phys1);  
  _xyMinMax[Coord].Max = _Log2Phys(l1, Log0, Log1, Phys0, Phys1);
  _CalibrationDone = 1;
  return 0;
}

/*********************************************************************
*
*       GUI_TOUCH_GetCalData
*/
void GUI_TOUCH_GetCalData(int Coord, int* pMin,int* pMax) {
  *pMin = _xyMinMax[Coord].Min;
  *pMax = _xyMinMax[Coord].Max; 
}
  
/*********************************************************************
*
*       GUI_TOUCH_Exec
*/
void GUI_TOUCH_Exec(void) {
  #ifndef WIN32
  static U8 ReadState;
  int x, y;
  int xLog, yLog;
  int xSize, ySize;
  if (_CalibrationDone == 0) {
    if ((((_OrientationPhys & GUI_SWAP_XY) == 0) && ((_OrientationPhys & GUI_MIRROR_X) == 0)) || (((_OrientationPhys & GUI_SWAP_XY) == 1) && ((_OrientationPhys & GUI_MIRROR_Y) == 0))) {
      _xyMinMax[0].Min = GUI_TOUCH_AD_LEFT;
      _xyMinMax[0].Max = GUI_TOUCH_AD_RIGHT;
    } else {
      _xyMinMax[0].Min = GUI_TOUCH_AD_RIGHT;
      _xyMinMax[0].Max = GUI_TOUCH_AD_LEFT;
    }
    if ((((_OrientationPhys & GUI_SWAP_XY) == 0) && ((_OrientationPhys & GUI_MIRROR_Y) == 0)) || (((_OrientationPhys & GUI_SWAP_XY) == 1) && ((_OrientationPhys & GUI_MIRROR_X) == 0))) {
      _xyMinMax[1].Min = GUI_TOUCH_AD_TOP;
      _xyMinMax[1].Max = GUI_TOUCH_AD_BOTTOM;
    } else {
      _xyMinMax[1].Min = GUI_TOUCH_AD_BOTTOM;
      _xyMinMax[1].Max = GUI_TOUCH_AD_TOP;
    }
    _CalibrationDone = 1;
  }
  /* calculate Min / Max values */
  if (_xyMinMax[GUI_COORD_X].Min < _xyMinMax[GUI_COORD_X].Max) {
    _xMin = _xyMinMax[GUI_COORD_X].Min;
    _xMax = _xyMinMax[GUI_COORD_X].Max;
  } else {
    _xMax = _xyMinMax[GUI_COORD_X].Min;
    _xMin = _xyMinMax[GUI_COORD_X].Max;
  }
  if (_xyMinMax[GUI_COORD_Y].Min < _xyMinMax[GUI_COORD_Y].Max) {
    _yMin = _xyMinMax[GUI_COORD_Y].Min;
    _yMax = _xyMinMax[GUI_COORD_Y].Max;
  } else {
    _yMax = _xyMinMax[GUI_COORD_Y].Min;
    _yMin = _xyMinMax[GUI_COORD_Y].Max;
  }
  /* Execute the state machine which reads the touch */
  switch (ReadState) {
  case 0:
    _yPhys = TOUCH_X_MeasureY();
    TOUCH_X_ActivateY();  /* Prepare X- measurement */
    ReadState++;
    break;
  default:
    _xPhys = TOUCH_X_MeasureX();
    TOUCH_X_ActivateX();  /* Prepare Y- measurement */
    /* Convert values into logical values */
    if ((_xPhys < _xMin) || (_xPhys > _xMax)  || (_yPhys < _yMin) || (_yPhys > _yMax)) {
      _StoreUnstable(-1, -1);
    } else {
      x = _AD2X(_xPhys);
      y = _AD2Y(_yPhys);
      /*
      * Consider logical orientation
      */
      if ((_OrientationLog & GUI_SWAP_XY) == 0) {
        xSize = LCD_GetXSizeEx(GUI_Context.SelLayer);
        ySize = LCD_GetYSizeEx(GUI_Context.SelLayer);
        if (_OrientationLog & GUI_MIRROR_X) {
          xLog = xSize - x - 1;
        } else {
          xLog = x;
        }
        if (_OrientationLog & GUI_MIRROR_Y) {
          yLog = ySize - y - 1;
        } else {
          yLog = y;
        }
      } else {
        xSize = LCD_GetYSizeEx(GUI_Context.SelLayer);
        ySize = LCD_GetXSizeEx(GUI_Context.SelLayer);
        if (_OrientationLog & GUI_MIRROR_X) {
          yLog = xSize - x - 1;
        } else {
          yLog = x;
        }
        if (_OrientationLog & GUI_MIRROR_Y) {
          xLog = ySize - y - 1;
        } else {
          xLog = y;
        }
      }
      _StoreUnstable(xLog, yLog);
    }
    /* Reset state machine */
    ReadState = 0;
    break;
  }
  #endif /* WIN32 */
}

#else

void GUI_TOUCH_DriverAnalog_C(void);  /* Avoid "no prototype" warnings */
void GUI_TOUCH_DriverAnalog_C(void) {}

#endif    /* defined(GUI_SUPPORT_TOUCH) && GUI_SUPPORT_TOUCH */

/*************************** End of file ****************************/
