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
File        : MMC_Drv.c
Purpose     : File system generic MMC/SD driver using SPI mode
---------------------------END-OF-HEADER------------------------------

Literature:
[1]  SD Specifications, Part 1, PHYSICAL LAYER, Simplified Specification Version 2.00, September 25, 2006
[2]  The MultiMediaCard System Specification Version 3.2
[3]  SD Specifications, Part 1, PHYSICAL LAYER Specification, Version 2.00, May 9, 2006
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include <stdio.h>

#include "FS_Int.h"
#include "MMC_X_HW.h"

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/

#ifdef FS_MMC_MAXUNIT
  #define NUM_UNITS FS_MMC_MAXUNIT
#else
  #define NUM_UNITS   1
#endif

#ifndef   FS_MMC_SUPPORT_HIGH_SPEED
  #define FS_MMC_SUPPORT_HIGH_SPEED           0                   // Support for high-speed mode for SD card in SPI mode is currently disabled
                                                                  // and shall not be activated yet. For some reason different SD-HC cards do not
                                                                  // answer or return with an illegal command which needs to be further investigated.
                                                                  // Such cards are: SanDisk extremeIII 16GB SDHC, extreme memory 8GB performance,
                                                                  //                 Transcend miniSD-HC 16GB
#endif

#ifndef    FS_MMC_NUM_RETRIES
  #define  FS_MMC_NUM_RETRIES      3
#endif

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/
/* Default sector size */
#define BYTES_PER_SECTOR       (512)

/* Time-out values. The maximum read/write timeout values are documented in
 * [1]: 4.6.2 Read, Write and Erase Timeout Conditions.
 */
#define NAC_CSD_MAX             (50000) /* max read cycles for CSD read */
#define TIME_NAC_READ_MAX       (100000) /* max read time (usec) */
#define TIME_NAC_WRITE_MAX      (250000) /* max write time (usec) */
#define TIME_NCR_MAX            (8)

/* hardware specific defines */
#define VOLT_LOWMIN             (1650)  /* Low Voltage mode minimum value */
#define VOLT_LOWMAX             (1950)  /* Low Voltage mode maximum value */
#define STARTUPFREQ              (400)  /* Max. startup frequency (KHz) */
/*
  The following constant defines the number of empty byte cycles that are transferred to
  the card before any command is issued. Referring to the specifications of SDA, this period
  must be at least 74 cycles, but it is recommended to send more that just the minimum value.
  The value here is a byte count so is multiplied by 8 to get cycle count.
*/
#define BOOTUPCYCLES            (10) /* Cycles sent to the card before initialization starts */
#define MAX_RETRIES             (5)

#define CARD_TYPE_MMC           (0)
#define CARD_TYPE_SD            (1)
#define CARD_TYPE_SDHC          (2)
/*********************************************************************
*
*       MMC/SD response tokens
*/
#define TOKEN_MULTI_BLOCK_WRITE_START     0xFC
#define TOKEN_MULTI_BLOCK_WRITE_STOP      0xFD
#define TOKEN_BLOCK_READ_START            0xFE                   /* read start token */
#define TOKEN_BLOCK_WRITE_START           0xFE                   /* write start token */
#define TOKEN_MULTI_BLOCK_READ_START      0xFE

/*********************************************************************
*
*       MMC/SD card commands
*/
#define CMD_GO_IDLE_STATE         0
#define CMD_SEND_OP_COND          1
#define CMD_ALL_SEND_CID          2
#define CMD_SET_REL_ADDR          3
#define CMD_SWITCH_FUNC           6
#define CMD_SELECT_CARD           7
#define CMD_SEND_IF_COND          8
#define CMD_SEND_CSD              9
#define CMD_SEND_CID             10
#define CMD_STOP_TRANSMISSION    12
#define CMD_SEND_STATUS          13
#define CMD_SET_BLOCKLEN         16
#define CMD_READ_SINGLE_BLOCK    17
#define CMD_READ_MULTIPLE_BLOCKS 18
#define CMD_WRITE_BLOCK          24
#define CMD_WRITE_MULTIPLE_BLOCK 25
#define CMD_ACMD_CMD             55
#define CMD_READ_OCR             58
#define ACMD_SEND_OP_COND        41
#define ACMD_SEND_SCR            51

#define CMD_LEN                   6

#define SD_SPEC_VER_100           0
#define SD_SPEC_VER_110           1
#define SD_SPEC_VER_200           2

/*********************************************************************
*
*       CSD register access macros
*/
#define CSD_STRUCTURE(pCSD)            _GetFromCSD(pCSD, 126, 127)
#define CSD_WRITE_PROTECT(pCSD)        _GetFromCSD(pCSD,  12,  13)
#define CSD_CCC_CLASSES(pCSD)          _GetFromCSD(pCSD,  84,  95)
#define CSD_FILE_FORMAT_GRP(pCSD)      _GetFromCSD(pCSD,  15,  15)
#define CSD_WRITE_BL_LEN(pCSD)         _GetFromCSD(pCSD,  22,  25)
#define CSD_R2W_FACTOR(pCSD)           _GetFromCSD(pCSD,  26,  28)
#define CSD_C_SIZE_MULT(pCSD)          _GetFromCSD(pCSD,  47,  49)
#define CSD_C_SIZE(pCSD)               _GetFromCSD(pCSD,  62,  73)
#define CSD_READ_BL_LEN(pCSD)          _GetFromCSD(pCSD,  80,  83)
#define CSD_TRAN_SPEED(pCSD)           (pCSD->aData[3])   // Same as, but more efficient than: _GetFromCSD(pCSD,  96, 103)
#define CSD_NSAC(pCSD)                 (pCSD->aData[2])   // Same as, but more efficient than: _GetFromCSD(pCSD, 104, 111)
#define CSD_TAAC(pCSD)                 (pCSD->aData[1])   // Same as, but more efficient than: _GetFromCSD(pCSD, 112, 119)
#define CSD_C_SIZE_V2(pCSD)            _GetFromCSD(pCSD,  48, 69)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {           /* CSD register structure */
  U8 aData[16];            /* Size is 128 bit */
} CSD;

typedef struct {
  U8           IsInited;
  U8           IsWriteProtected;
  U8           Unit;
  U8           CardType;
  U16          BytesPerSector;
  U32          NumSectors;
  U32          Nac;
  U32          Nwrite;
} MMC_INST;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
static U16 _CalcDataCRC16Dummy   (const U8 * p, unsigned NumBytes);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static MMC_INST * _apInst[NUM_UNITS];                                           // per Unit card info
static int        _NumUnits;
static U16 (*_pfCalcCRC)(const U8 * p, unsigned NumBytes) = _CalcDataCRC16Dummy;  // Function pointer for CRC check code for all cards

/*********************************************************************
*
*       static const
*
**********************************************************************
*/

/* Time value conversion factors for CSD TAAC and TRAN_SPEED values from
 * [1]: 5.3.2. Values here are 10x the spec values.
 */
static const U8 _aFactor[16] = {
  0,    /* 0: reserved - not supported */
  10,   /* 1 */
  12,   /* 2 */
  13,   /* 3 */
  15,   /* 4 */
  20,   /* 5 */
  25,   /* 6 */
  30,   /* 7 */
  35,   /* 8 */
  40,   /* 9 */
  45,   /* 10 */
  50,   /* 11 */
  55,   /* 12 */
  60,   /* 13 */
  65,   /* 14 */
  80    /* 15 */
};

