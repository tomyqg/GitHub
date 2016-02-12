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
File        : MMC_SD_CardMode_Drv.c
Purpose     : File system generic MMC/SD card mode driver

Literature  : []
              \\fileserver\techinfo\Company\MMCA.org
              "\\fileserver\techinfo\Company\SDCard_org\Copyrighted\Part 01 Physical Layer\Part 1 Physical Layer Specification Ver2.00 Final 060509.pdf"

---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include "FS_Int.h"
#include "MMC_SD_CardMode_X_HW.h"
#include "FS_CLib.h"

#ifdef FS_MMC_MAXUNIT
  #define NUM_UNITS FS_MMC_MAXUNIT
#else
  #define NUM_UNITS   1
#endif

#ifndef   NUM_RETRIES
  #define NUM_RETRIES    5
#endif

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

/*********************************************************************
*
*       ASSERT_UNIT_NO_IS_IN_RANGE
*/
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL
  #define ASSERT_UNIT_NO_IS_IN_RANGE(Unit)  if (Unit >= _NumUnits)               { FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC CM: Illegal unit number.\n"));   }
#else
  #define ASSERT_UNIT_NO_IS_IN_RANGE(Unit)
#endif


/*********************************************************************
*
*       CSD register access macros
*/
#define CSD_STRUCTURE(pCSD)            _GetFromCSD(pCSD, 126, 127)
#define CSD_WRITE_PROTECT(pCSD)        _GetFromCSD(pCSD, 12, 13)
#define CSD_FILE_FORMAT_GRP(pCSD)      _GetFromCSD(pCSD, 15, 15)
#define CSD_WRITE_BL_LEN(pCSD)         _GetFromCSD(pCSD, 22, 25)
#define CSD_R2W_FACTOR(pCSD)           _GetFromCSD(pCSD, 26, 28)
#define CSD_C_SIZE_MULT(pCSD)          _GetFromCSD(pCSD, 47, 49)
#define CSD_C_SIZE(pCSD)               _GetFromCSD(pCSD, 62, 73)
#define CSD_READ_BL_LEN(pCSD)          _GetFromCSD(pCSD, 80, 83)
#define CSD_TRAN_SPEED(pCSD)           (pCSD->aData[3])   // Same as, but more efficient than: _GetFromCSD(pCSD,  96, 103)
#define CSD_NSAC(pCSD)                 (pCSD->aData[2])   // Same as, but more efficient than: _GetFromCSD(pCSD, 104, 111)
#define CSD_TAAC(pCSD)                 (pCSD->aData[1])   // Same as, but more efficient than: _GetFromCSD(pCSD, 112, 119)
#define CSD_C_SIZE_V2(pCSD)            _GetFromCSD(pCSD,  48, 69)
#define CSD_CCC_CLASSES(pCSD)          _GetFromCSD(pCSD,  84, 95)

#define STARTUPFREQ              400  /* Max. startup frequency */

#define BYTES_PER_SECTOR              512
/*********************************************************************
*
*       Command definitions
*/
#define CMD_GO_IDLE_STATE           0
#define CMD_SEND_OP_COND            1        // MMC Cards only
#define CMD_ALL_SEND_CID            2
#define CMD_SET_REL_ADDR            3
#define CMD_SELECT_CARD             7
#define CMD_SWITCH_FUNC             6
#define CMD_SEND_CSD                9
#define CMD_SEND_IF_COND            8
#define CMD_SEND_CID               10
#define CMD_STOP_TRANSMISSION      12
#define CMD_SEND_STATUS            13
#define CMD_GO_INACTIVE_STATE      15
#define CMD_SET_BLOCKLEN           16
#define CMD_READ_SINGLE_BLOCK      17
#define CMD_READ_MULTIPLE_BLOCKS   18

#define CMD_WRITE_BLOCK            24
#define CMD_WRITE_MULTIPLE_BLOCKS  25
#define CMD_ACMD_CMD               55

#define ACMD_SET_BUS_WIDTH          6
#define ACMD_SD_STATUS             13
#define ACMD_SEND_OP_COND          41
#define ACMD_SET_CLR_CARD_DETECT   42
#define ACMD_SEND_SCR              51

#define BUS_WIDTH_1BIT              (0)
#define BUS_WIDTH_4BIT              (2)

#define SD_HC_SUPPORT                      (1UL << 30)
#define SD_CCS                             (1 <<  6)

#define FS_MMC_CARDTYPE_NONE              (0)
#define FS_MMC_CARDTYPE_MMC               (1 << 0)
#define FS_MMC_CARDTYPE_SD                (1 << 1)
#define FS_MMC_CARDTYPE_SDHC             ((1 << 1) | (1 << 2))

#define SD_SPEC_VER_100           0
#define SD_SPEC_VER_110           1
#define SD_SPEC_VER_200           2

/*********************************************************************
*
*       Defines non configurable
*
**********************************************************************
*/
#define VRANGE_OCR      0x001C0000UL  // MMC cards allowed at 3.0 to 3.3v
#define OCR_RDY_MASK    0x80          // Mask to check OCR ready bit
#define MAX_OCR_RET     0x2ff         // Maximum OCR request retries

#define MAX_RESPONSE_TIMEOUT 0xff
#define MAX_READ_TIMEOUT     0xffffffffUL

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {           /* CSD register structure */
  U8 Dummy;
  U8 aData[16];  /* CSD size is 128 bit */
} CM_CSD;

typedef struct {
  U8 aStatus[6];  /* CardStatus answer is 48 bit wide */
} MMC_CARD_STATUS;

typedef struct {
  U8 aOCR[6];  /* OCR answer is 48 bit wide */
} MMC_OCR_RESPONSE;

