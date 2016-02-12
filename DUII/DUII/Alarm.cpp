/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: Alarm.cpp
Description: Alarm class
-----------------------------------------------------------------
*/

#include "Alarm.hpp"

Alarm::Alarm(bool bEnabled, float fMinValue, float fMaxValue, GUI_COLOR cColor)
{
	Alarm(bEnabled, fMinValue, fMaxValue, cColor, cColor, NORMAL, NULL, NULL);
}

Alarm::Alarm(bool bEnabled, float fMinValue, float fMaxValue, GUI_COLOR cColor, GUI_COLOR cDarkColor, AlarmType tType, Alarm *pNext, Alarm *pPrev)
{
	Enabled = bEnabled;
	MinValue = fMinValue;
	MaxValue = fMaxValue;
	Color = cColor;
	DarkColor = cDarkColor;
	Next = pNext;
	Prev = pPrev;
	Type = tType;
}