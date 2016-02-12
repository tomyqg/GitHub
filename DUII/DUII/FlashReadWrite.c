#ifndef _WINDOWS
#ifdef PROFILER
#include "profiler.h"
#define THIS_TASK	MAINTASK_NUM		// functions in FlashReadWrite.c are primarily called by DUIIMain
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "commondefs.h"
#include "GUI.h"
#include "FlashReadWrite.h"
#include "InterTask.h"
#include "ConfigScreens.h"
#include "DUII.h"
#include "LEDHandler.h"
#include "RunningGauges.h"
#include "DataHandler.h"

#ifdef _WINDOWS
#include <direct.h>
#include "..\Application\dirent.h"
#else
#include "RTOS.H"
#include "FS.h"
#include "SPI_API.h"
#include "dbgu.h"
#include "adc.h"

#ifdef PROFILER
#include "profiler.h"
#endif

#endif
#include "LEDHandler.h"
#include "InterTask.h"
#include "RaceSetupScreens.h"
#include "ConfigurableGaugeCommon.h"

//
// External Data and Code references
//
extern void reset( void );
extern int ReFlashYorN( int, int );
#ifndef _WINDOWS
extern char OkToUseSDCard(void);
#endif
extern void DisplaySplashScreen3(void);
extern void ValidateCameraData(void);

extern int systemSettingsPopupScreen;

//
// Constants
//
#define	SIZE_OF_VER_00_00_SYS_DATA_STRUCTURE	220
#define	SIZE_OF_VER_00_01_SYS_DATA_STRUCTURE	220
// Note: as of 9/18/12 sizeof(sysData) = 244
#define	SIZE_OF_VER_00_02_SYS_DATA_STRUCTURE	244

#define	SIZE_OF_VER_00_00_VEHICLE_DATA_STRUCTURE	100
#define	SIZE_OF_VER_00_01_VEHICLE_DATA_STRUCTURE	108

const char SOFTWARE_VERSION[] = "00.00";
const char SYSTEM_VERSION[] = "00.02";
const char VEHICLE_VERSION[] = "00.01";
const char DRIVER_VERSION[] = "00.00";
const char TRACK_VERSION[] = "00.00";

const char SYSTEM_FILE_NAME[] = "SystemConfig";
const char BACKUP_SYSTEM_FILE_NAME[] = "BackupSystemConfig";
const char DEFAULT_DRIVER[] = "Driver A";
const char DEFAULT_VEHICLE[] = "Spec Miata 99-05";
const char DEFAULT_TRACK[] = "";

const char SYSTEM_PATH[] = { LEADING_PATH CFG_PATH };
const char DRIVER_PATH[] = { LEADING_PATH CFG_PATH "\\Drivers" };
const char VEHICLE_PATH[] = { LEADING_PATH CFG_PATH "\\Vehicles" };
const char TRACK_PATH[] = { LEADING_PATH CFG_PATH "\\Tracks" };
const char MY_SESSIONS_PATH[] = "MySessions";

//
// Global Memory allocations for working data structures
//
SYS_DATA_STRUCTURE sysData;
VEHICLE_DATA_STRUCTURE vehicleData;
DRIVER_DATA_STRUCTURE driverData;
TRACK_DATA_STRUCTURE trackData;
SENSOR_UNIT_STATUS_STRUCTURE suData;
UNSAVED_SYS_DATA_STRUCTURE unsavedSysData;
SECTOR_COMPARISON_TYPE sectors[NUMBER_OF_SECTORS];
#ifdef PROFILER
PROFILESTRUCT taskProfile[NUMBER_OF_TASKS] = {
	{ "DuIIMain", 0, 0, 0, 0},
	{ "SDCardRW", 0, 0, 0, 0},
	{ "Comm", 0, 0, 0, 0},
	{ "LapTimer", 0, 0, 0, 0},
	{ "TQMWrite", 0, 0, 0, 0},
	{ "LEDHandler", 0, 0, 0, 0},
	{ "SDCardMgr", 0, 0, 0, 0}
} ;
#endif

//
// Local Prototypes
//
int ReadSDCardData(void);
void SetDefaultAnalogInputData(int i);
void SetDefaultDigitalInputData(int i);
signed int FRWWriteFile(char *pNewData, char *fileName, int size);
char FRWReadFile(ALL_DATA_UNION *dataunion, char *filename, int size, enum DATATYPES type);
char UpgradeSysData00_01(void *pNewData);
char UpgradeSysData01_02(void *pNewData);
char UpgradeVehicleData00_01(void *pNewData);
void SetDefaultTempCalData(void);
void UpdateAnalogInputData(V0_VEHICLE_DATA_STRUCTURE *pNewData, int i);
void DisplayUpdatedVehicleDataPopup(void);

//
// Default vehicle definitions
// Sort these into alphabetical order
//
//   NOTE: FIRST ENTRY IS DEFAULT VEHICLE PATTERN
//

