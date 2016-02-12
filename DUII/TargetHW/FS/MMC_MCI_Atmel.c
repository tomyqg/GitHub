/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2007     SEGGER Microcontroller GmbH & Co KG       *
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
File        : FS_MMC_CM_Atmel.c
Purpose     : FS driver for ATMEL MCI SD/MMC card mode interface.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */

#include "FS_Int.h"
#include "FS_CLib.h"
#include "MMC_MCI_HW.h"
/*********************************************************************
*
*       #define constants
*
**********************************************************************
*/
#ifndef NUM_UNITS
  #define NUM_UNITS         2
#endif

// Transfer is pending.
#define MCI_STATUS_PENDING      1
// Transfer has been aborted because an error occurred.
#define MCI_STATUS_ERROR        2
// Card did not answer command.
#define MCI_STATUS_NORESPONSE   3

// MCI driver is currently in use.
#define MCI_ERROR_LOCK    1


// Start new data transfer
#define MCI_NEW_TRANSFER        0
// Continue data transfer
#define MCI_CONTINUE_TRANSFER   1

// MCI SD Bus Width 1-bit
#define MCI_SDCBUS_1BIT (0 << 7)
// MCI SD Bus Width 4-bit
#define MCI_SDCBUS_4BIT (1 << 7)


// SD card block size binary shift value
#define SECTOR_SIZE_SHIFT       9
// SD card block size in bytes.
#define SECTOR_SIZE           (1 << SECTOR_SIZE_SHIFT)

// -------- MCI_CR : (MCI Offset: 0x0) MCI Control Register --------
#define AT91C_MCI_MCIEN                       (0x1      <<  0) // (MCI) Multimedia Interface Enable
#define AT91C_MCI_MCIDIS                      (0x1      <<  1) // (MCI) Multimedia Interface Disable
#define AT91C_MCI_PWSEN                       (0x1      <<  2) // (MCI) Power Save Mode Enable
#define AT91C_MCI_PWSDIS                      (0x1      <<  3) // (MCI) Power Save Mode Disable
#define AT91C_MCI_SWRST                       (0x1      <<  7) // (MCI) MCI Software reset
// -------- MCI_MR : (MCI Offset: 0x4) MCI Mode Register --------
#define AT91C_MCI_CLKDIV                      (0xFFUL   <<  0) // (MCI) Clock Divider
#define AT91C_MCI_PWSDIV                      (0x07     <<  8) // (MCI) Power Saving Divider
#define AT91C_MCI_RDPROOF                     (0x01     << 11) // (MCI) Read Proof Enable
#define AT91C_MCI_WRPROOF                     (0x01     << 12) // (MCI) Write Proof Enable
#define AT91C_MCI_PDCFBYTE                    (0x01     << 13) // (MCI) PDC Force Byte Transfer
#define AT91C_MCI_PDCPADV                     (0x01     << 14) // (MCI) PDC Padding Value
#define AT91C_MCI_PDCMODE                     (0x01     << 15) // (MCI) PDC Oriented Mode
#define AT91C_MCI_BLKLEN                      (0xFFFFUL << 16) // (MCI) Data Block Length
// MCI Data Timeout Register
#define MCI_DTOR_DTOCYC                       (0x0F    <<  0) // (MCI) Data Timeout Cycle Number
#define MCI_DTOR_DTOMUL                       (0x07    <<  4) // (MCI) Data Timeout Multiplier
#define MCI_DTOR_DTOMUL_1                     (0x00    <<  4) // (MCI) DTOCYC x 1
#define MCI_DTOR_DTOMUL_16                    (0x01    <<  4) // (MCI) DTOCYC x 16
#define MCI_DTOR_DTOMUL_128                   (0x02    <<  4) // (MCI) DTOCYC x 128
#define MCI_DTOR_DTOMUL_256                   (0x03    <<  4) // (MCI) DTOCYC x 256
#define MCI_DTOR_DTOMUL_1024                  (0x04    <<  4) // (MCI) DTOCYC x 1024
#define MCI_DTOR_DTOMUL_4096                  (0x05    <<  4) // (MCI) DTOCYC x 4096
#define MCI_DTOR_DTOMUL_65536                 (0x06    <<  4) // (MCI) DTOCYC x 65536
#define MCI_DTOR_DTOMUL_1048576               (0x07    <<  4) // (MCI) DTOCYC x 1048576
// MCI SD Card Register --------
#define MCI_SDCR_SCDSEL                       (0x3     <<  0) // (MCI) SD Card Selector
#define MCI_SDCR_SCDBUS                       (0x1UL   <<  7) // (MCI) SDCard/SDIO Bus Width
// MCI Command Register bit definitions
#define MCI_CMDR_CMDNB                        (0x3F    <<  0) // (MCI) Command Number
#define MCI_CMDR_RSPTYP                       (0x03    <<  6) // (MCI) Response Type
#define MCI_CMDR_RSPTYP_NO                    (0x00    <<  6) // (MCI) No response
#define MCI_CMDR_RSPTYP_48                    (0x01    <<  6) // (MCI) 48-bit response
#define MCI_CMDR_RSPTYP_136                   (0x02    <<  6) // (MCI) 136-bit response
#define MCI_CMDR_SPCMD                        (0x07    <<  8) // (MCI) Special CMD
#define MCI_CMDR_SPCMD_NONE                   (0x00    <<  8) // (MCI) Not a special CMD
#define MCI_CMDR_SPCMD_INIT                   (0x01    <<  8) // (MCI) Initialization CMD
#define MCI_CMDR_SPCMD_SYNC                   (0x02    <<  8) // (MCI) Synchronized CMD
#define MCI_CMDR_SPCMD_IT_CMD                 (0x04    <<  8) // (MCI) Interrupt command
#define MCI_CMDR_SPCMD_IT_REP                 (0x05    <<  8) // (MCI) Interrupt response
#define MCI_CMDR_OPDCMD                       (0x01    << 11) // (MCI) Open Drain Command
#define MCI_CMDR_MAXLAT                       (0x01    << 12) // (MCI) Maximum Latency for Command to respond
#define MCI_CMDR_TRCMD                        (0x03    << 16) // (MCI) Transfer CMD
#define MCI_CMDR_TRCMD_NO                     (0x00    << 16) // (MCI) No transfer
#define MCI_CMDR_TRCMD_START                  (0x01    << 16) // (MCI) Start transfer
#define MCI_CMDR_TRCMD_STOP                   (0x02    << 16) // (MCI) Stop transfer
#define MCI_CMDR_TRDIR                        (0x01    << 18) // (MCI) Transfer Direction
#define MCI_CMDR_TRTYP                        (0x07    << 19) // (MCI) Transfer Type
#define MCI_CMDR_TRTYP_BLOCK                  (0x00    << 19) // (MCI) MMC/SDCard Single Block Transfer type
#define MCI_CMDR_TRTYP_MULTIPLE               (0x01    << 19) // (MCI) MMC/SDCard Multiple Block transfer type
#define MCI_CMDR_TRTYP_STREAM                 (0x02    << 19) // (MCI) MMC Stream transfer type
#define MCI_CMDR_TRTYP_SDIO_BYTE              (0x04    << 19) // (MCI) SDIO Byte transfer type
#define MCI_CMDR_TRTYP_SDIO_BLOCK             (0x05    << 19) // (MCI) SDIO Block transfer type
#define MCI_CMDR_IOSPCMD                      (0x03    << 24) // (MCI) SDIO Special Command
#define MCI_CMDR_IOSPCMD_NONE                 (0x00    << 24) // (MCI) NOT a special command
#define MCI_CMDR_IOSPCMD_SUSPEND              (0x01    << 24) // (MCI) SDIO Suspend Command
#define MCI_CMDR_IOSPCMD_RESUME               (0x02    << 24) // (MCI) SDIO Resume Command

// MCI Status Register bit definitions
#define MCI_STATUS_CMDRDY                     (0x01    <<  0) // (MCI) Command Ready flag
#define MCI_STATUS_BLKE                       (0x01    <<  3) // (MCI) Data Block Transfer Ended flag
#define MCI_STATUS_DTIP                       (0x01    <<  4) // (MCI) Data Transfer in Progress flag
#define MCI_STATUS_NOTBUSY                    (0x01    <<  5) // (MCI) Data Line Not Busy flag
#define MCI_STATUS_ENDRX                      (0x01    <<  6) // (MCI) End of RX Buffer flag
#define MCI_STATUS_ENDTX                      (0x01    <<  7) // (MCI) End of TX Buffer flag
#define MCI_STATUS_RXBUFF                     (0x01    << 14) // (MCI) RX Buffer Full flag
#define MCI_STATUS_RINDE                      (0x01    << 16) // (MCI) Response Index Error flag
#define MCI_STATUS_RDIRE                      (0x01    << 17) // (MCI) Response Direction Error flag
#define MCI_STATUS_RCRCE                      (0x01    << 18) // (MCI) Response CRC Error flag
#define MCI_STATUS_RENDE                      (0x01UL  << 19) // (MCI) Response End Bit Error flag
#define MCI_STATUS_RTOE                       (0x01UL  << 20) // (MCI) Response Time-out Error flag
#define MCI_STATUS_DCRCE                      (0x01UL  << 21) // (MCI) data CRC Error flag
#define MCI_STATUS_DTOE                       (0x01UL  << 22) // (MCI) Data timeout Error flag
#define MCI_STATUS_OVRE                       (0x01UL  << 30) // (MCI) Overrun flag
#define MCI_STATUS_UNRE                       (0x01UL  << 31) // (MCI) Underrun flag

#define AT91C_PDC_RXTEN       (0x1 <<  0) // (PDC) Receiver Transfer Enable
#define AT91C_PDC_RXTDIS      (0x1 <<  1) // (PDC) Receiver Transfer Disable
#define AT91C_PDC_TXTEN       (0x1 <<  8) // (PDC) Transmitter Transfer Enable
#define AT91C_PDC_TXTDIS      (0x1 <<  9) // (PDC) Transmitter Transfer Disable


/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/
// SD card operation states
#define SD_STATE_STBY     0
#define SD_STATE_DATA     1
#define SD_STATE_RCV      2
// Card type
#define UNKNOWN_CARD   0
#define CARD_SD        1
#define CARD_SDHC      2
#define CARD_MMC       3

