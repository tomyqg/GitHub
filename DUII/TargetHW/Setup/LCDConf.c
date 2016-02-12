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

#include "RTOS.h"
#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "LCDConf.h"

//
// KMC TEST
//
//#include "BSP.h"

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/

#define NEWLCD      (1)
#define NEWTOUCH    (1)

//
// Physical display size
//
// NEW SCREEN
#ifdef NEWLCD
#define XSIZE_PHYS 480
#define YSIZE_PHYS 272
#else
#define XSIZE_PHYS 240
#define YSIZE_PHYS 320
#endif

//
// Virtual display size
//
#define VXSIZE_PHYS (XSIZE_PHYS * 1)
#define VYSIZE_PHYS (YSIZE_PHYS * 3)

//
// Color conversion
//
#define COLOR_CONVERSION  GUICC_M565

//
// Display driver
//
#define DISPLAY_DRIVER  &GUIDRV_Lin_OXY_16_API    

//
// Pixel width in bytes
//
 #define PIXEL_WIDTH  2

//
// Video RAM address
//
#define VRAM_ADDR  ((U32)&_aVRAM[0] + 0x8000000)  // Write to non-cached SDRAM area

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif
#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif
#ifndef   VRAM_ADDR
  #define VRAM_ADDR 0
#endif

#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define CPU_MCK                            (12000000)

/****** Touch controller values *************************************/
#ifdef NEWTOUCH
#define TOUCH_AD_TOP                       (60)
// Add safety margin (to bottom only)
//#define TOUCH_AD_BOTTOM                    (960)
#define TOUCH_AD_BOTTOM                    (810)
#define TOUCH_AD_LEFT                      (980)
#define TOUCH_AD_RIGHT                     (40)
#else
#define TOUCH_AD_TOP                       (0x047)
#define TOUCH_AD_BOTTOM                    (0x3D8)
#define TOUCH_AD_LEFT                      (0x3DA)
#define TOUCH_AD_RIGHT                     (0x03F)
#endif
#define TOUCH_TIMER_INTERVAL               (10)

/****** SFRs used for LCD and touch *********************************/
//
// PIOs
//
#define PIOA_IDR                           (*(volatile U32*) (0xFFFFF444))
#define PIOA_PPUDR                         (*(volatile U32*) (0xFFFFF460))
#define PIOA_PPUER                         (*(volatile U32*) (0xFFFFF464))
#define PIOA_ASR                           (*(volatile U32*) (0xFFFFF470))
#define PIOA_BSR                           (*(volatile U32*) (0xFFFFF474))
#define PIOA_PDR                           (*(volatile U32*) (0xFFFFF404))

#define PIOC_IDR                           (*(volatile U32*) (0xFFFFF844))
#define PIOC_PPUDR                         (*(volatile U32*) (0xFFFFF860))
#define PIOC_PPUER                         (*(volatile U32*) (0xFFFFF864))
#define PIOC_ASR                           (*(volatile U32*) (0xFFFFF870))
#define PIOC_BSR                           (*(volatile U32*) (0xFFFFF874))
#define PIOC_PDR                           (*(volatile U32*) (0xFFFFF804))

//
// PMC
//
#define AT91C_BASE_PMC                    (*(volatile U32*) (0xFFFFFC10))  // (PMC) System Clock Enable Register

#define AT91C_ID_TSADC_BIT                (20)                             // Touch Screen / ADC Controller
#define AT91C_ID_LCDC_BIT                 (23)                             // LCD Controller
#define AT91C_LCDC_PS_BIT                 (0x3 <<  0)                      // (LCDC) LCD Contrast Counter Prescaler
#define AT91C_LCDC_PS_BIT_NOTDIVIDED_BIT  (0x0)                            // (LCDC) Counter Freq is System Freq.
#define AT91C_LCDC_POL_BIT                (0x1 <<  2)                      // (LCDC) Polarity of output Pulse
#define AT91C_LCDC_ENA_PWMGEMENABLED      (0x1 <<  3)                      // (LCDC) PWM Generator Disabled
#define AT91C_LCDC_POL_BIT_POSITIVEPULSE  (0x1 <<  2)                      // (LCDC) Positive Pulse
#define AT91C_LCDC_DMAEN_BIT              (0x1 <<  0)                      // (LCDC) DAM Enable
#define AT91C_LCDC_PWR_BIT                (0x1 <<  0)                      // (LCDC) LCD Module Power Control

