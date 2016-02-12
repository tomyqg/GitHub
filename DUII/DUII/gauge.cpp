/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: gauge.cpp
Description: The gauge object
-----------------------------------------------------------------
*/
#include "gauge.hpp"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "GeneralConfig.h"

//
// KMC - 9/15/12 - modified to accept input pointer to graphic to allow us to easily
// alter the background of the gauge screen.  Note that positioning of the redline and
// scale still depends on the image having the same proportions.
//
Gauge::Gauge(int x, int y, int Radius, float MinValue, float MaxValue, char *Name, char *Units, char *pInputNum, Alarm *AlarmList,
				 float MajorTicRange, float MinorTicRange, int Precision, int Multiplier, char showMultiplier,
				 WM_HWIN _hParent, const GUI_BITMAP *_pBitmap) : ControlBase(x, y, Name, Units, AlarmList)
{
	//float radians;

	if ((_hDlg = WM_CreateWindowAsChild(x, y, Radius * 2, Radius * 2, _hParent, 0, NULL, 0)) <= 0)
		SystemError();

//	WM_EnableMemdev(_hDlg);

	xLoc = x;
	yLoc = y;
	pBitmap = _pBitmap;
	//bRedlineEnabled = RedlineEnabled;
	//iRedline = RedlineValue;
	//bYellowLineEnabled = YellowLineEnabled;
	//iYellowLine = YellowLineValue;

	iRadius = Radius;
	iXCenter = xLoc + Radius;
	iYCenter = yLoc + Radius;

	// round the min and max values to the next whole integer
	if (MinValue < MaxValue)
	{
		iMin = (int)(MinValue - 0.5);
		iMax = (int)(MaxValue + 0.5);
	}
	else
	{
		iMin = (int)(MinValue + 0.5);
		iMax = (int)(MaxValue - 0.5);
	}

	iMultiplier = Multiplier;
	iShowMultiplier = showMultiplier;
	if(iMultiplier < 1)
		iMultiplier = 1;

	SetPrecision(Precision);
	//iPrecision = 2;

	iMajorTics = MajorTicRange;
	iMinorTics = MinorTicRange;

	sName = Name;
	sUnits = Units;
	sInputNum = pInputNum;

	iStartingAngle = GAUGE_START_ANGLE;
	iEndAngle = GAUGE_END_ANGLE;

	mMemSpace = -1;

	dZeroRad = DEGREES_TO_RADIANS(iEndAngle);
	//radians = (float)DEGREES_TO_RADIANS(iEndAngle - iStartingAngle);
	//dValuePerRad =  (float)(iMax - iMin) / radians;
	dValuePerRad = (float)(iMax - iMin) / (float)(iEndAngle - iStartingAngle);

	Value = iMin;

	bShowDigital = true;

	CreateGaugeBackground();

	// create the needle memory area
	memGauge = GUI_MEMDEV_Create(xLoc, yLoc, iRadius * 2, iRadius * 2);
}

/****************************************************************
*
*       ~Gauge()
*		The Destructor
*
******************************************************************/
Gauge::~Gauge()
{
	GUI_MEMDEV_Delete(mMemSpace);
	GUI_MEMDEV_Delete(memGauge);
}