// Delay between sending MMC commands
#define MMC_DELAY 0x4ff

#define SD_SECTOR_NO(pInst, SectorNo) (((pInst)->CardType == CARD_SDHC) ? (SectorNo):((SectorNo) << SECTOR_SIZE_SHIFT))

// Status register constants
#define STATUS_READY_FOR_DATA   (1 << 8)
#define STATUS_IDLE             (0 << 9)
#define STATUS_READY            (1 << 9)
#define STATUS_IDENT            (2 << 9)
#define STATUS_STBY             (3 << 9)
#define STATUS_TRAN             (4 << 9)
#define STATUS_DATA             (5 << 9)
#define STATUS_RCV              (6 << 9)
#define STATUS_PRG              (7 << 9)
#define STATUS_DIS              (8 << 9)
#define STATUS_STATE            (0xF << 9)

// Voltage that we can handle (OCR)
#define VDD_27_28                (1UL << 15)
#define VDD_28_29                (1UL << 16)
#define VDD_29_30                (1UL << 17)
#define VDD_30_31                (1UL << 18)
#define VDD_31_32                (1UL << 19)
#define VDD_32_33                (1UL << 20)
// Bit 31 of OCR is set, device is
#define CARD_POWER_UP_BUSY       (1UL << 31)

#define MMC_HOST_VOLTAGE_RANGE     (VDD_27_28 +\
                                    VDD_28_29 +\
                                    VDD_29_30 +\
                                    VDD_30_31 +\
                                    VDD_31_32 +\
                                    VDD_32_33)

#define SD_CCS              (1 << 30)

/// There was an error with the MCI driver.
#define SD_ERROR_MCI             1
/// The SD card did not answer the command.
#define SD_ERROR_NORESPONSE      2
/// The SD card did not answer the command.
#define SD_ERROR_NOT_INITIALIZED 3

// CSD register access macros.
#define SD_CSD(pInst, Bitfield, Bits)   ((((pInst)->csd)[3-(Bitfield) / 32] >> ((Bitfield) & 0x1f)) & ((1 << (Bits)) - 1))
#define SD_CSD_STRUCTURE(pInst)          SD_CSD(pInst, 126, 2) ///< CSD structure 00b  Version 1.0 01b version 2.0 High Cap
#define SD_CSD_TAAC(pInst)               SD_CSD(pInst, 112, 8) ///< Data read-access-time-1
#define SD_CSD_NSAC(pInst)               SD_CSD(pInst, 104, 8) ///< Data read access-time-2 in CLK cycles
#define SD_CSD_TRAN_SPEED(pInst)         SD_CSD(pInst, 96,  8) ///< Max. data transfer rate
#define SD_CSD_READ_BL_LEN(pInst)        SD_CSD(pInst, 80,  4) ///< Max. read data block length
#define SD_CSD_READ_BL_PARTIAL(pInst)    SD_CSD(pInst, 79,  1) ///< Partial blocks for read allowed
#define SD_CSD_WRITE_BLK_MISALIGN(pInst) SD_CSD(pInst, 78,  1) ///< Write block misalignment
#define SD_CSD_READ_BLK_MISALIGN(pInst)  SD_CSD(pInst, 77,  1) ///< Read block misalignment
#define SD_CSD_DSR_IMP(pInst)            SD_CSD(pInst, 76,  1) ///< DSP implemented
#define SD_CSD_C_SIZE(pInst)             ((SD_CSD(pInst, 64,  10) << 2) + SD_CSD(pInst, 62,  2)) ///< Device size
#define SD_CSD_C_SIZE_HC(pInst)          ((SD_CSD(pInst, 64,  6) << 16) + SD_CSD(pInst, 48,  16)) ///< Device size v2.0 High Capacity
#define SD_CSD_VDD_R_CURR_MIN(pInst)     SD_CSD(pInst, 59,  3) ///< Max. read current @VDD min
#define SD_CSD_VDD_R_CURR_MAX(pInst)     SD_CSD(pInst, 56,  3) ///< Max. read current @VDD max
#define SD_CSD_VDD_W_CURR_MIN(pInst)     SD_CSD(pInst, 53,  3) ///< Max. write current @VDD min
#define SD_CSD_VDD_W_CURR_MAX(pInst)     SD_CSD(pInst, 50,  3) ///< Max. write current @VDD max
#define SD_CSD_C_SIZE_MULT(pInst)        SD_CSD(pInst, 47,  3) ///< Device size multiplier
#define SD_CSD_ERASE_BLK_EN(pInst)       SD_CSD(pInst, 46,  1) ///< Erase single block enable
#define SD_CSD_SECTOR_SIZE(pInst)        SD_CSD(pInst, 39,  7) ///< Erase sector size
#define SD_CSD_WP_GRP_SIZE(pInst)        SD_CSD(pInst, 32,  7) ///< Write protect group size
#define SD_CSD_WP_GRP_ENABLE(pInst)      SD_CSD(pInst, 31,  1) ///< write protect group enable
#define SD_CSD_R2W_FACTOR(pInst)         SD_CSD(pInst, 26,  3) ///< Write speed factor
#define SD_CSD_WRITE_BL_LEN(pInst)       SD_CSD(pInst, 22,  4) ///< Max write block length
#define SD_CSD_WRITE_BL_PARTIAL(pInst)   SD_CSD(pInst, 21,  1) ///< Partial blocks for write allowed
#define SD_CSD_FILE_FORMAT_GRP(pInst)    SD_CSD(pInst, 15,  1) ///< File format group
#define SD_CSD_COPY(pInst)               SD_CSD(pInst, 14,  1) ///< Copy flag (OTP)
#define SD_CSD_PERM_WRITE_PROTECT(pInst) SD_CSD(pInst, 13,  1) ///< Permanent write protect
#define SD_CSD_TMP_WRITE_PROTECT(pInst)  SD_CSD(pInst, 12,  1) ///< Temporary write protection
#define SD_CSD_FILE_FORMAT(pInst)        SD_CSD(pInst, 11,  2) ///< File format
#define SD_CSD_CRC(pInst)                SD_CSD(pInst,  1,  7) ///< CRC
#define SD_CSD_MULT(pInst)               (1 << (SD_CSD_C_SIZE_MULT(pInst) + 2))
#define SD_CSD_BLOCKNR(pInst)            ((SD_CSD_C_SIZE(pInst) + 1) * SD_CSD_MULT(pInst))
#define SD_CSD_BLOCKNR_HC(pInst)         ((SD_CSD_C_SIZE_HC(pInst) + 1) * 1024)
#define SD_CSD_BLOCK_LEN(pInst)          (1 << SD_CSD_READ_BL_LEN(pInst))
#define SD_CSD_TOTAL_SIZE(pInst)         (SD_CSD_BLOCKNR(pInst) * SD_CSD_BLOCK_LEN(pInst))
#define SD_CSD_TOTAL_SIZE_HC(pInst)      ((SD_CSD_C_SIZE_HC(pInst) + 1) * 512* 1024)
#define SD_TOTAL_BLOCK(pInst)            ((pInst)->NumSectors)

// SCR register access macros.
#define SD_SCR_BUS_WIDTHS(pScr)        ((pScr[1] >> 16) & 0xF) ///< Describes all the DAT bus that are supported by this card
#define SD_SCR_BUS_WIDTH_4BITS         (1 << 1) ///< 4bit Bus Width is supported
#define SD_SCR_BUS_WIDTH_1BIT          (1 << 0) ///< 1bit Bus Width is supported


#define SD_CMD_POWER_ON_INIT              (0  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_INIT | MCI_CMDR_OPDCMD)
//
// Class 0 & 1 commands: Basic commands and Read Stream commands
//
#define SD_CMD_GO_IDLE_STATE              (0  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE)
#define SD_CMD_MMC_GO_IDLE_STATE          (0  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_OPDCMD)
#define SD_CMD_MMC_SEND_OP_COND           (1  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_OPDCMD)
#define SD_CMD_ALL_SEND_CID               (2  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_136)
#define SD_CMD_MMC_ALL_SEND_CID           (2  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_136  | MCI_CMDR_OPDCMD)
#define SD_CMD_SET_RELATIVE_ADDR          (3  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_MMC_SET_RELATIVE_ADDR      (3  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT | MCI_CMDR_OPDCMD)
#define SD_CMD_SET_DSR                    (4  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_NO   | MCI_CMDR_MAXLAT)    // not tested
#define SD_CMD_SEL_DESEL_CARD             (7  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_SEND_IF_COND               (8  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_SEND_CSD                   (9  | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_136  | MCI_CMDR_MAXLAT)
#define SD_CMD_SEND_CID                   (10 | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_136  | MCI_CMDR_MAXLAT)
#define SD_CMD_MMC_READ_DAT_UNTIL_STOP    (11 | MCI_CMDR_TRTYP_STREAM| MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRDIR   | MCI_CMDR_TRCMD_START | MCI_CMDR_MAXLAT)
#define SD_CMD_STOP_TRANSMISSION          (12 | MCI_CMDR_TRCMD_STOP  | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_STOP_TRANSMISSION_SYNC     (12 | MCI_CMDR_TRCMD_STOP  | MCI_CMDR_SPCMD_SYNC  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_SEND_STATUS                (13 | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_GO_INACTIVE_STATE          (15 | MCI_CMDR_RSPTYP_NO )
#define SD_CMD_SET_BLOCKLEN               (16 | MCI_CMDR_TRCMD_NO    | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_MAXLAT)
#define SD_CMD_READ_SINGLE_BLOCK          (17 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | MCI_CMDR_TRTYP_BLOCK     | MCI_CMDR_TRDIR   | MCI_CMDR_MAXLAT)
#define SD_CMD_READ_MULTIPLE_BLOCK        (18 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | MCI_CMDR_TRTYP_MULTIPLE  | MCI_CMDR_TRDIR   | MCI_CMDR_MAXLAT)
#define SD_CMD_WRITE_BLOCK                (24 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | (MCI_CMDR_TRTYP_BLOCK    &  ~(MCI_CMDR_TRDIR))  | MCI_CMDR_MAXLAT)
#define SD_CMD_WRITE_MULTIPLE_BLOCK       (25 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | (MCI_CMDR_TRTYP_MULTIPLE &  ~(MCI_CMDR_TRDIR))  | MCI_CMDR_MAXLAT)
#define SD_CMD_MMC_WRITE_DAT_UNTIL_STOP   (20 | MCI_CMDR_TRTYP_STREAM| MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48 & ~(MCI_CMDR_TRDIR) | MCI_CMDR_TRCMD_START | MCI_CMDR_MAXLAT) // MMC
#define SD_CMD_WRITE_BLOCK                (24 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | (MCI_CMDR_TRTYP_BLOCK    &  ~(MCI_CMDR_TRDIR))  | MCI_CMDR_MAXLAT)
#define SD_CMD_WRITE_MULTIPLE_BLOCK       (25 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | (MCI_CMDR_TRTYP_MULTIPLE &  ~(MCI_CMDR_TRDIR))  | MCI_CMDR_MAXLAT)
#define SD_CMD_PROGRAM_CSD                (27 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_SET_WRITE_PROT             (28 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_CLR_WRITE_PROT             (29 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_SEND_WRITE_PROT            (30 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_TAG_SECTOR_START           (32 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_TAG_SECTOR_END             (33 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_MMC_UNTAG_SECTOR           (34 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_MMC_TAG_ERASE_GROUP_START  (35 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_MMC_TAG_ERASE_GROUP_END    (36 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_MMC_UNTAG_ERASE_GROUP      (37 | MCI_CMDR_RSPTYP_48)
#define SD_CMD_ERASE                      (38 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_LOCK_UNLOCK                (42 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT) // not tested
#define SD_CMD_APP                        (55 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_GEN                        (56 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)    // not tested
#define SD_CMD_APP_SET_BUS_WIDTH          (6  | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_APP_STATUS                 (13 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_START | MCI_CMDR_TRTYP_BLOCK | MCI_CMDR_TRDIR | MCI_CMDR_MAXLAT)
#define SD_CMD_APP_SEND_NUM_WR_BLOCKS     (22 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_APP_SET_WR_BLK_ERASE_COUNT (23 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_APP_APP_OP_COND            (41 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO)
#define SD_CMD_APP_SET_CLR_CARD_DETECT    (42 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)
#define SD_CMD_APP_SEND_SCR               (51 | MCI_CMDR_SPCMD_NONE  | MCI_CMDR_RSPTYP_48   | MCI_CMDR_TRCMD_NO    | MCI_CMDR_MAXLAT)

