#include <stdlib.h>
#include <ctype.h>
#include <display.h>

//------------------------------------------------------------------------------------
// Menu.c
//------------------------------------------------------------------------------------
// Copyright 2004 - 2010 Track Systems Technologies, LLC
//
// AUTH: GAS
// DATE: 28 AUG 04
// 		 10 JAN 05 - load menus from flash
//		 25 JAN 05 - revised menu system, added erase session & session detail
//		 30 MAR 05 - changed menu system to scrolling cursor
//
// This file contains the menu control functions
//

// function prototype
void Draw_Menu(void);

// data structures used exclusively by menu system
static xdata menutype *menu;
static xdata menutype getback;

//
// Session_Init
//
// This function reads the Dataflash to create a list of sessions
//
void Session_Init() {
	xdata flashpagetype flashpage;		// where to get the data
	xdata u08 i,j;

	// get the index information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, INDEXPAGE);

	j = 0;
	for (i = 0; i<NUMSESSIONS; i++) {
		// look for a starting segment number
		if (0xFFFF != flashpage.index.session[i].startseg &&
			0xFFFF != flashpage.index.session[i].endseg) {

			// print session number
			sprintf(menunames.session[j], "%02d ", (int) (i+1));

			// add the date and time
			formattime(flashpage.index.session[i].starttime,
						flashpage.index.session[i].startweeks,
						SHORTDANDT, &(menunames.session[j][3]));

			// create the menu entry
			menulists.sessionlist[j].actionid = FUNCSUB;
			menulists.sessionlist[j].action.f = &sesstitle;
			menulists.sessionlist[j].itemname = menunames.session[j];

			j++;
		} // if
	} // for

	sessions.num_items = j;

	// point to last session entry
	if (sessions.num_items <= 3)
		sessions.top_line = 0;
	else
		sessions.top_line = sessions.num_items - 3;

	if (sessions.num_items == 0)
		sessions.current_sel = sessions.current_hl = 0;
	else
		sessions.current_sel = sessions.current_hl = j-1;

	// put in end marker
	menulists.sessionlist[j].actionid = LASTITEM;
	menulists.sessionlist[j].action.submenu = NULL;
	menulists.sessionlist[j].itemname = NULL;
} // Session_Init

//------------------------------------------------------------------------------------
// Menu_Op
//------------------------------------------------------------------------------------
//
// Bring up the specified menu
//
void Menu_Op (menuop operation, displaybutton button) reentrant {
	u08 temp;
	menutype *menuptr;
	static BOOL firsttime = TRUE;

	if (firsttime) {
		menu = &mainmenu;
		getback.menuname = NULL;
		getback.prev_menu = NULL;
		getback.num_items =
		getback.current_sel =
		getback.current_hl = 0;
		getback.menulist = NULL;
		firsttime = FALSE;
	} // if

	switch (operation) {
		case INIT: {
			// initialize if necessary
			if (0 == menu->num_items)
				for (menu->num_items=0;
					LASTITEM != menu->menulist[menu->num_items].actionid;
					menu->num_items++);

			// put display in direct line access mode
			Reset_Top_Line();

			if (0xFF == menu->top_line) { // graphic menu
				Write_Screen((u08 *) menu->menulist[menu->current_hl].itemname, '-');
			} // if
			else { // text menu
				Clear_Screen();

				// put up a line bar
				Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), '-');

				temp = 6*(20-strlen(menu->menuname))/2;
				// write menu centered name
				Write_Text(0, temp, menu->menuname, '-');

				// skip the b.s. if it is a viewlist
				if (VIEWLIST == menu->menulist[0].actionid)
					menu->current_sel = menu->current_hl = menu->top_line = 0;
				else {
					// start off with currently selected item on the screen
					if (PICKLIST == menu->menulist[0].actionid &&
						(menu->current_sel < menu->top_line ||
						menu->current_sel > menu->top_line+2)) {

						menu->current_hl = menu->current_sel;
						if (menu->num_items <= 3 || menu->current_hl == 0)
							menu->top_line = 0;
						else
							menu->top_line = MIN(menu->current_hl-1, menu->num_items-3);
					} // if
				} // if

				Draw_Menu();
			} // else
			break;
		} // INIT
		case BUTTONPRESS: {
			switch (button) {
				case UP:
					// note: up and dn button overrides works differently
					// from sel/back because they work within a single menu
					// check for button override
					if (NULL != button_overlay[UP]) {
						(*button_overlay[UP])(menu);
						break;
					} // if

					if (0xFF == menu->top_line) { // graphic menu
						if (menu->current_hl > 0) {
							menu->current_hl--;
							Write_Screen((u08 *) menu->menulist[menu->current_hl].itemname, '-');
						} // if
					} // if
					else { // text menu
						if (menu->num_items != 0) { // dont do anything to empty lists
							// if highlight is not top item, move it up a line
							if (menu->top_line < menu->current_hl)
								menu->current_hl--;
							else { // highlight on top line
								if (menu->top_line > 0) {	// check for first item
									menu->top_line--; 	// move highlight and top line up one
									menu->current_hl--;
								} // if
							} // else

							Draw_Menu();
						} // if
					} // else
					break;
				case DN:
					// note: up and dn button overrides works differently
					// from sel/back because they work within a single menu
					// check for button override
					if (NULL != button_overlay[DN]) {
						(*button_overlay[DN])(menu);
						break;
					} // if

					if (0xFF == menu->top_line) { // graphic menu
						if (menu->current_hl <= menu->num_items-1) {
							menu->current_hl++;
							Write_Screen((u08 *) menu->menulist[menu->current_hl].itemname, '-');
						} // if
					} // if
					else { // text menu
						if (menu->num_items != 0) { // dont do anything to empty lists
							// if highlight is not bottom item, move it down a line
							if (menu->current_hl - menu->top_line < 2)
								menu->current_hl = MIN(menu->current_hl+1, menu->num_items-1);
							else { // highlight on bottom line
								if (menu->top_line+3 < menu->num_items) {	// check for last item
									menu->top_line++; 	// move highlight and top line down one
									menu->current_hl++;
								} // if
							} // else

							Draw_Menu();
						} // if
					} // else
					break;
				case BACK:
					// note: back and sel button overrides works differently
					// from up/down because they cause menu to menu transitions
					// check for button override
					if (NULL != button_overlay[BACK]) {
						menuptr = (*button_overlay[BACK])(menu);

						// now execute menu function if desired
						if (NULL != menuptr) {
							menu = menuptr;
							Menu_Op(INIT, BACK);
						} // if
					} // if
					else { // no overlay so go back a menu
						menu = menu->prev_menu;
						tm_state = WAITING;
						Menu_Op(INIT, BACK);
					} // else
					break;
				case SEL:
					// note: back and sel button overrides works differently
					// from up/down because they cause menu to menu transitions
					// check for button override
					if (NULL != button_overlay[SEL]) {
						menuptr = (*button_overlay[SEL])(menu);

						// now execute menu function if desired
						if (NULL != menuptr) {
							menu = menuptr;
							Menu_Op(INIT, SEL);
						} // if
					} // if
					else {	//  no overlay so do what the menu tells you
						if (NULL != menu->menulist) {
							switch (menu->menulist[menu->current_hl].actionid) {
								case SUBMENU:	// transfer to another list of choices
									menu = menu->menulist[menu->current_hl].action.submenu;
									Menu_Op(INIT, SEL);
									break;
								case PICKLIST:	// pick an item from the list
									menu->current_sel = menu->current_hl;
									tm_state = WAITING;
									// execute if there is an action associated with this menu pick
									if (NULL != menu->menulist[menu->current_hl].action.f)
										menuptr = (menutype *)(menu->menulist[menu->current_hl].action.f)(menu);
									Menu_Op(INIT, SEL);
									break;
								case FUNCNORET:	// turn over all control to function
									// save a copy of menu to pass to function
									menuptr = menu;

									// create dummy menu for return from function
									getback.prev_menu = menu;
									menu = &getback;

									// execute the function
									if (NULL != menuptr->menulist[menuptr->current_hl].action.f)
										(menuptr->menulist[menuptr->current_hl].action.f)(menuptr);
									break;
								case FUNCRETURN: // execute function then go to prev menu
									// return up a level
									menu = menu->prev_menu;

									// execute the function
									if (NULL != menu->menulist[menu->current_hl].action.f)
										(menu->menulist[menu->current_hl].action.f)(NULL);

									tm_state = WAITING;
									Menu_Op(INIT, SEL);

									break;
								case FUNCSUB:	// execute initialization function then go into submenu
									// execute the function
									if (NULL != menu->menulist[menu->current_hl].action.f)
										menuptr = (menutype *)(menu->menulist[menu->current_hl].action.f)(menu);

									if (NULL != menuptr)
										menu = menuptr;

									Menu_Op(INIT, SEL);
									break;
								case CONFIRM:
									// fill in confirm menu for return from function
									confirm.prev_menu = menu;

									// assign the actions
									confirm.menulist[0].action.f =
										menu->menulist[menu->current_hl].action.f;
									confirm.menulist[1].action.submenu = menu;

									menu = &confirm;

									Menu_Op(INIT, SEL);
									break;
								case VIEWLIST:	// just scroll menu items to be viewed
									// keep current sel even with hl
//									menu->current_sel = menu->current_hl;
									break;
								default:	// do nothing
									break;
							} // switch
						} // if
					} // if
					break;
				case ONOFF: {
					u08 status;

					// save power
					BACKLITEKB = 0;
					BACKLITEDS = 0;

					// tell user
					Write_Screen((u08 *) end, '-');

					// save data
					if (RECORDING(tm_state))
						End_Session(session, pagecnt, iopagecnt);
					if (TIMING == tm_state ||
						AUTOXHOLDTIME == tm_state ||
						HILLCLIMBSTART == tm_state ||
						HILLCLIMBFINISH == tm_state ||
						DRAGHOLDTIME == tm_state ||
						DRAGHOLDNUMS == tm_state) {
							Lap_Store();
							Sector_Store();
					} // if

					// wait for Serial Flash to be finished
					do {
						status = DataFlash_Read_Status();
					} while (!(status & STATUS_READY_MASK));

					// wait for button release so we don't power right back up
//					while (!ONOFF_SW) ;

					// go away
					while (1) {
						u16 retry;

						// tell SU to shut us down
						SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) POWER_DOWN, (u08) 0);

						// wait a bit. if still alive, send it again
						for (retry = 0; retry < 40000; retry++) ;
					} // while
					break;
				} // case ONOFF
				case LITE:
					// toggle backlights
