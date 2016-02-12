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
File        : GUIValf.c
Purpose     : Displaying floating point values
---------------------------END-OF-HEADER------------------------------
*/

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
*       _RoundHalfUp
*
* Purpose:
*   Rounds the given value with the 'Symmetric Arithmetic Rounding'
*   method, also known as 'Round-Half-Up':
*
*    3.044  rounded to hundredths is  3.04
*    3.045  rounded to hundredths is  3.05
*    3.0447 rounded to hundredths is  3.04
*   -2.1349 rounded to hundredths is -2.13
*   -2.1350 rounded to hundredths is -2.14
*/
static float _RoundHalfUp(float f) {
  if (f < 0) {
    /*
    * Manage negative values
    */
    f -= 0.5f;
    f = (float)ceil(f);
  } else {
    /*
    * Manage positive values
    */
    f += 0.5f;
    f = (float)floor(f);
  }
  return f;
}

/*********************************************************************
*
*       _DispFloatFix
*
* Purpose:
*   The function draws the given floating point value, which can be
*   positive or negative:
*
* Parameters:
*   f            - Value to be shown
*   Len          - Number of digits including the decimal point
*   Decs         - Number of decimal places
*   DrawPlusSign - If != 0 a plus sign is shown for a positive value
*/
static void _DispFloatFix(float f, char Len, char Decs, int DrawPlusSign) {
  f *= GUI_Pow10[(unsigned)Decs];
  f = _RoundHalfUp(f);
  if (DrawPlusSign) {
    /*
    * Draw with plus sign
    */
    GUI_DispSDecShift((long)f, Len, Decs);
  } else {
    /*
    * Draw without plus sign
    */
    GUI_DispDecShift((long)f, Len, Decs);
  }
}

/*********************************************************************
*
*       _GetDecs
*/
static int _GetDecs(float f) {
  float fShift;
  char Decs = 0;
  fShift = f;
  if (fShift < 0) {
    fShift = -fShift;
  }
  fShift += 0.000000001f;
  if (fShift != 0) {
    do {
      fShift *= 10;
      Decs ++;
    } while ((unsigned)fShift == 0);
  }
  return Decs;
}

/*********************************************************************
*
*       _DispFloatMin
*/
static void _DispFloatMin(float f, char Fract, int DrawPlusSign) {
  int Len;
  float fRound; /* Value only used to calculate the length */
  int Decs;
  
  fRound = f;
  if (f == 0) {
    fRound = 1;
  } else if ((f >= 0) && (f < 1)) { /* Values between 0 and 1 */
    Decs = _GetDecs(f);
    if (Decs <= Fract) {
      fRound += 1;
    } else {
      if (f >= 0.5f) {
        if (Fract) {
          fRound = 1;
        } else {
          fRound = 0;
          f      = 1;
        }
      } else {
        if (Fract) {
          fRound = 1;
          f      = 0;
        } else {
          fRound = 0;
          f      = 0;
        }
      }
    }
  } else if ((f < 0) && (f > -1)) { /* Values between 0 and -1 */
    Decs = _GetDecs(f);
    if (Decs <= Fract) {
      fRound -= 1;
    } else {
      if (f <= -0.5f) {
        if (Fract) {
          fRound = 1;
        } else {
          f = fRound = -1;
        }
      } else {
        if (Fract) {
          fRound = 1;
        } else {
          f = fRound = 0;
        }
      }
    }
  }
  fRound = fRound * GUI_Pow10[(unsigned)Fract];
  fRound = _RoundHalfUp(fRound);
  Len = GUI_Long2Len((long)fRound) - Fract;
  if (Len < 0) {
    Len = 0;
  }
  if ((DrawPlusSign == 1) && (f > 0)) { /* Add one character for the plus sign if required */
    Len++;
  }
  _DispFloatFix(f, (char)(Len + Fract + (Fract ? 1 : 0)), (char)Fract, DrawPlusSign);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DispFloatFix
*/
void GUI_DispFloatFix(float f, char Len, char Decs) {
  _DispFloatFix(f, Len, Decs, 0);
}

/*********************************************************************
*
*       GUI_DispFloatMin
*/
void GUI_DispFloatMin(float f, char Fract) {
  _DispFloatMin(f, Fract, 0);
}

/*********************************************************************
*
*       GUI_DispFloat
*/
void GUI_DispFloat(float f, char Len) {
  int Decs;
  Decs =  Len - GUI_Long2Len((long)f) - 1;
  if ((f < 0) && (f > -1)) { /* If value < 0 and > -1 (e.g. -0.123) decrement Decs */
    Decs--;
  }
  if (Decs < 0) {
    Decs = 0;
  }
  _DispFloatFix(f, Len, (char)Decs, 0);
}

/*********************************************************************
*
*       GUI_DispSFloatFix
*/
void GUI_DispSFloatFix(float f, char Len, char Fract) {
  _DispFloatFix (f, Len, Fract, 1);
}

/*********************************************************************
*
*       GUI_DispSFloatMin
*/
void GUI_DispSFloatMin(float f, char Fract) {
  _DispFloatMin(f, Fract, 1);
}

/*************************** End of file ****************************/
