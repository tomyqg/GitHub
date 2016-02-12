/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : FS_Logf.c
Purpose     : File system log routines
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include <stdlib.h>
#include <stdarg.h>

#include "FS_Int.h"
#include "FS_Debug.h"

/*********************************************************************
*
*       #define constants
*
**********************************************************************
*/

/*********************************************************************
*
*       Local data types
*
**********************************************************************
*/

typedef struct {
  char * pBuffer;
  int BufferSize;
  int Cnt;
} BUFFER_DESC;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32  _LogFilter  = FS_LOG_MASK_DEFAULT;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       _StoreChar
*/
static void _StoreChar(BUFFER_DESC * p, char c) {
  int Cnt;

  Cnt = p->Cnt;
  if ((Cnt + 1) < p->BufferSize) {
    *(p->pBuffer + Cnt) = c;
    p->Cnt = Cnt + 1;
  }
}

/*********************************************************************
*
*       _PrintUnsigned
*/
static void _PrintUnsigned(BUFFER_DESC * pBufferDesc, U32 v, unsigned Base, int NumDigits) {
  static const char _aV2C[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  unsigned Div;
  U32 Digit = 1;
  //
  // Count how many digits are required
  //
  while (((v / Digit) >= Base) || (NumDigits > 1)) {
    Digit *= Base;
 	  NumDigits--;
  }
  //
  // Output digits
  //
  do {
    Div = v / Digit;
    v -= Div * Digit;
    _StoreChar(pBufferDesc, _aV2C[Div]);
    Digit /= Base;
  } while (Digit);
}


/*********************************************************************
*
*       _PrintInt
*/
static void _PrintInt(BUFFER_DESC * pBufferDesc, I32 v, unsigned Base, unsigned NumDigits) {
  //
  // Handle sign
  //
  if (v < 0) {
    v = -v;
    _StoreChar(pBufferDesc, '-');
  }
  _PrintUnsigned(pBufferDesc, v, Base, NumDigits);
}

/*********************************************************************
*
*       FS_PrintfSafe
*/
void FS_PrintfSafe(char * pBuffer, const char * sFormat, int BufferSize, va_list * pParamList) {
  char c;
  BUFFER_DESC BufferDesc;
  I32 v;
  unsigned NumDigits;

  BufferDesc.pBuffer    = pBuffer;
  BufferDesc.BufferSize = BufferSize;
  BufferDesc.Cnt        = 0;
  do {
    c = *sFormat++;
    if (c == 0) {
      break;
    }
    if (c == '%') {
      //
      // Filter out width specifier (number of digits)
      //
      NumDigits = 0;
      do {
        c = *sFormat++;
        if (c < '0' || c > '9') {
          break;
        }
        NumDigits = NumDigits * 10 + (c - '0');
      } while (1);
      //
      // Handle different qualifiers
      //
      do {
        if (c == 'l') {
          c = *sFormat++;
          continue;
        }
        break;
      } while (1);
      //
      // handle different types
      //
      v = va_arg(*pParamList, int);
      switch (c) {
      case 'd':
        _PrintInt(&BufferDesc, v, 10, NumDigits);
        break;
      case 'u':
        _PrintUnsigned(&BufferDesc, v, 10, NumDigits);
        break;
      case 'x':
        _PrintUnsigned(&BufferDesc, v, 16, NumDigits);
        break;
      case 's':
        {
          const char * s = (const char *)v;
          do {
            c = *s++;
            if (c == 0) {
              break;
            }
           _StoreChar(&BufferDesc, c);
          } while (1);
        }
        break;
      case 'p':
        _PrintUnsigned(&BufferDesc, v, 16, 8);
        break;
      default:
        FS_X_Panic(FS_ERROR_ILLEGAL_FORMAT_STRING);
      }
    } else {
      _StoreChar(&BufferDesc, c);
    }
  } while (1);
  //
  // Add trailing 0 to string
  //
  *(pBuffer + BufferDesc.Cnt) = 0;
}



/*********************************************************************
*
*       FS_Logf
*/
void FS_Logf(U32 Type, const char * sFormat, ...) {
  va_list ParamList;
  char acBuffer[100];

  //
  // Filter message. If logging for this type of message is not enabled, do  nothing.
  //
  if ((Type & _LogFilter) == 0) {
    return;
  }
  //
  // Replace place holders (%d, %x etc) by values and call output routine.
  //
  va_start(ParamList, sFormat);
  FS_PrintfSafe(acBuffer, sFormat, sizeof(acBuffer), &ParamList);
  FS_X_Log(acBuffer);
}


/*********************************************************************
*
*       FS_SetLogFilter
*/
void FS_SetLogFilter(U32 FilterMask) {
  _LogFilter = FilterMask;
}

/*********************************************************************
*
*       FS_AddLogFilter
*/
void FS_AddLogFilter(U32 FilterMask) {
  _LogFilter |= FilterMask;
}

/*************************** End of file ****************************/
