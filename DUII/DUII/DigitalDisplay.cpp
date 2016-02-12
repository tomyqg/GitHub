/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: DigitalDisplay.cpp
Description: The digital display object
-----------------------------------------------------------------
*/
#include "DigitalDisplay.hpp"
#include <string.h>


DigitalDisplay::DigitalDisplay(int xLoc, int yLoc, int xSize, int ySize, char *Name, char *Units, float dMin, float dMax, int iPrecision, int iDividerLock, DigitalOptions Options, Alarm *AlarmList, bool bShowLabel, GUI_FONT fNumberFont, GUI_FONT fLabelFont, WM_HWIN _hParent) : ControlBase(xLoc, yLoc, Name, Units, AlarmList)
{
	Width = xSize;
	Height = ySize;

	Precision = iPrecision;
	Min = dMin;
	Max = dMax;
	DividerLoc = iDividerLock;
	DisplayOptions = Options;

	Value = 0.0;
	ShowLabel = bShowLabel;
	
	// Create memory device (whether used or not)
	mem = GUI_MEMDEV_Create(x, y, Width, Height);

	fTextFont = fNumberFont;
	fUnitsFont = fLabelFont;

	MaxStrLen = GetMaxTextLength();
}

DigitalDisplay::DigitalDisplay(int xLoc, int yLoc, int xSize, int ySize, char *sName, char *sUnits, float dMin, float dMax, int iPrecision, int iDividerLoc, DigitalOptions Options, Alarm *AlarmList, bool bShowLabel, WM_HWIN _hParent) : ControlBase(xLoc, yLoc, sName, sUnits, AlarmList)
{
	Width = xSize;
	Height = ySize;
	Precision = iPrecision;
	Min = dMin;
	Max = dMax;
	DividerLoc = iDividerLoc;
	DisplayOptions = Options;

	Value = 0.0;
	ShowLabel = bShowLabel;
	
	mem = GUI_MEMDEV_Create(x, y, Width, Height);
	PickFont();
	fUnitsFont = fTextFont;

	MaxStrLen = GetMaxTextLength();
}

DigitalDisplay::~DigitalDisplay()
{
	GUI_MEMDEV_Delete(mem);
}


void DigitalDisplay::DrawControl()
{
	char strValue[20];
	int txt_length, txt_diff;
	GUI_COLOR Color = GUI_WHITE;
	GUI_RECT rect;

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetFont(&fTextFont);
	if(AlarmList != NULL)
	{
		Alarm *pAlarm;
		pAlarm = AlarmList;
		while(pAlarm != NULL)
		{
			if(pAlarm->Enabled == true)
			{
				if(Value >= pAlarm->MinValue && Value <= pAlarm->MaxValue)
				{
					Color = GUI_BLACK;
					break;
				}
				else
				{
					Color = GUI_WHITE;
				}
			}
			else
			{
				Color = GUI_WHITE;
			}

			pAlarm = pAlarm->Next;
		}
	}
	else
	{
		Color = GUI_WHITE;
	}

	GUI_SetColor(Color);

	strcpy(strValue, GetDisplayText());
	txt_length = GUI_GetStringDistX(strValue);

	rect.x0 = x;
	rect.y0 = y;
	rect.x1 = x + txt_length;
	rect.y1 = y + Height;

	if(ShowLabel)
	{
		txt_diff = GUI_GetStringDistX(sName);

		GUI_SetFont(&fUnitsFont);
		rect.x1 = x + txt_diff;
		GUI_DispStringInRect(sName, &rect, GUI_TA_VCENTER | GUI_TA_RIGHT);
		rect.x0 = rect.x1 + 1;
		rect.x1 = x + Width - GUI_GetStringDistX(sUnits) - 5;
	}

	GUI_SetFont(&fTextFont);
	GUI_DispStringInRect(strValue, &rect, GUI_TA_VCENTER | GUI_TA_RIGHT);

	GUI_SetFont(&fUnitsFont);
	rect.x0 = rect.x1 + 5;
	rect.x1 = x + Width;
	GUI_SetColor(GUI_LIGHTGRAY);
	GUI_DispStringInRect(sUnits, &rect, GUI_TA_VCENTER | GUI_TA_LEFT);
	GUI_SetColor(GUI_WHITE);
}



char *DigitalDisplay::GetDisplayText()
{
	char str[25];
	char strFormat[20];
	sprintf(str, "%.0f", Max);
	sprintf(strFormat, "%%%d.%df", strlen(str), Precision);

	sprintf(strValue, strFormat, Value);

	return strValue;
}

