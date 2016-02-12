/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: LinearGauge.cpp
Description: The linear gauge object
-----------------------------------------------------------------
*/
#include "LinearGauge.hpp"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "GeneralConfig.h"

extern GUI_BITMAP bmGreenPointer;

#define GUI_MEDDARKGREEN	0x0000B000
#define GUI_MEDPLUSGREEN	0x00009000
#define	GUI_MEDDARKRED		0x000000C0
#define	GUI_MEDPLUSRED		0x000000A0
#define	GUI_MEDGRAY			0x00707070
#define GUI_DARKGREEN2		0x00006000
#define	BOX_START_Y		30
#define	BOX_HEIGHT		35
#define BORDER_WIDTH	2
#define	CIRCLE_RADIUS	25
#define	CIRCLE_Y_OFFSET	-6
#define	Y_TEXT_OFFSET	4


LinearGauge::LinearGauge(int x, int y, int xSize, int ySize, float inMinValue, float inMaxValue, 
	int inUseLowerRed, float inLowerRed, int inUseUpperRed, float inUpperRed, char *inName, 
	char *inUnits, int Multiplier, WM_HWIN hParent, bool inEnabled)
{
	if ((hDlg = WM_CreateWindowAsChild(x, y, x + xSize, y + ySize, hParent, 0, NULL, 0)) <= 0)
	{
		SystemError();
		return;
	}

	// Save incoming variables
	xStart = x;
	yStart = y;
	xEnd = x + xSize;
	yEnd = y + ySize;
	minVal = inMinValue;
	maxVal = inMaxValue;
	useLowerRed = inUseLowerRed;
	useUpperRed = inUseUpperRed;
	lowerRed = inLowerRed;
	upperRed = inUpperRed;
	yPos = 0;
	Name = inName;
	Units = inUnits;
	enabled = inEnabled;
}

/****************************************************************
*
*       ~LinearGauge()
*		The Destructor
*
******************************************************************/
LinearGauge::~LinearGauge()
{
}

/****************************************************************
*
*       CreateGaugeBackground()
*		Draw the gauge except for the needle into a memory space
*
******************************************************************/
void LinearGauge::CreateGaugeBackground()
{
	GUI_RECT rect;
	char scratch[20];
	int xStartWBorder, xEndWBorder;

	// Draw outside frame of control bar
	GUI_SetColor(GUI_WHITE);

	GUI_DrawRect(xStart, yStart + BOX_START_Y, xEnd, yStart + BOX_START_Y + BOX_HEIGHT);
	GUI_DrawRect(xStart + 1, yStart + BOX_START_Y + 1, xEnd - 1, yStart + BOX_START_Y + BOX_HEIGHT - 1);

	// Draw name and units below bar
	GUI_SetFont(&GUI_Font20_ASCII);
	if (Name)
		GUI_DispStringHCenterAt(Name, ((xEnd - xStart) / 2) + xStart, yEnd - 40);
	if (Units)
		GUI_DispStringHCenterAt(Units, ((xEnd - xStart) / 2) + xStart, yEnd - 20);

	// Create rectangle for start/stop scale labels above bar
	rect.x0 = xStart;
	rect.x1 = xEnd;
	rect.y0 = yStart + 5;
	rect.y1 = yStart + 40;

	// Draw start/stop scale labels above bar
	sprintf(scratch, "%.2f", minVal);
	GUI_DispStringInRect(scratch, &rect, GUI_TA_LEFT);
	sprintf(scratch, "%.2f", maxVal);
	GUI_DispStringInRect(scratch, &rect, GUI_TA_RIGHT);

	// Calculate variables needed for scale calculations
	float totalScaleSize = maxVal - minVal;
	stepSize = ((xEnd - xStart) - (BORDER_WIDTH * 2)) / totalScaleSize;

	// Pre-define green rect to be what's left over after both reds are drawn
	GUI_RECT greenRect, lowerRedRect, upperRedRect;

	// Define active scale area (area within the borders)
	xStartWBorder = xStart + BORDER_WIDTH;
	xEndWBorder = xEnd - BORDER_WIDTH;
	
	greenRect.x0 = lowerRedRect.x0 = xStartWBorder;
	greenRect.y0 = lowerRedRect.y0 = upperRedRect.y0 = yStart + BOX_START_Y + BORDER_WIDTH;
	greenRect.x1 = upperRedRect.x1 = xEndWBorder;
	greenRect.y1 = lowerRedRect.y1 = upperRedRect.y1 = yStart + BOX_START_Y + BOX_HEIGHT - BORDER_WIDTH;

	// Calculate lower red zone (and redefine start of green rect) - if applicable
	if (useLowerRed)
	{
		int temp;

		temp = xStartWBorder + (int)((lowerRed - minVal) * stepSize);
		if (temp < xStartWBorder)
			temp = xStartWBorder;
		else if (temp > xEndWBorder)
			temp = xEndWBorder;
		lowerRedRect.x1 = greenRect.x0 = temp;
		GUI_DrawGradientV(lowerRedRect.x0, lowerRedRect.y0, lowerRedRect.x1, lowerRedRect.y1, (enabled? GUI_RED : GUI_DARKGRAY), GUI_BLACK);
	}

	// Calculate upper red zone (and redefine end of green rect) - if applicable
	if (useUpperRed)
	{
		int temp;

		temp = xEndWBorder - (int)((maxVal - upperRed) * stepSize);
		if (temp < xStartWBorder)
			temp = xStartWBorder;
		else if (temp > xEndWBorder)
			temp = xEndWBorder;
		upperRedRect.x0 = greenRect.x1 = temp;
		GUI_DrawGradientV(upperRedRect.x0, upperRedRect.y0, upperRedRect.x1, upperRedRect.y1, (enabled? GUI_RED : GUI_DARKGRAY), GUI_BLACK);
	}

	// Draw green rect in remainder
	GUI_DrawGradientV(greenRect.x0, greenRect.y0, greenRect.x1, greenRect.y1, (enabled? GUI_GREEN : GUI_MEDGRAY), GUI_BLACK);	
}

