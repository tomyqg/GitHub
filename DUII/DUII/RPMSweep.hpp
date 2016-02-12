/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: RPMSweep.hpp
Description: The RPM Sweep object header file
-----------------------------------------------------------------
*/
#ifndef RPM_SWEEP_HPP
#define RPM_SWEEP_HPP

#include "GUI.h"
#include "WM.h"
#include <stdio.h>
#include <string.h>
#include "alarm.hpp"

#define RPMSWEEP_PIXELS 585

#define TEXT_FONT &GUI_Font24_ASCII
#define GEAR_FONT &GUI_FontD32

#define GEAR_HEIGHT 40
#define GEAR_WIDTH GEAR_HEIGHT

#define ARC_RADIUS 520

#define RPMSWEEP_ALARM_WIDTH 7

#define SQ(x) x * x

class RPMSweep {
public:
	RPMSweep(int x, int y, int xsize, int ysize, int MinValue, int MaxValue, int MajorTicRange, int MinorTicRange, Alarm *pAlarmList, bool bShowGear);
	~RPMSweep();
	void DrawControl();
	void SetValue(float dValue);
	void SetGear(int iGear);

	int iMax;  // The max value to display
	int iMin;  // the min value to display
	float Value;

	void GetRect(GUI_RECT *rect)
	{
		rect->x0 = xLoc;
		rect->y0 = yLoc;
		rect->x1 = xLoc + xSize;
		rect->y1 = yLoc + ySize;
	}

private:
	void CreateBackground();
	int GetPixelsFromValue(float dValue);
	int GetXIntersect(float iXCenter, float iYCenter, float iRadius, int x1, int y1, int x2, int y2, int angle);
/****************************************************************
*
*       class variables
*
******************************************************************/
	int iMajorTicRange, iMinorTicRange; // the ranges between the major and minor tics
	int xLoc, yLoc;  // the coordinates of the upper left corner
	int xSize, ySize; // the width and height

	char *sName, *sUnits; // the name and units

	bool bYellowLineEnabled; // is the yellowline enabled
	int iYellowLine;  // the value the yellow line starts
	bool bRedLineEnabled; // is the red line enabled
	int iRedLine; // the value the redline starts

	int iYellowLinePix; // the pixels where the yellow line starts
	int iRedLinePix; // the pixels where the red line starts
	int Gear;  // the gear to be displayed
	float dValuePerDeg;
	bool ShowGear;
	char strGear[2];
	int iXCenter, iYCenter;
    GUI_POINT pts[4];
	GUI_RECT rectGear;
	float dTan;
	float angle, radians;

 	Alarm *AlarmList;

	GUI_MEMDEV_Handle mMemSpace;

};
#endif