//
// LCDC
//
#define LCDC_DMACON                       (*(volatile U32*) (0x0050001C))  // DMACON
#define LCDC_PWRCON                       (*(volatile U32*) (0x0050083C))  // DMACON
#define LCDC_LCDCON1                      (*(volatile U32*) (0x00500800))  // LCDCON1
#define LCDC_LCDCON2                      (*(volatile U32*) (0x00500804))  // LCDCON2
#define LCDC_LCDFRMCFG                    (*(volatile U32*) (0x00500810))  // LCDFRMCFG
#define LCDC_TIM1                         (*(volatile U32*) (0x00500808))  // LCDTIM1
#define LCDC_TIM2                         (*(volatile U32*) (0x0050080C))  // LCDTIM2
#define LCDC_CTRSTCON                     (*(volatile U32*) (0x00500840))  // CTRSTCON
#define LCDC_CTRSTVAL                     (*(volatile U32*) (0x00500844))  // CTRSTVAL
#define LCDC_DMAFRMCFG                    (*(volatile U32*) (0x00500018))  // DMAFRMCFG
#define LCDC_FIFO                         (*(volatile U32*) (0x00500814))  // FIFO
#define LCDC_DMABADDR1                    (*(volatile U32*) (0x00500000))  // DMABADDR1

#define LCD_MCK                           ((CPU_MCK * 50 / 3) / 2)
#define LCD_PIXELCLOCK                    (LCD_FRAMESIZE_PIXELS * LCD_FRAMERATE / LCD_IFWIDTH)
#define LCD_FRAMESIZE_PIXELS              (XSIZE_PHYS * YSIZE_PHYS * (PIXEL_WIDTH * 8))

#ifdef NEWLCD
// NEW SCREEN
#define LCD_FRAMERATE                     75
#define LCD_IFWIDTH                       24
#define LCD_TIMING_HFP                    2                               // Horizontal front porch in LCDDOTCLK cycles.
#define LCD_TIMING_HBP                    2                               // Horizontal back porch in LCDDOTCLK cycles.
#define LCD_TIMING_HPW                    41                              // Horizontal pulse width in LCDDOTCLK cycles.
#define LCD_TIMING_VFP                    1
#define LCD_TIMING_VBP                    1
#define LCD_TIMING_VPW                    2
#define LCD_TIMING_VHDLY                  1
#else
#define LCD_FRAMERATE                     60
#define LCD_IFWIDTH                       24
#define LCD_TIMING_HFP                    11                               // Horizontal front porch in LCDDOTCLK cycles.
#define LCD_TIMING_HBP                    17                               // Horizontal back porch in LCDDOTCLK cycles.
#define LCD_TIMING_HPW                    5                                // Horizontal pulse width in LCDDOTCLK cycles.
#define LCD_TIMING_VFP                    2
#define LCD_TIMING_VBP                    4
#define LCD_TIMING_VPW                    3
#define LCD_TIMING_VHDLY                  1
#endif

#define LCD_FRAMESIZE                     (LCD_FRAMESIZE_PIXELS / 32)

//
// TS
//
#define TSADCC_CR                         (*(volatile U32*) (0xFFFD0000))  // Control register
#define TSADCC_MR                         (*(volatile U32*) (0xFFFD0004))  // Mode register
#define TSADCC_TRGR                       (*(volatile U32*) (0xFFFD0008))  // Trigger register
#define TSADCC_TSR                        (*(volatile U32*) (0xFFFD000C))  // Touch Screen register
#define TSADCC_SR                         (*(volatile U32*) (0xFFFD001C))  // Status register
#define TSADCC_CDR0                       (*(volatile U32*) (0xFFFD0030))  // Channel data register 0
#define TSADCC_CDR1                       (*(volatile U32*) (0xFFFD0034))  // Channel data register 1
#define TSADCC_CDR2                       (*(volatile U32*) (0xFFFD0038))  // Channel data register 2
#define TSADCC_CDR3                       (*(volatile U32*) (0xFFFD003C))  // Channel data register 3

#define TSADCC_ADCCLK                     (1000000)                        // Touchscreen ADC clock frequency to use
#define TSADCC_SR_EOC3_BIT                (3)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Video RAM
//
__no_init static U16 _aVRAM[VXSIZE_PHYS * VYSIZE_PHYS];

//
// OS timers
//
static OS_TIMER _TOUCHTIMER;
static U32      _TOUCHTIMER_INTERVAL = 10;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ExecTouch
*/
static void _ExecTouch(void) {
  GUI_TOUCH_Exec();
  OS_RetriggerTimer(&_TOUCHTIMER);
}

