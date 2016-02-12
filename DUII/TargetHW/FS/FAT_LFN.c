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
File        : FAT_LFN.c
Purpose     : Handling of long file names for FAT file system
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       Fat specifications
*
*  Long file names
*    Storage of a Long-Name Within Long Directory Entries
*    A long name can consist of more characters than can fit in a single long directory entry. When this
*    occurs the name is stored in more than one long entry. Index any event, the name fields themselves
*    within the long entries are disjoint. The following example is provided to illustrate how a long name
*    is stored across several long directory entries. Names are also NUL terminated and padded with
*    0xFFFF characters in order to detect corruption of long name fields by errant disk utilities. A name
*    that fits exactly in a n long directory entries (i.e. is an integer multiple of 13) is not NUL terminated
*    and not padded with 0xFFFFs.
*
**********************************************************************
*/


/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FAT_Intern.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   FS_FAT_LFN_MAX_SHORT_NAME
  #define FS_FAT_LFN_MAX_SHORT_NAME  1000
#endif

#ifndef    FS_FAT_LFN_BIT_ARRAY_SIZE
  #define  FS_FAT_LFN_BIT_ARRAY_SIZE      256
#endif

#if FS_FAT_SUPPORT_UTF8
  #define GET_CHAR(c, sLongName)   {          \
    U16 UnicodeChar;                          \
    UnicodeChar = _UTF8_2_Unicode(sLongName); \
    UnicodeChar = _ToUpper(UnicodeChar);      \
    sLongName  += _GetCharSize(sLongName);    \
    if (UnicodeChar <= 0x7f) {                \
      c = (U8)UnicodeChar;                    \
    } else {                                  \
      c = '_';                                \
    }                                         \
  }
  #define GET_CHAR_SIZE(p)     _GetCharSize(p)
  #define GET_CHAR_CODE(p)     _UTF8_2_Unicode(p)
  #define STRLEN(p)            _UTF8_StrLen(p)
  #define STRNLEN(p, NumChars) _UTF8_StrnLen(p, NumChars)
  #define WALK_2_CHAR(s, Off)  _UTF8_Walk2Char(sLongName, Off)
#else
  #define GET_CHAR(c, sLongName) {         \
    c = *sLongName;                        \
    if ((int)c <= 0x7f) {                       \
      c = FS_TOUPPER(c);                   \
    } else {                               \
      c = '_';                             \
    }                                      \
    sLongName++;                           \
  }
  #define GET_CHAR_SIZE(p)       1
  #define GET_CHAR_CODE(p)      (*p) & 0x00ff
  #define STRLEN(p)             FS_STRLEN(p)
  #define STRNLEN(p, NumChars)  NumChars
  #define WALK_2_CHAR(s, Off)   Off
#endif




#if FS_FAT_SUPPORT_UTF8
/*********************************************************************
*
*       Static typedefs
*
**********************************************************************
*/
typedef struct {
  U16 LowerCase;
  U16 UpperCase;
} UNICODE_CHARTABLE;


/*********************************************************************
*
*       Static const
*
**********************************************************************
*/

