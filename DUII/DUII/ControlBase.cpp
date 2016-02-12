/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: ControlBase.cpp
Description: The base class for each display control.  gauge, digital display, etc
-----------------------------------------------------------------
*/

#include "ControlBase.hpp"
#include <string.h>

ControlBase::ControlBase(int X, int Y, char *Name, char *Units, Alarm *aAlarmList)
{
	x = X;
	y = Y;
	strcpy(sName, Name);
	strcpy(sUnits, Units);
	AlarmList = aAlarmList;
}

void ControlBase::SetValue(float dValue)
{
	Value = dValue;
}

float ControlBase::GetValue()
{
	return Value;
}

