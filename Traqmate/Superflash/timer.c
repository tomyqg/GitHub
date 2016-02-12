// Part of traqmate.c
// 9/6/2004
// 5/15/2007 added autocross mode
// Author: GAStephens
//
// These functions perform the lap timer function for the display unit
//

#include <display.h>
#include <math.h>
#define PI 3.14159265359
#define CIRCUM		40455.0										// circumference of earth in km
#define TODEGREE(A) ((A) * (180.0 / 2147483647.0))		// converts from storage format to degrees
#define LATMET 		(1000.0*CIRCUM/360.0)						// meters per degree
#define LONMET(B)	(1000.0*CIRCUM*cos((B)/57.29577951)/360.0)	// meters per degree, B must be current latitude

/******************************************************************************
** Write_Timer **************************************** GAS 6 SEP 04 ****
*******************************************************************************
This function writes the timer to the screen.
******************************************************************************/
void Write_Timer(void) {
	xdata s32 temp;
	xdata s32 displaytime;
	u08 digitoffset;
						  
	// hold the previous timer on screen for X seconds
	if (lap.currentlap > 1 && tm_state == TIMING && lap.timer < (1000L * (long) scratch.calibrate.lapholdtime))
		displaytime = laplist[lap.currentlap-2];	// -2 because it is 1 based
	else
		displaytime = lap.timer;

	if (REV == timingmode) digitoffset = 0;		// put lap times in center of screen for reviewing
	else digitoffset = 5;						// put lap times above bar graph

	// don't write garbage on screen
	if (displaytime < 0) displaytime = 0;

	// write the hour
	temp = (displaytime / (60L * 60L * 1000L)) % 24;
	Write_Icon(21, 9-digitoffset, 8, 14, &(largeascii['0' - ' '+ (temp % 10)][0][0]), '-');

	// write the minutes
	temp = (displaytime / (60L * 1000L)) % 60;
	Write_Icon(38, 9-digitoffset, 8, 14, &(largeascii['0' - ' ' + (temp / 10)][0][0]), '-');
	Write_Icon(49, 9-digitoffset, 8, 14, &(largeascii['0' - ' ' + (temp % 10)][0][0]), '-');

	// write the seconds
	temp = (displaytime / 1000L) % 60;
	Write_Icon(66, 9-digitoffset, 8, 14, &(largeascii['0' - ' ' + (temp / 10)][0][0]), '-');
	Write_Icon(77, 9-digitoffset, 8, 14, &(largeascii['0' - ' ' + (temp % 10)][0][0]), '-');

	// write the tenth
	temp = displaytime / (100L);
	Write_Icon(94, 9-digitoffset, 8, 14, &(largeascii['0' - ' ' + (temp % 10)][0][0]), '-');

} // Write_Timer

/******************************************************************************
** Write_Qual **************************************** GAS 11 AUG 07 ****
*******************************************************************************
This function writes the qualifying graph and digits to the screen.
******************************************************************************/
void Write_Qual(float sectoradd) {
	xdata BOOL plus;		// TRUE if gap positive
	xdata u16 pixels;		// pixel scale for graph
	xdata u08 digit1, digit2;
	
	// clear the graph
	Write_Icon(0, 23, 122, 8, (u08 *) graphbckgnd, '-');
	
	// up or down?
	plus = (sectoradd >= 0.0);
	
	// remove the sign and round up to nearest tenth
	sectoradd = FLOATABS(sectoradd) + 0.05;
		
	// scale to number of pixels, rounding up, checking bounds
	pixels = MIN(((u16) (.5 + (sectoradd / graphratio * 60.0))), 60);

	// limit the value
	if (sectoradd > 99.0) sectoradd = 99.0;

	if (sectoradd >= 10.0) {	// double digit
		digit1 = (int) sectoradd / 10;
		digit2 = (int) sectoradd % 10;

		// write the sign, gap, and graph
		if (plus) {
			Write_Icon(95, 23, 8, 8, (u08 *) plussign, '-');
			Write_Icon(105, 23, 6, 8, &(smallbolddigits[digit1][0][0]), '-');
			Write_Icon(113, 23, 6, 8, &(smallbolddigits[digit2][0][0]), '-');
			Write_Icon(62, 23, pixels, 8, (u08 *) reversebar, '^');
		} // if
		else {
			Write_Icon(3, 23, 8, 8, (u08 *) minussign, '-');
			Write_Icon(13, 23, 6, 8, &(smallbolddigits[digit1][0][0]), '-');
			Write_Icon(21, 23, 6, 8, &(smallbolddigits[digit2][0][0]), '-');
			Write_Icon(60-pixels, 23, pixels, 8, (u08 *) reversebar, '^');
		} // else
	} // if
	else { // single digit and tenth
		digit1 = (int) sectoradd % 10;
		digit2 = ((int) (sectoradd * 10.0)) % 10;

		// write the sign, gap, and graph
		if (plus) {
			Write_Icon(91, 23, 8, 8, (u08 *) plussign, '-');
			Write_Icon(101, 23, 6, 8, &(smallbolddigits[digit1][0][0]), '-');
 			Write_Icon(109, 23, 2, 8, (u08 *) smallboldperiod, '-');
			Write_Icon(113, 23, 6, 8, &(smallbolddigits[digit2][0][0]), '-');
			Write_Icon(62, 23, pixels, 8, (u08 *) reversebar, '^');
		} // if
		else {
			Write_Icon(3, 23, 8, 8, (u08 *) minussign, '-');
			Write_Icon(13, 23, 6, 8, &(smallbolddigits[digit1][0][0]), '-');
 			Write_Icon(21, 23, 2, 8, (u08 *) smallboldperiod, '-');
			Write_Icon(25, 23, 6, 8, &(smallbolddigits[digit2][0][0]), '-');
			Write_Icon(60-pixels, 23, pixels, 8, (u08 *) reversebar, '^');
		} // else
	} // else

} // Write_Qual