/****************************************************************
*
*       SetValue(float value)
*		Set the value of the gauge to the value specified
*
******************************************************************/
void DigitalDisplay::SetValue(float dValue)
{
	Value = dValue;
}

int DigitalDisplay::GetMaxTextLength()
{
	int txt_length;
	float old_value;
	char str[100];

	GUI_SetFont(&fTextFont);

	old_value = Value;
	Value = Max;
	if(ShowLabel)
		strcpy(str, sName);
	else
		strcpy(str, "");

	strcat(str, GetDisplayText());

	Value = old_value;

	txt_length = GUI_GetStringDistX(str);

	return txt_length;
}



void DigitalDisplay::PickFont()
{
	int txt_height, txt_width;

	// pick the right size font
	fTextFont = DIGITAL_LARGE_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_LARGE_FONT);
	txt_width = GetMaxTextLength();
	if(txt_height < Height && txt_width < Width)
	{
		fTextFont = DIGITAL_LARGE_FONT;
		return;
	}

	fTextFont = DIGITAL_MEDIUM_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_MEDIUM_FONT);
	txt_width = GetMaxTextLength();
	if(txt_height < Height && txt_width < Width)
	{
		fTextFont = DIGITAL_MEDIUM_FONT;
		return;
	}
	
	fTextFont = DIGITAL_SMALL_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_SMALL_FONT);
	txt_width = GetMaxTextLength();
	if(txt_height < Height && txt_width < Width)
	{
		fTextFont = DIGITAL_SMALL_FONT;
		return;
	}
	
	fTextFont = DIGITAL_TINY_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_TINY_FONT);
	txt_width = GetMaxTextLength();
	if(txt_height < Height && txt_width < Width)
	{
		fTextFont = DIGITAL_TINY_FONT;
		return;
	}
	
	fTextFont = DIGITAL_MEDIUM_FONT;		// KMC - MUST pick some font - otherwise text measurement will crash
}

GUI_MEMDEV_Handle DigitalDisplay::DrawToMemArea()
{
	char strValue[20];
	int txt_length, txt_diff, x1, y1, iTemp;
	GUI_COLOR BkColor = GUI_BLACK, Color = GUI_WHITE;
	GUI_RECT rect;
	GUI_MEMDEV_Select(mem);
	GUI_ClearRect(x, y, x + Width, y + Height);

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetFont(&fTextFont);
	if(AlarmList != NULL)
	{
		Alarm *pAlarm;
		pAlarm = AlarmList;
		while(pAlarm != NULL)
		{
			if(pAlarm->Enabled == true)
			{
				if(Value >= pAlarm->MinValue && Value <= pAlarm->MaxValue)
				{
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
	else
	{
		BkColor = GUI_BLACK;
		Color = GUI_WHITE;
	}

	GUI_SetColor(BkColor);
	GUI_FillRect(x, y, x + Width, y + Height);
	GUI_SetColor(Color);

	y1 = y + Height / 2 - GUI_GetYDistOfFont(&fTextFont) / 2;

	strcpy(strValue, GetDisplayText());
	txt_length = GUI_GetStringDistX(strValue);

	rect.x0 = x;
	rect.y0 = y;
	rect.x1 = x + txt_length;
	rect.y1 = y + Height;

	if(ShowLabel)
	{
		txt_diff = GUI_GetStringDistX(sName);

		GUI_SetFont(&fUnitsFont);
		rect.x1 = x + txt_diff;
		GUI_DispStringInRect(sName, &rect, GUI_TA_RIGHT);
		rect.x0 = rect.x1 + 1;
		rect.x1 = x + Width - GUI_GetStringDistX(sUnits) - 5;
	}

	GUI_SetFont(&fTextFont);
	x1 = rect.x1 - GUI_GetStringDistX(strValue) - 2;
	txt_diff = GUI_GetYSizeOfFont(&fUnitsFont);
	y1 = y + (Height / 2) - (txt_diff / 2);
	iTemp = y + (Height / 2) + txt_diff;
	GUI_DispStringAt(strValue, x1, y1);

	GUI_SetFont(&fUnitsFont);
	rect.x0 = rect.x1 + 5;
	rect.x1 = x + Width;
	GUI_SetColor(GUI_LIGHTGRAY);
	x1 = rect.x0;
	y1 = iTemp - GUI_GetYSizeOfFont(&fUnitsFont) - 10;
	GUI_DispStringAt(sUnits, x1, y1);
	GUI_SetColor(GUI_WHITE);

	GUI_SelectLCD();
	return mem;
}