#define CACHE_LINE_SIZE              32

#define DELAY(x)                    { volatile int LooPs = x; do {} while(--LooPs); }
/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/

// Bit mask for Status register errors.
#define STATUS_ERRORS (  MCI_STATUS_UNRE  \
                       | MCI_STATUS_OVRE  \
                       | MCI_STATUS_DTOE  \
                       | MCI_STATUS_DCRCE \
                       | MCI_STATUS_RTOE  \
                       | MCI_STATUS_RENDE \
                       | MCI_STATUS_RCRCE \
                       | MCI_STATUS_RDIRE \
                       | MCI_STATUS_RINDE)

// MCI data timeout configuration with 1048576 MCK cycles between 2 data transfers.
#define DTOR_1MEGA_CYCLES           (MCI_DTOR_DTOCYC | MCI_DTOR_DTOMUL)

#define DISABLE                     (0)    // Disable MCI interface
#define ENABLE                      (1)    // Enable MCI interface

// Used to write in to a SFR register.
#define WRITE_SFR_REG(pSFR, RegName, value)     pSFR->RegName = (value)
// Used to read from a SFR register.
#define READ_SFR_REG(pSFR, RegName)             (pSFR->RegName)


/*********************************************************************
*
*       Type definitions
*
**********************************************************************
*/

// MCI end-of-transfer callback function.
typedef void (MCI_CALLBACK)(U8 Status, void * pCommand);

//------------------------------------------------------------------------------
/// MCI Transfer Request prepared by the application upper layer. This structure
/// is sent to the __SendCommand function to start the transfer. At the end of
/// the transfer, the callback is invoked by the interrupt handler.
//------------------------------------------------------------------------------
typedef struct _MCICMD {
  U32            Cmd;             // Command code.
  U32            Arg;             // Command argument.
  U16            SectorSize;      // Size of data buffer in bytes.
  U16            NumSectors;      // Number of Sector to be transfered
  U8             ConTrans;        // Indicate if continue to transfer data
  U8             IsRead;          // Indicates if the command is a read operation.
  U8             ResponseSize;    // Size of SD card response in bytes.
  U8             IsAppCmd;        // Specifies whether it is a Application command is or not
  volatile U8    Status;          // Command status.
  U8           * pData;           // Data buffer.
  U32          * pResp;           // Response buffer.
  MCI_CALLBACK * pfCallback;      // Optional user-provided callback function.
  void         * pArg;            // Optional argument to the callback function.
} MCI_CMD;

typedef struct MCI_HWREG {
  volatile U32 MCI_CR;        // MCI Control Register
  volatile U32 MCI_MR;        // MCI Mode Register
  volatile U32 MCI_DTOR;      // MCI Data Timeout Register
  volatile U32 MCI_SDCR;      // MCI SD Card Register
  volatile U32 MCI_ARGR;      // MCI Argument Register
  volatile U32 MCI_CMDR;      // MCI Command Register
  volatile U32 MCI_BLKR;      // MCI Block Register
  volatile U32 Reserved0[1];  //
  volatile U32 MCI_RSPR[4];   // MCI Response Register
  volatile U32 MCI_RDR;       // MCI Receive Data Register
  volatile U32 MCI_TDR;       // MCI Transmit Data Register
  volatile U32 Reserved1[2];  //
  volatile U32 MCI_SR;        // MCI Status Register
  volatile U32 MCI_IER;       // MCI Interrupt Enable Register
  volatile U32 MCI_IDR;       // MCI Interrupt Disable Register
  volatile U32 MCI_IMR;       // MCI Interrupt Mask Register
  volatile U32 Reserved2[43]; //
  volatile U32 MCI_VR;        // MCI Version Register
  volatile U32 MCI_RPR;       // Receive Pointer Register
  volatile U32 MCI_RCR;       // Receive Counter Register
  volatile U32 MCI_TPR;       // Transmit Pointer Register
  volatile U32 MCI_TCR;       // Transmit Counter Register
  volatile U32 MCI_RNPR;      // Receive Next Pointer Register
  volatile U32 MCI_RNCR;      // Receive Next Counter Register
  volatile U32 MCI_TNPR;      // Transmit Next Pointer Register
  volatile U32 MCI_TNCR;      // Transmit Next Counter Register
  volatile U32 MCI_PTCR;      // PDC Transfer Control Register
  volatile U32 MCI_PTSR;      // PDC Transfer Status Register
} MCI_HWREG;


typedef struct {
  U32         csd[4];               // Card-specific data.
  U32         PreSector;            // Previous access block number.
  U32         NumSectors;           // Card block number
  MCI_HWREG * pMciHw;               // Pointer to a MCI peripheral sfrs.
  MCI_CMD     Command;              // Current MCI command being processed.
  U16         CardAddress;          // SD card current address.
  U16         NumSectorsAtOnce;     // Number of sectors that can be transferred at once
  U8          IsInited;             // Specifies whether the driver is initialized or not.
  U8          State;                // State after SD command complete
  U8          CardType;             // Card type
  U8          IsWriteProtected;     // Write protection switch
  U8          Unit;
} DRIVER_INST;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static DRIVER_INST  * _apInst[NUM_UNITS];
static int            _NumUnits;
//static U32            _aSectorBuffer[(NUM_SECTORS_AT_ONCE * SECTOR_SIZE + CACHE_LINE_SIZE) / 4];

static U32 _NumSectorsAtOnce;
static U32 _PAddrTransferMem;
static U32 _VAddrTransferMem;
static U32 _TransferMemSize;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _EnableMCI
*
*  Function description:
*    Enable/disable a MCI driver instance.
*
*  Parameters:
*    pInst    - Pointer to a MCI driver instance.
*    Enable    - 0 for disable MCI and 1 for enable MCI.
*
*  Return value:
*    void       -
*
*/
static void _EnableMCI(DRIVER_INST * pInst, U8 Enable) {
	MCI_HWREG * pMciHw = pInst->pMciHw;

  // Set the Control Register: Enable/Disable MCI interface clock
  if(Enable == DISABLE) {
    WRITE_SFR_REG(pMciHw, MCI_CR, AT91C_MCI_MCIDIS);
  } else {
    WRITE_SFR_REG(pMciHw, MCI_CR, AT91C_MCI_MCIEN);
  }
}

/*********************************************************************
*
*       _OnSectorsRead
*
*  Function description:
*    Callback function that is called whenever a data read transfer
*    is taken.
*    This makes sure that the ARM9 data cache is invalidated for the
*    memory region, that the new data in memory will be reread.
*
*  Parameters:
*    Status    - Status of the data transfer
*    pCmd      - Pointer to the Command data structure.
*
*
*/
static void _OnSectorsRead(U8 Status, void * pCmd) {
  U32  NumBytes;
  MCI_CMD * pCommand;
  U8 * p;

  FS_USE_PARA(Status);
  pCommand = (MCI_CMD *)pCmd;
  NumBytes = pCommand->SectorSize * pCommand->NumSectors;
  p = (U8 *)_VAddrTransferMem;
  FS_MEMCPY(pCommand->pData, p, NumBytes);
}

