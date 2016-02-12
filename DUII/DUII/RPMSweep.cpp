/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: RPMSweep.cpp
Description: The RPM Sweep object
-----------------------------------------------------------------
*/

#include "RPMSweep.hpp"
#include "CustomArc.h"
#include <math.h>
#include <stdlib.h>

extern "C" GUI_CONST_STORAGE GUI_BITMAP bmDigitalBarBlueSweep;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmSweepTachOnAsphalt;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmTransSweepTachOnAsphalt;

//#define USE_MATH 1

RPMSweep::RPMSweep(int x, int y, int xsize, int ysize, int MinValue, int MaxValue, int MajorTicRange, int MinorTicRange, Alarm *pAlarmList, bool bShowGear)
{
	xLoc = x;
	yLoc = y;
	xSize = xsize;
	ySize = ysize;
	ShowGear = bShowGear;

	iMax = MaxValue;
	iMin = MinValue;

	iMajorTicRange = MajorTicRange;
	iMinorTicRange = MinorTicRange;

	dValuePerDeg = (150.0 - 86.0)/ ((float)iMax - (float)iMin);

	AlarmList = pAlarmList;

	iXCenter = 453;
	iYCenter = 566;

	CreateBackground();
}

RPMSweep::~RPMSweep()
{
	GUI_MEMDEV_Delete(mMemSpace);
}

void RPMSweep::DrawControl()
{
	// Draw blue sweep in background
	GUI_DrawBitmap(&bmDigitalBarBlueSweep, xLoc, yLoc);

	// black out part of the sweep we don't want to see
	GUI_SetColor(GUI_BLACK);
	GUI_FillPolygon(pts, 4, iXCenter, iYCenter);

	// Draw foreground (scale and asphalt) on top
	GUI_MEMDEV_Write(mMemSpace);

	// draw the gear rectangle - must be done after memdev write
	if(ShowGear)
	{
		// draw the gear number
		GUI_SetColor(GUI_WHITE);
		GUI_SetFont(GEAR_FONT);
		GUI_DispStringInRect(strGear, &rectGear, GUI_TA_HCENTER | GUI_TA_VCENTER);
	}
}