typedef struct {
  U8  IsInited;
  U8  Unit;
  U8  HasError;
  U8  CardType;
  U8  Use4BitMode;
  U8  IsWriteProtected;
  U16 BytesPerSector;
  U16 Rca;
  U16 MaxReadBurst;
  U16 MaxWriteBurst;
  U16 MaxReadBurstSave;
  U16 MaxWriteBurstSave;
  U32 NumSectors;
  MMC_CARD_ID CardId;
  U8   Allow4bitMode;
  U8   IsSelected;
  U8   AllowHighSpeedMode;
  U8   WrRepeatSameAllowed;
} MMC_CM_INST;

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
static const U32 _aUnit[8] = {
  10000000UL,
  1000000UL,
  100000UL,
  10000UL,
  1000UL,
  100UL,
  10UL,
  1UL,
};

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

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static MMC_CM_INST *  _apInst[NUM_UNITS];
static int         _NumUnits;

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
static unsigned _GetFromCSD(const CM_CSD * pCSD, unsigned FirstBit, unsigned LastBit) {
  U32 Data;
  Data = _GetBits(pCSD->aData, FirstBit, LastBit, sizeof(pCSD->aData));
  return Data;
}

/*********************************************************************
*
*       _SendCmd
*/
static void _SendCmd(MMC_CM_INST * pInst, unsigned Cmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg) {
  FS_MMC_HW_X_SendCmd(pInst->Unit, Cmd, CmdFlags, ResponseType, Arg);
}

/*********************************************************************
*
*       _GetResponse
*
*  Function description
*
*  Return value
*    TBD
*/
static int _GetResponse(MMC_CM_INST * pInst, void *pBuffer, U32 Size) {
   return FS_MMC_HW_X_GetResponse(pInst->Unit, pBuffer, Size);
}


/*********************************************************************
*
*       _SendAdvCommand
*
*  Function description
*
*  Return value
*    TBD
*
*  Notes
*    If "success" is returned, caller MUST call _GetResponse
*/
static int _SendAdvCommand(MMC_CM_INST * pInst, U32 ACmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg, int MaxRetry) {
  int             r;
  MMC_CARD_STATUS CardStatus;

  do {
    FS_MEMSET(&CardStatus, 0, sizeof(MMC_CARD_STATUS));
    _SendCmd(pInst, CMD_ACMD_CMD, 0, FS_MMC_RESPONSE_FORMAT_R1, (U32)(pInst->Rca) << 16);
    r = _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
    if (r == 0) {
      _SendCmd(pInst, ACmd, CmdFlags, ResponseType, Arg);
      break;
    }
  } while(--MaxRetry);
  return r;
}


/**********************************************************
*
*       _HWInit
*/
static void _HWInit(MMC_CM_INST * pInst) {
  FS_MMC_HW_X_InitHW(pInst->Unit);
}

/**********************************************************
*
*       _CheckCardType
*
*  Function description
*    TBD
*
*  Return value
*    FS_MMC_CARDTYPE_SD      SD Card
*    FS_MMC_CARDTYPE_MMC     MMC Card
*    FS_MMC_CARDTYPE_NONE    No card found
*/
static int _CheckCardType(MMC_CM_INST * pInst) {
  MMC_OCR_RESPONSE ocr;
  int    ocrAttempts;
  // Poll card until it has completed the power-up sequence
  FS_MEMSET(&ocr.aOCR[0], 0, sizeof(ocr));
  ocrAttempts = MAX_OCR_RET;
  //
  // We initialize first a SD card. A SD card needs to be initialized differently
  // from the MMC card -> Send an ACMD41 to card which will give us the OCR information
  // and of course if card is ready. MMC cards with an invalid command and ignores this command
  // sequence.
  //
  while (ocrAttempts > 0) {
    //
    // If controller reports timeout, then the card is rather a MMC card than a SD card.
    //
    if (_SendAdvCommand(pInst, ACMD_SEND_OP_COND, 0, FS_MMC_RESPONSE_FORMAT_R3, VRANGE_OCR | (1UL << 30), 4) == FS_MMC_CARD_NO_ERROR) {
      if (_GetResponse(pInst, &ocr, sizeof (ocr)) == FS_MMC_CARD_NO_ERROR) {
        if (ocr.aOCR[1] == OCR_RDY_MASK) {
          return FS_MMC_CARDTYPE_SD;
        }
      }
    } else {
       break;
    }
    ocrAttempts--;
  }
  FS_MEMSET(&ocr, 0, sizeof(ocr));
  ocrAttempts = MAX_OCR_RET;
  while (ocrAttempts > 0) {
    //
    // MMC card will answer to this command if the MMC card is ready
    //
    FS_MEMSET(&ocr, 0, sizeof(ocr));
    _SendCmd(pInst, CMD_SEND_OP_COND, 0, FS_MMC_RESPONSE_FORMAT_R3, VRANGE_OCR);
    _GetResponse(pInst, &ocr, sizeof(ocr));
    ocrAttempts--;
    if ((ocr.aOCR[1] & OCR_RDY_MASK) != 0) {
      return FS_MMC_CARDTYPE_MMC;
    }
  }
  return FS_MMC_CARDTYPE_NONE;   // No valid card found
}


/**********************************************************
*
*       _CheckCardTypeV2
*/
static int _CheckCardTypeV2(MMC_CM_INST * pInst, U8 * pCCS) {
  MMC_OCR_RESPONSE ocr;
  int    ocrAttempts;
  /* Poll card until it has completed the power-up sequence */
  FS_MEMSET(&ocr.aOCR[0], 0, sizeof(ocr));
  ocrAttempts = MAX_OCR_RET;
  //
  // Initialization is almost the same as in SD card V1
  //
  while (ocrAttempts > 0) {
    _SendAdvCommand(pInst, ACMD_SEND_OP_COND, 0, FS_MMC_RESPONSE_FORMAT_R3, VRANGE_OCR | SD_HC_SUPPORT, 1);
    if (_GetResponse(pInst, &ocr, sizeof (ocr)) == FS_MMC_CARD_RESPONSE_TIMEOUT) {
      return FS_MMC_CARDTYPE_NONE;   // No valid card found
    }
    ocrAttempts--;
    if ((ocr.aOCR[1] & OCR_RDY_MASK)) {
      break;
    }
  }
  //
  // When card is in ready state, we additionlly check  which card type it is.
  // Bit 30 in OCR field indicates that card is a SDHC card.
  //
  if (ocr.aOCR[1] & SD_CCS) {
    *pCCS = 1;
  }
  return FS_MMC_CARDTYPE_SD;
}


