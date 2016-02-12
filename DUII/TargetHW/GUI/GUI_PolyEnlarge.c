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
File        : GUI_PolyEnlarge.c
Purpose     : Polygon enlarge
----------------------------------------------------------------------
*/

#include <math.h>

#include "GUI.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  float x, y;
} tfPoint;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _fround
*/
static int _fround(float f) {
  if (f > 0) {
    return (int)(f + 0.5);
  }
  return (int)(f - 0.5);
}

/*********************************************************************
*
*       _Normalize
*/
static void _Normalize(tfPoint * pfPoint) {
  float fx;
  float fy;
  float r;

  fx = pfPoint->x;
  fy = pfPoint->y;
  r = sqrt(fx * fx + fy * fy);
  if (r > 0) {
    pfPoint->x = fx / r;
    pfPoint->y = fy / r;
  }
}

/*********************************************************************
*
*       _ReverseLen
*/
static void _ReverseLen(tfPoint * pfPoint) {
  float fx;
  float fy;
  float r;
  
  fx = pfPoint->x;
  fy = pfPoint->y;
  r = sqrt((fx * fx + fy * fy) / 2);
  if (r > 0) {
    pfPoint->x = fx / r / r;
    pfPoint->y = fy / r / r;
  }
}

/*********************************************************************
*
*       _GetVect
*/
static void _GetVect(tfPoint * pfPoint, const GUI_POINT * pSrc, unsigned NumPoints, unsigned Index) {
  int Off0;
  int Off1;
  
  Off0 = (Index + NumPoints - 1) % NumPoints;
  Off1 = Index % NumPoints;
  pfPoint->x = pSrc[Off1].x - pSrc[Off0].x; 
  pfPoint->y = pSrc[Off1].y - pSrc[Off0].y; 
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_EnlargePolygon
*/
#if 0
void GUI_EnlargePolygon(GUI_POINT * pDest, const GUI_POINT * pSrc, int NumPoints, int Len) {
  int x, y, j;
  tfPoint aVect[2];
  
  /* Calc destination points */
  for (j = 0; j < NumPoints; j++) {
    /* Get the vectors */
    _GetVect(&aVect[0], pSrc, NumPoints, j);
    _GetVect(&aVect[1], pSrc, NumPoints, j + 1);
    /* Normalize the vectors and add vectors */
    _Normalize(&aVect[0]);
    _Normalize(&aVect[1]);
    aVect[0].x += aVect[1].x;
    aVect[0].y += aVect[1].y;
    /* Resulting vector needs to be normalized again */
    _Normalize(&aVect[0]);
    x =  _fround(aVect[0].y * Len);
    y = -_fround(aVect[0].x * Len);
    /* Store destination */
    (pDest + j)->x = (pSrc + j)->x + x;
    (pDest + j)->y = (pSrc + j)->y + y;
  }
}

#else

void GUI_EnlargePolygon(GUI_POINT * pDest, const GUI_POINT * pSrc, int NumPoints, int Len) {
  int x, y, j;
  tfPoint aVect[2];
  
  /* Calc destination points */
  for (j = 0; j < NumPoints; j++) {
    /* Get the vectors */
    _GetVect(&aVect[0], pSrc, NumPoints, j);
    _GetVect(&aVect[1], pSrc, NumPoints, j + 1);
    /* Normalize the vectors and add vectors */
    _Normalize(&aVect[0]);
    _Normalize(&aVect[1]);
    aVect[0].x += aVect[1].x;
    aVect[0].y += aVect[1].y;
    /* Resulting vector needs to be normalized again */
    _ReverseLen(&aVect[0]);
    x =  _fround(aVect[0].y * Len);
    y = -_fround(aVect[0].x * Len);
    /* Store destination */
    (pDest + j)->x = (pSrc + j)->x + x;
    (pDest + j)->y = (pSrc + j)->y + y;
  }
}
#endif

/*************************** End of file ****************************/