/******************************************************************************
** Write_Lap ********************************************** GAS 29 JAN 05 ****
*******************************************************************************
This function writes the lap number to the screen.
******************************************************************************/
void Write_Lap(void) {
	xdata u08 tempstring[5];

	// write lap number on lower left of screen
	if (lap.currentlap <= 99)
		sprintf(tempstring, "%02u ", lap.currentlap);
	else
		sprintf(tempstring, "%03u", lap.currentlap);
		
	// upper left
	Write_Med_Text((u08) 1, (u08) 1, tempstring, '-');

} // Write_Lap

#if 0
/******************************************************************************
** Write_Sigstrength *************************************** GAS 29 JAN 05 ****
*******************************************************************************
This function writes the signal strength meter on the upper left of the screen
******************************************************************************/
void Write_Sigstrength(BOOL replayicon) {

	if (replayicon)
		// Write the replay icon
		Write_Icon(0, 0, 12, 10, (u08 *) replay, '-');
	else
		// Write the appropriate GPS Lock Icon
		Write_Icon(0, 0, 10, 10, &(sigstrength[(gpslock+2)/3][0][0]), '-');

} // Write_Sigstrength
#endif

/******************************************************************************
** Write_Flag ********************************************** GAS 29 JAN 05 ****
*******************************************************************************
This function writes/clears the best lap (flag) indicator to the screen.
******************************************************************************/
void Write_Flag(BOOL on) {

	if (on)
		// write flag icon on upper right of screen
		Write_Icon(LAST_COLUMN - 10, 1, 10, 10, (u08 *) flag, '-');
	else
		// remove flag icon on upper right of screen
		Write_Icon(LAST_COLUMN - 10, 1, 10, 10, (u08 *) blankicon, '-');
			
} // Write_Flag

#if 0	// removed when LAP mode changed for 2.10
/******************************************************************************
** Write_Arrow ********************************************** GAS 29 JAN 05 ****
*******************************************************************************
This function writes up or down arrow indicator to the screen.
******************************************************************************/
void Write_Arrow(BOOL up) {

	if (up)
		// write up arrow icon on upper right of screen
		Write_Icon(LAST_COLUMN - 11, 2, 11, 10, (u08 *) arrowup, '-');
	else
		// write down arrow icon on upper right of screen
		Write_Icon(LAST_COLUMN - 11, 2, 11, 10, (u08 *) arrowdown, '-');

} // Write_Arrow
#endif

