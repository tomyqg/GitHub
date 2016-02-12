/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: gauge.hpp
Description: gauge object header file
-----------------------------------------------------------------
*/
#ifndef GAUGE_HPP
#define GAUGE_HPP

#include "DUII.h"
#include "ControlBase.hpp"
#include "Alarm.hpp"
#include "DigitalDisplay.hpp"
//#include <string.h>

//#define DEGREES_TO_RADIANS(x) x * 3.141592654 / 180
//#define RADIANS_TO_DEGREES(x) x * 180 / 3.141592654

const GUI_RECT PeakRPMRect = { 410, 210, 477, 234 };
const GUI_RECT RPMRect =	 { 410, 147, 477, 171 };

#define GAUGE_MAJOR_TIC_LENGTH 15
#define GAUGE_MINOR_TIC_LENGTH 12

#define GAUGE_DIGITAL_FONT &GUI_Font20_ASCII
#define GAUGE_LARGE_FONT &GUI_Font24_ASCII
#define GAUGE_SMALL_FONT &GUI_Font24B_ASCII
//#define GAUGE_SMALL_FONT GAUGE_LARGE_FONT
#define GAUGE_SMALL_FONT_BOLD &GUI_Font16B_ASCII

#define GAUGE_NEEDLE_POINT_COUNT  6
#define GAUGE_MIN_NEEDLE_WIDTH 5
#define GAUGE_MAX_NEEDLE_WIDTH 10
#define GAUGE_NEEDLE_COLOR GUI_RED

#define GAUGE_REDLINE_WIDTH 10

#define GAUGE_START_ANGLE 45
#define GAUGE_END_ANGLE 270

#define GAUGE_NEEDLE_WIDTH 4
#define GAUGE_NEEDLE_LENGTH iRadius - iRadius * 0.2
#define GAUGE_NEEDLE_CENTER_RADIUS 5

//#define GAUGE_DIGITAL_X iXCenter
//#define GAUGE_DIGITAL_Y iYCenter + radius / 5
#define GAUGE_DIGITAL_X iXCenter + 2 * radius / 3
#define GAUGE_DIGITAL_Y iYCenter

#define GAUGE_DARK_REDLINE_ARC_WIDTH 9
#define GAUGE_REDLINE_ARC_WIDTH 25
// KMC TEST #define GAUGE_DARK_REDLINE_ARC_RADIUS 81
#define GAUGE_DARK_REDLINE_ARC_RADIUS 97
// KMC TEST #define GAUGE_REDLINE_ARC_RADIUS 65
#define GAUGE_REDLINE_ARC_RADIUS 81

#define GAUGE_DIGITAL_BOX_RADIUS 5

extern "C" GUI_CONST_STORAGE GUI_BITMAP bmDefaultGauge;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmSmallGauge;

class Gauge : public ControlBase {
public:
	Gauge(int x, int y, int Radius, float MinValue, float MaxValue, char *Name, char *Units, char *pInputNum, Alarm *AlarmList,
				float MajorTicRange, float MinorTicRange, int Precision, int Multiplier, char showMultiplier, 
				WM_HWIN _hParent, const GUI_BITMAP *pBitmap);
	~Gauge();
	void DrawControl();
	GUI_MEMDEV_Handle DrawToMemArea();
	void SetValue(float Value);
	float GetValue();
	void ShowDigital(bool Show);
	void SetPrecision(int Precision);

//KMC	int iMax;  // The max value the gauge displays
//KMC	int iMin;  // the min value the gauge displays
	float iMax;  // The max value the gauge displays
	float iMin;  // the min value the gauge displays

	GUI_MEMDEV_Handle mMemSpace;
	void GetRect(GUI_RECT *rect)
	{
		rect->x0 = xLoc;
		rect->y0 = yLoc;
		rect->x1 = xLoc + (iRadius << 1);
		rect->y1 = yLoc + (iRadius << 1);
	}

private:
	void CreateGaugeBackground();
	void _cbCallback(WM_MESSAGE * pMsg);
	void CommonDraw(bool toMemArea);

/****************************************************************
*
*       class variables
*
******************************************************************/
	//int iValue;
	int iStepValue;
	//int iRedline;  // the value the redline starts at
	char *sName;
	char *sUnits;
	char *sPrecision;
	char *sInputNum;

	int iType; // gauge type.
	//bool bRedlineEnabled; // show the redline
	float iMajorTics; // range between major divisions
	float iMinorTics; // range between minor ticks
	int xLoc, yLoc;  // the RELATIVE coordinates of the upper left of the gauge rectangle
	int iXCenter, iYCenter; // the coordinates of the center of the gauge
	int iRadius;  // the radius of the gauge
	int iStartingAngle; // the angle the gauge starts at
	int iEndAngle; // the angle the gauge ends at
	int iTics; // number of tic marks
	int iInterval; // the interval between tic marks
	int iPrecision; // the precision of the digital display
	int iMaxTextLength; // the max length of the text in the digital area
	int iMultiplier; // the value to multiply the gauge value by to get the real value
	char iShowMultiplier;	// flag indicating if scale multiplier should be shown or not

	//bool bYellowLineEnabled; // show the yellow line
	//int iYellowLine; // value to start the yellow line
	bool bShowDigital; // wether or not to show the digital output as part of the gauge
	Point ptDigital; // the point to write the digital text so we don't have to calculate it every time.

	float dValuePerRad; // the value of the gauge to move one rad
	float dZeroRad; // the radian value for the zero on the gauge
	float Value;
	const GUI_BITMAP *pBitmap;	// Pointer to the bitmap gauge image to use

	GUI_POINT ptNeedle[GAUGE_NEEDLE_POINT_COUNT]; // the points for the needle polygon

	GUI_MEMDEV_Handle memGauge;

	GUI_POINT ptN[GAUGE_NEEDLE_POINT_COUNT];
	GUI_RECT DigitalRect;
	WM_HWIN _hDlg;
};

#endif