/*********************************************************************
*
*       _ISRHandler
*
*  Function description:
*    This is the interrupt service routine for the MCI controller.
*
*/
static void  _ISRHandler(void) {
  DRIVER_INST  * pInst;
  MCI_HWREG * pMciHw;
  MCI_CMD * pCommand;
  U32 Status;
  U8 i;

  pInst = _apInst[0];
  pMciHw   = pInst->pMciHw;
  pCommand = &pInst->Command;
  //
  // Read the Status register
  //
  Status  = READ_SFR_REG(pMciHw, MCI_SR);
  Status &= READ_SFR_REG(pMciHw, MCI_IMR);
  //
  // Check if an error has occurred
  //
  if ((Status & STATUS_ERRORS) != 0) {
    //
    // Check error code
    //
    if ((Status & STATUS_ERRORS) == MCI_STATUS_RTOE) {
      pCommand->Status = MCI_STATUS_NORESPONSE;
    }
    // if the command is SEND_OP_COND the CRC error flag is always present
    // (cf : R3 response)
    else if (((Status & STATUS_ERRORS) != MCI_STATUS_RCRCE) || ((pCommand->Cmd != SD_CMD_APP_APP_OP_COND) && (pCommand->Cmd != SD_CMD_MMC_SEND_OP_COND))) {
      pCommand->Status = MCI_STATUS_ERROR;
    }
  }
  // Check if a transfer has been completed
  if (((Status & MCI_STATUS_CMDRDY) != 0)  || ((Status & MCI_STATUS_ENDRX) != 0) ||
      ((Status & MCI_STATUS_RXBUFF) != 0)  || ((Status & MCI_STATUS_ENDTX) != 0) ||
      ((Status & MCI_STATUS_BLKE)   != 0)  || ((Status & MCI_STATUS_RTOE) != 0)) {
    if (((Status & MCI_STATUS_ENDRX) != 0) || ((Status & MCI_STATUS_RXBUFF) != 0) || ((Status & MCI_STATUS_ENDTX) != 0)) {
      _EnableMCI(pInst, DISABLE);
    }
    // If no error occurred, the transfer is successful
    if (pCommand->Status == MCI_STATUS_PENDING) {
      pCommand->Status = 0;
    }
    // Store the card response in the provided buffer
    if (pCommand->pResp) {
      for (i=0; i < pCommand->ResponseSize; i++) {
        pCommand->pResp[i] = READ_SFR_REG(pMciHw, MCI_RSPR[0]);
      }
    }
    //
    // Disable interrupt
    //
    WRITE_SFR_REG(pMciHw, MCI_IDR, READ_SFR_REG(pMciHw, MCI_IMR));
    //
    // Invoke the callback associated with the current command (if any)
    //
    if (pCommand->pfCallback) {
      (pCommand->pfCallback)(pCommand->Status, (void *)pCommand);
    }
  }
  //
  // Wake task (in case it is waiting for completion interrupt)
  //
  FS_OS_SIGNAL();
}

/*********************************************************************
*
*       _CmdCompleted
*
*  Function description:
*    Returns information, whether command has been successfully sent and processed.
*
*  Parameters:
*    pCommand    -
*
*  Return value:
*    1       - Command completed
*    0       - Command is still being processed
*
*/
static U8 _CmdCompleted(MCI_CMD * pCommand) {
  if (pCommand->Status != MCI_STATUS_PENDING) {
    if (pCommand->Status != 0) {
      FS_DEBUG_LOG((FS_MTYPE_DRIVER, "MCI_IsTxComplete %d\n\r", pCommand->Status));
    }
    return 1;
  } else {
    return 0;
  }
}


/*********************************************************************
*
*       __SendCommand
*
*  Function description:
*    Starts a MCI  transfer. This is a non blocking function. It will return
*    as soon as the transfer is started.
*
*  Parameters:
*    pInst    - Pointer to a MCI driver instance.
*    pCommand    - Pointer the to command structure.
*
*  Return value:
*    0       - Command completed successfully
*
*/
static U8 __SendCommand(DRIVER_INST * pInst) {
  U32          mciIer, ModeReg;
  U32          NumBytes;
  U8         * p;
  MCI_HWREG  * pMciHw = pInst->pMciHw;
  MCI_CMD    * pCommand = &pInst->Command;
  //
  // Command is now being executed
  //
  pCommand->Status = MCI_STATUS_PENDING;
  //
  //  Update hardware regs
  //
  FS_MCI_HW_EnableClock(pInst->Unit, 1);                        // Enable the MCI clock
  _EnableMCI(pInst, DISABLE);                            //Disable MCI clock, for multi-block data transfer
  //
  // Setup PDC data transfer direction
  //
  if(pCommand->SectorSize > 0) {
    if(pCommand->IsRead) {
      WRITE_SFR_REG(pMciHw, MCI_PTCR, AT91C_PDC_RXTEN);
    } else {
      WRITE_SFR_REG(pMciHw, MCI_PTCR, AT91C_PDC_TXTEN);
    }
  }
  WRITE_SFR_REG(pMciHw, MCI_PTCR, AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS);  // Disable transmitter and receiver
  ModeReg = READ_SFR_REG(pMciHw, MCI_MR) & (~(AT91C_MCI_BLKLEN | AT91C_MCI_PDCMODE));
  //
  // Command with DATA stage
  //
  if (pCommand->SectorSize > 0) {
    // Enable PDC mode and set block size
    if(pCommand->ConTrans != MCI_CONTINUE_TRANSFER) {
      WRITE_SFR_REG(pMciHw, MCI_MR, ModeReg | AT91C_MCI_PDCMODE | (pCommand->SectorSize << 16));
    }
    // DATA transfer from card to host
    if (pCommand->IsRead) {
      //
      // We always copy from our temporary buffer to the final one
      // It is not necessary to check if the final buffer can also be used by the DMA since
      // a) There should not be a big performance difference if we write directly into
      //    the final buffer or writing into our temporary buffer first and them copying it to the final one
      // b) In general we should always let the DMA write in our temporary buffer, since this memory is
      //    guaranteed to be fast enough so there are no problems that the DMA is blocked too long (by other tasks which use the same memory)
      //
      NumBytes = pCommand->SectorSize * pCommand->NumSectors;
      WRITE_SFR_REG(pMciHw, MCI_RPR, _PAddrTransferMem);
      pCommand->pfCallback = _OnSectorsRead;
      //
      // Sanity checks
      //
      if (pCommand->NumSectors == 0) {
        pCommand->NumSectors = 1;
      }
      WRITE_SFR_REG(pMciHw, MCI_RCR, NumBytes / 4);
      WRITE_SFR_REG(pMciHw, MCI_PTCR, AT91C_PDC_RXTEN);
      mciIer = MCI_STATUS_ENDRX | STATUS_ERRORS;
    } else {  // DATA transfer from host to card
      // Sanity check
      if (pCommand->NumSectors == 0) {
        pCommand->NumSectors = 1;
      }
      NumBytes = pCommand->SectorSize * pCommand->NumSectors;
      FS_MEMCPY((U8 *)_VAddrTransferMem, pCommand->pData, NumBytes);
      WRITE_SFR_REG(pMciHw, MCI_TPR, _PAddrTransferMem);
/*
      if ((((U32)pCommand->pData & 0x1f)) || (NumBytes & 0x1f)) {
        p = _GetBufferAddr();
        FS_MEMCPY(p, pCommand->pData, NumBytes);
      } else {
        p = pCommand->pData;
      }
      WRITE_SFR_REG(pMciHw, MCI_TPR, (int)p);
*/
      //
      // Update the PDC counter
      //
      WRITE_SFR_REG(pMciHw, MCI_TCR, (NumBytes) / 4);
      // MCI_BLKE notifies the end of Multiblock command
      mciIer = MCI_STATUS_BLKE | STATUS_ERRORS;
    }
  } else {  // No data transfer: stop at the end of the command
    WRITE_SFR_REG(pMciHw, MCI_MR, ModeReg);
    mciIer = MCI_STATUS_CMDRDY | STATUS_ERRORS;
  }
  //
  // Enable MCI clock
  //
  _EnableMCI(pInst, ENABLE);
  //
  // Send the command
  //
  if((pCommand->ConTrans != MCI_CONTINUE_TRANSFER) || (pCommand->SectorSize == 0)) {
    WRITE_SFR_REG(pMciHw, MCI_ARGR, pCommand->Arg);
    WRITE_SFR_REG(pMciHw, MCI_CMDR, pCommand->Cmd);
  }
  //
  // In case of transmit, the PDC shall be enabled after sending the command
  //
  if ((pCommand->SectorSize > 0) && !(pCommand->IsRead)) {
    WRITE_SFR_REG(pMciHw, MCI_PTCR, AT91C_PDC_TXTEN);
  }
  //
  // Ignore data error
  //
  mciIer &= ~(MCI_STATUS_UNRE | MCI_STATUS_OVRE | MCI_STATUS_DTOE | MCI_STATUS_DCRCE);
  //
  // Interrupt enable shall be done after PDC TXTEN and RXTEN
  //
  WRITE_SFR_REG(pMciHw, MCI_IER, mciIer);
  //
  // Wait for command to complete
  // As long as the PDC sends data, pCommand->Status does not change.
  // As soon as the PDC is finished, an interrupt is generated and _ISRHandler() is called.
  // This function will alter the pCommand->Status variable.
  //
  while(_CmdCompleted(pCommand) == 0) {
    FS_OS_WAIT(1000);
  }
  return 0;
}

/*********************************************************************
*
*       _InitMCI
*
*  Function description:
*    Initializes a MCI driver instance and the underlying peripheral
*
*  Return value:
*    Returns a pointer to a MCI instance.
*
*/
static void _InitMCI(DRIVER_INST * pInst){
  unsigned    ClkDiv;
  U32         Mode;
  MCI_INFO    Info;
  MCI_HWREG * pMciHw;

  FS_MCI_HW_Init(pInst->Unit);
  FS_MCI_HW_GetMCIInfo(pInst->Unit, &Info);
  Mode   = Info.Mode;
  pMciHw = (MCI_HWREG *)Info.BaseAddr;
  // Initialize the MCI driver structure
  pInst->pMciHw   = pMciHw;
  FS_MCI_HW_EnableClock(pInst->Unit, ENABLE);                                // Enable the MCI clock
  WRITE_SFR_REG(pMciHw, MCI_CR, AT91C_MCI_SWRST);        // Reset the MCI
  WRITE_SFR_REG(pMciHw, MCI_CR, AT91C_MCI_MCIDIS | AT91C_MCI_PWSDIS);  // Disable the MCI
  WRITE_SFR_REG(pMciHw, MCI_IDR, 0xFFFFFFFF);                          // Disable all the interrupts
  WRITE_SFR_REG(pMciHw, MCI_DTOR, DTOR_1MEGA_CYCLES);                  // Set the Data Timeout Register
  ClkDiv = (FS_MCI_HW_GetMClk(pInst->Unit) / (400000 * 2)) - 1;                         // Set the Mode Register: 400KHz for MCK = 48MHz (CLKDIV = 58)
  WRITE_SFR_REG(pMciHw, MCI_MR, (ClkDiv | (AT91C_MCI_PWSDIV & (0x7 << 8))));
  WRITE_SFR_REG(pMciHw, MCI_SDCR, Mode);                               // Set the SDCard Register
  WRITE_SFR_REG(pMciHw, MCI_CR, AT91C_MCI_MCIEN);                      // Enable the MCI and the Power Saving
  FS_MCI_HW_EnableClock(pInst->Unit, DISABLE);                                               // Disable the MCI peripheral clock.
  FS_MCI_HW_EnableISR(pInst->Unit, _ISRHandler);
}

