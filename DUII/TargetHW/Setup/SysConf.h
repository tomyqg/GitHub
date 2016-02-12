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

File    : SysConf.h
Purpose : Configuration settings for including middleware components
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef SYSCONF_H                       /* Avoid multiple inclusion */
#define SYSCONF_H

#ifndef   INCLUDE_IP
  #define INCLUDE_IP  (0)
#endif

#ifndef   INCLUDE_USB
  #define INCLUDE_USB (0)
#endif

#endif                                  /* Avoid multiple inclusion */

/*****  EOF  ********************************************************/