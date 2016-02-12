/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2009     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef OS_CONFIG_H                     /* Avoid multiple inclusion */
#define OS_CONFIG_H

/*********************************************************************
*
*       Configuration for RTOS build and UART
*
*  One of the following builds needs to be selected for both DEBUG and Release builds:
*
*  OS_LIBMODE_XR    Extremly small release build without Round robin
*  OS_LIBMODE_R     Release build
*  OS_LIBMODE_S     Release build with stack check
*  OS_LIBMODE_SP    Release build with stack check and profiling
*  OS_LIBMODE_D     Debug build
*  OS_LIBMODE_DP    Debug build with profiling
*  OS_LIBMODE_DT    Debug build with trace
*/

#ifndef   DEBUG
  #define DEBUG 0
#endif

#if DEBUG
  #define OS_LIBMODE_DP
#else
  #define OS_LIBMODE_R
  #define OS_VIEW_ENABLE   0
#endif

#endif                                  /* Avoid multiple inclusion */

/*************************** End of file ****************************/