/******************************************************************************
** Lap_Check **************************************** GAS 8 SEP 04 ****
*******************************************************************************
This function performs all numerical lap timer functions.
Could be enhanced to returns TRUE if we are at the Start/Finish Line
******************************************************************************/
void Lap_Check(lapop oper, gpstype *gps) {
#ifdef DEBUG
	xdata char tempstring[21];
#endif
	xdata float tempfloat;
	static BOOL inrange;
 
 	switch (oper) {
		case INC_TIME:
			// increment by msec
			lap.timer += 1000 / (samps_per_sec);
			break;

		case INITIALIZE:
			inrange = FALSE;
			seeklat = lap.startpos.lat;
			seeklon = lap.startpos.lon;
			seekhdg = lap.startheading;
			lap.timer = lap.timernminus1 = 0;
			refsectornum = cursectornum = 0;
			
		case NEW_POSITION: {
			xdata float fdeltalat, fdeltalon;
			xdata s16 eastvel, northvel, vertvel;

			// **** DOES THIS EVER GET SAVED IN FLASH?
			// in launch mode, now that we are moving, save the heading one second after launch
			if ((AUTOCROSS == tm_state || AUTOXFINISH == tm_state) && AUTOXSETHDGTIME == lap.timer)
				lap.startheading = lap.currentheading;

			lap.previouspos = lap.currentpos;
			lap.previousheading = lap.currentheading;
			lap.currentpos = *gps;

			// don't look for starting line in AutoX
			if (AUTOXSTAGE == tm_state) return;

			unpack_velocities (gps->velpack, &eastvel, &northvel, &vertvel);

			// skip all this if we are not moving to eliminate divide by zero
			if (eastvel != 0 || northvel != 0) {
				xdata int headrange;
				xdata int distrange;

				// use a smaller range circle for segment timing and autocross
				if (TIMING == tm_state || WAITSTART == tm_state) {
					headrange = LAP_HEADRANGE;
					distrange = LAP_DISTRANGE;
				} // if
				else {								
					headrange = AUTOX_HEADRANGE;
					distrange = AUTOX_DISTRANGE;
				} // else

				// compute heading = vector sum of east/west vel and north/south vel (due east is 0)
				lap.currentheading = compute_heading(eastvel, northvel);
//				if (WAITSTART != tm_state && INITIALIZE == oper) // we are done
//					return;

				// calculate lap distance
				tempfloat = TODEGREE(lap.previouspos.lat);

				// find difference in meters of lat, lon
				fdeltalat = ((TODEGREE(lap.currentpos.lat) - tempfloat) * LATMET);
				fdeltalon = ((TODEGREE(lap.currentpos.lon) - TODEGREE(lap.previouspos.lon)) * LONMET(tempfloat));

				// remember the distance at the last sample point
				lastdist = (s16) lap.distance;
				
				// add on distance traveled in this sample
				lap.distance += sqrt(fdeltalat * fdeltalat + fdeltalon * fdeltalon);

				if (lap.currentlap > 0 && lastdist > 0) {	// only process after passing S/F and collecting one more sample
					s16 lapdist = ((s16) lap.distance) % tracksampledist;		// integer lap distance - limit 40 miles long!!

					// get just the remainder in this sector
					lastdist = lastdist % tracksampledist;

					// process sectors if we are in a lap
					if (TIMING == tm_state ||
						HILLCLIMBFINISH == tm_state ||
						HILLCLIMBGETFINISH == tm_state ||
						AUTOCROSS == tm_state ||
						AUTOXFINISH == tm_state) {
	
						if (lapdist < lastdist) {	// crossed sector boundary
							xdata float sectoradd;		// bit to add to last timing measurement to get to sector bounday
							// interpolate and save the time
							sectoradd = (float) (tracksampledist - lastdist) /
										(float) (lapdist + tracksampledist - lastdist); // ratio between points
							sectoradd = sectoradd * (float) (lap.timer - lap.timernminus1); // time in ms to add to previous
	
							// if no best time then reference sectors all zero. otherwise check them
							if (0xFFFFFFFF != lap.besttime) {
								// are we out of comparison sectors?
								if (0L == sector[refsectornum].reference && refsectornum > 0)
									// if so, back up a sector
									refsectornum--;
							} // if

							// store interpolated sector time as current for comparison
							sector[cursectornum].current = lap.timernminus1 + (s32) sectoradd;
	
							// only update graph until we run out of sectors
							if (cursectornum < (NUMSECTORS-1)) {
								// if sector data loaded then compare progress and display graph						
								if (0xFFFFFFFF != lap.besttime) {
									// determine how much ahead / behind. + is ahead, - is behind
									sectoradd = (float) (sector[refsectornum].reference - sector[cursectornum].current) / 1000.0;
		
									// write the graph
									Write_Qual(sectoradd);
								} // if
								cursectornum++;
							} // if
							if (refsectornum < (NUMSECTORS-1))
								refsectornum++;
						} // if
					} // if
				} // if
				// save the current timer for future interpolation
				lap.timernminus1 = lap.timer;
				
				// if we are waiting for user action, don't bother going any further
				if (HILLCLIMBGETFINISH == tm_state || AUTOCROSS == tm_state)
					return;

				// if current heading matches the start/finish heading then check for position
				// while allowing for headings near 0 (due east)
				if (ABS((s16) lap.currentheading - (s16) seekhdg) < headrange ||
					ABS((s16) lap.currentheading - (s16) seekhdg) > (360 - headrange)) {

					tempfloat = TODEGREE(seeklat);

					// find difference in meters of lat, lon
					fdeltalat = ((TODEGREE(lap.currentpos.lat) - tempfloat) * LATMET);
					fdeltalon = ((TODEGREE(lap.currentpos.lon) - TODEGREE(seeklon)) * LONMET(tempfloat));

					if (ABS(fdeltalat) < distrange && ABS(fdeltalon) < distrange) {
						// found either the Start or the Finish
						if (!inrange) {
							xdata float distance;
							xdata float speed;
							xdata s32 offset;
							xdata BOOL dofirststart = FALSE;
							xdata BOOL dostart = FALSE;
							xdata BOOL dofinish = FALSE;

							// now we are in range
							inrange = TRUE;

							// find the actual distance to start/finish in meters
							distance = sqrt(fdeltalat * fdeltalat + fdeltalon * fdeltalon);

							// calculate speed in meters/msec
							speed = sqrt((float) eastvel * (float) eastvel + (float) northvel * (float) northvel);

							// fix up the timer and lap distance based on our speed, rounding up the hundredth
							offset = (s32) (50.0 + (1000.0 * distance) / speed);

							// process the incoming states							
							switch (tm_state) {
								case WAITSTART:
								case HILLCLIMBWAITSTART:
									dofirststart = TRUE;
									break;
								case HILLCLIMBSTART:
									dostart = TRUE;
									break;
								case TIMING:
									dostart = TRUE;
									dofinish = TRUE;
									break;
								case HILLCLIMBFINISH:
								case AUTOXFINISH:
									dofinish = TRUE;
									break;
							} // switch
						
							// first time start line stuff
							if (dofirststart) {
								u08 i;

								// found starting line so begin timing
								lap.currentlap = 0;
								lap.lastlap = 0;
								
//								Write_Timer();

								// initialize lap storage
								for (i = 0; i < NUMLAPS; i++)
									laplist[i] = 0xFFFFFFFF;
							} // if
						
							// general finish line stuff
							if (dofinish) {
								// fudge to where time would be when we cross s/f, rounding up
								lap.timer += offset;

								// store the lap plus a fudge based on our speed
								if (lap.currentlap <= NUMLAPS)
									laplist[lap.currentlap-1] = (u32) lap.timer;

								// autox is single 'lap' so don't show symbols
								if (AUTOXFINISH != tm_state) {	// put up all the symbols and get ready for next lap
									// clear the graph
									Write_Icon(0, 23, 122, 8, (u08 *) graphbckgnd, '-');
	
									// check for best lap
									if (lap.timer <= (u32) lap.besttime) {
										lap.besttime = lap.timer;
		
										// put checker up
										Write_Flag(TRUE);

										// make current lap the reference lap
										for (cursectornum = 0; cursectornum < NUMSECTORS; cursectornum++)
											sector[cursectornum].reference = sector[cursectornum].current;
									} // if
									else
										Write_Flag(FALSE);
								} // if
							} // if

							// general start line stuff
							if (dostart || dofirststart) {
								// initialize to where start will be
								lap.timer = -offset;
								lap.distance = -distance;
								lastdist = 0;
								lap.timernminus1 = 0;
								// clear the current sector data
								for (cursectornum = 0; cursectornum < NUMSECTORS; cursectornum++)
									sector[cursectornum].current = 0;
								cursectornum = 0;
								refsectornum = 0;

								// next lap please
								lap.currentlap++;
									
								// write new lap number on screen
								Write_Lap();
							} // if
							
							// do state transitions				
							switch (tm_state) {
								case WAITSTART:
									tm_state = TIMING;
									Write_Screen((u08 *) predictive, '-');
									if (0xFFFFFFFF == lap.besttime) {
										// no sectors
										Write_Icon(111, 23, 8, 8, (u08 *) plussign, '-');
										Write_Icon(3, 23, 8, 8, (u08 *) minussign, '-');
									} // if
									break;
								case HILLCLIMBWAITSTART:
									// has finish line been set?
									if (0L == lap.finishpos.lat) {
										tm_state = HILLCLIMBGETFINISH;
										Clear_Screen();
										Write_Icon(21, 4, 81, 14, (u08 *) timingicon, '-');
										Write_Text(3, 1, "Hit SELECT at Finish", '-');
										button_overlay[SEL] = markfinish;
									} // if
									else { // finish line set
										// set up to find the finish
										seeklat = lap.finishpos.lat;
										seeklon = lap.finishpos.lon;
										seekhdg = lap.finishheading;
										tm_state = HILLCLIMBFINISH;
										Write_Screen((u08 *) predictive, '-');
										// if no sector times show +- while gathering sectors on next lap
										if (0xFFFFFFFF == lap.besttime) {
											// no sectors
											Write_Icon(111, 23, 8, 8, (u08 *) plussign, '-');
											Write_Icon(3, 23, 8, 8, (u08 *) minussign, '-');
										} // if
										button_overlay[SEL] = NULL;
									} // else
									// write lap number on screen
									Write_Lap();
									break;
								case HILLCLIMBSTART:
									// set up to find the finish
									seeklat = lap.finishpos.lat;
									seeklon = lap.finishpos.lon;
									seekhdg = lap.finishheading;
									tm_state = HILLCLIMBFINISH;
									break;
								case HILLCLIMBFINISH:
									// set up to find the start
									seeklat = lap.startpos.lat;
									seeklon = lap.startpos.lon;
									seekhdg = lap.startheading;
									tm_state = HILLCLIMBSTART;
									
									// put up final adjusted number
									Write_Timer();
									break;
								case AUTOXFINISH:
									tm_state = AUTOXHOLDTIME;

									// put up final adjusted number
									Write_Timer();
									break;
								case TIMING:
									break;
							} // switch	
						} // if !inrange
					} // if
					else // no longer in range
						inrange = FALSE;
				} // if
			} // if
			break;
		} // INITIALIZE, NEW_POSITION
		case START_POSITION: {
			xdata flashpagetype tempflash;	// structure to access one page of flash

			// we are in range
			inrange = TRUE;

			lap.currentlap = 1;
			lap.timer = lap.timernminus1 = 0;
			lap.besttime = 0xFFFFFFFF;
			lap.lastlap = 0;
			lap.distance = 0.0;
			lastdist = 0L;

			// initialize lap storage
			for (cursectornum = 0; cursectornum < NUMLAPS; cursectornum++)
				laplist[cursectornum] = 0xFFFFFFFF;

			// initialize sectors
			for (cursectornum = 0; cursectornum < NUMSECTORS; cursectornum++)
				sector[cursectornum].reference = 
				sector[cursectornum].current = 0;
			cursectornum = 0;
			refsectornum = 0;

			// save starting position for this track
			// get the track info
			DataFlash_Page_Read(MAX_PAGE_SIZE, tempflash.bigbuff, TRACKPAGE);

			// initialize and save the information
			lap.startpos.lat = tempflash.circuit.track[lap.tracknum].startlat = lap.currentpos.lat;
			lap.startpos.lon = tempflash.circuit.track[lap.tracknum].startlon = lap.currentpos.lon;
			lap.startheading = tempflash.circuit.track[lap.tracknum].starthead = lap.currentheading;
//			tempflash.circuit.track[lap.tracknum].startalt = lap.startpos.alt;

			// write it out
			DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, tempflash.bigbuff, TRACKPAGE);

			Clear_Screen();
			Write_Icon(21, 4, 81, 14, (u08 *) timingicon, '-');

			// do we need a finish line?
			if (AUTOXREADY == tm_state || HILLCLIMB == tm_state) {			
				// has finish been set?
				if (0L == lap.finishpos.lat) {									
					// no finish line so get ready to get finish line			
					Write_Text(3, 1, "Hit SELECT at Finish", '-');
					button_overlay[SEL] = markfinish;
					if (HILLCLIMB == tm_state)
						tm_state = HILLCLIMBGETFINISH;
					else
						tm_state = AUTOCROSS;
				} // if
				else {	// got a finish line - can't happen in hillclimb mode
					tm_state = AUTOXFINISH;
					Write_Text(3, 4, "* Finish Line Set *", '-');
					button_overlay[SEL] = NULL;
					Write_Timer();
					
					// get ready for the big finish
					seeklat = lap.finishpos.lat;
					seeklon = lap.finishpos.lon;
					seekhdg = lap.finishheading;
				} // else
			} // if
			else {	// not AUTOCROSS or HILLCLIMB
				Write_Screen((u08 *) predictive, '-');
				Write_Icon(111, 23, 8, 8, (u08 *) plussign, '-');
				Write_Icon(3, 23, 8, 8, (u08 *) minussign, '-');
				button_overlay[SEL] = NULL;
				tm_state = TIMING;
				
				// find this starting line again next lap
				seeklat = lap.startpos.lat;
				seeklon = lap.startpos.lon;
				seekhdg = lap.startheading;
			} // else
			
			// write lap number on screen
			if (AUTOCROSS != tm_state && AUTOXFINISH != tm_state)
				Write_Lap();
			break;
		} // case
		case FINISH_POSITION: {
			xdata flashpagetype tempflash;	// structure to access one page of flash

			// finishandgear / autox run over. hold the time on the display
			if (HILLCLIMBGETFINISH == tm_state) {
				seeklat = lap.startpos.lat;
				seeklon = lap.startpos.lon;
				seekhdg = lap.startheading;
				tm_state = HILLCLIMBSTART;
				Write_Screen((u08 *) predictive, '-');
//				Write_Icon(111, 23, 8, 8, (u08 *) plussign, '-');
//				Write_Icon(3, 23, 8, 8, (u08 *) minussign, '-');
				Write_Timer();

				// save as best lap
				lap.besttime = lap.timer;
	
				// put checker up
				Write_Flag(TRUE);
	
				// make current lap the reference lap
				for (cursectornum = 0; cursectornum < NUMSECTORS; cursectornum++)
					sector[cursectornum].reference = sector[cursectornum].current;

			} // if
			else {
				tm_state = AUTOXHOLDTIME;
				// note that we have set the line
				Clear_Line(3);
				Write_Text(3, 4, "* Finish Line Set *", '-');
			} // else
				
			// remove sel button overlay
			button_overlay[SEL] = NULL;
		
			// save the lap
			laplist[0] = (u32) lap.timer;
	
			// get the track finish info
			DataFlash_Page_Read(MAX_PAGE_SIZE, tempflash.bigbuff, TRACKFINPAGE);		
		
			// save the finish information and set up to find the start
			lap.finishpos.lat = tempflash.finishandgear.trackfinish[lap.tracknum].finishlat = lap.currentpos.lat;
			lap.finishpos.lon = tempflash.finishandgear.trackfinish[lap.tracknum].finishlon = lap.currentpos.lon;
//			tempflash.finishandgear.trackfinish[lap.tracknum].finishalt = lap.currentpos.alt;
			lap.finishheading = tempflash.finishandgear.trackfinish[lap.tracknum].finishhead = lap.currentheading;

			// write to flash
			DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, tempflash.bigbuff, TRACKFINPAGE);
		} // case
	} // switch
} // Lap_Check