static const UNICODE_CHARTABLE _aLower2UpperTable[] = {
  /* Latin-1 Supplement */
  {0x00e0, 0x00c0}, {0x00e1, 0x00c1}, {0x00e2, 0x00c2}, {0x00e3, 0x00c3}, {0x00e4, 0x00c4}, {0x00e5, 0x00c5}, {0x00e6, 0x00c6}, {0x00e7, 0x00c7},
  {0x00e8, 0x00c8}, {0x00e9, 0x00c9}, {0x00ea, 0x00ca}, {0x00eb, 0x00cb}, {0x00ec, 0x00cc}, {0x00ed, 0x00cd}, {0x00ee, 0x00ce}, {0x00ef, 0x00cf},
  {0x00f0, 0x00d0}, {0x00f1, 0x00d1}, {0x00f2, 0x00d2}, {0x00f3, 0x00d3}, {0x00f4, 0x00d3}, {0x00f5, 0x00d3}, {0x00f6, 0x00d3}, {0x00f7, 0x00d3},
  {0x00f8, 0x00d3}, {0x00f9, 0x00d3}, {0x00fa, 0x00d3}, {0x00fb, 0x00d3}, {0x00fc, 0x00d3}, {0x00fd, 0x00d3}, {0x00fe, 0x00d3}, {0x00ff, 0x0178},

  /* Latin-1 Extended A */
  {0x0101, 0x0100}, {0x0103, 0x0102}, {0x0105, 0x0104}, {0x0107, 0x0106}, {0x0109, 0x0108}, {0x010b, 0x010a}, {0x010d, 0x010c}, {0x010f, 0x010e},
  {0x0111, 0x0110}, {0x0113, 0x0112}, {0x0115, 0x0114}, {0x0117, 0x0116}, {0x0119, 0x0118}, {0x011b, 0x011a}, {0x011d, 0x011c}, {0x011f, 0x011e},
  {0x0121, 0x0120}, {0x0123, 0x0122}, {0x0125, 0x0124}, {0x0127, 0x0126}, {0x0129, 0x0128}, {0x012b, 0x012a}, {0x012d, 0x012c}, {0x012f, 0x012e},
  {0x0131, 0x0130}, {0x0133, 0x0132}, {0x0135, 0x0134}, {0x0137, 0x0136}, {0x013a, 0x0139}, {0x013c, 0x013b}, {0x013e, 0x013d}, {0x0140, 0x013f},
  {0x0142, 0x0141}, {0x0144, 0x0143}, {0x0146, 0x0145}, {0x0148, 0x0147}, {0x014b, 0x014a}, {0x014d, 0x014c}, {0x014f, 0x014e},
  {0x0151, 0x0150}, {0x0153, 0x0152}, {0x0155, 0x0154}, {0x0157, 0x0156}, {0x0159, 0x0158}, {0x015b, 0x015a}, {0x015d, 0x015c}, {0x015f, 0x015e},
  {0x0161, 0x0160}, {0x0163, 0x0162}, {0x0165, 0x0164}, {0x0167, 0x0166}, {0x0169, 0x0168}, {0x016b, 0x016a}, {0x016d, 0x016c}, {0x016f, 0x016e},
  {0x0171, 0x0170}, {0x0173, 0x0172}, {0x0175, 0x0174}, {0x0177, 0x0176}, {0x017a, 0x0179}, {0x017c, 0x017b}, {0x017e, 0x017d},

  /* Latin-1 Extended B */
  {0x0183, 0x0182}, {0x0185, 0x0184}, {0x0188, 0x0187}, {0x018c, 0x018b},
  {0x0192, 0x0191}, {0x0199, 0x0198},
  {0x01a1, 0x01a0}, {0x01a3, 0x01a2}, {0x01a5, 0x01a4}, {0x01a8, 0x01a7}, {0x01b0, 0x01af},
  {0x01b4, 0x01b3}, {0x01b6, 0x01b5}, {0x01b9, 0x01b8},
  {0x01c6, 0x01c4}, {0x01c5, 0x01c4}, {0x01c8, 0x01c7}, {0x01c9, 0x01c7}, {0x01cb, 0x01ca}, {0x01cc, 0x01ca}, {0x01ce, 0x01cd},
  {0x01d0, 0x01cf}, {0x01d2, 0x01d1}, {0x01d4, 0x01d3}, {0x01d6, 0x01d5}, {0x01d8, 0x01d7}, {0x01da, 0x01d9}, {0x01dc, 0x01db}, {0x01dd, 0x018e}, {0x01df, 0x01de},
  {0x01e1, 0x01e0}, {0x01e3, 0x01e2}, {0x01e5, 0x01e4}, {0x01e7, 0x01e6}, {0x01e9, 0x01e8}, {0x01eb, 0x01ea}, {0x01ed, 0x01ec}, {0x01ef, 0x01ee},
  {0x01f2, 0x01f1}, {0x01f3, 0x01f1}, {0x01f5, 0x01f4}, {0x01fb, 0x01fa}, {0x01fd, 0x01fc}, {0x01ff, 0x01fe},
  {0x0201, 0x0200}, {0x0203, 0x0202}, {0x0205, 0x0204}, {0x0207, 0x0206}, {0x0209, 0x0208}, {0x020b, 0x020a}, {0x020d, 0x020c}, {0x020f, 0x020e},
  {0x0211, 0x0210}, {0x0213, 0x0212}, {0x0215, 0x0214}, {0x0217, 0x0216},

  /* Basic Greek */
  {0x03b1, 0x0391}, {0x03b2, 0x0392}, {0x03b3, 0x0393}, {0x03b4, 0x0394}, {0x03b5, 0x0395}, {0x03b6, 0x0396}, {0x03b7, 0x0397},
  {0x03b8, 0x0398}, {0x03b9, 0x0399}, {0x03ba, 0x039a}, {0x03bb, 0x039b}, {0x03bc, 0x039c}, {0x03bd, 0x039d}, {0x03be, 0x039e},
  {0x03c0, 0x03a0}, {0x03c1, 0x03a1}, {0x03c2, 0x03a3}, {0x03c3, 0x03a3}, {0x03c4, 0x03a4}, {0x03c5, 0x03a5}, {0x03c6, 0x03a6}, {0x03c7, 0x03a7},
  {0x03c8, 0x03a8}, {0x03c9, 0x03a9}, {0x03ca, 0x03aa}, {0x03cb, 0x03ab}, {0x03cc, 0x038c}, {0x03cd, 0x038e}, {0x03ce, 0x038f},
  {0x03ac, 0x0386}, {0x03ad, 0x0388}, {0x03ae, 0x0389}, {0x03af, 0x038a},

  /* Cyrillic */
  {0x0430, 0x0410}, {0x0431, 0x0411}, {0x0432, 0x0412}, {0x0433, 0x0413}, {0x0434, 0x0414}, {0x0435, 0x0415}, {0x0436, 0x0416}, {0x0437, 0x0417},
  {0x0438, 0x0418}, {0x0439, 0x0419}, {0x043a, 0x041a}, {0x043b, 0x041b}, {0x043c, 0x041c}, {0x043d, 0x041d}, {0x043e, 0x041e}, {0x043f, 0x041f},
  {0x0440, 0x0420}, {0x0441, 0x0421}, {0x0442, 0x0422}, {0x0443, 0x0423}, {0x0444, 0x0424}, {0x0445, 0x0425}, {0x0446, 0x0426}, {0x0447, 0x0427},
  {0x0448, 0x0428}, {0x0449, 0x0429}, {0x044a, 0x042a}, {0x044b, 0x042b}, {0x044c, 0x042c}, {0x044d, 0x042d}, {0x044e, 0x042e}, {0x044f, 0x042f},
  {0x0451, 0x0401}, {0x0452, 0x0402}, {0x0453, 0x0403}, {0x0454, 0x0404}, {0x0455, 0x0405}, {0x0456, 0x0406}, {0x0457, 0x0407},
  {0x0458, 0x0408}, {0x0459, 0x0409}, {0x045a, 0x040a}, {0x045b, 0x040b}, {0x045c, 0x040c}, {0x045e, 0x040e}, {0x045f, 0x040f},

  {0x0461, 0x0460}, {0x0463, 0x0462}, {0x0465, 0x0464}, {0x0467, 0x0466}, {0x0469, 0x0468}, {0x046b, 0x046a}, {0x046d, 0x046c}, {0x046f, 0x046e},
  {0x0471, 0x0470}, {0x0473, 0x0472}, {0x0475, 0x0474}, {0x0477, 0x0476}, {0x0479, 0x0478}, {0x047b, 0x047a}, {0x047d, 0x047c}, {0x047f, 0x047e},

  {0x0481, 0x0480},
  {0x0491, 0x0490}, {0x0493, 0x0492}, {0x0495, 0x0494}, {0x0497, 0x0496}, {0x0499, 0x0498}, {0x049b, 0x049a}, {0x049d, 0x049c}, {0x049f, 0x049e},
  {0x04a1, 0x04a0}, {0x04a3, 0x04a2}, {0x04a5, 0x04a4}, {0x04a7, 0x04a6}, {0x04a9, 0x04a8}, {0x04ab, 0x04aa}, {0x04ad, 0x04ac}, {0x04af, 0x04ae},
  {0x04b1, 0x04b0}, {0x04b3, 0x04b2}, {0x04b5, 0x04b4}, {0x04b7, 0x04b6}, {0x04b9, 0x04b8}, {0x04bb, 0x04ba}, {0x04bd, 0x04bc}, {0x04bf, 0x04be},

  {0x04c2, 0x04c1}, {0x04c4, 0x04c3}, {0x04c8, 0x04c7}, {0x04cc, 0x04cb},

  {0x04d1, 0x04d0}, {0x04d3, 0x04d2}, {0x04d5, 0x04d4}, {0x04d7, 0x04d6}, {0x04d9, 0x04d8}, {0x04db, 0x04da}, {0x04dd, 0x04dc}, {0x04df, 0x04de},
  {0x04e1, 0x04e0}, {0x04e3, 0x04e2}, {0x04e5, 0x04e4}, {0x04e7, 0x04e6}, {0x04e9, 0x04e8}, {0x04eb, 0x04ea}, {0x04ef, 0x04ee},
  {0x04f1, 0x04f0}, {0x04f3, 0x04f2}, {0x04f5, 0x04f4}, {0x04f9, 0x04f8},
  FS_UNICODE_UPPERCASE_EXT,
  {0x0000, 0x0000}  /* End of table */
};
#endif /* FS_FAT_SUPPORT_UTF8 */

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

