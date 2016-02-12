/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2007     SEGGER Microcontroller GmbH & Co KG       *
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
File        : FS_MCI_HW_AT91SAM9RL64.c
Purpose     : Generic MMC/SD card driver for Atmel generic MCI controller
---------------------------END-OF-HEADER------------------------------
*/

#include "FS_Int.h"
#include "MMC_MCI_HW.h"
#include "RTOS.h"
#include "BSP.h"

/*********************************************************************
*
*       defines configurable
*
**********************************************************************
*/
#define MCLK                         (100454400uL)

#define CARD_DETECT_PIN_AVAILABLE    1
#define CARD_DETECT_PIN              15
#define CARD_DETECT_PIN_PIO_BASE     PIOA_BASE
#define CARD_DETECT_PIN_PIO_ID       PIOA_ID

#define WRITE_PROTECT_PIN_AVAILABLE  1					// write protect pin in rev 3.0 boards
#define WRITE_PROTECT_PIN            9					// PA 9
#define WRITE_PROTECT_PIN_PIO_BASE   PIOA_BASE
#define WRITE_PROTECT_PIN_PIO_ID     PIOA_ID

#define PIOA_BASE                    ((PIO_HWREG *)0xFFFFF400) // (PIOA) Base Address
#define PIOB_BASE                    ((PIO_HWREG *)0xFFFFF600) // (PIOB) Base Address
#define PIOC_BASE                    ((PIO_HWREG *)0xFFFFF800) // (PIOC) Base Address
#define PMC_BASE                     ((PMC_HWREG *)0xFFFFFC00) // (PMC)  Base Address
#define MCI_BASE_ADDR                ((U32       *)0xFFFA4000) // (MCI)  Base Address

#define PIOA_ID                      (2)                       // Parallel IO Controller A
#define PIOB_ID                      (3)                       // Parallel IO Controller B
#define PIOC_ID                      (4)                       // Parallel IO Controller C
#define MCI_ID                       (10)                      // Multimedia Card Interface 0

/*********************************************************************
*
*       defines non-configurable
*
**********************************************************************
*/
#define MCI_MMC_SLOTA                    0    // MCI configuration with 1-bit data bus on slot A (for MMC cards).
#define MCI_MMC_SLOTB                    1    // MCI configuration with 1-bit data bus on slot B (for MMC cards).
#define MCI_SD_SLOTA               (1 << 7)   // MCI configuration with 4-bit data bus on slot A (for SD cards).
#define MCI_SD_SLOTB          (1 | (1 << 7))  // MCI configuration with 4-bit data bus on slot B (for SD cards).

// Pin types
#define PIO_PERIPH_A                0         // The pin is controlled by the associated signal of peripheral A.
#define PIO_PERIPH_B                1         // The pin is controlled by the associated signal of peripheral A.
#define PIO_INPUT                   2         // The pin is an input.
#define PIO_OUTPUT_0                3         // The pin is an output and has a default level of 0.
#define PIO_OUTPUT_1                4         // The pin is an output and has a default level of 1.

// Pin types
#define PIO_DEFAULT                 (0 << 0)  // Default pin configuration (no attribute).
#define PIO_PULLUP                  (1 << 0)  // The internal pin pull-up is active.
#define PIO_DEGLITCH                (1 << 1)  // The internal pin pull-up is active.
#define PIO_OPENDRAIN               (1 << 2)  // The pin is open-drain.

/*********************************************************************
*
*       Function replacement macros
*
**********************************************************************
*/
// Used to write in to a SFR register.
#define WRITE_SFR_REG(pSFR, RegName, value)     pSFR->RegName = (value)
// Used to read from a SFR register.
#define READ_SFR_REG(pSFR, RegName)             (pSFR->RegName)