//
// CreateBackground - but in this case it is actually creating the FOREGROUND
// This image will be the asphalt "background" with a transparent hole for
// the blue sweep area.  We then draw the scales and redlines on this image and
// any other static grpahics.
//
// When it is time to draw we first lay down the blue arc, then black out the 
// unused portion and then we lay the foreground on top of it.
//
void RPMSweep::CreateBackground()
{
	int iOuterArcRadius, iInnerArcRadius, iXCenter, iYCenter;
	int i, range, TicCount, iEndAngle, iMultiplier;
	int x1, y1, x2, y2, iTemp = 1, radius, TicLength;
	int iOuterX, iOuterY, iInnerX, iInnerY;
	float dCos, dSin, angle, radians, dValuePerDeg;
	char str[10];
#ifdef USE_MATH
	int iSlope, iYIntersect;
#endif

	// create the foreground memory area
	mMemSpace = GUI_MEMDEV_Create(xLoc, yLoc, xSize, ySize);
	GUI_MEMDEV_Select(mMemSpace);

	// Draw asphalt image with transparant sweep area
	GUI_DrawBitmap(&bmTransSweepTachOnAsphalt, xLoc, yLoc);

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetPenSize(4);
	iOuterArcRadius = ARC_RADIUS;
	iInnerArcRadius = ARC_RADIUS;
	iOuterX = 447;
	iOuterY = 522;
	iInnerX = 460;
	iInnerY = 610;
	GUI_DrawArc(iOuterX, iOuterY, iOuterArcRadius, iOuterArcRadius, 86, 150);
	GUI_DrawArc(iInnerX, iInnerY, iInnerArcRadius, iInnerArcRadius, 88, 140);

	// draw the alarm colors
	dValuePerDeg = (150.0 - 86.0) / (float)(iMax - iMin);

	//iStartAngle = 88;
	iEndAngle = 150;
	iXCenter = 453;
	iYCenter = 566;

	if(AlarmList != NULL)
	{
		Alarm *pAlarm;
		pAlarm = AlarmList;
		float iAlarmStartAngle, iAlarmEndAngle;
		//int temp;

		while(pAlarm != NULL)
		{
			if(pAlarm->Enabled == true)
			{
				iAlarmStartAngle = iEndAngle - (int)(((int)pAlarm->MaxValue - iMin) * dValuePerDeg);
				iAlarmEndAngle = iEndAngle - (int)(((int)pAlarm->MinValue - iMin) * dValuePerDeg);

				// calculate the angle for the proper tic mark
				angle = ceil(iEndAngle - (pAlarm->MaxValue - iMin) * dValuePerDeg);
				radians = DEGREES_TO_RADIANS(angle);
				dCos = cos(radians);
				dSin = sin(radians);
				
				if(angle < 105)
					iTemp = (int)((150.0 - (float)angle) / 3.4);
				else
					iTemp = (int)((150.0 - (float)angle) / 2.75);

				radius = iOuterArcRadius + 30 + iTemp;

				x1 = iXCenter + (int)(dCos * (radius - 5));
				y1 = iYCenter + (int)(dSin * (radius - 5) * -1);

				if (((int)iAlarmStartAngle != 86) && ((int)iAlarmStartAngle != 87))
					iAlarmStartAngle = 180 - RADIANS_TO_DEGREES(atan(((float)abs(y1 - iOuterY) / (float)abs(x1 - iOuterX))));

				angle = ceil(iEndAngle - (pAlarm->MinValue - iMin) * dValuePerDeg);
				radians = DEGREES_TO_RADIANS(angle);
				dCos = cos(radians);
				dSin = sin(radians);
				
				if(angle < 105)
					iTemp = (int)((150.0 - (float)angle) / 3.4);
				else
					iTemp = (int)((150.0 - (float)angle) / 2.75);

				radius = iOuterArcRadius + 30 + iTemp;

				x1 = iXCenter + (int)(dCos * (radius - 5));
				y1 = iYCenter + (int)(dSin * (radius - 5) * -1);

 				iAlarmEndAngle = 180 - RADIANS_TO_DEGREES(atan(((float)abs(y1 - iOuterY) / (float)abs(x1 - iOuterX))));


				if(iAlarmStartAngle > iAlarmEndAngle)
				{
					float temp;
					// GUI_DrawArc won't draw an arc if the start value is greater than the end value for some reason
					// we need to switch them so it'll work
					temp = iAlarmStartAngle;
					iAlarmStartAngle = iAlarmEndAngle;
					iAlarmEndAngle = temp;
				}

				iTemp = ARC_RADIUS - RPMSWEEP_ALARM_WIDTH - 1;
				//iTemp = ARC_RADIUS + 28;
				GUI_SetPenSize(RPMSWEEP_ALARM_WIDTH + 2);
				GUI_SetColor(GUI_BLACK);
				GUI_DrawArc2(iOuterX, iOuterY, iTemp, iTemp, iAlarmStartAngle, iAlarmEndAngle);
				//GUI_DrawArc(iXCenter, iYCenter, iTemp, iTemp, iAlarmStartAngle, iAlarmEndAngle);

				GUI_SetPenSize(RPMSWEEP_ALARM_WIDTH);
				GUI_SetColor(pAlarm->Color);
				GUI_DrawArc2(iOuterX, iOuterY, iTemp, iTemp, iAlarmStartAngle, iAlarmEndAngle);
				//GUI_DrawArc(iXCenter, iYCenter, iTemp, iTemp, iAlarmStartAngle, iAlarmEndAngle);
			}
			pAlarm = pAlarm->Next;
		}
	}

	GUI_SetPenSize(3);

	//iStartAngle = 86;

	range = iMax - iMin;
	TicCount = (int)(range / iMajorTicRange);
	if(TicCount > 9)
	{
		TicCount = (int)ceil((float)TicCount / 2.0);
		iMultiplier = 2;
	}
	else
	{
		iMultiplier = 1;
	}

	//dValuePerDeg = ((float)iEndAngle - (float)iStartAngle) / TicCount;
	//dValuePerDeg = (150 - 86) / TicCount;

	radius = iOuterArcRadius + 30;
	TicLength = 50;

	GUI_SetFont(TEXT_FONT);

	for(i=1; i < TicCount; i++)
	{
		angle = ceil(iEndAngle - ((float)i * dValuePerDeg * iMultiplier * iMajorTicRange));

		radians = DEGREES_TO_RADIANS(angle);

		dCos = cos(radians);
		dSin = sin(radians);

		if(angle < 105)
			iTemp = (int)((150.0 - (float)angle) / 3.4);
		else
			iTemp = (int)((150.0 - (float)angle) / 2.75);


		//radius = iOuterArcRadius + 30 + iTemp;
#ifdef USE_MATH
		radius = iOuterArcRadius * 2;
#else
		radius = iOuterArcRadius + 30 + iTemp;
#endif

// note numbers - first = 8000 rpm, second = 16000 rpm, 3rd = 12000, 4th =10000

		if (angle < 95)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle - 10.0) / (150.0 - 86.0)));	// straight up 86, 86
		else if (angle < 100)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle - 10.0) / (150.0 - 90.0)));	// 90, 90, 90
		else if (angle < 105)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle - 10.0) / (150.0 - 95.0)));	// 95, 102, 95
		else if (angle < 110)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle - 6.0) / (150.0 - 96.0)));	// 96, 96, 86, 96
		else if (angle < 115)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle - 6.0) / (150.0 - 98.0)));	// 98 5/8, 99, 101, 101
		else if (angle < 120)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle - 6.0) / (150.0 - 104.0)));	// 105, 107, 109, 109, 102
		else if (angle < 125)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle + 6.0) / (150.0 - 90.0)));	// 90, 86, 86
		else if (angle < 130)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle + 6.0) / (150.0 - 86.0)));	// 86, 90, 90
		else if (angle < 135)
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle + 6.0) / (150.0 - 90.0)));	// 90, 90, 90
		else
			iTemp = TicLength + (int)(TicLength * ((150.0 - angle + 6.0) / (150.0 - 86.0)));	// 86, 86, 86


		// get the start and end points of the line
		x1 = iXCenter + (int)(dCos * (radius - 5));
		y1 = iYCenter + (int)(dSin * (radius - 5) * -1);

