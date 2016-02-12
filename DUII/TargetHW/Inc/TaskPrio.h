/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (C) 2007   SEGGER Microcontroller Systeme GmbH               *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************

File    : Taskprio.h
Purpose : Configuration settings for task priorities
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef TASKPRIO_H                     /* Avoid multiple inclusion */
#define TASKPRIO_H

/*********************************************************************
*
*       Task priorities
*/
enum {
  TASKPRIO_TOUCH = 100,   // 100
  TASKPRIO_WINDOW,        // 101
  TASKPRIO_FTPSCHILD,     // 102
  TASKPRIO_FTPSPARENT,    // 103
  TASKPRIO_FTPS,          // 104
  TASKPRIO_WEBS,          // 105
  TASKPRIO_WEBSCHILD,     // 106
  TASKPRIO_WEBSPARENT,    // 107
  TASKPRIO_USB,           // 108
  TASKPRIO_MAINTASK,      // 109
  TASKPRIO_VNC,           // 110
  TASKPRIO_USBH_MAIN,     // 111
  TASKPRIO_USBH_ISR,      // 112
  TASKPRIO_IPMAIN,        // 113, should be higher than prio of server applications
  TASKPRIO_IPRX,          // 114, needs to have highest prio except for windows that need to be shown immediately
  TASKPRIO_TRIALWINDOW    // 115, highest prio, needs to be shown immediately
};


#endif                                  /* Avoid multiple inclusion */

/*****  EOF  ********************************************************/
