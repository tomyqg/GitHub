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
File        : GUIDEV_16.c
Purpose     : Implementation of memory devices
---------------------------END-OF-HEADER------------------------------
*/


#include <string.h>

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/

#define PIXELINDEX                      U16
#define BITSPERPIXEL                     16
#define API_LIST      GUI_MEMDEV__APIList16
#define DEVICE_API     GUI_MEMDEV_DEVICE_16

/*********************************************************************
*
*       _DrawBitLine16BPP_DDB
*/
static void _DrawBitLine16BPP_DDB(GUI_DEVICE * pDevice, GUI_USAGE * pUsage, int x, int y, const U16 * pSrc, int xsize, PIXELINDEX * pDest) {
  GUI_USE_PARA(pDevice);
  if (pSrc && pDest) {
    GUI_MEMCPY(pDest, pSrc, xsize * 2);
    if (pUsage) {
      GUI_USAGE_AddHLine(pUsage, x, y, xsize);
    }
  }
}

/*********************************************************************
*
*       _DrawBitLine32BPP
*/
/**///TBD!!!
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, GUI_USAGE * pUsage, int x, int y, const U32 * pSrc, int xsize, PIXELINDEX * pDest) {
  if (pSrc && pDest) {
    if (pUsage) {
      GUI_USAGE_AddHLine(pUsage, x, y, xsize);
    }
    do {
      U32 Color;
      U16 Index;
      
      Color = *pSrc;
      Index = pDevice->pColorConvAPI->pfColor2Index(Color); //GUI_MEMDEV__APIList16.pfColor2Index(Color);
      *pDest = Index;
      if (pUsage) {
        GUI_USAGE_AddPixel(pUsage, x, y);
      }
      x++;
      pDest++;
      pSrc++;
    } while (--xsize);
  }
}

/*********************************************************************
*
*       #include GUIDev code
*
**********************************************************************
*
* The main code is in an other "C" file.
* This has been done to avoid duplication of code.
*/
#include "GUIDEV_8.c"

#else

void GUIDEV16_C(void);
void GUIDEV16_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