//					BACKLITEKB = ~BACKLITEKB;
					BACKLITEDS = ~BACKLITEDS;
					break;
			} // switch button
		} // case BUTTONPRESS
	} // switch operation
} // Menu_Op

void Draw_Menu(void) {
	xdata char attrib;
	xdata u08 *icon;
	xdata u08 temp;

	// handle legitimately empty list
	if (0 == menu->num_items) {
		Write_Icon(0, 16, NUMBER_OF_COLUMNS, 8, &(reversebar[0]), '-');
		Write_Text(2, (u08) (NUMBER_OF_COLUMNS - 6*strlen("LIST EMPTY")),
					"LIST EMPTY", '^');
	}
	else {
		xdata u08 i;
		for (i = 1; i <= MIN(3, menu->num_items); i++) {
			// check for highlighting
			if (VIEWLIST != menu->menulist[menu->top_line+i-1].actionid &&
				menu->current_hl == (menu->top_line + i - 1))
				// white on black
				attrib = '~';
			else
				// black on white
				attrib = '-';

			// default line is just clear
			icon = (u08 *) clearbar;

			// write the appropriate icon
			// no icons if only one choice
			if (menu->num_items == 1)
				icon = (u08 *) cleartopbot;
			else { // more than one
				switch (i) {
					case 1: // working with first menu line
						if (menu->top_line == 0)
							// first item so show top bar
							icon = (u08 *) cleartop;
						else
							// more items to scroll so show arrow
							icon = (u08 *) clearup;
						break;
					case 2: // check for end of menu list
						if (menu->num_items != 2) break;
						// if only 2 menu items, treat it like the 3rd line so fall through
					case 3:	// working with last menu line
						if (menu->top_line + i == menu->num_items)
							// last item so show bottom bar
							icon = (u08 *) clearbot;
						else
							// more items to scroll so show arrow
							icon = (u08 *) cleardn;
						break;
				} // switch
			} // else

			// write the background
			Write_Icon(0, 8*i, NUMBER_OF_COLUMNS, 8, icon, attrib);

			// figure out starting column
			temp = NUMBER_OF_COLUMNS - 6*strlen(menu->menulist[menu->top_line+i-1].itemname);

			// if it's a PICKLIST, mark if currently selected item
			if (menu->top_line+i-1 == menu->current_sel && PICKLIST == menu->menulist[menu->top_line+i-1].actionid)
				Write_Text(i, (u08) (temp-12), "*", '^');

			// write the text
			Write_Text(i, temp,	menu->menulist[menu->top_line+i-1].itemname, '^');

		} // for
	} // if
} // Draw_Menu

// Menu Executable Functions

menutype *collectdata(menutype *menu) {
	xdata u08 storedata = FALSE;
	xdata u08 samprate;				// used for sampling rate and general temporary variable

	if (!su_present)
		Info_Screen("No SU");
	else {
		xdata u16 i;
		u08 index = menu->current_hl;

		// Set up input scaling
		DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);
		for (samprate=0; samprate<NUMANALOGS; samprate++)
			analogscaling[samprate] = flashpage.io.analogs[samprate];
		for (samprate=0; samprate<NUMTDDIGITALS; samprate++)
			digitalscaling[samprate] = flashpage.io.digitals[samprate+4];

		// put in the write address
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = 0;
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = IODEFPAGE & 0xFF;
	
		// copy segment into output buffer
		for (i=0; i < MAX_PAGE_SIZE; i++)
			com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2+i] = flashpage.bigbuff[i];
	
		// send io defs to SU for use in digital outputs
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) WRITE_SEGMENT, MSGDATA);

		// wait to get ACK from SU
		while (0 != CTS[SENSOR_UNIT-1]);

		// get the driver and car info
		DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

		// assign the current selections	
		lap.tracknum = flashpage.user.selectedtrack;
		samprate = (u08) flashpage.user.modesample & 0x0F;
		currentcar.cylandweight = flashpage.user.car[flashpage.user.selectedcar].carspecs.cylandweight;
		currentcar.revwarning = flashpage.user.car[flashpage.user.selectedcar].carspecs.revwarning;
		currentcar.revlimit = flashpage.user.car[flashpage.user.selectedcar].carspecs.revlimit;

		// set up the I/O collection
		iocollect = flashpage.user.iodata;

		// use to synch up io, accelerometer data, and GPS data
		firstpulse = TRUE;

		// make this one function work for collect data and other modes
		if (menu == &other)
			index += 20;

		switch (index) {

		// Record Data Modes
		case 0:	// laps
		case 1:	// qualifying
		case 9: { // hillclimb (segment timing)
			if (1 == index) timingmode = QUAL;
			else timingmode = LAP;

			Write_Screen((u08 *) findgps, '-');

			// initialize lap stuff
			lap.currentheading =
			lap.previousheading = 0;
			lap.previouspos.time =
			lap.currentpos.time = 0;
			lap.previouspos.lat =
			lap.currentpos.lat = 0;
			lap.previouspos.lon =
			lap.currentpos.lon = 0;
			lap.previouspos.alt =
			lap.currentpos.alt = 0;
			lap.currentlap = 0;
			*((u32 *) lap.currentpos.velpack) =
			*((u32 *) lap.previouspos.velpack) = 0L;
			lap.previouspos.temp =
			lap.currentpos.temp = 0;
			lap.distance = 0.0;
			lap.timernminus1 = 0;

			// figure out current graph scale settings
			switch ((scratch.calibrate.usersettings & 0xE0) >> 5) {
				case 2:
					graphratio = 0.5;
					break;
				case 4:
					graphratio = 2.0;
					break;
				case 5:
					graphratio = 5.0;
					break;
				case 6:
					graphratio = 10.0;
					break;
				case 3:
				default:
					graphratio = 1.0;					
			} // switch	
			
			// figure out current tracklength
			switch ((scratch.calibrate.usersettings & 0x0E) >> 1) {
				case 0:
					tracksampledist = SHORTTRACKDIST;
					break;
				case 2:
					tracksampledist = LONGTRACKDIST;
					break;
				case 1:
				default:
					tracksampledist = MEDTRACKDIST;					
			} // switch
										
			// initialize lap storage
			for (i = 0; i < NUMLAPS; i++)
				laplist[i] = 0xFFFFFFFF;

			// get the track info
			DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKPAGE);

			// retrieve the information
			lap.startpos.lat = flashpage.circuit.track[lap.tracknum].startlat;
			lap.startpos.lon = flashpage.circuit.track[lap.tracknum].startlon;
//			lap.startpos.alt = flashpage.circuit.track[lap.tracknum].startalt;
			lap.startheading = flashpage.circuit.track[lap.tracknum].starthead;
	
			// use menu entry storage to remember current track name
			strncpy(menunames.track[lap.tracknum], flashpage.circuit.track[lap.tracknum].name, NAMELEN);

			// put in null terminator just in case
			menunames.track[lap.tracknum][NAMELEN-1] = '\0';
		
			if (9 == index) {	// get the track finish line info for hillclimbs
				DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKFINPAGE);
	
				lap.finishpos.lat = flashpage.finishandgear.trackfinish[lap.tracknum].finishlat;
				lap.finishpos.lon = flashpage.finishandgear.trackfinish[lap.tracknum].finishlon;
//				lap.finishpos.alt = flashpage.finishandgear.trackfinish[lap.tracknum].finishalt;
				lap.finishheading = flashpage.finishandgear.trackfinish[lap.tracknum].finishhead;
			} // if

			// get sector information for this track
			DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, PREDPAGESTART + (lap.tracknum / 4));

			// if we don't have a starting position, get one	
			if (lap.startpos.lat == 0xFFFFFFFF || lap.startpos.lat == 0) {
				button_overlay[SEL] = markstart;
				if (9 == index)
					tm_state = HILLCLIMB;
				else
					tm_state = LAPS;
				
				// clear best sectors and initialize
				for (i = 0; i < NUMSECTORS; i++)
					sector[i].reference = sector[i].current =
					flashpage.storedsectors[lap.tracknum % 4].bestsectortimes[i] = 0L;
				
				// reset best laptime and initialize
				lap.besttime = flashpage.storedsectors[lap.tracknum % 4].bestlapever = 0xFFFFFFFF;

				// write out new best lap info
				DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, PREDPAGESTART + (lap.tracknum / 4));
			} // if
			else {
				// valid track with a starting position so get stored predictive lap timing sectors
				// and reference laptime for this track
				if (QUAL == timingmode) {
					for (i = 0; i < NUMSECTORS; i++)
						sector[i].current = sector[i].reference = 0L;
					lap.besttime = 0xFFFFFFFF;
				} // if
				else { // QUAL != timingmode)
					// get the best sectors
					for (i = 0; i < NUMSECTORS; i++) {
						sector[i].current = 0L;
						sector[i].reference = flashpage.storedsectors[lap.tracknum % 4].bestsectortimes[i];
					} // for
					
					// get the best laptime	
					lap.besttime = flashpage.storedsectors[lap.tracknum % 4].bestlapever;
				} // else

				button_overlay[SEL] = clearmenu;
				if (9 == index)
					tm_state = HILLCLIMBWAITSTART;
				else
					tm_state = WAITSTART;
			} // else

			storedata = TRUE;
			} // else
			break;
		case 2:	// Autocross (staged start)
		case 3: // 1/8 mile (staged start)
		case 4: // Drag (staged start)
			Write_Screen((u08 *) findgps, '-');

			// initialize lap stuff
			lap.currentheading =
			lap.previousheading = 0;
			lap.previouspos.time =
			lap.currentpos.time = 0;
			lap.previouspos.lat =
			lap.currentpos.lat = 0;
			lap.previouspos.lon =
			lap.currentpos.lon = 0;
			lap.previouspos.alt =
			lap.currentpos.alt = 0;
			*((u32 *) lap.currentpos.velpack) =
			*((u32 *) lap.previouspos.velpack) = 0L;
			lap.previouspos.temp =
			lap.currentpos.temp = 0;

			// initialize lap storage
			for (i = 0; i < NUMLAPS; i++)
				laplist[i] = 0xFFFFFFFF;

			// get the track info
			DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKFINPAGE);

			// retrieve the information
			lap.finishpos.lat = flashpage.finishandgear.trackfinish[lap.tracknum].finishlat;
			lap.finishpos.lon = flashpage.finishandgear.trackfinish[lap.tracknum].finishlon;