/*********************************************************************
*
*       _CloseMCI
*
*  Function description:
*    Close a MCI driver instance and the underlying peripheral.
*
*  Parameters:
*    pInst    - Pointer to a MCI driver instance.
*
*/
static void _CloseMCI(DRIVER_INST * pInst) {
  MCI_HWREG * pMciHw = pInst->pMciHw;

  //
  // Initialize the MCI driver structure
  //
  FS_OS_SIGNAL();
//  pInst->Command  = NULL;
  //
  //  Disable Hardware
  //
  FS_MCI_HW_EnableClock(pInst->Unit, 0);
  WRITE_SFR_REG(pMciHw, MCI_CR, AT91C_MCI_MCIDIS);           // Disable the MCI
  WRITE_SFR_REG(pMciHw, MCI_IDR, 0xFFFFFFFF);                // Disable all the interrupts
}

/*********************************************************************
*
*       MCI_SetBusWidth
*
*  Function description:
*    Configure the  MCI SDCBUS in the MCI_SDCR register. Only two modes available
*
*  Parameters:
*    pInst    - Pointer to a MCI driver instance.
*    BusWidth    - MCI_SDCBUS_1BIT or MCI_SDCBUS_4BIT
*
*/
static void _SetBusWidth(DRIVER_INST * pInst, U8 BusWidth) {
  MCI_HWREG * pMciHw = pInst->pMciHw;
  U32 mciSdcr;

  mciSdcr = (READ_SFR_REG(pMciHw, MCI_SDCR) & ~(MCI_SDCR_SCDBUS));
  WRITE_SFR_REG(pMciHw, MCI_SDCR, mciSdcr | BusWidth);
}


/*********************************************************************
*
*       MCI_CheckBusy
*
*  Function description:
*    Check NOTBUSY and DTIP bits of Status register on the given MCI driver.
*
*  Parameters:
*    pInst    - Pointer to a MCI driver instance.
*
*  Return value:
*    0           - Bus is ready
*    1           - Bus is busy
*
*/
static U8 _CheckBusy(DRIVER_INST * pInst) {
  U32 Status;
  MCI_HWREG * pMciHw = pInst->pMciHw;

  // Enable MCI clock
  _EnableMCI(pInst, ENABLE);
  Status = READ_SFR_REG(pMciHw, MCI_SR);
  if(((Status & MCI_STATUS_NOTBUSY)!=0) && ((Status & MCI_STATUS_DTIP)==0)) {
    // Disable MCI clock
    _EnableMCI(pInst, DISABLE);
    return 0;
  } else {
    return 1;
  }
}

/*********************************************************************
*
*       MCI_SetSpeed
*
*  Function description:
*    Configure the  MCI CLKDIV in the MCI_MR register. The max.
*    for MCI clock is MCK/2 and corresponds to CLKDIV = 0
*
*  Parameters:
*    pInst    - Pointer to a MCI driver instance.
*    Frequency   - Frequency given in Hz.
*
*/
static void MCI_SetSpeed(DRIVER_INST * pInst, U32 Frequency) {
  U32         ModeReg;
  unsigned    ClkDiv;
  MCI_HWREG * pMciHw = pInst->pMciHw;

  // Set the Mode Register: 400KHz for MCK = 48MHz (CLKDIV = 58)
  ModeReg = READ_SFR_REG(pMciHw, MCI_MR) & (~AT91C_MCI_CLKDIV);
  // Multimedia Card Interface clock (MCCK or MCI_CK) is Master Clock (MCK)
  // divided by (2*(CLKDIV+1))
  if (Frequency > 0) {
    ClkDiv = (FS_MCI_HW_GetMClk(pInst->Unit) / (Frequency * 2));
    if (ClkDiv > 0) {
      ClkDiv -= 1;
    }
  } else {
     ClkDiv= 0;
  }
  WRITE_SFR_REG(pMciHw, MCI_MR, ModeReg | ClkDiv);
}



/*********************************************************************
*
*       _SendCommand
*
*  Function description:
*    Sends the current SD card driver command to the card.
*
*  Parameters:
*    pInst    - Pointer to a DRIVER_INST driver instance.
*
*  Return value:
*    0          - if successful;
*    != 0       - Returns the transfer Status code or SD_ERROR_MCI
*                 if there was a problem with the MCI transfer.
*
*/
static U8 _SendCommand(DRIVER_INST * pInst) {
  MCI_CMD * pCommand = &(pInst->Command);
  U8        Error;

  //
  // Send command
  //
  Error = __SendCommand(pInst);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "SendCommand: Failed to send command (%d)\n\r", Error));
    return SD_ERROR_MCI;
  }
  if(pCommand->Cmd == SD_CMD_STOP_TRANSMISSION) {
    while (_CheckBusy(pInst) != 0);
  }
  //
  // Delay between sending commands, only for MMC card test.
  //
  if((pInst->CardType == CARD_MMC)||(pInst->CardType == UNKNOWN_CARD)) {
    int i;
    for(i=0; i < MMC_DELAY; i++);
  }
  return pCommand->Status;
}