/****************************************************************
*
*       SetValue(float value)
*		Set the value of the gauge to the value specified and draw the bubble or pointer
*
******************************************************************/
void LinearGauge::SetValue(float dValue)
{
	value = dValue;
	int x, startm, start1, start2, startblack;
	char scratch[20];

	// Draw pointer at new value
	x = (xEnd - BORDER_WIDTH) - (int)((maxVal - value) * stepSize);

	// Range check x - if beyond limits then draw at the end of the scale
	if (x < xStart + 1)
		x = xStart + 1;
	else if (x > xEnd - 1)
		x = xEnd - 1;

	if (enabled)		// Only draw circle and contents if enabled
	{
		// Draw circle in decending radius rings
		// Draw white border around circle on top
		// Outermost is black border
		GUI_SetColor(GUI_BLACK);
		GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS + 4);
		// Then white
		GUI_SetColor(GUI_WHITE);
		GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS + 2);
		// Then, if in alarm, fill circle in red
		if ((useLowerRed && dValue <= lowerRed) || (useUpperRed && dValue >= upperRed))
		{
			if (enabled) 
			{	// in alarm, fill inner circle with red
				GUI_SetColor(GUI_DARKRED);
				GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS);
				GUI_SetColor(GUI_MEDPLUSRED);
				GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS-2);
				GUI_SetColor(GUI_MEDDARKRED);
				GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS-4);
			}
			GUI_SetColor(GUI_WHITE);		// Set contrasting color for text
		}
		else
		{
			if (enabled)
			{	// NOT in alarm, fill inner circle with green
				GUI_SetColor(GUI_DARKGREEN2);
				GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS);
				GUI_SetColor(GUI_MEDPLUSGREEN);
				GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS-2);
				GUI_SetColor(GUI_MEDDARKGREEN);
				GUI_FillCircle(x, yStart - CIRCLE_Y_OFFSET, CIRCLE_RADIUS-4);
				GUI_SetColor(GUI_WHITE);		// Set contrasting color for text
			}
		}
	}

	if (enabled)
	{
		startm = start1 = start2 = 3;
		startblack = 3;
	}
	else
	{
		startm = -5;
		start1 = -4;
		start2 = -3;
		startblack = -2;
	}
	// Draw black border of the bar
	GUI_SetColor(GUI_BLACK);
	GUI_DrawLine( x - 3, yStart + BOX_START_Y + startblack, x - 3, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH);
	GUI_DrawLine( x + 3, yStart + BOX_START_Y + startblack, x + 3, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH);
	GUI_DrawLine( x - 4, yStart + BOX_START_Y + startblack, x - 4, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH);
	GUI_DrawLine( x + 4, yStart + BOX_START_Y + startblack, x + 4, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH);

	// Draw white center of the vertical bar
	GUI_SetColor(GUI_WHITE);
	GUI_DrawLine( x, yStart + BOX_START_Y + startm, x, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH + 3);
	GUI_DrawLine( x - 1, yStart + BOX_START_Y + start1, x - 1, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH + 2);
	GUI_DrawLine( x + 1, yStart + BOX_START_Y + start1, x + 1, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH + 2);
	GUI_DrawLine( x - 2, yStart + BOX_START_Y + start2, x - 2, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH + 1);
	GUI_DrawLine( x + 2, yStart + BOX_START_Y + start2, x + 2, yStart + BOX_START_Y + BOX_HEIGHT + BORDER_WIDTH + 1);

	if (enabled)			// Only draw value in circle if enabled
	{
		// Draw value in circle
		GUI_SetTextMode(GUI_TEXTMODE_TRANS);
		GUI_SetFont(&GUI_Font24B_ASCII);		// Set default font and change if required (very quick routine)

		// Format string
		if (dValue == 0.0)
			strcpy(scratch, "0");				// 1 character
		else if ((dValue > 1000.0) || (dValue < -100.0))
		{
			sprintf(scratch, "%0.0f", dValue);	// 4 or more characters
			GUI_SetFont(&GUI_Font16_ASCII);		// Must resize font as text won't fit
		}
		else if ((dValue > 100.0) || (dValue < -10.0))
			sprintf(scratch, "%0.0f", dValue);	// 3 characters
		else if ((dValue > 10.0) || (dValue < -1.0))
			sprintf(scratch, "%0.1f", dValue);	// 3 characters
		else if ((dValue > 1.0) || (dValue < -0.1))
			sprintf(scratch, "%0.2f", dValue);	// 3 characters
		else
		{
#ifdef AUTO_SCALE_FONT
			sprintf(scratch, "%0.3f", dValue);	// 4 digits (or 3 plus minus sign)
			GUI_SetFont(&GUI_Font16_ASCII);
#else
			sprintf(scratch, "%0.2f", dValue);	// 3 digits (or 2 plus minus sign)
#endif
		}
		GUI_SetColor(GUI_BLACK);
		GUI_DispStringHCenterAt(scratch, x+2, yStart - Y_TEXT_OFFSET+2);
		GUI_SetColor(GUI_WHITE);
		GUI_DispStringHCenterAt(scratch, x, yStart - Y_TEXT_OFFSET);
	}
}


/****************************************************************
*
*       DrawBackground()
*		Draw the background of the gauge without the pointer needle/bubble
*
******************************************************************/
void LinearGauge::DrawBackground()
{
	CreateGaugeBackground();
}