//			lap.finishpos.alt = flashpage.finishandgear.trackfinish[lap.tracknum].finishalt;
			lap.finishheading = flashpage.finishandgear.trackfinish[lap.tracknum].finishhead;

			// prepare to set a starting line position
			button_overlay[SEL] = stage;

			if (2 == index) {
				tm_state = AUTOXSTAGE;
				timingmode = QUAL;		// moves timing icon up on screen
			} // if
			else {
				tm_state = DRAGSTAGE;

				dragnumbers.quarter = (4 == index);		// TRUE if 1/4, FALSE if 1/8 mile
				dragnumbers.sixtyfoot =
				dragnumbers.zerosixty =
				dragnumbers.zerohundred =
				dragnumbers.threethirty =
				dragnumbers.eighthmile =
				dragnumbers.quartermile =
				dragnumbers.eighthtrap = 
				dragnumbers.quartertrap = 
				dragnumbers.reaction =
				dragnumbers.launchg =
				dragnumbers.horsepower =
				dragnumbers.torque = 0;
				
				timingmode = REV;		// timing icon in center of screen
			} // else

			// override programmed values for track length and sampling rate
			tracksampledist = SHORTTRACKDIST;	// all short distances
			samps_per_sec = 40;					// 40Hz
			samprate = 4;						// 40Hz			

			storedata = TRUE;
			break;
		case 5:	// Drive
			Clear_Screen();

			tm_state = DRIVE;
			storedata = TRUE;
			button_overlay[UP] = previnput;
			button_overlay[DN] = nextinput;
			
			// remember start if good gps signal
			if (gpslock >= GPS_GOOD) {
				// record starting position
				Lap_Check(START_POSITION, NULL);
			} // if
			break;
		case 6: // Tach & Speedo
			tm_state = GPSRPM;
			storedata = TRUE;
			button_overlay[UP] = previnput;
			button_overlay[DN] = nextinput;
			
			// remember start if good gps signal
			if (gpslock >= GPS_GOOD) {
				// record starting position
				Lap_Check(START_POSITION, NULL);
			} // if
			break;
		case 7: { // Gauges
			xdata BOOL found;
			xdata u08 checkbits;

			// find the first selected input point
			for (checkbits = 2; checkbits < 7; checkbits++)
				// check for bit set enabling i/o point
				if (found = ((iocollect >> checkbits) & 0x01)) break;
			if (found) {
				if (checkbits == 6) // RPM
					tm_state = GPSRPM;
				else if (checkbits == 5) // digitals
					tm_state = GAUGED4;
				else // analogs
					tm_state = GAUGEA0 + (checkbits-2);

				storedata = TRUE;
				button_overlay[UP] = previnput;
				button_overlay[DN] = nextinput;
			} // if
			else { // none enabled
				Info_Screen("No Inputs");
				tm_state = WAIT4BACK;
				storedata = FALSE;
			} // else
			break;
		} // case
		case 8: // G Forces
			tm_state = GFORCES;
			storedata = TRUE;
			button_overlay[UP] = previnput;
			button_overlay[DN] = nextinput;

			Clear_Screen();
			// put up a line bar
			Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), '-');
	
			// write menu centered name
			Write_Text(0, 36, "G-Forces", '-');
			break;
		case 10: // Gs and Location
			Clear_Screen();
			tm_state = GPSREC;
			storedata = TRUE;

			// remember start if good gps signal
			if (gpslock >= GPS_GOOD) {
				// record starting position
				Lap_Check(START_POSITION, NULL);
			} // if
			break;
		case 11: // Inputs
			Clear_Screen();
			// put up a line bar
			Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), '-');

			// write menu centered name
			Write_Text(0, 42, "Inputs", '-');
			tm_state = INPUTS;
			storedata = TRUE;
			break;
		// Other Modes
		case 20: // GPS Compass
			Clear_Screen();
			tm_state = COMPASS;
			samprate = 2;			// 20Hz
			storedata = FALSE;
			break;
		case 21: // Instruments
			Clear_Screen();
			tm_state = INSTRUMENT;
			samprate = 2;			// 20Hz
			storedata = FALSE;
			break;
		case 22: // GPS Location
			Clear_Screen();
			tm_state = GPSINFO;
			samprate = 2;			// 20Hz
			storedata = FALSE;
			break;
		case 23: // Check Inputs
			Clear_Screen();
			// put up a line bar
			Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), '-');

			// write menu centered name
			Write_Text(0, 42, "Inputs", '-');

			tm_state = CHKINPUTS;
			samprate = 2;			// 20Hz
			storedata = FALSE;
			break;

		} // switch
	} // else

	if (storedata) {
		// initialize
		gcnt = secondcnt = 0;

		firstpulse = TRUE;
		iobyte = MAX_PAGE_SIZE - 1;		// point to MSB
		session = Start_Session(&pagecnt, &iopagecnt);
		if (0 == session) { // no sessions available
			Info_Screen("Mem Full");
			tm_state = WAITING;
		} // if
	}
	// tell SU to start sending data
	if (tm_state != WAITING) {
		u08 i;
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = tm_state;
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = samprate;
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2] = iocollect;
		// set up input scaling, send over the number of bits to shift, 20 = 4, 10 = 3, 5 = 2
		for (i=0; i<NUMANALOGS; i++) {
			switch (analogscaling[i].maxscale) {
				case 10: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 3;
					break;
				case 5: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 2;
					break;
				case 20:
				default: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 4;
					break;
			} // switch
		} // for
		// send frequency alarm data for currently selected car
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+7] =
			flashpage.user.car[flashpage.user.selectedcar].carspecs.cylandweight & 0x0F;
		* (u16 *) (com[msgrouting[SENSOR_UNIT]].txbuff+DATA_START+8) =
			flashpage.user.car[flashpage.user.selectedcar].carspecs.revwarning;

		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) MODE_CHANGE, (6+NUMANALOGS));
	} // if

	// overlay back button to turn off data collection
	button_overlay[BACK] = idle;

	return(NULL);
} // collectdata

//
// Menu Functions
//
// These functions are automatically called during menu transitions based
// on the static and dynamic menu structures.
//

// menutype *calibrate(menutype *menu) { return(menu); }

menutype *eraseall(menutype *menu) {
	tm_state = WAITING;

	// wipe out the indexes
	DataFlash_Erase_Page(INDEXPAGE);

	// wipe out all data
	DataFlash_Erase_Range(FIRSTDATAPAGE, LASTLAPPAGE, TRUE);

	// just in case
	Init_Tables();

	Clear_Screen();

	return(&setup);
}

menutype *wipemem(menutype *menu) {
	tm_state = WAITING;

	// wipe out all data
	DataFlash_Erase_Range(INDEXPAGE, numpages-1, TRUE);

	// just in case
	Init_Tables();

	Clear_Screen();

	return(&setup);
}

#ifdef PACKMEM
menutype *packmem(menutype *menu) {
	tm_state = WAITING;

	Pack_Sessions(TRUE);

	Clear_Screen();

	return(&setup);
}
#endif

