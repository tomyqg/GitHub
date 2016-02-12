/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : MMC_X_HW.h
Purpose     : MMC hardware layer
---------------------------END-OF-HEADER------------------------------
*/

#ifndef __SPI_API_H__
#define __SPI_API_H__

#include "FS_Int.h"

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/
void SPIInit(void);
void TSInit(void);
extern char SDCardWriteProtected(void);
extern char SDCardAlmostFull(void);
extern char SDCardPresent(void);

/* Control line functions */
void TS_SPI_X_EnableCS(void);
void TS_SPI_X_DisableCS(void);
void DF_SPI_X_EnableCS(unsigned char);
void DF_SPI_X_DisableCS(unsigned char);

/* Data transfer functions */
void SPI_DMA_X_Read (U8 Unit, U8 * pData, int NumBytes);
void SPI_DMA_X_Write(U8 Unit, const U8 * pData, int NumBytes);
void TS_SPI_X_Read(U8 * pData, int NumBytes);
void TS_SPI_X_Write(const U8 * pData, int NumBytes);
void TS_SPI_X_Write_Read(const U8 *, int, U8 *, int);
void LED_SPI_X_Write(const U8 * pData, int NumBytes);
unsigned char xferSPI (unsigned char ch);

#include "AT91SAM9RL64.h"

#define PIOA_BASE 0xfffff400
#define PIOA_PDR  *(volatile U32*) (PIOA_BASE + 0x04) /* PIOA disable register               */
#define PIOA_ASR  *(volatile U32*) (PIOA_BASE + 0x70) /* PIOA "A" peripheral select register */
#define PIOA_BSR  *(volatile U32*) (PIOA_BASE + 0x74) /* PIOA "B" peripheral select register */
#define PIOA_PER  *(volatile U32*) (PIOA_BASE + 0x00) // Enable register, enables PIO function
#define PIOA_OER  *(volatile U32*) (PIOA_BASE + 0x10) // Output enable register, sets to output mode
#define PIOA_ODR  *(volatile U32*) (PIOA_BASE + 0x14) // Output enable register, sets to output mode
#define PIOA_SODR *(volatile U32*) (PIOA_BASE + 0x30) // Set output data
#define PIOA_CODR *(volatile U32*) (PIOA_BASE + 0x34) // Clear output data register
#define PIOA_ODSR *(volatile U32*) (PIOA_BASE + 0x38) // output data status register
#define PIOA_PDSR *(volatile U32*) (PIOA_BASE + 0x3c) // pin data status register
#define PIOA_MDDR *(volatile U32*) (PIOA_BASE + 0x54) //
#define PIOA_MDER *(volatile U32*) (PIOA_BASE + 0x50) //
#define PIOA_PUDR *(volatile U32*) (PIOA_BASE + 0x60) //
#define PIOA_PUER *(volatile U32*) (PIOA_BASE + 0x64) // pull-up enable register
#define PIOA_OWER *(volatile U32*) (PIOA_BASE + 0xA0) // Output write enable register
#define PIOA_OWDR *(volatile U32*) (PIOA_BASE + 0xA4) // Output write disable register

#define PIOB_BASE 0xfffff600
#define PIOB_PDR  *(volatile U32*) (PIOB_BASE + 0x04) /* PIOB disable register               */
#define PIOB_ASR  *(volatile U32*) (PIOB_BASE + 0x70) /* PIOB "A" peripheral select register */
#define PIOB_BSR  *(volatile U32*) (PIOB_BASE + 0x74) /* PIOB "B" peripheral select register */
#define PIOB_PER  *(volatile U32*) (PIOB_BASE + 0x00) // Enable register, enables PIO function
#define PIOB_OER  *(volatile U32*) (PIOB_BASE + 0x10) // Output enable register, sets to output mode
#define PIOB_ODR  *(volatile U32*) (PIOB_BASE + 0x14) // Output enable register, sets to output mode
#define PIOB_IFER *(volatile U32*) (PIOB_BASE + 0x20) //
#define PIOB_IFDR *(volatile U32*) (PIOB_BASE + 0x24) //
#define PIOB_SODR *(volatile U32*) (PIOB_BASE + 0x30) // Set output data
#define PIOB_CODR *(volatile U32*) (PIOB_BASE + 0x34) // Clear output data register
#define PIOB_ODSR *(volatile U32*) (PIOB_BASE + 0x38) // output data status register
#define PIOB_PDSR *(volatile U32*) (PIOB_BASE + 0x3c) // pin data status register
#define PIOB_PUDR *(volatile U32*) (PIOB_BASE + 0x60) //
#define PIOB_OWER *(volatile U32*) (PIOB_BASE + 0xA0) // Output write enable register
#define PIOB_OWDR *(volatile U32*) (PIOB_BASE + 0xA4) // Output write disable register