/*********************************************************************
*
*       _PowerOn
*
*  Function description:
*    Initialization delay: The maximum of 1 msec, 74 clock cycles and supply ramp
*    up time.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _PowerOn(DRIVER_INST * pInst) {
  U8 r;
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_POWER_ON_INIT;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  r = _SendCommand(pInst);
  DELAY(200);
  return r;
}

/*********************************************************************
*
*       _SendCMD0
*
*  Function description:
*    Resets all cards to idle state.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD0(DRIVER_INST * pInst) {
  U8       r;
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_GO_IDLE_STATE;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  r = _SendCommand(pInst);
  DELAY(200);
  return r;
}

/*********************************************************************
*
*       _SendCMD1
*
*  Function description:
*    MMC send operation condition command.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD1(DRIVER_INST * pInst) {
  U8        Error;
  U32       Response;
  MCI_CMD * pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd          = SD_CMD_MMC_SEND_OP_COND;
  pCommand->Arg          = MMC_HOST_VOLTAGE_RANGE;
  pCommand->ResponseSize = 1;
  pCommand->pResp        = &Response;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  Error = _SendCommand(pInst);
  if (Error) {
    return Error;
  }
  if ((Response & CARD_POWER_UP_BUSY) == CARD_POWER_UP_BUSY) {
    return 0;
  } else {
    return SD_ERROR_MCI;
  }
}

/*********************************************************************
*
*       _SendCMD2
*
*  Function description:
*    Asks to all cards to send the CID numbers.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    pCID       - Pointer to a buffer for storing the CID
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD2(DRIVER_INST * pInst, U32 * pCid) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd          = SD_CMD_ALL_SEND_CID;
  pCommand->ResponseSize = 4;
  pCommand->pResp        = pCid;
  //
  // Set SD command state
  //
  pInst->State        = SD_STATE_STBY;
  // Send the command
      return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD3
*
*  Function description:
*    Asks The card to publish a new relative address.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD3(DRIVER_INST * pInst) {
  MCI_CMD *pCommand = &(pInst->Command);
  U32 CardAddress;
  U8 Error;

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_SET_RELATIVE_ADDR;
  // Assign relative address to MMC card
  if (pInst->CardType == CARD_MMC) {
    pCommand->Arg = (0x1 << 16);
  }
  pCommand->ResponseSize = 1;
  pCommand->pResp = &CardAddress;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  Error = _SendCommand(pInst);
  if (Error) {
    return Error;
  }
  // Save card address in driver
  if (pInst->CardType != CARD_MMC) {
    pInst->CardAddress = (U16)((CardAddress >> 16) & 0xFFFF);
  } else {
    // Default MMC RCA is 0x0001
    pInst->CardAddress = 1;
  }
  return 0;
}

/*********************************************************************
*
*       _SendCMD7
*
*  Function description:
*    Toggles a card between the stand-by and the transfer states or between the
*    programming and disconnects states.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    Rca        - Relative card address, that was previously set during initialization
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD7(DRIVER_INST * pInst, U16 Rca) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_SEL_DESEL_CARD;
  pCommand->Arg = Rca << 16;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD8
*
*  Function description:
*    Voltage check.
*
*  Parameters:
*    pInst       - Pointer to a SD card driver instance.
*    SupplyVoltage - Supply voltage supported by SD/MMC HOST
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD8(DRIVER_INST * pInst, U8 SupplyVoltage) {
  MCI_CMD *pCommand = &(pInst->Command);
  U32 Response;
  U8 Error;

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd             = SD_CMD_SEND_IF_COND;
  pCommand->Arg             = (SupplyVoltage << 8) | (0xAA);
  pCommand->ResponseSize    = 1;
  pCommand->pResp           = &Response;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  Error = _SendCommand(pInst);
  // Check result
  if (Error == MCI_STATUS_NORESPONSE) {
    return SD_ERROR_NORESPONSE;
  } else if (!Error && (Response == (U32)((SupplyVoltage << 8) | 0xAA))) {
    return 0;
  } else {
    return SD_ERROR_MCI;
  }
}

/*********************************************************************
*
*       _SendCMD9
*
*  Function description:
*    Addressed card sends its card specific data.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD9(DRIVER_INST * pInst) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_SEND_CSD;
  pCommand->Arg = pInst->CardAddress << 16;
  pCommand->ResponseSize = 4;
  pCommand->pResp = pInst->csd;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD12
*
*  Function description:
*    Forces the card to stop transmission.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD12(DRIVER_INST * pInst) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd      = SD_CMD_STOP_TRANSMISSION;
  pCommand->ConTrans = MCI_NEW_TRANSFER;
  //
  // Set SD command state
  //
  pInst->State         = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD13
*
*  Function description:
*    Addressed card sends its Status register.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    pStatus    - Pointer to a Status variable.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD13(DRIVER_INST * pInst, U32 *pStatus) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_SEND_STATUS;
  pCommand->Arg = pInst->CardAddress << 16;
  pCommand->ResponseSize = 1;
  pCommand->pResp = pStatus;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD16
*
*  Function description:
*    In the case of a Standard Capacity SD Memory Card, this command sets the
*    block length/sector size (in bytes) for all following block commands
*    (read, write, lock).
*    Default block length is fixed to 512 Bytes.
*
*    Set length is valid for memory access commands only if partial block read
*    operation are allowed in CSD.
*
*    In the case of a High Capacity SD Memory Card, block length set by CMD16
*    command does not affect the memory read and write commands. Always 512
*    Bytes fixed block length is used. This command is effective for LOCK_UNLOCK command.
*    In both cases, if block length is set larger than 512Bytes, the card sets the
*    BLOCK_LEN_ERROR bit.

*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    SectorSize - Sector size in bytes.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD16(DRIVER_INST * pInst, U16 SectorSize) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_SET_BLOCKLEN;
  pCommand->Arg = SectorSize;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD18
*
*  Function description:
*    Continuously transfers data blocks from card to host until interrupted by a
*    STOP_TRANSMISSION command.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    SectorNo   - Start sector no. to transfer
*    NumSectors - Number of sector to be transferred
*    pData      - Pointer to the data buffer.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD18(DRIVER_INST * pInst, unsigned NumSectors, U8 *pData, U32 SectorNo) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_READ_MULTIPLE_BLOCK;
  pCommand->Arg = SectorNo;
  pCommand->SectorSize = SECTOR_SIZE;
  pCommand->NumSectors = NumSectors;
  pCommand->pData = pData;
  pCommand->IsRead = 1;
  pCommand->ConTrans = MCI_NEW_TRANSFER;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_DATA;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD25
*
*  Function description:
*    Write block command
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    SectorNo   - Start sector no. to transfer
*    NumSectors - Number of sector to be transferred
*    pData      - Pointer to the data buffer.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD25(DRIVER_INST * pInst, unsigned NumBlocks, U8 *pData, U32 SectorNo) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_WRITE_MULTIPLE_BLOCK;
  pCommand->Arg = SectorNo;
  pCommand->SectorSize = SECTOR_SIZE;
  pCommand->NumSectors = NumBlocks;
  pCommand->pData = (U8 *) pData;
  pCommand->ConTrans = MCI_NEW_TRANSFER;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_RCV;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendCMD55
*
*  Function description:
*    Sends the APP command to indicate that an application command is sent.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendCMD55(DRIVER_INST * pInst) {
  MCI_CMD *pCommand = &(pInst->Command);

  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd = SD_CMD_APP;
  pCommand->Arg = (pInst->CardAddress << 16);
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendACMD6
*
*  Function description:
*    Defines the data bus width (’00’=1bit or ’10’=4 bits bus) to be used for data transfer.
*    The allowed data bus widths are given in SCR register.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    BusWidth   - Bus Width in Bits
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendACMD6(DRIVER_INST * pInst, U8 BusWidth) {
  MCI_CMD *pCommand = &(pInst->Command);
  U8 Error;

  //
  // Send ACMD
  //
  Error = _SendCMD55(pInst);
  if (Error) {
    return Error;
  }
  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->Cmd      = SD_CMD_APP_SET_BUS_WIDTH;
  pCommand->IsAppCmd = 1;
  if (BusWidth == 4) {
    pCommand->Arg = SD_SCR_BUS_WIDTH_4BITS;
  } else {
    pCommand->Arg = SD_SCR_BUS_WIDTH_1BIT;
  }
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_STBY;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _SendACMD41
*
*  Function description:
*    Asks to all cards to send their operations conditions.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    hcs        - Shall be set to 1 if Host support High capacity.
*    pCCS       - Set the pointed flag to 1 if hcs != 0 and SD OCR CCS flag is set.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _SendACMD41(DRIVER_INST * pInst, U8 hcs, U8 *pCCS) {
  MCI_CMD *pCommand = &(pInst->Command);
  U8 Error;
  U32 Response;

  do {
    //
    // Delay
    //
    DELAY(2000);
    Error = _SendCMD55(pInst);
    if (Error) {
      return Error;
    }
    ZEROFILL(pCommand, sizeof(MCI_CMD));
    //
    // Fill command information
    //
    pCommand->Cmd      = SD_CMD_APP_APP_OP_COND;
    pCommand->Arg      = MMC_HOST_VOLTAGE_RANGE;
    pCommand->IsAppCmd = 1;
    if (hcs) {
      pCommand->Arg |= SD_CCS;
    }
    pCommand->ResponseSize = 1;
    pCommand->pResp = &Response;
    //
    // Set SD command state
    //
    pInst->State = SD_STATE_STBY;
    //
    // Send command
    //
    Error = _SendCommand(pInst);
    if (Error) {
      return Error;
    }
    *pCCS  = ((Response & SD_CCS) != 0);
  } while ((Response & CARD_POWER_UP_BUSY) != CARD_POWER_UP_BUSY);
  return 0;
}


/*********************************************************************
*
*       _ContinuousRead
*
*  Function description:
*    Continue to transfer data blocks from host to card until interrupted by a
*    STOP_TRANSMISSION command.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    SectorNo   - Start sector no. to transfer
*    NumSectors - Number of sector to be transferred
*    pData      - Pointer to the data buffer.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _ContinuousRead(DRIVER_INST * pInst, unsigned NumBlocks, U8 *pData, U32 SectorNo) {
  MCI_CMD *pCommand = &(pInst->Command);

  FS_USE_PARA(SectorNo);
  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->SectorSize = SECTOR_SIZE;
  pCommand->NumSectors = NumBlocks;
  pCommand->pData = pData;
  pCommand->IsRead = 1;
  pCommand->ConTrans = MCI_CONTINUE_TRANSFER;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_DATA;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _ContinuousWrite
*
*  Function description:
*    Continue to transfer data blocks from host to card until interrupted by a
*    STOP_TRANSMISSION command.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    SectorNo   - Start sector no. to transfer
*    NumSectors - Number of sector to be transferred
*    pData      - Pointer to the data buffer.
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _ContinuousWrite(DRIVER_INST * pInst, unsigned NumBlocks, const U8 *pData, U32 SectorNo) {
  MCI_CMD * pCommand = &(pInst->Command);

  FS_USE_PARA(SectorNo);
  ZEROFILL(pCommand, sizeof(MCI_CMD));
  //
  // Fill command information
  //
  pCommand->SectorSize = SECTOR_SIZE;
  pCommand->NumSectors = NumBlocks;
  pCommand->pData = (U8 *) pData;
  pCommand->ConTrans = MCI_CONTINUE_TRANSFER;
  //
  // Set SD command state
  //
  pInst->State = SD_STATE_RCV;
  //
  // Send command
  //
  return _SendCommand(pInst);
}

/*********************************************************************
*
*       _MoveToTransferState
*
*  Function description:
*    Move SD card to transfer state. The buffer size must be at
*    least 512 byte long. This function checks the SD card Status register and
*    address the card if required before sending the transfer command.
*
*  Parameters:
*    pInst    - Pointer to a SD card driver instance.
*    SectorNo   - Start sector no. to transfer
*    NumSectors - Number of sector to be transferred
*    pData      - Pointer to the data buffer.
*    IsRead     - Specifies whether a read or write operation
*
*  Return value:
*     Returns 0 if successful
*     Otherwise returns an code describing the Error.
*
*/
static U8 _MoveToTransferState(DRIVER_INST * pInst, U32 SectorNo, unsigned NumSectors, U8 *pData, U8 IsRead) {
  U32 Status;
  U8 Error;

  if((pInst->State == SD_STATE_DATA) || (pInst->State == SD_STATE_RCV)) {
    Error = _SendCMD12(pInst);
    if (Error) {
      return Error;
    }
  }
  pInst->PreSector = SectorNo + (NumSectors-1);
  if(IsRead) {
    // Wait for card to be ready for data transfers
    do {
      Error = _SendCMD13(pInst, &Status);
      if (Error) {
        return Error;
      }
      if (((Status & STATUS_STATE) == STATUS_IDLE) || ((Status & STATUS_STATE) == STATUS_READY) || ((Status & STATUS_STATE) == STATUS_IDENT)) {
        FS_DEBUG_LOG((FS_MTYPE_DRIVER,"state = %x\n\r", (Status & STATUS_STATE) >> 9));
        return SD_ERROR_NOT_INITIALIZED;
      }
      // If the SD card is in sending data state or in receive data state
      if (((Status & STATUS_STATE) == STATUS_RCV) || ((Status & STATUS_STATE) == STATUS_DATA) ){
        FS_DEBUG_LOG((FS_MTYPE_DRIVER,"state = %x\n\r", (Status & STATUS_STATE) >> 9));
      }
    } while (((Status & STATUS_READY_FOR_DATA) == 0) || ((Status & STATUS_STATE) != STATUS_TRAN));
    if ((Status & STATUS_STATE) != STATUS_TRAN) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "SD Card can't be configured in transfer state %x\n\r", (Status & STATUS_STATE) >> 9));
      return SD_ERROR_NOT_INITIALIZED;
    }
    //
    // Read data
    // Move to Sending data state
    //
    Error = _SendCMD18(pInst, NumSectors, pData, SD_SECTOR_NO(pInst,SectorNo));
    if (Error) {
      return Error;
    }
  } else {
    //
    // Wait for card to be ready for data transfers
    //
    do {
      Error = _SendCMD13(pInst, &Status);
      if (Error) {
        return Error;
      }
    }  while ((Status & STATUS_READY_FOR_DATA) == 0);
    //
    // If the SD card is in standby state go in transfer state
    //
    if ((Status & STATUS_STATE) == STATUS_STBY) {
      //
      // Go into transfer state
      //
      Error = _SendCMD7(pInst, pInst->CardAddress);
      if (Error) {
        return Error;
      }
    }
    //
    // Move to Sending data state
    //
    Error = _SendCMD25(pInst, NumSectors, pData, SD_SECTOR_NO(pInst,SectorNo));
    if (Error) {
      return Error;
    }
  }
  return Error;
}