//
// This function saves the current user selections to flash
//
menutype *savepicklists(menutype *menu) {
	xdata flashpagetype flashpage;		// where to get the data

	// get the driver and car info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	// assign the current selections	
	flashpage.user.selecteddriver = drivers.current_sel;
	flashpage.user.selectedtrack = tracks.current_sel;
	flashpage.user.selectedcar = vehicles.current_sel;
	// clear out current mode
	flashpage.user.modesample &= 0xF0;
	// assign new sampling rate
	switch (samps_per_sec) {
		case 20: // 20Hz
			flashpage.user.modesample += 2;
			break;
		case 40: // 40Hz
			flashpage.user.modesample += 4;
			break;
		case 10: // 10Hz
		default:	// do nothing
			break;
	} // switch

	// update the index
	DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	return(menu);
}

// this function saves the sampling rate
menutype *savesamp(menutype *menu) {

	// determine which selection was chosen
	switch (menu->current_hl) {
		case 1: // 20Hz
			samps_per_sec = 20;
			break;
		case 2: // 40Hz
			samps_per_sec = 40;
			break;
		case 0: // 10Hz
		default:
			samps_per_sec = 10;
	} // switch

	savepicklists(menu);

	return (menu);
} // savesamp

// This function reads the configuration data written by Traqview and creates
// the driver menu selections from it.
//
menutype *loaddrivers(menutype *menu) {
	// dynamically create menu lists with driver names
	xdata flashpagetype flashpage;		// where to get the data
	xdata int i,j;

	tm_state = WAITING;

	// get the driver info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	// assign the current selections	
	drivers.current_sel = flashpage.user.selecteddriver;
	vehicles.current_sel = flashpage.user.selectedcar;
	tracks.current_sel = flashpage.user.selectedtrack;

	j = 0;
	for (i=0; i<NUMDRIVERS; i++) {
		if (0xFF != flashpage.user.drivername[i][0]
			&& '\0' != flashpage.user.drivername[i][0]) {

			// create the menu entry
			strncpy(menunames.driver[j], flashpage.user.drivername[i], NAMELEN);

			// put in null terminator just in case
			menunames.driver[j][NAMELEN-1] = '\0';

			// fill in the menu structure
			menulists.driverlist[j].actionid = PICKLIST;
			menulists.driverlist[j].action.f = &savepicklists;
			menulists.driverlist[j].itemname = menunames.driver[j];
			j++;
		} // if
	} // for
	menulists.driverlist[j].actionid = LASTITEM;
	menulists.driverlist[j].action.submenu = NULL;
	menulists.driverlist[j].itemname = NULL;

	// link the menu chain to newly-created menu list
	return (&drivers);
} // loaddrivers

// This function reads the configuration data written by Traqview and creates
// the vehicle menu selections from it.
//
menutype *loadvehicles(menutype *menu) {
	xdata flashpagetype flashpage;		// where to get the data
	xdata int i,j;

	tm_state = WAITING;

	// get the driver and car info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	// assign the current selections	
	drivers.current_sel = flashpage.user.selecteddriver;
	vehicles.current_sel = flashpage.user.selectedcar;
	tracks.current_sel = flashpage.user.selectedtrack;

	j = 0;
	for (i=0; i<NUMCARS; i++) {
		if (0xFF != flashpage.user.car[i].name[0]
			&& '\0' != flashpage.user.car[i].name[0]) {

			// create the menu entry
			strncpy(menunames.vehicle[j], flashpage.user.car[i].name, NAMELEN);

			// put in null terminator just in case
			menunames.vehicle[j][NAMELEN-1] = '\0';

			// fill in the menu structure
			menulists.vehiclelist[j].actionid = PICKLIST;
			menulists.vehiclelist[j].action.f = &savepicklists;
			menulists.vehiclelist[j].itemname = menunames.vehicle[j];
			j++;
		} // if
	} // for
	menulists.vehiclelist[j].actionid = LASTITEM;
	menulists.vehiclelist[j].action.submenu = NULL;
	menulists.vehiclelist[j].itemname = NULL;

	// link the menu chain to newly-created menu list
	return (&vehicles);
} // loadvehicles

// This function reads the configuration data written by Traqview and creates
// the track menu selections from it.
// 
menutype *loadtracks(menutype *menu) {
	xdata flashpagetype flashpage;		// where to get the data
	xdata int i,j;

	tm_state = WAITING;

	// get the driver and car info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	// assign the current selections	
	drivers.current_sel = flashpage.user.selecteddriver;
	vehicles.current_sel = flashpage.user.selectedcar;
	tracks.current_sel = flashpage.user.selectedtrack;

	// get the track info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKPAGE);

	j = 0;
	for (i=0; i<NUMTRACKS; i++) {
		if (0xFF != flashpage.circuit.track[i].name[0]
			&& '\0' != flashpage.circuit.track[i].name[0]) {

			// create the menu entry
			strncpy(menunames.track[j], flashpage.circuit.track[i].name, NAMELEN);

			// put in null terminator just in case
			menunames.track[j][NAMELEN-1] = '\0';

			// fill in the menu structure
			menulists.tracklist[j].actionid = PICKLIST;
			menulists.tracklist[j].action.f = &savepicklists;
			menulists.tracklist[j].itemname = menunames.track[j];
			j++;
		} // if
	} // for
	menulists.tracklist[j].actionid = LASTITEM;
	menulists.tracklist[j].action.submenu = NULL;
	menulists.tracklist[j].itemname = NULL;

	// link the menu chain to newly-created menu list
	return (&tracks);
} // loadtracks

//
// This function reads the configuration data written by Traqview and assigns
// the currently selected sample rate from it
// 
menutype *loadsamp(menutype *menu) {
	xdata flashpagetype flashpage;		// where to get the data

	// get the sample info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	// assign the current selections
	drivers.current_sel = flashpage.user.selecteddriver;
	vehicles.current_sel = flashpage.user.selectedcar;
	tracks.current_sel = flashpage.user.selectedtrack;

	switch (flashpage.user.modesample & 0x0F) {
		case 2:
			samps_per_sec = 20;
			samprates.current_sel = samprates.current_hl = 1;
			break;
		case 4:
			samps_per_sec = 40;
			samprates.current_sel = samprates.current_hl = 2;
			break;
		case 0:
		default:
			samps_per_sec = 10;
			samprates.current_sel = samprates.current_hl = 0;
	} // switch

	// link the menu chain to sample rate menu
	return (&samprates);
} // loadsamp

// this function saves the display units
menutype *saveunits(menutype *menu) {

	if (0 == menu->current_hl)  // English Standard units, set bit to 1
		scratch.calibrate.usersettings |= 0x01;
	else  // Metric units, set bit to 0
		scratch.calibrate.usersettings &= 0xFE;

	// save new contrast in micro flash
	EraseScratchpad();
	WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);

	return (menu);
} // saveunits

//
// This function reads the unit configuration written by Traqview and assigns
// the currently selected units from it
// 
menutype *loadunits(menutype *menu) {

	if (scratch.calibrate.usersettings & 0x01)  // english
		unitchoices.current_sel = unitchoices.current_hl = 0;
	else // metric
		unitchoices.current_sel = unitchoices.current_hl = 1;					

	// link the menu chain to unit selection menu
	return (&unitchoices);
} // loadunits


// this function saves the track length
//   bits 7-5 eight possible states for graph scale
//		000 = +-0.1 sec, 001 = +-0.2 sec, 010 = +-0.5 sec, 011 = +-1 sec,
//		100 = +-2 sec, 101 = +-5 sec, 110 = +-10 sec, 111 = +-20 sec
//   bits 4-2 eight possible states for track size
//		000 = small, 001 = med, 010 = large, others unused
menutype *savetracklen(menutype *menu) {

	scratch.calibrate.usersettings = (scratch.calibrate.usersettings & 0xF1) |
		(tracklenchoices.current_sel << 1);
		
	// save new values in micro flash
	EraseScratchpad();
	WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);

	return (menu);
} // savetracklength

//
// This function reads the graph scale from microprocessor flash memory
// and put it into the menu system
// 
menutype *loadtracklen(menutype *menu) {

	tracklenchoices.current_sel =
	tracklenchoices.current_hl =
	(scratch.calibrate.usersettings & 0x0E) >> 1;

	// check range
	if (tracklenchoices.current_sel > 2)
		tracklenchoices.current_sel =
		tracklenchoices.current_hl = 1;

	// link the menu chain to unit selection menu
	return (&tracklenchoices);
} // loadtracklen

// this function saves the graph scale
menutype *savescale(menutype *menu) {

	scratch.calibrate.usersettings = (scratch.calibrate.usersettings & 0x1F) |
		((scalechoices.current_sel + 2) << 5); 

	// save values in micro flash
	EraseScratchpad();
	WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);

	return (menu);
} // savescale

//
// This function reads the graph scale from microprocessor flash memory
// and put it into the menu system
// 
menutype *loadgraphscale(menutype *menu) {

	switch ((scratch.calibrate.usersettings & 0xE0) >> 5) {
		case 2:		// 0.5
			scalechoices.current_sel = scalechoices.current_hl = 0;
			break;
		case 4:		// 2.0
			scalechoices.current_sel = scalechoices.current_hl = 2;
			break;
		case 5:		// 5.0
			scalechoices.current_sel = scalechoices.current_hl = 3;
			break;
		case 6:		// 10.0
			scalechoices.current_sel = scalechoices.current_hl = 4;
			break;
		case 3:		// 1.0
		default:
			scalechoices.current_sel = scalechoices.current_hl = 1;					
	} // switch

	// link the menu chain to unit selection menu
	return (&scalechoices);
} // loadgraphscale

