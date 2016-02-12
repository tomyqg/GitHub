/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2007  SEGGER Microcontroller Systeme GmbH         *
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
*       embOS version: 3.32p                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : OS_Start_2Tasks.c
Purpose : Skeleton program for OS
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"

OS_STACKPTR int _Stack[128];          // Task stacks
OS_TASK         _TCB;                 // Task-control-blocks


/*********************************************************************
*
*       HPTask
*/
static void HPTask(void) {
  while (1) {
    OS_Delay (10);
  }
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  OS_CREATETASK(&_TCB, "HPTask", HPTask, 150, _Stack);
  while (1) {
    OS_Delay (50);
  }
}

/*************************** End of file ****************************/
