/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2004  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_ColorList.c
Purpose     : Shows color list
----------------------------------------------------------------------
*/

#include "LCD_ConfDefaults.h"  /* valid LCD configuration */
#include "GUI.h"
#include "GUIDEMO.h"

/*********************************************************************
*
*       GUIDEMO_ShowColorList
*
**********************************************************************
*/

#if (LCD_BITSPERPIXEL > 2) && (LCD_BITSPERPIXEL <= 8)

void GUIDEMO_ShowColorList(void) {
  int NumColors = LCD_GetDevCap(LCD_DEVCAP_NUMCOLORS);
  int Lines = (NumColors - 1) / 16 + 1;
  int y0;
  int lcd_xsize;
  int lcd_ysize;
  lcd_xsize = LCD_GetXSize();
  lcd_ysize = LCD_GetYSize();
  #if (LCD_BITSPERPIXEL > 8)
    int XStep = lcd_xsize / 256;
    int YStep = NumColors / 256;
  #endif
  int i;
  y0 = 70;
  GUIDEMO_ShowIntro("Available colors",
                    "Shows the first of the"
                    "\navailable colors in the"
                    "\nsimulated configuration");
  GUI_SetColor(GUI_WHITE);
  GUI_SetBkColor(GUI_BLACK); 
  GUI_Clear();
	GUI_SetFont(&GUI_Font8x16);
	GUI_DispString("Available colors\n");
  GUI_SetFont(&GUI_Font8_ASCII);
  #ifdef LCD_CONTROLLER
    GUI_DispString("\nLCD_CONTROLLER: ");
    GUI_DispDecMin(LCD_CONTROLLER);
    GUI_DispString("\n");
  #endif
  GUI_DispDecMin(LCD_BITSPERPIXEL);
  GUI_DispString(" bpp");
  #ifdef LCD_BUSWIDTH
    GUI_DispString(", ");
    GUI_DispDecMin(LCD_BUSWIDTH);
    GUI_DispString(" bit bus");
  #endif
  GUI_DispString(", ");
  GUI_DispDecMin(NumColors);
  GUI_DispString(" colors\n");
  #if (LCD_FIXEDPALETTE) 
    GUI_DispString("Fixed palette: ");
    GUI_DispDecMin(LCD_FIXEDPALETTE);
  #else
    GUI_DispString("(Custom)");
  #endif
  #if (LCD_BITSPERPIXEL <= 8)
  {
    int XStep = lcd_xsize / 16;
    #if (lcd_ysize < 320)
      int YStep = (lcd_ysize - y0) / Lines;
    #else
      int YStep = (lcd_ysize - y0 - 60) / Lines;
    #endif
    int dy = (YStep <= 3) ? 1 : 2;
	  for (i = 0; i < NumColors; i++) {
      GUI_SetColorIndex(i);
      GUI_FillRect((i%16) * XStep + 2,
                   (i/16) * YStep + y0 + 1,
                   (i%16 + 1) * XStep - 1,
                   (i/16 + 1) * YStep + y0 - dy);
    }
  }
  #else
  {
	  for (i = 0; i < NumColors; i++) {
      GUI_SetColorIndex(i);
      GUI_DrawPoint((U8)(i) + ((lcd_xsize - 1 - 256) >> 1), (i >> 8) + y0);
    }
  }
  #endif
  GUIDEMO_Delay(2500);
}

#else

void GUIDEMO_ShowColorList(void) {}

#endif
