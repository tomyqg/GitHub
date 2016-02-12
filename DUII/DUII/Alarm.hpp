/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: Alarm.hpp
Description: Alarm class
-----------------------------------------------------------------
*/

#include "DUII.h"

#ifndef ALARM_HPP
#define ALARM_HPP

typedef enum {
	NORMAL = 1,
	LOW,
	HIGH
} AlarmType;

class Alarm
{
public:
	Alarm(bool bEnabled, float fMinValue, float fMaxValue, GUI_COLOR cColor);
	Alarm(bool bEnabled, float fMinValue, float fMaxValue, GUI_COLOR cColor, GUI_COLOR cDarkColor, AlarmType tType, Alarm *pNext, Alarm *pPrev);

	bool Enabled;
	float MinValue, MaxValue;	
	GUI_COLOR Color;
	GUI_COLOR DarkColor;
	AlarmType Type;
	Alarm *Next, *Prev;
};

#endif
