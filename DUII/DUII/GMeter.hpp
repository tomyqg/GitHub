/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: GMeter.hpp
Description: gauge object header file
-----------------------------------------------------------------
*/
#ifndef GMETER_HPP
#define GMETER_HPP

#include "GUI.h"
#include "WM.h"
#include <stdio.h>
#include "LCDConf.h"

#define GMETER_MARKER_DIA 8
#define GMETER_BASE_COLOR GUI_GREEN
#define GMETER_YELLOW_COLOR GUI_YELLOW
#define GMETER_RED_COLOR GUI_RED

#define GMETER_CIRCLE_X (XSIZE_PHYS / 2) - GMETER_RADIUS
#define GMETER_CIRCLE_Y 10
#define GMETER_RADIUS	125

class GMeter {
public:
	GMeter(int iMaxG);
	~GMeter();
	void DrawControl();
	void SetValue(float dXValue, float dYValue);
	void ResetMaxs(void);

	int MaxG;
	float xValue, yValue;
private:
	void CreateBackground();
	void DrawThing(int x, int y);

	int x, y;  // top left RELATIVE location
	int new_x1, new_y1;
	int xCenter, yCenter;  // the center of the control
	int Radius;
	float dPixelsPerG;
	
	float xMaxValue, yMaxValue; // the peak values seen
	float xMinValue, yMinValue; // the low value seen
	float totalG;

	GUI_MEMDEV_Handle mMemBackground;
	GUI_MEMDEV_Handle mem;			// KMC
//	WM_HTIMER timer;
};

#endif