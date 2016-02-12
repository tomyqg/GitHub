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
File        : FS_CLib.c
Purpose     : File system's standard c-lib replacement routines
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_Int.h"


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__CLIB_atoi
*
*  Function description:
*    Convert string to int. The function stops with
*    the first character it cannot convert. It expects decimal numbers only.
*    It can handle +/- at the beginning and leading 0. It cannot handle
*    HEX or any other numbers.
*
*  Parameters:
*    s           - Pointer to a zero terminated string.
* 
*  Return value:
*    ==0         - In case of any problem or if the converted value is zero.
*    !=0         - Integer value of the converted string.
*/
int FS__CLIB_atoi(const char *s) {
  unsigned int base;
  const char   *t;
  signed char  sign;
  char         c;
  int          value;
  int          len;
  int          i;

  value = 0;
  //
  // Check for +/-
  //
  sign = 1;
  len = FS__CLIB_strlen(s);
  if (len <= 0) {
    return 0;
  }
  t = s;
  if (*t == '-') {
    t++;
    sign = -1;
  }
  else if (*t == '+') {
    t++;
  }
  //
  // Skip leading 0
  //
  len = FS__CLIB_strlen(t);
  if (len <= 0) {
    return 0;
  }
  while (*t == '0') {
    t++;
    len--;
    if (len <= 0) {
      break;
    }
  }
  //
  // Find end of number
  //
  for (i = 0; i < len; i++) {
    if (t[i] > '9') {
      break;
    }
    if (t[i] < '0') {
      break;
    }
  }
  len = i;
  if (len <= 0) {
      return 0;
  }
  //
  // Calculate base
  //
  base = 1;
  for (i = 1; i < len; i++) {
    base *= 10;
  }
  //
  // Get value
  //
  for (i = 0; i < len; i++) {
    c = t[i];
    if (c > '9') {
      break;
    }
    if (c < '0') {
      break;
    }
    c -= '0';
    value += c*base;
    base /= 10;
  }
  return sign * value;
}

/*********************************************************************
*
*       FS__CLIB_memcmp
*
*  Function description:
*    Compare bytes in two buffers
*
*  Parameters:
*    s1          - Pointer to first buffer.
*    s2          - Pointer to second buffer.
*    NumBytes    - Number of bytes to compare
* 
*  Return value:
*    0           - bytes are equal
*    1           - bytes are different
*/

int FS__CLIB_memcmp(const void *s1, const void *s2, unsigned NumBytes) {	
  const U8 * p1;
  const U8 * p2;

  p1 = (const U8*) s1;
  p2 = (const U8*) s2;

  while (NumBytes--) {
    if (*p1++ != *p2++) {
      return 1;             // Not equal
    }
  }
  return 0;                 // Equal
}

/*********************************************************************
*
*       FS__CLIB_memset
*
*  Function description:
*    FS internal function. Copy the value of c (converted to an unsigned
*    char) into each of the first n characters of the object pointed to 
*    by s.
*
*  Parameters:
*    s           - Pointer to an object.
*    c           - 'Character' value to be set.
*    n           - Number of characters to be set.
* 
*  Return value:
*    Value of s.
*/
void * FS__CLIB_memset(void *pData, int Fill, U32 NumBytes) {
  U8 * p;
  int  NumInts;

  p = (U8 *)pData;
  //
  // Write bytes until we are done or have reached an int boundary
  //
  while (NumBytes && ((sizeof(int) - 1) & (U32)p)) {
    *p++ = (U8)Fill;
    NumBytes--;
  }
  //
  // Write Ints
  //
  NumInts = (unsigned)NumBytes / sizeof(int);
  if (NumInts) {
    int FillInt;
    int *pInt;
    NumBytes &= (sizeof(int) - 1);
    if (sizeof(int) == 2) {        // May some compilers generate a warning at this line: Condition is always true/false
      FillInt = Fill * 0x101;      // May some compilers generate a warning at this line: Unreachable code
    } else if (sizeof(int) == 4) { // May some compilers generate a warning at this line: Condition is always true/false
      FillInt = Fill * 0x1010101UL;  // May some compilers generate a warning at this line: Unreachable code
    }
    pInt = (int*)p;
    //
    // Fill large amount of data at a time
    //
    while (NumInts >= 4) { 
      *pInt++ = FillInt;
      *pInt++ = FillInt;
      *pInt++ = FillInt;
      *pInt++ = FillInt;
      NumInts -= 4;
    }
    //
    // Fill one int at a time
    //
    while (NumInts) {
      *pInt++ = FillInt;
      NumInts--;
    }
    p = (U8*)pInt;
  }
  /* Fill the remainder byte wise */
  while (NumBytes) {
    *p++ = (U8)Fill;
    NumBytes--;
  }
  return pData;
}

