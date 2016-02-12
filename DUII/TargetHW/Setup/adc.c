/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2009  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

//#define	LED_TEMP_TEST_CODE	1

#include "RTOS.h"
#include "adc.h"
#include "Global.h"
#include "SPI_API.h"
#ifdef	LED_TEMP_TEST_CODE
#include "DataHandler.h"
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define AT91C_ID_TSADC_BIT                (20)                             // Touch Screen / ADC Controller

//
// ADC
//
#define TSADCC_CR							(*(volatile U32*) (0xFFFD0000))	// Control register
#define TSADCC_MR							(*(volatile U32*) (0xFFFD0004))	// Mode register
#define TSADCC_TRGR							(*(volatile U32*) (0xFFFD0008))	// Trigger register
#define TSADCC_TSR							(*(volatile U32*) (0xFFFD000C))	// Touch Screen register
#define TSADCC_CHER							(*(volatile U32*) (0xFFFD0010)) // (TSADC_CHER) Channel Enable Register
#define TSADCC_CHDR							(*(volatile U32*) (0xFFFD0014)) // (TSADC_CHDR) Channel Disable Register
#define TSADCC_CHSR							(*(volatile U32*) (0xFFFD0018)) // (TSADC_CHSR) Channel Status Register
#define TSADCC_SR							(*(volatile U32*) (0xFFFD001C))	// Status register
#define TSADCC_LCDR							(*(volatile U32*) (0xFFFD0020)) // (TSADC_LCDR) Last Converted Register
#define TSADCC_IER							(*(volatile U32*) (0xFFFD0024)) // (TSADC_IER) Interrupt Enable Register
#define TSADCC_IDR							(*(volatile U32*) (0xFFFD0028)) // (TSADC_IDR) Interrupt Disable Register
#define TSADCC_IMR							(*(volatile U32*) (0xFFFD002C)) // (TSADC_IMR) Interrupt Mask Register
#define TSADCC_CDR0							(*(volatile U32*) (0xFFFD0030))	// Channel data register 0
#define TSADCC_CDR1							(*(volatile U32*) (0xFFFD0034))	// Channel data register 1
#define TSADCC_CDR2							(*(volatile U32*) (0xFFFD0038))	// Channel data register 2
#define TSADCC_CDR3							(*(volatile U32*) (0xFFFD003C))	// Channel data register 3
#define TSADCC_CDR4							(*(volatile U32*) (0xFFFD0040))	// Channel data register 4
#define TSADCC_CDR5							(*(volatile U32*) (0xFFFD0044))	// Channel data register 5

#define TSADCC_ADCCLK						(1000000)						// ADC clock frequency to use

/*********************************************************************
*
*       InitADC
*
* Purpose:
*   Initializes the A/D converter
*
*/
void InitADC(void) {

  //
  // Setup the ports
  //
  PMC_PCER			|= (1 << AT91C_ID_TSADC_BIT);  // Enable ADC clock
  PIOA_ASR        = 0                           // Setup ports to peripheral A
                    | (0x01 << 17)              // PA17: AD0
                    | (0x01 << 18)              // PA18: AD1
                    | (0x01 << 19)              // PA19: AD2
                    | (0x01 << 20)              // PA20: AD3
                    ;
  PIOA_ODR        = 0                           // disable port output
                    | (0x01 << 17)              // PA17: AD0
                    | (0x01 << 18)              // PA18: AD1
                    | (0x01 << 19)              // PA19: AD2
                    | (0x01 << 20)              // PA20: AD3
                    ;	
  PIOA_PUDR       = 0							// disable pullups
                    | (0x01 << 17)              // PA17: AD0
                    | (0x01 << 18)              // PA18: AD1
                    | (0x01 << 19)              // PA19: AD2
                    | (0x01 << 20)              // PA20: AD3
					;
  PIOA_PDR        = 0                           // disable port so they can be used as A-Ds
                    | (0x01 << 17)              // PA17: AD0
                    | (0x01 << 18)              // PA18: AD1
                    | (0x01 << 19)              // PA19: AD2
                    | (0x01 << 20)              // PA20: AD3
                    ;
  PIOD_PDR =		0							// disable PIO port D pins 6 and 7 so they can be used as A-Ds
	  				| (1 << 6)
					| (1 << 7)
					;

  // setup the touchscreen adc
  TSADCC_CHER =		0
	  				| (1 << 0)					// channel 0		// 12V power line
//	  				| (1 << 1)					// channel 1
//	  				| (1 << 2)					// channel 2
//	  				| (1 << 3)					// channel 3
	  				| (1 << 4)					// channel 4		// light sensor
	  				| (1 << 5)					// channel 5		// temperature sensor
					;

  TSADCC_MR       = 0							// adc mode register
// KMC TEST - try 10 bit mode	  				| (1 << 4)					// low res
	  				| (0 << 4)					// high res
                    | (0x3F <<  8)				// Prescaler rate selection (PRESCAL = (MCK / (2 * ADCCLK)) + 1))
                    | (0x7F << 16)				// Start Up Time (STARTUP = (time[s] * ADCCLK) / (1000000 * 8) - 1))
                    | (0x0F << 24)				// Track and Hold time (SHTIM = (time[ns] x ADCCLK) / 1000000000 - 1)
					;

  TSADCC_TRGR	  =	0x06;						// continuous mode

  TSADCC_IDR	  =	0;							// disable interrupts

  TSADCC_CR =		0x02;						// start conversions

} // InitADC
					/*********************************************************************
*
*       unsigned char GetADC( unsigned char channel)
*
* Function decription:
*   Checks if a valid touch AD conversion has been done.
*
* Return value:
*   adc result of specified channel
*/

// KMC TEST - try 10 bit mode	unsigned char GetADC(unsigned char channel) {
// KMC TEST - try 10 bit mode	unsigned char adc_result;
unsigned int GetADC(unsigned char channel) {
	unsigned int adc_result;
	
	switch (channel) {
		case 0:
			adc_result = TSADCC_CDR0;
			break;
		case 1:
			adc_result = TSADCC_CDR1;
			break;
		case 2:
			adc_result = TSADCC_CDR2;
			break;
		case 3:
			adc_result = TSADCC_CDR3;
			break;
		case 4:
			adc_result = TSADCC_CDR4;
			break;
		case 5:
#ifdef LED_TEMP_TEST_CODE
			{
				int temp;
				
				temp = (int) (200.0 * (1.0 + DataValues[Y_G].fVal)); // scale -1G = 0, +1G = 400
				if (temp > 400)
					temp = 400; // ensure proper ranges
				if (temp < 0)
					temp = 0;
				adc_result = temp + 200;  // -1G = 200 value, +1G = 600 value
			}
#else
			adc_result = TSADCC_CDR5;
#endif
			break;
	} // switch

  return (adc_result);

}

/*************************** End of file ****************************/