#if FS_FAT_SUPPORT_UTF8
/*********************************************************************
*
*       _GetCharCode
*
* Purpose:
*   Return the UNICODE character code of the current character.
*/
static U16 _UTF8_2_Unicode(const char * s) {
  U16 r;
  U8 Char = *s;
  if ((Char & 0x80) == 0) {                /* Single byte (ASCII)  */
    r = Char;
  } else if ((Char & 0xe0) == 0xc0) {      /* Double byte sequence */
    r = (Char & 0x1f) << 6;
    Char = *(++s);
    Char &= 0x3f;
    r |= Char;
  } else if ((Char & 0xf0) == 0xe0) {      /* 3 byte sequence      */
    r = (Char & 0x0f) << 12;
    Char = *(++s);
    Char &= 0x3f;
    r |= (Char << 6);
    Char = *(++s);
    Char &= 0x3f;
    r |= Char;
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_FS, "Illegal character during UTF-8 decoding!"));
    r = 1;       /* Illegal character. To avoid endless loops in upper layers, we return 1 rather than 0. */
  }
  return r;
}

/*********************************************************************
*
*       _CalcSizeOfChar
*
* Purpose:
*   Return the number of bytes needed for the given character.
*/
static int _CalcSizeOfChar(U16 Char) {
  int r;
  if (Char & 0xF800) {                /* Single byte (ASCII)  */
    r = 3;
  } else if (Char & 0xFF80) {         /* Double byte sequence */
    r = 2;
  } else {                            /* 3 byte sequence      */
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       _Unicode_2_UTF8
*
* Purpose:
*   Encode character into 1/2/3 bytes.
*/
static int _Unicode_2_UTF8(char *s, U16 Char) {
  int r;
  r = _CalcSizeOfChar(Char);
  switch (r) {
  case 1:
    *s = (char)Char;
    break;
  case 2:
    *s++ = 0xC0 | (Char >> 6);
    *s   = 0x80 | (Char & 0x3F);
    break;
  case 3:
    *s++ = 0xE0 | (Char >> 12);
    *s++ = 0x80 | ((Char >> 6) & 0x3F);
    *s   = 0x80 | (Char & 0x3F);
    break;
  }
  return r;
}

/*********************************************************************
*
*       _GetCharSize
*
* Purpose:
*   Return the number of bytes of the current character.
*/
static int _GetCharSize(const char * s) {
  U8 Char = *s;
  if ((Char & 0x80) == 0) {
    return 1;
  } else if ((Char & 0xe0) == 0xc0) {
    return 2;
  } else if ((Char & 0xf0) == 0xe0) {
    return 3;
  }
  FS_DEBUG_ERROROUT((FS_MTYPE_FS, "Illegal character during UTF-8 decoding!"));
  return 1;       /* Illegal character. To avoid endless loops in upper layers, we return 1 rather than 0. */
}

/*********************************************************************
*
*       _UTF8_StrLen
*
* Purpose:
*   Returns the number of characters in the UTF8 encoded string.
*/
static int _UTF8_StrLen(const char* s) {
  int r;
  int Len;
  r = 0;
  while(*s) {
    Len = _GetCharSize(s);
    r++;
    s += Len;
  }
  return r;
}

/*********************************************************************
*
*       _UTF8_StrnLen
*
* Purpose:
*   Returns the number of characters in the UTF8 encoded string.
*/
static int _UTF8_StrnLen(const char* s, unsigned NumChars) {
  int r;
  int Len;
  r = 0;
  while((*s) && (NumChars)) {
    Len = _GetCharSize(s);
    r++;
    NumChars-= Len;
    s += Len;

  }
  return r;
}


/*********************************************************************
*
*       _UTF8_Walk2Char
*
* Purpose:
*   returns the offset of the desired characters to walk.
*/
static int _UTF8_Walk2Char(const char* s, int NumChars) {
  int r;
  int Len;
  r = 0;
  while((NumChars--) && (*s)) {
    Len = _GetCharSize(s);
    r += Len;
    s += Len;
  }
  return r;
}
#endif /* FS_FAT_SUPPORT_UTF8 */

/*********************************************************************
*
*       _CalcNumLongEntries
*
* Returns
*   Number of directory entries for the long file name.
*   Examples:
*     "test.txt"                         -> 1
*     "FileName.txt"                     -> 1
*     "LongFileName.txt"                 -> 2
*     "Very very very LongFileName.txt"  -> 3
*/
static int _CalcNumLongEntries(const char * sLongName, int Len) {
  if (Len == 0) {
    Len = STRLEN(sLongName);
  } else {
    Len = STRNLEN(sLongName, Len);
  }
  return (Len + 12) / 13;
}

#if FS_FAT_SUPPORT_UTF8
/*********************************************************************
*
*       _LoadU16
*
*/
static U16 _LoadU16(const U16 * pSrc) {
  U16         Data16;

  if ((U32)pSrc & 1) {
    const U8 * pSrc8;

    pSrc8   = (const U8 *)pSrc;
    Data16  = *pSrc8;
    Data16 |= (*(pSrc8 + 1) << 8);
  } else {
    Data16 = *pSrc;
  }
  return Data16;
}

/*********************************************************************
*
*       _CopyString, UTF8 version
*
*/
static int _CopyString(char * pDest,  const void * p, int MaxLen) {
  int LenDest;
  const U16 * pSrc;
  U16         Data16;

  pSrc = (const U16 *)p;
  LenDest = 0;
  while (MaxLen--) {
    int NumBytes;

    Data16 = _LoadU16(pSrc);
    NumBytes = _Unicode_2_UTF8(pDest, Data16);
    pSrc++;
    Data16 = _LoadU16(pSrc);
    if (Data16 == 0xffff) {
      break;
    }
    pDest   += NumBytes;
    LenDest += NumBytes;
  }
  return LenDest;
}
#else
/*********************************************************************
*
*       _CopyString
*
*/
static int _CopyString(char * pDest,  const void * p, int MaxLen) {
  int i;
  const U8 * pSrc;

  pSrc = (const U8 *)p;
  for (i = 0; i < MaxLen; i++) {
    if (*pSrc == 0xff) {
      break;
    }
    *pDest++ = *pSrc;
    pSrc += 2;
  }
  return i;
}
#endif

/*********************************************************************
*
*       _CopyLongName
*
*/
static int _CopyLongName(char * pDest, const U8 * pSrc) {
  int r;
  r = 0;
  r += _CopyString (pDest,  pSrc, 5);
  r += _CopyString (pDest + r, (pSrc + 13), 6);
  r += _CopyString (pDest + r, (pSrc + 27), 2);
  return r;
}

/*********************************************************************
*
*       _CalcNumChar
*
*/
static U8 _CalcNumChar(const U8 * pSrc, int MaxLen) {
  U8 i;
  for (i = 0; i < MaxLen; i++, pSrc += 2) {
    if (*pSrc == 0xff) {
      break;
    }
  }
  return i;
}

/*********************************************************************
*
*       _GetNumCharInEntry
*
*/
static int _GetNumCharInEntry(const U8 * pEntry) {
  int r;
  r = 0;
  r += _CalcNumChar(pEntry,      5);
  r += _CalcNumChar(pEntry + 13, 6);
  r += _CalcNumChar(pEntry + 27, 2);
  return r;
}

/*********************************************************************
*
*       _CorrectTrail
*
*/
static int _CorrectTrail(const char *pFileName, int Len) {
  int i;
  const char * p;

  //
  //  Determine the length of the string
  //
  if (Len == 0) {
    Len = FS_STRLEN(pFileName);
  }
  //
  //  Set pointer to the end of the string and
  //  check the string reverse from any spaces
  //  that need to be removed
  //
  p = pFileName + Len - 1;
  for (i = Len; i; i--) {
    if (*p == ' ') {
      Len--;
    } else {
      break;
    }
    p--;
  }
  return Len;
}

/*********************************************************************
*
*       _WriteName
*
*/
static int _WriteName(U8 * pDest, const char * pSrc, unsigned NumBytes, int RemBytes) {
  int Len;
  int NumChars;

  NumChars = 0;
  Len      = 1;
  do {
    if (RemBytes > 0) {
      U16 UnicodeChar;
      Len = GET_CHAR_SIZE(pSrc);
      UnicodeChar = GET_CHAR_CODE(pSrc);

      *pDest       = (U8)UnicodeChar;
      *(pDest + 1) = (U8)(UnicodeChar >> 8);
      pSrc += Len;
      NumChars += Len;

    } else if (RemBytes < 0) {                 /* Padding ? */
      *pDest       = 0xff;
      *(pDest + 1) = 0xff;
    } else {
      Len = 1;
    }
    pDest    += 2;
    RemBytes--;
  } while (--NumBytes);
  return NumChars;
}

/*********************************************************************
*
*       _IsInvalidChar
*
*  Function description:
*    
*
*/
static int _IsInvalidChar(U16 UnicodeChar) {
  char c;

  c = (char)UnicodeChar;
  switch(c) {
  case '+':
  case ',':
  case ';':
  case '=':
  case '[':
  case ']':
    return 1;
  default:
    break;
  }
  return 0;
}


/*********************************************************************
*
*       _CheckFilename
*
*  Function description:
*    
*
*/
static int _CheckFilename(const char * sFileName, unsigned Len) {
  int LenChar;
  do {
    U16 UnicodeChar;
    LenChar = GET_CHAR_SIZE(sFileName);
    UnicodeChar = GET_CHAR_CODE(sFileName);
    if (_IsInvalidChar(UnicodeChar)) {
      return -1;
    }
    sFileName += LenChar;
  } while (--Len);
  return 0;

}


/*********************************************************************
*
*       _ComputeLongDirEntry
*
*
*/
static void _ComputeLongDirEntry(FS_FAT_DENTRY * pDirEntry, const char * sLongName, int Len, int CurrentIndex, U8 CheckSum) {
  int NumDirEntries;
  unsigned Off;
  int NumChars;

  if (Len == 0) {
    Len = FS_STRLEN(sLongName);
  }
  NumDirEntries = _CalcNumLongEntries(sLongName, Len);
  FS_MEMSET(pDirEntry, 0, sizeof(FS_FAT_DENTRY));
  /* Ordinal. Or 0x40 for last (first) entry) */
  pDirEntry->data[0] = (U8)CurrentIndex;
  if (CurrentIndex == NumDirEntries) {
    pDirEntry->data[0] |= 0x40;
  }

  pDirEntry->data[11] = FS_FAT_ATTR_LONGNAME;         /* Attributes. Must be long file name */
  pDirEntry->data[13] = CheckSum;
  if (Len == 0) {
    Len = STRLEN(sLongName);
  } else {
    Len = STRNLEN(sLongName, Len);
  }
  /* Write file name */
  Off       = (CurrentIndex -1) * 13;
  NumChars  = WALK_2_CHAR(sLongName, Off);
  NumChars += _WriteName(&pDirEntry->data[1],  sLongName + NumChars, 5, Len - Off);
  NumChars += _WriteName(&pDirEntry->data[14], sLongName + NumChars, 6, Len - Off -  5);
  NumChars  = _WriteName(&pDirEntry->data[28], sLongName + NumChars, 2, Len - Off - 11);
}

/*********************************************************************
*
*       _ToUpper
*
*/
static U16 _ToUpper(U16 c) {
  if ((c >= 'a') && (c <= 'z')) {
    c &= 0xdf;
  }
#if FS_FAT_SUPPORT_UTF8
  else if (c > 0x7f) {
    unsigned i;
    for (i = 0; i < COUNTOF(_aLower2UpperTable); i++) {
      if (c == _aLower2UpperTable[i].LowerCase) {
        c = _aLower2UpperTable[i].UpperCase;
        break;
      };
    }
  }
#endif
  return c;
}

/*********************************************************************
*
*       _CompareChar
*
*  Return value
*    0        Equal
*    1        Not equal
*/
static char _CompareChar(const U8 * p0, const U8 * p1, unsigned NumBytes) {
  U16 c0, c1;
  do {
    c0  = *p0;
    c0 |= (*(++p0) << 8);
    c1  = *p1;
    c1 |= (*(++p1) << 8);
    c0 = _ToUpper(c0);
    c1 = _ToUpper(c1);
    if (c0 != c1) {
      return 1;
    }
    p0++;
    p1++;
  } while (--NumBytes);
  return 0;               /* Equal */
}

/*********************************************************************
*
*       _CompareLongDirEntry
*
*  Function description
*    Compares the long file part which is stored in short filename.
*    These are 13 double-byte characters stored in the 32 byte directory entry.
*    The directory entry stores the filename in 3 blocks:
*    0
*
*  Return value
*    0        Equal
*    1        Not equal
*/
static char _CompareLongDirEntry(FS_FAT_DENTRY * pDirEntry0, FS_FAT_DENTRY * pDirEntry1) {
  //
  // Check if the indices are not equal, we immediately return.
  //
  if (pDirEntry0->data[0] != pDirEntry1->data[0]) {
    return 1;
  }
  //
  // If indices are equal, we check all the UNICODE chars in the long dir entry.(if possible upper case).
  //
  if (_CompareChar((U8 *)&pDirEntry0->data[1], (U8 *)&pDirEntry1->data[1], 5)) {
    return 1;                 /* Not equal */
  }
  return _CompareChar((U8 *)&pDirEntry0->data[14], (U8 *)&pDirEntry1->data[14], 9);
}

/*********************************************************************
*
*       _CalcCheckSum
*
*  Return value
*
*/
static U8 _CalcCheckSum(FS_83NAME * pShortName) {
  const char * sShortName;
  U8 Sum;
  int i;

  sShortName = &pShortName->ac[0];
  Sum = 0;
  for (i = 0; i < 11; i++) {
    if (Sum & 1) {
      Sum = (Sum >> 1) | 0x80;
    } else {
      Sum >>= 1;
    }
    Sum += *sShortName++;
  }
  return Sum;
}

/*********************************************************************
*
*       _MarkIndexAsUsed
*
*/
static void _MarkIndexAsUsed(U8 * pBase, unsigned Index) {
  U8   Mask;
  U8 * pData;

  Mask    = 1 << (Index & 7);
  if(Index >= FS_FAT_LFN_BIT_ARRAY_SIZE) {
    return;
  }
  pData   = pBase + (Index >> 3);
  *pData |= (unsigned)Mask;    // Mark block as allocated
}

/*********************************************************************
*
*       _IsIndexUsed
*
*/
static char _IsIndexUsed(U8 * pBase, unsigned Index) {
  U8   Mask;
  U8 * pData;

  Mask  = 1 << (Index & 7);
  pData = pBase + (Index >> 3);
  return *pData & Mask;
}

/*********************************************************************
*
*       _FindCharPos
*
*/
static int _FindCharPos(const char *s, char c, int MaxNumChars2Check) {
  int Pos = 0;

  do {
    if (*s == c) {
      return Pos;
    }
    Pos++;
    if (Pos >= MaxNumChars2Check) {
      break;
    }
  } while (*s++);
  return -1;
}

/*********************************************************************
*
*       _atoi
*
*/
static unsigned _atoi(const char * sBase, unsigned NumDigits) {
  unsigned Number = 0;
  
  do {
    Number *= 10;
    Number += (*sBase++ - '0');
  } while(--NumDigits);
  return Number;
}

/*********************************************************************
*
*       _FindDirEntry83
*
*  Description:
*    Find the directory entry in the specified directory
*
*  Parameters:
*    pVolume     - Volume information
*    pEntryName  - Directory entry name; if zero, return the root directory.
*
*  Return value:
*    != NULL     - pointer to directory entry (in the smart buffer)
*    NULL        - Entry not found
*/
static int _FindFreeIndex(FS_VOLUME * pVolume, FS_SB * pSB, FS_83NAME * pEntryName, U32 DirStart, U8 * paBitField, unsigned StartIndex) {
  FS_FAT_DENTRY * pDirEntry;
  FS_DIR_POS      DirPos;
  unsigned        i;
  unsigned        NumDigits = 0;
  unsigned        Index;
  int             TildePos;

  /* Read directory */
  FS_FAT_InitDirEntryScan(&pVolume->FSInfo.FATInfo, &DirPos, DirStart);
  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
    if (pDirEntry == (FS_FAT_DENTRY *)NULL) {
      break;
    }
    if (pDirEntry->data[0] == 0) {
      break;  /* No more entries. Not found. */
    }
    TildePos = _FindCharPos((const char *)&pDirEntry->data[0], '~', 8);
    if (TildePos != -1) {
      NumDigits = 8 -TildePos - 1;
      if ((FS_MEMCMP(&pDirEntry->data[0],                       &pEntryName->ac[0],                       8 - NumDigits - 1) == 0) &&
          (FS_MEMCMP(&pDirEntry->data[DIR_ENTRY_OFF_EXTENSION], &pEntryName->ac[DIR_ENTRY_OFF_EXTENSION], 3) == 0) &&
          (pDirEntry->data[TildePos] == '~')) { /* Name does match */
        Index = _atoi((const char *)&pDirEntry->data[TildePos + 1], NumDigits) - StartIndex;
        _MarkIndexAsUsed(paBitField, Index);
      }
    }
    FS_FAT_IncDirPos(&DirPos);
  } while (1);
  for (i = 0; i < FS_FAT_LFN_BIT_ARRAY_SIZE; i++) {
    if (_IsIndexUsed(paBitField, i) == 0){
      return i + StartIndex;
    }
  }
  return -1;
}

/*********************************************************************
*
*       _GenerateShortEntry
*
*  Function description
*    Generates a short directory entry from a long file name.
*
*    The conversion scheme is done acc. to

*/
static void _GenerateShortEntry(FS_83NAME * pShortName, const char * sLongName, int Index) {
  int NumDigits;
  int i;
  U8    c;
  const char * s;
  char * sShortName;
  char HasExtension;

  if (FS_FAT_Make83Name(pShortName, sLongName, 0)) {
    HasExtension = 0;
    sShortName = &pShortName->ac[0];
    FS_MEMSET(sShortName, 0x20, 11);       /* Initialize with spaces */
    /* Compute the number of characters for the trailing index */
    NumDigits = 1;
    i = Index;
    while (i >= 10) {
      NumDigits++;
      i /= 10;
    }

    /* Copy name without extension */
    for (i = 0; i < 8 - NumDigits -1;) {
      GET_CHAR(c, sLongName);

      if (c == 0) {
        break;
      }
      if (c == '.') {
        break;
      }
      if (FS_FAT_IsValidShortNameChar(c)) {
        *sShortName++ = c;
        i++;
      }
    }

    /* Add index */
    *sShortName++ = '~';
    i = NumDigits;
    do {
      c = '0' + (char)(Index % 10);
      *(sShortName + i - 1) = c;
      Index /= 10;
    } while (--i);
    sShortName += NumDigits;
    /* Copy extension */
    s = sLongName;
    do {
      sLongName = s + 1;
      s = FS__strchr(sLongName, '.');
      if (s) {
        HasExtension = 1;
      }
    } while (s);

    if (HasExtension) {
      for (i = 0; i < 3;) {
        c = *sLongName++;
        c = FS_TOUPPER(c);
        if (c == 0) {
          break;
        }
        if (FS_FAT_IsValidShortNameChar(c)) {
          *sShortName++ = c;
          i++;
        }
      }
    }
  }
}

/*********************************************************************
*
*       _SetDirPosIndex
*
*  Description:
*    Increments/Decrements the position of DirPos
*/
static void _SetDirPosIndex(FS_DIR_POS * pDirPos, int Pos) {
  pDirPos->DirEntryIndex += Pos;
}

/*********************************************************************
*
*       _LFN_ReadDirEntryInfo
*/
static int _LFN_ReadDirEntryInfo(FS__DIR * pDir, FS_DIRENTRY_INFO * pDirEntryInfo, FS_SB * pSB) {
  FS_FAT_DENTRY  * pDirEntry;
  FS_FAT_INFO    * pFATInfo;
  FS_VOLUME      * pVolume;
  int              r;
  U32              DirIndex;

  pVolume      = pDir->pVolume;
  pFATInfo     = &pVolume->FSInfo.FATInfo;
  DirIndex     = pDir->DirEntryIndex;
  r            = -1;
  if (DirIndex == 0) {
    FS_FAT_InitDirEntryScan(pFATInfo, &pDir->DirPos, pDir->FirstCluster);
  }

  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &pDir->DirPos);
    FS_FAT_IncDirPos(&pDir->DirPos);
    if (pDirEntry == 0) {
      break;
    }
    if (pDirEntry->data[0] == 0x00) {         /* Last entry found ? */
      break;
    }
    if (pDirEntry->data[0] != (U8)0xE5) { /* not a deleted file */
      U8 Attrib;
      Attrib = pDirEntry->data[11];
      if (Attrib != FS_FAT_ATTR_VOLUME_ID) {
        if (Attrib != (FS_FAT_ATTR_LONGNAME)) { /* Also not a long entry, so it is a valid entry */
          FS_FAT_CopyShortName(pDirEntryInfo->sFileName, (const char*)&pDirEntry->data[0], pDirEntryInfo->SizeofFileName);
          FS_FAT_CopyDirEntryInfo(pDirEntry, pDirEntryInfo);
          r = 0;
          pDir->DirEntryIndex++;
          break;
        } else {
          int NumEntries;
          int Index;
          U8 CheckSum;
          U8 CalcCheckSum;
          char  IsDifferent;
          NumEntries   = pDirEntry->data[0] & 0x3f;
          Index        = NumEntries;
          CheckSum     = pDirEntry->data[13];
          IsDifferent  = 0;
          while(--Index) {
            pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &pDir->DirPos);
            if (pDirEntry == 0) {
              IsDifferent = 1;
              break;
            }
            if (pDirEntry->data[13] != CheckSum) {
              IsDifferent = 1;
            }
            FS_FAT_IncDirPos(&pDir->DirPos);
          }
          pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &pDir->DirPos);
          CalcCheckSum = _CalcCheckSum((FS_83NAME *)pDirEntry);
          if ((IsDifferent == 0) && (CalcCheckSum == CheckSum)) {
            int    NumBytesCopied;
            int    DirEntryLen;
            U8     UseShortName;
            char * p;
            Index          = NumEntries - 1;
            NumBytesCopied = 0;
            DirEntryLen    = 0;
            UseShortName   = 0;
            p              = pDirEntryInfo->sFileName;
            if (p) {
              do {
                _SetDirPosIndex(&pDir->DirPos, -1);
                pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &pDir->DirPos);
                /* Check if we exceed the maximum of DirName size */
                if ((DirEntryLen + _GetNumCharInEntry(&pDirEntry->data[1])) > (pDirEntryInfo->SizeofFileName - 1)) {
                  /* We cannot process this long file name */
                  _SetDirPosIndex(&pDir->DirPos, -Index);
                  UseShortName = 1;
                  break;
                }
                NumBytesCopied = _CopyLongName(p, &pDirEntry->data[1]);
                p             += NumBytesCopied;
                DirEntryLen   += NumBytesCopied;
              } while (Index--);
            }
            _SetDirPosIndex(&pDir->DirPos, NumEntries);
            pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &pDir->DirPos);
            if (UseShortName) {
              FS_FAT_CopyShortName(pDirEntryInfo->sFileName, (const char*)&pDirEntry->data[0], pDirEntryInfo->SizeofFileName);
            } else {
              if (p) {
                *p = 0;
              }
            }
            FS_FAT_CopyDirEntryInfo(pDirEntry, pDirEntryInfo);
            r = 0;
            pDir->DirEntryIndex++;
            _SetDirPosIndex(&pDir->DirPos, 1);
            break;
          }
        }
      }
    }
  } while (1);
  return r;
}