/**********************************************************
*
*       _SelectCard
*/
static int _SelectCard(MMC_CM_INST * pInst, MMC_CARD_STATUS * pCardStatus) {
  unsigned CardState;
  unsigned NumRetries;
  int      r;


  FS_MEMSET(pCardStatus, 0, sizeof(MMC_CARD_STATUS));
  NumRetries = 0x20;
  r          = 1;
  do {
    U32 rca;
    rca = pInst->Rca;
    _SendCmd(pInst, CMD_SELECT_CARD,  FS_MMC_CMD_FLAG_SETBUSY, FS_MMC_RESPONSE_FORMAT_R1, rca << 16);
    _GetResponse(pInst, pCardStatus, sizeof(MMC_CARD_STATUS));
    _SendCmd(pInst, CMD_SEND_STATUS, 0, FS_MMC_RESPONSE_FORMAT_R1, rca << 16);
    _GetResponse(pInst, pCardStatus, sizeof (MMC_CARD_STATUS));
    if ((pCardStatus->aStatus[1]) || (pCardStatus->aStatus[2] & 0x3f)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC CM: Card reports an error after selecting card.\n"));
      break;
    }
    CardState = pCardStatus->aStatus[3];
    if (((CardState >> 1) == 0x04) && ((CardState & 1))) {
      r = 0;
      if (pInst->IsSelected) {
        pInst->IsSelected = 0;
      }
      pInst->IsSelected = 1;
      break;
    }
  } while (--NumRetries);
  return r;
}

/**********************************************************
*
*       _DeSelectCard
*/
static int _DeSelectCard(MMC_CM_INST * pInst) {
  unsigned CmdFlags;

  CmdFlags = 0;
  if ((pInst->CardType & FS_MMC_CARDTYPE_SD) == 0) { /* It is a MMC card */
    CmdFlags |= FS_MMC_CMD_FLAG_SETBUSY;
  }
  //
  // No response is expected after deselecting the card.
  //
  _SendCmd(pInst, CMD_SELECT_CARD, CmdFlags, FS_MMC_RESPONSE_FORMAT_NONE, 0);
  pInst->IsSelected = 0;
  return 0;
}

/**********************************************************
*
*       _SetBlockLen
*/
static int _SetBlockLen(MMC_CM_INST * pInst, MMC_CARD_STATUS * pCardStatus) {
  //
  // Setup the SD controller to transfer the standard sector size of the card.
  //
  FS_MMC_HW_X_SetHWBlockLen(pInst->Unit, pInst->BytesPerSector);
  // Send block length command to card
  _SendCmd(pInst, CMD_SET_BLOCKLEN, 0, FS_MMC_RESPONSE_FORMAT_R1, BYTES_PER_SECTOR);
  return _GetResponse(pInst, pCardStatus, sizeof (MMC_CARD_STATUS));
}

/**********************************************************
*
*       _Wait4Ready
*
*/
static int _Wait4Ready(MMC_CM_INST * pInst) {
  MMC_CARD_STATUS CardStatus;
  U32 TimeOut;
  TimeOut = 0xFFFFFFUL;
  FS_MEMSET(&CardStatus, 0, sizeof(CardStatus));
  do {
    _SendCmd(pInst, CMD_SEND_STATUS, 0, FS_MMC_RESPONSE_FORMAT_R1, ((U32)pInst->Rca) << 16);
    if (_GetResponse(pInst, &CardStatus, sizeof (CardStatus)) == FS_MMC_CARD_RESPONSE_TIMEOUT) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Card did not respond to send card status command.\n"));
      return 1;
    }
    if ((CardStatus.aStatus[1]) || (CardStatus.aStatus[2] & 0x3f)) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Card reports error 0x%x.\n", (((U32)CardStatus.aStatus[1] << 24) | ((U32)CardStatus.aStatus[2] << 16) |
                                                                         ((U32)CardStatus.aStatus[3] <<  8) | (CardStatus.aStatus[4]))));
      pInst->HasError = 1;
      return 1;
    }
    if (CardStatus.aStatus[3] == 0x09) {
      return 0;
    }
  } while (--TimeOut);
  FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC CM: Card ran into timeout.\n"));
  return 1;
}

