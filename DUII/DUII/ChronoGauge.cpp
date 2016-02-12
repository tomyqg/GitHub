/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: ChronoGauge.cpp
Description: The Chrono Gauge object
-----------------------------------------------------------------
*/
#include "ChronoGauge.hpp"

//
// KMC - 9/15/12 - modified to accept input pointer to graphic to allow us to easily
// alter the background of the gauge screen.  Note that positioning of the redline and
// scale still depends on the image having the same proportions.
//
	float holdValue;
	int holdTime;

ChronoGauge::ChronoGauge(int x, int y, int Radius, float MinValue, float MaxValue, char *Name, char *Units, char *pInputNum, Alarm *AlarmList,
				 float MajorTicRange, float MinorTicRange, int Precision, int Multiplier, char showMultiplier,
				 WM_HWIN _hParent, const GUI_BITMAP *_pBitmap) : Gauge(x, y, Radius, MinValue, MaxValue, Name, Units, pInputNum, AlarmList,
				 MajorTicRange, MinorTicRange, Precision, Multiplier, showMultiplier,
				 _hParent, _pBitmap)
{
	 holdValue = 0;
	 holdTime = 0;
}

ChronoGauge::~ChronoGauge()
{
}

/****************************************************************
*
*       SetValue(float value)
*		Set the value of the gauge to the value specified
*
******************************************************************/
#define	HOLD_TIME	500

void ChronoGauge::SetValue(float dValue)
{
//	Value = dValue;
	if (dValue < holdValue)			// if value is lower than our hold point
	{
		if (holdTime == 0)			// if this is the first decreasing value
		{
			holdTime = GUI_GetTime();				// start hold time
		}
		else if ((GUI_GetTime() - holdTime) > HOLD_TIME)	// If it has been falling for HOLD_TIME
		{
			Gauge::SetValue(holdValue = dValue);	// update to new value
			holdTime = GUI_GetTime();				// and restart hold time
		}
		// otherwise keep waiting
	}
	else								// else value is rising - update display normally
	{
		Gauge::SetValue(holdValue = dValue);
		holdTime = 0;
	}
}

