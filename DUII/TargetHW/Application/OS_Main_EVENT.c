/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (C) 2005   SEGGER Microcontroller Systeme GmbH               *
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

----------------------------------------------------------------------
File    : OS_Main_EVENT.c
Purpose : Sample program for embOS using EVENT object
--------- END-OF-HEADER --------------------------------------------*/

#include "RTOS.h"

OS_STACKPTR int StackHP[128];          /* Task stacks */
OS_TASK TCBHP;                         /* Task-control-blocks */

/********************************************************************/

/****** Interface to HW module **************************************/

void HW_Wait(void);
void HW_Free(void);
void HW_Init(void);

/********************************************************************/
/****** HW module ***************************************************/

OS_STACKPTR int _StackHW[128];                       /* Task stack  */
OS_TASK _TCBHW;                              /* Task-control-block  */

/****** local data **************************************************/

static OS_EVENT _HW_Event;

/****** local functions *********************************************/

static void _HWTask(void) {
  /* Initialize HW functionallity    */
  OS_Delay(100);
  /* Init done, send broadcast to waiting tasks */
  HW_Free();
  while (1) {
    OS_Delay (40);
  }
}

/****** global functions ********************************************/

void HW_Wait(void) {
  OS_EVENT_Wait(&_HW_Event);
}

void HW_Free(void) {
  OS_EVENT_Set(&_HW_Event);
}

void HW_Init(void) {
  OS_EVENT_Create(&_HW_Event);
  OS_CREATETASK(&_TCBHW, "HWTask", _HWTask, 25, _StackHW);
}

/********************************************************************/
/********************************************************************/

static void HPTask(void) {
  HW_Wait();                      /* Wait until HW module is set up */
  while (1) {
    OS_Delay (50);
  }
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  HW_Init();
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 150, StackHP);
  OS_SendString("Start project will start multitasking !\n");
  HW_Wait();                      /* Wait until HW module is set up */
  while (1) {
    OS_Delay (200);
  }
}