/**********************************************************
*
*       _Use4BitMode
*/
static void _Use4BitMode(MMC_CM_INST * pInst) {
  MMC_CARD_STATUS CardStatus;

  if (pInst->Use4BitMode == 0) {
    if (pInst->CardType & FS_MMC_CARDTYPE_SD) {
      U8  aSCR[8] = {0};
      U8  aStatus[64] = {0};
      U8  Unit;
      int r;

      _SelectCard(pInst, &CardStatus);
      Unit  = pInst->Unit;
      //
      // Send initially a "get SD Status"
      //
      FS_MMC_HW_X_SetHWNumBlocks(pInst->Unit, 1);
      FS_MMC_HW_X_SetHWBlockLen(pInst->Unit, sizeof(aStatus));
      FS_MMC_HW_X_SetDataPointer(pInst->Unit, aStatus);
      _SendAdvCommand(pInst, ACMD_SD_STATUS, FS_MMC_CMD_FLAG_DATATRANSFER, FS_MMC_RESPONSE_FORMAT_R1, 0, 10);
      _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
      FS_MMC_HW_X_ReadData(Unit, aStatus, sizeof(aStatus), 1);
      //
      // Retrieve if SD card supports 4-bit mode
      //
      FS_MMC_HW_X_SetHWNumBlocks(pInst->Unit, 1);
      FS_MMC_HW_X_SetHWBlockLen(pInst->Unit, sizeof(aSCR));
      FS_MMC_HW_X_SetDataPointer(pInst->Unit, aSCR);
      _SendAdvCommand(pInst, ACMD_SEND_SCR, FS_MMC_CMD_FLAG_DATATRANSFER, FS_MMC_RESPONSE_FORMAT_R1, 0, 10);
      _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
      r = FS_MMC_HW_X_ReadData(pInst->Unit, aSCR, sizeof(aSCR), 1);
      if (r == 0) {
        //
        // SD 4-bit mode supported
        //
        if (aSCR[1] & (1 << 2)) {
          //
          // Setup the card into 4-bit mode
          //
          _SendAdvCommand(pInst, ACMD_SET_BUS_WIDTH, 0, FS_MMC_RESPONSE_FORMAT_R1, BUS_WIDTH_4BIT, 10);
          if (_GetResponse(pInst, &CardStatus, sizeof (CardStatus)) == 0) {
            //
            //  These line have been commented out since it may not work with hardware where an external pull-up on that line (DAT3, pin1)
            //  on the board is not connected or available. As the internal pull-up is between 10-90 kOhm, it may not hurt when an external
            //  is available.
            //
            //
            // _SendAdvCommand(pInst, ACMD_SET_CLR_CARD_DETECT, 0, FS_MMC_RESPONSE_FORMAT_R1, 0, 10);
            // _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
          }
          _DeSelectCard(pInst);
          _SelectCard(pInst, &CardStatus);
          FS_MMC_HW_X_Delay(10);
          //
          // Send a SD send status to check if SD 4-bit mode is working, otherwise we switch back to SD 1-bit mode (hardware reports read timeout)
          //
          FS_MMC_HW_X_SetHWNumBlocks(pInst->Unit, 1);
          FS_MMC_HW_X_SetHWBlockLen(pInst->Unit, sizeof(aStatus));
          FS_MMC_HW_X_SetDataPointer(pInst->Unit, aStatus);
          _SendAdvCommand(pInst, ACMD_SD_STATUS, FS_MMC_CMD_FLAG_DATATRANSFER | FS_MMC_CMD_FLAG_USE_SD4MODE, FS_MMC_RESPONSE_FORMAT_R1, 0, 10);
          _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
          r = FS_MMC_HW_X_ReadData(pInst->Unit, aStatus, sizeof(aStatus), 1);
          if (r == 0) {
            pInst->Use4BitMode = 1;
          } else {
            _SendAdvCommand(pInst, ACMD_SET_BUS_WIDTH, 0, FS_MMC_RESPONSE_FORMAT_R1, BUS_WIDTH_1BIT, 10);
            _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
            pInst->Use4BitMode = 0;
          }
        }
      }
      _DeSelectCard(pInst);
    }
  }
}

/*********************************************************************
*
*       _ReadCSD
*
*  Description:
*    Read the card's CSD (card specific data) registers and check
*    its contents.
*
*  Parameters:
*    Unit      - Device unit number
*    pCSD      - pointer to CSD structure
*
*  Return value:
*    ==0           - CSD has been read and all parameters are valid.
*    <0            - An error has occurred.
*/
static U8 _ReadCSD(MMC_CM_INST * pInst, CM_CSD * pCSD) {
  FS_MEMSET(pCSD, 0, sizeof(CM_CSD));
  _SendCmd(pInst, CMD_SEND_CSD, 0, FS_MMC_RESPONSE_FORMAT_R2, (U32)(pInst->Rca) << 16);
  if (_GetResponse(pInst, pCSD, sizeof (CM_CSD)) != FS_MMC_CARD_NO_ERROR) {
    return 0xff;
  }
  return 0;     /* No problem ! */
}

/*********************************************************************
*
*       _ExecSwitchFunc
*
*/
static int _ExecSwitchFunc(MMC_CM_INST * pInst, int Mode, int Group, U8 Value, U8 * pResp) {
  U32             Arg;
  MMC_CARD_STATUS CardStatus;
  U8              Unit;

  Unit     = pInst->Unit;
  Arg = Mode << 31 | 0x00FFFFFF;
  Arg &= ~(0x0FUL << (Group * 4));
  Arg |= Value << (Group * 4);
  FS_MMC_HW_X_SetHWBlockLen(pInst->Unit, 64);
  FS_MMC_HW_X_SetHWNumBlocks(pInst->Unit, 1);
  FS_MMC_HW_X_SetDataPointer(pInst->Unit, pResp);
  _SendCmd(pInst, CMD_SWITCH_FUNC, FS_MMC_CMD_FLAG_DATATRANSFER, FS_MMC_RESPONSE_FORMAT_R1, Arg);
  if (_GetResponse(pInst, &CardStatus, sizeof (CardStatus)) == FS_MMC_CARD_NO_ERROR) {
    FS_MMC_HW_X_ReadData(Unit, pResp, 64, 1);
  } else {
    return -1;
  }
  return 0;
}

