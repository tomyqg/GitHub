/****************************************************************
*							Track Systems						*
-----------------------------------------------------------------
File: DigitalStringOutput.cpp
Description: The digital display object
-----------------------------------------------------------------
*/
#include "DigitalStringOutput.hpp"
#include <string.h>

DigitalStringOutput::DigitalStringOutput(int x, int y, int xSize, int ySize, DigitalOptions Options, WM_HWIN _hParent, GUI_FONT font, GUI_COLOR cColor) : ControlBase(x, y, "", "", NULL)
{
	Width = xSize;
	Height = ySize;

	DisplayOptions = Options;

	strcpy(Value, " ");

	mem = GUI_MEMDEV_Create(x, y, Width + 1, Height);
	fTextFont = font;
	Color = cColor;

	rect.x0 = x;
	rect.y0 = y;
	rect.x1 = x + Width;
	rect.y1 = y + Height;

}

DigitalStringOutput::~DigitalStringOutput()
{
	GUI_MEMDEV_Delete(mem);
}

void DigitalStringOutput::DrawControl()
{
	GUI_COLOR OldColor;

	OldColor = GUI_GetColor();
	GUI_SetColor(Color);

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetFont(&fTextFont);

	GUI_DispStringInRect(Value, &rect, GUI_TA_RIGHT);

	GUI_SetColor(OldColor);
}



char *DigitalStringOutput::GetDisplayText()
{
	char str[25];
	char strFormat[20];
	sprintf(str, "%.0f", Max);
	sprintf(strFormat, "%%%d.%df %%s", strlen(str), Precision);

	sprintf(strValue, strFormat, Value, sUnits);

	return strValue;
}

/****************************************************************
*
*       SetValue(double value)
*		Set the value of the gauge to the value specified
*
******************************************************************/
void DigitalStringOutput::SetValue(char *sValue)
{
	strcpy(Value, sValue);
}


void DigitalStringOutput::PickFont()
{
	int txt_height;
	// pick the right size font
	fTextFont = DIGITAL_LARGE_FONT;
	//GUI_SetFont(&DIGITAL_LARGE_FONT);
	txt_height = GUI_GetYDistOfFont(&DIGITAL_LARGE_FONT);
	//txt_width = GetMaxTextLength();
	if(txt_height < Height)
	{
		fTextFont = DIGITAL_LARGE_FONT;
		return;
	}

	fTextFont = DIGITAL_MEDIUM_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_MEDIUM_FONT);
	//txt_width = GetMaxTextLength();
	if(txt_height < Height)
	{
		fTextFont = DIGITAL_MEDIUM_FONT;
		return;
	}
	
	fTextFont = DIGITAL_SMALL_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_SMALL_FONT);
	//txt_width = GetMaxTextLength();
	if(txt_height < Height)
	{
		fTextFont = DIGITAL_SMALL_FONT;
		return;
	}
	
	fTextFont = DIGITAL_TINY_FONT;
	txt_height = GUI_GetYDistOfFont(&DIGITAL_TINY_FONT);
	//txt_width = GetMaxTextLength();
	if(txt_height < Height)
	{
		fTextFont = DIGITAL_TINY_FONT;
		return;
	}
	
	fTextFont = DIGITAL_MEDIUM_FONT;		// KMC - MUST pick some font - otherwise text measurement will crash
}

GUI_MEMDEV_Handle DigitalStringOutput::DrawToMemArea()
{
	GUI_COLOR OldColor;

	OldColor = GUI_GetColor();
	GUI_SetColor(Color);

	GUI_MEMDEV_Select(mem);
	GUI_ClearRect(rect.x0, rect.y0, rect.x1, rect.y1);

	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_SetFont(&fTextFont);

	GUI_DispStringInRect(Value, &rect, GUI_TA_RIGHT);

	GUI_SetColor(OldColor);

	GUI_SelectLCD();
	return mem;
}