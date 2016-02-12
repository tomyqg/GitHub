

/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: GMeter.cpp
Description: gauge object header file
-----------------------------------------------------------------
*/
#include "GUI.h"
#include "GMeter.hpp"
#include <math.h>
#include "commondefs.h"
#include "GMeterScreen.h"
#include "ConfigScreens.h"
#include "RunningGauges.h"
#include "DUII.h"
#include "DataHandler.h"


extern WM_HWIN hGMeterScreen;
extern "C" {
extern GUI_CONST_STORAGE GUI_BITMAP bmGMeterBackground;
}

#define	RIGHT_COLUMN	385
// 2 second persist time
#define	PERSIST_TIME	3500
#define	PRIME_DOT       0x0000FF00
#define	NUM_DOTS		(PERSIST_TIME / G_METER_SCREEN_REFRESH_TIME)
#define	START_COLOR		GUI_DARKGRAY
#define	FINISH_COLOR	GUI_LIGHTGRAY
//#define	COLOR_STEP		((FINISH_COLOR - START_COLOR) / NUM_DOTS)
#define	COLOR_STEP		0x00040404

struct DOT {
	int x;
	int y;
};

struct DOT dotArray[NUM_DOTS];
int dotIndex = 0;


GMeter::GMeter(int iMaxG)
{

	xCenter = yCenter = 0;
	mMemBackground = 0;
	
	x = GMETER_CIRCLE_X;
	y = GMETER_CIRCLE_Y;

	Radius = GMETER_RADIUS;
	MaxG = iMaxG;

	dPixelsPerG = Radius / MaxG;

	xMaxValue = 0;
	xMinValue = 0;
	yMaxValue = 0;
	yMinValue = 0;
	
	xValue = yValue = 0;		// KMC added, these were not initialized on target hardware!
	new_x1 = new_y1 = 0;

	// Initialize dot array
	GUI__memset((unsigned char *)dotArray, 0, sizeof(dotArray));
	dotIndex = 0;

	CreateBackground();
}

GMeter::~GMeter()
{
	GUI_MEMDEV_Delete(mMemBackground);
}

