#ifndef TQMFILE_H
#define TQMFILE_H

#include "FS_Types.h"

// number of seconds from t_date used by Traqview 1/1/1970 to gps start data 1/6/1980, 2 leap years
#define T_DATE_TO_GPS_DATE		((((1980 - 1970) * 365) + 2 + (6 - 1)) * 24 * 60 * 60)
#define SECONDS_IN_A_WEEK		(7 * 24 * 60 * 60)

void CreateTqmFileHeader( char *, BOOL );
void TQMWriteTask ( void );

#endif
