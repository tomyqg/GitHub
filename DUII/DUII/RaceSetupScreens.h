
#ifndef	_RACESETUPSCREENS_H
#define	_RACESETUPSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif


enum CHOOSE_TRACK_SCREEN_TYPE {
	CT_NORMAL_MODE,
	CT_RACE_ENTRY
};

enum SESSION_TEXT_TYPE {
	ST_GENERAL,
	ST_PERFORMANCE,
	ST_INPUTS,
	ST_CONFIG
};

struct NAMESTRUCT {
	char time[10];
	char date[11];
	char track[NAMELEN+1];
	char vehicle[NAMELEN+1];
	char driver[NAMELEN+1];
	enum SESSION_TYPE sessionType;
	enum SESSION_TYPE dragTrackType;
	char sessionNumber;
};


WM_HWIN GetRaceSetupScreensWindowHandle(void);
void SetRaceSetupScreensWindowHandle(WM_HWIN);
extern struct SCREEN_LIST RaceSetupScreenList[];
extern void RaceSetupScreensCallback(WM_MESSAGE *);
void RaceSetupIgnoreNextKeyRelease(void);
int CreateFileList(enum DATATYPES type);
char **CreateLapsList(void);
char *GetBestLap(int displayLapNumber, int index, char *pStr);
char *GetAutoxTime(int index, char *pStr);
char *GetDragTime(int index, char *pStr);
char *GetSessionName(void);
void DeleteSession(void);
void ResetAllFileLists(char doCreateTracks);
char ValidFileName(char *pName);
void SetTrackTableDefaultData(void);
void DisplayTrackSelection(WM_HWIN);
void CreateTrackList(void);
void FinishDisplayTrackSelection(WM_HWIN hParent);
void DisplayLapsSelection(WM_HWIN);
void ExitTrackSelectionScreen(void);
void CheckWorkingTrackName(void);
void SetWorkingTrackName(void);
// KMC added 6/6/12 to allow GoRacing to display CLOSEST track first, not currently selected track
void ClearWorkingTrackName(void);
void GetSessionData(void);
void GetSessionNameData(WM_HWIN hParent, char primary, enum SESSION_TYPE type);
int FindNewestFile(enum SESSION_TYPE type);
int GetBestLapNumber(int session);
char *GetSessionText(enum SESSION_TEXT_TYPE type);
char *GetSelectedReviewModeText(void);
void SetBestTimeButtonText(void);
void GetStringNumber(char index, char *pDest, char *pSource);
void IncrementDisplayType(void);
void InitReviewDisplayTypes(void);
enum TRACK_TYPES SessionTypeToTrackType(enum SESSION_TYPE type);
enum SESSION_TYPE TrackTypeToSessionType(enum TRACK_TYPES type);
char *GetSessionDataString(char index, enum SESSION_TEXT_TYPE type, char *pDest);
int GetNumTracks(void);
char *GetBestLapAutocross(char *pDest);
extern void ChangeVehicle(void);
void HideSprites(void);
void UnhideSprites(void);
void ClearSessionNameData(void);


extern char **pTracks;
extern char **pLaps;
extern int numLaps;
extern char sessionName[];
extern enum CHOOSE_TRACK_SCREEN_TYPE ctType;
extern int trackTableIndex;		// Global index to the track table of the currently selected track
extern struct NAMESTRUCT NameStruct;
extern struct NAMESTRUCT SessionNameData[];
extern long BestLaps[];
extern char vehicleSelectionChanged;
extern char tempVehicleName[];

#ifdef __cplusplus
}
#endif


#define	NUM_RACE_SETUP_SCREENS		4

#define	DRIVER_SELECTION			0
#define	VEHICLE_SELECTION			1
#define	TRACK_SELECTION				2
#define	LAPSQUALIFYING_SELECTION	3


#endif