menutype *review(menutype *menu) {
	tm_state = WAITING;

	button_overlay[BACK] = NULL;

	// dynamically create menu lists with sessions names
	Session_Init();

	// link the menu chain to newly-created menu list
	return (&sessions);
}

// add session number to menu title
menutype *sesstitle(menutype *menu) {
	xdata int i,n;

	tm_state = WAITING;

	// get the index information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, INDEXPAGE);

	// find the nth non-blank session, i is session number - 1
	n = 0;
	for (i = 0; i<NUMSESSIONS; i++) {
		// look for a starting segment number
		if (0xFFFF != flashpage.index.session[i].startseg &&
			0xFFFF != flashpage.index.session[i].endseg)
				if (menu->current_hl == n++) break;
	} // for

	n = strlen(sessionmenu.menuname);
	sprintf(&(sessionmenu.menuname[n-2]), "%02d", (int) (i+1));

	button_overlay[BACK] = review;

	// link the menu chain to session option list
	return (&sessionmenu);
}

//
// detail - shows details of a session
//
// dynamically create the detail text lines
// line 0: driver name
// line 1: vehicle name
// line 2: track name
// line 3: memory usage
// line 4: starting date/time
// line 5: ending date/time
// line 6: laps, sampling rate
// line 7: temperature in F
//
menutype *detail(menutype *menu) {
	xdata int i;
	xdata u08 drivernum, tracknum, carnum;
	xdata u16 startpage;
	xdata u32 mem;
	xdata u16 laps;
	xdata float temperature;

	tm_state = WAITING;

	// initialize
	details.menuname = sessionmenutitle;
	details.prev_menu = &sessionmenu;
	details.menulist = detaillist;
	details.num_items =
	details.current_sel =
	details.current_hl =
	details.top_line = 0;

	for (i = 0; i < NUMDETAILS; i++) {
		detaillist[i].actionid = VIEWLIST;
		detaillist[i].itemname = menunames.menudetail[i];
		detaillist[i].action.submenu = NULL;
	} // for
	// mark the end of the list
	detaillist[NUMDETAILS].actionid = LASTITEM;
	detaillist[NUMDETAILS].itemname = NULL;
	detaillist[NUMDETAILS].action.submenu = NULL;

	// get the session number out of the title & convert to index
	i = strlen(sessionmenu.menuname);
	i = atoi(&(sessionmenu.menuname[i-2])) - 1;

	// get the lap information
	DataFlash_Page_Read(MAX_PAGE_SIZE, (u08 *) laplist, (LASTLAPPAGE - NUMLAPPAGES + i + 1));

	for (laps = 0; laps < NUMLAPS; laps++)
		if (0xFFFFFFFF == laplist[laps]) break;

	// get the index information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, INDEXPAGE);

	// get the drivernum, tracknum, carnum out of session header
	drivernum = flashpage.index.session[i].driver;
	if (drivernum > NUMDRIVERS) drivernum = 0;
	tracknum = flashpage.index.session[i].track;
	if (tracknum > NUMTRACKS) tracknum = 0;
	carnum = flashpage.index.session[i].car;
	if (carnum > NUMCARS) carnum = 0;

	// remember where data starts to get temperature reading
	startpage = flashpage.index.session[i].startseg;

	// fill in the detail pages
	// starting date/time
	formattime(flashpage.index.session[i].starttime,
				flashpage.index.session[i].startweeks,
				LONGDANDT, menunames.menudetail[4]);
	// ending date/time
	formattime(flashpage.index.session[i].endtime,
				flashpage.index.session[i].endweeks,
				LONGDANDT, menunames.menudetail[5]);
	// get number of segments
	mem = (flashpage.index.session[i].endseg - flashpage.index.session[i].startseg + 1) +
		(flashpage.index.session[i].iostartseg - flashpage.index.session[i].ioendseg + 1);
	// memory usage
	sprintf(menunames.menudetail[3], "MEM %dK/%dK, %d%%",
		(u16) (mem/2), (numpages/2), (u16) ((mem*100)/numpages));

	// laps and sampling rate
	switch (flashpage.index.session[i].modesample & 0x0F) {
		default:
		case 0:		// 10 ACC & 1 GPS per second (samptype10)
		case 1:		// 10 ACC & 2 GPS per second (samptype5)
			sprintf(menunames.menudetail[6], "LAPS %d, RATE 10Hz", laps);
			break;
		case 2:		// 20 ACC & 2 GPS per second (samptype10)
		case 3:		// 20 ACC & 4 GPS per second (samptype5)
			sprintf(menunames.menudetail[6], "LAPS %d, RATE 20Hz", laps);
			break;
		case 4:		// 40 ACC & 4 GPS per second (samptype10)
			sprintf(menunames.menudetail[6], "LAPS %d, RATE 40Hz", laps);
			break;
	} // switch

	// get the driver and car information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);

	// copy out driver's name
	strncpy(menunames.menudetail[0], flashpage.user.drivername[drivernum], NAMELEN);
	// put in null terminator just in case
	menunames.menudetail[0][NAMELEN-1] = '\0';

	// copy out car's name
	strncpy(menunames.menudetail[1], flashpage.user.car[carnum].name, NAMELEN);
	// put in null terminator just in case
	menunames.menudetail[1][NAMELEN-1] = '\0';

	// get the track information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKPAGE);

	// copy out track's name
	strncpy(menunames.menudetail[2], flashpage.circuit.track[tracknum].name, NAMELEN);
	// put in null terminator just in case
	menunames.menudetail[2][NAMELEN-1] = '\0';

	// get the temperature from first programmed page
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, startpage);

	// should work for either samptype5 or samptype10
	i = flashpage.secsamp10[1].gps.temp;
	temperature = dotemp(flashpage.secsamp10[1].gps.temp, 'F');
	sprintf(menunames.menudetail[7], "Temp %3.2f F", temperature);

	return (&details);
} // detail

//
// status - shows current unit status
//
// dynamically create the status text lines
// line 0: memory available
// line 1: sessions available
// line 2: battery voltage
// line 3: inputs enabled
//
// list of items must be less than NUMDETAILS
//
menutype *status(menutype *menu) {
	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	xdata u16 i;
	xdata u16 mem = 0;
	xdata u16 sess = 0;
	xdata float hrsleft;
	u08 iobytes = 0;
	xdata u16 batvolt;

	tm_state = WAITING;

	// start ADC conversion to read battery voltage
	SFRPAGE = ADC0_PAGE;
	AD0INT = 0;
	AD0BUSY = 1;

	// initialize
	details.menuname = statustitle;
	details.prev_menu = &mainmenu;
	details.menulist = detaillist;
	details.num_items =
	details.current_sel =
	details.current_hl =
	details.top_line = 0;

	for (i = 0; i < NUMSTATUS; i++) {
		detaillist[i].actionid = VIEWLIST;
		detaillist[i].itemname = menunames.menudetail[i];
		detaillist[i].action.submenu = NULL;
	} // for
	// mark the end of the list
	detaillist[NUMSTATUS].actionid = LASTITEM;
	detaillist[NUMSTATUS].itemname = NULL;
	detaillist[NUMSTATUS].action.submenu = NULL;

	SFRPAGE = SFRPAGE_SAVE;

	// refresh the current selections
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, USERPAGE);
	// set up the I/O collection
	iocollect = flashpage.user.iodata;

	// get the index information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, INDEXPAGE);

	// get memory usage and number of segments used
	for (i = 0; i<NUMSESSIONS; i++) {
		// look for a starting segment number
		if (0xFFFF != flashpage.index.session[i].startseg) {
			sess++;
			mem += (flashpage.index.session[i].endseg - flashpage.index.session[i].startseg + 1) +
			(flashpage.index.session[i].iostartseg - flashpage.index.session[i].ioendseg + 1);
		} // if
	} // for

	// record time = memory available / record rate
	// record rate = 3600 sec/hr * (pagesize/(samprate*iobytes) + (samprate/10/SAMP10S_PER_PAGE))
	for (i=0; i < NUMTDINPUTBYTES; i++)
		if (iocollect & (1 << i)) iobytes++;