/*********************************************************************
*
*       _CheckAndSwitchToHighSpeed
*
*/
static int _CheckAndSwitchToHighSpeed(MMC_CM_INST * pInst, const CM_CSD * pCSD) {
  int r;
  U32      CCCSupported;

  r = -1;
  if (pInst->AllowHighSpeedMode) {
    CCCSupported = CSD_CCC_CLASSES(pCSD);
    //
    //  If the switch function is supported, we
    //  ask the card whether it supports high speed mode.
    //  This is only true for SD-Cards.
    //  MMCs do not support such a command class
    //
    if ((CCCSupported & (1 << 10)) && (pInst->CardType & FS_MMC_CARDTYPE_SD)) {
      MMC_CARD_STATUS CardStatus;
      U8              aSCR[8];
      U8              ScrVersion;
      _SelectCard(pInst, &CardStatus);
      //
      // Send initially a "get SCR"
      //
      FS_MMC_HW_X_SetHWNumBlocks(pInst->Unit, 1);
      FS_MMC_HW_X_SetHWBlockLen(pInst->Unit, sizeof(aSCR));
      //lint -e{603} suppress "Symol aSCR not initialized"
      FS_MMC_HW_X_SetDataPointer(pInst->Unit, &aSCR[0]);
      _SendAdvCommand(pInst, ACMD_SEND_SCR, FS_MMC_CMD_FLAG_DATATRANSFER, FS_MMC_RESPONSE_FORMAT_R1, 0, 10);
      if (_GetResponse(pInst, &CardStatus, sizeof (CardStatus)) == FS_MMC_CARD_NO_ERROR) {
        FS_MMC_HW_X_ReadData(pInst->Unit, aSCR, sizeof(aSCR), 1);
        //
        //  Check the SCR version
        //
        ScrVersion = _GetBits(aSCR, 56, 59, sizeof(aSCR));
        if (ScrVersion >= SD_SPEC_VER_200) {
          U8 aSwitch[64];
          U32 Data;
          //
          // In case we deal with a card that conforms to the SD specification V2.00
          // we check the available functions on the card
          //
          if (_ExecSwitchFunc(pInst, 0, 0, 1, aSwitch) == 0) {
            Data = _GetBits(aSwitch, 400, 415, 64);
            //
            // Is function 1 and group 1 supported, we try to switch card in high-speed mode
            //
            if (Data & 1) {
              if (_ExecSwitchFunc(pInst, 1, 0, 1, aSwitch) == 0) {
                Data = _GetBits(aSwitch, 376, 379, 64);
                //
                // If the card switches to high-speed mode,
                // the card will respond with "0x01" otherwise the
                // response will be "0x0f"
                //
                if ((Data & 0xF) == 1) {
                  r = 0;
                }
              }
            }
          }
        }
      }
      _DeSelectCard(pInst);
    }
  }
  return r;
}

/*********************************************************************
*
*       _ApplyCSD
*
*  Description:
*    Read the card's CSD (card specific data) registers and check
*    its contents.
*
*  Parameters:
*    Unit      - Device unit number
*    pCSD      - pointer to CSD structure
*
*  Return value:
*    ==0           - CSD has been read and all parameters are valid.
*    <0            - An error has occurred.
*/
static U8 _ApplyCSD(MMC_CM_INST * pInst, const CM_CSD * pCSD, U8 ccs) {
  int   c;
  int   tmp;
  U32   Factor;
  U32   Freq;
  U32   CardSize;
  U8    Unit;
  unsigned CSDVersion;

  Unit = pInst->Unit;
  //
  // CSD version is only check for SD card. MMC card have almost the same
  // CSD structure as SD V1 cards.
  //
  if (pInst->CardType == FS_MMC_CARDTYPE_SD) {
    CSDVersion = CSD_STRUCTURE(pCSD);
  } else {
    CSDVersion = 0;
  }
  //
  // Calculate maximum communication speed according to card specification
  //
  tmp = CSD_TRAN_SPEED(pCSD);
  c = 6 - (tmp & 0x03);
  Freq = _aUnit[c];
  c = ((tmp & 0x78) >> 3); /* filter frequency bits */
  Freq *= _aFactor[c];
  Freq = FS_MMC_HW_X_SetMaxSpeed(Unit, (U16)Freq);
  if (Freq == 0) {
    return 0xff;
  }
  /* Card parameter interpretation */
  if (CSDVersion == 0) {
    //
    //  Calc number of sectors available on the medium
    //
    Factor     = (U16)(1 << CSD_READ_BL_LEN(pCSD)) / BYTES_PER_SECTOR;
    Factor     *= 1 << (CSD_C_SIZE_MULT(pCSD) + 2);
    CardSize   = CSD_C_SIZE(pCSD) + 1;
    CardSize   *= Factor;
    //
    //  Store calced values into medium's instance structure
    //
    pInst->IsWriteProtected = CSD_WRITE_PROTECT(pCSD);
    if (pInst->CardType != FS_MMC_CARDTYPE_MMC) {
      pInst->IsWriteProtected |= FS_MMC_HW_X_IsWriteProtected(Unit);
    }
    pInst->BytesPerSector   = BYTES_PER_SECTOR;
    pInst->NumSectors       = CardSize;
    return 0;
  } else if (CSDVersion == 1) {  // Newer SD V2 cards.
    //
    //  Calc number of sectors available on the medium
    //
    CardSize   = CSD_C_SIZE_V2(pCSD) << 10;
    //
    //  Store calced values into medium's instance structure
    //
    pInst->IsWriteProtected = CSD_WRITE_PROTECT(pCSD) | FS_MMC_HW_X_IsWriteProtected(Unit);
    pInst->BytesPerSector   = BYTES_PER_SECTOR;
    pInst->NumSectors       = CardSize;
    if (ccs) {
      pInst->CardType         = FS_MMC_CARDTYPE_SDHC;
    }
    return 0;

  }
  FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "_ApplyCSD: Unsupported CSD version"));
  return 1;
}

