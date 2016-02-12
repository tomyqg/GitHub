
#ifndef	_SIMCONF_H
#define	_SIMCONF_H

void SIM_Init(void);
int SIM_Tick(void);
void SIM_Powerkey_CB(int KeyIndex, int State);

#define	POWER_KEY			7
#define	WARNING_LED_1		8
#define	WARNING_LED_2		9
#define	TOP_LED_CENTER		0
#define	TOP_LED_LEFT_1		1
#define	TOP_LED_LEFT_2		3
#define	TOP_LED_LEFT_3		5
#define	TOP_LED_RIGHT_1	2
#define	TOP_LED_RIGHT_2	4
#define	TOP_LED_RIGHT_3	6


#endif
