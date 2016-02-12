
#ifndef	FLASHREADWRITE_H
#define	FLASHREADWRITE_H

#include "DUII.h"

#ifdef _WINDOWS
#define LEADING_PATH	""
#else
#define LEADING_PATH	"\\"
#endif

#define VER_FILE		"Version"				// filename for sw version file
#define CFG_PATH		"ConfigFiles"			// path to config files
#define DAT_EXT			".dat"					// data file extension

#define	DEFAULT_DIFF_RATIO		(4.10F)			// numbers for placeholders
#define	DEFAULT_VEHICLE_WEIGHT	3000

#define	DEFAULT_RADIUS			161
// Needed to make the world "bigger" due to math error in calculating distance to Adelaide
#define	WHOLE_WORLD				50000
#define	OLD_WORLD				20000

#define	INVALID_SYSTEM_DATA		0x01
#define	INVALID_VEHICLE_DATA	0x02
#define	INVALID_TRACK_DATA		0x04
#define	INVALID_DRIVER_DATA		0x08

int ReadDataStructures(void);
int WriteDataStructures(enum DATATYPES);
void InitializeData(void);
int WriteUserDataEvent(enum DATATYPES);
int ReloadData(enum DATATYPES);
void InitializeCriticalVariables(void);
void SetDefaultSystemData(char full);
void SetDefaultDriverData(void);
void SetDefaultTrackData(void);
void SetDefaultVehicleData(int how);
void ConvertToFilename(char *pDest, char *pName, enum DATATYPES type);
void AddPath(char *pDest, char *pName, enum DATATYPES type);
void SetDefaultTach(void);
void SetDefaultGears(void);
int InitializeDirectoryStructure(void);
char verifyData(enum DATATYPES type, void *, int);
void SetMandatoryDefaultSelections(void);
void DeleteAllFiles(enum DATATYPES type);
char AnyFilesFound(enum DATATYPES type);
void CreateAllFiles(enum DATATYPES type);
void WipeFiles(char *);

#ifdef _WINDOWS
void ConfigureHomeDirectory(void);
#endif

extern const char SYSTEM_PATH[];
extern const char DRIVER_PATH[];
extern const char VEHICLE_PATH[];
extern const char TRACK_PATH[];
extern const char MY_SESSIONS_PATH[];
extern const char DEFAULT_TRACK[];
extern const char DEFAULT_DRIVER[];
extern const char DEFAULT_VEHICLE[];
extern const char DEFAULT_TRACK[];
extern struct MINI_VEHICLE_DATA_STRUCTURE BoxsterS_03;
extern struct MINI_VEHICLE_DATA_STRUCTURE GT3RS_03;
extern struct MINI_VEHICLE_DATA_STRUCTURE S2000_00;
extern struct MINI_VEHICLE_DATA_STRUCTURE NISSAN_350Z_03;
extern struct MINI_VEHICLE_DATA_STRUCTURE BMW_E30_M3;

#endif