/* Time unit conversion factors for CSD TAAC values from [1]: 5.3.2.
 * Values here are divisors that are 1/100 of the spec values (ie. the
 * result of dividing by them is 100 times larger than it should be).
 */
static const U32 _aUnit[8] = {
  10000000UL,  /* 0 -   1ns */
  1000000UL,   /* 1 -  10ns */
  100000UL,    /* 2 - 100ns */
  10000UL,     /* 3 -   1us */
  1000UL,      /* 4 -  10us */
  100UL,       /* 5 - 100us */
  10UL,        /* 6 -   1ms */
  1UL,         /* 7 -  10ms */
};

/* Transfer rate conversion factors for CSD TRAN_SPEED values from [1]: 5.3.2.
 * Values here are multipliers that are 1/10 the value needed to convert to
 * kbits/s.
 */
static const U32 _aRateUnit[4] = {
  10UL,        /* 0 - 100 kbits/s */
  100UL,       /* 1 -   1 Mbits/s */
  1000UL,      /* 2 -  10 Mbits/s */
  10000UL,     /* 3 - 100 Mbits/s */
};


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _GetBits
*
*  Function description
*    Returns a value from the bit field.
*/
static unsigned _GetBits(const U8 * pData, unsigned FirstBit, unsigned LastBit, unsigned NumBytesAvailable) {
  unsigned Off;
  unsigned OffLast;
  unsigned NumBytes;
  U32 Data;


  Off      = FirstBit / 8;
  OffLast  = LastBit / 8;
  NumBytes = OffLast - Off + 1;
  Off      = (NumBytesAvailable - 1) - OffLast;                      // Bytes are reversed in CSD
  Data = 0;
  //
  // Read data into 32 bits
  //
  do {
    Data <<= 8;
    Data |= pData[Off++];
  } while (--NumBytes);
  //
  // Shift and mask result
  //
  Data >>= (FirstBit & 7);
  Data &= (2 << (LastBit - FirstBit)) - 1;                // Mask out bits that are outside of given bit range
  return Data;
}


/*********************************************************************
*
*       _GetFromCSD
*
*  Function description
*    Returns a value from the CSD field. These values are stored in
*    a 128 bit array; the bit-indices documented in [1]: 5.3 CSD register, page 69
*    can be used as parameters when calling the function
*/
static unsigned _GetFromCSD(const CSD * pCSD, unsigned FirstBit, unsigned LastBit) {
  U32 Data;
  Data = _GetBits(pCSD->aData, FirstBit, LastBit, sizeof(pCSD->aData));
  return Data;
}

/*********************************************************************
*
*       _CalcCRC7
*
*  Function Description
*    Returns the 7 bit CRC generated with the 1021 polynom using a table.
*/
static U8 _CalcCRC7(U8 * pData, unsigned NumBytes) {
  U8       crc;
  unsigned iBit;
  unsigned iByte;
  U8       Data;

  crc=0;
  for (iByte = 0; iByte < NumBytes; iByte++) {
    Data = *pData++;
    for (iBit = 0;iBit < 8; iBit++) {
      crc <<= 1;
      if ((Data & 0x80)^(crc & 0x80))
      crc ^=0x09;
      Data <<= 1;
    }
  }
  crc = (crc << 1) | 1;
  return crc;
}

/*********************************************************************
*
*       _CalcDataCRC16ViaTable
*
*  Function Description
*    Returns the 16 bit CRC generated with the 1021 polynom using a table.
*    Using the table is about 10 times as fast as computing the CRC on a bit by bit basis,
*    which is the reason why it is used here.
*    The CRC algorithm is described in some detail in [1]: 4.5, page 40.
*
*  Notes
*    (1) Code verification
*        The table has been generated automatically, so typos can be ruled out.
*        Code verification is difficult if tables are used; but in this case,
*        the table can be verified also by running the "CRC Sample":
*        512 bytes with 0xFF data --> CRC16 = 0x7FA1
*/
static U16 _CalcDataCRC16ViaTable(const U8 * pData, unsigned NumBytes) {
  U16 Crc;

  Crc = FS_CRC16_Calc(pData, NumBytes, 0);
  return Crc;
}

/*********************************************************************
*
*       _CalcDataCRC16Dummy
*
*  Function Description
*    Returns a dummy value (0xFFFF) which indicates that CRC has not been computed.
*/
static U16 _CalcDataCRC16Dummy(const U8 * p, unsigned NumBytes) {
  FS_USE_PARA(p);
  FS_USE_PARA(NumBytes);
  return 0xFFFF;
}

/*********************************************************************
*
*       _CalcDataCRC16
*
*  Function Description
*    Is used to compute the 16-bit CRC for data.
*    It calls the actual computation routine via function pointer.
*    The function pointer is either the "Dummy" routine returning the
*    0xFFFF or a routine computing the correct 16-bit CRC
*/
static U16 _CalcDataCRC16(const void * p, unsigned NumBytes) {
  return _pfCalcCRC((const U8*)p, NumBytes);
}

/*********************************************************************
*
*       _SendEmptyCycles
*
*  Description:
*    After each transaction, the MMC card
*    needs at least one empty cycle phase. During this 8 clock cycle phase
*    data line must be at high level.
*
*  Parameters:
*    Unit                 - Device index number
*/
static void _SendEmptyCycles(U8 Unit, int n) {
  U8  c;

  c = 0xFF; /* data line must be kept high */
  for (; n > 0; n--) {
    FS_MMC_HW_X_Write(Unit, &c, 1);
  }
}

/*********************************************************************
*
*       _CheckR1
*
*  Description:
*    Read the R1 response,assert that no error occurred
*    and returns the response R1.
*    Bit definition of R1:
*      Bit 0:    In idle state
*      Bit 1:    Erase Reset
*      Bit 2:    Illegal command
*      Bit 3:    Communication CRC error
*      Bit 4:    Erase sequence error
*      Bit 5:    Address error
*      Bit 6:    Parameter error
*      Bit 7:    Always 0.
*
*  Parameters:
*    Unit                    - Device Index
*
*  Notes
*    (1) The response is as follows:
*        NCR bits of value 1 (up to 8 bits)
*        single byte, response format R1 (MMC spec [2]: 7.6.2)
*        The bit 7 is always 0, the other bits indicate errors if set.
*        In other words: The response consists of 8 - 16 bits.
*        The last 8 bits are relevant; of these bit 7 is always 0.
*
*
*
*/
static U8 _CheckR1(U8 Unit) {
  U8  Response;
  U8  LoopRem;

  /* Read NCR bits and response token */
  FS_MMC_HW_X_Read(Unit, &Response, 1);            // First byte is always NCR, never a valid response
  if (Response != 0xff) {
    LoopRem = 10;
  } else {
    LoopRem = 7;
  }
  do {
    FS_MMC_HW_X_Read(Unit, &Response, 1);          /* Note 1 */
    if (Response != 0xff) {
      break;
    }
    if (--LoopRem == 0) {
      return 0xff;                                 /* Error: More than 8 bytes NCR (0xff) */
    }
  } while (1);
  return Response;                        /* No error if 0 */
}

