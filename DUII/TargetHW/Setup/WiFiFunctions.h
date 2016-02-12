#ifndef	WIFIFUNCTIONS_H
#define	WIFIFUNCTIONS_H

// include after FlashReadWrite.h and FS.h

#define WLAN_PATH		"SD_WLAN"				// folder where WiFi SD card config is located
#define WIFI_CFG_FILE	"CONFIG"				// filename of WiFi SD card config
#define FLASHAIR_PATH	"GUPIXINF"				// folder where WiFi SD card stores some junk
#define PIC_PATH		"DCIM"					// where pictures are stored

// function prototypes
void WiFiSetup(int);
void WriteLine(FS_FILE *, char *);
int ReadLine(FS_FILE *, char *, int);

// global data
extern const char WIFI_PATH[];
extern const char WIFI_FILE[];

#endif