void GMeter::DrawControl()
{
	int loopCounter;
	GUI_FONT fOldFont;
	char str[10];
	GUI_COLOR color, OldColor;
	float temp;

	OldColor = GUI_GetColor();
	fOldFont = *GUI_GetFont();

	GUI_DrawBitmap(&bmGMeterBackground, 0, 0);

	// Draw all the dots, starting with the oldest and moving to the current one - so the most current is drawn on top
	GUI_SetColor(color = START_COLOR);

	for (loopCounter = 0; loopCounter < NUM_DOTS; loopCounter++)
	{
		int index;
		if ((index = loopCounter + dotIndex) >= NUM_DOTS)
			index -= NUM_DOTS;

		x = dotArray[index].x;
		y = dotArray[index].y;
		if ((x != 0) && (y != 0))		// Only draw dots once set
		{
			if (loopCounter >= NUM_DOTS - 1)
			{	// Final dot - current position
				// Draw red line from center to current g-spot (if valid data available)
				GUI_SetColor(GUI_RED);
				GUI_SetPenSize(5);
				GUI_DrawLine(xCenter, yCenter, new_x1, new_y1);

				GUI_SetColor(GUI_BLACK);	// Set color for final dot
				GUI_FillCircle(x, y, GMETER_MARKER_DIA + 1);
				GUI_SetColor(PRIME_DOT);	// Set color for final dot
				GUI_FillCircle(x, y, GMETER_MARKER_DIA);
			}
			else GUI_FillCircle(x, y, GMETER_MARKER_DIA);
		}
		if ((color += COLOR_STEP) > FINISH_COLOR)
			color = FINISH_COLOR;
		GUI_SetColor(color);
	}

	//
	// Draw static text and current values and peaks
	//
	GUI_SetTextMode(GUI_TM_TRANS);			// All text transparent

	// Font for current values
	GUI_SetFont(&GUI_FontD32);

	// Draw current Y
	GUI_SetColor(GUI_YELLOW);
	if ((temp = -yValue) < 0)
		temp = 0;
	sprintf(str, "%.2f", temp);				// Current Left
	GUI_DispStringAt(str, 0, 100);

	if ((temp = yValue) < 0)
		temp = 0;
	sprintf(str, "%.2f", temp);				// Current Right
	GUI_DispStringAt(str, RIGHT_COLUMN, 100);

	// Draw current X
	GUI_SetColor(GUI_RED);
	if ((temp = xValue) < 0)
		temp = 0;
	sprintf(str, "%.2f", temp);			// Current Braking
	GUI_DispStringAt(str, RIGHT_COLUMN, 5);

	GUI_SetColor(GUI_GREEN);
	if ((temp = -xValue) < 0)
		temp = 0;
	sprintf(str, "%.2f", temp);			// Current Acceleration
	GUI_DispStringAt(str, RIGHT_COLUMN, 236);

	// Draw static text
	GUI_SetFont(&GUI_Font20B_ASCII);
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DispStringAt("Bmax", RIGHT_COLUMN + 45, 44);
	GUI_DispStringAt("Rmax", RIGHT_COLUMN + 45, 140);
	GUI_DispStringAt("Amax", RIGHT_COLUMN + 45, 205);
	GUI_DispStringAt("Lmax", 0, 140);

	// Show the peak values - uses same font
	GUI_SetColor(GUI_RED);
	sprintf(str, "%.2f", xMaxValue);		// Max Braking
	GUI_DispStringAt(str, RIGHT_COLUMN, 44);
	
	GUI_SetColor(GUI_GREEN);
	sprintf(str, "%.2f", xMinValue);		// Max Accel
	GUI_DispStringAt(str, RIGHT_COLUMN, 205);

	GUI_SetColor(GUI_YELLOW);
	sprintf(str, "%.2f", yMinValue);		// Max Left
	GUI_DispStringAt(str, 60, 140);

	sprintf(str, "%.2f", yMaxValue);		// Max right
	GUI_DispStringAt(str, RIGHT_COLUMN, 140);

	// Draw totalG in center
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontD32);
	sprintf(str, "%0.2f", totalG);
	GUI_DispStringHCenterAt(str, xCenter, yCenter - 16);

	GUI_SetFont(&fOldFont);
	GUI_SetColor(OldColor);

	// draw navigation arrow
	GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

#ifdef	INCLUDE_RACE_HELP
	// If the help screen is still being displayed then show the appropriate help screen
	if (raceTimerActive)
	{
		CheckRaceHelpScreen(GMETER);
	}
#endif
}


void GMeter::CreateBackground()
{
	// Note - background is now all handled by a static image
	xCenter = x + Radius;
	yCenter = y + Radius;
}

void GMeter::SetValue(float dXValue, float dYValue)
{
	xValue = dXValue;
	yValue = dYValue;

	xMaxValue = GetValue(MAX_X_G).fVal;
	xMinValue = -GetValue(MIN_X_G).fVal;
	yMaxValue = GetValue(MAX_Y_G).fVal;
	yMinValue = -GetValue(MIN_Y_G).fVal;

	totalG = GetValue(COMBINED_G).fVal;

	// Calculate current "G-Spot"  ;)
	new_x1 = (int)(xCenter + (int) (2.0 * dPixelsPerG * yValue / (float) MaxG));
	new_y1 = (int)(yCenter + (int) (2.0 * dPixelsPerG * -xValue / (float) MaxG));

	// Save new g-spot to the next place in the array
	dotArray[dotIndex].x = new_x1;
	dotArray[dotIndex].y = new_y1;

	if (++dotIndex >= NUM_DOTS)
		dotIndex = 0;
}