/*********************************************************************
*
*       _WaitUntilReady
*
*  Description:
*    Wait for the busy flag to be deactivated (DOUT == 1)
*
*  Parameters:
*    Unit              - Device Index
*
*  Return value:
*    0:     O.K.
*    else   Error condition (time out)
*
*
*/
static char _WaitUntilReady(MMC_INST * pInst) {
  U32 LoopRem;
  U8  aData[2];
  U8  Unit;

  Unit = pInst->Unit;
  /* Wait for the card to report ready (Note 2) */
  LoopRem = pInst->Nac ;     /* Timeout value in loops */
  do {
    FS_MMC_HW_X_Read(Unit, aData, 1);
    if (aData[0] == 0xFF) {
      return 0;                          /* O.K., Card is ready */
    }
  } while (LoopRem-- > 0);
  return 1;                              /* Time out error */
}

/*********************************************************************
*
*       _ExecCmdR1
*
*  Description:
*    MMC/SD driver internal function. Execute a command sequence with R1
*    response and return the card's response.
*
*  Parameters:
*    Unit            - Unit number.
*    Cmd             - Command index.
*    Arg             - Command argument.
*
*  Return value:
*    Card response token type R1.
*/
static unsigned char _ExecCmdR1(MMC_INST * pInst, unsigned char Cmd, U32 Arg) {
  U8 r;
  U8 Unit;
  U8 aCmdBuffer[CMD_LEN];
  Unit = pInst->Unit;
  //
  // Build setup command token (48 bit)
  //
  aCmdBuffer[0] = (U8)(0x40 | (Cmd & 0x3F));
  aCmdBuffer[1] = (U8)((Arg >> 24) & 0xFF);
  aCmdBuffer[2] = (U8)((Arg >> 16) & 0xFF);
  aCmdBuffer[3] = (U8)((Arg >>  8) & 0xFF);
  aCmdBuffer[4] = (U8)( Arg        & 0xFF);
  aCmdBuffer[5] = _CalcCRC7(&aCmdBuffer[0], 5);  /* SPI default mode, no use for CRC */

  /* Make sure the card is ready */
  r = _WaitUntilReady(pInst);
  if (r) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _ExecCmdR1: Timeout occurred when receiving the response.\n"));
    return 0xff;                           /* Time out waiting for ready */
  }
  FS_MMC_HW_X_Write(Unit, aCmdBuffer, CMD_LEN);
  r = _CheckR1(Unit);   /* Receive response */
  return r; /* return received response token */
}


/*********************************************************************
*
*       _ReadCSD
*
*  Description:
*    MMC/SD driver internal function.
*    Read the card's CSD (card specific data) register.
*
*  Parameters:
*    Unit      - Device index number
*
*  Return value:
*    ==0           - CSD has been read and all parameters are valid.
*    <0            - An error has occurred.
*/
static U8 _ReadCSD(MMC_INST * pInst, CSD * pCSD) {
  U8   a;
  U8   c;
  U32  to;
  U16  i;
  U8   r;
  U8   Unit;

  Unit = pInst->Unit;
  r = 0xff;
  _SendEmptyCycles(Unit, 1);
  /* Execute CMD9 (SEND_CSD) */
  FS_MMC_HW_X_EnableCS(Unit);  /* CS on */
  a = _ExecCmdR1(pInst, CMD_SEND_CSD, 0);  /* Do not handle clock & CS automatically */
  if (a != 0) {
    goto End;

  }
  /* wait for CSD transfer to begin */
  i = 0;
  to = pInst->Nac ;
  do {
    FS_MMC_HW_X_Read(Unit, &c, 1);
    if (c == TOKEN_BLOCK_READ_START) {
      break;
    }
    /* a = c & 0x3F; filter CSD signature bits */
    if (c == 0xFC) {
      break;
    }
    if (++i == to) { /* timeout reached */
      goto End;
    }
  } while (1);

  /* Read the CSD */
  FS_MMC_HW_X_Read(Unit, pCSD->aData, sizeof(CSD));
  FS_MMC_HW_X_Read(Unit, &a, 1); /* read CRC16 high part */
  FS_MMC_HW_X_Read(Unit, &a, 1); /* read CRC16 low part */
  r = 0;
End:
  FS_MMC_HW_X_DisableCS(Unit);  /* CS off */
  _SendEmptyCycles(Unit, 1);  /* Clock card after command */
  return r;     /* No problem ! */
}


/*********************************************************************
*
*       _WaitToGetReady
*
*  Description:
*    Waits until the card returns from busy state.
*    This function only waits _nwrite[Unit] cycles,
*    if the card is still in busy after this cycles, a timeout occurs.
*
*  Return value:
*    ==  0            - Card is ready to accept data/commands.
*    == -1            - timeout occurred.
*/
static int _WaitToGetReady(MMC_INST * pInst) {
  U8  BusyState;
  U8  Unit;
  U32 LoopRem;

  Unit    = pInst->Unit;
  LoopRem = pInst->Nwrite ; /* get timeout from description block */
  do {
    FS_MMC_HW_X_Read(Unit, &BusyState, 1);
    if (BusyState != 0) {
      break;
    }
    if (--LoopRem == 0) {
      return -1;                  /* Error */
    }
  } while (1);
  return 0;
}

/*********************************************************************
*
*       _WaitBlockRdTok
*
*  Description:
*    Wait for valid block read confirmation token.
*
*  Parameters:
*    Unit  - Device Index
*
*  Return value:
*    0xfe     - Success
*    0xff     - Error, time-out
*/
static U8 _WaitBlockRdTok(MMC_INST * pInst) {
  U8  c;
  U32 i;
  U32 to;
  U8  Unit;

  Unit = pInst->Unit;
  to   = pInst->Nac ; /* get timeout from description block */
  for (i = 0; i < to; i++) {
    FS_MMC_HW_X_Read(Unit, &c, 1);
    if (c == TOKEN_BLOCK_READ_START) {
      return c;
    }
  }
  FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: MMC: _WaitBlockRdTok timed out.\n"));
  return 0xff;
}

/*********************************************************************
*
*       _ExecCmdInit
*
*  Description:
*    Send init command (CMD0)
*
*  Parameters:
*    Unit        - Unit number.
*    Cmd         - Command index.
*    Arg         - Command argument.
*
*  Notes:
*    The sequence is very similar, but not identical to the CmdR1 sequence.
*    Main difference is that we do not "Wait until ready" because the card's
*    output may not be enabled for SD cards.
*/
static U8 _ExecCmdInit(U8 Unit) {
  U8 r;
  const U8 CmdBuffer[6] = { 0x40, 0, 0, 0, 0, 0x95 };

  FS_MMC_HW_X_EnableCS(Unit);
  FS_MMC_HW_X_Write(Unit, CmdBuffer, 6);
  r = _CheckR1(Unit);   /* Receive response */
  FS_MMC_HW_X_DisableCS(Unit); /* CS (SPI) off */
  _SendEmptyCycles(Unit, 1);
  return r; /* return received response token */
}

