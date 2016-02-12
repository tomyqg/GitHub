//
// Track Systems specific defiinitions common to all files - target hardware as well as PC simulation
//
//	These are common to all tasks.
//

#ifndef	_COMMONDEFS_H
#define	_COMMONDEFS_H

// Board Settings
#define MASTER_CLOCK 12000000		// frequency of master clock on our board

//
// Application defines
//
#define NEWSCREEN   (1)

//
// Physical display size
//
#define XSIZE_PHYS 480
#define YSIZE_PHYS 272

//
// Virtual display size
//
// Note: if mofifying these values also change duplicate vales in LCDConf.c
#define NUM_VSCREENS	3
#define VXSIZE_PHYS		(XSIZE_PHYS * 1)
#define VYSIZE_PHYS		(YSIZE_PHYS * NUM_VSCREENS)

#define	NUMBER_LENGTH	9

// universal data elements
typedef unsigned char u08;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef signed char s08;
typedef signed short s16;
typedef signed int s32;
typedef signed long s64;

#ifndef _WINDOWS
typedef char BOOL;
typedef union {unsigned short i; unsigned char c[2];} WORD;		// used for access to each byte
#else
#define Null_Function(A) do{ } while(0)
#define OS_Use(A) Null_Function(A)
#define OS_Unuse(A) Null_Function(A)
#endif

#define	true	1
#define	false	0
#define	TRUE	true
#define	FALSE	false

#define	FIVE_SECONDS		5000

#endif  /* _COMMONDEFS_H */