DEFAULT_VEHICLE_STRUCTURE defaultVehicles[] = {
	{	
		"350Z",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.794,		// gear1
			2.324,		// gear2
			1.624,		// gear3
			1.271,		// gear4
			1.000,		// gear5
			0.794,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.54,		// diff
			83.821,		// tire circumference
			245,		// tire width
			18,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3225,		// weight
			0.3,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"370Z",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.794,		// gear1
			2.324,		// gear2
			1.624,		// gear3
			1.271,		// gear4
			1.0,		// gear5
			0.794,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.692,		// diff
			80.098,		// tire circumference
			275,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3232,		// weight
			0.3,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"BMW 135i",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		6,
		0,
		7000,
		0,
		0,
		6500,
		7000,
		0,
		0,
		4.06,
		2.4,
		1.58,
		1.19,
		1,
		0.87,
		0,
		0,
		3.08,
		77.761,
		245,
		35,
		18,
		1,
		3200,
		0.37,
		0.5
		}
	},
	{
		"BMW 1M",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		6,
		0,
		7000,
		0,
		0,
		6200,
		7000,
		0,
		0,
		4.11,
		2.315,
		1.542,
		1.179,
		1.1,
		0.846,
		0,
		0,
		3.154,
		82.633,
		265,
		35,
		19,
		1,
		3300,
		0.37,
		0.5
		}
	},
	{	
		"BMW 2002",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6300,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.76,		// gear1
			2.02,		// gear2
			1.32,		// gear3
			1.0,		// gear4
			0.0,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.90,		// diff
			72.87,		// tire circumference
			185,		// tire width
			13,			// wheel size
			70,			// tire aspect
			1,			// user entered tire data
			2225,		// weight
			0.4,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BMW E30",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5800,		// tach Warning
			6200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.83,		// gear1
			2.2,		// gear2
			1.4,		// gear3
			1.0,		// gear4
			0.81,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.91,		// diff
			72.92,		// tire circumference
			195,		// tire width
			14,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2800,		// weight
			0.3,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BMW E30 M3",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.83,		// gear1
			2.2,		// gear2
			1.4,		// gear3
			1.0,		// gear4
			0.81,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.10,		// diff
			72.48,		// tire circumference
			205,		// tire width
			15,			// wheel size
			55,			// tire aspect
			1,			// user entered tire data
			2800,		// weight
			0.3,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BMW E36 325",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6200,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.23,		// gear1
			2.52,		// gear2
			1.66,		// gear3
			1.22,		// gear4
			1.0,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.15,		// diff
			78.156,		// tire circumference
			205,		// tire width
			16,			// wheel size
			55,			// tire aspect
			1,			// user entered tire data
			3200,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BMW E36 M3",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.2,		// gear1
			2.49,		// gear2
			1.66,		// gear3
			1.24,		// gear4
			1.0,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.23,		// diff
			77.651,		// tire circumference
			245,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3200,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"BMW E39 M5",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		8,
		0,
		7000,
		0,
		0,
		6500,
		7000,
		0,
		0,
		4.227,
		2.528,
		1.669,
		1.226,
		1,
		0.827,
		0,
		0,
		3.15,
		76.956,
		275,
		30,
		18,
		1,
		3859,
		0.29,
		0.5
		}
	},
	{	
		"BMW E46 325",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6200,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.23,		// gear1
			2.52,		// gear2
			1.66,		// gear3
			1.22,		// gear4
			1.0,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.15,		// diff
			78.452,		// tire circumference
			225,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3200,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BMW E46 M3",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			7900,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.23,		// gear1
			2.53,		// gear2
			1.67,		// gear3
			1.23,		// gear4
			1.1,		// gear5
			0.83,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.64,		// diff
			81.769,		// tire circumference
			255,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3415,		// weight
			0.32,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"BMW E60 M5",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		10,
		0,
		8000,
		0,
		0,
		7700,
		8000,
		0,
		0,
		3.99,
		2.65,
		1.81,
		1.39,
		1.16,
		1,
		0,
		0,
		3.62,
		84.92,
		255,
		40,
		19,
		1,
		4012,
		0.3,
		0.5
		}
	},
	{
		"BMW E90 325",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		6,
		0,
		7000,
		0,
		0,
		6500,
		7000,
		0,
		0,
		4.32,
		2.46,
		1.66,
		1.23,
		1,
		0.85,
		0,
		0,
		3.23,
		78.637,
		255,
		40,
		17,
		1,
		3400,
		0.31,
		0.5
		}
	},
	{	
		"BMW E92 M3",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			9000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			8200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.06,		// gear1
			2.4,		// gear2
			1.58,		// gear3
			1.19,		// gear4
			1.0,		// gear5
			0.87,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.85,		// diff
			82.633,		// tire circumference
			265,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3649,		// weight
			0.3,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"BMW F10 M5",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		8,
		0,
		6800,
		0,
		0,
		6000,
		6800,
		0,
		0,
		4.80,
		2.59,
		1.7,
		1.28,
		1,
		0.87,
		0,
		0,
		3.15,
		84.726,
		295,
		30,
		20,
		1,
		4387,
		0.33,
		0.5
		}
	},
	{
		"BMW Z4 3L",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		6,
		0,
		6500,
		0,
		0,
		6200,
		6500,
		0,
		0,
		4.35,
		2.5,
		1.66,
		1.23,
		1,
		0.85,
		0,
		0,
		3.07,
		78.637,
		255,
		40,
		17,
		1,
		3100,
		0.32,
		0.5
		}
	},
	{
		"BMW Z4 M",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		6,
		0,
		8000,
		0,
		0,
		7500,
		8000,
		0,
		0,
		4.35,
		2.53,
		1.66,
		1.23,
		1,
		0.85,
		0,
		0,
		3.62,
		81.779,
		255,
		40,
		18,
		1,
		3150,
		0.32,
		0.5
		}
	},
	{	
		"Boxster 01",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.5,		// gear1
			2.12,		// gear2
			1.43,		// gear3
			1.09,		// gear4
			0.84,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.56,		// diff
			78.094,		// tire circumference
			225,		// tire width
			16,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2778,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Boxster 03",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.5,		// gear1
			2.12,		// gear2
			1.43,		// gear3
			1.09,		// gear4
			0.84,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.56,		// diff
			78.637,		// tire circumference
			255,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			2811,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Boxster 04",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.67,		// gear1
			2.05,		// gear2
			1.41,		// gear3
			1.13,		// gear4
			0.97,		// gear5
			0.82,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.87,		// diff
			78.637,		// tire circumference
			255,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			2855,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BoxsterS 00",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.2,		// gear2
			1.52,		// gear3
			1.22,		// gear4
			1.02,		// gear5
			0.84,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.44,		// diff
			79.492,		// tire circumference
			265,		// tire width
			18,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			2855,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BoxsterS 03",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.2,		// gear2
			1.52,		// gear3
			1.22,		// gear4
			1.02,		// gear5
			0.84,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.44,		// diff
			79.492,		// tire circumference
			265,		// tire width
			18,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			2910,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BoxsterS 04",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7500,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6800,		// tach Warning
			7200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.67,		// gear1
			2.05,		// gear2
			1.41,		// gear3
			1.13,		// gear4
			0.97,		// gear5
			0.82,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.87,		// diff
			79.492,		// tire circumference
			265,		// tire width
			18,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			2965,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BoxsterS DSG 14",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.91,		// gear1
			2.29,		// gear2
			1.65,		// gear3
			1.3,		// gear4
			1.08,		// gear5
			0.88,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.25,		// diff
			85.775,		// tire circumference
			265,		// tire width
			20,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3000,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"BR-Z",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7400,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.626,		// gear1
			2.188,		// gear2
			1.541,		// gear3
			1.213,		// gear4
			1.0,		// gear5
			0.767,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			77.34,		// tire circumference
			215,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2745,		// weight
			0.27,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"CaymanS 04",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6800,		// tach Warning
			7200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.67,		// gear1
			2.05,		// gear2
			1.41,		// gear3
			1.13,		// gear4
			0.97,		// gear5
			0.82,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.87,		// diff
			79.492,		// tire circumference
			265,		// tire width
			18,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			2995,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"CaymanS DSG 14",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.91,		// gear1
			2.29,		// gear2
			1.65,		// gear3
			1.3,		// gear4
			1.08,		// gear5
			0.88,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.25,		// diff
			85.775,		// tire circumference
			265,		// tire width
			20,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3000,		// weight
			0.29,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Civic 92-95",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.25,		// gear1
			1.761,		// gear2
			1.172,		// gear3
			0.909,		// gear4
			0.702,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.058,		// diff
			71.44,		// tire circumference
			185,		// tire width
			14,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2200,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Elise",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			8000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.17,		// gear1
			2.05,		// gear2
			1.48,		// gear3
			1.17,		// gear4
			0.92,		// gear5
			0.81,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.53,		// diff
			78.453,		// tire circumference
			225,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			1975,		// weight
			0.4,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"EVO 04",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.928,		// gear1
			1.95,		// gear2
			1.407,		// gear3
			1.031,		// gear4
			0.72,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.416,		// diff
			79.567,		// tire circumference
			235,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3100,		// weight
			0.35,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"EVO MR 05",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.909,		// gear1
			1.944,		// gear2
			1.434,		// gear3
			1.1,		// gear4
			0.868,		// gear5
			0.693,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.58,		// diff
			79.567,		// tire circumference
			235,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3285,		// weight
			0.35,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"EVO TWIN 08",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.655,		// gear1
			2.368,		// gear2
			1.754,		// gear3
			1.322,		// gear4
			1.008,		// gear5
			0.775,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.011,		// diff
			80.792,		// tire circumference
			245,		// tire width
			18,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3500,		// weight
			0.35,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Evora",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6200,		// tach Warning
			6800,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.538,		// gear1
			1.913,		// gear2
			1.407,		// gear3
			1.091,		// gear4
			0.96,		// gear5
			0.86,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.77,		// diff
			81.769,		// tire circumference
			255,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3000,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Exige",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			8000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.17,		// gear1
			2.05,		// gear2
			1.48,		// gear3
			1.17,		// gear4
			0.92,		// gear5
			0.81,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.53,		// diff
			78.453,		// tire circumference
			255,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2057,		// weight
			0.45,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Ferrari 360",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			7900,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.29,		// gear1
			2.16,		// gear2
			1.61,		// gear3
			1.27,		// gear4
			1.03,		// gear5
			0.85,			// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.44,		// diff
			80.839,		// tire circumference
			285,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3064,		// weight
			0.34,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Ferrari 430",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			8400,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7800,		// tach Warning
			8200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.29,		// gear1
			2.16,		// gear2
			1.61,		// gear3
			1.27,		// gear4
			1.03,		// gear5
			0.82,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.3,		// diff
			80.839,		// tire circumference
			285,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3197,		// weight
			0.34,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"Ferrari 458",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		8,
		0,
		9000,
		0,
		0,
		8500,
		9000,
		0,
		0,
		3.08,
		2.19,
		1.63,
		1.29,
		1.03,
		0.84,
		0,
		0,
		5.14,
		88.373,
		295,
		35,
		20,
		1,
		3200,
		0.33,
		0.5
		}
	},
	{	
		"Fiat 500 Abarth",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.909,		// gear1
			2.238,		// gear2
			1.52,		// gear3
			1.156,		// gear4
			0.847,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.353,		// diff
			76.23,		// tire circumference
			205,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			2500,		// weight
			0.325,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"FR-S",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7400,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.626,		// gear1
			2.188,		// gear2
			1.541,		// gear3
			1.213,		// gear4
			1.0,		// gear5
			0.767,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			77.34,		// tire circumference
			215,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2745,		// weight
			0.27,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Genesis Turbo",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5800,		// tach Warning
			6000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.17,		// gear1
			2.47,		// gear2
			1.52,		// gear3
			1.23,		// gear4
			0.87,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.727,		// diff
			83.934,		// tire circumference
			245,		// tire width
			19,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3350,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Genesis V6",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5500,		// tach Warning
			6200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.17,		// gear1
			2.47,		// gear2
			1.52,		// gear3
			1.23,		// gear4
			0.87,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.73,		// diff
			83.934,		// tire circumference
			245,		// tire width
			19,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3550,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"GT3 996 03",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7400,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.15,		// gear2
			1.56,		// gear3
			1.21,		// gear4
			1.1,		// gear5
			0.85,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.44,		// diff
			78.442,		// tire circumference
			295,		// tire width
			18,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3042,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"GT3 996 99",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7400,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.15,		// gear2
			1.56,		// gear3
			1.21,		// gear4
			0.97,		// gear5
			0.82,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.44,		// diff
			77.698,		// tire circumference
			285,		// tire width
			18,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			2976,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"GT3 997 06",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			9000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			8000,		// tach Warning
			8400,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.26,		// gear2
			1.64,		// gear3
			1.29,		// gear4
			1.06,		// gear5
			0.92,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.44,		// diff
			78.442,		// tire circumference
			295,		// tire width
			18,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3075,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"GT3 RS 996 03",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7400,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.15,		// gear2
			1.56,		// gear3
			1.21,		// gear4
			1.1,		// gear5
			0.85,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.44,		// diff
			82.325,		// tire circumference
			305,		// tire width
			19,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			2998,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"GT3 RS 997 10",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			9000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			8000,		// tach Warning
			8500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.82,		// gear1
			2.26,		// gear2
			1.64,		// gear3
			1.29,		// gear4
			1.06,		// gear5
			0.88,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.89,		// diff
			83.808,		// tire circumference
			325,		// tire width
			19,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3023,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"GTR",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6800,		// tach Warning
			7200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			4.056,		// gear1
			2.301,		// gear2
			1.595,		// gear3
			1.248,		// gear4
			1.001,		// gear5
			0.796,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.7,		// diff
			83.981,		// tire circumference
			285,		// tire width
			20,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3800,		// weight
			0.26,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Integra GSR 94",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			7900,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.23,		// gear1
			1.9,		// gear2
			1.36,		// gear3
			1.034,		// gear4
			0.787,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.4,		// diff
			72.926,		// tire circumference
			195,		// tire width
			14,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2643,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Integra LS 02",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5700,		// tach Warning
			6000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.166,		// gear1
			1.857,		// gear2
			1.259,		// gear3
			0.935,		// gear4
			0.742,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.4,		// diff
			72.926,		// tire circumference
			195,		// tire width
			14,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2560,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Integra R 99",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			7900,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.23,		// gear1
			2.105,		// gear2
			1.458,		// gear3
			1.107,		// gear4
			0.848,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.4,		// diff
			71.242,		// tire circumference
			195,		// tire width
			15,			// wheel size
			55,			// tire aspect
			1,			// user entered tire data
			2639,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Miata 90-93",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.136,		// gear1
			1.888,		// gear2
			1.33,		// gear3
			1,			// gear4
			0.814,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.3,		// diff
			71.44,		// tire circumference
			185,		// tire width
			14,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2300,		// weight
			0.38,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Miata 94-97",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.136,		// gear1
			1.888,		// gear2
			1.33,		// gear3
			1,			// gear4
			0.814,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			71.44,		// tire circumference
			185,		// tire width
			14,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2350,		// weight
			0.38,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Miata 99-05 5spd",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.136,		// gear1
			1.888,		// gear2
			1.33,		// gear3
			1,			// gear4
			0.814,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.3,		// diff
			73.086,		// tire circumference
			205,		// tire width
			16,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2400,		// weight
			0.37,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Miata 99-05 6spd",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.76,		// gear1
			2.269,		// gear2
			1.645,		// gear3
			1.257,		// gear4
			1.0,		// gear5
			0.843,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.9,		// diff
			73.086,		// tire circumference
			205,		// tire width
			16,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2350,		// weight
			0.37,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Miata MX5 06+",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.815,		// gear1
			2.26,		// gear2
			1.64,		// gear3
			1.177,		// gear4
			1.1,		// gear5
			0.832,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			76.228,		// tire circumference
			205,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2498,		// weight
			0.32,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Mini CooperS 03",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed1
			3.308,		// gear1
			2.130,		// gear2
			1.483,		// gear3
			1.139,		// gear4
			0.949,		// gear5
			0.816,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.65,		// diff
			76.23,		// tire circumference
			205,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			2678,		// weight
			0.35,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"MR2 Spyder 00",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6700,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.166,		// gear1
			1.9,		// gear2
			1.39,		// gear3
			1.031,		// gear4
			0.815,		// gear5
			0.725,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.529,		// diff
			74.952,		// tire circumference
			205,		// tire width
			15,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2195,		// weight
			0.35,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"MR2 Turbo 94",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5800,		// tach Warning
			6200,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.23,		// gear1
			1.91,		// gear2
			1.32,		// gear3
			0.92,		// gear4
			0.73,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.29,		// diff
			74.952,		// tire circumference
			225,		// tire width
			15,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2700,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Mustang GT 01",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5200,		// tach Warning
			5700,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.37,		// gear1
			1.99,		// gear2
			1.33,		// gear3
			1.1,		// gear4
			0.67,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.27,		// diff
			80.679,		// tire circumference
			245,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3274,		// weight
			0.36,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Mustang GT 11",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			6800,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.66,		// gear1
			2.43,		// gear2
			1.69,		// gear3
			1.32,		// gear4
			1.0,		// gear5
			0.65,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.73,		// diff
			85.615,		// tire circumference
			235,		// tire width
			18,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			3600,		// weight
			0.38,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"NSX 02",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7600,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.07,		// gear1
			1.96,		// gear2
			1.43,		// gear3
			1.13,		// gear4
			0.91,		// gear5
			0.72,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.06,		// diff
			78.637,		// tire circumference
			255,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3153,		// weight
			0.3,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"NSX 94",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7600,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.07,		// gear1
			1.73,		// gear2
			1.23,		// gear3
			0.97,		// gear4
			0.77,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.06,		// diff
			77.651,		// tire circumference
			245,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3100,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"RSX",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7000,		// tach Warning
			7700,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.267,		// gear1
			2.13,		// gear2
			1.517,		// gear3
			1.147,		// gear4
			0.921,		// gear5
			0.738,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.389,		// diff
			77.55,		// tire circumference
			205,		// tire width
			15,			// wheel size
			60,			// tire aspect
			1,			// user entered tire data
			2778,		// weight
			0.32,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"RX7 78-85 4spd",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		2,
		0,
		7500,
		0,
		0,
		7000,
		8000,
		0,
		0,
		3.674,
		2.217,
		1.432,
		1,
		0,
		0,
		0,
		0,
		3.909,
		72.87,
		185,
		70,
		13,
		1,
		2350,
		0.31,
		0.5
		}
	},
	{
		"RX7 78-85 5spd",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		2,
		0,
		7500,
		0,
		0,
		7000,
		8000,
		0,
		0,
		3.674,
		2.217,
		1.432,
		1,
		0.825,
		0,
		0,
		0,
		3.909,
		72.87,
		185,
		70,
		13,
		1,
		2385,
		0.31,
		0.5
		}
	},
	{
		"RX7 84-85 GSL-SE",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		2,
		0,
		7500,
		0,
		0,
		7000,
		8000,
		0,
		0,
		3.622,
		2.186,
		1.419,
		1,
		0.758,
		0,
		0,
		0,
		3.909,
		72.87,
		185,
		70,
		13,
		1,
		2590,
		0.31,
		0.5
		}
	},
	{
		"RX7 FC 86-91",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		2,
		0,
		7500,
		0,
		0,
		7000,
		7500,
		0,
		0,
		3.483,
		2.02,
		1.391,
		1,
		0.76,
		0,
		0,
		0,
		4.1,
		75.62,
		205,
		50,
		16,
		1,
		2300,
		0.31,
		0.5
		}
	},
	{
		"RX7 FD 91-02",
		{
			MOBIUS_DELAY,			// usbCameraDelay
		2,
		0,
		7800,
		0,
		0,
		7000,
		7800,
		0,
		0,
		3.483,
		2.015,
		1.391,
		1.00,
		0.719,
		0,
		0,
		0,
		4.100,
		78.094,
		225,
		50,
		16,
		1,
		2800,
		0.31,
		0.5
		}
	},
	{	
		"RX8",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			2,			// engineCylinders
			0,			// tach scaleStart
			9000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			8000,		// tach Warning
			8700,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.76,		// gear1
			2.269,		// gear2
			1.645,		// gear3
			1.187,		// gear4
			1.0,		// gear5
			0.843,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.4,		// diff
			81.593,		// tire circumference
			225,		// tire width
			18,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3060,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"S2000 00-03",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			9000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			8500,		// tach Warning
			8900,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.13,		// gear1
			2.05,		// gear2
			1.48,		// gear3
			1.16,		// gear4
			0.97,		// gear5
			0.81,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			78.094,		// tire circumference
			225,		// tire width
			16,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2809,		// weight
			0.31,		// wind dragS2000
			0.5			// rolling resistance
		}
	},
	{	
		"S2000 04-09",
		{		// MINI_VEHICLE_STRUCTURE1
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			8000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			7500,		// tach Warning
			7900,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.13,		// gear1
			2.05,		// gear2
			1.48,		// gear3
			1.16,		// gear4
			0.943,		// gear5
			0.763,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			78.452,		// tire circumference
			225,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			2809,		// weight
			0.31,		// wind drag
			0.5			// rolling resistance
		}
	},
	{
		"Spec Miata 90-93",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.136,		// gear1
			1.888,		// gear2
			1.33,		// gear3
			1,			// gear4
			0.814,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.3,		// diff
			72.48,		// tire circumference
			205,		// tire width
			15,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2300,		// weight
			0.38,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Spec Miata 94-97",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.136,		// gear1
			1.888,		// gear2
			1.33,		// gear3
			1,			// gear4
			0.814,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.1,		// diff
			72.48,		// tire circumference
			205,		// tire width
			15,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2350,		// weight
			0.38,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Spec Miata 99-05",				// DEFAULT VEHICLE -- Do not change name!!!
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7300,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.136,		// gear1
			1.888,		// gear2
			1.33,		// gear3
			1,			// gear4
			0.814,		// gear5
			0.0,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			4.3,		// diff
			72.48,		// tire circumference
			205,		// tire width
			15,			// wheel size
			50,			// tire aspect
			1,			// user entered tire data
			2400,		// weight
			0.37,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Supra 98",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			6,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6200,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.83,		// gear1
			2.36,		// gear2
			1.69,		// gear3
			1.31,		// gear4
			1.1,		// gear5
			0.79,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.27,		// diff
			78.637,		// tire circumference
			255,		// tire width
			17,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3450,		// weight
			0.32,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Viper 05",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			10,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5500,		// tach Warning
			6000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.66,		// gear1
			1.78,		// gear2
			1.3,		// gear3
			1.1,		// gear4
			0.74,		// gear5
			0.5,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.07,		// diff
			85.294,		// tire circumference
			345,		// tire width
			19,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3380,		// weight
			0.35,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Viper 12",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			10,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			5500,		// tach Warning
			6000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.26,		// gear1
			1.58,		// gear2
			1.19,		// gear3
			1,			// gear4
			0.77,		// gear5
			0.63,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.55,		// diff
			85.294,		// tire circumference
			345,		// tire width
			19,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3297,		// weight
			0.36,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"WRX STI 04",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6200,		// tach Warning
			6800,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.636,		// gear1
			2.375,		// gear2
			1.761,		// gear3
			1.346,		// gear4
			0.971,		// gear5
			0.756,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.9,		// diff
			78.452,		// tire circumference
			225,		// tire width
			17,			// wheel size
			45,			// tire aspect
			1,			// user entered tire data
			3263,		// weight
			0.33,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"WRX STI 08",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6300,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.636,		// gear1
			2.235,		// gear2
			1.521,		// gear3
			1.137,		// gear4
			0.971,		// gear5
			0.756,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.9,		// diff
			80.792,		// tire circumference
			245,		// tire width
			18,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3395,		// weight
			0.36,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"WRX STI 11",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			4,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6300,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			3.636,		// gear1
			2.235,		// gear2
			1.521,		// gear3
			1.137,		// gear4
			0.971,		// gear5
			0.756,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.54,		// diff
			80.792,		// tire circumference
			245,		// tire width
			18,			// wheel size
			40,			// tire aspect
			1,			// user entered tire data
			3373,		// weight
			0.36,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Z06 C5",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6000,		// tach Warning
			6500,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.66,		// gear1
			1.78,		// gear2
			1.3,		// gear3
			1,			// gear4
			0.74,		// gear5
			0.5,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.42,		// diff
			81.584,		// tire circumference
			295,		// tire width
			19,			// wheel size
			35,			// tire aspect
			1,			// user entered tire data
			3115,		// weight
			0.34,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"Z06 C6",
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.97,		// gear1
			2.07,		// gear2
			1.43,		// gear3
			1,			// gear4
			0.84,		// gear5
			0.56,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.42,		// diff
			83.808,		// tire circumference
			325,		// tire width
			19,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3180,		// weight
			0.34,		// wind drag
			0.5			// rolling resistance
		}
	},
	{	
		"",		// NULL VEHICLE MARKS END OF LIST
		{		// MINI_VEHICLE_STRUCTURE
			MOBIUS_DELAY,			// usbCameraDelay
			8,			// engineCylinders
			0,			// tach scaleStart
			7000,		// tach scaleEnd
			0,			// tach greenStart
			0,			// tach greenEnd
			6500,		// tach Warning
			7000,		// tach Redline
			0,			// tach lowerYellow
			0,			// tach lowerRed
			2.97,		// gear1
			2.07,		// gear2
			1.43,		// gear3
			1,			// gear4
			0.84,		// gear5
			0.56,		// gear6
			0.0,		// gear7
			0.0,		// gear8
			3.42,		// diff
			83.808,		// tire circumference
			325,		// tire width
			19,			// wheel size
			30,			// tire aspect
			1,			// user entered tire data
			3180,		// weight
			0.34,		// wind drag
			0.5			// rolling resistance
		}
	}
}; // defaultVehicles