/*********************************************************************
*
*       _ReadSectors
*
*  Function description:
*    Read Block of data in a buffer pointed by pData. The buffer size must be at
*    least 512 byte long. This function checks the SD card Status register and
*    address the card if required before sending the read command.
*
*  Parameters:
*    Unit       - Unit number, which instance shall be used.
*    SectorNo   - Sector that shall be read
*    NumSectors - Number of sector to read
*    pData      - Pointer to the data buffer to store read data
*
*  Return value:
*    0 if successful
*    otherwise returns an code describing the Error.
*
*/
static U8 _ReadSectors(U8 Unit, U32 SectorNo, U8 * p, unsigned NumSectors) {
  U8 Error;
  DRIVER_INST * pInst;

  pInst = _apInst[Unit];
  if((pInst->State == SD_STATE_DATA) && ((pInst->PreSector + 1) == SectorNo)) {
    Error = _ContinuousRead(pInst, NumSectors, p, SD_SECTOR_NO(pInst,SectorNo));
    pInst->PreSector = SectorNo + (NumSectors-1);
  } else {
    Error = _MoveToTransferState(pInst, SectorNo, NumSectors, p, 1);
  }
  return Error;
}

/*********************************************************************
*
*       _WriteSectors
*
*  Function description:
*    Write Block of data pointed by pData. The buffer size must be at
*    least 512 byte long. This function checks the SD card Status register and
*    address the card if required before sending the write command.
*
*  Parameters:
*    Unit       - Unit number, which instance shall be used.
*    SectorNo   - Sector that shall be written
*    NumSectors - Number of sector to write
*    pData      - Pointer to the data buffer to holds the data
*
*  Return value:
*    Returns 0 if successful
*    Otherwise returns an SD_ERROR code.
*
*/
static U8 _WriteSectors(U8 Unit, U32 SectorNo, const U8 * p, unsigned NumSectors) {
  U8 Error;
  DRIVER_INST * pInst;

  pInst = _apInst[Unit];
  if (pInst->IsWriteProtected) {
    return 1;
  }
  if((pInst->State == SD_STATE_RCV)  && ((pInst->PreSector + 1) == SectorNo)) {
    Error = _ContinuousWrite(pInst, NumSectors, p, SD_SECTOR_NO(pInst,SectorNo));
    pInst->PreSector = SectorNo + (NumSectors-1);
  } else {
    Error = _MoveToTransferState(pInst, SectorNo, NumSectors, (U8 *)p, 0);
  }
  return Error;
}

/*********************************************************************
*
*       _Init
*
*  Function description:
*    Run the SDcard initialization sequence. This function runs the initialization
*    procedure and the identification process, then it sets the SD card in transfer
*    state to set the block length and the bus width.
*
*  Parameters:
*    Unit       - Unit number, which instance shall be used.
*
*  Return value:
*    Returns 0 if successful; otherwise returns an code describing the Error.
*
*/
static U8 _Init(U8 Unit) {
  U32             sdCid[4];
  U8              IsCCSet;
  U32             Status;
  U8              Error;
  U8              cmd8Retries = 2;
  U8              Cmd1Retries = 100;
  DRIVER_INST   * pInst;

  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- _Init()\n\r"));
  //
  // Initialize the MCI driver
  //
  pInst  = _apInst[Unit];
  _InitMCI(pInst);
  //
  // Initialize DRIVER_INST structure
  //
  pInst->CardAddress = 0;
  pInst->PreSector   = 0xffffffff;
  pInst->State       = SD_STATE_STBY;
  pInst->CardType    = UNKNOWN_CARD;
  ZEROFILL(&(pInst->Command), sizeof(MCI_CMD));
  //
  // Initialization delay: The maximum of 1 msec, 74 clock cycles and supply ramp up time
  // ‘Supply ramp up time’ provides the time that the power is built up to the operating level (the bus
  // master supply voltage) and the time to wait until the SD card can accept the first command
  // Power On Init Special Command
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Pon()\n\r"));
  Error = _PowerOn(pInst);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  //
  // The command GO_IDLE_STATE (CMD0) is the software reset command and sets card into Idle State
  // regardless of the current card state.
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd0()\n\r"));
  Error = _SendCMD0(pInst);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  //
  // After power-on or CMD0, all cards’ CMD lines are in input mode, waiting for start bit of the next command.
  // The cards are initialized with a default relative card address (RCA=0x0000) and with a default
  // driver stage register setting (lowest speed, highest driving current capability).
  // CMD8 is newly added in the Physical Layer Specification Version 2.00 to support multiple voltage
  // ranges and used to check whether the card supports supplied voltage. The version 2.00 host shall
  // issue CMD8 and verify voltage before card initialization.
  // The host that does not support CMD8 shall supply high voltage range...
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd8()\n\r"));
  do {
    Error = _SendCMD8(pInst, 1);
  } while ((Error == SD_ERROR_NORESPONSE) && (cmd8Retries-- > 0));
  if (Error == SD_ERROR_NORESPONSE) {
    //
    // No response : Ver2.00 or later SD Memory Card(voltage mismatch)
    // or Ver1.X SD Memory Card
    // or not SD Memory Card
    //
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- No response to Cmd8\n\r"));
    //
    // ACMD41 is a synchronization command used to negotiate the operation voltage range and to poll the
    // cards until they are out of their power-up sequence.
    //
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Acmd41()\n\r"));
    Error = _SendACMD41(pInst, 0, &IsCCSet);
    if (Error) {
      //
      // Acmd41 failed : MMC card or unknown card
      //
      Error = _SendCMD0(pInst);
      if (Error) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
        return Error;
      }
      do {
        Error = _SendCMD1(pInst);
      } while ((Error) && (Cmd1Retries-- > 0));
      if (Error) {
        FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
        return Error;
      } else {
        pInst->CardType = CARD_MMC;
      }
    } else {
      if(IsCCSet == 0) {
        pInst->CardType = CARD_SD;
      }
    }
  } else if (!Error) {
    //
    // Valid response : Ver2.00 or later SD Memory Card
    //
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Acmd41()\n\r"));
    Error = _SendACMD41(pInst, 1, &IsCCSet);
    if (Error) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
      return Error;
    }
    if (IsCCSet) {
      pInst->CardType = CARD_SDHC;
    } else {
      pInst->CardType = CARD_SD;
    }
  } else {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  //
  // The host then issues the command ALL_SEND_CID (CMD2) to the card to get its unique card identification (CID) number.
  // Card that is unidentified (i.e. which is in Ready State) sends its CID number as the response (on the CMD line).
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd2()\n\r"));
  Error = _SendCMD2(pInst, sdCid);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  //
  // Thereafter, the host issues CMD3 (SEND_RELATIVE_ADDR) asks the
  // card to publish a new relative card address (RCA), which is shorter than CID and which is used to
  // address the card in the future data transfer mode. Once the RCA is received the card state changes to
  // the Stand-by State. At this point, if the host wants to assign another RCA number, it can ask the card to
  // publish a new number by sending another CMD3 command to the card. The last published RCA is the
  // actual RCA number of the card.
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd3()\n\r"));
  Error = _SendCMD3(pInst);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  //
  // The host issues SEND_CSD (CMD9) to obtain the Card Specific Data (CSD register),
  // e.g. block length, card storage capacity, etc...
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd9()\n\r"));
  Error = _SendCMD9(pInst);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  //
  // At this stage the Initialization and identification process is achieved
  // The SD card is supposed to be in Stand-by State
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd13()\n\r"));
  do {
    Error = _SendCMD13(pInst, &Status);
    if (Error) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
      return Error;
    }
  } while ((Status & STATUS_READY_FOR_DATA) == 0);
  //
  // If the 4 bit bus transfer is supported switch to this mode
  // Select the current SD, goto transfer state
  //
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd7()\n\r"));
  Error = _SendCMD7(pInst, pInst->CardAddress);
  if (Error) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
    return Error;
  }
  if (pInst->CardType != CARD_MMC) {
    //
    // Switch to 4 bits bus width (All SD Card shall support 1-bit, 4 bitswidth
    //
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Acmd6()\n\r"));
    Error = _SendACMD6(pInst, 4);
    if (Error) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
      return Error;
    }
  } else {
    _SetBusWidth(pInst, MCI_SDCBUS_1BIT);
  }
  //
  // In the case of a Standard Capacity SD Memory Card, this command sets the
  // block length (in bytes) for all following block commands (read, write, lock).
  // Default block length is fixed to 512 Bytes.
  // Set length is valid for memory access commands only if partial block read
  // operation are allowed in CSD.
  // In the case of a High Capacity SD Memory Card, block length set by CMD16
  // command does not affect the memory read and write commands. Always 512
  // Bytes fixed block length is used. This command is effective for LOCK_UNLOCK command.
  // In both cases, if block length is set larger than 512Bytes, the card sets the
  // BLOCK_LEN_ERROR bit.
  //
  if (pInst->CardType == CARD_SD) {
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Cmd16()\n\r"));
    Error = _SendCMD16(pInst, SECTOR_SIZE);
    if (Error) {
      FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "Error during initialization (%d)\n\r", Error));
      return Error;
    }
  }
  if(pInst->CardType != CARD_SDHC) {
    pInst->NumSectors = SD_CSD_BLOCKNR(pInst);
  } else {
    pInst->NumSectors = SD_CSD_BLOCKNR_HC(pInst);
  }
  FS_DEBUG_LOG((FS_MTYPE_DRIVER, "TAAC %x, NSAC %x\n\r",SD_CSD_TAAC(pInst),SD_CSD_NSAC(pInst)));
  pInst->IsWriteProtected = FS_MCI_HW_IsCardWriteProtected(Unit);
  if (pInst->CardType == UNKNOWN_CARD) {
    return SD_ERROR_NOT_INITIALIZED;
  } else {
    MCI_SetSpeed(pInst, 20000000);
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- Card Type: %x\n\r", pInst->CardType));
    return 0;
  }
}

