
#include "WM.h"
#include "commondefs.h"
#include "DataHandler.h"
#include "Utilities.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "DUII.h"

//
// General purpose utility functions
//

//
// Local function prototypes
//
char *formatLapTime(unsigned int, unsigned int);
char *formatElapsedTime(void);
void trim(char *s);
void trimn(char *s, int n);
float GetPredictiveScale(void);
void iSort(int arr[], int n);
void fSort(float arr[], int n);
int CalculateScaleTop(int rpm);
void EnsurePositive(char *pStr);

//
// Public variables
//
char timeString[15];

//
// formatElapsedTime
//
// This routine will retrieve the current elapsed time and format it into a printable string
// of the format mm:ss.
//
// Output: pointer to the formatted string
//
char *formatElapsedTime(void)
{
	unsigned int time;
	
	time = GetValue(ELAPSED_RACE_TIME).uVal / 1000;		// time = time in seconds
	
	if (time < 3600)
		sprintf(timeString, "%02d:%02d", time / 60, time % 60);
	else if (time < 36000)
		sprintf(timeString, "%d:%02d:%02d", time/3600, (time/60)%60, time % 60);
	else 		// > 10 hours
		sprintf(timeString, "%d:%02d", time/3600, (time/60)%60);

	return timeString;
}

//
// Format time in milliseconds into:
//		mm:ss.s or
//		ss.s
//
char *formatLapTime(unsigned int time, unsigned int twodigits)
{
	unsigned int tenhun, secs, mins;
	char *pStr;

	if (twodigits)
		time += 5;		// round to hundredths
	else
		time += 50;		// round to tenths

	tenhun = (time / 10) % 100;		// get 2 digits now
	secs = (time / 1000) % 60;

	pStr = timeString;

	if (time >= 60000)	{
		mins = time / 60000;
		pStr += sprintf(timeString, "%d:%02d.%d", mins, secs, tenhun / 10);
	} // if
	else
		pStr += sprintf(timeString, "%d.%d", secs, tenhun / 10);

	if (twodigits)		// add the hundredths character
		sprintf(pStr, "%d", tenhun % 10);

	return timeString;
}

//
// Trim trailing zeros off the end of the string produced by sprintf(%f)
//
void trim(char *s)
{
    char *ptr;
    if (!s)
        return;		// handle NULL string
    if (!*s)
        return;		// handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && *ptr == '0'; --ptr)
		;
    ptr[1] = '\0';
	if ((ptr[0] == '.') && (ptr != s))	// If the next previous character is the decimal point and we are not yet at the start of the string
		ptr[0] = '\0';					// Clear that out too
}

//
// Trim trailing zeros and also ensure string is no longer than n (including NULL)
//
void trimn(char *s, int n)
{
	*(s + n) = 0;
	trim(s);
}

//
// This string shoule be positive only.  Terminate string at first '-' character
//
void EnsurePositive(char *pStr)
{
	char ch;

	while ((ch = *pStr) != 0)
	{
		if (ch == '-')
		{						// Found a '-' character
			*pStr = 0;			// End string here
			return;		
		}
		else pStr++;
	}
}

float GetPredictiveScale(void)
{
	switch (sysData.lapMeter)
	{
	case 0:	return 0.5;
	case 1: return 1.0;
	case 2: return 2.0;
	case 3: return 5.0;
	case 4: return 10.0;
	default: return 1.0;
	}
}

//
// Perform bubble sort on array of floats
//
void iSort(int arr[], int n)
{
      int swapped = true;
	  int i;
      int j = 0;
      int tmp;
      while (swapped) {
            swapped = false;
            j++;
            for (i = 0; i < n - j; i++) {
                  if (arr[i] > arr[i + 1]) {
                        tmp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = tmp;
                        swapped = true;
                  }
            }
      }
}

void fSort(float arr[], int n)
{
      int swapped = true;
	  int i;
      int j = 0;
      float tmp;
      while (swapped) {
            swapped = false;
            j++;
            for (i = 0; i < n - j; i++) {
                  if (arr[i] > arr[i + 1]) {
                        tmp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = tmp;
                        swapped = true;
                  }
            }
      }
}

//
// Given an RPM find the next whole 1000 after it
//
int CalculateScaleTop(int rpm)
{
	if (rpm >= 9000)
		return ((rpm / 2000) + 1) * 2000;
	else
		return ((rpm / 1000) + 1) * 1000;
}

//
// KMC - created a case insensitive string compare for use by the LISTVIEW compare function
// so that files would be listed in alphabetical order reguardless of case
//
int kmc_stricmp(const char s1[], const char s2[])
{   
    int i = 0;
    while ( s1[i] != '\0' )
    {
		char a, b;
		
        if( s2[i] == '\0' ) 
			return 1;
        else if( (a = toupper(s1[i])) < (b = toupper(s2[i])) )
			return -1;
        else if( a > b )
			return 1;
        i++;
    }   
    return 0;
}


