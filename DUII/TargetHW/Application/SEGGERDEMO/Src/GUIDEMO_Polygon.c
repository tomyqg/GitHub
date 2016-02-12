/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2008     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : GUIDEMO_Polygon.c
Purpose     : Several GUIDEMO routines
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "GUIDEMO.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static const GUI_POINT _aArrow[] = {
  {  0,  30 },
  {-40,   0 },
  {-10,  10 },
  {-10, -50 },
  { 10, -50 },
  { 10,  10 },
  { 40,   0 }
};

static const GUI_POINT _aTriangle[] = {
	{  0,  0 }, 
  {-30, 30 },
  { 30, 30 },
};

#if GUI_SUPPORT_AA
static const GUI_POINT _aiCursor[] = {
  {  0,  -10 }, 
  { 50,    0 }, 
  {  0, -100 }, 
  {-50,    0 }
};
#endif

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _MagnifyPolygon
*/
static void _MagnifyPolygon(GUI_POINT* pDest, const GUI_POINT* pSrc, int NumPoints, float Mag) {
  int j;
  for (j = 0; j < NumPoints; j++) {
    (pDest + j)->x = (int)((pSrc + j)->x * Mag);
    (pDest + j)->y = (int)((pSrc + j)->y * Mag);
  }
}

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_DemoPolygon
*/
void GUIDEMO_DemoPolygon(void) {
  GUI_COLOR Color;
  int XMid  = (LCD_GetXSize() >> 1);
  int YMid  = (LCD_GetYSize() >> 1);
  int lcd_xsize;
  int lcd_ysize;
  lcd_xsize = LCD_GetXSize();
  lcd_ysize = LCD_GetYSize();
  GUIDEMO_ShowIntro("Arbitrary Polygons", 
                    "\nStandard and antialiased");
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_FontComic18B_ASCII);
  GUI_DispStringAt("Arbitrary\nPolygons", 0, 0);
  YMid += 10;
  if ((lcd_xsize < 320) || (lcd_ysize < 240)) { //#if ((LCD_XSIZE != 320) || (LCD_YSIZE != 240))
    GUI_POINT aPolygon[8];
    float Factor;
    if ((lcd_xsize / 320) < (lcd_ysize / 240)) {
      Factor = LCD_GetXSize() / 320.f;
    } else {
      Factor = LCD_GetYSize() / 240.f;
    }
    /* Draw arrows */
    _MagnifyPolygon(aPolygon, _aArrow, GUI_COUNTOF(_aArrow), Factor);
    GUI_FillPolygon(aPolygon, 7, (int)(XMid - (50 * Factor)), YMid);
    GUI_FillPolygon(aPolygon, 7, (int)(XMid + (50 * Factor)), YMid);
    /* Draw triangle */
    _MagnifyPolygon(aPolygon, _aTriangle, GUI_COUNTOF(_aTriangle), Factor);
    Color = GUIDEMO_SetColor(GUI_GREEN);
    if (Color != GUI_GREEN) {
      GUI_SetColor(GUI_YELLOW);
    }
    GUI_FillPolygon(aPolygon, 3, XMid, (int)(YMid + (10 * Factor)));
    /* Draw cursor */
    #if GUI_SUPPORT_AA
      GUI_SetColor(GUI_WHITE);
      GUI_AA_EnableHiRes();
      _MagnifyPolygon(aPolygon, _aiCursor, GUI_COUNTOF(_aiCursor), Factor);
      GUI_AA_FillPolygon(aPolygon, 4, XMid * 3, (int)((YMid - (10 * Factor)) * 3));
    #endif
  } else {
    GUI_FillPolygon(_aArrow, 7, XMid - 50, YMid);
    GUI_FillPolygon(_aArrow, 7, XMid + 50, YMid);
    /* Draw triangle */
    Color = GUIDEMO_SetColor(GUI_GREEN);
    if (Color != GUI_GREEN) {
      GUI_SetColor(GUI_YELLOW);
    }
    GUI_FillPolygon(_aTriangle, 3, XMid, YMid + 10);
    /* Draw cursor */
    #if GUI_SUPPORT_AA
      GUI_SetColor(GUI_WHITE);
      GUI_AA_EnableHiRes();
      GUI_AA_FillPolygon((GUI_POINT*)_aiCursor, 4, XMid * 3, (YMid - 10) * 3);
    #endif
  }
  GUIDEMO_Wait();
}
