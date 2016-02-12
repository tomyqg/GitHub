/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (C) 2006   SEGGER Microcontroller Systeme GmbH               *
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
*       embOS version: 3.32a                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : OS_Main_TaskEx.c
Purpose : Sample program for embOS using OC_CREATETASK_EX
--------- END-OF-HEADER --------------------------------------------*/

#include "RTOS.h"

OS_STACKPTR int StackHP[128], StackLP[128];          /* Task stacks */
OS_TASK TCBHP, TCBLP;                        /* Task-control-blocks */

/********************************************************************/

static void TaskEx(void* pData) {
  while (1) {
    OS_Delay ((OS_TIME) pData);
  }
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  OS_CREATETASK_EX(&TCBHP, "HP Task", TaskEx, 100, StackHP, (void*) 50);
  OS_CREATETASK_EX(&TCBLP, "LP Task", TaskEx,  50, StackLP, (void*) 200);
  OS_SendString("Start project will start multitasking !\n");
  OS_Terminate(0);
}

