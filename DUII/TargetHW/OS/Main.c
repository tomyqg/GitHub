/*********************************************************************
*          SEGGER MICROCONTROLLER SYSTEME GmbH
*   Solutions for real time microcontroller applications
**********************************************************************
File    : Main.c
Purpose :
--------- END-OF-HEADER --------------------------------------------*/

#define MAIN_C

#include <ctype.h>
#include <stdio.h>
#include "RTOS.H"
#include "BSP.h"
#include "DUII.h"
#include "FlashReadWrite.h"
#include "DataHandler.h"
#include "SDCardManager.h"
#include "SDCardRW.h"
#include "adc.h"
#include "commondefs.h"
#include "dataflash.h"
#include "LEDHandler.h"

void SPIInit(void);

// *** TEST #define TQM_Q_SIZE		(512 * 8)			// tqm file queue. enough memory for 8 SD card sectors
#define TQM_Q_SIZE		(1024 * 2)			// tqm file queue. enough memory for 4 SD card sectors

void MainTask(void);

static OS_STACKPTR int mainStack[4096]; /* Task stacks         */
OS_TASK mainTCB;                 /* Task-control-blocks */

void SDCardRWTask(void);

static OS_STACKPTR int SDcardRWStack[1024]; /* Task stacks         */
OS_TASK SDcardRWTCB;                 /* Task-control-blocks */

void CommSimulatorTask(void);
#ifndef _WINDOWS
void CommTask(void);
#endif

OS_STACKPTR int commStack[2048];  /* Task stacks         */
OS_TASK commTCB;                 /* Task-control-blocks */

static OS_STACKPTR int LEDStack[512];  /* Task stacks         */
static OS_TASK LEDTCB;                 /* Task-control-blocks */

void TQMWriteTask(void);

static OS_STACKPTR int TQMStack[512]; /* Task stacks  -- combine with SDcardRW if works  */
OS_TASK TQMWriteTCB;                 /* Task-control-blocks */

void LapTimerTask(void);

static OS_STACKPTR int lapTimerStack[512];  /* Task stacks         */
OS_TASK lapTimerTCB;                 /* Task-control-blocks */

static OS_STACKPTR int sdCardManagerStack[512];
OS_TASK sdCardManagerTCB;

// Mailboxes
OS_MAILBOX MB_SDcardRW;				// mailbox to SD card write task

// Semaphores
OS_RSEMA SEMA_GlobalData;			// resource semaphore around global data

// Queues
OS_Q Queue_tqm;						// queue of data to be written to .tqm file
char tqmQMem[TQM_Q_SIZE];			// where to get memory for queue
	
// OS Events

// Global Data Declarations

#define	NUM_USER_SETTINGS_MSGS	5
//char UserSettingsMessageBuffer[NUM_USER_SETTINGS_MSGS * sizeof(GLB_DATA_STRUCTURE)];
char UserSettingsMessageBuffer[NUM_USER_SETTINGS_MSGS * sizeof(int)];

void App_COM_Init(void);

