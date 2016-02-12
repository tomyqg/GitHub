#ifndef PREDICTIVELAPTIMER_H
#define PREDICTIVELAPTIMER_H

#include "GUI.h"
#include "WM.h"
#include <stdio.h>

#define LINE_WIDTH 3
#define LAP_TIMER_FONT &GUI_Font32B_ASCII

class PredictiveLapTimer
{
public:
	PredictiveLapTimer(int x, int y, int xsize, int ysize, float fMax, float fMin, WM_HWIN _hParent);
	~PredictiveLapTimer();
	void DrawControl();
	void SetValue(float dValue);
	void SetValue(char *pText);
	GUI_MEMDEV_Handle DrawToMemArea();

	void GetRect(GUI_RECT *rect)
	{
		rect->x0 = xLoc;
		rect->y0 = yLoc;
		rect->x1 = xLoc + xSize;
		rect->y1 = yLoc + ySize;
	}

private:
	void CreateBackground();
	int xLoc, yLoc, xSize, ySize;
	int iZeroLoc; // the x pixel location that coorisponds to zero
	float Value, dValuePerPixel;
	char *pAltText;

	float Max, Min;
	
	WM_HWIN _hDlg;
	GUI_MEMDEV_Handle mMemSpace, mem;

};

#endif