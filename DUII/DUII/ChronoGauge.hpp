/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: ChronoGauge.hpp
Description: Chrono gauge object header file
-----------------------------------------------------------------
*/
#ifndef CHRONOGAUGE_HPP
#define CHRONOGAUGE_HPP

#include "gauge.hpp"

class ChronoGauge : public Gauge {
public:
	ChronoGauge(int x, int y, int Radius, float MinValue, float MaxValue, char *Name, char *Units, char *pInputNum, Alarm *AlarmList,
				float MajorTicRange, float MinorTicRange, int Precision, int Multiplier, char showMultiplier, 
				WM_HWIN _hParent, const GUI_BITMAP *pBitmap);
	~ChronoGauge();
void SetValue(float dValue);
private:
	float holdValue;
	int holdTime;
};


#endif