/*********************************************************************
*
*       main
*/
void main(void) {
	u16 numpages, bytesperpage;
	int callwrite = false;
	unsigned int featureVal;

	OS_IncDI();                      /* Initially disable interrupts  */
	OS_InitKern();                   /* Initialize OS                 */
	OS_InitHW();                     /* Initialize Hardware for OS    */

	InitializeCriticalVariables();	/* Initialize application critical variables */

	App_COM_Init();                  /* Initialize Serial port for application */
	InitADC();                       /* Turn on A-D converter */
	SPIInit();
	DataFlash_Init((u08) 0, &numpages, &bytesperpage);
	unsavedSysData.dataflash[0].numPages = numpages;
	unsavedSysData.dataflash[0].bytesPerPage = bytesperpage;
	DataFlash_Init((u08) 1, &numpages, &bytesperpage);
	unsavedSysData.dataflash[1].numPages = numpages;
	unsavedSysData.dataflash[1].bytesPerPage = bytesperpage;

	// Initialize LED structures before flashing starts
	InitializeLEDStructures();
	
	// read the user configuration from flash
	ReadUserData();
	
	// check for unprogrammed user data
	if (0xFF == unsavedSysData.unitInfo.owner[0]) {
		unsavedSysData.unitInfo.owner[0] = '\0';
		unsavedSysData.unitInfo.phone[0] = '\0';
		callwrite = true;
	} // if
	// Validate data - particularly wifiPass as that may have been uninitialized data  
	// since it may have been past the end of the old "name" field string.
	if (!(isalnum(unsavedSysData.unitInfo.wifiName[0]))) {
		strcpy (unsavedSysData.unitInfo.wifiName, DEFAULT_WIFI_NAME);
		sprintf (unsavedSysData.unitInfo.wifiName + strlen(unsavedSysData.unitInfo.wifiName), "%04d", unsavedSysData.unitInfo.serno % 10000);
		callwrite = true;
	} // if
	if (!(isalnum(unsavedSysData.unitInfo.wifiPass[0]))) {
		strcpy (unsavedSysData.unitInfo.wifiPass, DEFAULT_WIFI_PASS);
		callwrite = true;
	} // if
	if (0xFFFFFFFF == unsavedSysData.unitInfo.serno) {
		unsavedSysData.unitInfo.serno = DEFAULT_SERIAL_NUMBER;
		callwrite = true;
	} // if
	if (0xFFFFFFFF == unsavedSysData.unitInfo.week) {
		unsavedSysData.unitInfo.week = DEFAULT_BUILD_WEEK;
		callwrite = true;
	} // if
	if (0xFFFFFFFF == unsavedSysData.unitInfo.year) {
		unsavedSysData.unitInfo.year = DEFAULT_BUILD_YEAR;
		callwrite = true;
	} // if
	// allows bootloaders up to V9.98. Anything else is invalid and will be set back to default
	if ((0xFFFFFFFF == unsavedSysData.unitInfo.bootloaderSwRev) || (0 == unsavedSysData.unitInfo.bootloaderSwRev) || (unsavedSysData.unitInfo.bootloaderSwRev > 998)) {
		unsavedSysData.unitInfo.bootloaderSwRev = DEFAULT_BOOTLOADER_REV;		// original bootloader does not identify itself
		callwrite = true;
	} // if
	if (0xFFFFFF57 != (unsavedSysData.unitInfo.feature1)) 	{	// clear lower byte of feature1 (used for WiFi)
		unsavedSysData.unitInfo.feature1 = 0xFFFFFFFF;
		callwrite = true;
	} // if
	if (0xFFFFFFFF != unsavedSysData.unitInfo.feature2) {		// clear feature bitmaps for future use
		unsavedSysData.unitInfo.feature2 = 0xFFFFFFFF;
		callwrite = true;
	} // if
	if (0xFFFFFFFF != unsavedSysData.unitInfo.feature3) {
		unsavedSysData.unitInfo.feature3 = 0xFFFFFFFF;
		callwrite = true;
	} // if
	if (0xFFFFFFFF != unsavedSysData.unitInfo.feature4) {
		unsavedSysData.unitInfo.feature4 = 0xFFFFFFFF;
		callwrite = true;
	} // if

	if (callwrite)
		WriteUserData();

// *** This forces WiFi Enable. Use bootloader V1.80 instead.
//	unsavedSysData.unitInfo.feature1 = 0xFFFFFF57;
//	WriteUserData();
// ***
	
	// check for wifi enabled
	featureVal = (unsavedSysData.unitInfo.feature1 & 0x000000FF);
	if (0x00000057 == featureVal)			// 'W'
		unsavedSysData.wifiEnabled = 1;
	
// create mailboxes and semaphores and queues, oh my
	OS_CREATEMB(&MB_SDcardRW, sizeof(int), NUM_USER_SETTINGS_MSGS, UserSettingsMessageBuffer);
	OS_CREATERSEMA(&SEMA_GlobalData);
	OS_Q_Create (&Queue_tqm, &tqmQMem, TQM_Q_SIZE);

	// these initializations are here because decisions are made on startup that need suPresent, etc.
	suData.suPresent = FALSE;					// TRUE if su is connected and initialized
	suData.tmState = STARTUP;					// where in the state machine
	suData.traqDataConnected = 0;				// whether io interface is present
	suData.suSwRev = 0;							// software rev from SU * 100
	suData.suDataRev = 0;						// data rev from SU
	suData.suHwRev = 0;							// hardware rev from SU * 100
	if (unsavedSysData.unitInfo.bootloaderSwRev <= FIRST_GEN_BOOTLOADER)	// if old bootloader, turn off usb camera support
		(vehicleData.usbCameraDelay &= 0x7F);

	// create tasks
	OS_CREATETASK(&mainTCB, "MainTask", MainTask, 100, mainStack);					// main loop with gui, changes to priority 5 later
	OS_CREATETASK(&SDcardRWTCB, "SDCardRWTask", SDCardRWTask, 80, SDcardRWStack);	// writes out config files, etc.
	OS_CREATETASK(&commTCB, "CommTask", CommTask, 120, commStack);					// communicates with su and creates data for writing
	OS_CREATETASK(&lapTimerTCB, "LapTimerTask", LapTimerTask, 110, lapTimerStack);	// times the laps
	OS_CREATETASK(&TQMWriteTCB, "TQMWriteTask", TQMWriteTask, 10, TQMStack);		// low priority file write task just for tqm files
	OS_CREATETASK(&LEDTCB, "LEDHandlerTask", LEDHandlerTask, 20, LEDStack);			// Low priority LED monitor task
	OS_CREATETASK(&sdCardManagerTCB, "SDCardManagerTask", SDCardManagerTask, 50, sdCardManagerStack);	// watches for card inserts/removals

//    _WDT_MR &= ~(1 << 15);                    // Enable watchdog ??

	OS_Start();
}
