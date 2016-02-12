/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: ControlBase.hpp
Description: The base class for each display control.  gauge, digital display, etc
-----------------------------------------------------------------
*/

#ifndef CTRL_BASE
#define CTRL_BASE

#include "Alarm.hpp"

class ControlBase
{
public:
	ControlBase(int X, int Y, char *Name, char *Units, Alarm *aAlarmList);
	void DrawControl();
	void SetValue(float dValue);
	float GetValue();
protected:
	float Value;
	int x, y;
	char sName[25], sUnits[25];
	Alarm *AlarmList;
};

#endif