//
// Read the user data out of memory
//
// Attempt to read the data off the SD card.  If that read fails then
// use the data stored in serial flash.  If that fails load the structures
// with default data.
//
// Return values:
//		0 = user values read from SD card
//		Not equal 0 = Error code (bit mask)
//
int ReadDataStructures(void)
{
	return ReadSDCardData();
}

int ReadSDCardData(void)
{
	int retval;

	retval = 0;
// Read system data
	if (ReloadData(SYSTEM_DATA) == false)
		retval = INVALID_SYSTEM_DATA;

// Read vehicle data
	if (ReloadData(VEHICLE_DATA) == false)
		retval |= INVALID_VEHICLE_DATA;

// See if an old camera type is selected (in VehicleSettingsScreens.c), reset and tell user if necessary
	ValidateCameraData();

// Update splash screen with info
	DisplaySplashScreen3();

// Read track data
	if (ReloadData(TRACK_DATA) == false)
		retval |= INVALID_TRACK_DATA;

// Read driver data
	if (ReloadData(DRIVER_DATA) == false)
		retval |= INVALID_DRIVER_DATA;

#ifndef _WINDOWS
	FS_Sync("");		// Flush cache to be sure all files written to disk
#endif

// return the results of the load
	return retval;
}

//
// Verify that this data is valid for the appropriate file type.
//
// Input:	type of file
//			pointer to data to be verified
//			flag telling to copy data to primary structures if true
//
//  Note that if data type is system or vehicle will check to see if this file
//  is an older version.  If so it will upgrade to the newer format.
//
char verifyData(enum DATATYPES data, void *pNewData, int copyData)
{
	const char *pVersion;
	int originalChecksum;
	long size;

	switch (data)
	{
	case SYSTEM_DATA:
		pVersion = SYSTEM_VERSION;
		size = sizeof(SYS_DATA_STRUCTURE);
		break;
	case TRACK_DATA:
		pVersion = TRACK_VERSION;
		size = sizeof(TRACK_DATA_STRUCTURE);
		break;
	case VEHICLE_DATA:
		pVersion = VEHICLE_VERSION;
		size = sizeof(VEHICLE_DATA_STRUCTURE);
		break;
	case DRIVER_DATA:
		pVersion = DRIVER_VERSION;
		size = sizeof(DRIVER_DATA_STRUCTURE);
		break;
	default:
		return false;
	}
	
	// checksum is always last 4 bytes in structure stored as int
	originalChecksum = *((int *) ((unsigned char *) pNewData + size - sizeof(int)));

	if (!strcmp(pNewData, pVersion))			// Version string always the first element in the structure
	{
		int checksum;
		int i;
		unsigned char *pData;

		// Verify the checksum of the data read
		checksum = 0;
		pData = pNewData;
		for (i = size - sizeof(int); i > 0; i--)
			checksum += *pData++;

		if (originalChecksum == checksum)
		{	// data good so copy into globals if requested
			if (copyData)
			{
				// This is what it should have been doing all along
				// If requested by the caller copy data into the primary data structures
				// otherwise just validate the data and return.
				switch (data)
				{
				case SYSTEM_DATA:
					memcpy(&sysData, pNewData, size);
					break;
				case TRACK_DATA:
					memcpy(&trackData, pNewData, size);
					break;
				case VEHICLE_DATA:
					memcpy(&vehicleData, pNewData, size);
					break;
				case DRIVER_DATA:
					memcpy(&driverData, pNewData, size);
					break;
				default:
					return false;
				}
			}
			
			return true;
		}
	}
	else
	{
		// Version numbers do not match - see if a valid older version
		if (data == SYSTEM_DATA)
		{
			if (!(strcmp(pNewData, "00.00")))
				if (UpgradeSysData00_01(pNewData) == false)
					return false;
				// if return true fall through to the upgrade from 01 to 02

			if (!(strcmp(pNewData, "00.01")))
			{
				if (UpgradeSysData01_02(pNewData) == false)
					return false;
				else
				{
					// Updated system data - be sure the new image is written to disk
					WriteUserDataEvent(SYSTEM_DATA);
					return true;
				}
			}
		}
		else if (data == VEHICLE_DATA)
		{
			if (!(strcmp(pNewData, "00.00")))
			{
				if (UpgradeVehicleData00_01(pNewData) == false)
					return false;
				else
				{
					// Updated vehicle data - be sure the new image is written to disk
					WriteUserDataEvent(VEHICLE_DATA);
					DisplayUpdatedVehicleDataPopup();
					return true;
				}
			}
		}
		else return false;
	}
	return false;
}