/*********************************************************************
*
*       _CheckCardOCR
*
*  Description:
*    MMC/SD driver driver internal function. Read the card's OCR register and checks,
*    if the provided voltage is supported.
*
*  Parameters:
*    Unit        - Unit number.
*    pCCS        - Pointer to variable to receive CCS bit (if CCS is valid)
*
*  Return value:
*    ==0         - Voltage is supported.
*    !=0         - An error has occurred.
*/
static U8 _CheckCardOCR(MMC_INST * pInst, U8 * pCCS) {
  unsigned uitmp; /* temporary word */
  unsigned Vmax;
  unsigned Vmin;
  unsigned i;
  U8       a[4];
  U8       c;
  U8       r;
  U32      ocr;
  U16      mask;
  U8       Unit;

  Unit    = pInst->Unit;
  FS_MMC_HW_X_EnableCS(Unit);  /* CS on */
  r = _ExecCmdR1(pInst, CMD_READ_OCR, 0);
  if (r != 0) {
    FS_MMC_HW_X_DisableCS(Unit);  /* CS off */
    _SendEmptyCycles(Unit, 1);
    return 0xff;
  }
  /* Get OCR */
  FS_MMC_HW_X_Read(Unit, &a[0], 4);
  ocr =  FS_LoadU32BE(&a[0]);
  FS_MMC_HW_X_DisableCS(Unit);  /* CS off */
  _SendEmptyCycles(Unit, 1);

  /* Return CCS bit value if caller wants it. It's only valid if power
   * status bit is set.
   */
  if (pCCS && ((ocr >> 31) & 1)) {
    *pCCS = (U8)((ocr >> 30) & 1);
  }

  /* test for low voltage mode support */
  r = (U8)(ocr & 0x80);
  if (r != 0) {
    Vmin = VOLT_LOWMIN;
    Vmax = VOLT_LOWMAX;
  } else {
    Vmin = 0;
    Vmax = 0;
  }
  /* filter voltage flags to uitmp */
  uitmp = (U16) (ocr >> 8) & 0xFFFF;
  /* calculate lower voltage limit */
  mask = 0x0001;
  r = 0;
  for (c = 0; c < 16; c++) {
    i = uitmp & mask;
    if (i != 0) {
      break;
    }
    mask <<= 1;
    r++;
  }
  /* Calculate voltage from OCR field.
     Bit position means 100mV, offset is 2000mV */
  if (Vmin < VOLT_LOWMIN) {
    Vmin = 2000 + (r * 100);
  }
  /* calculate high voltage limit */
  for (; c < 16; c++) {
    i = uitmp & mask;
    if (i == 0) {
      break;
    }
    mask <<= 1;
    r++;
  }
  Vmax = (2000 + (r * 100));
  c = FS_MMC_HW_X_SetVoltage(Unit, (U16)Vmin, (U16)Vmax);
  /* Indicate error if card didn't support requested voltage range */
  if (c == 0) {
    return 0xff;
  }
  return 0;
}


#if FS_MMC_SUPPORT_HIGH_SPEED
/*********************************************************************
*
*       _ExecSwitchFunc
*
*  Function description:
*
*
*  Parameters:
*    pInst    -
*    Mode    -
*    Group    -
*    Value    -
*    pResp    -
*
*  Return value:
*    0        - Success
*
*/
static int _ExecSwitchFunc(MMC_INST * pInst, int Mode, int Group, U8 Value, U8 * pResp) {
  U32      Arg;
  unsigned Response;
  U8       aCRC[2];
  U8       Unit;
  int      r;

  Unit     = pInst->Unit;
  Arg = Mode << 31 | 0x00FFFFFF;
  Arg &= ~(0x0FUL << (Group * 4));
  Arg |= Value << (Group * 4);
  FS_MMC_HW_X_EnableCS(Unit);
  Response = _ExecCmdR1(pInst, CMD_SWITCH_FUNC, Arg);
  if (Response == 0xff) {
    r = -1;                   // Error
    goto End;
  }
  Response = _WaitBlockRdTok(pInst);                // Wait for data block to begin
  if (Response == 0xFF) {
    r = -1;                   // Error
    goto End;
  }
  r = 0;
  FS_MMC_HW_X_Read(Unit, pResp, 64);                // read SCR data
  FS_MMC_HW_X_Read(Unit, aCRC, 2);                // read CRC16
End:
  FS_MMC_HW_X_DisableCS(Unit);
  return r;
}
#endif