//	hrsleft = 3600.0 * ((float) pagesize/(float) (samps_per_sec*iobytes) + ((float) samps_per_sec/10.0/(float) SAMP10S_PER_PAGE));
	hrsleft = (float) (samps_per_sec*iobytes) / (float) pagesize;
	hrsleft += (float) samps_per_sec/10.0/(float) SAMP10S_PER_PAGE;
	hrsleft = hrsleft*3600.0;	// seg per hour
	if (mem >= (LASTDATAPAGE-FIRSTDATAPAGE))
		hrsleft = 0.0;
	else
		hrsleft = (float) (LASTDATAPAGE - mem) / hrsleft;	// hours

	sprintf(menunames.menudetail[0], "Rec Time: %2.1f Hrs", hrsleft);
								   
	// session usage
	sprintf(menunames.menudetail[1], "Sess Avail: %d", NUMSESSIONS - sess);

	// input points
	//   bit 7 = 16bit Freq, 6 = all digitals, 5 = Anlg3, 4 = Anlg2
	//   bit 3 = Anlg1, 2 = Anlg0, 1 = Freq1, 0 = Freq0
	sprintf(menunames.menudetail[3], "I/O: A---- D-- F--", sess, NUMSESSIONS);

	// analogs
	for (i=0; i<NUMANALOGS; i++)
		if (iocollect & (1 << (i+2)))
			menunames.menudetail[3][6+i] = '0' + i;

	// digitals
	if (iocollect & 0x40) {
		menunames.menudetail[3][12] = '4';
		menunames.menudetail[3][13] = '5';
	} // if

	// rpm
	if (iocollect & 0x80) {
		menunames.menudetail[3][15] = 'R';
		menunames.menudetail[3][16] = 'P';
		menunames.menudetail[3][17] = 'M';
	} // if

	SFRPAGE = ADC0_PAGE;
	i=0;
	// battery voltage
	while (!AD0INT)
		if (++i > 10000) break;		// escape hatch 

	batvolt = ADC0;		// read ADC value

	SFRPAGE = SFRPAGE_SAVE;

	// usable range 3000 to 4096
#define LOWVOLT (POWER_THRESHOLD/4)
#define MAXVOLT (1000)		// just under 5V

	i = (100 * (batvolt - LOWVOLT)) / (MAXVOLT - LOWVOLT);
	sprintf(menunames.menudetail[2], "Power: %u%%", i);

#undef MAXVOLT
#undef LOWVOLT

	return (&details);
} // status

// finds the right session number and erases it
menutype *erasesess(menutype *menu) {
	xdata u08 i;

	tm_state = WAITING;

	// get the index information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, INDEXPAGE);

	// get the session number out of the title
	i = strlen(sessionmenu.menuname);
	i = atoi(&(sessionmenu.menuname[i-2]));

	// make sure index is valid
	if (i <= NUMSESSIONS)
		// session numbers start at 1
		Erase_Session(i);

	// since we just deleted an item, recreate menu lists with sessions names
	Session_Init();

	// go back to "sessions"
	return (&sessions);
} // erasesess

menutype *idle(menutype *menu) {
	u08 samprate;
	u08 i;
	xdata flashpagetype tempflash;

	if (DRIVING(tm_state)) {		// save the finish location
		// get the track finish info
		DataFlash_Page_Read(MAX_PAGE_SIZE, tempflash.bigbuff, TRACKFINPAGE);		
	
		// save the finish information and set up to find the start
		lap.finishpos.lat = tempflash.finishandgear.trackfinish[lap.tracknum].finishlat = lap.currentpos.lat;
		lap.finishpos.lon = tempflash.finishandgear.trackfinish[lap.tracknum].finishlon = lap.currentpos.lon;
		lap.finishheading = tempflash.finishandgear.trackfinish[lap.tracknum].finishhead = lap.currentheading;
	
		// write to flash
		DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, tempflash.bigbuff, TRACKFINPAGE);
	} // if

	// only do this if we are storing data
	if (RECORDING(tm_state)) {
		// write out the io data
		if (iocollect)
			DataFlash_Page_Write_From_Buffer_Erase(IOBUFFER, iopagecnt);
		// stop collecting data
		End_Session(session, pagecnt, iopagecnt);
	} // if

	if (TIMING == tm_state ||
		AUTOXHOLDTIME == tm_state ||
		DRAGHOLDTIME == tm_state ||
		DRAGHOLDNUMS == tm_state ||
		HILLCLIMBSTART == tm_state ||
		HILLCLIMBFINISH == tm_state) {
		// save the laps collected
		Lap_Store();

		// save the best lap sectors
		Sector_Store();
	} // if

	tm_state = WAITING;

	// get the sample info in case the mode changed it
	DataFlash_Page_Read(MAX_PAGE_SIZE, tempflash.bigbuff, USERPAGE);

	switch (tempflash.user.modesample & 0x0F) {
		case 2:
			samps_per_sec = 20;
			samprate = 2;
			break;
		case 4:
			samps_per_sec = 40;
			samprate = 4;
			break;
		case 0:
		default:
			samps_per_sec = 10;
			samprate = 0;
	} // switch

	// tell the sensor unit
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = tm_state;
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = samprate;
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2] = iocollect;
	
	// Set up input scaling
	for (i=0; i<NUMANALOGS; i++) {
		switch (analogscaling[i].maxscale) {
			case 20: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 4;
				break;
			case 10: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 3;
				break;
			case 5: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 2;
				break;
		} // switch
	} // for
	SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) MODE_CHANGE, (6+NUMANALOGS));

	// remove button overlays
	button_overlay[SEL] =
	button_overlay[BACK] =
	button_overlay[UP] =
	button_overlay[DN] = NULL;

	return(menu->prev_menu);
} // idle

menutype *markstart(menutype *menu) {

	// only remember start finish and start timing if good gps signal
	if (gpslock >= GPS_GOOD) {

		// record starting position
		Lap_Check(START_POSITION, NULL);
	}
	return(NULL);
} // markstart

menutype *markfinish(menutype *menu) {

	Lap_Check(FINISH_POSITION, NULL);
	
	return(NULL);
} // markfinish

menutype *clearstart(menutype *tempmenu) {
	int i;

	// save current data
	DataFlash_Page_Write_Erase(GENBUFFER, pagesize, flashpage.bigbuff, pagecnt);

	// clear the temporary location
	lap.distance = 0.0;
	lap.currentlap = 0;

	// start finish changed so clear out best lap and sector times
	// get the track finishline information
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, PREDPAGESTART + (lap.tracknum / 4));

	// clear out sector times
	for (i = 0; i < NUMSECTORS; i++)
		flashpage.storedsectors[lap.tracknum % 4].bestsectortimes[i] = 0L;
					
	// clear the best laptime	
	flashpage.storedsectors[lap.tracknum % 4].bestlapever = 0xFFFFFFFFL;
	
	// write it back out
	DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, PREDPAGESTART + (lap.tracknum / 4));

	// get the track info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKPAGE);

	// change the permanent start information
	lap.startpos.lat = flashpage.circuit.track[lap.tracknum].startlat = 
	lap.startpos.lon = flashpage.circuit.track[lap.tracknum].startlon = 0L;
	flashpage.circuit.track[lap.tracknum].startalt = 0;
	lap.startheading = flashpage.circuit.track[lap.tracknum].starthead = 0;

	// write it back out
	DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, TRACKPAGE);

	// get the track finish line info
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, TRACKFINPAGE);

	// clear the permanent finish information
	lap.finishpos.lat = flashpage.finishandgear.trackfinish[lap.tracknum].finishlat = 
	lap.finishpos.lon = flashpage.finishandgear.trackfinish[lap.tracknum].finishlon = 0L;
	flashpage.finishandgear.trackfinish[lap.tracknum].finishalt = 0;
	lap.finishheading = flashpage.finishandgear.trackfinish[lap.tracknum].finishhead = 0;

	// write it back out
	DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, TRACKFINPAGE);

	// retrieve current data
	DataFlash_Page_Read(pagesize, flashpage.bigbuff, pagecnt);

	// overlay back button to turn off data collection
	button_overlay[BACK] = idle;

	if (tm_state == WAITING_LAP) {	// lapping mode
		// overlay select to choose starting point
		button_overlay[SEL] = markstart;
		// wait for user to hit button
		tm_state = LAPS;
	} // if
	else if (tm_state == WAITING_HILL) {
		// overlay select to choose starting point
		button_overlay[SEL] = markstart;

		tm_state = HILLCLIMB;
	} // else
	else {	// autox mode
		// overlay select to choose starting point
		button_overlay[SEL] = NULL;	 // don't need to clear it again, eh?

		Write_Screen((u08 *) autoxready, '-');
		tm_state = AUTOXREADY;
	} // else

	getback.prev_menu = &collect;
	menu = &getback;

	return(NULL);
} // clearstart

menutype *dontclear(menutype *tempmenu) {
	// overlay back button to turn off data collection
	button_overlay[BACK] = idle;
	button_overlay[SEL] = clearmenu;
	switch (tm_state) {
		case WAITING_LAP:
			tm_state = WAITSTART;
			break;
		case WAITING_HILL:
			tm_state = HILLCLIMBWAITSTART;
			break;
		default:
			Write_Screen((u08 *) autoxready, '-');
			tm_state = AUTOXREADY;
	} // switch

	getback.prev_menu = &collect;
	menu = &getback;

	return(NULL);
} // dontclear

menutype *clearmenu(menutype *menu) {

	// get back to whatever we've got now
	clearsf.prev_menu=menu;
	button_overlay[SEL] = NULL;
	button_overlay[BACK] = dontclear;

	// capture incoming state in wait state
	switch (tm_state) {
		case WAITSTART:
			tm_state = WAITING_LAP;
			break;
		case HILLCLIMBWAITSTART:
			tm_state = WAITING_HILL;
			break;
		default:
			tm_state = WAITING_AUTOX;
			break;
	} // switch

	return (&clearsf);
} // clearmenu