/****************************************************************
*
*       CreateGaugeBackground()
*		Draw the gauge except for the needle into a memory space
*
******************************************************************/
void Gauge::CreateGaugeBackground()
{
	int i, TicCount, value, MinorTicCount, TicLength, iTemp, radius;
	int x1, y1, x2, y2;
	int txt_length, txt_height, /* iRedlineWidth, */ iNeedleWidth;
	float dCos, dSin, angle, radians, dTemp, Mag, dValuePerDeg, range;
	char str[20], strPrecision[20];
	GUI_COLOR OldColor;
	//GUI_RECT rect;

	if(mMemSpace > 0)
		GUI_MEMDEV_Delete(mMemSpace);

	mMemSpace = GUI_MEMDEV_Create(xLoc, yLoc, iRadius * 2, iRadius * 2);
	GUI_MEMDEV_Select(mMemSpace);

	//GUI_SetTextMode(GUI_TEXTMODE_TRANS | GUI_TM_XOR);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetColor(GUI_WHITE);
	GUI_SetPenSize(1);

	radius = (int)(iRadius - iRadius * 0.15);
	Mag = 1000.0;
	iTemp = (int)((Mag / 1000) * iRadius);
	x1 = iXCenter - iTemp;
	iTemp = (int)((Mag / 1000) * iRadius);
	y1 = iYCenter - iTemp;
	if (pBitmap)
		GUI_DrawBitmapEx(pBitmap, x1, y1, 0, 0, (int)Mag, (int)Mag);

	dValuePerDeg = ((float)iEndAngle - (float)iStartingAngle) / ((float)iMax - (float)iMin);

	// check for alarm conditions
	if(AlarmList != NULL)
	{
		Alarm *pAlarm;
		pAlarm = AlarmList;
		int iAlarmStartAngle, iAlarmEndAngle, temp, iArcRadius;
		float kmcMag;

		OldColor = GUI_GetColor();

		GUI_SetPenSize(GAUGE_REDLINE_WIDTH);
		while(pAlarm != NULL)
		{
			if(pAlarm->Enabled == true)
			{
				iAlarmStartAngle = iEndAngle - (int)((pAlarm->MaxValue - iMin) * dValuePerDeg);
				iAlarmEndAngle = iEndAngle - (int)((pAlarm->MinValue - iMin) * dValuePerDeg);
				if(iAlarmStartAngle > iAlarmEndAngle)
				{
					// GUI_DrawArc won't draw an arc if the start value is greater than the end value for some reason
					// we need to switch them so it'll work
					temp = iAlarmStartAngle;
					iAlarmStartAngle = iAlarmEndAngle;
					iAlarmEndAngle = temp;
				}

	// KMC - REFERENCE image - the image from which measurements have been taken - is 272 pixels on a side.
	// Mag is supposed to refer to the magnification of the image BUT we use two different images!  Therefore
	// these fixed offsets should be scaled based on the ratio of the actual image size to the reference in
	// addition to the scaling of this image relative to it's own native image
				kmcMag = (float)(iRadius * 2) / 272.0;

				// draw the dark part of the arc so it fits with the gauge
				GUI_SetPenSize((int)(GAUGE_DARK_REDLINE_ARC_WIDTH * (Mag / 1000)));
				GUI_SetColor(pAlarm->DarkColor);
				iArcRadius = (int)(GAUGE_DARK_REDLINE_ARC_RADIUS * kmcMag * (Mag / 1000));
				GUI_DrawArc(iXCenter, iYCenter, iArcRadius, iArcRadius, iAlarmStartAngle, iAlarmEndAngle);

				// draw the main part of the arc
				GUI_SetPenSize((int)(GAUGE_REDLINE_ARC_WIDTH * (Mag / 1000)));
				GUI_SetColor(pAlarm->Color);
				GUI_DrawArc(iXCenter, iYCenter, iArcRadius - ((GAUGE_REDLINE_ARC_WIDTH + GAUGE_DARK_REDLINE_ARC_WIDTH) / 2), iArcRadius - ((GAUGE_REDLINE_ARC_WIDTH + GAUGE_DARK_REDLINE_ARC_WIDTH) / 2), iAlarmStartAngle, iAlarmEndAngle);
			}
			pAlarm = pAlarm->Next;
		}
		GUI_SetPenSize(1);
		GUI_SetColor(OldColor);
	}

	// draw the tic marks
// KMC	range = (int) iMax - (int) iMin;
	range = iMax - iMin;

//KMC	TicCount = (int)(range / iMinorTics);
	TicCount = (int)((range / iMinorTics) + 0.500001);		// Add a tiny bit more to account for floating point round-off error ((int)9.9999999 = 9, not 10 without 'adjustment')
	MinorTicCount = (int)(iMajorTics / iMinorTics);

	//interval = (int)(((float)iEndAngle - (float)iStartingAngle) / ((float)TicCount));
	GUI_SetFont(GAUGE_SMALL_FONT);

	for(i = 0; i < TicCount + 1; i++)
	{
		angle = iEndAngle - (int)(i * iMinorTics) * dValuePerDeg;

		radians = DEGREES_TO_RADIANS(angle);

		dCos = cos(radians);
		dSin = sin(radians);

		if(i % MinorTicCount == 0)
		{
			GUI_SetPenSize(3);
			TicLength = (int)(iRadius * 0.10);
			if(TicLength < GAUGE_MAJOR_TIC_LENGTH)
				TicLength = GAUGE_MAJOR_TIC_LENGTH;
		}
		else
		{
			GUI_SetPenSize(1);
			TicLength = (int)(radius * 0.07);
			if(TicLength < GAUGE_MINOR_TIC_LENGTH)
				TicLength = GAUGE_MINOR_TIC_LENGTH;	
		}

		x1 = iXCenter + (int)(dCos * radius);
		y1 = iYCenter + (int)(dSin * radius * -1);
		x2 = iXCenter + (int)(dCos * (radius - TicLength));
		y2 = iYCenter + (int)(dSin * (radius - TicLength) * -1);
		
		iTemp = GUI_GetPenSize();
		GUI_SetColor(GUI_BLACK);
		GUI_SetPenSize(iTemp + 2);
		GUI_DrawLine(x1, y1, x2, y2);
		GUI_SetColor(GUI_WHITE);
		GUI_SetPenSize(iTemp);
		GUI_DrawLine(x1, y1, x2, y2);

		if(i % MinorTicCount == 0)
		{
			value = (int) (iMin + iMinorTics * i);
			sprintf(str, "%d", value / iMultiplier);
			txt_length = GUI_GetStringDistX(str);
			txt_height = GUI_GetFontDistY();

			// draw the tic text
			dTemp = ((float)radius - (float)TicLength - ((float)radius / (float)6));
			x1 = iXCenter + (int)(dCos * dTemp);
			y1 = iYCenter + (int)(dSin * dTemp * -1);

			// adjust for the text size
			x1 -= txt_length / 2;
			y1 -= txt_height / 2;

			GUI_SetColor(GUI_BLACK);
			GUI_DispStringAt(str, x1 + 1, y1 + 1);
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt(str, x1, y1);
		}
	}

	// calculate the needle polygon pointing straight down
	iNeedleWidth = radius / 20;
	if(iNeedleWidth < GAUGE_MIN_NEEDLE_WIDTH)
		iNeedleWidth = GAUGE_MIN_NEEDLE_WIDTH;

	if(iNeedleWidth > GAUGE_MAX_NEEDLE_WIDTH)
		iNeedleWidth = GAUGE_MAX_NEEDLE_WIDTH;

	iTemp = -1 * radius / 15;
	if(iTemp > -10) iTemp = -10;

	ptNeedle[0].x = 0;
	ptNeedle[0].y = iTemp;
	ptNeedle[1].x = ptNeedle[0].x - iNeedleWidth;
	ptNeedle[1].y = 0;
	ptNeedle[2].x = ptNeedle[0].x - (int)(iNeedleWidth / 2);
	ptNeedle[2].y = radius - 40;
	ptNeedle[3].x = 0;
	ptNeedle[3].y = radius - 15;
	ptNeedle[4].x = ptNeedle[0].x + (int)(iNeedleWidth / 2);
	ptNeedle[4].y = radius - 40;
	ptNeedle[5].x = ptNeedle[0].x + iNeedleWidth;
	ptNeedle[5].y = 0;

	GUI_SetFont(GAUGE_DIGITAL_FONT);
	// create the digital output if necessary
	if(bShowDigital == true)
	{
		float fMax;
		fMax = (float)iMax;
		if (iMax > 100)
			strcpy (strPrecision, "%.0f");
		else if (iMax > 10)
			strcpy (strPrecision, "%.1f");
		else
			strcpy (strPrecision, "%.2f");
		
		sprintf(str, strPrecision, fMax);
		txt_length = (int)(GUI_GetStringDistX(str) * 2.5);
		txt_height = GUI_GetFontDistY();

		iMaxTextLength = txt_length;

		// Calculate position of digital insert
		// save the digital rectangle so we can use it later for alarm conditions
		if (pBitmap == &bmDefaultGauge)
		{
			x1 = (GAUGE_DIGITAL_X - txt_length / 4) + 20;
			y1 = (GAUGE_DIGITAL_Y) + 12;
			x2 = (GAUGE_DIGITAL_X + txt_length / 4) + 20;
			y2 = y1 + txt_height + 2;

			DigitalRect.x0 = x1 + 1;
			DigitalRect.y0 = y1 + 1;
			DigitalRect.x1 = x2 - 1;
			DigitalRect.y1 = y2 - 1;
		}
		else
		{
			x1 = GAUGE_DIGITAL_X - txt_length / 4 - 10;
			y1 = GAUGE_DIGITAL_Y;
			x2 = GAUGE_DIGITAL_X + txt_length / 4;
			y2 = y1 + txt_height + 2;

			DigitalRect.x0 = x1 + 1;
			DigitalRect.y0 = y1 + 1;
			DigitalRect.x1 = x2 - 1;
			DigitalRect.y1 = y2 - 1;
		}

		GUI_DrawRoundedRect(x1, y1, x2, y2, GAUGE_DIGITAL_BOX_RADIUS);

		ptDigital.x = x2 - 5;
		ptDigital.y = y1 + 1;
	}

	
	GUI_SetFont(GAUGE_SMALL_FONT_BOLD);
	//if(iMultiplier > 1)
	//{
	//	sprintf(str, "x%d", iMultiplier);
	//	GUI_DispStringAt(str, DigitalRect.x0 + 5, ptDigital.y + txt_height + 5);
	//}
	if(iMultiplier > 1)
	{
		sprintf(str, "%s x%d", sUnits, iMultiplier);
	}
	else
	{
		strcpy(str, sUnits);
	}

	txt_length = GUI_GetStringDistX(str);
	txt_height = GUI_GetFontDistY();
	x2 = iXCenter - txt_length / 2;
	y2 = iYCenter + txt_height * 2 + 4;

	GUI_SetColor(GUI_WHITE);
	//GUI_DispStringAt(str, x2, y2);
// KMC - only display string if requested
	if (iShowMultiplier)
		GUI_DispStringAt(str, DigitalRect.x0 + 5, ptDigital.y + txt_height + 10);

	iTemp = txt_height;

	//GUI_SetFont(GAUGE_SMALL_FONT);
	// display the text for the title and the units
	txt_length = GUI_GetStringDistX(sName);
	txt_height = GUI_GetFontDistY();
	x1 = iXCenter - txt_length / 2;
	y1 = (int)(iYCenter - txt_height * 1.75 - 2);
	
// KMC - only dislpay string if provided
	if (sName)
		GUI_DispStringAt(sName, x1, y1);
	if (sInputNum[0])
		GUI_DispStringHCenterAt(sInputNum, iXCenter, 170);

	GUI_SetPenSize(1);
	GUI_SelectLCD();
}