/*********************************************************************
*
*             FS__CLIB_strncat
*
*  Function description:
*    Appends n characters from the array
*    pointed to by s2 to the array pointed to by s1.
*
*  Parameters:
*    s1          - Pointer to a character array.
*    s2          - Pointer to a character array.
*    n           - Number of characters to append
* 
*  Return value:
*    Value of s1.
*/
char * FS__CLIB_strncat(char *s1, const char *s2, U32 n)  {
  char *s;

  if (s1) {
    for (s = s1; *s != '\0'; ++s);       // find end of s1[]
    if (s2) {
      for (; (0 < n) && (*s2 != 0); --n) {  // copy at most n chars from s2[]
        *s++ = *s2++;
      }
      *s = '\0';
    }
  }
  return s1;
}


/*********************************************************************
*
*             FS__CLIB_strcat
*
*  Function description:
*    Adds characters from the array
*    pointed to by s2 to the array pointed to by s1.
*
*  Parameters:
*    s1          - Pointer to a character array.
*    s2          - Pointer to a character array.
* 
*  Return value:
*    Value of s1.
*/
char * FS__CLIB_strcat(char *s1, const char *s2) {
	char * s;

  if (s1) {
    for (s = s1; *s != '\0'; ++s);            // find end of s1[]
    if (s2) {
      for (; (*s = *s2) != '\0'; ++s, ++s2);    // copy s2[] to end
    }
  }
  return (s1);
}

/*********************************************************************
*
*       FS__CLIB_strcmp
*
*  Function description:
*    Compare the sring pointed to by s1 with the 
*    string pointed to by s2.
*
*  Parameters:
*    s1          - Pointer to a zero terminated string.
*    s2          - Pointer to a zero terminated string.
* 
*  Return value:
*    0           - bytes are equal
*    1           - bytes are different
*/
int FS__CLIB_strcmp(const char *s1, const char *s2)	{
  char c1, c2;
  do {
    c1 = *s1++;
    c2 = *s2++;
    if (c1 != c2) {
      return 1;             // Different
    }
  } while (c1);
  return 0;                 // Equal
}


/*********************************************************************
*
*      FS__CLIB_strcpy
*
*  Description:
*    Copy characters from the array
*    pointed to by s2 to the array pointed to by s1.
*
*  Parameters:
*    s1          - Pointer to a character array.
*    s2          - Pointer to a character array.
* 
*  Return value:
*    Value of s1.
*/
char * FS__CLIB_strcpy(char *s1, const char *s2) {
   char *p = NULL;
   if (s1) {
     p = s1;
     while(*s2 != 0) {
       *s1++  = *s2++;
     }
     *s1 = 0;
   }
   return p;
}


/*********************************************************************
*
*       FS__CLIB_strlen
*
*  Function description:
*    Compute the length of a string pointed to by s.
*
*  Parameters:
*    s           - Pointer to a zero terminated string.
* 
*  Return value:
*    Number of characters preceding the terminating 0.
*/
unsigned FS__CLIB_strlen(const char *s) {	
  unsigned Len;

  Len = 0;
  while (*s++) {
    Len++;
  }
  return Len;
}

/*********************************************************************
*
*       FS__CLIB_strncmp
*
*  Function description:
*    Compare no more than n characters from the
*    array pointed to by s1 to the array pointed to by s2.
*
*  Parameters:
*    s1          - Pointer to a character array.
*    s2          - Pointer to a character array.
*    n           - Number of characters to compare.
* 
*  Return value:
*    0           - bytes are equal
*    1           - bytes are different
*/
int FS__CLIB_strncmp(const char *s1, const char *s2, int n) {
  for (; n > 0; n--) {
    char c1, c2;
    c1 = *s1++;
    c2 = *s2++;
    if (c1 != c2) {
      return 1;             // Different
    }
    if (*s1 == '\0') {
      return 0;             // Equal
    }
  }
  return 0;                 // Equal
}


/*********************************************************************
*
*       FS__CLIB_strncpy
*
*  Function description:
*    Copy not more than n characters from the array
*    pointed to by s2 to the array pointed to by s1.
*
*  Parameters:
*    s1          - Pointer to a character array.
*    s2          - Pointer to a character array.
*    n           - Number of characters to copy.
* 
*  Return value:
*    Value of s1.
*/
char *FS__CLIB_strncpy(char *s1, const char *s2, U32 n) {
  char *s;
  
  for (s = s1; (0 < n) && (*s2 != '\0'); --n) {
    *s++ = *s2++;         // copy at most n chars from s2[]
  }
  for (; 0 < n; --n) {
    *s++ = '\0';
  }
  return s1;
}

/*********************************************************************
*
*       FS__CLIB_toupper
*
*  Function description:
*    yConvert a lowecase letter to a corresponding
*    uppercase letter. 
*
*  Parameters:
*    c           - Letter to convert.
* 
*  Return value:
*    Corresponding uppercase character.
*/
int FS__CLIB_toupper(int c) {
  if ((c >= 'a') && (c <= 'z')) {
    c &= 0xdf;
  }
  return c;
}

/*************************** End of file ****************************/
