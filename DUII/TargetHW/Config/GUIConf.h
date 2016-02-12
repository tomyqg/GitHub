/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2009  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIConf.h
Purpose     : Configuration of available features and default values
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Multi layer/display support
*/
#define GUI_NUM_LAYERS            (16) // Maximum number of available layers

/*********************************************************************
*
*       Multi tasking support
*/
#define GUI_OS                    (1)  // Compile with multitasking support

/*********************************************************************
*
*       Configuration of available packages
*/
#ifndef   GUI_SUPPORT_TOUCH
  #define GUI_SUPPORT_TOUCH       (1)  // Support touchscreen
#endif
#define GUI_SUPPORT_MOUSE         (1)  // Support a mouse
#define GUI_SUPPORT_UNICODE       (1)  // Support mixed ASCII/UNICODE strings
#define GUI_WINSUPPORT            (1)  // Window manager package available
#define GUI_SUPPORT_MEMDEV        (1)  // Memory devices available
#define GUI_SUPPORT_AA            (1)  // Anti aliasing available
#define WM_SUPPORT_STATIC_MEMDEV  (1)  // Static memory devices available

// NOTE!  THE SIMULATOR BUILD HAS A DIFFERENT GUIConf.h!!!
// COPY CHANGES FROM HERE TO THERE
// KMC
#define	BUTTON_REACT_ON_LEVEL		1
/* Define colors */
//  #define BUTTON_BKCOLOR0_DEFAULT   GUI_BLUE
//  #define BUTTON_BKCOLOR1_DEFAULT   GUI_WHITE
//  #define BUTTON_BKCOLOR2_DEFAULT   GUI_LIGHTCYAN
  #define BUTTON_TEXTCOLOR0_DEFAULT GUI_WHITE
//  #define BUTTON_TEXTCOLOR1_DEFAULT GUI_BLACK
//  #define BUTTON_TEXTCOLOR2_DEFAULT GUI_LIGHTGREEN
  #define BUTTON_FOCUSCOLOR_DEFAULT GUI_GRAY
//  #define BUTTON_FRAMECOLOR_DEFAULT GUI_GRAY
#define	BUTTON_3D_MOVE_X	0
#define	BUTTON_3D_MOVE_Y	0

///*********************************************************************
//*
//*       Default font
//*/
//#define GUI_DEFAULT_FONT          &GUI_Font6x8

// Define ICONVIEW selected state background color as transparent
// = max alpha blending (0xFF) on BLACK - any color would do (0xxxxxxx)
#define ICONVIEW_BKCOLOR1_DEFAULT 0xFF000000

#endif  /* Avoid multiple inclusion */

/*************************** End of file ****************************/