/*********************************************************************
*
*       _InitController
*
* Purpose:
*   Initializes the LCD controller
*
*/
static void _InitController(unsigned LayerIndex) {
  U32 TouchOrientation;

  //
  // Initialize touch screen
  //
  AT91C_BASE_PMC |= (1 << AT91C_ID_TSADC_BIT);  // Enable Touch Screen Controller clock
  PIOA_ASR        = 0                           // Setup ports to peripheral A
                    | (0x01 << 17)              // PA17: AD0
                    | (0x01 << 18)              // PA18: AD1
                    | (0x01 << 19)              // PA19: AD2
                    | (0x01 << 20)              // PA20: AD3
                    ;
  PIOA_PPUDR      = 0
                    | (0x01 << 17)              // PA17: AD0
                    | (0x01 << 18)              // PA18: AD1
                    | (0x01 << 19)              // PA19: AD2
                    | (0x01 << 20)              // PA20: AD3
                    ;
//// Original Values
//  TSADCC_MR       = 0
//                    | (0x01 <<  0)              // Touch screen only mode
//                    | (0x01 <<  6)              // Enable pen detect
//						  | (0x3B <<  8)              // Prescaler rate selection (PRESCAL = (MCK / (2 * ADCCLK)) + 1))
//                    | (0x04 << 16)              // Start Up Time (STARTUP = (time[s] * ADCCLK) / (1000000 * 8) - 1))
//                    | (0x00 << 24)              // Track and Hold time (SHTIM = (time[ns] x ADCCLK) / 1000000000 - 1)
//                    | (0x0E << 28)              // Pen Detect debouncing period (PENDBC = (2^PENDBC) / ADCCLK)
//                    ;
//  TSADCC_TSR      =   (0x00 << 24);             // Track and Hold time (SHTIM = (time[ns] x ADCCLK) / 1000000000 - 1)	- original
// New Values  
  TSADCC_MR       = 0
                    | (0x01 <<  0)              // Touch screen only mode
                    | (0x01 <<  6)              // Enable pen detect
                    | (0x3F <<  8)              // Prescaler rate selection (PRESCAL = (MCK / (2 * ADCCLK)) + 1))
                    | (0x7F << 16)              // Start Up Time (STARTUP = (time[s] * ADCCLK) / (1000000 * 8) - 1)) new
                    | (0x0F << 24)              // Track and Hold time (SHTIM = (time[ns] x ADCCLK) / 1000000000 - 1)  new
                    | (0x0E << 28)              // Pen Detect debouncing period (PENDBC = (2^PENDBC) / ADCCLK)
                    ;
  TSADCC_TSR      =   (0x0F << 24);             // Track and Hold time (SHTIM = (time[ns] x ADCCLK) / 1000000000 - 1)		- new value
  //
  // Calibrate touch
  //
  TouchOrientation = (GUI_MIRROR_X * LCD_GetMirrorXEx(0)) |
                     (GUI_MIRROR_Y * LCD_GetMirrorYEx(0)) |
                     (GUI_SWAP_XY  * LCD_GetSwapXYEx (0)) ;
  GUI_TOUCH_SetOrientation(TouchOrientation);
#ifdef NEWTOUCH
  GUI_TOUCH_Calibrate(GUI_COORD_X, 0, XSIZE_PHYS, TOUCH_AD_RIGHT, TOUCH_AD_LEFT);  // x axis
  GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, YSIZE_PHYS, TOUCH_AD_BOTTOM, TOUCH_AD_TOP);   // y axis
#else
  GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 240, TOUCH_AD_TOP , TOUCH_AD_BOTTOM);  // x axis
  GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 320, TOUCH_AD_LEFT, TOUCH_AD_RIGHT);   // y axis
#endif
  //
  // Initialize LCD
  //
  PIOC_IDR        = 0xE8;
  PIOC_PPUDR      = 0xE8;
  PIOC_ASR        = 0xE8;
  PIOC_PDR        = 0xE8;
#ifdef NEWLCD
// NEW SCREEN
  PIOC_IDR        = 0x03FFBE12;
  PIOC_PPUDR      = 0x03FFBE12;
  PIOC_BSR        = 0x03FFBE12;
  PIOC_PDR        = 0x03FFBE12;
#else
  PIOC_IDR        = 0x03FFBE02;
  PIOC_PPUDR      = 0x03FFBE02;
  PIOC_BSR        = 0x03FFBE02;
  PIOC_PDR        = 0x03FFBE02;
