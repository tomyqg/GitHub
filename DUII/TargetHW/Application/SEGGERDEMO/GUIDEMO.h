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
File        : GUIDEMO.h
Purpose     : Multiple choice for emWin
----------------------------------------------------------------------
*/


#ifndef GUIDEMO_H
#define GUIDEMO_H

#include "LCD_ConfDefaults.h"  /* valid LCD configuration */


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*      GUIDEMO publics
*
*********************************************************************

  The following are the routines which are available to the
  demo modules.
*/

void GUIDEMO_main(void);

/*********************************************************************
*
*      GUIDEMO main routines
*
*********************************************************************

  The following are the routines which are available to the
  demo modules.
*/

GUI_COLOR GUIDEMO_SetBkColor      (GUI_COLOR Color);
GUI_COLOR GUIDEMO_SetColor        (GUI_COLOR Color);
void      GUIDEMO_Wait            (void);
int       GUIDEMO_Delay           (int t);
void      GUIDEMO_ShowIntro       (const char*s, const char* sExplain);
void      GUIDEMO_ShowInfo        (const char* sExplain);
int       GUIDEMO_CheckCancel     (void);
void      GUIDEMO_NotifyStartNext (void);
void      GUIDEMO_HideInfoWin     (void);
int       GUIDEMO_WaitKey         (void);
void      GUIDEMO_ShowControlWin  (int Show);


extern const GUI_BITMAP GUIDEMO_bm4bpp;

/*********************************************************************
*
*      Demo modules
*
*********************************************************************

  The demo consists of different modules which are usually called
  by the main routine.
  The following are the declarations for the individual routines.
*/

void GUIDEMO_Automotive(void);
void GUIDEMO_Bitmap(void);
void GUIDEMO_Circle(void);
void GUIDEMO_Cursor(void);







void GUIDEMO_DemoBITMAPObj(void);





void GUIDEMO_DemoFrameWin(void);
void GUIDEMO_DemoLISTBOX(void);
void GUIDEMO_DemoLUT(void);
void GUIDEMO_DemoPolygon(void);
void GUIDEMO_DemoProgBar(void);
void GUIDEMO_DemoWidgets(void);
void GUIDEMO_Dialog(void);
void GUIDEMO_Graph(void);
void GUIDEMO_HardwareInfo(void);
void GUIDEMO_IconView(void);
void GUIDEMO_ImageFlow(void);
void GUIDEMO_Intro(void);
void GUIDEMO_Listview(void);
void GUIDEMO_Messagebox(void);
void GUIDEMO_Navigation(void);
void GUIDEMO_ShowColorBar(void);
void GUIDEMO_ShowColorList(void);
void GUIDEMO_ShowInternationalFonts(void);
void GUIDEMO_ShowMemoryDevice(void);
void GUIDEMO_Speed(void);
void GUIDEMO_Touch(void);
void GUIDEMO_Treeview(void);
void GUIDEMO_VScreen(void);
void GUIDEMO_WashingMachine(void);

void GUIDEMO_VScreen(void);

extern const GUI_BITMAP bmemWinGSCLogo320x46;
extern const GUI_BITMAP bmSeggerLogoBlue;

/*********************************************************************
*
*      GUIDEMO defines
*
*********************************************************************
*/

#ifndef GUIDEMO_SHOW_SPRITES
  #define GUIDEMO_SHOW_SPRITES 1
#endif

#if defined(__cplusplus)
  }
#endif

#endif

