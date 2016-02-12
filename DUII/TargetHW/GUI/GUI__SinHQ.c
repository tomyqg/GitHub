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
File        : GUI__SinHQ.c
Purpose     : Integer based sin and cos functions
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define DEG_90   90000L
#define DEG_180 180000L
#define DEG_270 270000L
#define DEG_360 360000L

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/
/*********************************************************************
*
*       _aSin
*
* Purpose:
*   Array of sinus values for 0 - 90 degrees (<< 16)
*
**********************************************************************
*/
static const U32 _aSin[] = {
  0    , 1144 , 2287 , 3430 , 4572 , 5712 , 6850 ,
  7987 , 9121 , 10252, 11380, 12505, 13626, 14742,
  15855, 16962, 18064, 19161, 20252, 21336, 22415,
  23486, 24550, 25607, 26656, 27697, 28729, 29753,
  30767, 31772, 32768, 33754, 34729, 35693, 36647,
  37590, 38521, 39441, 40348, 41243, 42126, 42995,
  43852, 44695, 45525, 46341, 47143, 47930, 48703,
  49461, 50203, 50931, 51643, 52339, 53020, 53684,
  54332, 54963, 55578, 56175, 56756, 57319, 57865,
  58393, 58903, 59396, 59870, 60326, 60764, 61183,
  61584, 61966, 62328, 62672, 62997, 63303, 63589,
  63856, 64104, 64332, 64540, 64729, 64898, 65048,
  65177, 65287, 65376, 65446, 65496, 65526, 65536,
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__SinHQ
*
* Purpose:
*   Returns the sinus value for the given angle (<< 16)
*
* Parameter:
*   Requested angle (in degrees) * 1000
*
* Return value:
*   sin << 16
*/
I32 GUI__SinHQ(I32 Ang1000) {
  I32 r;
  int Index;
  int Mul;
  int Remain;

  if (Ang1000 < 0) {
    Mul = -1;
    Ang1000 = -Ang1000;
  } else {
    Mul = 1;
  }
  Ang1000 %= DEG_360;
  if (Ang1000 > DEG_180) {
    Mul = -Mul;
    Ang1000 -= DEG_180;
  }
  if (Ang1000 > DEG_90) {
    Ang1000 = DEG_90 - (Ang1000 - DEG_90);
  }
  Index  = Ang1000 / 1000;
  Remain = Ang1000 - Index * 1000;
  r = (_aSin[Index] + ((_aSin[Index + 1] - _aSin[Index]) * Remain) / 1000);
  r = (Mul > 0) ? r : -r;
  return r;
}

/*********************************************************************
*
*       GUI__CosHQ
*
* Purpose:
*   Returns the cosine value for the given angle (<< 16)
*
* Parameter:
*   Requested angle (in degrees) * 1000
*
* Return value:
*   cos << 16
*/
I32 GUI__CosHQ(I32 Ang1000) {
  return GUI__SinHQ(Ang1000 + DEG_90);
}

/*************************** end of file ****************************/