/****************************************************************
*
*       SetValue(float value)
*		Set the value of the gauge to the value specified
*
******************************************************************/
void Gauge::SetValue(float dValue)
{
	Value = dValue;
}

/****************************************************************
*
*       ShowDigital(bool Show)
*		set the gauge to show the digital output as part of the gauge
*
******************************************************************/
void Gauge::ShowDigital(bool Show)
{
	bShowDigital = Show;
	// we change what we show so we have to redo the background
	CreateGaugeBackground();
}

float Gauge::GetValue()
{
	return Value;
}

void Gauge::SetPrecision(int Precision)
{
	iPrecision = Precision;
}

/****************************************************************
*
*       DrawGauge()
*		Draw the gauge
*
******************************************************************/
void Gauge::DrawControl()
{
	CommonDraw(false);
}

GUI_MEMDEV_Handle Gauge::DrawToMemArea()
{
	CommonDraw(true);
	return memGauge;
}

void Gauge::CommonDraw(bool toMemArea)
{
	float radians, dValue, degrees, dSin, dCos, dVal;
	int x1, y1, x2, y2;
	int txt_width;
	char str[10], strFormat[5];
	Alarm *pAlarm;

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	if (toMemArea)
		GUI_MEMDEV_Select(memGauge);

	// draw the background
	GUI_MEMDEV_Write(mMemSpace);

	// draw the digital component
	if (bShowDigital == true)
	{
		GUI_COLOR BkColor, Color;

		GUI_SetFont(GAUGE_DIGITAL_FONT);
		
		if (iMax > 100)
			strcpy (strFormat, "%.0f");
		else if (iMax > 10)
			strcpy (strFormat, "%.1f");
		else
			strcpy (strFormat, "%.2f");

		sprintf(str, strFormat, Value);
		txt_width = GUI_GetStringDistX(str);
		
		x1 = ptDigital.x - txt_width;
		y1 = ptDigital.y;

		BkColor = GUI_BLACK;
		Color = GUI_WHITE;

		if(AlarmList != NULL)
		{
			pAlarm = AlarmList;
			while(pAlarm != NULL)
			{
				if(pAlarm->Enabled == true)
				{
					if((Value >= pAlarm->MinValue && Value <= pAlarm->MaxValue) || (pAlarm->Type == HIGH && Value > pAlarm->MaxValue) || (pAlarm->Type == LOW && Value < pAlarm->MinValue))
					{
						// we have an alarm condition.  set the color
						BkColor = pAlarm->Color;
						Color = GUI_BLACK;
						break;
					}
					else
					{
						BkColor = GUI_BLACK;
						Color = GUI_WHITE;
					}
				}
				else
				{
					BkColor = GUI_BLACK;
					Color = GUI_WHITE;
				}
				pAlarm = pAlarm->Next;
			}
		}
		GUI_SetColor(BkColor);
		GUI_FillRoundedRect(DigitalRect.x0, DigitalRect.y0, DigitalRect.x1, DigitalRect.y1, GAUGE_DIGITAL_BOX_RADIUS);
		GUI_SetColor(Color);
		GUI_DispStringAt(str, x1, y1 + 1);
	}

	GUI_SetColor(GUI_WHITE);
	// draw the needle
	dVal = Value / iMultiplier;

	// limit the needle movements so we don't loop around one way or the other
	if(dVal < (iMin - iMinorTics))
		dValue = iMin - iMinorTics;
	else if (dVal > (iMax + iMinorTics))
		dValue = iMax + iMinorTics;
	else
		dValue = Value;

	degrees = ((float)iEndAngle - (((dValue - iMin) / dValuePerRad)));
	radians = DEGREES_TO_RADIANS(degrees);

	dCos = cos(radians);
	dSin = sin(radians);

	GUI_SetColor(GUI_BLACK);
	GUI_SetPenSize(GAUGE_NEEDLE_WIDTH + 2);
	x1 = iXCenter + (int)(dCos * (GAUGE_NEEDLE_LENGTH + 1));
	y1 = iYCenter + (int)(dSin * (GAUGE_NEEDLE_LENGTH + 1) * -1);
	x2 = iXCenter;
	y2 = iYCenter;
	GUI_DrawLine(x1, y1, x2, y2);

	GUI_SetPenSize(GAUGE_NEEDLE_WIDTH);
	GUI_SetColor(GAUGE_NEEDLE_COLOR);
	x1 = iXCenter + (int)(dCos * (GAUGE_NEEDLE_LENGTH));
	y1 = iYCenter + (int)(dSin * (GAUGE_NEEDLE_LENGTH) * -1);
	GUI_DrawLine(x1, y1, x2, y2);

	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_FillCircle(iXCenter, iYCenter, GAUGE_NEEDLE_CENTER_RADIUS);
	
	if (toMemArea)
		GUI_SelectLCD();
}

