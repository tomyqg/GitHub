/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*           (C) 2003 - 2008 SEGGER Microcontroller GmbH & Co. KG     *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : BSP.h
Purpose : BSP (Board support package)
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef BSP_H                           /* avoid multiple inclusion */
#define BSP_H

#include "SEGGER.h"

/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/
#define KEY_STAT_UP       (1 << 0)
#define KEY_STAT_DOWN     (1 << 1)
#define KEY_STAT_LEFT     (1 << 2)
#define KEY_STAT_RIGHT    (1 << 3)
#define KEY_STAT_BUTTON1  (1 << 4)
#define KEY_STAT_BUTTON2  (1 << 5)

/* In order to avoid warnings for undefined parameters */
#ifndef BSP_USE_PARA
  #if defined(NC30) || defined(NC308)
    #define BSP_USE_PARA(para)
  #else
    #define BSP_USE_PARA(para) para=para;
  #endif
#endif

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

/*********************************************************************
*
*       General
*/
void     BSP_Init       (void);
void     BSP_SetLED     (int Index);
void     BSP_ClrLED     (int Index);
void     BSP_ToggleLED  (int Index);
unsigned BSP_GetKeyStat (void);

/*********************************************************************
*
*       GUI
*/
void BSP_GUI_Init(void);

/*********************************************************************
*
*       USB
*/
void BSP_USB_Attach         (void);
void BSP_USB_InstallISR     (void (*pfISR)(void));
void BSP_USB_InstallISR_Ex  (int ISRIndex, void (*pfISR)(void), int Prio);
void BSP_USB_ISR_Handler    (void);

/*********************************************************************
*
*       USBH
*/
void BSP_USBH_InstallISR    (void (*pfISR)(void));
void BSP_USBH_Init          (void);

/*********************************************************************
*
*       ETH
*
*  Functions for ethernet controllers (as far as present)
*/
void   BSP_ETH_Init           (unsigned Unit);
void   BSP_ETH_InstallISR     (void (*pfISR)(void));
void   BSP_ETH_InstallISR_Ex  (int ISRIndex, void (*pfISR)(void), int Prio);
void   BSP_ETH_ISR_Handler    (void);
U32    BSP_ETH_GetTransferMem (U32 * pPAddr, U32 * pVAddr);

/*********************************************************************
*
*       CACHE
*/
void BSP_CACHE_CleanInvalidateRange (void * p, unsigned NumBytes);
void BSP_CACHE_CleanRange           (void * p, unsigned NumBytes);
void BSP_CACHE_InvalidateRange      (void * p, unsigned NumBytes);

/*********************************************************************
*
*       UART
*/
void BSP_UART_SetReadCallback(unsigned Unit, void (*pfOnRx) (unsigned Unit, unsigned char Data));
void BSP_UART_SetWriteCallback(unsigned Unit, void (*pfOnTx) (unsigned Unit));
void BSP_UART_Write1(U8 Data);
void BSP_UART_SetBaudrate(unsigned Baudrate);
void BSP_UART_Init(void);

/*********************************************************************
*
*       SD  (used by file system)
*/

/***************************************************
*
*       BSP_SD_GetTransferMem
*
*  Function description
*     Delivers a memory area to be used by the SD-Card controller as transfer.
*     This function delivers the physical address and the virtual address of the tranfer memory.
*     The transfer area needs to be:
*     - Word aligned
*     - Uncached
*     - Have identical virtual and physical addresses
*     - The virtual address of the transfer area must be non-cacheable.
*     Additional requirements are that the memory used is fast enough to not block DMA transfers for too long.
*     In most systems, IRAM is used instead of external SDRAM, since the SDRAM can have relatively long latencies, primarily due to refresh cycles.
*     The size of the memory are is also returned (via pointer). It needs to be at least 512 bytes. In general, bigger values allow higher
*     performance since it allows transfer of multiple sectors without break.
*/
U32 BSP_SD_GetTransferMem(U32 * pPAddr, U32 * pVAddr);

#endif                                  /* avoid multiple inclusion */

/*************************** End of file ****************************/