#define PIOC_IDR                           (*(volatile U32*) (0xFFFFF844))
#define PIOC_PPUDR                         (*(volatile U32*) (0xFFFFF860))
#define PIOC_PPUER                         (*(volatile U32*) (0xFFFFF864))
#define PIOC_ASR                           (*(volatile U32*) (0xFFFFF870))
#define PIOC_BSR                           (*(volatile U32*) (0xFFFFF874))
#define PIOC_PDR                           (*(volatile U32*) (0xFFFFF804))

#define PIOD_BASE 0xfffffA00
#define PIOD_PDR  *(volatile U32*) (PIOD_BASE + 0x04) /* PIOD disable register               */
#define PIOD_PER  *(volatile U32*) (PIOD_BASE + 0x00) // Enable register, enables PIO function
#define PIOD_OER  *(volatile U32*) (PIOD_BASE + 0x10) // Output enable register, sets to output mode
#define PIOD_ODR  *(volatile U32*) (PIOD_BASE + 0x14) // Output enable register, sets to output mode
#define PIOD_OSR  *(volatile U32*) (PIOD_BASE + 0x18) // Output status register
#define PIOD_IFDR *(volatile U32*) (PIOD_BASE + 0x24) //
#define PIOD_SODR *(volatile U32*) (PIOD_BASE + 0x30) // Set output data
#define PIOD_CODR *(volatile U32*) (PIOD_BASE + 0x34) // Clear output data register
#define PIOD_ODSR *(volatile U32*) (PIOD_BASE + 0x38) // output data status register
#define PIOD_PDSR *(volatile U32*) (PIOD_BASE + 0x3c) // pin data status register
#define PIOD_IDR  *(volatile U32*) (PIOD_BASE + 0x44) //
#define PIOD_MDDR *(volatile U32*) (PIOD_BASE + 0x54) //
#define PIOD_MDER *(volatile U32*) (PIOD_BASE + 0x50) //
#define PIOD_PUDR *(volatile U32*) (PIOD_BASE + 0x60) //
#define PIOD_PUER *(volatile U32*) (PIOD_BASE + 0x64) // pull-up enable register
#define PIOD_ASR  *(volatile U32*) (PIOD_BASE + 0x70) /* PIOD "A" peripheral select register */
#define PIOD_BSR  *(volatile U32*) (PIOD_BASE + 0x74) /* PIOD "B" peripheral select register */
#define PIOD_OWER *(volatile U32*) (PIOD_BASE + 0xA0) // Output write enable register
#define PIOD_OWDR *(volatile U32*) (PIOD_BASE + 0xA4) // Output write disable register

#define SPI_BASE  0xFFFCC000
#define SPI_CR    *(volatile U32*) (SPI_BASE + 0x00)
#define SPI_MR    *(volatile U32*) (SPI_BASE + 0x04)
#define SPI_RDR   *(volatile U32*) (SPI_BASE + 0x08)
#define SPI_TDR   *(volatile U32*) (SPI_BASE + 0x0C)
#define SPI_SR    *(volatile U32*) (SPI_BASE + 0x10)
#define RDRF		0x00000001
#define TDRE		0x00000002
// KMC #define TXEMPTY	0x00000020
#define TXEMPTY	0x00000200

