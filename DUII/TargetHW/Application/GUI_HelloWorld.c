/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2008  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS version: 3.52e                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : GUI_HelloWorld.c
Purpose : emWin and embOS demo application
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       main()
*/
void MainTask(void);
void MainTask(void) {
  GUI_Init();
  GUI_DispString("Hello World!");
  while(1) {
    GUI_Delay(500);
  }
}

/****** End of File *************************************************/