/*********************************************************************
*
*       _MMC_Init
*
*  Description:
*    MMC/SD driver internal function.
*    Initialize the SD host controller and MMC/SD card
*    contents.
*
*  Parameters:
*    Unit      - Device unit number
*
*  Return value:
*    ==0           - Initialization was successful
*    ==1           - An error has occurred.
*/
static int _MMC_Init(MMC_CM_INST * pInst) {
  int               Init;
  int               CardType;
  int               IsPresent;
  int               r;
  CM_CSD            csd;
  MMC_CARD_STATUS   CardStatus;
  U8                Unit;
  U8                ccs = 0;
  U8                aCardId[16];  // Card identifier number (128 bits)

  r = 1;
  if (pInst->IsInited == 0) {
    Unit = pInst->Unit;
    //
    // Initialize all values
    //
    pInst->HasError         = 0;
    pInst->CardType         = 0;
    pInst->Use4BitMode      = 0;
    pInst->IsWriteProtected = 0;
    pInst->BytesPerSector   = 0;
    pInst->Rca              = 0;
    pInst->NumSectors       = 0;
    pInst->MaxWriteBurst    = FS_MMC_HW_X_GetMaxWriteBurst(Unit);
    pInst->MaxReadBurst     = FS_MMC_HW_X_GetMaxReadBurst(Unit);
    pInst->MaxWriteBurstSave= 1;
    pInst->MaxReadBurstSave = 1;
    _HWInit(pInst);
    IsPresent = FS_MMC_HW_X_IsPresent(Unit);
    if (IsPresent != FS_MEDIA_NOT_PRESENT) {
      FS_MMC_HW_X_SetMaxSpeed(Unit, STARTUPFREQ);
      FS_MMC_HW_X_SetResponseTimeOut(Unit, MAX_RESPONSE_TIMEOUT);
      FS_MMC_HW_X_SetReadDataTimeOut(Unit, MAX_READ_TIMEOUT);
      // Set all cards to IDLE mode
      _SendCmd(pInst, CMD_GO_IDLE_STATE, FS_MMC_CMD_FLAG_INITIALIZE, FS_MMC_RESPONSE_FORMAT_NONE , 0);
      FS_MMC_HW_X_Delay(10);
      //
      // Initially send command SEND_IF_COND to determine whether card is SD V2 or SD V1/MMC Vx
      // If card does respond to this command, card is SD V2 compliant.
      //
      FS_MEMSET(&CardStatus, 0, sizeof(CardStatus));
      _SendCmd(pInst, CMD_SEND_IF_COND, 0, FS_MMC_RESPONSE_FORMAT_R1, (0x01 << 8) | 0xAA);
      r = _GetResponse(pInst, &CardStatus, sizeof(CardStatus));
      if (r == FS_MMC_CARD_RESPONSE_TIMEOUT || CardStatus.aStatus[4] != 0xAA) {
        CardType = _CheckCardType(pInst);
      } else {
        CardType = _CheckCardTypeV2(pInst, &ccs);
      }
      if (CardType != FS_MMC_CARDTYPE_NONE) {
        // Request the CID from the MMC/SD card(s)
        _SendCmd(pInst, CMD_ALL_SEND_CID, 0, FS_MMC_RESPONSE_FORMAT_R2 , 0);
        _GetResponse(pInst, &aCardId[0], sizeof (aCardId));
        //
        // Set the relative address of this card (only 1 card is
        // currently supported)
        //
        if (CardType & FS_MMC_CARDTYPE_SD) {
          //
          // A SD card tells us its RCA (relative card address)
          //
          _SendCmd(pInst, CMD_SET_REL_ADDR, 0, FS_MMC_RESPONSE_FORMAT_R1, 0);
          _GetResponse(pInst, &CardStatus, sizeof (CardStatus));
          pInst->Rca = (CardStatus.aStatus[1] << 8) | CardStatus.aStatus[2];
        } else {
          //
          // MMC cards need to set a RCA
          //
          pInst->Rca = 0x0001;
          _SendCmd(pInst, CMD_SET_REL_ADDR, 0, FS_MMC_RESPONSE_FORMAT_R1, (U32)pInst->Rca << 16);
          FS_MMC_HW_X_Delay(10);
          _GetResponse(pInst, &CardStatus, sizeof (CardStatus));

        }
        Init = _ReadCSD(pInst, &csd);
        if (Init != 0) {
           return 1;
        }
        pInst->CardType = CardType;
        if (_CheckAndSwitchToHighSpeed(pInst, &csd) == 0) {
          Init = _ReadCSD(pInst, &csd);
        }

        //
        // Retrieve the Card ID from card to set the card from identification mode to ready mode
        //
        _SendCmd(pInst, CMD_SEND_CID, 0, FS_MMC_RESPONSE_FORMAT_R2, (U32)pInst->Rca << 16);
        pInst->CardType = CardType;
        FS_MMC_HW_X_Delay(1);
        _GetResponse(pInst, &pInst->CardId, sizeof (MMC_CARD_ID));
        _DeSelectCard(pInst);
        Init = _ApplyCSD(pInst, &csd, ccs);
        pInst->IsInited       = 1;
        if (pInst->Allow4bitMode) {
          _Use4BitMode(pInst);
        }
        _SelectCard(pInst, &CardStatus);
        _SetBlockLen(pInst, &CardStatus);
        _DeSelectCard(pInst);
        r = 0;
        FS_MMC_HW_X_Delay(10);
      }
    } else {
      return 1;
    }
  } else {
    r = 0;
  }
  return r;
}

