#ifndef DATADECODE_H
#define DATADECODE_H
#endif

#ifndef _WINDOWS
#include "FS.h"
unsigned char breakOutTime ( FS_FILETIME *FileTime );
#endif

void unpack_velocities( u08 *, s16 *, s16 *, s16 *);
float compute_heading (s16 , s16 );
char *formattime (timeforms , char *);
void dodate(U16 *, U16 *, U16 *, U16 );
U32 getTimeStamp ( void );
float dotemp(s08, char);
U32 FileTimeToSeconds(FS_FILETIME *);