//
// Check to see if the NewData image is a valid previous version of sysData
// If so, upgrade to the next version and return true
// otherwise return false (bad data)
//
// Note: this routine only upgrades from version 00.00 to 00.01
//
char UpgradeSysData00_01(void *pNewData)
{
	int checksum;
	int i;
	char *pData;
	int originalChecksum;

	// checksum is always last 4 bytes in structure stored as int
	originalChecksum = *((int *) ((unsigned char *) pNewData + SIZE_OF_VER_00_00_SYS_DATA_STRUCTURE - sizeof(int)));

	// Verify the checksum of the data read
	checksum = 0;
	pData = pNewData;
	for (i = SIZE_OF_VER_00_00_SYS_DATA_STRUCTURE - sizeof(int); i > 0; i--)
		checksum += *pData++;

	if (originalChecksum == checksum)
	{
		// data good so copy into globals
	//	sysData = (* (ALL_DATA_UNION *) pNewData).sysdata;
		memcpy((void *)&sysData, (void const *)pNewData, SIZE_OF_VER_00_00_SYS_DATA_STRUCTURE);
		

		// Initialize new version 1 data variables
		sysData.dragDistance = DD_1_4;			// Last seleted drag race distance = 1/4 mile
		sysData.disableDriveRecordMode = 0;		// Record mode during DRIVE MODE
		sysData.checksum = 0;
		strcpy(sysData.version, "00.01");		// Mark data as valid version 01

		// Calculate new checksum for a VERSION 1 data structure!
		pData = (char *)&sysData;
		checksum = 0;
		for (i = SIZE_OF_VER_00_01_SYS_DATA_STRUCTURE - sizeof(int); i > 0; i--)
			checksum += *pData++;
		// Write new checksum
		// checksum is always last 4 bytes in structure stored as int
		*(int *)pData = checksum;

		// Copy sysData back to pNewData for possible follow-up tests
		memcpy(pNewData, (void const *)&sysData, SIZE_OF_VER_00_01_SYS_DATA_STRUCTURE);
		
		return true;
	}
	else return false;
}

//
// Check to see if the NewData image is a valid previous version of sysData
// If so, upgrade to the next version and return true
// otherwise return false (bad data)
//
// Note: this routine only upgrades from version 00.01 to 00.02
//
char UpgradeSysData01_02(void *pNewData)
{
	int checksum;
	int i;
	char *pData;
	int originalChecksum;

	// checksum is always last 4 bytes in structure stored as int
	originalChecksum = *((int *) ((unsigned char *) pNewData + SIZE_OF_VER_00_01_SYS_DATA_STRUCTURE - sizeof(int)));

	// Verify the checksum of the data read
	checksum = 0;
	pData = pNewData;
	for (i = SIZE_OF_VER_00_01_SYS_DATA_STRUCTURE - sizeof(int); i > 0; i--)
		checksum += *pData++;

	if (originalChecksum == checksum)
	{
		// data good so copy into globals
	//	sysData = (* (ALL_DATA_UNION *) pNewData).sysdata;
		memcpy((void *)&sysData, (void const *)pNewData, SIZE_OF_VER_00_01_SYS_DATA_STRUCTURE);

		// Initialize new version 2 data variables
		SetDefaultTempCalData();
		sysData.checksum = 0;
		strcpy(sysData.version, "00.02");		// Mark data as valid version 02

		// Calculate new checksum for a VERSION 2 data structure!
		pData = (char *)&sysData;
		checksum = 0;
		for (i = sizeof(SYS_DATA_STRUCTURE) - sizeof(int); i > 0; i--)	// SYS_DATA_STRUCTURE = size of CURRENT sysData structure
			checksum += *pData++;
		// Write new checksum
		// checksum is always last 4 bytes in structure stored as int
		*(int *)pData = checksum;

		// Copy sysData back to pNewData for possible follow-up tests
		memcpy(pNewData, (void const *)&sysData, sizeof(SYS_DATA_STRUCTURE));
		
		return true;
	}
	else return false;
}