menutype *showlaps(menutype *menu) {
	xdata u08 i;

// 2.15 try
	tm_state = REVIEW;

	// initialize
	lap.currentlap = 1;
	
	lap.besttime = 0xFFFFFFFF;
	lap.lastlap = 0;

	// get the session number out of the title
	i = strlen(sessionmenu.menuname);
	i = atoi(&(sessionmenu.menuname[i-2]));

	// make sure index is valid
	if (i <= NUMSESSIONS)
		// read in the laplist
		DataFlash_Page_Read(MAX_PAGE_SIZE, (u08 *) laplist, (LASTLAPPAGE - NUMLAPPAGES + i));

	// find last programmed lap and best lap
	for (i=0; i < NUMLAPS; i++) {
		if (0xFFFFFFFF == laplist[i]) break;
		if (laplist[i] < lap.besttime) lap.besttime = laplist[i];
	} // for

	// check for no programmed laps
	if (0 == (lap.lastlap = i)) {

		// Tell user that no laps programmed
		Info_Screen("No Laps");

	} // if
	else {
		// enable the buttons
		button_overlay[UP] = uplap;
		button_overlay[DN] = dnlap;

		lap.timer = laplist[0];

		// bring up the timing screen
		timingmode = REV;		// make sure numbers appear in correct spot
		Clear_Screen();
		Write_Icon(21, 9, 81, 14, (u08 *) timingicon, '-');
		Write_Timer();
//		// Write the replay icon
//		Write_Icon(0, 0, 12, 10, (u08 *) replay, '-');
		Write_Lap();
		Write_Flag(lap.timer == lap.besttime);
	} // else

	// gets you out of review
	button_overlay[BACK] = backlap;

	return (NULL);
} // showlaps

menutype *dnlap(menutype *menu) {

	if (lap.currentlap == lap.lastlap) lap.currentlap = 1;
	else lap.currentlap++;

	lap.timer = laplist[lap.currentlap-1];

	// bring up the timing screen
	Write_Timer();
//	// Write the replay icon
//	Write_Icon(0, 0, 12, 10, (u08 *) replay, '-');
	Write_Lap();
	Write_Flag(lap.timer == lap.besttime);

	return (NULL);
} // dnlap

menutype *uplap(menutype *menu) {

	if (lap.currentlap == 1) lap.currentlap = lap.lastlap;
	else lap.currentlap--;

	lap.timer = laplist[lap.currentlap-1];

	// bring up the timing screen
	Write_Timer();
//	// Write the replay icon
//	Write_Icon(0, 0, 12, 10, (u08 *) replay, '-');
	Write_Lap();
	Write_Flag(lap.timer == lap.besttime);

	return (NULL);
} // uplap

menutype *backlap(menutype *menu) {

	tm_state = WAITING;

	// remove button overlays
	button_overlay[BACK] = 
	button_overlay[UP] =
	button_overlay[DN] = NULL;

	return(menu->prev_menu);
} // backlap

menutype *previnput(menutype *menu) {
	return (nextinput(menu));
} // previnput

menutype *nextinput(menutype *menu) {
	switch (tm_state) {
		case DRIVE:
			if (iocollect & 0x04) {
				tm_state = GAUGEA0;
				break; }
		case GAUGEA0:
			if (iocollect & 0x08) {
				tm_state = GAUGEA1;
				break; }
		case GAUGEA1:
			if (iocollect & 0x10) {
				tm_state = GAUGEA2;
				break; }
		case GAUGEA2:
			if (iocollect & 0x20) {
				tm_state = GAUGEA3;
				break; }
		case GAUGEA3:
			if (iocollect & 0x40) {
				tm_state = GAUGED4;
				break; }
		case GAUGED4:
			if (iocollect & 0x40) {
				tm_state = GAUGED5;
			break; }
		case GAUGED5:
			tm_state = GPSRPM;
			break;
		case GPSRPM:
			tm_state = GFORCES;
			Clear_Screen();
			// put up a line bar
			Write_Icon(0, 0, NUMBER_OF_COLUMNS, 8, &(menubar[0]), '-');
	
			// write menu centered name
			Write_Text(0, 36, "G-Forces", '-');
			break;
		case GFORCES:
			tm_state = DRIVE;
			break;
		default:
			break;
	} // switch

	return (NULL);
} // nextinput

menutype *upcontrast(menutype *menu) {
	xdata u08 tempstring[13];
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	// increase contrast
	SFRPAGE = PCA0_PAGE;
	if (++scratch.calibrate.contrast > CONTRAST_UPPER)
		scratch.calibrate.contrast = CONTRAST_UPPER;
	PCA0CPL0 = scratch.calibrate.contrast;
	PCA0CPH0 = scratch.calibrate.contrast;
	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page

	sprintf(tempstring, "Cont %d", (s16) scratch.calibrate.contrast - CONTRAST_CENTER);
	Info_Screen(tempstring);

	return (NULL);
} // upcontrast

menutype *dncontrast(menutype *menu) {
	xdata u08 tempstring[13];
	u08 SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page

	// decrease contrast
	SFRPAGE = PCA0_PAGE;
	if (--scratch.calibrate.contrast < CONTRAST_LOWER)
		scratch.calibrate.contrast = CONTRAST_LOWER;
	PCA0CPL0 = scratch.calibrate.contrast;
	PCA0CPH0 = scratch.calibrate.contrast;
	SFRPAGE = SFRPAGE_SAVE;			// Restore SFR page

	sprintf(tempstring, "Cont %d", (s16) scratch.calibrate.contrast - CONTRAST_CENTER);
	Info_Screen(tempstring);

	return (NULL);
} // dncontrast

menutype *backcontrast(menutype *menu) {

	tm_state = WAITING;

	// save new contrast in micro flash
	EraseScratchpad();
	WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);

	// remove button overlays
	button_overlay[BACK] = 
	button_overlay[UP] =
	button_overlay[DN] = NULL;

	return(menu->prev_menu);
} // backcontrast

menutype *contrast(menutype *menu) {
	xdata u08 tempstring[13];

	tm_state = WAITING;

	sprintf(tempstring, "Cont %d", (s16) scratch.calibrate.contrast - CONTRAST_CENTER);
	Info_Screen(tempstring);

	// enable the buttons
	button_overlay[UP] = upcontrast;
	button_overlay[DN] = dncontrast;
	button_overlay[BACK] = backcontrast;

	return(NULL);
} // contrast

menutype *upgmt(menutype *menu) {
	xdata u08 tempstring[13];

	if (++scratch.calibrate.timecode > TIMECODE_UPPER)
		scratch.calibrate.timecode = TIMECODE_UPPER;

	sprintf(tempstring, "GMT %+02d", (s16) scratch.calibrate.timecode);
	Info_Screen(tempstring);

	return (NULL);
} // upgmt

menutype *dngmt(menutype *menu) {
	xdata u08 tempstring[13];

	if (--scratch.calibrate.timecode < TIMECODE_LOWER)
		scratch.calibrate.timecode = TIMECODE_LOWER;

	sprintf(tempstring, "GMT %+02d", (s16) scratch.calibrate.timecode);
	Info_Screen(tempstring);

	return (NULL);
} // dngmt

//
// This function allows the setting of time offset
// 
menutype *dogmt(menutype *menu) {
	xdata u08 tempstring[13];

	tm_state = WAITING;

	sprintf(tempstring, "GMT %+02d", (s16) scratch.calibrate.timecode);
	Info_Screen(tempstring);

	// enable the buttons
	button_overlay[UP] = upgmt;
	button_overlay[DN] = dngmt;
	button_overlay[BACK] = backcontrast; // doubling up on this function

	return (NULL);
} // dogmt

menutype *uplaphold(menutype *menu) {
	xdata u08 tempstring[13];

	if ((scratch.calibrate.lapholdtime += 10) > LAPHOLD_UPPER)
		scratch.calibrate.lapholdtime = LAPHOLD_UPPER;

	sprintf(tempstring, "LapHold %3d", (s16) scratch.calibrate.lapholdtime);
	Info_Screen(tempstring);

	return (NULL);
} // uplaphold

menutype *dnlaphold(menutype *menu) {
	xdata u08 tempstring[13];

	if (scratch.calibrate.lapholdtime <= (10 + LAPHOLD_LOWER))
		scratch.calibrate.lapholdtime = LAPHOLD_LOWER;
	else
		scratch.calibrate.lapholdtime -= 10;

	sprintf(tempstring, "LapHold %3d", (s16) scratch.calibrate.lapholdtime);
	Info_Screen(tempstring);

	return (NULL);
} // dnlaphold

//
// This function allows the setting of time offset
// 
menutype *laphold(menutype *menu) {
	xdata u08 tempstring[13];

	tm_state = WAITING;

	sprintf(tempstring, "LapHold %3d", (s16) scratch.calibrate.lapholdtime);
	Info_Screen(tempstring);

	// enable the buttons
	button_overlay[UP] = uplaphold;
	button_overlay[DN] = dnlaphold;
	button_overlay[BACK] = backcontrast; // doubling up on this function

	return (NULL);
} // laphold

