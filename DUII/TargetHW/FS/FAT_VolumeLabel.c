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
File        : FAT_VolumeLabel.c
Purpose     : FAT File System Layer for handling the volume label
---------------------------END-OF-HEADER------------------------------
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
*       #define constants
*
**********************************************************************
*/

/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/


/*********************************************************************
*
*       Static const
*
**********************************************************************
*/


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CopyShortName
*
*/
static void _CopyVolName(char * pDest, U8 * pSrc, unsigned VolumeLabelSize) {
  int       i;
  char    * p;
  unsigned  MaxLen;
  
  MaxLen = MIN(11 , VolumeLabelSize);
  FS_MEMCPY(pDest, pSrc, MaxLen);
  p = pDest + MaxLen--;
  *p-- = 0;
  for (i = MaxLen; i ; i--) {  
    if (*p == ' ') {
       *p = 0;
    } else {
      break;
    }
    p--;
  }
}

/*********************************************************************
*
*       _FindVolumeDirEntry
*
*/
static FS_FAT_DENTRY * _FindVolumeDirEntry(FS_VOLUME * pVolume, FS_SB * pSB) {
  FS_FAT_DENTRY * pDirEntry;
  FS_DIR_POS      DirPos;

  
  FS_FAT_InitDirEntryScan(&pVolume->FSInfo.FATInfo, &DirPos, 0);
  do {
    pDirEntry = FS_FAT_GetDirEntry(pVolume, pSB, &DirPos);
    if (!pDirEntry) {
      break;
    }
    if (pDirEntry->data[0] == 0) {
      pDirEntry = (FS_FAT_DENTRY*)NULL;
      break;  /* No more entries. Not found. */
    }
    if ((pDirEntry->data[DIR_ENTRY_OFF_ATTRIBUTES] == FS_FAT_ATTR_VOLUME_ID) &&
        (pDirEntry->data[0] != 0xe5)) { /* Attributes does match and not a deleted entry */
      break;
    }
    FS_FAT_IncDirPos(&DirPos);
  
  } while (1);
  return pDirEntry;
}

/*********************************************************************
*
*       _CheckVolumeLabelChar
*
*  Function description
*
*  Return value
* 
*  Review Remarks (OO,RS)
*    - Function name does not indicate return value. Better: IsLegalVolumeChar() ?
*    - "White list" better than "black list" ?
*    - Reference to documentation missing
*    - Header: FD, RV missing
*    
*/
static int _CheckVolumeLabelChar(char c) {
  switch (c) {
  case '"':
  case '&':
  case '*':
  case '+':
  case '-':
  case ',':
  case '.':
  case '/':
  case ':':
  case ';':
  case '<':
  case '=':
  case '>':
  case '?':
  case '[':
  case ']':
  case '\\':
    return 1;     // Illegal character
  }
  return 0;       // Permitted character
}

#if 0
/*********************************************************************
*
*       _IsLegalVolumeChar
*
*  Function description
*    Checks if the given character can be used in a volume label
*
*  Return value
* 
*
*  Review
*    RS 08-07-20
*/
static int _IsLegalVolumeChar(char c) {
  if ((c >= '0') && (c <= '9') {
    return 1;       // Digits are permitted
  }
  if ((c >= 'a') && (c <= 'z') {
    return 1;       // lowercase are permitted
  }
  if ((c >= 'A') && (c <= 'Z') {
    return 1;       // lowercase are permitted
  }
  switch (c) {
  case '&':
  case '*':
  case '+':
  case '-':
  case ',':
  case '.':
  case '/':
  case ':':
  case ';':
  case '<':
  case '=':
  case '>':
  case '?':
  case '[':
  case ']':
  case '\\':
    return 1;     // Legal character
  }
  return 0;       // Not Permitted
}
#endif



/*********************************************************************
*
*       _MakeVolLabelName
*
*  Function description:
*
*/
static void _MakeVolLabelName(FS_83NAME * pVolLabel, const char * pVolumeLabel) {
  char     * p;
  unsigned   MaxLen;
  unsigned   i;

  MaxLen = FS_STRLEN(pVolumeLabel);
  MaxLen = MIN(11, MaxLen);
  FS_MEMSET(&pVolLabel->ac[0], ' ', sizeof(pVolLabel->ac));
  p = &pVolLabel->ac[0];
  for (i = 0; i < MaxLen; i++) {
    char c;
    c = *pVolumeLabel++;
    if (_CheckVolumeLabelChar(c)) {
      *p++ = '_';
    } else {
      *p++ = FS_TOUPPER(c);
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_GetVolumeLabel
*
*  Function description:
*    Retrieves the label for a FAT volume, if it exists.
*
*  Return value:
*   == 0    - O.K.
*   != 0    - Error
*
*/
int FS_FAT_GetVolumeLabel(FS_VOLUME * pVolume, char * pVolumeLabel, unsigned VolumeLabelSize) {
  FS_FAT_DENTRY * pDirEntry;
  FS_SB           sb;

  FS__SB_Create(&sb, &pVolume->Partition);
  //
  // Find the volume label entry
  //
  pDirEntry = _FindVolumeDirEntry(pVolume,  &sb);
  if (pDirEntry) {
    //
    // volume label found, copy the name.
    //
    _CopyVolName(pVolumeLabel, pDirEntry->data, VolumeLabelSize);
  } else {
    //
    // No volume label available
    //
    *pVolumeLabel = 0;
  }
  FS__SB_Delete(&sb);
  return 0;
}

/*********************************************************************
*
*       FS_FAT_GetVolumeLabel
*
*  Function description:
*    Sets a label for a FAT volume.
*
*  Return value:
*   == 0    - O.K.
*   != 0    - Error
*
*/
int FS_FAT_SetVolumeLabel(FS_VOLUME * pVolume, const char * pVolumeLabel) {
  FS_FAT_DENTRY * pDirEntry;
  FS_SB           sb;
  int             r;

  r = -1;
  FS__SB_Create(&sb, &pVolume->Partition);
  //
  // Find the volume label entry
  //
  pDirEntry = _FindVolumeDirEntry(pVolume,  &sb);
  //
  // Create/Delete the volume label
  //
  if (pVolumeLabel) {
    FS_83NAME VolLabel;
      U32 TimeDate;

    TimeDate = FS_X_GetTimeDate();
    //
    _MakeVolLabelName(&VolLabel, pVolumeLabel);
    if (pDirEntry == (FS_FAT_DENTRY *)NULL) {
      pDirEntry = FS_FAT_FindEmptyDirEntry(pVolume, &sb, 0);
    }
    
    FS_FAT_WriteDirEntry83(pDirEntry, &VolLabel, 0, FS_FAT_ATTR_VOLUME_ID, 0, (U16)(TimeDate >> 16), (U16)(TimeDate & 0xffff));
    r = 0;
  } else {
    if (pDirEntry) {
      //
      // Delete this volume label entry
      //
      pDirEntry->data[0] = 0xe5;
      r = 0;
    }
  }
  FS__SB_MarkDirty(&sb);
  FS__SB_Delete(&sb);
  return r;
}

/*************************** End of file ****************************/