#define SPI_IER   *(volatile U32*) (SPI_BASE + 0x14)
#define SPI_IDR   *(volatile U32*) (SPI_BASE + 0x18)
#define SPI_IMR   *(volatile U32*) (SPI_BASE + 0x1c)
#define SPI_CSR0  *(volatile U32*) (SPI_BASE + 0x30)
#define SPI_CSR1  *(volatile U32*) (SPI_BASE + 0x34)
#define SPI_CSR2  *(volatile U32*) (SPI_BASE + 0x38)
#define SPI_CSR3  *(volatile U32*) (SPI_BASE + 0x3c)

/*      SPI PDC */
#define SPI_PDC_RPR  *(volatile U32*) (SPI_BASE + 0x100)
#define SPI_PDC_RCR  *(volatile U32*) (SPI_BASE + 0x104)
#define SPI_PDC_TPR  *(volatile U32*) (SPI_BASE + 0x108)
#define SPI_PDC_TCR  *(volatile U32*) (SPI_BASE + 0x10c)
#define SPI_PDC_RNPR *(volatile U32*) (SPI_BASE + 0x110)
#define SPI_PDC_RNCR *(volatile U32*) (SPI_BASE + 0x114)
#define SPI_PDC_TNPR *(volatile U32*) (SPI_BASE + 0x118)
#define SPI_PDC_TNCR *(volatile U32*) (SPI_BASE + 0x11c)
#define SPI_PDC_PTCR *(volatile U32*) (SPI_BASE + 0x120)
#define SPI_PDC_PTSR *(volatile U32*) (SPI_BASE + 0x124)

#define SPI_RXBUFF    ((unsigned int) 0x1 <<  6)    // (SPI) RXBUFF Interrupt
#define SPI_TXBUFE    ((unsigned int) 0x1 <<  7)    // (SPI) TXBUFE Interrupt

#define PDC_RXTEN     ((unsigned int) 0x1 <<  0)    // (PDC) Receiver Transfer Enable
#define PDC_RXTDIS    ((unsigned int) 0x1 <<  1)    // (PDC) Receiver Transfer Disable
#define PDC_TXTEN     ((unsigned int) 0x1 <<  8)    // (PDC) Transmitter Transfer Enable
#define PDC_TXTDIS    ((unsigned int) 0x1 <<  9)    // (PDC) Transmitter Transfer Disable

/*      Power management controller */
#define PMC_BASE  0xFFFFFC00
#define PMC_SCER  *(volatile U32*) (PMC_BASE + 0x00) // System Clock Enable Register
#define PMC_SCDR  *(volatile U32*) (PMC_BASE + 0x04) // System Clock Disable Register
#define PMC_SCSR  *(volatile U32*) (PMC_BASE + 0x08) // System Clock Status Register

#define PMC_PCER  *(volatile U32*) (PMC_BASE + 0x10)  /* Peripheral clock enable register */


// ========== Register definition for PWMC peripheral ==========
#define PWMC_MR   (*(volatile U32*) 	(0xFFFC8000)) // (PWMC) PWMC Mode Register
#define PWMC_ENA  (*(volatile U32*) 	(0xFFFC8004)) // (PWMC) PWMC Enable Register
#define PWMC_DIS  (*(volatile U32*) 	(0xFFFC8008)) // (PWMC) PWMC Disable Register
#define PWMC_SR   (*(volatile U32*) 	(0xFFFC800C)) // (PWMC) PWMC Status Register
#define PWMC_IER  (*(volatile U32*) 	(0xFFFC8010)) // (PWMC) PWMC Interrupt Enable Register
#define PWMC_IDR  (*(volatile U32*) 	(0xFFFC8014)) // (PWMC) PWMC Interrupt Disable Register
#define PWMC_IMR  (*(volatile U32*) 	(0xFFFC8018)) // (PWMC) PWMC Interrupt Mask Register
#define PWMC_ISR  (*(volatile U32*) 	(0xFFFC801C)) // (PWMC) PWMC Interrupt Status Register
#define PWMC_VR   (*(volatile U32*) 	(0xFFFC80FC)) // (PWMC) PWMC Version Register

