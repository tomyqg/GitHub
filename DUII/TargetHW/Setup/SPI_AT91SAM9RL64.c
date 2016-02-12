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

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
----------------------------------------------------------------------
File        : MMC_HW_SPI.c
Purpose     : Sample MMC hardware layer for Atmel AT91SAM9261
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "SPI_API.h"
#include "FS_OS.h"
#include "RTOS.h"
#include "MMC_MCI_HW.h"

//
// Public Prototypes
//

//
// Local Prototypes
//
void uSDelay(int);
void mSDelay(int);
void DummyWrite(void);
void _SPI_Common_X_Write(const U8 * pData, int NumBytes);
void LED_SPI_X_EnableCS(void);
void DF0_SPI_X_EnableCS(void);
void DF1_SPI_X_EnableCS(void);
void LED_SPI_X_DisableCS(void);
void DF0_SPI_X_DisableCS(void);
void DF1_SPI_X_DisableCS(void);

/*********************************************************************
*
*       Defines, Configurable
*
**********************************************************************
*/
#define USE_OS    0     // 0: Polled mode, 1 means the task is suspended during DMA transfer, reducing CPU load and allowing the CPU to execute an other task

#ifndef USE_PDC
  #define USE_PDC 1    // 0: PDC shall not be used. When PDC is used to transfer data from/to internal memory, wrong data are received. 1 : PDC shall be used.
#endif

/*********************************************************************
*
*       Defines, sfr
*
**********************************************************************
*/

/*********************************************************************
*
*       #define Macros
*
**********************************************************************
*/
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8           _sbcr = MCLK_SPICLOCK / 4000000UL;
static U32          _aBuffer[(BUFFER_SIZE + CACHE_LINE_SIZE) / 4];   // Used to transfer data from or to DMA
static const U8     _aFF[512] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _PDC_SetTx
*/
static void _PDC_SetTx(const U8 * pAddress, unsigned NumBytes) {
  SPI_PDC_TPR = (unsigned int)pAddress;
  SPI_PDC_TCR = NumBytes;
}

/*********************************************************************
*
*       _PDC_SetRx
*/
static void _PDC_SetRx(U8 * pAddress, unsigned Bytes) {
  SPI_PDC_RPR = (unsigned int)pAddress;
  SPI_PDC_RCR = Bytes;
}

/*********************************************************************
*
*       _PDC_EnableTx
*/
static void _PDC_EnableTx(void) {
  SPI_PDC_PTCR = PDC_TXTEN;
}

/*********************************************************************
*
*       _PDC_EnableRx
*/
static void _PDC_EnableRx(void) {
  SPI_PDC_PTCR = PDC_RXTEN;
}

#if USE_OS
/**********************************************************
*
*       _IrqHandler
*/
static void _IrqHandler(void) {
  U32 v;

  //
  // Disable further interrupts
  //
  v  = SPI_IMR;
  SPI_IDR =  v;
  //
  // Signal (wake) the task waiting
  //
  FS_OS_SIGNAL();
}
#endif
/**********************************************************
*
*       _GetBufferAddr
*/
static U8 * _GetBufferAddr(void) {
  U8 * p;

  p = (U8 *)(((U32)&_aBuffer[0] + 0x1f) & ~0x1f);
  return p;
}

