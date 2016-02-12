
#ifndef	_GEARINGSCREENS_H
#define	_GEARINGSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif

WM_HWIN GetGearingWindowHandle(void);
void SetGearingWindowHandle(WM_HWIN);
extern struct SCREEN_LIST GearingScreenList[];
extern void GearingScreensCallback(WM_MESSAGE *);
void GearingIgnoreNextKeyRelease(void);
WM_HWIN GetTireSizeScreenWindowHandle(void);
extern void TCKeyboardSaveButton(void);
void GetTireCircumferenceString(char withUnits, char *scratch);


#ifdef __cplusplus
}
#endif


#define	NUM_GEARING_SCREENS			4

#define	TIRESIZE_SELECTION_SCREEN	0
#define	DIFFRATIO_SCREEN			1
#define	TRANSRATIO_SCREEN			2
#define	HELP_SCREEN					3
// Add tiresize screen to enumeration (but not num screens) for navigation arrow lookup
#define	TIRESIZE_SCREEN				4
#define	TIRECIRCUMFERENCE_SCREEN	5

#endif
