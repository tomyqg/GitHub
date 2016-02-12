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
File    : OS_Main_OS_Q.c
Purpose : Sample program for embOS using queues
--------- END-OF-HEADER --------------------------------------------*/

#include "RTOS.h"

OS_STACKPTR int StackHP[128];          /* Task stacks */
OS_TASK TCBHP;                         /* Task-control-blocks */

/********************************************************************/

static OS_Q _MyQ;
static char _MyQBuffer[100];
	
static void HPTask(void) {
  char* pData;
  while (1) {
    int Len;
    Len = OS_Q_GetPtr(&_MyQ, (void**)&pData);
    OS_Delay(10);
    if (Len) {  /* Evaluate Message ... */
      OS_SendString(pData);
      OS_Q_Purge(&_MyQ);
    }
  }
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  OS_Q_Create(&_MyQ, &_MyQBuffer, sizeof(_MyQBuffer));
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 150, StackHP);
  OS_SendString("embOS OS_Q example");
  OS_SendString("\n\nDemonstrating message passing\n");
  while (1) {
    OS_Q_Put(&_MyQ, "\nHello", 7);
    OS_Q_Put(&_MyQ, "\nWorld !", 9);
    OS_Delay (500);
  }
}