//
// Check to see if the NewData image is a valid previous version of Vehicle Data.
// If so, upgrade to the next version and return true
// otherwise return false (bad data)
//
// Note: this routine only upgrades from version 00.00 to 00.01
//
char UpgradeVehicleData00_01(void *pNewData)
{
	int checksum;
	int i;
	char *pData;
	int originalChecksum;

	// Index into the data structure using the previous template to get the checksum word
	originalChecksum = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->checksum;

	// Verify the checksum of the data read
	checksum = 0;
	pData = pNewData;
	for (i = sizeof(V0_VEHICLE_DATA_STRUCTURE) - sizeof(int); i > 0; i--)
		checksum += *pData++;

	if (originalChecksum == checksum)
	{
		// data good so copy into globals
		memset((void *)&vehicleData, 0, sizeof(VEHICLE_DATA_STRUCTURE));
		// Copy everything from the old buffer up to the start of the vehicle data structures into the new buffer
		memcpy(&vehicleData.version, VEHICLE_VERSION, sizeof(VEHICLE_VERSION));
		vehicleData.usbCameraDelay = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->usbCameraDelay;
		vehicleData.engineCylinders = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->engineCylinders;
		vehicleData.tach = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->tach;
		vehicleData.gearRatios[0] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[0];
		vehicleData.gearRatios[1] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[1];
		vehicleData.gearRatios[2] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[2];
		vehicleData.gearRatios[3] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[3];
		vehicleData.gearRatios[4] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[4];
		vehicleData.gearRatios[5] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[5];
		vehicleData.gearRatios[6] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[6];
		vehicleData.gearRatios[7] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->gearRatios[7];
		vehicleData.diffRatio = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->diffRatio;
		vehicleData.tireCircumference = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->tireCircumference;
		vehicleData.treadWidth = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->treadWidth;
		vehicleData.wheelDiameter = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->wheelDiameter;
		vehicleData.aspectRatio = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->aspectRatio;
		vehicleData.userEnteredTireDimensions = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->userEnteredTireDimensions;
		vehicleData.weight = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->weight;
		vehicleData.windDrag = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->windDrag;
		vehicleData.rollingResistance = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->rollingResistance;
		vehicleData.camera = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->camera;
		for (i = 0; i < 4; i++)
			UpdateAnalogInputData(pNewData, i);
		vehicleData.digitalInput[0] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->digitalInput[0];
		vehicleData.digitalInput[1] = ((V0_VEHICLE_DATA_STRUCTURE *)(pNewData))->digitalInput[1];

		// The following is not required if no follow-on update to the next version is
		// required as the checksum will be calculated when the file is written to disk.
		////// Calculate new checksum for a VERSION 1 data structure!
		////pData = (char *)&vehicleData;
		////checksum = 0;
		////for (i = SIZE_OF_VER_00_01_VEHICLE_DATA_STRUCTURE - sizeof(int); i > 0; i--)
		////	checksum += *pData++;
		////// Write new checksum
		////// checksum is always last 4 bytes in structure stored as int
		////*(int *)pData = checksum;

		////// Copy sysData back to pNewData for possible follow-up tests
		////memcpy(pNewData, (void const *)&vehicleData, SIZE_OF_VER_00_01_VEHICLE_DATA_STRUCTURE);
		
		return true;
	}
	else return false;
}

void UpdateAnalogInputData(V0_VEHICLE_DATA_STRUCTURE *pNewData, int i)
{
	memcpy((void *)&vehicleData.analogInput[i].Name, (void *)pNewData->analogInput[i].Name, NAMELEN + 1);
	memcpy((void *)&vehicleData.analogInput[i].Units, (void *)pNewData->analogInput[i].Units, MAX_UNITS);
	vehicleData.analogInput[i].enabled = pNewData->analogInput[i].enabled;
	if (pNewData->analogInput[i].alarmTriggerLevel == BELOW)
	{
		vehicleData.analogInput[i].lowerAlarmTriggerLevel = pNewData->analogInput[i].alarmTriggerLevel;
		vehicleData.analogInput[i].lowerAlarmTriggerComparison = pNewData->analogInput[i].alarmTriggerComparison;
		vehicleData.analogInput[i].upperAlarmTriggerComparison = OFF;
		vehicleData.analogInput[i].upperAlarmTriggerLevel = 0.0;
	}
	else if ((pNewData->analogInput[i].alarmTriggerComparison == ABOVE) || (pNewData->analogInput[i].alarmTriggerComparison == EQUAL_TO))
	{
		vehicleData.analogInput[i].upperAlarmTriggerLevel = pNewData->analogInput[i].alarmTriggerLevel;
		vehicleData.analogInput[i].upperAlarmTriggerComparison = pNewData->analogInput[i].alarmTriggerComparison;
		vehicleData.analogInput[i].lowerAlarmTriggerComparison = OFF;
		vehicleData.analogInput[i].lowerAlarmTriggerLevel = 0.0;
	}
	else
	{
		vehicleData.analogInput[i].upperAlarmTriggerLevel = 0.0;
		vehicleData.analogInput[i].upperAlarmTriggerComparison = OFF;
		vehicleData.analogInput[i].lowerAlarmTriggerComparison = OFF;
		vehicleData.analogInput[i].lowerAlarmTriggerLevel = 0.0;
	}
	vehicleData.analogInput[i].inputRange = pNewData->analogInput[i].inputRange;
	vehicleData.analogInput[i].lowVoltagePoint = pNewData->analogInput[i].lowVoltagePoint;
	vehicleData.analogInput[i].highVoltagePoint = pNewData->analogInput[i].highVoltagePoint;
	vehicleData.analogInput[i].lowValuePoint = pNewData->analogInput[i].lowValuePoint;
	vehicleData.analogInput[i].highValuePoint = pNewData->analogInput[i].highValuePoint;
	vehicleData.analogInput[i].EnableLED1 = pNewData->analogInput[i].EnableLED1;
	vehicleData.analogInput[i].EnableLED2 = pNewData->analogInput[i].EnableLED2;
}

void SetDefaultTempCalData(void)
{
	sysData.calibrationTemperature = ROOM_TEMPERATURE;		// Default calibration temp assumed to be 24C = 75F, units always stored in metric
#ifdef _WINDOWS
	sysData.calibrationTempReading = ROOM_TEMP_READING;		// Simulate current board reading and set temp correction offset
#else
	sysData.calibrationTempReading = GetADC(ADC_LED_BOARD_TEMP);	// Take current board reading and set temp correction offset
#endif
	sysData.LEDtempOffset = CalculateTempCorrection();
}


//
// Write the selected data structure to permanent storage
//
// Write the selected data structure to the SD card and to internal serial flash
//
// Input: SYSTEM_DATA, TRACK_DATA, VEHICLE_DATA, DRIVER_DATA
// Return values:
//		0 = user values written to both SD card and serial data flash
//		-1 = failed one of the writes
//		-2 = failed both of the writes
//
int WriteDataStructures(enum DATATYPES data)
{
#ifdef	_WINDOWS
	WriteUserDataEvent(data);
#else
	if (data == TRACK_DATA)
		OS_PutMail(&MB_SDcardRW, &data);
	else WriteUserDataEvent(data);
#endif
	return 0;
}


int WriteUserDataEvent(enum DATATYPES data)
{
int size;
int checksum;
int checksumSave;
int i;
char *pData, *pNewData;
char fileName[256];

	switch (data)
	{		
	case SYSTEM_DATA:
		pNewData = (char *)&sysData;
		size = sizeof(SYS_DATA_STRUCTURE);
		ConvertToFilename(fileName, (char *)SYSTEM_FILE_NAME, SYSTEM_DATA);
		break;
	case TRACK_DATA:
		pNewData = (char *)&trackData;
		size = sizeof(TRACK_DATA_STRUCTURE);
		ConvertToFilename(fileName, sysData.track, TRACK_DATA);
		break;
	case VEHICLE_DATA:
		pNewData = (char *)&vehicleData;
		size = sizeof(VEHICLE_DATA_STRUCTURE);
		ConvertToFilename(fileName, sysData.vehicle, VEHICLE_DATA);
		break;
	case DRIVER_DATA:
		pNewData = (char *)&driverData;
		size = sizeof(DRIVER_DATA_STRUCTURE);
		ConvertToFilename(fileName, sysData.driver, DRIVER_DATA);
		break;
	default:
		return -1;
	}

	// Calculate the checksum before writing
	checksum = 0;
	pData = pNewData;
	for (i = size -  sizeof(int); i > 0; i--)
		checksum += *pData++;
	*(int *)pData = checksumSave = checksum;

	if (data == SYSTEM_DATA)
	{
		signed int retval;
		char backupFileName[256];

		retval = FRWWriteFile(pNewData, fileName, size);	// Write primary file
		ConvertToFilename(backupFileName, (char *)BACKUP_SYSTEM_FILE_NAME, SYSTEM_DATA);
		FRWWriteFile(pNewData, backupFileName, size);				// Write backup file
		
		// Verify write!
		// Calculate the checksum before writing
		checksum = 0;
		pData = pNewData;
		for (i = size -  sizeof(int); i > 0; i--)
			checksum += *pData++;
		if (checksumSave != checksum)
		{
			// Someone overwrote the data structure while we were writing it,
			// reset the new checksum and try one more time to save the data
			// Note that this should never happen, but since I created the test
			// I might as well add the safety measure.
			*(int *)pData = checksum;
			retval = FRWWriteFile(pNewData, fileName, size);		// Write primary file
			FRWWriteFile(pNewData, backupFileName, size);			// Write backup file
		}
		
		return retval;
	}
	else return FRWWriteFile(pNewData, fileName, size);
}