/*********************************************************************
*
*      _ApplyCSD
*
*  Description:
*    MMC/SD driver internal function.
*    Read the card's CSD (card specific data) registers and check
*    its contents.
*
*  Parameters:
*    Unit      - Device index number
*
*  Return value:
*    ==0           - CSD has been read and all parameters are valid.
*    <0            - An error has occurred.
*
*  Notes
*    (1) SectorSize
*        Newer SD card (4 GByte-card) return a block size larger than 512.
*        Sector Size used however is always 512 bytes.
*/
static int _ApplyCSD(MMC_INST * pInst, CSD * pCSD) {
  int   c;
  int   tmp;
  U32   TimeUnit;
  U32   Factor;        // R2W_FACTOR value
  U32   Freq;          // Card transfer rate in kbit/s
  U32   TimeValue;
  U32   Nac;           // Max configured byte transfer cycles for read access
  U32   NacRead;       // Max byte transfer cycles to allow for read access
  U32   NacWrite;      // Max byte transfer cycles to allow for write access
  U32   CardSize;
  U8       Unit;
  U8       ccs;
  unsigned CSDVersion;
  unsigned AllowHighSpeed = 0;

  c = _CheckCardOCR(pInst, &ccs);
  if (c != 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _ApplyCSD: OCR invalid.\n"));
    return -1;
  }
  Unit = pInst->Unit;
  //
  // CSD version is only checked for SD card. MMC card have almost the same
  // CSD structure as SD V1 cards.
  if (pInst->CardType == CARD_TYPE_SD) {
    CSDVersion = CSD_STRUCTURE(pCSD);
  } else {
    CSDVersion = 0;
  }
#if FS_MMC_SUPPORT_HIGH_SPEED
  {
    U32      CCCSupported;
    //
    //  If the switch function is supported, we
    //  ask the card whether it supports high speed mode.
    //  This is only true for SD-Cards.
    //  MMCs do not support such a command class
    //
    CCCSupported = CSD_CCC_CLASSES(pCSD);
    if ((CCCSupported & (1 << 10)) && (pInst->CardType == CARD_TYPE_SD)) {
      unsigned Response;
      U8       aSCR[8];
      U8       aCRC[2];
      U8       ScrVersion;
      //
      //  Retrieve the SCR (SD card register)
      //
      FS_MMC_HW_X_EnableCS(Unit);
      Response = _ExecCmdR1(pInst, CMD_ACMD_CMD, 0);    // Prepare for advanced command
      _SendEmptyCycles(Unit, 1); /* Clock card before next command */
      Response = _ExecCmdR1(pInst, ACMD_SEND_SCR, 0);   // Send ACMD51 (SD_SEND_SCR)
      Response = _WaitBlockRdTok(pInst);                // Wait for data block to begin
      if (Response == 0xFF) {
        FS_MMC_HW_X_DisableCS(Unit);
        goto Continue;                   // Error
      }
      FS_MMC_HW_X_Read(Unit, aSCR, 8);                // read SCR data
      FS_MMC_HW_X_Read(Unit, aCRC, 2);                // read CRC16
      FS_MMC_HW_X_DisableCS(Unit);
      ScrVersion = _GetBits(aSCR, 56, 59, sizeof(aSCR));
      if (ScrVersion >= SD_SPEC_VER_200) {
        U8 aSwitch[64];
        U32 Data;
        if (_ExecSwitchFunc(pInst, 0, 0, 1, aSwitch) == 0) {
          Data = _GetBits(aSwitch, 400, 415, 64);
          if (Data & 1) {
            if (_ExecSwitchFunc(pInst, 1, 0, 1, aSwitch) == 0) {
              Data = _GetBits(aSwitch, 376, 379, 64);
              if ((Data & 0xF) == 1) {
                AllowHighSpeed = 1;
              }
            }
          }
        }
      }
    }
Continue:;
  }
#endif
  //
  // Interpret card parameters. Some of this code has to differ depending on
  // the card's CSD version (1 or 2). However, we can use the same code to
  // calculate timeout values for any card. We can use the same logic for
  // either CSD version 1 or 2 because version 2 cards supply hard coded timing
  // parameters that are guaranteed to match or exceed the maximum allowed
  // timeouts. Thus, for version 2 cards the timeouts will always be set to
  // the maximum allowed values. This is correct behavior according to the
  // spec. See [1] 5.3.3 CSD Register (CSD Version 2.0), TAAC description.
  //
  //
  // Calculate maximum communication speed according to card specification.
  // Determine transfer rate unit and then combine with time value to get
  // rate in kbit/s.
  //
  if (AllowHighSpeed) {
    _ReadCSD(pInst, pCSD);
  }
  tmp  = CSD_TRAN_SPEED(pCSD);
  c    = tmp & 0x03;
  Freq = _aRateUnit[c];
  c    = (tmp & 0x78) >> 3; /* filter frequency bits */
  Freq *= _aFactor[c];
  //
  //  Set the rate that will be used to talk to card to highest supported rate
  //  that is less than max allowed rate. Freq is set to that actual rate.
  //
  Freq = FS_MMC_HW_X_SetMaxSpeed(Unit, (U16)Freq);
  if (Freq == 0) {
    return 0xff;
  }
  //
  // Determine asynchronous (ie. time based) part of data access time by
  // decoding TAAC value. We determine a numerator and denominator that when
  // combined, via division, yield the access time as fractions of a second.
  // The numerator (stored in TimeValue) is based on the "time value" spec
  // quantity. The denominator (stored in TimeUnit) is based on the "time
  // unit" spec quantity. Because the TimeValue is 10 times what it should
  // be and the TimeUnit is 1/100 what it should be, when we do the division
  // the result is 1000 times what it should be. This means it yields an
  // access time in msec.
  //
  tmp       = CSD_TAAC(pCSD);
  TimeUnit  = _aUnit[tmp & 0x07];
  TimeValue = _aFactor[(tmp >> 3) & 0x0F];
  if (TimeValue == 0) {
    return 0xff;        /* Error, illegal value */
  }
  //
  // According to [1] the description of NSAC says the total typical read
  // access time is "the sum of TAAC and NSAC". [3] clarifies that the values
  // are combined "according to Table 4.47". That table specifies the
  // equation for the maximum read access time as 100 times the typical
  // access time:
  //     Nac(max) = 100(TAAC*FOP + 100*NSAC)
  // Because of the units used in the earlier calculations, we can compute
  // TAAC*FOP by combining the quantities already determined to get a value
  // in cycles.
  //
  Nac = Freq * TimeValue / TimeUnit;
  Nac += 100 * CSD_NSAC(pCSD);            // Add in the "clock dependent" factor of the access time
  Nac *= 100;                             // Worst case value is 100 times typical value.
  Nac >>= 3;                              // We want timeout as a count of byte transfers, not bit transfers
  //
  // According to [1]: "4.6.2.1 Read" (Timeout Conditions), the maximum read
  // timeout needs to be limited to 100 msec. Convert 100 msec to byte
  // transfers using the FOP.
  //
  NacRead = 100 * Freq;
  NacRead >>= 3;
  if (NacRead > Nac) {
    NacRead = Nac;
  }
  // Calculate the write timeout which is calculated from Nac using the
  // R2W_FACTOR. R2W_FACTOR is a power-of-2 value so we can use a simple
  // shift to apply it. Note that even with all the configuration parameters
  // at their maximum values, we can be sure this calculation won't overflow
  // 32 bits.
  //
  Factor = CSD_R2W_FACTOR(pCSD);
  Nac <<= Factor;
  //
  // According to [1]: "4.6.2.2 Write" (Timeout Conditions), the maximum
  // write timeout needs to be limited to 250 msec. Convert 250 msec to
  // byte transfers using the FOP.
  //
  NacWrite = 250 * Freq;
  NacWrite >>= 3;
  if (NacWrite > Nac) {
    NacWrite = Nac;
  }
  /* Decode the version-specific parameters */
  if (CSDVersion == 0) {
    /* Calculate number of sectors available on the medium */
    Factor     = (U16)(1 << CSD_READ_BL_LEN(pCSD)) / BYTES_PER_SECTOR;
    Factor     *= 1 << (CSD_C_SIZE_MULT(pCSD) + 2);
    CardSize   = CSD_C_SIZE(pCSD) + 1;
    CardSize   *= Factor;
  } else if (CSDVersion == 1){  // Newer SD V2 cards.
    /* Calculate number of sectors available on the medium */
    CardSize   = CSD_C_SIZE_V2(pCSD) << 10;
    /* Version 2 cards use CCS to specify SDHC support */
    if (ccs) {
      pInst->CardType = CARD_TYPE_SDHC;
    }
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _ApplyCSD: Unsupported CSD version.\n"));
    return 1;
  }
  //
  //  Store calculated values into medium's instance structure.
  //
  pInst->Nac              = NacRead;
  pInst->Nwrite           = NacWrite;
  pInst->IsWriteProtected = CSD_WRITE_PROTECT(pCSD) | FS_MMC_HW_X_IsWriteProtected(Unit);
  pInst->BytesPerSector   = BYTES_PER_SECTOR;
  pInst->NumSectors       = CardSize;
  return 0;
}

