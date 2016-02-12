/*********************************************************************
*               SEGGER MICROCONTROLLER SYSTEME GmbH                  *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2007  SEGGER Microcontroller Systeme GmbH         *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS version: 3.32n                                         *
*                                                                    *
**********************************************************************

INTERNAL: Reference values - Flash configuration:
ARM7-AT91SAM7S                  47.9MHz, IAR EWARM V5.20,  ARM   Mode =>  1448 Loops/sec => 30.2 Loops/sec/MHz
ARM7-AT91SAM7S                  47.9MHz, IAR EWARM V5.20,  Thumb Mode =>  2213 Loops/sec => 46.2 Loops/sec/MHz
SH2A-7203 (write-thru cache)   200.0MHz, HEW4 V4.04                   =>  5678 Loops/sec => 28.4 Loops/sec/MHz
SH2A-7203 (write-back cache)   200.0MHz, HEW4 V4.04                   => 14748 Loops/sec => 73.7 Loops/sec/MHz
LH7A404 (fully cached)         200.0MHz, IAR EWARM V4.42,  ARM   Mode =>  9002 Loops/sec => 45.0 Loops/sec/MHz
CM3-STM32F103                   72.0MHz, IAR EWARM V5.301, Thumb Mode =>  4009 Loops/sec => 55.7 Loops/sec/MHz

----------------------------------------------------------------------
File    : OS_PerformanceTest.c
Purpose : Performance test program
          It checks the performance of the entire system, outputing one
          result value per second. Higher values are better.
          The program computes the first 1000 prime numbers in a loop.
          The value output is the number of times this could be performed
          in a second.
          Version 2.0, July-14 2008
----------------------------------------------------------------------
Reference values -  RAM configuration:
ARM926-EJS fully chached, 201.0MHz, IAR EWARM V5.20, Thumb Mode => 11834 Loops/sec => 58.9 Loops/sec/MHz
ARM926-EJS fully chached, 201.0MHz, IAR EWARM V5.20, ARM   Mode => 12497 Loops/sec => 62.2 Loops/sec/MHz
ARM7-TDMI                  47.9MHz, IAR EWARM V5.20, Thumb Mode =>  2656 Loops/sec => 55.4 Loops/sec/MHz
ARM7-TDMI                  47.9MHz, IAR EWARM V5.20, ARM   Mode =>  2646 Loops/sec => 55.2 Loops/sec/MHz

--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.H"
#include <stdio.h>
#include <stdlib.h>


char aIsPrime[1000];
int NumPrimes;

/*********************************************************************
*
*       _CalcPrimes
*/
static void _CalcPrimes(int NumItems) {
  int i;
  //
  // Mark all as potential prime numbers
  //
  memset(aIsPrime, 1, NumItems);
  //
  // 2 deserves a special treatment
  //
  for (i = 4; i < NumItems; i += 2) {
    aIsPrime[i] = 0;     // Cross it out: not a prime
  }
  //
  // Cross out multiples of every prime starting at 3. Crossing out starts at i^2.
  //
  for (i = 3; i * i < NumItems; i++) {
    if (aIsPrime[i]) {
      int j;

      j = i * i;    // The square of this prime is the first we need to cross out
      do {
        aIsPrime[j] = 0;     // Cross it out: not a prime
        j += 2 * i;          // Skip even multiples (only 3*, 5*, 7* etc)
      } while (j < NumItems);
    }
  }
  //
  // Count prime numbers
  //
  NumPrimes = 0;
  for (i = 2; i < NumItems; i++) {
    if (aIsPrime[i]) {
      NumPrimes++;
    }
  }
}

/*********************************************************************
*
*       _PrintDec
*/
static void _PrintDec(unsigned v) {
  int Digit;
  int r;

  Digit = 10;
  while (Digit < v) {
    Digit *= 10;
  }
  do {
    Digit /= 10;
    r = v / Digit;
    v -= r * Digit;
    putchar(r + '0');
  } while (v | (Digit > 1));
}

/*********************************************************************
*
*       _PrintResult
*/
static void _PrintResult(unsigned Cnt) {
  if (NumPrimes != 168) {
    puts("Error");
  } else {
    puts("Loops/sec:");
    _PrintDec(Cnt);
  }
  puts("\n");
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void);
void MainTask(void) {
  int Cnt;
  int TestTime;

  while(1) {
    Cnt = 0;
    TestTime = OS_GetTime32() + 1000;
    while ((TestTime - OS_GetTime()) >= 0) {
      _CalcPrimes(sizeof(aIsPrime));
      Cnt++;
    }
    _PrintResult(Cnt);
  }
}
