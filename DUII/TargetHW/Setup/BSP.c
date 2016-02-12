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
*       embOS version: 3.50b                                         *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : BSP.c
Purpose : BSP for AT91SAM9RL64-EK
--------  END-OF-HEADER  ---------------------------------------------
*/

#define BSP_C
#include "SEGGER.h"
#include "SPI_API.h"
#include "BSP.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// KMC - No longer use eval boards LEDs - do not include Segger's LED control software
//
#if 0
/****** Addresses of sfrs and peripheral identifiers ****************/
#define PIOD_BASE_ADDR                     (0xFFFFFA00)
#define PMC_BASE_ADDR                      (0xFFFFFC00)
#define PIOD_ID                            (5)
#define PIO_PER_OFFS                       (0x00)
#define PIO_OER_OFFS                       (0x10)
#define PIO_SODR_OFFS                      (0x30)                               // Set output data
#define PIO_CODR_OFFS                      (0x34)                               // Clear output data register
#define PIO_ODSR_OFFS                      (0x38)                               // output data status register

/****** SFRs and identifiers used for LED demo **********************/
#define PMC_PCER                           (*(volatile U32*) (PMC_BASE_ADDR + 0x10))

/****** Assign LEDs to Ports ****************************************/
#define LEDPORT_BASE_ADDR                  (PIOD_BASE_ADDR)
#define LEDPORT_ID                         (PIOD_ID)
#define LED0_BIT                           (15)  // LED1 on AT91SAM9RL64-EK board, active low
#define LED1_BIT                           (16)  // LED2 on AT91SAM9RL64-EK board, active low

#define LEDPORT_PER                        (*(volatile U32*) (LEDPORT_BASE_ADDR + PIO_PER_OFFS))
#define LEDPORT_OER                        (*(volatile U32*) (LEDPORT_BASE_ADDR + PIO_OER_OFFS))
#define LEDPORT_SODR                       (*(volatile U32*) (LEDPORT_BASE_ADDR + PIO_SODR_OFFS))
#define LEDPORT_CODR                       (*(volatile U32*) (LEDPORT_BASE_ADDR + PIO_CODR_OFFS))
#define LEDPORT_STATE                      (*(volatile U32*) (LEDPORT_BASE_ADDR + PIO_ODSR_OFFS))
#define LEDMASK_ALL                        ((1 << LED0_BIT) | (1 << LED1_BIT))
/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       LED_Init()
*/
void BSP_Init(void) {
  PMC_PCER     |= (1 << LEDPORT_ID); /* Enable peripheral clock for LED-Port    */
  LEDPORT_PER   = LEDMASK_ALL;       /* enable Port           */
  LEDPORT_OER   = LEDMASK_ALL;       /* switch to output mode */
  LEDPORT_SODR  = LEDMASK_ALL;       /* Initially clear LEDs  */
}

/*********************************************************************
*
*       LED switching routines
*       LEDs are switched on with low level on port lines
*/
void BSP_SetLED(int Index) {
  LEDPORT_CODR = (1 << (Index + LED0_BIT));         // Switch on LED
}

void BSP_ClrLED(int Index) {
  LEDPORT_SODR = (1 << (Index + LED0_BIT));         // Switch off LED
}

void BSP_ToggleLED(int Index) {
  if (LEDPORT_STATE & (1 << (Index + LED0_BIT))) {  // LED is switched off
    BSP_SetLED(Index);
  } else {
    BSP_ClrLED(Index);
  }
}

#endif

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
U32 BSP_SD_GetTransferMem(U32 * pPAddr, U32 * pVAddr) {
  //
  // We use the highest 4 KB of the IRAM as transfer address
  //

  *pPAddr = (U32) (TOP_OF_ISRAM - DMA_SIZE);    // Transfer address used by the DMA.
  *pVAddr = *pPAddr;  							  // Must be non-cachable
  return DMA_SIZE;
}


/*************************** End of file ****************************/