signed int FRWWriteFile(char *pNewData, char *fileName, int size)
{
#ifdef _WINDOWS
FILE *pFile;
unsigned long byteswritten;

	// Attempt to open SD card configuration file
	if ((pFile = fopen(fileName, "wb")) != 0)
	{
		// file opened - read the data and be sure it's the right version
		byteswritten = fwrite(pNewData, size, 1, pFile);
		fclose(pFile);
		if (byteswritten != 1)
			return -1;			// failed writing to SD card
	}
	else return -1;			// failed writing to SD card

#else
FS_FILE *pFile;
unsigned long byteswritten;

	if (!OkToUseSDCard())
		return -1;
	
	// Attempt to open SD card configuration file
	if (pFile = FS_FOpen(fileName, "wb"))
	{
		// file opened - read the data and be sure it's the right version
		byteswritten = FS_FWrite(pNewData, size, 1, pFile);
		FS_FClose(pFile);
		FS_Sync("");			// added to ensure caches and buffers flushed -- gas 4/29/11
		if (byteswritten != 1)
			return -1;			// failed writing to SD card
	}
	else return -1;			// failed writing to SD card

#endif

	return 0;
}


#ifdef _WINDOWS

//
// Determine if we are running on a hard disk or a removable media and then
// build the required executable directory structure and move is there so that
// all subsequent file access can work relative to our current location.
//
void ConfigureHomeDirectory(void)
{
	char* pPathInit;

// If running on removable media then run here, done
	if (GetDriveType(NULL) == DRIVE_REMOVABLE)	// See if the current drive is a removable media
		return;

// If ConfigFiles folder is in the current directory then run here, done
	if (SetCurrentDirectory("ConfigFiles"))
		return;

// If we get here we are going to run in the My Documents folder, whether it exists or not
// so just proceed to make it and go there.
	pPathInit = getenv("USERPROFILE");			// Get 'user path' (c:\Documents and Settings\USERNAME)
	SetCurrentDirectory(pPathInit);

	SetCurrentDirectory("My Documents");

	CreateDirectory("Traqmate Data", NULL);
	SetCurrentDirectory("Traqmate Data");

	CreateDirectory("Traqdash PC", NULL);
	SetCurrentDirectory("Traqdash PC");
}

// reference utilities
	//_getcwd(directory, 255);			// Get current working drive and directory
	//if (directory[0] == 'c')
	//	removableMedia = false;
	//else removableMedia = true;

	//if (_getdrive() == 3)				// Get drive letter enumeration (a = 1, b = 2, c = 3,...)
	//	removableMedia = false;
	//else removableMedia = true;

	//if (!strcmp(directory + 1, ":"))
	//	runningInRoot = true;
	//else runningInRoot = false;
#endif


//
// Make sure target directory structure exists, build them if they don't
// Note, this builds from the 'application root directory' in a downward direction.
// If we are running on the PC ConfigureHomeDirectory must be called first
// to create the proper root directory and move us there.
//
// Note: the directories must be built in logical order - from top to bottom
// If a directory already exisits that call will fail and decrement the return counter.
//
int InitializeDirectoryStructure(void)
{
	int retval;

#ifdef _WINDOWS
	retval = CreateDirectory(SYSTEM_PATH, NULL);
	retval += CreateDirectory(DRIVER_PATH, NULL);
	retval += CreateDirectory(VEHICLE_PATH, NULL);
	retval += CreateDirectory(TRACK_PATH, NULL);
	retval += CreateDirectory(MY_SESSIONS_PATH, NULL);

	return retval;
#else
	if (unsavedSysData.sdCardInserted)
	{
		retval = FS_MkDir(SYSTEM_PATH);
		retval += FS_MkDir(DRIVER_PATH);
		retval += FS_MkDir(VEHICLE_PATH);
		retval += FS_MkDir(TRACK_PATH);
		retval += FS_MkDir(MY_SESSIONS_PATH);

		return retval;
	}
	else return -1;
#endif
}

void InitializeData(void)
{
	SetDefaultSystemData(false);
	SetDefaultVehicleData(false);
	SetDefaultTrackData();
	SetDefaultDriverData();
}

void SetDefaultSystemData(char full)
{
	if (full)
	{
		memset(&sysData, 0, sizeof(sysData));
		InitializeCriticalVariables();
	}

	strcpy(sysData.version, SYSTEM_VERSION);
	sysData.lappingMode = true;
	sysData.lastGaugeScreen = DEFAULT_RACE_MODE_SCREEN_INDEX;				// Set to default screen (note: lastXxxScreen is an Index, not an ID
	sysData.lastInstrumentsScreen = DEFAULT_INSTRUMENTS_MODE_SCREEN_INDEX;	// See GAUGE_SCREEN_LISTs in RunningGauges.c
	sysData.lastDriveScreen = DEFAULT_DRIVE_MODE_SCREEN_INDEX;
	sysData.launchGForce = 3;				// 3 = 0.3G launch force
	sysData.lapMeter = 1;					// +- 1.0 predictive lap meter scale
	sysData.timeZone = -5;					// -5 = Eastern
	sysData.dstOn = false;
	sysData.autoOn = false;
	sysData.autoRecord = false;
	sysData.radius = DEFAULT_RADIUS;
	SetMandatoryDefaultSelections();
	sysData.selectedSessionType = ROADRACE_TYPE;
	SetConfigurableItem( VMIN );			// Provide default to dummy value of VMIN input monitor
	SetDefaultTempCalData();
}

//
// These selections MUST be made at initial power up AND when the SD car is reformatted!
//
void SetMandatoryDefaultSelections(void)
{
	strcpy(sysData.driver, DEFAULT_DRIVER);
	strcpy(sysData.vehicle, DEFAULT_VEHICLE);
	strcpy(sysData.track, DEFAULT_TRACK);
}

void SetDefaultDriverData(void)
{
	memset(&driverData, 0, sizeof(driverData));
	strcpy(driverData.version, DRIVER_VERSION);
}

void SetDefaultTrackData(void)
{
	memset(&trackData, 0, sizeof(trackData));
	strcpy(trackData.version, TRACK_VERSION);
	trackData.laptimeHold = 60;
}

//
// SetDefaultVehicleData
//
// Input: true = look up in default vehicle table to find this vehicle and use
//				that data if available, otherwise use "default default" data
//		  false = just use plain default data (= 99 Mazda Miata)
// Returns:	nothing
//
void SetDefaultVehicleData(int how)
{
	int i, j;

	memset(&vehicleData, 0, sizeof(vehicleData));
	strcpy(vehicleData.version, VEHICLE_VERSION);
	vehicleData.camera = TD_CAM_MOBIUS;			// Set default TraqDash camera to Mobius and SU camera to none
	for (i = 0; i < 4; i++)
		SetDefaultAnalogInputData(i);

	for (i = 0; i < 2; i++)
		SetDefaultDigitalInputData(i);

	if (how == true)
	{
		j = 0;
		// see if this vehicle can be found in the pre-defined vehicle table
		while ('\0' != defaultVehicles[j].vehicleName[0])
		{
			if (strcmp(sysData.vehicle, defaultVehicles[j].vehicleName) == 0)
			{
				// Found the vehicle!  Copy the data and exit
				memcpy(&vehicleData.usbCameraDelay, &(defaultVehicles[j].carData), sizeof(defaultVehicles[j].carData));
				return;
			}
			j++;
		} // while
		// Did not find vehicle, use default data
	}

	// otherwise use normal default data (Spec Miata 99-05)
	// Find DEFAULT_VEHICLE in list
	j = 0;
	while ('\0' != defaultVehicles[j].vehicleName[0])
	{
		if (strcmp(DEFAULT_VEHICLE, defaultVehicles[j].vehicleName) == 0)
			break;
		else j++;
	}
	if ('\0' == defaultVehicles[j].vehicleName[0])
		j = 0;			// In case search fails us first vehicle in list

	memcpy(&vehicleData.usbCameraDelay, &(defaultVehicles[j].carData), sizeof(defaultVehicles[j].carData));
}