/*********************************************************************
*
*       _ReadSectors
*
*/
static int _ReadSectors(MMC_CM_INST * pInst, U32 SectorNo, void *pBuffer, U32 NumSectors, U16 MaxReadBurst) {
  int             r;
  MMC_CARD_STATUS CardStatus;
  unsigned        CmdFlags;
  U32             SectorSize;
  U8              Unit;
  unsigned        Cmd;
  int             Status;
  U8            * p;

  if (pInst->HasError) {
    return -1;
  }
  r          = 0;
  Unit       = pInst->Unit;
  SectorSize = pInst->BytesPerSector;
  p          = (U8 *)pBuffer;
  // Select card 'rca' as the active card
  if (_SelectCard(pInst, &CardStatus) == 0) {
    do {
      U32 NumSectorsAtOnce;
      U32 Addr;

      NumSectorsAtOnce = MIN(NumSectors, MaxReadBurst);
      _SetBlockLen(pInst, &CardStatus);
      Cmd = (NumSectorsAtOnce == 1) ? CMD_READ_SINGLE_BLOCK : CMD_READ_MULTIPLE_BLOCKS;
      // Issue the read block command
      CmdFlags = FS_MMC_CMD_FLAG_DATATRANSFER;
      if (pInst->Allow4bitMode) {
        if (pInst->Use4BitMode) {
          CmdFlags |= FS_MMC_CMD_FLAG_USE_SD4MODE;
        }
      }
      if (pInst->CardType != FS_MMC_CARDTYPE_SDHC) {
        Addr   = SectorNo * SectorSize;
      } else {
        Addr   = SectorNo;
      }
      FS_MMC_HW_X_SetHWNumBlocks(pInst->Unit, (U16)NumSectorsAtOnce);
      FS_MMC_HW_X_SetDataPointer(pInst->Unit, p);
      _SendCmd(pInst, Cmd, CmdFlags, FS_MMC_RESPONSE_FORMAT_R1, Addr);
      _GetResponse(pInst, &CardStatus, sizeof(CardStatus));
      r = FS_MMC_HW_X_ReadData(Unit, p, SectorSize, NumSectorsAtOnce);
      if (r) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "FS__MMC_CM_ReadSector: Hardware reports error: %d.\n", r));
        r = -1;
      }
      //
      // In case of multi block transfer or we got an error, a stop command is sent to the card.
      //
      if ((Cmd == CMD_READ_MULTIPLE_BLOCKS) || (r == -1)) {
        _SendCmd(pInst, CMD_STOP_TRANSMISSION, 0, FS_MMC_RESPONSE_FORMAT_R1, 0);
        _GetResponse(pInst, &CardStatus, sizeof(CardStatus));
      }
      Status = _Wait4Ready(pInst);
      FS_USE_PARA(Status);
      if (r) {
        break;
      }
      NumSectors -= NumSectorsAtOnce;
      SectorNo   += NumSectorsAtOnce;
      p          += NumSectorsAtOnce * SectorSize;
    } while (NumSectors);
    // Set card 'rca' as an inactive card
    _DeSelectCard(pInst);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC CM: Card could not be selected.\n"));
    r = -1;  // Error
  }
  return r;
}

/*********************************************************************
*
*       _WriteSectors
*
*/
static int _WriteSectors(MMC_CM_INST * pInst, U32 SectorNo, const void *pBuffer, U32 NumSectors, U8 RepeatSame, U16 MaxWriteBurst) {
  MMC_CARD_STATUS CardStatus;
  MMC_CARD_STATUS CardStatus1;
  unsigned        CmdFlags;
  int             r;
  U32             SectorSize;
  int             Status;
  U8              Unit;
  const U8      * p;
  unsigned        Cmd;

  if (pInst->HasError) {
    return -1;
  }
  if (pInst->IsWriteProtected) {
    return -1;
  }
  SectorSize = pInst->BytesPerSector;
  r          = 0;
  Unit       = pInst->Unit;
  p          = (const U8 *)pBuffer;
  CmdFlags   = FS_MMC_CMD_FLAG_WRITETRANSFER | FS_MMC_CMD_FLAG_DATATRANSFER;
  if (pInst->Allow4bitMode && pInst->Use4BitMode) {
    CmdFlags |= FS_MMC_CMD_FLAG_USE_SD4MODE;
  }
  if (RepeatSame) {
    CmdFlags |= FS_MMC_CMD_FLAG_REPEAT_SAME_SECTOR_DATA;
  }
  // Select card 'rca' as the active card
  if (_SelectCard(pInst, &CardStatus) == 0) {
    do {
      U32 NumSectorsAtOnce;
      U32 Addr;

      //
      // In case of write multiple sectors the same data,
      // we set NumSectorsAtOnce to 1
      //
      if (RepeatSame && (pInst->WrRepeatSameAllowed == 0)) {
        NumSectorsAtOnce = 1;
      } else {
        NumSectorsAtOnce = MIN(NumSectors, MaxWriteBurst);
      }
      
      if (pInst->CardType != FS_MMC_CARDTYPE_SDHC) {
        Addr = SectorNo * SectorSize;
      } else {
        Addr  = SectorNo;
      }
      Cmd = (NumSectorsAtOnce == 1) ? CMD_WRITE_BLOCK : CMD_WRITE_MULTIPLE_BLOCKS;
      _SetBlockLen(pInst, &CardStatus);
      //
      // Issue the write block command
      //
      FS_MMC_HW_X_SetHWNumBlocks(Unit, (U16)NumSectorsAtOnce);
      FS_MMC_HW_X_SetDataPointer(pInst->Unit, p);
      _SendCmd(pInst, Cmd, CmdFlags, FS_MMC_RESPONSE_FORMAT_R1, Addr);
      _GetResponse(pInst, &CardStatus1, sizeof (CardStatus1));
      r = FS_MMC_HW_X_WriteData(Unit, p, SectorSize, NumSectorsAtOnce);
      if (r) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "FS__MMC_CM_WriteBurst: Hardware reports error: %d.\n", r));
        r = -1;
      }
      //
      // In case of multi block transfer or we got an error, a stop command is sent to the card.
      //
      if ((Cmd == CMD_WRITE_MULTIPLE_BLOCKS) || (r == -1)) {
        _SendCmd(pInst, CMD_STOP_TRANSMISSION, FS_MMC_CMD_FLAG_SETBUSY | FS_MMC_CMD_FLAG_STOP_TRANS, FS_MMC_RESPONSE_FORMAT_R1, 0);
        _GetResponse(pInst, &CardStatus, sizeof(CardStatus));
      }
      //
      // Wait until card is ready.
      //
      Status = _Wait4Ready(pInst);
      FS_USE_PARA(Status);      
      if (r) {
        break;
      }
      NumSectors -= NumSectorsAtOnce;
      SectorNo   += NumSectorsAtOnce;
      if (RepeatSame == 0) {
        p += NumSectorsAtOnce * SectorSize;
      }
    } while (NumSectors);
    // Set all cards inactive
    _DeSelectCard(pInst);
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER,  "MMC CM: Card could not be selected"));
    r = -1;  // Error
  }
  return r;
}

