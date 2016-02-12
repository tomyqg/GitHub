// these functions are used with the Toshiba FlashAir WiFi SD card
//

#include "DUII.h"
#include "commondefs.h"
#include "GeneralConfig.h"
#if	_WINDOWS
#include "windows.h"
#else
#include "RTOS.H"
#include "FS.h"
#include "dataflash.h"
#include "FlashReadWrite.h"
#include "WiFiFunctions.h"
#endif
#include <stdio.h>
#include <string.h>

#define LINELEN	80

const char WIFI_PATH[] = { LEADING_PATH WLAN_PATH };
const char WIFI_FILE[] = { LEADING_PATH WLAN_PATH "\\" WIFI_CFG_FILE };
const char WIFI_OLD[] = { LEADING_PATH WLAN_PATH "\\CFG_OLD" };
const char JUNK_PATH[] = { LEADING_PATH FLASHAIR_PATH };
const char PHOTO_PATH[] = { LEADING_PATH PIC_PATH };
const char WIFI_TEMP[] = { LEADING_PATH WIFI_CFG_FILE };

/* function spec
on sd card

change folder GUPININF to read/write
remove folder GUPIXINF and contents / subfolders
change folder DCIM to read/write
remove folder DCIM and contents / subfolders
change folder SD_WLAN to read/write

in folder SD_WLAN
delete *.jpg
delete *.htm

// determine if config file is original
open \\SD_WLAN\CONFIG for read
read first line
read second line
if second line is 'VENDOR=Traqmate' process complete
close \\SD_WLAN\CONFIG

rename file \\SD_WLAN\CONFIG to \\SD_WLAN\CFG_OLD
open file \\SD_WLAN\CFG_OLD for read
open file \\SD_WLAN\CONFIG for write

write line in CONFIG
[Vendor]
VENDOR=Traqmate
PRODUCT=TraqDashWiFi
LOCK=1
APPMODE=4
APPAUTOTIME=0
APPNAME=traqdash

if (unsavedSysData.wifiname[0] == '\0' || unsavedSysData.wifiname[0] == 0xFF)
	APPSSID=TraqDashWiFiXXXX where XXXX is last 4 digits of TraqDash serial number
	unsavedSysData.wifiname = TraqDashWiFiXXXX where XXXX is last 4 digits of TraqDash serial number
else
	APPSID= unsavedSysData.wifiname

		
if (unsavedSysData.wifipass[0] == '\0' || unsavedSysData.wifiname[0] == 0xFF)
	APPNETWORKKEY="traqmate"
	unsavedSysData.wifiname = "traqmate"
else
	APPNETWORKKEY= unsavedSysData.wifipass
		
read CFG_OLD line by line
if line starts with 'VERSION=' or 'CID='
copy line to \\SD_WLAN\CONFIG
*/

//
// this function will "terraform" a fresh Toshiba FlashAir WiFi SD card to make it into a TraqDash WiFi network
//
void WiFiSetup(int rewrite) {
	FS_FILE *pConfig;
	FS_FILE *pOldConfig;
	char line[LINELEN];				// max 80 characters
	
	// check for GUPIXINF folder
	if (0xFF != FS_GetFileAttributes(JUNK_PATH)) {
		rewrite = true;
		// delete folder GUPININF and everything in it
		WipeFiles((char *) JUNK_PATH);
	} // if

	// check for DCIM folder
	if (0xFF != FS_GetFileAttributes(PHOTO_PATH)) {
		rewrite = true;
		// delete folder DCIM and everything in it
		WipeFiles((char *) PHOTO_PATH);
	} // if

	if (false == rewrite) {		// check to see if it is an updated file
		// open file \\SD_WLAN\CONFIG for write
		pConfig = FS_FOpen(WIFI_FILE, "r");

		while (0 != (ReadLine(pConfig, line, LINELEN))) {
			if (0 == strncmp(line, "VENDOR=Traqmate", 15)) {
				rewrite = false;
				break;
			} // if
		} // while

		// close the file
		FS_FClose(pConfig);
	} // if
	
	if (rewrite) { // clean SD_WLAN folder
		// move CONFIG file from SD_WLAN to root directory
		FS_Move(WIFI_FILE, "\\");
		
		// remove SD_WLAN directory (in order to purge all files except CONFIG)
		WipeFiles((char *) WIFI_PATH);
		
		// create SD_WLAN directory
		FS_MkDir(WIFI_PATH);
		
		// move CONFIG file back to SD_WLAN directory
		FS_Move("\\CONFIG", "\\SD_WLAN\\");
		
		// rename file \\SD_WLAN\CONFIG to \\SD_WLAN\CFG_OLD
		FS_Rename("\\SD_WLAN\\CONFIG", "CFG_OLD");
		
		// flush out the cache
		FS_Sync("");
	  
		// open file \\SD_WLAN\CFG_OLD for read
		pOldConfig = FS_FOpen(WIFI_OLD, "r");

		// open file \\SD_WLAN\CONFIG for write
		pConfig = FS_FOpen(WIFI_FILE, "w");
			
		// put stuff in new config file - note that order is important
		WriteLine(pConfig, "[Vendor]");
		WriteLine(pConfig, "APPMODE=4");
		WriteLine(pConfig, "APPAUTOTIME=0");
		WriteLine(pConfig, "APPNAME=traqdash");
		strcpy (line, "APPNETWORKKEY=");
		strcat (line, unsavedSysData.unitInfo.wifiPass);
		WriteLine(pConfig, line);
		
		// read old config file line by line and copy 2 important ones over
		while (0 != (ReadLine(pOldConfig, line, LINELEN))) {
			if ((0 == strncmp(line, "VERSION=", 8)) || (0 == strncmp(line, "CID=", 4)))
				FS_Write(pConfig, line, strlen(line));		// use FS_Write because already contains newline
		} // while

		WriteLine(pConfig, "PRODUCT=TraqDashWiFi");
		WriteLine(pConfig, "VENDOR=Traqmate");
		strcpy (line, "APPSSID=");
		strcat (line, unsavedSysData.unitInfo.wifiName);
		WriteLine(pConfig, line);
		WriteLine(pConfig, "LOCK=1");

		// close the files
		FS_FClose(pOldConfig);
		FS_FClose(pConfig);
	
		// change path back to hidden and read only
		FS_SetFileAttributes(WIFI_PATH, FS_ATTR_HIDDEN | FS_ATTR_READ_ONLY);

	} // if
} // WiFiSetup()

// writes a text line to designated file and appends a newline
void WriteLine(FS_FILE *pFile, char *line) {
	char outline[LINELEN+2];
	int len = -1;
	
	do {
		len++;
	} while ('\0' != (outline[len] = line[len]));
	
	outline[len++] = '\r';
	outline[len++] = '\n';
	FS_Write(pFile, outline, len);

} // WriteLine

// gets a string up to and including a carriage return / newline from the designated file
int ReadLine(FS_FILE *pFile, char *line, int len) {
	char ch;
	int cnt = 0;
	int numread;
	
	while (0 != (numread = FS_Read(pFile, &ch, 1))) {
		line[cnt++] = ch;
		if (cnt >= (len-1)) break;
		if ('\n' == ch) break;
	} // while

	if (0 == numread)
		return 0;

	line[cnt++] = '\0';
	return cnt;
} // ReadLine