void SetDefaultAnalogInputData(int i)
{
	switch (i) {	
		case 0:		// AEM Air/Fuel Meter
			vehicleData.analogInput[i].upperAlarmTriggerComparison = ABOVE;
			vehicleData.analogInput[i].upperAlarmTriggerLevel = 13.7;
			vehicleData.analogInput[i].enabled = false;
			vehicleData.analogInput[i].EnableLED1 = false;
			vehicleData.analogInput[i].EnableLED2 = false;
			vehicleData.analogInput[i].highValuePoint = 18.0;
			vehicleData.analogInput[i].lowValuePoint = 8.5;
			vehicleData.analogInput[i].highVoltagePoint = 5.0;
			vehicleData.analogInput[i].lowVoltagePoint = 0.0;
			strcpy(vehicleData.analogInput[i].Name, "AEM AirFuel");
			strcpy(vehicleData.analogInput[i].Units, "Ratio");
			vehicleData.analogInput[i].inputRange = 5;
			break;
		case 1:		// AEM Pressure
			vehicleData.analogInput[i].upperAlarmTriggerComparison = ABOVE;
			vehicleData.analogInput[i].upperAlarmTriggerLevel = 20.0;
			vehicleData.analogInput[i].enabled = false;
			vehicleData.analogInput[i].EnableLED1 = false;
			vehicleData.analogInput[i].EnableLED2 = false;
			vehicleData.analogInput[i].highValuePoint = 35.3;
			vehicleData.analogInput[i].lowValuePoint = -14.7;
			vehicleData.analogInput[i].highVoltagePoint = 4.5;
			vehicleData.analogInput[i].lowVoltagePoint = 0.5;
			strcpy(vehicleData.analogInput[i].Name, "AEM MAP75");
			strcpy(vehicleData.analogInput[i].Units, "PSI");
			vehicleData.analogInput[i].inputRange = 5;
			break;
#if 0		// switched to PLX
		case 2:		// AEM Temperature
			// v = iR, v=4.71V reference, assume 40k internal resistance
			// 0 deg c = 32 f
			// 0 deg c = 9.5k ohm = .904 volts
			// 110 deg c = 230 f
			// 110 deg c = 134.7 ohm = .0158 volts
			vehicleData.analogInput[i].upperAlarmTriggerComparison = ABOVE;
			vehicleData.analogInput[i].upperAlarmTriggerLevel = 210.0;
			vehicleData.analogInput[i].enabled = false;
			vehicleData.analogInput[i].EnableLED1 = false;
			vehicleData.analogInput[i].EnableLED2 = false;
			vehicleData.analogInput[i].highValuePoint = 32.0;
			vehicleData.analogInput[i].lowValuePoint = 230.0;
			vehicleData.analogInput[i].highVoltagePoint = 0.904;
			vehicleData.analogInput[i].lowVoltagePoint = 0.0158;
			strcpy(vehicleData.analogInput[i].Name, "AEM Temp");
			strcpy(vehicleData.analogInput[i].Units, "DegF");
			vehicleData.analogInput[i].inputRange = 5;
			break;
#endif
		case 2:		// PLX Temperature
			vehicleData.analogInput[i].upperAlarmTriggerComparison = ABOVE;
			vehicleData.analogInput[i].upperAlarmTriggerLevel = 210.0;
			vehicleData.analogInput[i].enabled = false;
			vehicleData.analogInput[i].EnableLED1 = false;
			vehicleData.analogInput[i].EnableLED2 = false;
			vehicleData.analogInput[i].highValuePoint = 302.0;
			vehicleData.analogInput[i].lowValuePoint = 32.0;
			vehicleData.analogInput[i].highVoltagePoint = 5.0;
			vehicleData.analogInput[i].lowVoltagePoint = 0.0;
			strcpy(vehicleData.analogInput[i].Name, "PLX Temp");
			strcpy(vehicleData.analogInput[i].Units, "DegF");
			vehicleData.analogInput[i].inputRange = 5;
			break;
		case 3:		// Battery Voltage
			vehicleData.analogInput[i].lowerAlarmTriggerComparison = BELOW;
			vehicleData.analogInput[i].lowerAlarmTriggerLevel = 11.5;
			vehicleData.analogInput[i].enabled = false;
			vehicleData.analogInput[i].EnableLED1 = false;
			vehicleData.analogInput[i].EnableLED2 = false;
			vehicleData.analogInput[i].highValuePoint = 20.0;
			vehicleData.analogInput[i].lowValuePoint = 0.0;
			vehicleData.analogInput[i].highVoltagePoint = 20.0;
			vehicleData.analogInput[i].lowVoltagePoint = 0.0;
			strcpy(vehicleData.analogInput[i].Name, "Battery");
			strcpy(vehicleData.analogInput[i].Units, "Volts");
			vehicleData.analogInput[i].inputRange = 20;
			break;
	} // switch
}

void SetDefaultDigitalInputData(int i)
{
	char scratch[10];

	switch (i) {	
		case 1:		// Brake light
			vehicleData.digitalInput[i].alarmTriggerState = DI_OFF;
			vehicleData.digitalInput[i].enabled = false;
			vehicleData.digitalInput[i].EnableLED1 = false;
			vehicleData.digitalInput[i].EnableLED2 = false;
			strcpy(vehicleData.digitalInput[i].Name, "Brake");
			strcpy(vehicleData.digitalInput[i].HighName, "Go");
			strcpy(vehicleData.digitalInput[i].LowName, "Stop");
			break;
		default:
		case 0:		// Usually camera so set generic
			vehicleData.digitalInput[i].alarmTriggerState = DI_ON;
			vehicleData.digitalInput[i].enabled = false;
			vehicleData.digitalInput[i].EnableLED1 = false;
			vehicleData.digitalInput[i].EnableLED2 = false;
			sprintf(scratch, "Digital %d", i + 4);
			strcpy(vehicleData.digitalInput[i].Name, scratch);
			strcpy(vehicleData.digitalInput[i].HighName, "ON");
			strcpy(vehicleData.digitalInput[i].LowName, "OFF");
			break;
	} // switch
}

void SetDefaultTach(void)
{
	vehicleData.tach.upperRedStart = 7000;
	vehicleData.tach.upperYellowStart = 5000;
	vehicleData.tach.greenEnd = 5000;
	vehicleData.tach.greenStart = 4000;
	vehicleData.tach.scaleEnd = 7300;
}

void SetDefaultGears(void)
{
	vehicleData.gearRatios[0] = 3.14;
	vehicleData.gearRatios[1] = 1.89;
	vehicleData.gearRatios[2] = 1.33;
	vehicleData.gearRatios[3] = 1.00;
	vehicleData.gearRatios[4] = 0.81;
	vehicleData.gearRatios[5] = 0.0;
	vehicleData.gearRatios[6] = 0.0;
	vehicleData.gearRatios[7] = 0.0;
}

//
// Initialize veriables required immediately at startup
//
void InitializeCriticalVariables(void)
{
	unsavedSysData.systemMode = DU2_STARTUP;

	sysData.AccelerometerSampleRate = 40;
	sysData.GPSSampleRate = 4;
	sysData.screentype = GENERAL_CONFIG_SCREEN_TYPE;
	sysData.lastConfigScreen = -1;		// -1 = screen not yet set
	sysData.lastGaugeScreen = -1;		// -1 = screen not yet set
	sysData.touchScreenCalibrated = FALSE;
	sysData.units = STD;
	sysData.backlight = DEFAULT_BACKLIGHT_BRIGHTNESS;
	sysData.LEDBrightness = DEFAULT_LED_BRIGHTNESS;
	sysData.lightLevelWhenIntensitySet = DEFAULT_LIGHT_LEVEL_WHEN_LED_SET;
	sysData.autoAdjustOn = true;
	sysData.timeZone = -5;		// -5 = Eastern
	sysData.dstOn = false;
	sysData.autoOn = false;
	sysData.autoRecord = false;
// KMC - this variable is already in use!  Do not change!	unsavedSysData.sdCardInserted = false;
}

//
// Take simple file name with extension and make a complete file name with path (no file extension is added)
//
// Input:	pDest	pointer to empty array to contain new file name with path
//			pName	pointer to file name (without path)
//			type	data type of file
//
// Output:	pDest	file name with path
//
void AddPath(char *pDest, char *pName, enum DATATYPES type)
{
	const char *pPath;

	switch (type)
	{
	default:
	case SYSTEM_DATA:	pPath = SYSTEM_PATH;	break;
	case DRIVER_DATA:	pPath = DRIVER_PATH;	break;
	case VEHICLE_DATA:	pPath = VEHICLE_PATH;	break;
	case TRACK_DATA:	pPath = TRACK_PATH;		break;
	}

	strcpy(pDest, pPath);
	strcat(pDest, "\\");
	strcat(pDest, pName);
}


//
// Take simple file name and make a complete file name with path and file extension as required for this file type
//
// Input:	pDest	pointer to empty array to contain new destintation full file name
//			pName	pointer to simple file name (no extension or path)
//			type	data type of file
//
// Output:	pDest	contains fully qualified name
//
void ConvertToFilename(char *pDest, char *pName, enum DATATYPES type)
{
	AddPath(pDest, pName, type);
	strcat(pDest, DAT_EXT);
}