/*********************************************************************
*
*       _MMC_GetStatus
*
*  Description:
*    Get status of the media.
*
*  Parameters:
*    Unit        - Unit number.
*
*  Return value:
*    FS_MEDIA_STATE_UNKNOWN if the state of the media is unknown.
*    FS_MEDIA_NOT_PRESENT   if no card is present.
*    FS_MEDIA_IS_PRESENT    if a card is present.
*/
static int _MMC_GetStatus(U8 Unit) {
  return FS_MMC_HW_X_IsPresent(Unit); /* Check if a card is present */
}

/*********************************************************************
*
*       _MMC_Unmount
*
*  Description:
*    FS driver function. Unmounts the volume.
*
*  Parameters:
*    Unit        - Unit number.
*
*/
static void _Unmount(MMC_CM_INST * pInst) {
    pInst->IsInited = 0;
}

/*********************************************************************
*
*       _MMC_IoCtl
*
*  Description:
*    Execute device command.
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
  MMC_CM_INST    * pInst;

  pInst = _apInst[Unit];
  FS_USE_PARA(Aux);
  switch (Cmd) {
  case FS_CMD_UNMOUNT:
  case FS_CMD_UNMOUNT_FORCED:
    _Unmount(pInst);
    break;
  case FS_CMD_GET_DEVINFO: /* Get general device information */
    if (pInst->HasError) {
      return -1;
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
  default:
    return -1;
  }
  return 0;
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
  MMC_CM_INST * pInst;
  int           r;
  int           Retry = NUM_RETRIES;

  pInst = _apInst[Unit];
  if (pInst->IsInited) {
OnRetry:
    r = _WriteSectors(pInst, SectorNo, p, NumSectors, RepeatSame, pInst->MaxWriteBurst);
    if (r && Retry) {
      --Retry;
      pInst->MaxWriteBurst = pInst->MaxWriteBurstSave;
      goto OnRetry;
    }
  } else {
    FS_DEBUG_WARN((FS_MTYPE_DRIVER, "Card is not initialized\n"));
    r = -1;
  }
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
  MMC_CM_INST * pInst;
  int           r;
  int           Retry = NUM_RETRIES;

  pInst = _apInst[Unit];
  if (pInst->IsInited) {
OnRetry:
    r = _ReadSectors(pInst, SectorNo, p, NumSectors, pInst->MaxReadBurst);
    if (r && Retry) {
      --Retry;
      pInst->MaxReadBurst = pInst->MaxReadBurstSave;
      goto OnRetry;
    }
  } else {
    FS_DEBUG_WARN((FS_MTYPE_DRIVER, "Card is not initialized\n"));
    r = -1;
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
*    == 0                       - Device okay and ready for operation.
*    <  0                       - An error has occurred.
*/
static int _MMC_InitMedium(U8 Unit) {
  int        r;
  MMC_CM_INST * pInst;

  pInst = _apInst[Unit];
  r     = 0;
  if (pInst->IsInited == 0) {
    r = _MMC_Init(pInst);
    if (r != 0) { /* init failed, no valid card in slot */
      FS_DEBUG_WARN((FS_MTYPE_DRIVER, "MMC CM: Init failure, no valid card found"));
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
  MMC_CM_INST * pInst;

  if (_NumUnits >= NUM_UNITS) {
    return -1;
  }
  Unit = _NumUnits++;
  pInst = (MMC_CM_INST *)FS_AllocZeroed(sizeof(MMC_CM_INST));   // Alloc memory. This is guaranteed to work by the memory module.
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
*       Public data
*
**********************************************************************
*/
const FS_DEVICE_TYPE FS_MMC_CardMode_Driver = {
  _MMC_GetDriverName,
  _MMC_AddDevice,
  _MMC_Read,
  _MMC_Write,
  _MMC_IoCtl,
  _MMC_InitMedium,
  _MMC_GetStatus,
  _MMC_GetNumUnits
};


/*********************************************************************
*
*       FS_MMC_CM_Allow4bitMode
*
*  Description:
*    Allows the driver to use 4bit mode for SD cards.
*    This function shall only be used when configuring the driver
*    in FS_X_AddDevices().
*
*  Parameters:
*    Unit     - Unit number to configure
*    OnOff    - Enable or disables the 4bit mode.
*
*
*/
void FS_MMC_CM_Allow4bitMode(U8 Unit, U8 OnOff) {
  MMC_CM_INST * pInst;

  ASSERT_UNIT_NO_IS_IN_RANGE(Unit);
  pInst = _apInst[Unit];
  if (pInst) {
    pInst->Allow4bitMode = OnOff;
  }
}

/*********************************************************************
*
*       FS_MMC_CM_AllowHighSpeedMode
*
*  Description:
*    Allows the driver to use high speed mode for SD v2 cards.
*    This function shall only be used when configuring the driver
*    in FS_X_AddDevices().
*
*  Parameters:
*    Unit     - Unit number to configure
*    OnOff    - Enable or disables the high speed mode.
*
*
*/
void FS_MMC_CM_AllowHighSpeedMode(U8 Unit, U8 OnOff);
void FS_MMC_CM_AllowHighSpeedMode(U8 Unit, U8 OnOff) {
  MMC_CM_INST * pInst;

  ASSERT_UNIT_NO_IS_IN_RANGE(Unit);
  pInst = _apInst[Unit];
  if (pInst) {
    pInst->AllowHighSpeedMode = OnOff;
  }
}

/*********************************************************************
*
*       FS_MMC_CM_IsWrRepeatSameAllowed
*
*  Description:
*    Allows writing multiple sectors with the same sector data.
*    
*
*  Parameters:
*    Unit     - Unit number to configure
*    OnOff    - Enable or disables the high speed mode.
*
*
*/
void FS_MMC_CM_IsWrRepeatSameAllowed(U8 Unit, U8 OnOff);
void FS_MMC_CM_IsWrRepeatSameAllowed(U8 Unit, U8 OnOff) {
  MMC_CM_INST * pInst;

  ASSERT_UNIT_NO_IS_IN_RANGE(Unit);
  pInst = _apInst[Unit];
  if (pInst) {
    pInst->WrRepeatSameAllowed = OnOff;
  }
}


/*************************** End of file ****************************/
