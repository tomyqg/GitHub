#include "PredictiveLapTimer.hpp"
#include "GeneralConfig.h"
#include <string.h>

PredictiveLapTimer::PredictiveLapTimer(int x, int y, int xsize, int ysize, float fMax, float fMin, WM_HWIN _hParent)
{
	if ((_hDlg = WM_CreateWindowAsChild(x, y, xsize, ysize, _hParent, 0, NULL, 0)) <= 0)
		SystemError();

	xLoc = x;
	yLoc = y;
	xSize = xsize;
	ySize = ysize;

	Max = fMax;
	Min = fMin;

	mMemSpace = 0;

	mem = GUI_MEMDEV_Create(xLoc, yLoc, xSize, ySize);
	
	dValuePerPixel = (float)(xSize) / (Max - Min);

	CreateBackground();
}

PredictiveLapTimer::~PredictiveLapTimer()
{
	GUI_MEMDEV_Delete(mem);
	GUI_MEMDEV_Delete(mMemSpace);
}

void PredictiveLapTimer::CreateBackground()
{
	int x1, y1, x2, y2;
	int i, iTemp;

	if(mMemSpace)
		GUI_MEMDEV_Delete(mMemSpace);

	mMemSpace = GUI_MEMDEV_Create(xLoc, yLoc, xSize, ySize);
	GUI_MEMDEV_Select(mMemSpace);

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetPenSize(LINE_WIDTH);
	GUI_SetColor(GUI_WHITE);

	iZeroLoc = xLoc + xSize / 2;

	//Draw the rectangle
	x1 = xLoc;
	y1 = yLoc;
	x2 = xLoc + xSize - LINE_WIDTH;
	y2 = yLoc + ySize - LINE_WIDTH;

	GUI_DrawLine(x1, y1, x2, y1);
	GUI_DrawLine(x2, y1, x2, y2);
	GUI_DrawLine(x2, y2, x1, y2);
	GUI_DrawLine(x1, y2, x1, y1);

	// draw the divider lines
	iTemp = xSize / 4;

	//GUI_SetPenSize(3);
	for(i = 1; i < 4; i++)
	{
		x1 = xLoc + i * iTemp;

//#define DARK_TICS
#ifdef DARK_TICS
		if (i & 0x01)		// odd lines gray
			GUI_SetColor(GUI_GRAY);
		else				// even lines white
			GUI_SetColor(GUI_WHITE);
#endif
		GUI_DrawLine(x1, y1, x1, y2);
	}

#ifdef DARK_TICS
	GUI_SetColor(GUI_WHITE);
#endif
	GUI_SelectLCD();
}

#define PREDICTIVE_TEXT_POS_OFFSET	(8)
#define PREDICTIVE_TEXT_NEG_OFFSET	(-72)

void PredictiveLapTimer::DrawControl()
{
	int x1, y1, x2, y2, iTemp;
	char str[10];
	
	if (pAltText)
	{
		GUI_RECT rect;

		//Draw the rectangle
		GUI_SetPenSize(LINE_WIDTH - 1);
		GUI_SetColor(GUI_WHITE);
		x1 = xLoc;
		y1 = yLoc;
		x2 = xLoc + xSize - LINE_WIDTH;
		y2 = yLoc + ySize - LINE_WIDTH;

		GUI_DrawLine(x1, y1, x2, y1);
		GUI_DrawLine(x2, y1, x2, y2);
		GUI_DrawLine(x2, y2, x1, y2);
		GUI_DrawLine(x1, y2, x1, y1);

		// Draw the text (track name)
		GetRect(&rect);

		if ((strlen(pAltText) > 15) && (xSize < 300))
			GUI_SetFont(&GUI_Font20_ASCII);
		else GUI_SetFont(LAP_TIMER_FONT);

		GUI_DispStringInRect(pAltText, &rect, GUI_TA_CENTER | GUI_TA_VCENTER);
	}
	else
	{
		GUI_ClearRect(xLoc, yLoc, xLoc + xSize, yLoc + ySize);

		if(Value >= 0)
		{
			// draw the red bar
			x1 = xLoc;
			if ((x2 = iZeroLoc - (int)(Value * dValuePerPixel)) < xLoc)
				x2 = xLoc;
			y1 = yLoc + 1 + LINE_WIDTH;
			y2 = yLoc + ySize - 1;
			GUI_DrawGradientV(x2, y1, iZeroLoc, y2, GUI_RED, GUI_BLACK);
		}
		else
		{
			// draw the green bar
			if ((x1 = iZeroLoc - (int)(Value * dValuePerPixel)) > (xLoc + xSize - LINE_WIDTH))
				x1 = xLoc + xSize - LINE_WIDTH;
			x2 = xLoc + xSize;
			y1 = yLoc + 1 + LINE_WIDTH;
			y2 = yLoc + ySize - 1;
			GUI_DrawGradientV(iZeroLoc, y1, x1, y2, GUI_GREEN, GUI_BLACK);
		}

		GUI_MEMDEV_Write(mMemSpace);			// Draw the background grid and lines

		GUI_SetFont(LAP_TIMER_FONT);
		if (Value >= 10.0 || Value <= -10.0)
			sprintf(str, "%+.1f", Value);
		else
			sprintf(str, "%+.2f", Value);
		iTemp = GUI_GetFontDistY();
		y1 = yLoc + ySize / 2 - iTemp / 2;
		if (Value >= 0)
		{
			x1 = iZeroLoc + PREDICTIVE_TEXT_POS_OFFSET;
			GUI_SetColor(GUI_RED);
		}
		else
		{
			x1 = iZeroLoc + PREDICTIVE_TEXT_NEG_OFFSET;
			GUI_SetColor(GUI_GREEN);
		}
		GUI_DispStringAt(str, x1, y1);
	}
}