/******************************************************************************
** Lap_Store **************************************** GAS 28 JAN 05 ****
*******************************************************************************
This function stores the laps collected into flash after a session.
******************************************************************************/
void Lap_Store( void ) {

	// update the index
	DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, (u08 *) laplist, (LASTLAPPAGE - NUMLAPPAGES + session));

} // Lap_Store

/******************************************************************************
** Sector_Store **************************************** GAS 13 APR 07 ****
*******************************************************************************
This function stores the lap sectors collected into flash after a session.
******************************************************************************/
void Sector_Store( void ) {

	// get the predictive lap timing sectors for this track
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, PREDPAGESTART + (lap.tracknum / 4));
		
	// if lap better than that stored, replace it
	if ((u32) lap.besttime < (u32) flashpage.storedsectors[lap.tracknum % 4].bestlapever) {
		int i;
		
		// copy lap time
		flashpage.storedsectors[lap.tracknum % 4].bestlapever = lap.besttime;

		// copy sector times
		for (i = 0; i < NUMSECTORS; i++)
			flashpage.storedsectors[lap.tracknum % 4].bestsectortimes[i] = sector[i].reference;

		// write out new best lap info
		DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, PREDPAGESTART + (lap.tracknum / 4));
	} // if
} // Sector_Store

void unpack_velocities( u08 *velptr, s16 *eastvel, s16 *northvel, s16 *vertvel) {
	
	*eastvel = (s16) ((((u16) (velptr[0] & 0xE0)) << 3) + (u16) velptr[1]);
	// sign extend
	if (*eastvel & 0x0400) *eastvel |= 0xF800;

	*northvel = (s16) ((((u16) (velptr[0] & 0x1C)) << 6) + (u16) velptr[2]);
	// sign extend
	if (*northvel & 0x0400) *northvel |= 0xF800;

	*vertvel = (s16) ((((u16) (velptr[0] & 0x03)) << 8) + (u16) velptr[3]);
	// sign extend
	if (*vertvel & 0x0200) *vertvel |= 0xFC00;
} // unpack_velocities