/*********************************************************************
*
*       _InitMMCSD
*
*  Description:
*    Initializes SD V1.xx and all MMC card in order to get all necessary
*    information from card.
*
*  Return value:
*    ==0           - CSD has been read and all parameters are okay.
*    <0            - An error has occurred.
*
*  Notes
*    (1)           MMC Specs says (sect 6.3, power up) that it takes max. 1 ms
*                  for the card to be ready (leave idle/init)
*                  At 400 kHz, this equals 9 loops.
*                  At lower speeds, less repetitions would be o.k.,
*                  but a few more ms. should not hurt (only in case the
*                  card is not present)
*/
static int _InitMMCSD(MMC_INST * pInst) {
  CSD csd;
  int i;
  U8  Response;
  U8  r;
  U8  Unit;

  Unit = pInst->Unit;
  i = 2000;       /* Note 1 */
  do {   /* Try initializing as SD card first. Note (1) */
    FS_MMC_HW_X_EnableCS(Unit);
    Response = _ExecCmdR1(pInst, CMD_ACMD_CMD, 0); /* Prepare for advanced command */
    _SendEmptyCycles(Unit, 1); /* Clock card before next command */
    Response = _ExecCmdR1(pInst, ACMD_SEND_OP_COND, 0); /* Send ACMD41 (SD_SEND_OP_COND) */
    FS_MMC_HW_X_DisableCS(Unit);
    _SendEmptyCycles(Unit, 1); /* Clock card before next command */

    if (Response == 0) { /* SD card is now ready */
      pInst->CardType = CARD_TYPE_SD;
      break;
    }
    if (Response & 4) {
      break; /* command not accepted */
    }
    if (--i == 0) {
      Response = 0xff; // Report time-out
      break;
    }
  } while (1);
  if (Response != 0) {                /* We need to try to init it as MMC */
    i = 2000;                    /* Note 1 */
    do {
      FS_MMC_HW_X_EnableCS(Unit);
      Response = _ExecCmdR1(pInst, CMD_SEND_OP_COND, 0); /* Send CMD1 (SEND_OP_COND) until ready or timeout. */
      FS_MMC_HW_X_DisableCS(Unit);
      _SendEmptyCycles(Unit, 1); /* Clock card before next command */
      if (Response == 0) {
        pInst->CardType = CARD_TYPE_MMC;
        break;          /* Card is ready ! */

      }
      if (Response != 1) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _InitMMCSD: Illegal response.\n"));
        return -1;
      }
      if  (--i == 0) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _InitMMCSD: Time out during init.\n"));
        return -1;
      }
    } while (1);
  }
  r = _ReadCSD(pInst, &csd);
  if (r != 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _InitMMCSD: CSD (card spec. data) invalid.\n"));
    return -1;       /* Error */
  }
  r = _ApplyCSD(pInst, &csd);
  if (r != 0) {
    return -1;       /* Error */
  }
  pInst->IsInited = 1;
  return 0;
}

/*********************************************************************
*
*       _InitSDV2
*
*  Description:
*
*    and set it to SPI mode.
*
*  Parameters:
*    Unit      - Device index number
*
*  Return value:
*    ==0           - CSD has been read and all parameters are OK.
*    <0            - An error has occurred.
*
*  Notes
*    (1)           MMC Specs says (sect 6.3, power up) that it takes max. 1 ms
*                  for the card to be ready (leave idle/init)
*                  At 400 kHz, this equals 9 loops.
*                  At lower speeds, less repetitions would be o.k.,
*                  but a few more ms. should not hurt (only in case the
*                  card is not present)
*/
static int _InitSDV2(MMC_INST * pInst) {
  CSD csd;
  int i;
  U8  c;
  U8  Unit;

  Unit = pInst->Unit;
  i = 2000;       /* Note 1 */
  do {   /* Try initializing as SD card first. Note (1) */
    FS_MMC_HW_X_EnableCS(Unit);
    //
    // Prepare for advanced command
    //
    c = _ExecCmdR1(pInst, CMD_ACMD_CMD, 0);
    _SendEmptyCycles(Unit, 1); /* Clock card before next command */
    //
    // Send ACMD41 (SD_SEND_OP_COND), set argument, that host supports HC.
    //
    c = _ExecCmdR1(pInst, ACMD_SEND_OP_COND, (1UL << 30));
    FS_MMC_HW_X_DisableCS(Unit);
    _SendEmptyCycles(Unit, 1); /* Clock card before next command */

    if (c == 0) { /* SD card is now ready */
      break;
    }
  } while (--i);
  c = _ReadCSD(pInst, &csd);
  if (c != 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _InitSDV2: CSD (card spec. data) invalid.\n"));
    return -1;       /* Error */
  }
  //
  // Card is identified as SD card
  // We check later whether it is a standard or HC card.
  pInst->CardType = CARD_TYPE_SD;
  c = _ApplyCSD(pInst, &csd);
  if (c != 0) {
    return -1;       /* Error */
  }
  pInst->IsInited = 1;
  return 0;
}



/*********************************************************************
*
*       _MMC_Init
*
*  Description:
*    MMC driver internal function. Reset the card, reset SPI clock speed
*    and set it to SPI mode.
*
*  Parameters:
*    Unit      - Device index number
*
*  Return value:
*    ==0           - CSD has been read and all parameters are okay.
*    <0            - An error has occurred.
*
*  Notes
*    (1)   Argument structure for CMD8 (SEND_IF_COND)
*           [31..12] - Shall be zero
*           [11.. 8] - Voltage Supply (VHS):
*                      Defined Voltage Supplied Values (VHS defined by SDCard Spec V2.00)
*                        0x00 - Not Defined
*                        0x01 - 2.7-3.6V
*                        0x02 - Reserved for Low Voltage Range
*                        0x04 - Reserved
*                        0x08 - Reserved
*                        Others Not Defined
*           [ 7.. 0] - Check Pattern. It is recommended to use the value 0xAA as pattern.
*
*
*/
static int _MMC_Init(MMC_INST * pInst) {
  int i;
  U8  Response;
  U16 CurrFreq;
  U8  Unit;
  U8  aResponse7[4];
  int r;

  Unit     = pInst->Unit;
  Response = 0;
  r        = -1;
  CurrFreq = FS_MMC_HW_X_SetMaxSpeed(Unit, STARTUPFREQ); /* set initial speed for SPI */
  if (CurrFreq > STARTUPFREQ) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _MMC_Init: Frequency is greater than allowed for initialization.\n"));
    return -1;
  }
  pInst->Nac = NAC_CSD_MAX;
  //
  // Send empty cycles and CMD0 (GO_IDLE_STATE) until card responds with 0x01 = OK.
  // Allow multiple tries.
  //
  i = 0;
  do {
    FS_MMC_HW_X_DisableCS(Unit);
    _SendEmptyCycles(Unit, BOOTUPCYCLES); /* Clock card before reset sequence */
    Response = _ExecCmdInit(Unit); /* CMD0 (GO_IDLE_STATE) */
    if (Response & 1) { /* if card is not in idle state, return with error */
      break;
    }
    if (++i == MAX_RETRIES) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: Init: Card is not in idle state.\n"));
      return -1;  /* now you may need to power cycle the card */
    }
  } while (1);
  //
  // Send CMD8 to card, SD HC or SD cards V2.00 card will accept the command
  // all other cards will reply that this is an illegal command.
  // Initially we will only read one byte from card.
  // If it is not an illegal command, we will do further reading.
  //
  FS_MMC_HW_X_EnableCS(Unit);
  Response = _ExecCmdR1(pInst, CMD_SEND_IF_COND, (0x01 << 8) | 0xAA); // Note 1
  if ((Response & 4)) {  // Illegal command, not a SD V2 card.
    FS_MMC_HW_X_DisableCS(Unit);
    r = _InitMMCSD(pInst);
  } else {
    FS_MMC_HW_X_Read(pInst->Unit, &aResponse7[0], sizeof(aResponse7));
    FS_MMC_HW_X_DisableCS(Unit);
    //
    // Did the card return the correct pattern?
    //
    if ((aResponse7[3] == 0xAA) && ((aResponse7[2] & 0xF) == 0x01)) {
      r = _InitSDV2(pInst);
    }
  }
  return r;
}


/*********************************************************************
*
*       Semi-public Code called through function pointer
*
**********************************************************************
*/