#endif
  AT91C_BASE_PMC  = 1 << AT91C_ID_LCDC_BIT;                         // Enable peripheral clock
  LCDC_DMACON     = 0x00;                                           // Disable the LCD and the DMA
  LCDC_PWRCON     = 0 << 1;
#ifdef NEWLCD
// NEW SCREEN
  LCDC_LCDCON1    = ((LCD_MCK / 2 / LCD_PIXELCLOCK) - 3) << 12;     // DCLK of 10.00MHz
  LCDC_LCDCON2    = 0x80008682;                                     // Make VSYNC and HSYNC normally high, pulse low
#else
  LCDC_LCDCON1    = ((LCD_MCK / 2 / LCD_PIXELCLOCK) - 2) << 12;     // Configure the LCD controller
  LCDC_LCDCON2    = 0x80008082;
#endif
  LCDC_LCDFRMCFG  = ((XSIZE_PHYS - 1) << 21) | (YSIZE_PHYS - 1);
  LCDC_TIM1       = LCD_TIMING_VFP                                  // Configure timings
                |  (LCD_TIMING_VBP      << 8)
                | ((LCD_TIMING_VPW-1)   << 16)
                | ((LCD_TIMING_VHDLY-1) << 24);
  LCDC_TIM2       = (LCD_TIMING_HBP-1) | ((LCD_TIMING_HPW-1) << 8) | ((LCD_TIMING_HFP-1) << 24);
  LCDC_CTRSTCON  &= ~AT91C_LCDC_PS_BIT;                             // Configure contrast
  LCDC_CTRSTCON  |= AT91C_LCDC_PS_BIT_NOTDIVIDED_BIT;
  LCDC_CTRSTCON  &= ~AT91C_LCDC_POL_BIT;
  LCDC_CTRSTVAL   = 0x80;
  LCDC_CTRSTCON  |= AT91C_LCDC_POL_BIT_POSITIVEPULSE;
  LCDC_CTRSTCON  |= AT91C_LCDC_ENA_PWMGEMENABLED;
  LCDC_DMAFRMCFG  = LCD_FRAMESIZE | (LCDC_DMAFRMCFG & 0xFF000000);  // Configure DMA
  LCDC_DMAFRMCFG &= 0x00FFFFFF;
  LCDC_DMAFRMCFG |= ((2-1) << 24);
  LCDC_FIFO       = 2048 - (2 * 4 + 3);
  LCDC_DMABADDR1  = (U32)VRAM_ADDR;
  LCDC_DMACON     = AT91C_LCDC_DMAEN_BIT;
  LCDC_PWRCON     = AT91C_LCDC_PWR_BIT | (0x0C << 1);
  LCDC_FIFO       = 2048 - (2 * 4 + 3);
  LCDC_DMABADDR1  = (U32)VRAM_ADDR;                                 // Set frame buffer
  LCDC_DMACON     = AT91C_LCDC_DMAEN_BIT;                           // Enable DMA and LCD
  LCDC_PWRCON     = AT91C_LCDC_PWR_BIT | (0x0C << 1);
  //
  // Start touch measurement
  //
  OS_CREATETIMER(&_TOUCHTIMER, _ExecTouch, _TOUCHTIMER_INTERVAL);
}

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _SetDisplayOrigin()
*/
static void _SetDisplayOrigin(int x, int y) {
  //
  // Set start address for display data and enable LCD controller
  //
  LCDC_DMABADDR1 = (unsigned long) VRAM_ADDR + (y * XSIZE_PHYS * PIXEL_WIDTH);   // Needs to be set, before LCDC is enabled
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/
void LCD_X_Config(void) {
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  //
  // Display driver configuration, required for Lin-driver
  //
  if (LCD_GetSwapXYEx(0)) {
    LCD_SetSizeEx    (0, YSIZE_PHYS,   XSIZE_PHYS);
    LCD_SetVSizeEx   (0, VYSIZE_PHYS,  VXSIZE_PHYS);
  } else {
    LCD_SetSizeEx    (0, XSIZE_PHYS,   YSIZE_PHYS);
    LCD_SetVSizeEx   (0, VXSIZE_PHYS,  VYSIZE_PHYS);
  }
  LCD_SetVRAMAddrEx(0, (void *)VRAM_ADDR);
  //
  // Set user palette data (only required if no fixed palette is used)
  //
  #if defined(PALETTE)
    LCD_SetLUTEx(0, PALETTE);
  #endif
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  LCD_X_SETORG_INFO * pSetOrg;

  (void) LayerIndex;
  (void) pData;

  switch (Cmd) {
  //
  // Required
  //
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    // ...
    _InitController(0);
    return 0;
  case LCD_X_SETORG:
    //
    // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
    //
    pSetOrg = (LCD_X_SETORG_INFO *)pData;
    _SetDisplayOrigin(pSetOrg->xPos, pSetOrg->yPos);
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}

/*********************************************************************
*
*       GUI_TOUCH
*/

#if GUI_SUPPORT_TOUCH // Dummy routines needed when touch support is enabled

/*********************************************************************
*
*       Static data for global Touch functions
*
**********************************************************************
*/

static U32 _TouchX;
static U32 _TouchY;
static int _IsTouch;

/*********************************************************************
*
*       Local GUI_TOUCH functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _CheckTouch()
*
* Function decription:
*   Checks if a valid touch AD conversion has been done.
*
* Return value:
*   0: No valid touch value
*   1: Valid touch converted
*/

#undef	KEYBOARD_TRUNCATION_TEST

#ifdef KEYBOARD_TRUNCATION_TEST
#define	KEYBOARD_TRUNCATION_AMOUNT	0x000F
#endif

static int _CheckTouch(void) {
  //
  // Start conversion
  //
  
  TSADCC_CR = 2;
  //
  // Wait for conversion of last channel
  //
  while ((TSADCC_SR & TSADCC_SR_EOC3_BIT) != TSADCC_SR_EOC3_BIT);
  //
  // Calculate x & y value
  //
  
#ifdef NEWTOUCH
  _TouchX  = (TSADCC_CDR1 * 1024);
  _TouchX /=  TSADCC_CDR0;
  _TouchY  = (TSADCC_CDR3 * 1024);
  _TouchY /=  TSADCC_CDR2;
#else
  _TouchX  = (TSADCC_CDR3 * 1024);
  _TouchX /=  TSADCC_CDR2;
  _TouchY  = (TSADCC_CDR1 * 1024);
  _TouchY /=  TSADCC_CDR0;
#endif

#ifdef KEYBOARD_TRUNCATION_TEST
  // KMC TEST
  _TouchY = (_TouchY + (KEYBOARD_TRUNCATION_AMOUNT >> 1)) & ~KEYBOARD_TRUNCATION_AMOUNT;
  _TouchX = (_TouchX + (KEYBOARD_TRUNCATION_AMOUNT >> 1)) & ~KEYBOARD_TRUNCATION_AMOUNT;
#endif

  if (_TouchY < 0x400) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       Global GUI_TOUCH functions
*
**********************************************************************
*/

/*********************************************************************
*
*       GUI_TOUCH_X_ActivateX()
*
* Function decription:
*   Called from GUI, if touch support is enabled.
*   Switches on voltage on X-axis,
*   prepares measurement for Y-axis.
*   Voltage on Y-axis is switched off.
*/
void GUI_TOUCH_X_ActivateX(void) {
}

/*********************************************************************
*
*       GUI_TOUCH_X_ActivateY()
*
* Function decription:
*   Called from GUI, if touch support is enabled.
*   Switches on voltage on Y-axis,
*   prepares measurement for X-axis.
*   Voltage on X-axis is switched off.
*/
void GUI_TOUCH_X_ActivateY(void) {
}


/*********************************************************************
*
*       GUI_TOUCH_X_MeasureX()
*
* Function decription:
*   Called from GUI, if touch support is enabled.
*   Measures voltage of X-axis.
*/
int  GUI_TOUCH_X_MeasureX(void) {
  if (_IsTouch) {
    return _TouchX;
  }
  return -1;
}

/*********************************************************************
*
*       GUI_TOUCH_X_MeasureY()
*
* Function decription:
*   Called from GUI, if touch support is enabled.
*   Measures voltage of Y-axis.
*/
int  GUI_TOUCH_X_MeasureY(void) {
  _IsTouch = _CheckTouch();
  if ((TSADCC_SR & (1 << 20))) {
    return 1;
  }
  if ((TSADCC_SR & (1 << 21))) {
    return 1;
  }
  if (_IsTouch) {
    return _TouchY;
  }
  return -1;
}
#endif // GUI_SUPPORT_TOUCH

//
// Dummy routines for case with old touch screen
//
int StartTouchCalibration(void)
{
	return 0;
}
int GetTouchPoint(void)
{
	return 0;
}

/*************************** End of file ****************************/