#define PWMC_MR0  (*(volatile U32*)		(0xFFFC8200)) // (PWMC) PWMC Channel 0 Mode Register
#define PWMC_DTY0 (*(volatile U32*)		(0xFFFC8204)) // (PWMC) PWMC Channel 0 Duty Cycle Register
#define PWMC_PRD0 (*(volatile U32*)		(0xFFFC8208)) // (PWMC) PWMC Channel 0 Period Register
#define PWMC_CNT0 (*(volatile U32*)		(0xFFFC820C)) // (PWMC) PWMC Channel 0 Counter Register
#define PWMC_UPD0 (*(volatile U32*)		(0xFFFC8210)) // (PWMC) PWMC Channel 0 Update Register

#define PWMC_MR1  (*(volatile U32*)		(0xFFFC8220)) // (PWMC) PWMC Channel 1 Mode Register
#define PWMC_DTY1 (*(volatile U32*)		(0xFFFC8224)) // (PWMC) PWMC Channel 1 Duty Cycle Register
#define PWMC_PRD1 (*(volatile U32*)		(0xFFFC8228)) // (PWMC) PWMC Channel 1 Period Register
#define PWMC_CNT1 (*(volatile U32*)		(0xFFFC822C)) // (PWMC) PWMC Channel 1 Counter Register
#define PWMC_UPD1 (*(volatile U32*)		(0xFFFC8230)) // (PWMC) PWMC Channel 1 Update Register

#define PWMC_MR2  (*(volatile U32*)		(0xFFFC8240)) // (PWMC) PWMC Channel 2 Mode Register
#define PWMC_DTY2 (*(volatile U32*)		(0xFFFC8244)) // (PWMC) PWMC Channel 2 Duty Cycle Register
#define PWMC_PRD2 (*(volatile U32*)		(0xFFFC8248)) // (PWMC) PWMC Channel 2 Period Register
#define PWMC_CNT2 (*(volatile U32*)		(0xFFFC824C)) // (PWMC) PWMC Channel 2 Counter Register
#define PWMC_UPD2 (*(volatile U32*)		(0xFFFC8250)) // (PWMC) PWMC Channel 2 Update Register

#define PWMC_MR3  (*(volatile U32*)		(0xFFFC8260)) // (PWMC) PWMC Channel 3 Mode Register
#define PWMC_DTY3 (*(volatile U32*)		(0xFFFC8264)) // (PWMC) PWMC Channel 3 Duty Cycle Register
#define PWMC_PRD3 (*(volatile U32*)		(0xFFFC8268)) // (PWMC) PWMC Channel 3 Period Register
#define PWMC_CNT3 (*(volatile U32*)		(0xFFFC826C)) // (PWMC) PWMC Channel 3 Counter Register
#define PWMC_UPD3 (*(volatile U32*)		(0xFFFC8270)) // (PWMC) PWMC Channel 3 Update Register






//
// Select AR1020 chip select
//            |(1 << 0)         // 1 : Master mode
//            |(0 << 1)         // 0 : Fixed chip select
//            |(0 << 2)         // Chip select
//            |(0 << 3)         // 0: Use MCLK as clock
//            |(1 << 4)         // 1: Fault detection disable
//            |(0 << 7)         // 1: Loopback
//            |(1 << 16)        // Select NPCS1
#define SELECT_TS (SPI_MR = (SPI_MR & 0xFFF0FFF1) | 0x000D0000)			// cs1
//
// Select LED controller chip
//            |(1 << 0)         // 1 : Master mode
//            |(0 << 1)         // 0 : Fixed chip select
//            |(0 << 2)         // Chip select
//            |(0 << 3)         // 0: Use MCLK as clock
//            |(1 << 4)         // 1: Fault detection disable
//            |(0 << 7)         // 1: Loopback
//            |(7 << 16)        // Select NCS3 - unused CS - handled by GPIO into Latch enabled instead
#define SELECT_LED (SPI_MR = (SPI_MR & 0xFFF0FFF1) | 0x00070000)		// cs3