/*********************************************************************
*
*       _MMC_GetStatus
*
*  Description:
*    FS driver function. Get status of the media,
*    Initialize the card if necessary.
*
*  Parameters:
*    Unit        - Unit number.
*
*  Return value:
*    FS_MEDIA_STATE_UNKNOWN - if the state of the media is unknown.
*    FS_MEDIA_NOT_PRESENT   - if no card is present.
*    FS_MEDIA_IS_PRESENT    - if a card is present.
*/
static int _MMC_GetStatus(U8 Unit) {
  return FS_MMC_HW_X_IsPresent(Unit);
}

/*********************************************************************
*
*       _MMC_IoCtl
*
*  Description:
*    FS driver function. Execute device command.
*
*  Parameters:
*    Unit      - Device Index.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value:
*    Command specific. In general a negative value means an error.
*/
static int _MMC_IoCtl(U8 Unit, I32 Cmd, I32 Aux, void *pBuffer) {
  FS_DEV_INFO * pDevInfo;
  MMC_INST    * pInst;

  FS_USE_PARA(Aux);
  pInst = _apInst[Unit];
  switch (Cmd) {
  case FS_CMD_UNMOUNT:
  case FS_CMD_UNMOUNT_FORCED:
    FS_MEMSET(pInst, 0, sizeof(MMC_INST));
    break;
  case FS_CMD_GET_DEVINFO: /* Get general device information */
    if (pInst->IsInited == 0) {
      if (_MMC_Init(pInst)) {
        return -1;
      }
    }
    pDevInfo = (FS_DEV_INFO *)pBuffer;
    pDevInfo->BytesPerSector = pInst->BytesPerSector;
    pDevInfo->NumSectors     = pInst->NumSectors;
    break;
#if FS_SUPPORT_DEINIT
  case FS_CMD_DEINIT:
    FS_FREE(pInst);
    _NumUnits--;
    return 0;
#endif
  }
  return 0;
}

/*********************************************************************
*
*       _ReadSectors
*
*/
static int _ReadSectors(MMC_INST * pInst, U32 SectorNo, U8 * pBuffer, U32 NumSectors) {
  U8         a;
  U8         aCRC[2];
  U16        Crc;
  U16        CalcedCrc;
  U8         Cmd;
  int        r;
  U8         Unit;

  Unit  = pInst->Unit;
  r     = -1;                       // Default is to return error
  Cmd   = (NumSectors == 1) ? CMD_READ_SINGLE_BLOCK : CMD_READ_MULTIPLE_BLOCKS;
  if (pInst->CardType != CARD_TYPE_SDHC) {
    SectorNo *= BYTES_PER_SECTOR;
  }
  //
  // Send command
  //
  FS_MMC_HW_X_DisableCS(Unit);
  _SendEmptyCycles(Unit, 1);
  FS_MMC_HW_X_EnableCS(Unit);
  a = _ExecCmdR1(pInst, Cmd, SectorNo);  /* Do not handle clock&CS automatically */
  //
  // Read sector by sector
  //
  if (a == 0) {
    do {
      a = _WaitBlockRdTok(pInst); /* wait for data block to begin */
      if (a == 0xFF) {
        break;                   // Error
      }
      FS_MMC_HW_X_Read(Unit, pBuffer, BYTES_PER_SECTOR); // read one sector
      FS_MMC_HW_X_Read(Unit, aCRC, 2);                   // read CRC16
      Crc = (aCRC[0] << 8) | (aCRC[1]);
      CalcedCrc = _CalcDataCRC16(pBuffer, BYTES_PER_SECTOR);
      if ((CalcedCrc != 0xffff) && (CalcedCrc != Crc)) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Received wrong CRC, received 0x%8x, expected 0x%8x @Sector 0x%8x", Crc, CalcedCrc, SectorNo));
        if (Cmd == CMD_READ_MULTIPLE_BLOCKS) {
          _ExecCmdR1(pInst, CMD_STOP_TRANSMISSION, 0);  /* Do not handle clock&CS automatically */
        }
        break;
      }
      pBuffer += BYTES_PER_SECTOR;
      if (--NumSectors == 0) {                           // Are we done ?
        if (Cmd == CMD_READ_MULTIPLE_BLOCKS) {
          _ExecCmdR1(pInst, CMD_STOP_TRANSMISSION, 0);  /* Do not handle clock&CS automatically */
        }
        r = 0;
        break;
      }
    } while (1);
  }
  //
  // We are done. Disable CS and send some dummy clocks.
  //
  FS_MMC_HW_X_DisableCS(Unit);
  _SendEmptyCycles(Unit, 1);
  return r;
}

/*********************************************************************
*
*       _MMC_Read
*
*  Description:
*    Reads one or more sectors from the media.
*
*    Unit          - Device index number
*    Sector        - Sector to be read from the device.
*    NumSectors    - Number of sectors to be transferred
*    pBuffer       - Pointer to buffer to be stored.
*    RepeatSame    - Shall be the same data written
*
*  Return value:
*    ==0         - All sector(s) have been read
*    <0          - An error has occurred.
*/
static int _MMC_Read(U8 Unit, U32 SectorNo, void * p, U32 NumSectors) {
  int           r;
  U8       * pBuffer;
  MMC_INST * pInst;
  int        Retry = FS_MMC_NUM_RETRIES;

  pInst   = _apInst[Unit];
  pBuffer = (U8 *)p;

OnRetry:
  r = _ReadSectors(pInst, SectorNo, pBuffer, NumSectors);
  if (r && Retry) {
    --Retry;
    goto OnRetry;
  }
  return r;

}



/*********************************************************************
*
*       _MMC_Write
*
*  Description:
*    Write one ore more sectors to the media.
*
*  Parameters:
*    Unit          - Device index number
*    Sector        - Sector to be written to the device.
*    NumSectors    - Number of sectors to be transferred
*    pBuffer       - Pointer to data to be stored.
*    RepeatSame    - Shall be the same data written
*
*  Return value:
*    ==0           - Sector has been written to the device.
*    <0            - An error has occurred.
*/
static int _WriteSectors(MMC_INST * pInst, U32 SectorNo, const U8 * pBuffer, U32 NumSectors, U8 RepeatSame) {
  U8         c;
  int        r;
  U8         aCRC[2];
  U16        crc;
  U8         Cmd;
  U8         Unit;

  Unit    = pInst->Unit;
  r       = -1;
  Cmd     = (NumSectors == 1) ? CMD_WRITE_BLOCK : CMD_WRITE_MULTIPLE_BLOCK;
  if (pInst->CardType != CARD_TYPE_SDHC) {
    SectorNo *= BYTES_PER_SECTOR;
  }
  //
  // Send command
  //
  FS_MMC_HW_X_DisableCS(Unit);
  _SendEmptyCycles(Unit, 1);
  FS_MMC_HW_X_EnableCS(Unit);  /* CS on */
  c = _ExecCmdR1(pInst, Cmd, SectorNo);
  if (c != 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _Write: Write failed.\n"));
    goto End;
  }
  _SendEmptyCycles(Unit, 1); /* Nwr */
  //
  // Transfer data one sector at a time
  //
  do {
    crc = _CalcDataCRC16(pBuffer, BYTES_PER_SECTOR);
    aCRC[0] = (U8)(crc >> 8);
    aCRC[1] = (U8)crc;
    c = (Cmd == CMD_WRITE_BLOCK) ? TOKEN_BLOCK_WRITE_START : TOKEN_MULTI_BLOCK_WRITE_START;   /* Send data token */
    FS_MMC_HW_X_Write(Unit, &c, 1);
    FS_MMC_HW_X_Write(Unit, pBuffer, BYTES_PER_SECTOR);
    FS_MMC_HW_X_Write(Unit, aCRC, 2);
    //
    // Get data response token (MMC spec 7.6.2, Figure 52)
    // Should be XXX00101
    //
    FS_MMC_HW_X_Read(Unit, &c, 1);
    if ((c & 0x1f) != 5) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _Write: Write failed, data not accepted.\n"));
      goto End;
    }
    //
    // Wait for card to get ready
    //
    if (_WaitToGetReady(pInst)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _Write: Write failed (timeout).\n"));
      goto End;
    }
    if (RepeatSame == 0) {
      pBuffer += BYTES_PER_SECTOR;
    }
  } while (--NumSectors);
  //
  // Send the stop token to card, this indicates, that we are finished sending data to card
  //
  if (Cmd == CMD_WRITE_MULTIPLE_BLOCK) {
    c = TOKEN_MULTI_BLOCK_WRITE_STOP;
    FS_MMC_HW_X_Write(Unit, &c, 1);
    if (_WaitToGetReady(pInst) == 0) {
      r = 0;                                    // Success !
    }
  } else {
    r = 0;
  }