/*********************************************************************
*
*       _LFN_FindDirEntry
*/
static FS_FAT_DENTRY * _LFN_FindDirEntry(FS_VOLUME * pVolume, FS_SB * pSB, const char * sLongName, int Len, U32 DirStart, U8 AttrRequired, I32 * pLongDirEntryIndex) {
  FS_FAT_DENTRY         DirEntry;
  FS_FAT_DENTRY       * pDirEntry;
  int                   NumEntries;
  int                   CurrentIndex;
  int                   LastIndex;
  FS_FAT_INFO         * pFATInfo;
  FS_DIR_POS            DirPos;
  U8                    CheckSum;
  I32                   LongDirEntryIndex;

  pFATInfo = &pVolume->FSInfo.FATInfo;


  Len                 = _CorrectTrail(sLongName, Len);
  LastIndex           = -1;     /* Invalidate */
  NumEntries          = _CalcNumLongEntries(sLongName, Len);
  CurrentIndex        = NumEntries;
  CheckSum            = 0;
  LongDirEntryIndex   = -1;
  /* Read directory */
  FS_FAT_InitDirEntryScan(pFATInfo, &DirPos, DirStart);
  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
    if (!pDirEntry) {
      break;
    }
    if (pDirEntry->data[0] == 0) {
      pDirEntry = (FS_FAT_DENTRY*)NULL;
      break;  /* No more entries. Not found. */
    }

    if (pDirEntry->data[0] == 0xE5) {
      FS_FAT_IncDirPos(&DirPos);
      continue;  // Skip deleted directory entries.
    }

    if (CurrentIndex != LastIndex) {
      if (CurrentIndex) {
        _ComputeLongDirEntry(&DirEntry, sLongName, Len, CurrentIndex, 0);
        LastIndex = CurrentIndex;
        //
        // If the long name len in character is a multiple of 13 (such as "1234567890.12"),
        // The long name is not padded. This means that if we have a longer entry,
        // we need to skip all DirEntries until after we find a short one or the last long one.
        //
        if (0 == (Len % 13)) {
          if (pDirEntry->data[0] > 0x40 + NumEntries) {   // Is this entry is too long for what we are looking for ?
            do {
              if (pDirEntry->data[0] == 0) {
                return NULL;      // End of directory, file not found
              }
              if (pDirEntry->data[11] != 0xF) {
                break;
              }
              FS_FAT_IncDirPos(&DirPos);
              pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
            } while (1);
            FS_FAT_IncDirPos(&DirPos);    // Skip one more
            CurrentIndex = NumEntries;           /* Start over */
            continue;
          }
        }
      }
    }
    //
    // Check if the DirEntry matches. For a long name with n characters,
    // We need to check (n +12) / 13 Long-DirEntry
    // And last one Short Entry
    //
    if (CurrentIndex) {
      //
      // Check DirEntry as part of Long name
      //
      char IsDifferent;
      IsDifferent = _CompareLongDirEntry(pDirEntry, &DirEntry);
      if (CurrentIndex == NumEntries) {
        CheckSum = pDirEntry->data[13];
        LongDirEntryIndex = DirPos.DirEntryIndex;
      } else {
        if (CheckSum != pDirEntry->data[13]) {
          IsDifferent = 1;
        }
      }
      if (IsDifferent == 0) { /* Name does match */
        CurrentIndex--;
      } else {
        CurrentIndex = NumEntries;           /* Start over */
      }
    } else {
      //
      // Long name O.K., now check short name as well
      //
      U8 CheckSumShort;
      CheckSumShort = _CalcCheckSum((FS_83NAME*)pDirEntry);
      if ((CheckSumShort != CheckSum) || ((pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] & AttrRequired) != AttrRequired)) {
        CurrentIndex = NumEntries;           /* Start over */
      } else {
        /* Success ! We have found a matching long entry */
        if (pLongDirEntryIndex) {
          *pLongDirEntryIndex = LongDirEntryIndex;
        }
        return pDirEntry;
      }
    }
    FS_FAT_IncDirPos(&DirPos);
  } while (1);
  return NULL;         /* Not found */
}