u16 compute_heading (s16 eastvel, s16 northvel) {
	xdata float tempfloat;
	xdata s16 tempint;

	// compute heading = vector sum of east/west vel and inverse of north/south vel (due east is 0)
	tempfloat = atan2((float) northvel, (float) eastvel);

	tempfloat = (tempfloat * (180.0 / PI));
	tempint = (s16) tempfloat;

	// make sure it is 0-360
	tempint = (tempint + 360) % 360;

	return ((u16) tempint);

} // compute_heading

#ifdef AUTOX
/******************************************************************************
** AutoCross/Rally Functions******************************** GAS 15 May 07 ****
*******************************************************************************/

// this function times the run and looks for the finish line if previously set
BOOL AutoX_Timing(lapop oper, gpstype *gps) {

	switch (oper) {
		case INC_TIME:
			// increment by msec
			lap.timer += 1000 / (samps_per_sec);

			// now that we are moving, save the heading one second after launch
			if (AUTOXSETHDGTIME == lap.timer)
				lap.startheading = lap.currentheading;
			break;

		case NEW_POSITION: {
			xdata s32 deltalat, deltalon;
			xdata s16 eastvel, northvel, vertvel;
			xdata float tempfloat;

			lap.previouspos = lap.currentpos;
			lap.previousheading = lap.currentheading;
			lap.currentpos = *gps;

			unpack_velocities (gps->velpack, &eastvel, &northvel, &vertvel);

			// skip all this if we are not moving to eliminate divide by zero
			if (eastvel != 0 || northvel != 0) {
 				// get heading
 				lap.currentheading = compute_heading(eastvel, northvel);
			
				if (AUTOXFINISH == tm_state) {		// finish line present so look for it 
					// if current heading matches the finish heading then check for position
					// while allowing for headings near 0 (due east)
					if (ABS((s16) lap.currentheading - (s16) lap.finishheading) < AUTOX_HEADRANGE ||
						ABS((s16) lap.currentheading - (s16) lap.finishheading) > (360 - AUTOX_HEADRANGE)) {
			
						tempfloat = TODEGREE(lap.finishpos.lat);
			
						// find difference in meters of lat, lon
						deltalat = (s32) ((TODEGREE(lap.currentpos.lat) - tempfloat) * LATMET);
						deltalon = (s32) ((TODEGREE(lap.currentpos.lon) - TODEGREE(lap.finishpos.lon)) * LONMET(tempfloat));
			
						// check if we are close to finish point
						if (ABS(deltalat) < AUTOX_DISTRANGE && ABS(deltalon) < AUTOX_DISTRANGE) {
							xdata float distance;
							xdata float speed;
							xdata s32 offset;
			
							// find the actual distance to finish in met/sec
							distance =
							sqrt((float) deltalat * (float) deltalat + (float) deltalon * (float) deltalon);
			
							// calculate speed in meters/msec
							speed =
							sqrt((float) eastvel * (float) eastvel + (float) northvel * (float) northvel);
			
							// fix up the timer based on our speed, rounding to the hundredth
							offset = (s32) (50.0 + (1000.0 * distance) / speed);
			
							// fudge to where time would be when we cross finish
							lap.timer += offset;
	
							// save the lap
							laplist[0] = (u32) lap.timer;
	
							return (TRUE);
						} // if
					} // if
				} // if
			} // if
			break;
		} // case
	} // switch
	return (FALSE);
} // AutoX_Timing
#endif