End:
  FS_MMC_HW_X_DisableCS(Unit);
  _SendEmptyCycles(Unit, 1);
  if (r) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC SPI: _Write: Write failed.\n"));
  }
  return r;
}


/*********************************************************************
*
*       _MMC_Write
*
*  Description:
*    Write one ore more sectors to the media.
*
*  Parameters:
*    Unit          - Device index number
*    Sector        - Sector to be written to the device.
*    NumSectors    - Number of sectors to be transferred
*    pBuffer       - Pointer to data to be stored.
*    RepeatSame    - Shall be the same data written
*
*  Return value:
*    ==0           - Sector has been written to the device.
*    <0            - An error has occurred.
*/
static int _MMC_Write(U8 Unit, U32 SectorNo, const void * p, U32 NumSectors, U8 RepeatSame) {
  const U8 * pBuffer;
  MMC_INST * pInst;
  int           r;
  int           Retry = FS_MMC_NUM_RETRIES;

  pInst = _apInst[Unit];
  if (pInst->IsWriteProtected) {
    return -1;
  }
  pBuffer = (const U8 *)p;
OnRetry:
  r = _WriteSectors(pInst, SectorNo, pBuffer, NumSectors, RepeatSame);
  if (r && Retry) {
    --Retry;
    goto OnRetry;
  }
  return r;


}
/*********************************************************************
*
*       _MMC_InitMedium
*
*  Description:
*    Initialize the card.
*
*  Parameters:
*    Unit        - Unit number.
*
*  Return value:
*    == 0                       - Device OK and ready for operation.
*    <  0                       - An error has occurred.
*/
static int _MMC_InitMedium(U8 Unit) {
  int        r;
  MMC_INST * pInst;

  pInst = _apInst[Unit];
  r     = -1;
  if (pInst->IsInited == 0) {
    if (_MMC_GetStatus(Unit) != FS_MEDIA_NOT_PRESENT) {
      r = _MMC_Init(pInst);
      if (r < 0) { /* init failed, no valid card in slot */
        FS_DEBUG_WARN((FS_MTYPE_DRIVER, "MMC: Init failure, no valid card found"));
      }
    }
  }
  return r;
}


/*********************************************************************
*
*       _MMC_AddDevice
*
*  Description:
*    Initializes the low-level driver object.
*
*  Return value:
*    >= 0                       - Command successfully executed, Unit no.
*    <  0                       - Error, could not add device
*
*/
static int _MMC_AddDevice(void) {
  U8         Unit;
  MMC_INST * pInst;

  if (_NumUnits >= NUM_UNITS) {
    return -1;
  }
  Unit = _NumUnits++;
  pInst = (MMC_INST *)FS_AllocZeroed(sizeof(MMC_INST));   // Alloc memory. This is guaranteed to work by the memory module.
  _apInst[Unit] = pInst;
  pInst->Unit   = Unit;
  return Unit;
}

/*********************************************************************
*
*       _MMC_GetNumUnits
*/
static int _MMC_GetNumUnits(void) {
  return _NumUnits;
}

/*********************************************************************
*
*       _MMC_GetDriverName
*/
static const char * _MMC_GetDriverName(U8 Unit) {
  FS_USE_PARA(Unit);
  return "mmc";
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*      FS_MMC_GetCardId
*
*  Description:
*    This function retrieves the card Id of SD/MMC card.
*
*  Parameters:
*    Unit          - Device index number.
*    pCardID       - Pointer to a MMC_CARD_ID structure.
*
*  Return value:
*    ==0           - CardId has been read.
*    !=0           - An error has occurred.
*/
int FS_MMC_GetCardId(U8 Unit, MMC_CARD_ID * pCardId) {
  U8   Response;
  U32  i;
  U32  TimeOut;
  U8   aData[2];
  int  r;
  MMC_INST * pInst;

  r = 0;
  pInst = _apInst[Unit];
  _SendEmptyCycles(Unit, 1);
  FS_MMC_HW_X_EnableCS(Unit);  /* CS on */

  /* Execute CMD10 (SEND_CID) */
  Response = _ExecCmdR1(pInst, CMD_SEND_CID, 0);  /* Do not handle clock & CS automatically */
  if (Response != 0) {
    r = 1;                // Error
    goto End;
  }
  /* wait for CardId transfer to begin */
  i = 0;
  TimeOut = pInst->Nac ;
  do {
    FS_MMC_HW_X_Read(Unit, aData, 1);
    if (aData[0] == TOKEN_BLOCK_READ_START) {
      break;
    }
    if (aData[0] == TOKEN_MULTI_BLOCK_WRITE_START) {
      break;
    }
    if (++i == TimeOut) { /* timeout reached */
      r = 1;                // Error
      goto End;
    }
  } while (1);

  /* Read the CardID */
  FS_MMC_HW_X_Read(Unit, (U8*)pCardId, sizeof(MMC_CARD_ID));
  FS_MMC_HW_X_Read(Unit, aData, 2);                       // Read CRC16
End:
  FS_MMC_HW_X_DisableCS(Unit);
  _SendEmptyCycles(Unit, 1);                             // Clock card after command
  return r;
}

/*********************************************************************
*
*       FS_MMC_ActivateCRC
*/
void FS_MMC_ActivateCRC(void) {
  _pfCalcCRC = _CalcDataCRC16ViaTable;
}

/*********************************************************************
*
*       FS_MMC_DeactivateCRC
*/
void FS_MMC_DeactivateCRC(void) {
  _pfCalcCRC = _CalcDataCRC16Dummy;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
const FS_DEVICE_TYPE FS_MMC_SPI_Driver = {
  _MMC_GetDriverName,
  _MMC_AddDevice,
  _MMC_Read,
  _MMC_Write,
  _MMC_IoCtl,
  _MMC_InitMedium,
  _MMC_GetStatus,
  _MMC_GetNumUnits
};

/*************************** End of file ****************************/