menutype *stage(menutype *menu) {
//	u08 samprate;
	u08 i;

	if (AUTOXSTAGE == tm_state) {
		Write_Screen((u08 *) autoxready, '-');
	
		// check for finish position
		if (lap.finishpos.lat == 0xFFFFFFFF || lap.finishpos.lat == 0L)
			// no finish position so no need to clear it
			button_overlay[SEL] = NULL;
		else
			// select button will go to clear finish menu
			button_overlay[SEL] = clearmenu;
	
		// now staged
		tm_state = AUTOXREADY;
	} // if
	else {	// DRAGSTAGE
		Write_Icon(0, 0, 59, 32, (u08 *) readytext, '-');
		
		// now staged
		tm_state = DRAGREADY;
		draglighttimer = 0;
		
		// clear select button
		button_overlay[SEL] = NULL;
	} //else
	
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = tm_state;
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+1] = 4;		// sampling rate always 40Hz for staged modes
	com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+2] = iocollect;

	// Set up input scaling
	for (i=0; i<NUMANALOGS; i++) {
		switch (analogscaling[i].maxscale) {
			case 20: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 4;
				break;
			case 10: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 3;
				break;
			case 5: com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START+3+i] = 2;
				break;
		} // switch
	} // for
	SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) MODE_CHANGE, (6+NUMANALOGS));

	return(NULL);
} // stage

//
// This function reads the start screen configuration from scratchpad and assigns
// the currently selected start screen value from it
// 
menutype *loadstartscreen(menutype *menu) {

	switch (scratch.calibrate.menustart) {
		case (0xFF):
			startscreenchoices.current_sel = startscreenchoices.current_hl = 0;
			break;
		default:
			startscreenchoices.current_sel = startscreenchoices.current_hl = scratch.calibrate.menustart;
			break;
	} // switch

	// link the menu chain to unit selection menu
	return (&startscreenchoices);
} // loadstartscreen

// this function saves the sampling rate
menutype *savestartscreen(menutype *menu) {

	// determine which selection was chosen
	switch (menu->current_hl) {
		case 0: // Main Menu
			scratch.calibrate.menustart = 0xFF;
			break;
		default:
			scratch.calibrate.menustart = menu->current_hl;
			break;
	} // switch

	// save new start menu in micro flash
	EraseScratchpad();
	WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);

	return (menu);
} // savestartscreen

//
// This function reads the start screen configuration from scratchpad and assigns
// the currently selected start screen value from it
// 
menutype *loadcameras(menutype *menu) {

	// get the page
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);
	
	switch (flashpage.io.digouts[0] & 0x0F) {
		case SONYLANC:
			camerachoices.current_sel = camerachoices.current_hl = 6;
			break;		
		case GOPROCAMERA:
			camerachoices.current_sel = camerachoices.current_hl = 4;
			break;
		case REPLAYCAMERA:
			camerachoices.current_sel = camerachoices.current_hl = 5;
			break;
		case REPLAYXDUSB:
			camerachoices.current_sel = camerachoices.current_hl = 7;
			break;
		case REPLAYMINI:
			camerachoices.current_sel = camerachoices.current_hl = 2;
			break;
		case REPLAYPRIME:
			camerachoices.current_sel = camerachoices.current_hl = 3;
			break;	
		case CAMERA:
		case MOBIUS:
			camerachoices.current_sel = camerachoices.current_hl = 1;
			break;
		default:
			camerachoices.current_sel = camerachoices.current_hl = 0;
			break;
	} // switch
			
	// link the menu chain to unit selection menu
	return (&camerachoices);
} // loadcameras

// this function saves the sampling rate
menutype *savecameras(menutype *menu) {

	// get the page
	DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);
	
	// determine which selection was chosen and program the camera bits
	switch (menu->current_hl) {
		case 0: // None
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + DISABLED;
			break;
		case 1: // Mobius
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + MOBIUS;
			break;
		case 2: // Replay Mini USB
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYMINI;
			break;
		case 3: // Replay PrimeX USB
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYPRIME;
			break;
		case 4: // GoPro
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + GOPROCAMERA;
			break;
		case 5: // ReplayXD
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYCAMERA;
			break;
		case 6: // Sony HD
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + SONYLANC;
			break;
		case 7: // ReplayXD USB
			flashpage.io.digouts[0] = (flashpage.io.digouts[0] & 0xF0) + REPLAYXDUSB;
			break;
	} // switch

	// write buffer to flash
	DataFlash_Page_Write_Erase(GENBUFFER, MAX_PAGE_SIZE, flashpage.bigbuff, IODEFPAGE);

	return (menu);
} // savecameras

//
// This function reads the auto on configuration from scratchpad and assigns
// the currently selected auto on value from it
// 
menutype *loadautoon(menutype *menu) {

	if (scratch.calibrate.usersettings & 0x10)  // disabled
		autoonchoices.current_sel = autoonchoices.current_hl = 0;
	else // enabled
		autoonchoices.current_sel = autoonchoices.current_hl = 1;					

	// link the menu chain to unit selection menu
	return (&autoonchoices);
} // loadautoon

// this function saves the sampling rate
menutype *saveautoon(menutype *menu) {

	// determine which selection was chosen
	switch (menu->current_hl) {
		case 1: // AutoOn Enabled
			scratch.calibrate.usersettings &= 0xEF;
			break;
		case 0: // AutoOn Disabled
		default:
			scratch.calibrate.usersettings |= 0x10;
			break;
	} // switch

	// save new autoon status in micro flash
	EraseScratchpad();
	WriteScratchpad(scratch.scratchbuff, SCRATCHLEN);

	if (su_present && su_hw >= 200) {		// only do this if we are connected to SU2
		// Send AutoOn status to SU
		com[msgrouting[SENSOR_UNIT]].txbuff[DATA_START] = AUTOONENABLED;
		SendMessage((u08) DISPLAY_UNIT, (u08) SENSOR_UNIT, (u08) AUTO_ON, !(menu->current_hl));
	} // if

	return (menu);
} // savesautoon

// this function displays the drag numbers
menutype *displaydragnums(menutype *menu) {
	xdata float tempfloat;
	xdata char tempstring[21];
	xdata u16 tempweight;
	xdata u08 hploc;			// column on screen to put HP
						
	tm_state = DRAGHOLDNUMS;
	button_overlay[SEL] = NULL;

	// put up final adjusted numbers
	Clear_Screen();

	if (0 == dragnumbers.zerosixty) {
		Write_Text(0, 0, "0-60: N/A", '-');
	} // if
	else {
		sprintf(tempstring, "0-60: %1.1f", (float) dragnumbers.zerosixty / 100.0);
		Write_Text(0, 0, tempstring, '-');
	} // else
	
	if (dragnumbers.reaction < 0) {
		Write_Text(0, 61, "REDLIGHT", '-');
	} // if
	else {
		tempfloat = (float) dragnumbers.reaction;
		tempfloat = tempfloat / 100.0;
		sprintf(tempstring, "RT: %1.2f", tempfloat);
		Write_Text(0, 61, tempstring, '-');
	} // else

	sprintf(tempstring, "1/8:%1.2f", (float) dragnumbers.eighthmile / 100.0);
	Write_Text(1, 0, tempstring, '-');

	sprintf(tempstring, "60': %1.2f", (float) dragnumbers.sixtyfoot / 100.0);
	Write_Text(3, 0, tempstring, '-');
	sprintf(tempstring, "330':%1.2f", (float) dragnumbers.threethirty / 100.0);
	Write_Text(3, 61, tempstring, '-');

	if (dragnumbers.quarter) {
		hploc = 0;		// put hp on screen at 2, 0
//		if (0 != dragnumbers.zerohundred) {
//			sprintf(tempstring, "100:%1.2f", (float) dragnumbers.zerohundred / 100.0);
//			Write_Text(0, 61, tempstring, '-');
//		} // if

		sprintf(tempstring, "1/4:%1.2f", (float) dragnumbers.quartermile / 100.0);
		Write_Text(1, 61, tempstring, '-');
		sprintf(tempstring, "@ %1.1f", (float) dragnumbers.quartertrap / 10.0);
		Write_Text(2, 61, tempstring, '-');
		
	} // if
	else { 		// 1/8 mile
		hploc = 61;		// put hp on screen at 2, 61

		sprintf(tempstring, "Gs: %1.2f", (float) dragnumbers.launchg / 100.0);
		Write_Text(1, 61, tempstring, '-');

		sprintf(tempstring, "@ %1.1f", (float) dragnumbers.eighthtrap / 10.0);
		Write_Text(2, 0, tempstring, '-');
	} // else
	

	// calculate HP and torque
	// see if vehicle weight is non-zero
	// see if RPM enabled
	// 1/8 mile 2,61
	// 1/4 mile 
	
	// calculate hp if user has entered weight for vehicle
	if (0 == (tempweight = currentcar.cylandweight & 0xFFF0)) {
		Write_Text(2, hploc, "HP: No Wt", '-');
	} // if
	else {		// got a weight value so calculate hp
		xdata float weightlbs, et, factor, hp;
		
		weightlbs = 2.0 * (float) (tempweight >> 4);
//			weightkg = (float) weightlbs * 0.4536;
		et = ((float) dragnumbers.eighthmile) / 100.0;
//		factor = et / 3.67926;		// conversion factor for 1/8 mile FROM WEBSITE THEN CONVERTED TO 1/8
//		factor = et / 3.85;			// conversion factor for 1/8 mile FIRST ATTEMPT AT CALIBRATION
		factor = et / 4.00;			// conversion factor for 1/8 mile CALIBRATED

		hp = weightlbs / (factor * factor * factor);

		dragnumbers.horsepower = (u16) (hp * 10.0);

		sprintf(tempstring, "HP: %1.1f", hp);
		Write_Text(2, hploc, tempstring, '-');
	} // if

	return (NULL);
} // displaydragnums
