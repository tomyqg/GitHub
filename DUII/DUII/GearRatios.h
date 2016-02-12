
#ifndef	_GEARRATIOS_H
#define	_GEARRATIOS_H


WM_HWIN GetGearRatiosWindowHandle(void);
void SetGearRatiosWindowHandle(WM_HWIN);
extern struct SCREEN_LIST GearRatiosScreenList[];
extern void GearRatiosCallback(WM_MESSAGE *);
void GearRatiosIgnoreNextKeyRelease(void);
void DeleteGearRatiosKeyboards(void);
extern void trim(char *s);

#define	NUM_GEAR_RATIOS_SCREENS	8

#define	FIRST_GEAR_SCREEN		0
#define	SECOND_GEAR_SCREEN		1
#define	THIRD_GEAR_SCREEN		2
#define	FOURTH_GEAR_SCREEN		3
#define	FIFTH_GEAR_SCREEN		4
#define	SIXTH_GEAR_SCREEN		5
#define	SEVENTH_GEAR_SCREEN		6
#define	EIGHTH_GEAR_SCREEN		7


#endif