#ifdef USE_MATH
		x2 = iXCenter;
		y2 = iYCenter;
		iSlope = (y1 - y2) / (x1 - x2);
		iYIntersect = iYCenter + (tan(180 - angle) * iXCenter);

		//iTemp = sqrt(SQ(iSlope) * SQ(iOuterArcRadius) - SQ(iYIntersect) + (2 * iXCenter - 2 * iYCenter * iSlope) * iYIntersect - SQ(iXCenter) * SQ(iSlope) + 2 * iXCenter * iYCenter * iSlope - SQ(iYCenter)) + iSlope * iYIntersect - iXCenter * iSlope - iYCenter
		x1 = GetXIntersect(iOuterX, iOuterY, iOuterArcRadius, x1, y1, x2, y2, 180 - angle);
		y1 = iSlope * x1 + iYIntersect;
		x2 = iXCenter;
		y2 = iYCenter;
		//x2 = GetXIntersect(iInnerX, iInnerY, iInnerArcRadius, x1, y1, x2, y2, 180 - angle);
		//y2 = iSlope * x1 + iYIntersect;
#else
		x2 = iXCenter + (int)(dCos * (radius - iTemp));
		y2 = iYCenter + (int)(dSin * (radius - iTemp) * -1);

#endif
		iTemp = GUI_GetPenSize();
		GUI_SetColor(GUI_BLACK);
		GUI_SetPenSize(iTemp + 2);
		GUI_DrawLine(x1, y1, x2, y2);
		GUI_SetColor(GUI_WHITE);
		GUI_SetPenSize(iTemp);
		GUI_DrawLine(x1, y1, x2, y2);
		

		// print the text
		sprintf(str, "%d", i * iMultiplier);
		if(angle < 100)
			y2 -= GUI_GetFontDistY();
		else if (angle < 115)
			y2 -= GUI_GetFontDistY() * 1.15;
		else if (angle < 125)
			y2 -= GUI_GetFontDistY() * 1.25;
		else
			y2 -= GUI_GetFontDistY() * 1.35;

		x2 += GUI_GetStringDistX(str) / 2;
		if (angle > 140)
			x2 -= 12;
		else if (angle > 120)
			x2 -= 7;

		GUI_SetColor(GUI_BLACK);
		GUI_DispStringAt(str, x2 + 1, y2 + 1);
		GUI_SetColor(GUI_WHITE);
		GUI_DispStringAt(str, x2, y2);

		//GUI_SetPenSize(60);
		//GUI_DrawArc(iXCenter, iYCenter, ARC_RADIUS, ARC_RADIUS, angle, angle);
		GUI_SetPenSize(iTemp);
	}

	// Draw the end lines
	angle = 86;
		radians = DEGREES_TO_RADIANS(angle);

		dCos = cos(radians);
		dSin = sin(radians);

	radius = iOuterArcRadius + 30;
		//x1 = iXCenter + (int)(dCos * (radius - 5));
		//y1 = iYCenter + (int)(dSin * (radius - 5) * -1);
		//x2 = iXCenter + (int)(dCos * (radius));
		//y2 = iYCenter + (int)(dSin * (radius) * -1);

		x1 = xLoc + xSize - 2;
		y1 = iInnerY - ARC_RADIUS;
		x2 = x1;
		y2 = iOuterY - ARC_RADIUS;

		//GUI_SetColor(GUI_BLACK);
		//GUI_SetPenSize(iTemp + 2);
		//GUI_DrawLine(x1, y1, x2, y2);
		GUI_SetColor(GUI_WHITE);
		GUI_SetPenSize(iTemp);
		GUI_DrawLine(x1, y1, x2, y2);

	// draw the Gear indicator background
	if(ShowGear)
	{
		GUI_SetColor(GUI_BLACK);
		x1 = xLoc + xSize - 50;
		y1 = yLoc + 15;
		GUI_FillRoundedRect(x1, y1, x1 + GEAR_HEIGHT, y1 + GEAR_WIDTH, 5);

		rectGear.x0 = x1 + 1;
		rectGear.y0 = y1 + 1;
		rectGear.x1 = x1 + GEAR_WIDTH - 2;
		rectGear.y1 = y1 + GEAR_HEIGHT - 2;
		GUI_SetColor(GUI_WHITE);
	}
	GUI_SetPenSize(1);
	GUI_SelectLCD();
}