/*********************************************************************
*
*       _LFN_DelLongEntry
*/
static void _LFN_DelLongEntry(FS_VOLUME * pVolume, FS_SB * pSB, U32 DirStart, I32 LongDirEntryIndex) {
  FS_DIR_POS            DirPos;
  FS_FAT_DENTRY       * pDirEntry;
  int                   NumShortEntries;

  if (LongDirEntryIndex >= 0) {    /* Delete only if it is a long directory entry */
    /* Goto first directory entry */
    FS_FAT_InitDirEntryScan(&pVolume->FSInfo.FATInfo, &DirPos, DirStart);
    do {
      pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
      FS_FAT_IncDirPos(&DirPos);
    } while ((int)DirPos.DirEntryIndex <= LongDirEntryIndex);

    /* Calc number of short entries for this long entry */
    NumShortEntries = (pDirEntry->data[0] & 0x3f);

    /* Delete entries */
    do {
      pDirEntry->data[0] = 0xE5;                        /* Mark entry as deleted */
      FS__SB_MarkDirty(pSB);
      pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
      FS_FAT_IncDirPos(&DirPos);
    } while (--NumShortEntries);
  }
}

/*********************************************************************
*
*       _LFN_CreateDirEntry
*
*
*  Function description
*    Creates a long directory entry.
*
*  Return value:
*    NULL      Error, could not create long file name entry
*    != NULL   Pointer to "main" directory entry (of the short name)
*
*  Notes
*    (1) Order of entries
*        A long directory name consists of a number of entries making up the long name,
*        immediately followed by the short name.
*    (2) Finding a unique short name
*        The short name needs to be unique in a directory.
*        We can build different short names (basically by adding a number).
*        The strategy is to try the short names until we find one that does
*        is unique.
*    (3) Finding an empty slot
*        The important point is that all directory entries (n long + 1 short)
*        are adjacent. We therefor need to look for n+1 adjacent, unused entries.
*/
static FS_FAT_DENTRY * _LFN_CreateDirEntry(FS_VOLUME * pVolume, FS_SB * pSB, const char * pFileName, U32 DirStart, U32 ClusterId, U8 Attribute, U32 Size, U16 Time, U16 Date) {
  int               NumLongEntries;   /* Number of required directory entries */
  int               FreeEntryCnt;    /* Number of empty, adjacent directory entries */
  FS_DIR_POS        DirPos;
  FS_DIR_POS        DirPosStart;     /* Remember directory position of first empty entry */
  FS_FAT_INFO     * pFATInfo;
  FS_FAT_DENTRY   * pDirEntry;
  FS_83NAME         ShortEntry;
  U8                CheckSum;
  int               Index;
  int               Len;
  int               FreeIndex;
  U8                aBitField[(FS_FAT_LFN_BIT_ARRAY_SIZE + 7) >> 3];

  if (STRLEN(pFileName) > FS_FAT_MAX_DIRNAME) {
    return NULL;  /* file names greater than 255 characters can not be handled by Microsoft */
  }

  Len = _CorrectTrail(pFileName, 0);
  if (_CheckFilename(pFileName, Len) == -1) {
    return NULL;
  }
  pFATInfo    = &pVolume->FSInfo.FATInfo;
  NumLongEntries = _CalcNumLongEntries(pFileName, Len);
  //
  // Find short directory name that has not yet been taken (Note 2)
  //
  Index = 0;
  while (1) {
    FS_MEMSET(aBitField, 0, sizeof(aBitField));
    _GenerateShortEntry(&ShortEntry, pFileName, Index);
    //
    // Look for a free index number for the short name
    //
    FreeIndex = _FindFreeIndex(pVolume, pSB, &ShortEntry, DirStart, aBitField, Index);    
    if (FreeIndex >= 0) {
      //
      // We find an entry, generate the real short name
      //
      _GenerateShortEntry(&ShortEntry, pFileName, FreeIndex);
      break;
    }
    if (Index >= FS_FAT_LFN_MAX_SHORT_NAME) {
      return NULL;          /* We give up. All short names seem to be taken */
    }
    Index += FS_FAT_LFN_BIT_ARRAY_SIZE;
  }
  /*
   * Read directory, trying to find an empty slot (Note 3)
   */
  FS_FAT_InitDirEntryScan(pFATInfo, &DirPos, DirStart);
  FreeEntryCnt = 0;
  do {
    U8 c;
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
    if (pDirEntry == NULL) {
      /*
       * Grow directory if possible
       */
      if ((DirStart == 0) && (pFATInfo->RootEntCnt)) {
        /* Root directory of FAT12/16 medium can not be increased */
        FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_LFN_CreateDirEntry: Root directory too small.\n"));
        return NULL;                  /* Can not create, directory is full */
      } else {
        U32 NewCluster;
        U32 LastCluster;
        LastCluster = FS_FAT_FindLastCluster(pVolume, pSB, DirPos.Cluster, (U32*)NULL);
        NewCluster  = FS_FAT_AllocCluster   (pVolume, pSB, LastCluster, NULL);
        FS__SB_Flush(pSB);
        if (NewCluster) {
          U32 DirSector;
          int SectorNo;
          /*
           * Clean new directory cluster  (Fill with 0)
           */
          FS_MEMSET(pSB->pBuffer, 0x00, pFATInfo->BytesPerSec);
          DirSector = FS_FAT_ClusterId2SectorNo(pFATInfo, NewCluster);
          for (SectorNo = pFATInfo->SecPerClus - 1; SectorNo >= 0; SectorNo--) {
            FS__SB_SetSector(pSB, DirSector + SectorNo, FS_SB_TYPE_DIRECTORY);
            FS__SB_Write(pSB);
          }
          pDirEntry = (FS_FAT_DENTRY *)pSB->pBuffer;
        } else {
          FS_DEBUG_ERROROUT((FS_MTYPE_FS, "FS_FAT_LFN_CreateDirEntry: Disk is full"));
          return NULL;
        }
      }
    }
    c = pDirEntry->data[0];
    if ((c == 0) || (c == 0xE5)) {       /* Is this entry free ? */
      if (FreeEntryCnt == 0) {
        DirPosStart = DirPos;
      }
      if (FreeEntryCnt++ == NumLongEntries) {
        break;                  /* We found sufficient entries */
      }
    } else {
      FreeEntryCnt = 0;
    }
    FS_FAT_IncDirPos(&DirPos);
  } while (1);
  /*
   * Create long file name directory entry
   */
  CheckSum = _CalcCheckSum(&ShortEntry);
  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPosStart);
    if (pDirEntry == NULL) {
      return NULL;
    }
    _ComputeLongDirEntry(pDirEntry, pFileName, Len, NumLongEntries, CheckSum);
    FS__SB_MarkDirty(pSB);
    FS_FAT_IncDirPos(&DirPosStart);
  } while (--NumLongEntries);
  /*
   * Create short directory entry
   */
  pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPosStart);
  FS_FAT_WriteDirEntry83(pDirEntry, &ShortEntry, ClusterId, Attribute, Size, Time, Date);
  FS__SB_MarkDirty(pSB);
  return pDirEntry;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_SupportLFN
*
*/
void FS_FAT_SupportLFN(void) {
  FS_LOCK_SYS();
  FAT_pDirEntryAPI = &FAT_LFN_API;
  FS_UNLOCK_SYS();
}


/*********************************************************************
*
*       FS_FAT_SupportLFN
*
*/
void FS_FAT_DisableLFN(void) {
  FS_LOCK_SYS();
  FAT_pDirEntryAPI = &FAT_SFN_API;
  FS_UNLOCK_SYS();
}

/*********************************************************************
*
*       Public const
*
**********************************************************************
*/
const FAT_DIRENTRY_API FAT_LFN_API = {
  _LFN_ReadDirEntryInfo,
  _LFN_FindDirEntry,
  _LFN_CreateDirEntry,
  _LFN_DelLongEntry
};


/*************************** End of file ****************************/
