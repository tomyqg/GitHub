
#ifndef	TRACK_DETECT_H
#define	TRACK_DETECT_H

#define NUM_SYSTEM_TRACKS	375
#define NUM_USER_TRACKS		50
#define NUM_TOTAL_TRACKS	(NUM_SYSTEM_TRACKS + NUM_USER_TRACKS)

#define CURRENT_TRACK_KM		16.0				// if we are within 16 km of current track, don't change
#define CURRENT_TRACK_M			(CURRENT_TRACK_KM * 1000.0)

typedef struct {
	char trackName[NAMELEN+1];						// 17
	int sdCardEntry;								// 4 true if this entry has more information on sd card
	double  startLineLat;							// 8
	double  startLineLon;							// 8
	float   startLineHeading;						// 4
	double  finishLineLat;							// 8
	double  finishLineLon;							// 8
	float   finishLineHeading;						// 4
	int		courseType;								// 4 what kind of race track
} TRACK_LIST_TYPE;									// 68

int ReadTracks(void);
int CreateTracks(void);
int DeleteTrack( char *);
int AddTrack( char *, TRACK_DATA_STRUCTURE *);
int CurrentTrackInRange( void );
void GetClosestTracks( int );
void UpdateSFLines(void);

extern int trackShortList[];						// indexes index into trackTable by closest distance to current position
extern int numberOfShortListTracks;					// number of tracks on the short list
extern int numberOfTracks;							// number of tracks on the list
extern TRACK_LIST_TYPE trackTable[];				// preprogrammed and user tracks combined table

#endif