void RPMSweep::SetValue(float dValue)
{
	Value = dValue;

	angle = 60 - (Value * dValuePerDeg);
	radians = DEGREES_TO_RADIANS(angle);
	
	dTan = tan(radians);

	pts[0].x = 0;
	pts[0].y = 0;
	pts[1].x = 23;
	pts[1].y = 0;
	pts[2].x = 23;
	pts[2].y = -566;
	pts[3].x = -1 * (int)(dTan * (float)iYCenter);
	pts[3].y = -566;
}

int RPMSweep::GetPixelsFromValue(float dValue)
{
	int iPixels;
	float dValuePerPix;

	dValuePerPix = (iMax - iMin) / RPMSWEEP_PIXELS;
	iPixels = (int)(dValue / dValuePerPix);

	return iPixels;
}


void RPMSweep::SetGear(int iGear)
{
	Gear = iGear;
	if (Gear > 0)
		sprintf(strGear, "%1.0d", Gear);
	else
		strGear[0] = '\0';
}

int RPMSweep::GetXIntersect(float iXCenter, float iYCenter, float Radius, int x1, int y1, int x2, int y2, int angle)
{
	int x;
	float Slope, radians, iYIntersect, dPart1, dPart2;
	Slope = ((float)y1 - (float)y2) / ((float)x1 - (float)x2);
	radians = DEGREES_TO_RADIANS(angle);
	iYIntersect = y2 - (tan(radians) * x2);
 	//iTemp = SQ(Radius);

	//iTemp = sqrt((float)((SQ(Slope) + 1) * SQ(iRadius) - SQ(iYIntersect) + (2 * iYCenter - iXCenter * Slope) * iYIntersect - SQ(iXCenter) * SQ(Slope) + 2 * iXCenter * iYCenter * Slope - SQ(iYCenter))) + Slope * (iYIntersect - iXCenter) - iYCenter;
	//dPart1 = (SQ(Slope) + 1) * SQ(iRadius) - SQ(iXCenter) * SQ(Slope) + (2 * iXCenter * iYCenter - 2 * iYIntersect * iXCenter) * Slope - SQ(iYCenter) + 2 * iYIntersect * iYCenter - SQ(Slope) ;
	//dPart2 = sqrt(dPart1) + (iYIntersect - iYCenter) * Slope - iXCenter;
	//dPart2 = sqrt(dPart1) + (iYCenter - iYIntersect) * Slope + iXCenter;

	dPart1 = -1.0 * SQ(iYIntersect) + (2.0 * iYCenter - 2 * Slope * iXCenter) * iYIntersect - SQ(iYCenter) + 2.0 * Slope * iXCenter * iYCenter - SQ(Slope) * SQ(iXCenter) + SQ(Radius) * SQ(Slope) + SQ(Radius);
	dPart2 = sqrt(dPart1) + Slope * iYIntersect - Slope * iYCenter - iXCenter;
	
	x = (int)(dPart2 / (SQ(Slope) + 1.0));
	return x;
}