// dataflash 0 and 1
#define SELECT_DF0 (SPI_MR = (SPI_MR & 0xFFF0FFF1) | 0x000E0000)		// cs0
#define SELECT_DF1 (SPI_MR = (SPI_MR & 0xFFF0FFF1) | 0x000B0000)		// cs2

//
// Port A Pin definitions
//
#define SD0_CS_PIN			28			// first serial dataflash
#define	SDCARD_DETECT		15			// sd card inserted
#define SDCARD_WP			9			// sd card write protect
#define SENSE_12V			17			// comparator for detecting power fail
#define USB_OC_PIN			19			// input, USB overcurrent alarm, active low
#define USB_ENABLE_PIN		20			// output, enables USB port, active high
#define DRXD_PIN			21
#define DTXD_PIN			22
#define SD_MISO_PIN			25
#define SD_MOSI_PIN			26
#define SD_CLK_PIN			27
#define SPI_CSR				SPI_CSR0

//
// Peripheral IDs (page 35 of AT91SAM9RL-64 manual)
//
#define PIOA_ID				2
#define PIOB_ID				3
#define PIOD_ID				5
#define SPI_ID				13
#define PWM_ID				19

//
// Peripheral IO B port Pin definitions
//
#define TS_CS_PIN			7
#define PWR_LOSS_12V_PIN	10

//
// Conditional compile flags
//
// Either build Eval Board or Production Board!
#undef	EVAL_BOARD

//
// PIO Controller D pin definitions
//
#ifdef	EVAL_BOARD
#define LED_CS_PIN				8		// latch enable for LED controller
#else
#define LED_CS_PIN				5		// latch enable for LED controller
#endif

#define LIGHT_SENSOR			6		// analog input for ambient light sensor
#define SD1_CS_PIN				8		// second serial dataflash
#define ONOFF_BUTTON			0		// turns unit on/off
#define LCD_5V_ENABLE			10		// LCD power
#define LCD_BACKLIGHT_ENABLE	11		// turns on LCD backlight

#define LCD_BACKLIGHT_PIN		14
#define UPPER_WARNING_LED_PIN	15
#define LOWER_WARNING_LED_PIN	16

#define HPSD					17		// audio enable. low = off.
#define LED_OE_PIN		  		18		// LED output enable
#define TS_RESET_PIN			20		// touchscreen reset
#define ENABLE_5V				21		// board 5V power

#define MCLK					(100045440uL)  // may depend on PLL
#define MCLK_SPICLOCK			(MCLK)         // defaults is SD clock = MCLK

#define BUFFER_SIZE				0x200
#define CACHE_LINE_SIZE			32

// Duty cycle for side LED off state
#define WARNING_LED_OFF 		100
#define TACH_LED_OFF			0
#define BACKLIGHT_LED_OFF		0

// Memory definitions for application
#define START_OF_DRAM		0x20000000
#define FIRST_CODE			0x20000040	// afte 0x40 vector table in DRAM
#define SIZE_OF_DRAM		0x4000000
#define SIZE_OF DFLASH0		0x882000	// with 1056 sector size
#define SIZE_OF_DFLASH1		0x882000	// with 1056 sector size
#define START_OF_ISRAM		0x300000	// internal SRAM
#define SIZE_OF_ISRAM		0x10000		// 64K
#define FS_ALLOC_SIZE		0x2000		// sector buffer for file system
#define DMA_SIZE			0x2000		// DMA buffer for SD card access

#define TOP_OF_ISRAM		(START_OF_ISRAM + SIZE_OF_ISRAM)
#define FS_CACHE_SIZE		(SIZE_OF_ISRAM - FS_ALLOC_SIZE - DMA_SIZE)

#define CODESTART	0x10000		// where code goes in DRAM
#define CODELOAD(x)	(CODESTART / unsavedSysData.dataflash[x].bytesPerPage)		// works for either 1024 or 1056 sectors on either flash chip
#define BOOTLOAD	0x0000		// where bootloader goes in memory, including vector table

#endif  /* __SPI_API_H__ */

/*************************** End of file ****************************/