void PredictiveLapTimer::SetValue(float dValue)
{
	Value = dValue;
	pAltText = 0;
}

void PredictiveLapTimer::SetValue(char *pText)
{
	pAltText = pText;
}

GUI_MEMDEV_Handle PredictiveLapTimer::DrawToMemArea()
{
	int x1, y1, x2, y2, iTemp;
	char str[10];
	
	if (pAltText)
	{
		GUI_RECT rect;

		//Draw the rectangle
		GUI_SetPenSize(LINE_WIDTH - 1);
		GUI_SetColor(GUI_WHITE);
		x1 = xLoc;
		y1 = yLoc;
		x2 = xLoc + xSize - LINE_WIDTH;
		y2 = yLoc + ySize - LINE_WIDTH;

		GUI_DrawLine(x1, y1, x2, y1);
		GUI_DrawLine(x2, y1, x2, y2);
		GUI_DrawLine(x2, y2, x1, y2);
		GUI_DrawLine(x1, y2, x1, y1);

		// Draw the text (track name)
		GetRect(&rect);
		GUI_SetFont(LAP_TIMER_FONT);
		GUI_DispStringInRect(pAltText, &rect, GUI_TA_CENTER | GUI_TA_VCENTER);

		// Do not return a handle to a memory device for this operation
		return 0;
	}
	else
	{
		GUI_MEMDEV_Select(mem);

		GUI_ClearRect(xLoc, yLoc, xLoc + xSize, yLoc + ySize);

		if(Value >= 0)
		{
			// draw the red bar
			x1 = xLoc;
			if ((x2 = iZeroLoc - (int)(Value * dValuePerPixel)) < xLoc)
				x2 = xLoc;
			y1 = yLoc + 1 + LINE_WIDTH;
			y2 = yLoc + ySize - 1;
			GUI_DrawGradientV(x2, y1, iZeroLoc, y2, GUI_RED, GUI_BLACK);
		}
		else
		{
			// draw the green bar
			if ((x1 = iZeroLoc - (int)(Value * dValuePerPixel)) > (xLoc + xSize - LINE_WIDTH))
				x1 = xLoc + xSize - LINE_WIDTH;
			x2 = xLoc + xSize;
			y1 = yLoc + 1 + LINE_WIDTH;
			y2 = yLoc + ySize - 1;
			GUI_DrawGradientV(iZeroLoc, y1, x1, y2, GUI_GREEN, GUI_BLACK);
		}

		GUI_MEMDEV_Write(mMemSpace);			// Draw the background grid and lines

		GUI_SetFont(LAP_TIMER_FONT);
		sprintf(str, "%+.2f", Value);
		iTemp = GUI_GetFontDistY();
		y1 = yLoc + ySize / 2 - iTemp / 2;

		if (Value >= 0)
		{
			x1 = iZeroLoc + PREDICTIVE_TEXT_POS_OFFSET;
			GUI_SetColor(GUI_RED);
		}
		else
		{
			x1 = iZeroLoc + PREDICTIVE_TEXT_NEG_OFFSET;
			GUI_SetColor(GUI_GREEN);
		}
		GUI_DispStringAt(str, x1, y1);

		GUI_SelectLCD();
		return mem;
	}
}