//
// Will reload the selected data item.  If the file is not found or the load is corrupted or fails
// then this routine will initialize the data structure with default data.
//
// returns: true - successfully loaded data
//			false - loaded default data
//
int ReloadData(enum DATATYPES type)
{
	long size;
	char shortFileName[NORMAL_FILENAME_SIZE];
	char fileName[MAX_FILENAME_SIZE];
	ALL_DATA_UNION dataunion;


	switch (type)
	{
		case SYSTEM_DATA:
			strcpy(shortFileName, SYSTEM_FILE_NAME);
			size = sizeof(SYS_DATA_STRUCTURE);
			break;
		case DRIVER_DATA:
			strcpy(shortFileName, sysData.driver);
			size = sizeof(DRIVER_DATA_STRUCTURE);
			break;
		case VEHICLE_DATA:
			strcpy(shortFileName, sysData.vehicle);
			size = sizeof(VEHICLE_DATA_STRUCTURE);
			break;
		case TRACK_DATA:
			strcpy(shortFileName, sysData.track);
			size = sizeof(TRACK_DATA_STRUCTURE);
			break;
	default:
		return false;
	}

	// If we somehow reverted to default data then the track name might be blank!
	// Or if, for any other reason, the name of the file we are about to open is blank
	// then skip this and go directly to loading default data
	if (strcmp("", shortFileName) != 0)
	{
		ConvertToFilename(fileName, shortFileName, type);
	
		if (FRWReadFile(&dataunion, fileName, size, type))
			return true;
		else if (type == SYSTEM_DATA)
		{
			ConvertToFilename(fileName, (char *)BACKUP_SYSTEM_FILE_NAME, type);
			if (FRWReadFile(&dataunion, fileName, size, type))
				return true;
		}
	}

	// Read or verify failed - load default data
	switch (type)
	{
		case SYSTEM_DATA:
			SetDefaultSystemData(true);
			break;
		case DRIVER_DATA:
			SetDefaultDriverData();
			if (!AnyFilesFound(type))
				CreateAllFiles(type);
			break;
		case VEHICLE_DATA:
			SetDefaultVehicleData(true);		// Attempt to find vehicle in default table and use that data
			if (!AnyFilesFound(type))
				CreateAllFiles(type);
			break;
		case TRACK_DATA:
			SetDefaultTrackData();
			SetTrackTableDefaultData();
			// This track only exists in the track table but there is not yet a disk file for this track.
			// Do not create the disk file yet - wait until after a session is performed or data is manually
			// changed in the track settings before creating a disk file.
			return true;
	}
	WriteUserDataEvent(type);			// Save newly restored/initialized data

	return false;
}

char FRWReadFile(ALL_DATA_UNION *dataunion, char *fileName, int size, enum DATATYPES type)
{
#ifdef _WINDOWS
	FILE *pFile;

	if ((pFile = fopen(fileName, "rb")) != 0)
	{
		// file opened - read the data and be sure it's the right version
		////results = fread(dataunion, size, 1, pFile);
		fread(dataunion, size, 1, pFile);
		fclose(pFile);

		// KMC NOTE - cannot test results for size of file here!  If we have upgraded versions we
		// might have asked to read more data than the file actually contained.  In which case
		// this call will return 0.  Instead allow to pass through and use my own checksum and version string
		// tests to determine the file validity and update if possible.
		////if (results != 0)
#else
	unsigned long results;
	FS_FILE *pFile;
	
	if (!OkToUseSDCard())
		return false;

	// Attempt to read SD card configuration file
	if (pFile = FS_FOpen(fileName, "rb"))
	{
		// file opened - read the data and be sure it's the right version
		results = FS_Read(pFile, dataunion, size);
		FS_FClose(pFile);
		
		// KMC NOTE - cannot test results for size of file here!  If we have upgraded versions we
		// might have asked to read more data than the file actually contained.  In which case
		// this call will return a value other than the size we asked for.  Instead only check
		// for non-zero and allow to pass through and use my own checksum and version string
		// tests to determine the file validity and update if possible.
		if (results != 0)
#endif
			if (verifyData(type, dataunion, true) == true)
				return true;
			else return false;	// else initialize to default data
	}
	return false;
}


//
// See if any ".dat" files exist in this directory
// Return true if one or more files found, else return false
//
char AnyFilesFound(enum DATATYPES type)
{
	const char *pDirectory;
#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

	switch (type)
	{
	case DRIVER_DATA:	pDirectory = DRIVER_PATH;		break;
	case VEHICLE_DATA:	pDirectory = VEHICLE_PATH;		break;
	case TRACK_DATA:	pDirectory = TRACK_PATH;		break;
	default:
	case SESSION_DATA:	pDirectory = MY_SESSIONS_PATH;	break;
	}

#ifdef _WINDOWS
	dir = opendir(pDirectory);
	if (dir != NULL)
	{
		while ((ent = readdir(dir)) != NULL) {
			if (ValidFileName(ent->d_name))
			{
				closedir(dir);
				return true;
			}
		}
		closedir(dir);
	}
#else
	if (FS_FindFirstFile(&fd, pDirectory, acFilename, sizeof(acFilename)) == 0) {
		do {
			if (ValidFileName(acFilename))
			{
				FS_FindClose(&fd);
				return true;
			}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);
#endif
	return false;
}

//
// Populate the specified directory with the standard file set
// Note: assumes the relivant data structure is initialized with default data
// Will return the the relivant data structure still containing the default data
//
void CreateAllFiles(enum DATATYPES type)
{
	if (type == DRIVER_DATA)
	{
		strcpy(sysData.driver, "Driver B");
		WriteUserDataEvent(DRIVER_DATA);
		strcpy(sysData.driver, "Driver C");
		WriteUserDataEvent(DRIVER_DATA);
		strcpy(sysData.driver, "Driver D");
		WriteUserDataEvent(DRIVER_DATA);
		strcpy(sysData.driver, "Driver E");
		WriteUserDataEvent(DRIVER_DATA);
		strcpy(sysData.driver, DEFAULT_DRIVER);
	}
	else if (type == VEHICLE_DATA)
	{
		
		// make up files for all the default vehicles
		int i=0;
		while ('\0' != defaultVehicles[i].vehicleName[0]) {
			
			strcpy(sysData.vehicle, defaultVehicles[i].vehicleName);
			memcpy(&vehicleData.usbCameraDelay, &(defaultVehicles[i].carData), sizeof(defaultVehicles[i].carData));
			WriteUserDataEvent(VEHICLE_DATA);
			
			i++;
		} // while

	//	strcpy(sysData.vehicle, DEFAULT_VEHICLE);
	//	memcpy(&vehicleData.usbCameraDelay, &(defaultVehicles[0].carData), sizeof(defaultVehicles[i].carData));		// Restore data back to Miata (default)
	//	WriteUserDataEvent(VEHICLE_DATA);

		strcpy(tempVehicleName, DEFAULT_VEHICLE);	// Set default vehicle name
		vehicleSelectionChanged = true;				// mark as new vehicle selection
		ChangeVehicle();							// load the data for this vehicle
	}
}


//
// Delete all files in the indicated TraqDash standard folder
//
void DeleteAllFiles(enum DATATYPES type)
{
	const char *pDirectory;
	char fullName[MAX_FILENAME_SIZE];
#ifdef _WINDOWS
	DIR *dir;
	struct dirent *ent;
#else
	FS_FIND_DATA fd;
	char acFilename[NORMAL_FILENAME_SIZE];
#endif

	switch (type)
	{
	case DRIVER_DATA:	pDirectory = DRIVER_PATH;		break;
	case VEHICLE_DATA:	pDirectory = VEHICLE_PATH;		break;
	case TRACK_DATA:	pDirectory = TRACK_PATH;		break;
	default:
	case SESSION_DATA:	pDirectory = MY_SESSIONS_PATH;	break;
	}

#ifdef _WINDOWS

	dir = opendir(pDirectory);
	if (dir != NULL)
	{
		while ((ent = readdir(dir)) != NULL) {
			if (ValidFileName(ent->d_name))
			{
				strcpy(fullName, pDirectory);
				strcat(fullName, "\\");
				strcat(fullName, ent->d_name);
				remove(fullName);
			}
		}
		closedir(dir);
	}

#else

	if (FS_FindFirstFile(&fd, pDirectory, acFilename, sizeof(acFilename)) == 0) {
		do {
			if (ValidFileName(acFilename))
			{
				strcpy(fullName, pDirectory);
				strcat(fullName, "\\");
				strcat(fullName, acFilename);
				FS_Remove(fullName);
			}
		} while (FS_FindNextFile (&fd));
	}
	FS_FindClose(&fd);

#endif
}

#ifndef _WINDOWS
//
// General utility that a folder and all files in the folder including subdirectories
// NOTE: Recursion !!
//
void WipeFiles(char *folder) {
	char fullName[MAX_FILENAME_SIZE];
	FS_FIND_DATA fd;		// file attributes
	char acFilename[NORMAL_FILENAME_SIZE];
	U8 attrib;				// file attributes

	// make sure the selected folder is read/write
	FS_SetFileAttributes (folder, 0);
				
	if (FS_FindFirstFile(&fd, folder, acFilename, sizeof(acFilename)) == 0) {
		do {
			if ('.' != acFilename[0]) {		// screen out '.' and '..'
				strcpy(fullName, folder);
				strcat(fullName, "\\");
				strcat(fullName, acFilename);
				
				attrib = FS_GetFileAttributes(fullName);
				if (attrib & FS_ATTR_READ_ONLY)
					FS_SetFileAttributes(fullName, 0);
				
				if (attrib & FS_ATTR_DIRECTORY) 
					WipeFiles(fullName);			// call this routine recursively
				else 		// regular file			
					FS_Remove(fullName);
			} // if

		} while (FS_FindNextFile (&fd));
	} // if
	FS_FindClose(&fd);
	
	// now get rid of folder
	FS_RmDir(folder);
	
} // WipeFiles
#endif

static const GUI_WIDGET_CREATE_INFO UpdatedFileResources[] = {
	{ FRAMEWIN_CreateIndirect, "Check Your Settings", 0, 30, 20, 400, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Vehicle data, including inputs,", 0, 0, 10, 390, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "updated to a new version.",    0, 0, 40, 390, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Check your alarm settings.",    0, 0, 70, 390, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 145, 115, 100, 60 },
};

void DisplayUpdatedVehicleDataPopup(void)
{
	int x, y;

	HideSprites();

	SetPopupDefaults(WARNING_POPUP);

	glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed

	systemSettingsPopupScreen = 0;

	GUI_GetOrg(&x, &y);	
	GUI_ExecDialogBox(UpdatedFileResources, GUI_COUNTOF(UpdatedFileResources), &popupCallback, WM_GetDesktopWindow(), 10, y + 10);

	UnhideSprites();
}