/*********************************************************************
*
*       Data types
*
**********************************************************************
*/
typedef struct {
	volatile U32 PIO_PER; 	    // PIO Enable Register
	volatile U32 PIO_PDR; 	    // PIO Disable Register
	volatile U32 PIO_PSR; 	    // PIO Status Register
	volatile U32 Reserved0[1]; 	//
	volatile U32 PIO_OER; 	    // Output Enable Register
	volatile U32 PIO_ODR; 	    // Output Disable Register
	volatile U32 PIO_OSR; 	    // Output Status Register
	volatile U32 Reserved1[1]; 	//
	volatile U32 PIO_IFER; 	    // Input Filter Enable Register
	volatile U32 PIO_IFDR; 	    // Input Filter Disable Register
	volatile U32 PIO_IFSR; 	    // Input Filter Status Register
	volatile U32 Reserved2[1]; 	//
	volatile U32 PIO_SODR; 	    // Set Output Data Register
	volatile U32 PIO_CODR; 	    // Clear Output Data Register
	volatile U32 PIO_ODSR; 	    // Output Data Status Register
	volatile U32 PIO_PDSR; 	    // Pin Data Status Register
	volatile U32 PIO_IER; 	    // Interrupt Enable Register
	volatile U32 PIO_IDR; 	    // Interrupt Disable Register
	volatile U32 PIO_IMR; 	    // Interrupt Mask Register
	volatile U32 PIO_ISR; 	    // Interrupt Status Register
	volatile U32 PIO_MDER; 	    // Multi-driver Enable Register
	volatile U32 PIO_MDDR; 	    // Multi-driver Disable Register
	volatile U32 PIO_MDSR; 	    // Multi-driver Status Register
	volatile U32 Reserved3[1]; 	//
	volatile U32 PIO_PPUDR; 	  // Pull-up Disable Register
	volatile U32 PIO_PPUER; 	  // Pull-up Enable Register
	volatile U32 PIO_PPUSR; 	  // Pull-up Status Register
	volatile U32 Reserved4[1]; 	//
	volatile U32 PIO_ASR; 	    // Select A Register
	volatile U32 PIO_BSR; 	    // Select B Register
	volatile U32 PIO_ABSR; 	    // AB Select Status Register
	volatile U32 Reserved5[9]; 	//
	volatile U32 PIO_OWER; 	    // Output Write Enable Register
	volatile U32 PIO_OWDR; 	    // Output Write Disable Register
	volatile U32 PIO_OWSR; 	    // Output Write Status Register
} PIO_HWREG;

typedef struct {
	volatile U32 PMC_SCER; 	    // System Clock Enable Register
	volatile U32 PMC_SCDR; 	    // System Clock Disable Register
	volatile U32 PMC_SCSR; 	    // System Clock Status Register
	volatile U32 Reserved0[1]; 	//
	volatile U32 PMC_PCER; 	    // Peripheral Clock Enable Register
	volatile U32 PMC_PCDR; 	    // Peripheral Clock Disable Register
	volatile U32 PMC_PCSR; 	    // Peripheral Clock Status Register
	volatile U32 Reserved1[1]; 	//
	volatile U32 PMC_MOR; 	    // Main Oscillator Register
	volatile U32 PMC_MCFR; 	    // Main Clock  Frequency Register
	volatile U32 PMC_PLLAR; 	  // PLL A Register
	volatile U32 PMC_PLLBR; 	  // PLL B Register
	volatile U32 PMC_MCKR; 	    // Master Clock Register
	volatile U32 Reserved2[3]; 	//
	volatile U32 PMC_PCKR[8]; 	// Programmable Clock Register
	volatile U32 PMC_IER; 	    // Interrupt Enable Register
	volatile U32 PMC_IDR; 	    // Interrupt Disable Register
	volatile U32 PMC_SR; 	      // Status Register
	volatile U32 PMC_IMR; 	    // Interrupt Mask Register
} PMC_HWREG;


