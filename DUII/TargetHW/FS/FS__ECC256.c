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
File        : FS__ECC256.c
Purpose     : ECC functions (primarily for NAND driver)
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       Includes
*
**********************************************************************
*/

#include "FS_Int.h"


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CalcParity32
*
*  Function description:
*    Computes the parity of a 32-bit item.
*    Returns 0 for even parity, 1 for odd parity.
*
*  Add. notes:
*    Most compilers will (and should) inline this code in higher optimization levels
*/
static U32 _CalcParity32(U32 Data) {
  Data = (Data >> 16) ^ Data;           // Reduce 32 bits to 16 bits
  Data = (Data >>  8) ^ Data;           // Reduce 16 bits to 8 bits
  Data = (Data >>  4) ^ Data;           // Reduce 8 bits to 4 bits
  Data = (Data >>  2) ^ Data;           // Reduce 4 bits to 2 bits
  Data = (Data >>  1) ^ Data;           // Reduce 2 bits to 1 bit
  return Data & 1;
}


/*********************************************************************
*
*       _ParityToECC
*
*  Function description:
*    Compute the ECC Pn bits (located at odd bit positions)
*/
FS_OPTIMIZE
static U32 _ParityToECC(U32 ParLo, U32 ParHi) {
  U32 ecc;
  U32 Necc;
  ecc  = _CalcParity32(ParLo & 0xAAAAAAAAUL) << 19;   // p1
  ecc |= _CalcParity32(ParLo & 0xCCCCCCCCUL) << 21;   // p2
  ecc |= _CalcParity32(ParLo & 0xF0F0F0F0UL) << 23;   // p4
  ecc |= _CalcParity32(ParLo & 0xFF00FF00UL) << 1;    // p8
  ecc |= _CalcParity32(ParLo & 0xFFFF0000UL) << 3;    // p16

  ecc |= (ParHi & (1 << 0)) << 5;                   // p32
  ecc |= (ParHi & (1 << 1)) << 6;                   // p64
  ecc |= (ParHi & (1 << 2)) << 7;                   // p128
  ecc |= (ParHi & (1 << 3)) << 8;                   // p256
  ecc |= (ParHi & (1 << 4)) << 9;                   // p512
  ecc |= (ParHi & (1 << 5)) <<10;                   // p1024
  //
  // Compute the even bits of the ECC: Pn' = Pn ^ P;
  //
  Necc = ecc >> 1;
  if (_CalcParity32(ParLo)) {
    Necc ^= 0x00545555UL;
  }
  ecc |= Necc;
  return ecc ^ 0xFCFFFFUL;      // Note: Bits 16 and 17 are not used, therefor 0
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS__ECC256_Calc
*
*  Function description:
*    Calculates the ECC on a given 256 bytes stripe.
*/
FS_OPTIMIZE
U32 FS__ECC256_Calc(const U32 * pData) {
  U32 i = 0;
  U32 ParLo  = 0;       // Parity info for low bits 0..4
  U32 Par32  = 0;       // Parity info for bit 5  (A2 == 1)
  U32 Par64  = 0;       // Parity info for bit 6  (A3 == 1)
  U32 Par128 = 0;       // Parity info for bit 7  (A4 == 1)
  U32 ParHi  = 0;       // Parity info for high bits 8..10
  //
  // Load all data as words and xor into the variables.
  //
  do {
    U32 Data;
    U32 Par = 0;
    Data = *pData++; Par ^= Data;
    Data = *pData++; Par ^= Data; Par32 ^= Data;
    Data = *pData++; Par ^= Data;                Par64 ^= Data;
    Data = *pData++; Par ^= Data; Par32 ^= Data; Par64 ^= Data;
    Data = *pData++; Par ^= Data;                               Par128 ^= Data;
    Data = *pData++; Par ^= Data; Par32 ^= Data;                Par128 ^= Data;
    Data = *pData++; Par ^= Data;                Par64 ^= Data; Par128 ^= Data;
    Data = *pData++; Par ^= Data; Par32 ^= Data; Par64 ^= Data; Par128 ^= Data;
    ParLo ^= Par;
    //
    // Compute High parity bits
    //
    ParHi ^= i * _CalcParity32(Par);
  } while (++i < 8);
  ParHi <<= 3;
  ParHi |= _CalcParity32(Par32);
  ParHi |= _CalcParity32(Par64)  << 1;
  ParHi |= _CalcParity32(Par128) << 2;
  return _ParityToECC(ParLo, ParHi);
}


/*********************************************************************
*
*       FS__ECC256_Apply
*
*  Function description:
*    Uses the ECC values to correct the data if necessary
*    Works on a 256 bytes stripe
*
*  Return value:
*    0    - No error in data
*    1    - 1 bit error in data which has been corrected
*    2    - Error in ECC
*    3    - Uncorrectable error
*/
int FS__ECC256_Apply(U32 * pData, U32 eccRead) {
  U32 eccCalced;
  U32 eccXor;
  int i;
  int NumDiffBits;
  unsigned BitPos;
  unsigned Off;

  eccCalced = FS__ECC256_Calc(pData);
  eccXor = eccCalced ^ eccRead;
//  eccXor &= 0xFCFFFF;    // Eliminate unused bits 16 and 17. Not required, but older ECC had these bits set to 1, so we stay compatible with old value.
  if (eccXor == 0) {
    return 0;          // Both ECCs match, data is o.k. without correction
  }
  //
  // Count number of different bits in both ECCs
  //
  NumDiffBits = 0;
  for (i = 0; i < 24; i++) {
    if (eccXor & (U32)(1UL << i)) {
      NumDiffBits++;
    }
  }
  //
  // Check if this is a correctable error
  //
  if (NumDiffBits == 1) {
    return 2;        // Error in ECC
  }
  if (NumDiffBits != 11) {
    return 3;          // Uncorrectable error
  }
  //
  // Perform correction
  //
  BitPos =  ((eccXor >> 19) & 1)
          | (((eccXor >> 21) & 1) << 1)
          | (((eccXor >> 23) & 1) << 2)
          | (((eccXor >>  1) & 1) << 3)
          | (((eccXor >>  3) & 1) << 4);
  Off =      ((eccXor >>  5) & 1)
          | (((eccXor >>  7) & 1) << 1)
          | (((eccXor >>  9) & 1) << 2)
          | (((eccXor >> 11) & 1) << 3)
          | (((eccXor >> 13) & 1) << 4)
          | (((eccXor >> 15) & 1) << 5);
  *(pData + Off) ^= (U32)(1UL << BitPos);
  return 1;       // Error has been corrected
}

/*********************************************************************
*
*       FS__ECC256_Store
*
*  Function description:
*    Stores the 24 bit ecc in memory
*/
void FS__ECC256_Store(U8 * p, U32 ecc) {
  *p++ = (U8)(ecc >>  0);
  *p++ = (U8)(ecc >>  8);
  *p   = (U8)(ecc >> 16);
}

/*********************************************************************
*
*       FS__ECC256_Load
*
*  Function description:
*    Loads a 24 bit ecc from memory
*/
U32 FS__ECC256_Load(const U8 * p) {
  U32 ecc;
  ecc  = *p++;
  ecc |= ((U16)*p++) <<  8;  
  ecc |= ((U32)*p++) << 16;
  return ecc;
}

/*********************************************************************
*
*       FS__ECC256_IsValid
*
*  Function description:
*    Returns if a ECC appears to be valid. A valid ECC must have bits 16,17 == 0.
*/
int  FS__ECC256_IsValid(U32 ecc) {
  if ((ecc & 0x30000UL) == 0) {
    return 1;                     // Valid
  }
  return 0;                       // Invalid
}



/*************************** End of file ****************************/