/*********************************************************************
*
*       _Init
*/
static void _Init(void) {
  //
  // Enable "Power" (clock) for PIOA, B, D, PWM and SPI block
  //
  PMC_PCER = (1 << SPI_ID)
           | (1 << PIOA_ID)
           | (1 << PIOB_ID)
           | (1 << PIOD_ID)
           | (1 << PWM_ID)
			;
  //
  // Setup Pins
  //
  PIOA_SODR = 0								// set output data register
            | (1 << SD0_CS_PIN)
            | (1 << SD_MOSI_PIN)
            | (1 << SD_CLK_PIN)
			| (1 << SENSE_12V)
			| (1 << SDCARD_DETECT)
			| (1 << SDCARD_WP)
            | (1 << SD0_CS_PIN)
            ;
  PIOA_PER =  0								// pio enable register
			| (1 << USB_OC_PIN)
			| (1 << USB_ENABLE_PIN)
			| (1 << SENSE_12V)
			| (1 << SDCARD_DETECT)
			| (1 << SDCARD_WP)
            | (1 << SD0_CS_PIN)
			;
  PIOA_OER  = 0								// output enable register
	 		| (1 << DTXD_PIN)				// Serial Debug Unit data Tx Pin
            | (1 << SD0_CS_PIN)
			| (1 << USB_ENABLE_PIN)
            | (1 << SD_MOSI_PIN)
            | (1 << SD_CLK_PIN)
            | (1 << SD0_CS_PIN)
            ;
  PIOA_ODR  = 0								// output disable register
	 		| (1 << DRXD_PIN)				// Serial Debug Unit data Rx Pin
            | (1 << SD_MOSI_PIN)
            ;
  PIOA_PDR =  0								// port disable register
	 		| (1 << DRXD_PIN)				// Serial Debug Unit data Rx Pin
	 		| (1 << DTXD_PIN)				// Serial Debug Unit data Tx Pin
            | (1 << SD_MISO_PIN)       // SPI-MISO
            | (1 << SD_MOSI_PIN)       // SPI-MOSI
            | (1 << SD_CLK_PIN)        // SPI-Clock
			;
  PIOA_ASR  = 0								// A select register
	 		| (1 << DRXD_PIN)				// Serial Debug Unit data Rx Pin
	 		| (1 << DTXD_PIN)				// Serial Debug Unit data Tx Pin
            | (1 << SD_MISO_PIN)       // SPI-MISO
            | (1 << SD_MOSI_PIN)       // SPI-MOSI
            | (1 << SD_CLK_PIN)        // SPI-Clock
			;

  PIOA_MDER =	0							// enable open collector
			| (1 << SENSE_12V)				// power loss comparator
			| (1 << SDCARD_DETECT)			// card detect
			| (1 << SDCARD_WP)				// write protect
			| (1 << USB_OC_PIN)				// read overcurrent
			;
  PIOA_PUDR =	0								// pull-up disable
			| (1 << SENSE_12V)
			| (1 << USB_ENABLE_PIN)			// turn off usb
			| (1 << USB_OC_PIN)				// turn off pullup so we can read overcurrent
			;
  PIOA_CODR = 0								// clear output data register
			| (1 << SENSE_12V)				// set to low
			| (1 << USB_ENABLE_PIN)			// turn off usb
			| (1 << USB_OC_PIN)				// turn off pullup so we can read overcurrent
            ;

#if 0
  // reference code only
  //
  // Initialize LCD
  //
;  PIOC_IDR        = 0xE8;
;  PIOC_PPUDR      = 0xE8;
  PIOC_ASR        = 0xE8;
  PIOC_PDR        = 0xE8;
;  PIOC_IDR        = 0x03FFBE12;
;  PIOC_PPUDR      = 0x03FFBE12;
;  PIOC_BSR        = 0x03FFBE12;
  PIOC_PDR        = 0x03FFBE12;
#endif

  PIOB_PER =	0
				| (1 << TS_CS_PIN)
				| (1 << PWR_LOSS_12V_PIN)
				;

  PIOB_SODR =	0
				| (1 << TS_CS_PIN)
				;

  PIOB_OER  =	0								// enable outputs
				| (1 << TS_CS_PIN)
				;

  PIOB_ODR  =	0								// disable outputs
				| (1 << PWR_LOSS_12V_PIN)
				;

  PIOB_PUDR  =	0								// disable pullups
				| (1 << PWR_LOSS_12V_PIN)
				;

  PIOB_IFER  =	0								// enable input glitch filtering
				| (1 << PWR_LOSS_12V_PIN)
				;

// Do this before configuring GPIO to prevent flicker of LEDs when PWM pin is enabled

//
// Configure PWM for LED brightness control
// PIO Controller D, Peripheral A, PWM3
// pin PD18 to eval board connector J26 pin 20
//
// PWM0 - PD14 - Backlight control
// PWM1 - PD15 - Main board, upper, side warning LED
// PWM2 - PD16 - Main board, lower, side warning LED
// PWM3 - PD18 - LED board (tach LEDs)
//
	// ROB'S RECOMMENDATION
	PWMC_MR	= 0x00000A02;   // which is MCLK/ (1024*2) = 97KHz on clkA
	PWMC_ENA	= 0x0000000F;   // to enable PWM 0, 1, 2 and 3
	
// Tach LEDs
	PWMC_MR3	= 0x0000000B;  // to use clkA
	PWMC_PRD3= 0x00000064;  // set period to about 488Hz
	PWMC_DTY3 = TACH_LED_OFF;	// Set duty cycle to off
	
// Configure PWN for Warning LED brightness control
// PIO Controller D, Peripheral B, PWM3
// pins PD15, PD16 to eval board connector J26 pins 15, 18
// Set side warning LEDs initially with the same data
// Note PWM 1,2 are inverted from PWM3
	// Backlight
	PWMC_MR0  = 0x0000000B;   	// to use clkA
	PWMC_PRD0 = 0x00000190;   	// 400 decimal, set period to about 120Hz
	PWMC_DTY0 = BACKLIGHT_LED_OFF;  	// Set duty cycle to off

//	Warning LED 1
	PWMC_MR1  = 0x0000000B;   		// to use clkA
	PWMC_PRD1 = 0x00000064;   		// set period to about 488Hz
	PWMC_DTY1 = WARNING_LED_OFF;  // Set duty cycle to off

//	Warning LED 2
	PWMC_MR2  = 0x0000000B;   		// to use clkA
	PWMC_PRD2 = 0x00000064;   		// set period to about 488Hz
	PWMC_DTY2 = WARNING_LED_OFF;  // Set duty cycle to off

// KMC - make GPIO
  PIOD_PER =	0								// PIO enable
	 			| (1 << LCD_5V_ENABLE)
				| (1 << LED_CS_PIN)
				| (1 << TS_RESET_PIN)
				| (1 << HPSD)
	            | (1 << SD1_CS_PIN)
				| (1 << ONOFF_BUTTON)
				;

  PIOD_PDR =	0								// PIO disable
				| (1 << LIGHT_SENSOR)
				| (1 << LED_OE_PIN)
				| (1 << UPPER_WARNING_LED_PIN)
				| (1 << LOWER_WARNING_LED_PIN)
				| (1 << LCD_BACKLIGHT_PIN)
				;
				
  PIOD_OER  = 0									// enable output
	 			| (1 << LCD_5V_ENABLE)
				| (1 << LED_CS_PIN)
				| (1 << TS_RESET_PIN)
				| (1 << HPSD)
				| (1 << UPPER_WARNING_LED_PIN)
				| (1 << LOWER_WARNING_LED_PIN)
				| (1 << LCD_BACKLIGHT_PIN)
	            | (1 << SD1_CS_PIN)
				;

  PIOD_ODR =	0								// output disable
				| (1 << ONOFF_BUTTON)
				| (1 << LIGHT_SENSOR)
				;

  PIOD_IDR =	0								// interrupt disable
	 			| (1 << LCD_5V_ENABLE)
				| (1 << LED_CS_PIN)
				| (1 << TS_RESET_PIN)
				| (1 << HPSD)
				| (1 << ONOFF_BUTTON)
				;

  PIOD_PUDR =	0								// pull-up disable
	 			| (1 << LCD_5V_ENABLE)
				| (1 << LED_CS_PIN)
				| (1 << LED_OE_PIN)
				| (1 << TS_RESET_PIN)
				| (1 << HPSD)
				| (1 << UPPER_WARNING_LED_PIN)
				| (1 << LOWER_WARNING_LED_PIN)
				| (1 << LCD_BACKLIGHT_PIN)
				| (1 << LIGHT_SENSOR)
				;

  PIOD_PUER =	0								// pullup enable
				| (1 << ONOFF_BUTTON)
				;

  PIOD_MDDR =	0								// disable open drain
	 			| (1 << LCD_5V_ENABLE)
				| (1 << LED_CS_PIN)
				| (1 << LED_OE_PIN)
				| (1 << TS_RESET_PIN)
				| (1 << HPSD)
				| (1 << UPPER_WARNING_LED_PIN)
				| (1 << LOWER_WARNING_LED_PIN)
				| (1 << LCD_BACKLIGHT_PIN)
				;

  PIOD_MDER =	0								// enable open drain
				| (1 << ONOFF_BUTTON)
				;

  PIOD_ASR =	0								// choose A peripheral
				| (1 << LED_OE_PIN)
				| (1 << LIGHT_SENSOR)
				;

  PIOD_BSR =	0								// choose B peripheral
				| (1 << UPPER_WARNING_LED_PIN)
				| (1 << LOWER_WARNING_LED_PIN)
				| (1 << LCD_BACKLIGHT_PIN)
				;

// SET INITIAL STATE OF PIOD
  PIOD_CODR =	0								// set output line low
				| (1 << LED_CS_PIN)
				| (1 << HPSD)
				| (1 << UPPER_WARNING_LED_PIN)
				| (1 << LOWER_WARNING_LED_PIN)
				| (1 << LCD_BACKLIGHT_PIN)
				| (1 << LED_OE_PIN)
				;

  PIOD_SODR  =	0								// set output line high
	 			| (1 << LCD_5V_ENABLE)
				| (1 << TS_RESET_PIN)
				| (1 << SD1_CS_PIN)
				;
  //
  // SPI
  //
  SPI_CR    = (1 << 7);      // Software reset
  SPI_CR    = (1 << 7);      // KMC - Atmel code sends 2 consecutive resets!
  // see C:\Program Files\IAR Systems\Embedded Workbench 5.4\arm\examples\Atmel\at91lib\memories\spi-flash line 73
  SPI_MR    = 0
            |(1 << 0)         // 1 : Master mode
            |(0 << 1)         // 0 : Fixed chip select
            |(0 << 2)         // Chip select
            |(0 << 3)         // 0: Use MCLK as clock
            |(1 << 4)         // 1: Fault detection disable
            |(0 << 7)         // 1: Loopback
            |(0xD << 16)      // Default to Touch Screen Controller as selected by default (must be something - cannot be none)
            ;
  SPI_CSR0  = 0				  // dataflash 0
            |(1 << 0)         // 1 : Clock polarity of idle is high
            |(0 << 1)         // Clock hase sel
            |(0 << 3)         // Chip select 0 becomes inactive after transfer (KMC - was: Leave CS0 stay low)
            |(0 << 4)         // 0000b: 8 bits per transfer
            |(_sbcr << 8)     // 8..15: SCBR: Baud rate divider  **** THIS WORKS ****
// SPI can run at 50MHz = MCLK / 2. Trying 4 = 25MHz -- GAS
//			| (4 << 8)			// 25MHz
				|(0x100000)
            ;
  SPI_CSR1  = 0					// KMC TODO - initially assuming communication with touch screen controller chip
	 									// is the same as serial flash (whose setting are left over from the settings used
	 									// by the 9261 access to the SD card).
            |(0 << 0)         // CPOL = 0, Clock polarity of idle is low
            |(1 << 1)         // NCPHA = 1??????????????
            |(0 << 3)         // Chip select 1 becomes inactive after transfer
            |(0 << 4)         // 0000b: 8 bits per transfer
            |((_sbcr << 3) << 8)	   // 8..15: SCBR: Baud rate divider -- KMC TEST - slow it down
            |(0x100000)
            ;
  SPI_CSR2  = 0				  // dataflash 1
            |(1 << 0)         // 1 : Clock polarity of idle is high
            |(0 << 1)         // Clock hase sel
            |(0 << 3)         // Chip select 0 becomes inactive after transfer (KMC - was: Leave CS0 stay low)
            |(0 << 4)         // 0000b: 8 bits per transfer
            |(_sbcr << 8)     // 8..15: SCBR: Baud rate divider *** SEE ABOVE ***
//			| (4 << 8)			// 25 MHz
            |(0x100000)
			;
  SPI_CSR3  = 0					// LED controller
            |(1 << 0)         // 1 : Clock polarity of idle is high
            |(0 << 1)         // Clock hase sel
            |(0 << 3)         // Chip select 0 becomes inactive after transfer (KMC - was: Leave CS0 stay low)
            |(0 << 4)         // 0000b: 8 bits per transfer
            |(_sbcr << 8)     // 8..15: SCBR: Baud rate divider
//            |(0xFF << 8)	   // 8..15: SCBR: Baud rate divider -- KMC TEST - slow it WAY down (0xff = slowest, 0x00 = fastest)
            |(0x100000);
            ;
  SPI_CR    = (1 << 0);       // Enable SPI

	
#if USE_OS
  //
  // HW Init done
  // Setup IRQ handler
  //
  SPI_IDR = 0xFFFFFFFF;                                     // Disable all interrupts
  *(U32*)(0xFFFFF080 + 4 * SPI_ID)   = (U32)_IrqHandler;    // Set interrupt vector
  *(U32*)(0xFFFFF128)                = (1 << SPI_ID);       // Clear pending interrupt
  *(U32*)(0xFFFFF120)                = (1 << SPI_ID);       // Enable Interrupt
#endif
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*             SPIInit
*
*  Description:
*    KMC TODO - combine this with other routine
*    Initialize the SPI subsystem.  Note, this routine might be combined
*    with another HW init routine to make sure registered are initialized
*    correctly the first time - and only once!
*
*  Parameters:
*    none
*
*  Return value:
*    void
*/
void SPIInit(void)
{
	_Init();
}

//
// Initialize TI TSC2006 chip
//
// Config word == 0x8000
// 0b10001000 = control byte 1, scan X and Y continuously, do not reset, do not stop
//
#if 0
// My original code
unsigned const char SWRESET_COMMAND[] = { 0x82 };
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND[] = { 0x60, 0x80, 0x02 };
unsigned const char START_PROCESSING_TOUCH_COMMAND[] = { 0x88 };

void TSInit(void)
{
	TS_SPI_X_Write(SWRESET_COMMAND, sizeof(SWRESET_COMMAND));		// Send software reset to reset registers
	uSDelay(10);																	// Delay just in case
	TS_SPI_X_Write(INITIALIZE_TOUCHCONTROLLER_COMMAND, sizeof(INITIALIZE_TOUCHCONTROLLER_COMMAND));		// Configure controller
	TS_SPI_X_Write(START_PROCESSING_TOUCH_COMMAND, sizeof(START_PROCESSING_TOUCH_COMMAND));				// Send command to start processing touch
}

// New code from Robs measurements
// According to Rob's captures - this should be 86	unsigned const char SWRESET_COMMAND[] = { 0x82 };
unsigned const char SWRESET_COMMAND[] = { 0x86 };
unsigned const char SWRESET_COMMAND2[] = { 0x85 };
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND[] = { 0x62, 0xA9, 0x24 };
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND2[] = { 0x6A, 0x01, 0x00 };
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND3[] = { 0x72, 0x50, 0x1F };
unsigned const char START_PROCESSING_TOUCH_COMMAND[] = { 0x84 };
#else
// Ken's hybrid
unsigned const char SWRESET_COMMAND[] = { 0x82 };			// Software reset, change from xyz, 12 bits to xy, 10 bits
unsigned const char SWRESET_COMMAND2[] = { 0x81 };			// Abort current conversion, change to 10 bits
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND[] = { 0x60, 0x80, 0x02 };	// Added enable of continuous scan
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND1[] = { 0x62, 0xA9, 0x24 };
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND2[] = { 0x6A, 0x01, 0x00 };
unsigned const char INITIALIZE_TOUCHCONTROLLER_COMMAND3[] = { 0x72, 0x50, 0x1F };
unsigned const char START_PROCESSING_TOUCH_COMMAND[] = { 0x88 };		// change to 10 bit, xy only

void TSInit(void)
{
   DummyWrite();			// Write a dummy byte to setup SPI bus

	TS_SPI_X_Write(SWRESET_COMMAND, sizeof(SWRESET_COMMAND));		// Send software reset to reset registers
	TS_SPI_X_Write(SWRESET_COMMAND2, sizeof(SWRESET_COMMAND2));		// Send abort current conversion
	TS_SPI_X_Write(INITIALIZE_TOUCHCONTROLLER_COMMAND, sizeof(INITIALIZE_TOUCHCONTROLLER_COMMAND));		// Configure controller
	TS_SPI_X_Write(INITIALIZE_TOUCHCONTROLLER_COMMAND1, sizeof(INITIALIZE_TOUCHCONTROLLER_COMMAND1));		// Configure controller
	TS_SPI_X_Write(INITIALIZE_TOUCHCONTROLLER_COMMAND2, sizeof(INITIALIZE_TOUCHCONTROLLER_COMMAND2));		// Configure controller
	TS_SPI_X_Write(INITIALIZE_TOUCHCONTROLLER_COMMAND3, sizeof(INITIALIZE_TOUCHCONTROLLER_COMMAND3));		// Configure controller
	TS_SPI_X_Write(START_PROCESSING_TOUCH_COMMAND, sizeof(START_PROCESSING_TOUCH_COMMAND));				// Send command to start processing touch
}
#endif

/*********************************************************************
*
*             TS_SPI_X_EnableCS
*
*  Description:
*    FS low level function. Sets the card slot active using the
*    chip select (CS) line.
*
*  Parameters:
*    none
*
*  Return value:
*    void
*/

void TS_SPI_X_EnableCS(void) {
	SELECT_TS;			// Ensure SPI_CSR1 is selected
	PIOB_CODR  = (1 <<  TS_CS_PIN);       // CS1 on eval board
}

void LED_SPI_X_EnableCS(void) {
	SELECT_LED;			// Ensure SPI_CSR3 is selected
	PIOD_SODR  = (1 <<  LED_CS_PIN);       // Set LED latch enable high before shifting out data
}

void DF_SPI_X_EnableCS(unsigned char chip) {
	if (0 == chip) {
		SELECT_DF0;								// Ensure SPI_CSR0 is selected
		PIOA_CODR  = (1 <<  SD0_CS_PIN);
	} // if
	else {
		SELECT_DF1;								// Ensure SPI_CSR2 is selected
		PIOD_CODR  = (1 <<  SD1_CS_PIN);
	} // else
}

/*********************************************************************
*
*             TS_SPI_X_DisableCS
*
*  Description:
*    FS low level function. Sets the card slot inactive using the
*    chip select (CS) line.
*
*  Parameters:
*    none
*
*  Return value:
*    void
*/

void TS_SPI_X_DisableCS(void) {
	PIOB_SODR  = (1 <<  TS_CS_PIN);       // CS1 on eval board
}

void LED_SPI_X_DisableCS(void) {
	PIOD_CODR  = (1 <<  LED_CS_PIN);       // Set LED latch enable line low after all data is sent
	PIOD_CODR  = (1 <<  LED_OE_PIN);       // Set LED brightness full bright
}

void DF_SPI_X_DisableCS(unsigned char chip) {
	if (0 == chip)
		PIOA_SODR  = (1 <<  SD0_CS_PIN);		// CS0
	else
		PIOD_SODR  = (1 <<  SD1_CS_PIN);		// CS2
}
/*********************************************************************
*
*             SPI_DMA_X_Read
*
*  Description:
*    FS low level function. Reads a specified number of bytes from MMC
*    card to buffer.
*
*  Parameters:
*    Unit      - Device Index
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/
void SPI_DMA_X_Read (U8 Unit, U8 * pData, int NumBytes) {
  U8 * p;
  volatile int v;

  v = SPI_RDR;  // Do a dummy read to clear RDRF flag
  if (USE_PDC) {
    p = _GetBufferAddr();
    _PDC_SetRx(p, NumBytes);
    _PDC_SetTx(_aFF, NumBytes);
    _PDC_EnableRx();
    _PDC_EnableTx();
    //
    // For larger blocks of data, block task to save CPU time (if an OS is present)
    //
    if (USE_OS && (NumBytes >= 512)) {
      SPI_IER = SPI_RXBUFF;
      FS_OS_WAIT(1000);
    }
    //
    // Make sure we have received all bytes
    //
    while (SPI_PDC_RCR);
    OS_ARM_DCACHE_InvalidateRange(p, (((NumBytes + 0x1f) >> 5) << 5));
    FS_MEMCPY(pData, p , NumBytes);
  } else {
    do {
      SPI_TDR = 0xff;
      while ((SPI_SR & (1 << 0)) == 0);
      *pData++ = SPI_RDR;
    } while (--NumBytes);
  }
}

/*********************************************************************
*
*             SPI_DMA_X_Write
*
*  Description:
*    FS low level function. Writes a specified number of bytes from
*    data buffer to the MMC/SD card.
*
*  Parameters:
*    Unit      - Device Index
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/

void SPI_DMA_X_Write(U8 Unit, const U8 * pData, int NumBytes) {
  U8 * p;

  if (USE_PDC) {
    p = _GetBufferAddr();
    FS_MEMCPY(p, pData, NumBytes);
    OS_ARM_DCACHE_CleanRange(p, (((NumBytes + 0x1f) >> 5) << 5));
    //
    // Start transmission using the PDC
    //
    _PDC_SetTx(p, NumBytes);
    _PDC_EnableTx();
    //
    // For larger blocks of data, block task to save CPU time (if an OS is present)
    //
    if (USE_OS && (NumBytes >= 512)) {
      SPI_IER = SPI_TXBUFE;
      FS_OS_WAIT(1000);
    }
    //
    // Check PDC counter to make sure all bytes are transferred
    //
    while (SPI_PDC_TCR);
  } else {
    do {
      SPI_TDR = *pData++;
      while ((SPI_SR & (1 << 1)) == 0); // Check TDRE
    } while (--NumBytes);

  }
  //
  // Check TXEMPTY-flag to make sure the last byte has left the serializer (has been sent)
  //
  while ((SPI_SR & (1 << 9)) == 0);   // Check TXEMPTY for sureness
}

/*********************************************************************
*
*             TS_SPI_X_Read
*
*  Description:
*    TS low level function. Reads a specified number of bytes from the
*    Touch Screen to the buffer.
*
*  Parameters:
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/
void TS_SPI_X_Read(U8 * pData, int NumBytes)
{
volatile int v;

	TS_SPI_X_EnableCS();
	v = SPI_RDR;  // Do a dummy read to clear RDRF flag
	do
	{
      SPI_TDR = 0x00;
      while ((SPI_SR & RDRF) == 0)
			;
      *pData++ = SPI_RDR;
   } while (--NumBytes);
	TS_SPI_X_DisableCS();
}


/*********************************************************************
*
*             LED_SPI_X_Write
*
*  Description:
*    LED low level function.  Writes a specified number of bytes from
*    data buffer to the LED controller chip.
*
*  Parameters:
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/
void LED_SPI_X_Write(const U8 * pData, int NumBytes)
{
	_SPI_Common_X_Write(pData, NumBytes);	// Common SPI Tx routine
	LED_SPI_X_EnableCS();						// Latch data into register
	LED_SPI_X_DisableCS();
}

/*********************************************************************
*
*             TS_SPI_X_Write
*
*  Description:
*    FS low level function. Writes a specified number of bytes from
*    data buffer to the touch screen controller chip.
*
*  Parameters:
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/
void TS_SPI_X_Write(const U8 * pData, int NumBytes)
{
	TS_SPI_X_EnableCS();
	uSDelay(1);				// Added delay for Brian's observation that the clock is shifted before
								// Chip select is active during initial mode selection command
	_SPI_Common_X_Write(pData, NumBytes);	// Common SPI Tx routine
	TS_SPI_X_DisableCS();
}

/*********************************************************************
*
*             _SPI_Common_X_Write
*
*  Description:
*    Common SPI transmit routine.
*
*  Parameters:
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/
void _SPI_Common_X_Write(const U8 * pData, int NumBytes)
{
   do
	{
		SPI_TDR = *pData++;
		while ((SPI_SR & TDRE) == 0)		// Check TDRE
			;
		// Once the byte is shifted out wait 50us before sending the next byte!
	} while (--NumBytes);

	//
	// Check TXEMPTY-flag to make sure the last byte has left the serializer (has been sent)
	//
	while ((SPI_SR & TXEMPTY) == 0)		// Check TXEMPTY for sureness
		;
}


void DummyWrite(void)
{
	SELECT_TS;			// Ensure SPI_CSR1 is selected
	// with NO chip select
	uSDelay(1);				// Added delay for Brian's observation that the clock is shifted before
								// Chip select is active during initial mode selection command
	SPI_TDR = 0;
	while ((SPI_SR & TDRE) == 0)		// Check TDRE
		;

	//
	// Check TXEMPTY-flag to make sure the last byte has left the serializer (has been sent)
	//
	while ((SPI_SR & TXEMPTY) == 0)		// Check TXEMPTY for sureness
		;
}

/*********************************************************************
*
*             TS_SPI_X_Write_Read
*
*  Description:
*    TS low level function. Reads a specified number of bytes from the
*    Touch Screen to the buffer.  This function DOES NOT touch chip select!
*    Chip select must be manually controlled by the calling function.
*
*  Parameters:
*    pData            - Pointer to a data buffer
*    NumBytes         - Number of bytes
*
*  Return value:
*    void
*/
void TS_SPI_X_Write_Read(const U8 * pWData, int NumWBytes, U8 * pRData, int NumRBytes)
{
volatile int v;

	do
	{
		SPI_TDR = *pWData++;
		while ((SPI_SR & TDRE) == 0)		// Check TDRE
			;
	} while (--NumWBytes);

	//
	// Check TXEMPTY-flag to make sure the last byte has left the serializer (has been sent)
	//
	while ((SPI_SR & TXEMPTY) == 0)		// Check TXEMPTY for sureness
		;

//	uSDelay(10);
	
	v = SPI_RDR;  // Do a dummy read to clear RDRF flag
	
#if 1
	do
	{
      SPI_TDR = 0x00;
      while ((SPI_SR & RDRF) == 0)
			;
	
//		uSDelay(10);			// test by adding inter-byte delay to make that 1/2 bit clearer
		
      *pRData++ = SPI_RDR;
   } while (--NumRBytes);
#else	
	// Configure SPI for 16 bit transfers
	SPI_CSR1  = 0					// KMC TODO - initially assuming communication with touch screen controller chip
	 									// is the same as serial flash (whose setting are left over from the settings used
	 									// by the 9261 access to the SD card).
            |(0 << 0)         // CPOL = 0, Clock polarity of idle is low
            |(1 << 1)         // NCPHA = 1??????????????
            |(0 << 3)         // Chip select 1 becomes inactive after transfer
            |(8 << 4)         // 16 bits per transfer
            |((_sbcr << 3) << 8)	   // 8..15: SCBR: Baud rate divider -- KMC TEST - slow it down
            |(0x100000);
	
      SPI_TDR = 0x0000;
      while ((SPI_SR & RDRF) == 0)
			;
	
      *((U16 *)pRData) = SPI_RDR;

	// Put it back in mode for 8 bit transfers
	SPI_CSR1  = 0					// KMC TODO - initially assuming communication with touch screen controller chip
	 									// is the same as serial flash (whose setting are left over from the settings used
	 									// by the 9261 access to the SD card).
            |(0 << 0)         // CPOL = 0, Clock polarity of idle is low
            |(1 << 1)         // NCPHA = 1??????????????
            |(0 << 3)         // Chip select 1 becomes inactive after transfer
            |(0 << 4)         // 8 bits per transfer
            |((_sbcr << 3) << 8)	   // 8..15: SCBR: Baud rate divider -- KMC TEST - slow it down
            |(0x100000);
#endif
}


int gdelay;

#pragma optimize=none
void uSDelay(int time)
{
	while (--time)
	{
		for (gdelay = 0; gdelay < 50; gdelay++)
		  	;
	}
}

void mSDelay(int);

#pragma optimize=none
void mSDelay(int time)
{
  	time *= 1000;
	while (--time)
	{
		for (gdelay = 0; gdelay < 12; gdelay++)
		  	;
	}
}

// simultaneously write	and read one unsigned char to SPI (mode 3). Blocking.
unsigned char xferSPI (unsigned char ch) {
	
	SPI_TDR = ch;

	// wait for shifting
	while ((0 == (SPI_SR & TDRE))) ;
	
	// wait for char
	while ((0 == (SPI_SR & RDRF))) ;
	
	ch = SPI_RDR;

	return (ch);
}

//
// Check to see if the SD card is inserted and not write-protected
// Returns true (1) of the SD card is present and not write-protected
// Returns false (0) if the SD card is not inserted or write-protected
//
char SDCardPresent(void)
{
	if (1 == FS_MCI_HW_IsCardPresent(0))
		return (!FS_MCI_HW_IsCardWriteProtected(0));
	else
		return 0;
}

//
// Check to see if the SD card is write-protected
// Returns true (1) of the SD card is write-protected
// Returns false (0) if the SD card is not write-protected
//
char SDCardWriteProtected(void)
{
	return (FS_MCI_HW_IsCardWriteProtected(0));
}

//
// Check to see if there is much free space on the SD card
// Returns true (1) if SD card is almost full
// Otherwise returns false (0)
//
char SDCardAlmostFull(void)
{
	return 0;
}
/*************************** End of file ****************************/