typedef struct {
  unsigned int    Mask;       // Bit mask indicating which pin(s) to configure.
  PIO_HWREG     * pPio;       // Pointer to the PIO controller which has the pin(s).
  unsigned char   Id;         // Peripheral ID of the PIO controller which has the pin(s).
  unsigned char   Type;       // Pin type (see "Pin types").
  unsigned char   Attribute;  // Pin attribute (see "Pin attributes").
} PIN;

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
static const PIN _SDPins[] = {
  {0x0000003F, PIOA_BASE, PIOA_ID, PIO_PERIPH_A, PIO_PULLUP}
#if CARD_DETECT_PIN_AVAILABLE
  ,{1 << CARD_DETECT_PIN, CARD_DETECT_PIN_PIO_BASE, CARD_DETECT_PIN_PIO_ID, PIO_INPUT, PIO_PULLUP}
#endif
#if WRITE_PROTECT_PIN_AVAILABLE
  ,{1 << WRITE_PROTECT_PIN, CARD_DETECT_PIN_PIO_BASE, CARD_DETECT_PIN_PIO_ID, PIO_INPUT, PIO_PULLUP}
#endif
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetPIO2Input
*
*  Function description:
*    Configures one or more pin(s) or a PIO controller as inputs. Optionally,
*    the corresponding internal pull-up(s) and glitch filter(s) can be
*    enabled.
*
*  Parameters:
*    pPIO             - Pointer to a PIO controller.
*    Mask             - Bit mask of one or more pin(s) to configure
*    EnablePullUp     - Indicates if the internal pull-up(s) must be enabled.
*    EnableFilter     - Indicates if the glitch filter(s) must be enabled.
*/
static void _SetPIO2Input(PIO_HWREG * pPIO, U32 Mask, U8 EnablePullUp, U8 EnableFilter) {
  // Disable interrupts
  WRITE_SFR_REG(pPIO, PIO_IDR, Mask);
  // Enable pull-up(s) if necessary
  if (EnablePullUp) {
    WRITE_SFR_REG(pPIO, PIO_PPUER, Mask);
  } else {
    WRITE_SFR_REG(pPIO, PIO_PPUDR, Mask);
  }
  // Enable filter(s) if necessary
  if (EnableFilter) {
    WRITE_SFR_REG(pPIO, PIO_IFER, Mask);
  } else {
    WRITE_SFR_REG(pPIO, PIO_IFDR, Mask);
  }
  // Configure pin as input
  WRITE_SFR_REG(pPIO, PIO_ODR, Mask);
  WRITE_SFR_REG(pPIO, PIO_PER, Mask);
}

/*********************************************************************
*
*       _SetPIO2Output
*
*  Function description:
*    Configures one or more pin(s) of a PIO controller as outputs, with the
*    given default value. Optionally, the multi-drive feature can be enabled
*    on the pin(s).
*
*  Parameters:
*    pPIO             - Pointer to a PIO controller.
*    Mask             - Bit mask of one or more pin(s) to configure
*    DefaultValue     - Default level on the pin(s).
*    EnableMultiDrive - Indicates if the pin(s) shall be configured as open-drain.
*    EnablePullUp     - Indicates if the pin shall have its pull-up activated.
*/
static void _SetPIO2Output(PIO_HWREG * pPIO, U32 Mask, U8 DefaultValue, U8 EnableMultiDrive, U8 EnablePullUp) {
  // Disable interrupts
  WRITE_SFR_REG(pPIO, PIO_IDR, Mask);
  // Enable pull-up(s) if necessary
  if (EnablePullUp) {
    WRITE_SFR_REG(pPIO, PIO_PPUER, Mask);
  } else {
    WRITE_SFR_REG(pPIO, PIO_PPUDR, Mask);
  }
  // Enable multi-drive if necessary
  if (EnableMultiDrive) {
    WRITE_SFR_REG(pPIO, PIO_MDER, Mask);
  } else {
    WRITE_SFR_REG(pPIO, PIO_MDDR, Mask);
  }
  // Set default value
  if (DefaultValue) {
    WRITE_SFR_REG(pPIO, PIO_SODR, Mask);
  } else {
    WRITE_SFR_REG(pPIO, PIO_CODR, Mask);
  }
  // Configure pin(s) as output(s)
  WRITE_SFR_REG(pPIO, PIO_OER, Mask);
  WRITE_SFR_REG(pPIO, PIO_PER, Mask);
}

/*********************************************************************
*
*       _SetPIO2Peripheral
*
*  Function description:
*    Configures one or more pin(s) of a PIO controller as being controlled by
*    peripheral A/B. Optionally, the corresponding internal pull-up(s) can be
*    enabled.
*
*  Parameters:
*    pPIO            - Pointer to a PIO controller.
*    PeripheralType  - Specifies the peripheral Type (PIO_PERIPH_A or PIO_PERIPH_B)
*    Mask            - Bit mask of one or more pin(s) to configure
*    EnablePullUp    - Indicates if the pin(s) internal pull-up shall be configured.
*
*/
static void _SetPIO2Peripheral(PIO_HWREG * pPIO, U8 PeripheralType, U32 Mask, U8 EnablePullUp) {
  // Disable interrupts on the pin(s)
  WRITE_SFR_REG(pPIO, PIO_IDR, Mask);
  // Enable the pull-up(s) if necessary
  if (EnablePullUp) {
    WRITE_SFR_REG(pPIO, PIO_PPUER, Mask);
  } else {
    WRITE_SFR_REG(pPIO, PIO_PPUDR, Mask);
  }
  //
  // Configure pin
  //
  if (PeripheralType == PIO_PERIPH_A) {
    WRITE_SFR_REG(pPIO, PIO_ASR, Mask);
  } else if (PeripheralType == PIO_PERIPH_B) {
    WRITE_SFR_REG(pPIO, PIO_BSR, Mask);
  }
  WRITE_SFR_REG(pPIO, PIO_PDR, Mask);
}

/*********************************************************************
*
*       _ConfigurePIO
*
*  Function description:
*    Configures a list of Pin instances, which can either hold a single pin or a
*    group of pins, depending on the mask value; all pins are configured by this
*    function.
*
*  Parameters:
*    pList      - Pointer to an array of PIO list instances.
*    NumItems   - Number of PIO list instances.
*
*  Return value:
*    0          - Success
*    1          - Error
*
*/
static U8 _ConfigurePIO(const PIN *pList, unsigned NumItems) {
  // Configure pins
  while (NumItems > 0) {
    switch (pList->Type) {
    case PIO_PERIPH_A:
    case PIO_PERIPH_B:
      _SetPIO2Peripheral(pList->pPio, pList->Type, pList->Mask, (pList->Attribute & PIO_PULLUP) ? 1 : 0);
      break;
    case PIO_INPUT:
      WRITE_SFR_REG(PMC_BASE, PMC_PCER, (1 << pList->Id));    // Enable the PIO clock
      _SetPIO2Input(pList->pPio, pList->Mask, (pList->Attribute & PIO_PULLUP) ? 1 : 0, (pList->Attribute & PIO_DEGLITCH)? 1 : 0);
      break;
    case PIO_OUTPUT_0:
    case PIO_OUTPUT_1:
      WRITE_SFR_REG(PMC_BASE, PMC_PCER, (1 << pList->Id));    // Enable the PIO clock
      _SetPIO2Output(pList->pPio, pList->Mask, (pList->Type == PIO_OUTPUT_1), (pList->Attribute & PIO_OPENDRAIN) ? 1 : 0, (pList->Attribute & PIO_PULLUP) ? 1 : 0);
      break;
    default:
      return 0;
    }
    pList++;
    NumItems--;
  }
  return 1;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_MCI_HW_IsCardPresent
*
*  Function description:
*    Returns whether a card is inserted or not.
*    When a card detect pin is not available. The function shall return
*    FS_MEDIA_STATE_UNKNOWN. The driver above will check, whether there
*    a valid card
*
*  Parameters:
*    Unit    - MCI Card unit that shall be used
*
*  Return value:
*    FS_MEDIA_STATE_UNKNOWN  - Card state is unknown, since no card detect pin available
*    FS_MEDIA_NOT_PRESENT    - No Card is inserted in slot.
*    FS_MEDIA_IS_PRESENT     - Card is inserted in slot.
*/
int FS_MCI_HW_IsCardPresent(U8 Unit) {
  U8 r;
  r = FS_MEDIA_STATE_UNKNOWN;
  if (CARD_DETECT_PIN_AVAILABLE) {
    r = READ_SFR_REG(CARD_DETECT_PIN_PIO_BASE, PIO_PDSR) & (1 << CARD_DETECT_PIN) ? FS_MEDIA_NOT_PRESENT : FS_MEDIA_IS_PRESENT;
  }
  return r;
}

/*********************************************************************
*
*       FS_MCI_HW_IsCardWriteProtected
*
*  Function description:
*
*
*  Parameters:
*    Unit    - MCI Card unit that shall be used
*
*  Return value:
*    0       - Card is not write protected.
*    1       - Card is     write protected.
*
*/
U8 FS_MCI_HW_IsCardWriteProtected(U8 Unit) {
  U8 r;
  r = 0;
  if (WRITE_PROTECT_PIN_AVAILABLE) {
    r = READ_SFR_REG(WRITE_PROTECT_PIN_PIO_BASE, PIO_PDSR) & (1 << WRITE_PROTECT_PIN) ? 1 : 0;
  }
  return r;
}

/*********************************************************************
*
*       FS_MCI_HW_GetMClk
*
*  Function description:
*
*
*  Parameters:
*    Unit    - MCI Card unit that shall be used
*
*  Return value:
*    The AT91 master clock (MCLK) given in Hz.
*
*/
U32 FS_MCI_HW_GetMClk(U8 Unit) {
  return MCLK;
}

/*********************************************************************
*
*       FS_MCI_HW_Init
*
*  Function description:
*
*
*  Parameters:
*    Unit    - MCI Card unit that shall be used
*
*/
void FS_MCI_HW_Init(U8 Unit) {
  // Configure SDcard pins
  _ConfigurePIO(_SDPins, COUNTOF(_SDPins));
}

/*********************************************************************
*
*       FS_MCI_HW_EnableClock
*
*  Function description:
*
*
*  Parameters:
*    Unit    - MCI Card unit that shall be used
*    OnOff   - 1 - Enable the clock
*              0 - Disable the clock
*
*/
void FS_MCI_HW_EnableClock(U8 Unit, unsigned OnOff) {
  if (OnOff) {
    WRITE_SFR_REG(PMC_BASE, PMC_PCER, (1 << MCI_ID)); // Enable the MCI peripheral clock.
  } else {
    WRITE_SFR_REG(PMC_BASE, PMC_PCDR, (1 << MCI_ID)); // Disable the MCI peripheral clock.
  }
}

/*********************************************************************
*
*       FS_MCI_HW_GetMCIInfo
*
*  Function description:
*
*
*  Parameters:
*    Unit    - MCI Card unit that shall be used
*    pInfo   - Pointer a MCI_INFO structure that shall be filled
*              by this function.
*
*/
void FS_MCI_HW_GetMCIInfo(U8 Unit, MCI_INFO * pInfo) {
  if (pInfo) {
    pInfo->BaseAddr = (U32)MCI_BASE_ADDR;
    pInfo->Mode     = MCI_SD_SLOTA;
  }
}

/*********************************************************************
*
*       FS_MCI_HW_EnableISR
*
*  Function description:
*
*
*  Parameters:
*    Unit         - MCI Card unit that shall be used
*    pISRHandler  - Pointer to the ISR handler that shall be installed.
*
*/
void FS_MCI_HW_EnableISR(U8 Unit, ISR_FUNC * pISRHandler) {
  //
  // Install interrupt service routine
  //
  OS_ARM_InstallISRHandler(MCI_ID, pISRHandler);
  OS_ARM_EnableISR(MCI_ID);
}

/***************************************************
*
*       FS_MCI_HW_GetTransferMem
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
U32 FS_MCI_HW_GetTransferMem(U32 * pPAddr, U32 * pVAddr) {
  return BSP_SD_GetTransferMem(pPAddr, pVAddr);                // In order to have device dependend memory allocation at one place, we call a BSP function
}

/*************************** End of file ****************************/
