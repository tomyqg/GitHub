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
*       embOS version: 3.32e                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : OS_Start_LEDBlink.c
Purpose : Sample program for OS running on EVAL-boards with LEDs
--------- END-OF-HEADER --------------------------------------------*/


#include "RTOS.h"
#include "BSP.h"

OS_STACKPTR int _Stack[128];          // Task stacks
OS_TASK         _TCB;                 // Task-control-blocks


/*********************************************************************
*
*       HPTask
*/
static void HPTask(void) {
  while (1) {
    BSP_ToggleLED(0);
    OS_Delay (50);
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
    BSP_ToggleLED(1);
    OS_Delay (200);
  }
}

/*************************** End of file ****************************/