/*********************************************************************
*
*       _StopSD
*
*  Function description:
*    Stop the SDcard. This function stops all SD operations.
*
*  Parameters:
*    Unit       - Unit number, which instance shall be used.
*
*  Return value:
*    Returns 0 if successful; otherwise returns an code describing the Error.
*
*/
static U8 _StopSD(U8 Unit) {
  U8             Error;
  MCI_CMD      * pCommand;
  DRIVER_INST * pInst;

  pInst = _apInst[Unit];
  pCommand = &(pInst->Command);
  if(pCommand->ConTrans == MCI_CONTINUE_TRANSFER) {
    FS_DEBUG_LOG((FS_MTYPE_DRIVER, "-D- SD_StopTransmission()\n\r"));
    Error = _SendCMD12(pInst);
    if(Error) {
      return Error;
    }
  }
  _CloseMCI(pInst);
  return 0;
}

/*********************************************************************
*
*       _GetDeviceInfo
*
*  Function description:
*    Retrieves the SD card information such as Number of sectors available
*    and sector size.
*
*  Parameters:
*    Unit       - Unit number, which instance shall be used.
*
*  Return value:
*    == 0 - OK
*    != 0 - Error
*
*/
static U8 _GetDeviceInfo(DRIVER_INST * pInst, FS_DEV_INFO * pDevInfo) {
  pDevInfo->NumHeads        = 0;                              // Heads
  pDevInfo->SectorsPerTrack = 0;                              // Sectors per track
  pDevInfo->NumSectors      = SD_TOTAL_BLOCK(pInst);          // Number of sectors
  pDevInfo->BytesPerSector  = SECTOR_SIZE;                    // We use a default sector size of 512 Bytes
  return 0;
}

/*********************************************************************
*
*       _Unmount
*
*  Description:
*    Unmounts the volume.
*
*  Parameters:
*    Unit        - Unit number.
*
*/
static void _Unmount(U8 Unit) {
  if (_apInst[Unit]->IsInited == 1) {
    FS_MEMSET(_apInst[Unit], 0, sizeof(DRIVER_INST));
  }
}

/*********************************************************************
*
*       Public code (indirectly thru callback)
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetStatus
*
*  Description:
*    FS driver function. Get status of the media.
*
*  Parameters:
*    Unit                  - Unit number.
*
*  Return value:
*    FS_MEDIA_STATE_UNKNOWN - if the state of the media is unknown.
*    FS_MEDIA_NOT_PRESENT   - if no card is present.
*    FS_MEDIA_IS_PRESENT    - if a card is present.
*/
static int _GetStatus(U8 Unit) {
  return FS_MCI_HW_IsCardPresent(Unit);
}

/*********************************************************************
*
*       _Read
*
*  Function Description
*    Driver callback function.
*    Reads one or more logical sectors from storage device.
*
*  Parameters:
*    Unit        - Unit number.
*    Sector      - Sector to be read from the device.
*    pBuffer     - Pointer to buffer for storing the data.
*    NumSectors  - Number of sectors to read
*
*  Return value:
*      0                       - Data successfully written.
*    !=0                       - An error has occurred.
*
*/
static int _Read(U8 Unit, U32 SectorNo, void * p, U32 NumSectors) {
  int r;
  U8  Error;

  if (_apInst[Unit]->IsInited == 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "ReadSector was called before IniMedium was called."));
    return -1;
  }
  do {
    unsigned NumSectorsAtOnce;

    NumSectorsAtOnce = MIN(NumSectors, _apInst[Unit]->NumSectorsAtOnce);
    r = _ReadSectors(Unit, SectorNo, (U8 *)p, NumSectorsAtOnce);
    p           = (void *)((U8 *)p + NumSectorsAtOnce * 512);
    NumSectors -= NumSectorsAtOnce;
    SectorNo   += NumSectorsAtOnce;
  } while (NumSectors);
  if((_apInst[Unit]->State == SD_STATE_DATA) || (_apInst[Unit]->State == SD_STATE_RCV)) {
    Error = _SendCMD12(_apInst[Unit]);
    if (Error) {
      return Error;
    }
  }
  return r;
}

/*********************************************************************
*
*       _Write
*
*  Description:
*    Driver callback function.
*    Writes one or more logical sectors to storage device.
*
*  Parameters:
*    Unit        - Unit number.
*    SectorNo    - Sector number to be written to the device.
*    pBuffer     - Pointer to buffer containing the data to write
*    NumSectors  - Number of sectors to store.
*    RepeatSame  - Repeat the same data to sectors.
*
*  Return value:
*      0                       - Data successfully written.
*    !=0                       - An error has occurred.
*
*/
static int _Write(U8 Unit, U32 SectorNo, const void  * p, U32 NumSectors, U8  RepeatSame) {
  int r;
  U8 Error;

  if (_apInst[Unit]->IsInited == 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "WriteSector was called before IniMedium was called."));
    return -1;
  }
  r = 0;
  if (RepeatSame) {
    do {
      if (_WriteSectors(Unit, SectorNo, (const U8 *)p, 1)) {
        r = -1;
        break;
      }
      SectorNo++;
    } while (--NumSectors);
  } else {
    do {
      unsigned NumSectorsAtOnce;
      NumSectorsAtOnce = MIN(NumSectors, _apInst[Unit]->NumSectorsAtOnce);
      r = _WriteSectors(Unit, SectorNo, (const U8*)p, NumSectorsAtOnce);
      if (r) {
        r = -1;
        break;
      }
      p           = (const void *)((const U8 *)p + NumSectorsAtOnce * 512);
      NumSectors -= NumSectorsAtOnce;
      SectorNo   += NumSectorsAtOnce;
    } while (NumSectors);
  }
  if((_apInst[Unit]->State == SD_STATE_DATA) || (_apInst[Unit]->State == SD_STATE_RCV)) {
    Error = _SendCMD12(_apInst[Unit]);
    if (Error) {
      return Error;
    }
  }
  return r;
}

/*********************************************************************
*
*       _DevIoCtl
*
*  Description:
*    FS driver function. Execute device command.
*
*  Parameters:
*    Unit    - Unit number.
*    Cmd         - Command to be executed.
*    Aux         - Parameter depending on command.
*    pBuffer     - Pointer to a buffer used for the command.
*
*  Return value:
*    Command specific. In general a negative value means an error.
*/
static int _IoCtl(U8 Unit, I32 Cmd, I32 Aux, void *pBuffer) {
  FS_DEV_INFO * pInfo;
  int           r;

  if (_apInst[Unit]->IsInited == 0) {
    FS_DEBUG_ERROROUT((FS_MTYPE_DRIVER, "IoCtl was called before IniMedium was called."));
    return -1;
  }
  r     = -1;
  FS_USE_PARA(Aux);
  switch (Cmd) {
  case FS_CMD_GET_DEVINFO:
    pInfo = (FS_DEV_INFO *)pBuffer;
    r = _GetDeviceInfo(_apInst[Unit], pInfo);
    break;
  case FS_CMD_UNMOUNT:
    //
    // (Optional)
    // Device shall be unmounted - sync all operations and mark it as unmounted
    //
    _Unmount(Unit);
    r = 0;
    break;
  case FS_CMD_UNMOUNT_FORCED:
    //
    // (Optional)
    // Device shall be unmounted - mark it as unmounted without syncing any pending operations
    //
    _StopSD(Unit);
    break;
  case FS_CMD_SYNC:
    //
    // (Optional)
    // Sync/flush any pending operations
    //

    // ToDo: Call the function
    break;
  default:
    break;
  }
  return r;
}

/*********************************************************************
*
*       _InitMedium
*
*  Description:
*    Initialize the specified medium.
*
*  Parameters:
*    Unit    - Unit number.
*
*  Return value:
*/
static int _InitMedium(U8 Unit) {
  int i;
  DRIVER_INST * pInst;

  pInst = _apInst[Unit];
  if (pInst->IsInited == 0) {
    //
    // Initialize transfer memory variables
    // which are used by the DMA to read/write data
    //
    _TransferMemSize = FS_MCI_HW_GetTransferMem(&_PAddrTransferMem, &_VAddrTransferMem);
    if (_TransferMemSize < (1 << SECTOR_SIZE_SHIFT)) {
      FS_DEBUG_WARN((FS_MTYPE_DRIVER, "Size of transfer memory area for DMA is too small. A minimum of 512 bytes is needed."));
      return -1;
    }
    _NumSectorsAtOnce = _TransferMemSize >> SECTOR_SIZE_SHIFT;
    if (_NumSectorsAtOnce < 1) {
      FS_DEBUG_WARN((FS_MTYPE_DRIVER, "Size of transfer memory area for DMA is too small. A minimum of 512 bytes is needed."));
      return -1;
    }
    i = _Init(Unit);
    if (i) { /* init failed, no valid card in slot */
      FS_DEBUG_WARN((FS_MTYPE_DRIVER, "IDE/CF: Init failure, no valid card found"));
      return -1;
    } else {
      pInst->NumSectorsAtOnce = _NumSectorsAtOnce;
      pInst->IsInited = 1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       _AddDevice
*
*  Function Description:
*    Initializes the low-level driver object.
*
*  Return value:
*    >= 0                       - Command successfully executed, Unit no.
*    <  0                       - Error, could not add device
*
*/
static int _AddDevice(void) {
  U8            Unit;
  DRIVER_INST * pInst;

  if (_NumUnits >= NUM_UNITS) {
    return -1;
  }
  Unit = _NumUnits++;
  pInst = (DRIVER_INST *)FS_AllocZeroed(sizeof(DRIVER_INST));   // Alloc memory. This is guaranteed to work by the memory module.
  _apInst[Unit] = pInst;
  return Unit;
}

/*********************************************************************
*
*       _GetNumUnits
*
*  Function description:
*
*  Return value:
*    >= 0                       - Command successfully executed, Unit no.
*    <  0                       - Error, could not add device
*/
static int _GetNumUnits(void) {
  return _NumUnits;
}

/*********************************************************************
*
*       _GetDriverName
*/
static const char * _GetDriverName(U8 Unit) {
  FS_USE_PARA(Unit);
  return "mmc";
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
const FS_DEVICE_TYPE FS_MMC_CM_Driver4Atmel = {
  _GetDriverName,
  _AddDevice,
  _Read,
  _Write,
  _IoCtl,
  _InitMedium,
  _GetStatus,
  _GetNumUnits
};

/*************************** End of file ****************************/
