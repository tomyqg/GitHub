// TrackDetect.c
//
//	These functions will create the data structures for the preprogrammed and user defined tracks
//	and determine a short list of the closest tracks.
//
//	GLOBAL DATA
//
//	int trackShortList[NUM_TOTAL_TRACKS];			// indexes index into trackTable by closest distance to current position
//	int numberOfShortListTracks;						// number of tracks on the short list
//	int numberOfTracks = NUM_SYSTEM_TRACKS;				// number of tracks on the list
//	TRACK_LIST_TYPE trackTable[NUM_TOTAL_TRACKS];		// preprogrammed and user tracks combined table
//
//	HOW TO USE THESE FUNCTIONS:
//
//  On startup call CreateTracks. This will copy the system tracks to working table and add the user tracks to the track table while maintaining the Tracktable in alphanumeric order.
//
//  If the user wants to add a track:
//  Write the track to the SD card with trackname.
//  Call Addtrack with the name of the track and the TRACK_DATA_STRUCTURE that was written. This will update the tables
//	without having to re-read all the tracks.
//
//	To rename a track:
//	Change the name of the track file if it is a user track file.
//	Create a user track file if the track is a pre-programmed track file.
//	Change the name in the track table.
//
//	To delete a track:
//	Only user tracks can be deleted.
//	Erase track file from SD card.
//	Call DeleteTrack with name of track.
//
//	To change the coordinates of a track:
//	To change a pre-programmed track, create a track file and copy over the contents. It can be named the same
//	as the pre-programmed track. It will replace the pre-programmed track in the table upon startup.
//	Call ReadTracks
//	To change a user track, change the coordinates in the track file
//	Call ReadTracks
//
//	When entering Lap/Race Mode
//	Call CurrentTrackInRange. This will check to see if the currently selected track has 0 lat/lon or if it is near our location.
//	if true, enter recording mode as normal
//	if false, call GetClosestTracks. This will create a short list of nearby tracks, sorted by distance, closest first.
//	present this list to user and make them pick one or create a new track.
//	note: close list may be empty if none are nearby.
//	trackTable[trackShortList[0]] is the first nearby list entry assuming trackShortList is the user array of indexes
//
//	When user has finished recording or set a start/finish:
//	Save the current track as a user file, even if current track is pre-programmed.
//	This will preserve the start/finish and predictive lap information for that track.
//
//	FULL TRACK LIST
//	If user wants to select from the full track list, create an alphanumeric list of track names for the ListWheel using
//	the TrackTable in the order it exists
//

#include "commondefs.h"
#include "DUII.h"
#include "TrackDetect.h"
#include "RaceSetupScreens.h"
#include "FlashReadWrite.h"
#include "DataHandler.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WINDOWS
#include "windows.h"
#include "..\Application\dirent.h"
#else
#include "FS.h"
#endif

#ifndef FLOATABS
#define FLOATABS(a) ((a >= 0.0)? (a) : -(a))
#endif


//
// Public data declarations
//
													// trackTable[trackShortList[n]].trackName is (n+1)st name by distance from current location
int trackShortList[NUM_TOTAL_TRACKS];				// indexes index into trackTable by closest distance to current position

int numberOfShortListTracks = 0;					// number of tracks on the short list

int numberOfTracks = NUM_SYSTEM_TRACKS;				// number of tracks on the list

// NOTE: This track list MUST BE SORTED BY ALPHANUMERIC NAME (1st field in structure)
TRACK_LIST_TYPE trackTable[NUM_TOTAL_TRACKS];		// where to build the track table

// format for track data
// name, SDCardEntry=0, start lat, start lon, start hdg, fin lat, fin lon, fin hdg, coursetype
// coursetype, 0=roadcourse, 1=oval, 2=autoxrally, 3=drageighth, 4=drag1000, 5=dragquarter, 6=drag, 7=hillclimb, 8=drive		

// NOTE: 15 character maximum track name length
//
const TRACK_LIST_TYPE romTrackTable[NUM_SYSTEM_TRACKS]=		// preprogrammed tracks
{
{ "Adelaide",0,-34.930387,138.620489,323,0,0,0,0 },
{ "Adria",0,45.044718,12.147784,0,0,0,0,0 },
{ "Aintree",0,53.475264,-2.940280,88,0,0,0,0 },
{ "Albacete",0,39.005159,-1.795613,336,0,0,0,0 },
{ "Albi",0,43.916046,2.118621,122,0,0,0,0 },
{ "Aldo Scribante",0,-33.806821,25.645893,51,0,0,0,0 },
{ "Alfa Romeo",0,45.478785,8.298540,19,0,0,0,0 },
{ "AMP",0,34.433171,-84.176247,145,0,0,0,0 },
{ "AMP North",0,34.435466,-84.180383,117,0,0,0,0 },
{ "AMP South",0,34.433171,-84.176247,145,0,0,0,0 },
{ "AMS",0,33.385770,-84.316895,289,0,0,0,1 },
{ "Arizona",0,33.516861,-112.391797,4,0,0,0,0 },
{ "Arroyo Seco",0,32.236839,-107.429681,301,0,0,0,0 },
{ "Asan",0,34.089217,133.858532,145,0,0,0,0 },
{ "Ascari",0,36.824852,-5.083314,117,0,0,0,0 },
{ "Aspen",0,39.266417,-106.880040,183,0,0,0,0 },
{ "Assen",0,52.962311,6.524101,51,0,0,0,0 },
{ "Atlantic",0,45.145113,-63.450025,159,0,0,0,0 },
{ "Autobahn Full",0,41.456558,-88.125895,297,0,0,0,0 },
{ "Autobahn North",0,41.456558,-88.125895,297,0,0,0,0 },
{ "Autobahn South",0,41.455211,-88.126874,122,0,0,0,0 },
{ "Autopolis",0,33.037452,130.975166,123,0,0,0,0 },
{ "Bahia Blanca",0,-38.681687,-62.177065,70,0,0,0,0 },
{ "Bahrain",0,26.032448,50.510419,2,0,0,0,0 },
{ "Balcarce",0,-37.882962,-58.266697,212,0,0,0,0 },
{ "Barbagallo",0,-31.664183,115.786342,1,0,0,0,0 },
{ "Barber",0,33.532616,-86.619623,46,0,0,0,0 },
{ "Baskerville",0,-42.746488,147.294738,34,0,0,0,0 },
{ "Bathurst",0,-33.439383,149.558226,280,0,0,0,0 },
{ "Belle Isle",0,42.337098,-82.998583,76,0,0,0,0 },
{ "Bedford",0,52.235135,-0.474326,234,0,0,0,0 },
{ "Blackhawk",0,42.487885,-89.117478,1,0,0,0,0 },
{ "Blue-Grass",0,38.764675,-84.803520,111,0,0,0,0 },
{ "Bois-Guyon",0,48.759831,1.371472,339,0,0,0,0 },
{ "BordeauxMerigna",0,44.843020,-0.726142,21,0,0,0,0 },
{ "Brainerd",0,46.416862,-94.274808,104,0,0,0,0 },
{ "Brands Hatch",0,51.360292,0.260306,65,0,0,0,0 },
{ "Brasilia",0,-15.772817,-47.900251,69,0,0,0,0 },
{ "Bretagne",0,47.345564,-1.798571,283,0,0,0,0 },
{ "Bristol",0,36.515996,-82.256168,328,0,0,0,1 },
{ "Brno",0,49.202850,16.445444,294,0,0,0,0 },
{ "Brooklands",0,51.345272,-0.469927,110,0,0,0,0 },
{ "Buenos Aires",0,-34.694227,-58.461389,34,0,0,0,0 },
{ "Bushy Park",0,13.138678,-59.466997,218,0,0,0,0 },
{ "Buttonwillow",0,35.488769,-119.544479,86,0,0,0,0 },
{ "ButtonwillowCCW",0,35.488769,-119.544479,266,0,0,0,0 },
{ "Cadwell Park",0,53.310238,-0.059483,203,0,0,0,0 },
{ "Calabogie",0,45.301735,-76.672361,276,0,0,0,0 },
{ "Calabogie East",0,45.301735,-76.672361,276,0,0,0,0 },
{ "Calabogie West",0,45.304807,-76.676331,198,0,0,0,0 },
{ "Calafat",0,40.932306,0.839517,42,0,0,0,0 },
{ "Calder Oval",0,-37.673054,144.760001,2,0,0,0,1 },
{ "Calder Park",0,-37.670166,144.753435,161,0,0,0,0 },
{ "California",0,34.086069,-117.500525,88,0,0,0,0 },
{ "Campo Grande",0,-20.475911,-54.466025,343,0,0,0,0 },
{ "Carole",0,48.978825,2.522772,352,0,0,0,0 },
{ "Carolina MSP",0,34.487727,-80.597004,135,0,0,0,0 },
{ "Cartagena",0,37.645100,-1.035785,326,0,0,0,0 },
{ "Cascavel",0,-24.980593,-53.382729,290,0,0,0,0 },
{ "Castle Combe",0,51.492737,-2.216025,73,0,0,0,0 },
{ "Castrol",0,53.332740,-113.595620,114,0,0,0,0 },
{ "Catalunya",0,41.570030,2.261216,212,0,0,0,0 },
{ "Central",0,35.024892,134.922377,210,0,0,0,0 },
{ "Charlotte",0,35.352264,-80.685588,195,0,0,0,1 },
{ "Chicagoland",0,41.473132,-88.059875,140,0,0,0,1 },
{ "Chimay",0,50.055310,4.303996,245,0,0,0,0 },
{ "Chuckwalla",0,33.753664,-115.319613,38,0,0,0,0 },
{ "Chuckwalla CCW",0,33.753664,-115.319613,218,0,0,0,0 },
{ "ClermontFerrand",0,45.740409,3.026166,62,0,0,0,0 },
{ "Comodoro",0,-45.902735,-67.543902,97,0,0,0,0 },
{ "Concordia",0,-31.308441,-58.003817,101,0,0,0,0 },
{ "Corvette Museum",0,37.000960,-86.368431,239,0,0,0,0 },
{ "COTA",0,30.131743,-97.639629,127,0,0,0,0 },
{ "Croft",0,54.455376,-1.555548,15,0,0,0,0 },
{ "Croix enTernois",0,50.378946,2.296258,66,0,0,0,0 },
{ "Darlington",0,34.293699,-79.906054,104,0,0,0,1 },
{ "Daytona",0,29.187673,-81.072805,213,0,0,0,1 },
{ "Daytona Road",0,29.187673,-81.072805,213,0,0,0,0 },
{ "Del Jar",0,40.614653,-3.583160,162,0,0,0,0 },
{ "Dijon-Prenois",0,47.364841,4.899811,45,0,0,0,0 },
{ "Dominion",0,38.138519,-77.502503,37,0,0,0,0 },
{ "Donington",0,52.829818,-1.379557,281,0,0,0,0 },
{ "Dover",0,39.190759,-75.528747,351,0,0,0,1 },
{ "Dubai",0,25.047425,55.239581,274,0,0,0,0 },
{ "Eagles Canyon",0,33.367612,-97.429672,307,0,0,0,0 },
{ "Eastern Creek",0,-33.803774,150.870928,281,0,0,0,0 },
{ "Ebisu East",0,37.644430,140.370609,298,0,0,0,0 },
{ "Ebisu North",0,37.646367,140.364139,210,0,0,0,0 },
{ "Ebisu West",0,37.639885,140.370759,87,0,0,0,0 },
{ "El Jabali",0,13.811084,-89.330029,60,0,0,0,0 },
{ "Elvington",0,53.992314,-0.988268,251,0,0,0,0 },
{ "Enna-Pergusa",0,37.520149,14.305430,60,0,0,0,0 },
{ "Estoril",0,38.749417,-9.392906,204,0,0,0,0 },
{ "European MP",0,29.847475,-82.057073,165,0,0,0,0 },
{ "Evergreen",0,47.869230,-121.988359,181,0,0,0,1 },
{ "Fiorano",0,44.534466,10.857515,44,0,0,0,0 },
{ "Firebird East",0,33.273780,-111.963341,148,0,0,0,0 },
{ "Firebird Main",0,33.267526,-111.968143,147,0,0,0,0 },
{ "Firebird West",0,33.268914,-111.971970,195,0,0,0,0 },
{ "Ford Test",0,42.298379,-83.225928,118,0,0,0,1 },
{ "Fortaleza",0,-3.882334,-38.458790,279,0,0,0,0 },
{ "Fuji",0,35.373825,138.929737,51,0,0,0,0 },
{ "Fundidora Park",0,25.677801,-100.286475,185,0,0,0,0 },
{ "G Villeneuve",0,45.500053,-73.522715,169,0,0,0,0 },
{ "Gateway",0,38.650451,-90.136911,165,0,0,0,0 },
{ "General Roca",0,-39.076409,-67.580413,98,0,0,0,0 },
{ "Gimli",0,50.631988,-97.053995,150,0,0,0,0 },
{ "Gingerman",0,42.408039,-86.140462,359,0,0,0,0 },
{ "Goiania",0,-16.719399,-49.193029,317,0,0,0,0 },
{ "Goodwood",0,50.857956,-0.752628,174,0,0,0,0 },
{ "Goodyear",0,43.524038,3.791573,52,0,0,0,0 },
{ "Gotland Ring",0,57.837584,18.830808,262,0,0,0,0 },
{ "Grand Bayou",0,30.018324,-91.080817,97,0,0,0,0 },
{ "GrandSport",0,29.324723,-95.046283,158,0,0,0,0 },
{ "Grattan",0,43.096055,-85.384305,76,0,0,0,0 },
{ "Hallett CCW",0,36.220678,-96.590397,354,0,0,0,0 },
{ "Hallett CW",0,36.220678,-96.590397,174,0,0,0,0 },
{ "Harris Hill",0,29.918896,-97.873212,44,0,0,0,0 },
{ "Hastings",0,40.580126,-98.351323,89,0,0,0,0 },
{ "Heartland Park",0,38.926652,-95.676750,179,0,0,0,0 },
{ "Heidbergring",0,53.452284,10.364013,192,0,0,0,0 },
{ "Hermanos Rodrig",0,19.406146,-99.092836,98,0,0,0,0 },
{ "Hidden Valley",0,-12.448305,130.907829,66,0,0,0,0 },
{ "High Plains",0,39.734947,-103.893971,101,0,0,0,0 },
{ "Hockenheim",0,49.327784,8.565841,334,0,0,0,0 },
{ "Hokkaido",0,42.921596,140.714468,105,0,0,0,0 },
{ "Homestead",0,25.453701,-80.409708,243,0,0,0,1 },
{ "Honda Proving",0,35.260408,-117.958971,78,0,0,0,0 },
{ "HSR Kyushu",0,32.897757,130.877496,262,0,0,0,0 },
{ "Hullavington",0,51.528749,-2.134542,209,0,0,0,0 },
{ "Hungaroring",0,47.578883,19.248414,308,0,0,0,0 },
{ "Hutchinson Isle",0,32.085091,-81.073473,94,0,0,0,0 },
{ "Imola",0,44.344376,11.713781,281,0,0,0,0 },
{ "IndeRanch CW",0,32.225470,-110.005716,73,0,0,0,0 },
{ "IndeRanch CCW",0,32.225470,-110.005716,264,0,0,0,0 },
{ "IndeRanchN CW",0,32.230527,-110.010774,36,0,0,0,0 },
{ "IndeRanchN CCW",0,32.230527,-110.010774,218,0,0,0,0 },
{ "IndeRanchS CW",0,32.225470,-110.005716,73,0,0,0,0 },
{ "IndeRanchS CCW",0,32.225470,-110.005716,264,0,0,0,0 },
{ "Indy Oval",0,39.793116,-86.238899,180,0,0,0,1 },
{ "Indy Roadcourse",0,39.793116,-86.238899,359,0,0,0,0 },
{ "Interlagos",0,-23.704032,-46.699871,165,0,0,0,0 },
{ "Interlomas",0,-38.620731,-72.467027,294,0,0,0,0 },
{ "Irwindale",0,34.110099,-117.987546,292,0,0,0,1 },
{ "Isla Grande",0,18.457705,-66.090912,263,0,0,0,0 },
{ "Jerez",0,37.709674,-6.032495,337,0,0,0,0 },
{ "Johor",0,1.480085,103.908335,304,0,0,0,0 },
{ "Jyllandsringen",0,56.176384,9.660040,295,0,0,0,0 },
{ "Kansas",0,39.116582,-94.833870,194,0,0,0,1 },
{ "Keevil",0,51.309396,-2.117743,308,0,0,0,0 },
{ "Keimola",0,60.319609,24.832014,149,0,0,0,0 },
{ "Kentucky",0,38.712543,-84.918469,219,0,0,0,1 },
{ "Kia Test",0,35.048619,-118.032625,64,0,0,0,0 },
{ "Kinnekulle",0,58.546931,13.396765,86,0,0,0,0 },
{ "Knockhill",0,56.130921,-3.506583,97,0,0,0,0 },
{ "Knutstorp",0,55.986756,13.114835,304,0,0,0,0 },
{ "Kyalami",0,-25.998775,28.069899,42,0,0,0,0 },
{ "La Guacima",0,9.974421,-84.248102,1,0,0,0,0 },
{ "La Plata",0,-34.979526,-58.181852,300,0,0,0,0 },
{ "Laguna Seca",0,36.586455,-121.756617,213,0,0,0,0 },
{ "Lakeside",0,-27.228468,152.964902,286,0,0,0,0 },
{ "Las Flores",0,-36.006546,-59.071139,187,0,0,0,0 },
{ "Las Vegas",0,36.274502,-115.012612,227,0,0,0,1 },
{ "Las Vegas Bike",0,36.268112,-115.016851,160,0,0,0,0 },
{ "Las Vegas Inf",0,36.272284,-115.009738,50,0,0,0,0 },
{ "Lausitzring",0,51.534948,13.927526,253,0,0,0,0 },
{ "Leconfield",0,53.874843,-0.437868,39,0,0,0,0 },
{ "Ledenon",0,43.923181,4.504175,192,0,0,0,0 },
{ "LeMans",0,47.949863,0.207512,2,0,0,0,0 },
{ "Lime Rock",0,41.928617,-73.380993,115,0,0,0,0 },
{ "Llandow",0,51.432468,-3.497462,90,0,0,0,0 },
{ "Long Beach",0,33.763395,-118.185430,234,0,0,0,0 },
{ "Lotus Hethel",0,52.562046,1.178361,6,0,0,0,0 },
{ "Lydden Hill",0,51.177361,1.198170,296,0,0,0,0 },
{ "Macau",0,22.198655,113.558594,253,0,0,0,0 },
{ "Magione",0,43.131256,12.239110,331,0,0,0,0 },
{ "Magny-Cours",0,46.864090,3.163692,52,0,0,0,0 },
{ "Mallala",0,-34.415210,138.505854,271,0,0,0,0 },
{ "Mallory Park",0,52.598647,-1.336990,179,0,0,0,0 },
{ "Manfield",0,-40.235791,175.558439,303,0,0,0,0 },
{ "Mantorp Park",0,58.369112,15.283769,64,0,0,0,0 },
{ "Mar de Ajo",0,-36.710566,-56.720414,173,0,0,0,0 },
{ "Mar del Plata",0,-37.994771,-57.638728,79,0,0,0,0 },
{ "Martinsville",0,36.634232,-79.852273,203,0,0,0,1 },
{ "Mas du Clos",0,45.936117,2.301679,327,0,0,0,0 },
{ "Megara",0,37.987021,23.362906,74,0,0,0,0 },
{ "Melbourne",0,-37.849971,144.968870,315,0,0,0,0 },
{ "Memphis",0,35.282179,-89.946841,49,0,0,0,1 },
{ "Michigan",0,42.067642,-84.244741,192,0,0,0,1 },
{ "Mid America",0,41.024469,-95.821256,0,0,0,0,0 },
{ "Mid Ohio",0,40.689477,-82.635255,272,0,0,0,0 },
{ "Miller East",0,40.583147,-112.376375,270,0,0,0,0 },
{ "Miller Full",0,40.583147,-112.376375,270,0,0,0,0 },
{ "Miller Outer",0,40.583147,-112.376375,270,0,0,0,0 },
{ "Miller West",0,40.583094,-112.382433,270,0,0,0,0 },
{ "Milwaukee Mile",0,43.020429,-88.012904,180,0,0,0,1 },
{ "Misano",0,43.961868,12.683971,204,0,0,0,0 },
{ "Mission Raceway",0,49.126684,-122.325565,296,0,0,0,0 },
{ "Monaco GP",0,43.735025,7.421238,353,0,0,0,0 },
{ "Mondello Park",0,53.257430,-6.744543,156,0,0,0,0 },
{ "Monticello",0,41.619834,-74.697488,124,0,0,0,0 },
{ "MontTremblant",0,46.193319,-74.611641,57,0,0,0,0 },
{ "MontTremblant N",0,46.193319,-74.611641,57,0,0,0,0 },
{ "MontTremblant S",0,46.188120,-74.611480,354,0,0,0,0 },
{ "Monza",0,45.618968,9.281224,6,0,0,0,0 },
{ "Mosport",0,44.054352,-78.674383,107,0,0,0,0 },
{ "Most",0,50.519437,13.607745,280,0,0,0,0 },
{ "Motegi",0,36.533032,140.226713,331,0,0,0,0 },
{ "Motorland",0,34.807201,136.505186,25,0,0,0,0 },
{ "MSR Cresson",0,32.523127,-97.615258,216,0,0,0,0 },
{ "MSR Cresson CW",0,32.523127,-97.615258,36,0,0,0,0 },
{ "MSR Houston",0,29.279053,-95.422399,101,0,0,0,0 },
{ "MSR Houston CW",0,29.279053,-95.422399,281,0,0,0,0 },
{ "Mugello",0,43.997594,11.371528,24,0,0,0,0 },
{ "Myrtle Beach",0,33.745949,-78.952571,129,0,0,0,1 },
{ "Nakayama",0,34.786056,134.176878,9,0,0,0,0 },
{ "Nashville",0,36.046243,-86.409681,188,0,0,0,1 },
{ "Nasu Motors",0,36.970795,140.127397,252,0,0,0,0 },
{ "Nazereth",0,40.727403,-75.318883,27,0,0,0,1 },
{ "NCCAR CCW",0,36.482192,-77.579926,178,0,0,0,0 },
{ "NCCAR CW",0,36.482192,-77.579926,358,0,0,0,0 },
{ "Nelson Ledges",0,41.307095,-81.014114,179,0,0,0,0 },
{ "New Hampshire",0,43.363078,-71.462014,199,0,0,0,1 },
{ "Nihonkai",0,37.744895,138.801838,63,0,0,0,0 },
{ "NJ Lightning",0,39.361533,-75.053952,215,0,0,0,0 },
{ "NJ Thunderbolt",0,39.360727,-75.073626,301,0,0,0,0 },
{ "Nogaro",0,43.770693,-0.040576,324,0,0,0,0 },
{ "NOLA",0,29.885390,-90.199173,236,0,0,0,0 },
{ "North Florida",0,30.574890,-83.156411,56,0,0,0,0 },
{ "Nueve de Julio",0,-35.434895,-60.911237,144,0,0,0,0 },
{ "Nurburgring",0,50.335530,6.947828,225,0,0,0,0 },
{ "Okayama",0,34.914294,134.218942,356,0,0,0,0 },
{ "Oporto",0,41.173519,-8.685182,266,0,0,0,0 },
{ "Oran Park",0,-34.007854,150.733012,62,0,0,0,0 },
{ "Oregon Raceway",0,45.364339,-120.744582,0,0,0,0,0 },
{ "OReilly Indy",0,39.812513,-86.341836,179,0,0,0,1 },
{ "Oschersleben",0,52.027538,11.278134,288,0,0,0,0 },
{ "Oulton Park",0,53.180000,-2.612810,169,0,0,0,0 },
{ "Pacific",0,47.320661,-122.143744,276,0,0,0,0 },
{ "Palm Beach",0,26.920702,-80.305826,5,0,0,0,0 },
{ "Pau-Arnos",0,43.447105,-0.532606,153,0,0,0,0 },
{ "Paul Ricard",0,43.251171,5.793034,307,0,0,0,0 },
{ "Pedro Cofino",0,14.382467,-90.820621,65,0,0,0,0 },
{ "Pembrey",0,51.705867,-4.323844,30,0,0,0,0 },
{ "Phakisa",0,-27.904714,26.711055,175,0,0,0,0 },
{ "Phillip Island",0,-38.502368,145.232190,162,0,0,0,0 },
{ "Phoenix",0,33.375998,-112.311418,250,0,0,0,0 },
{ "Pikes Peak CCW",0,38.590954,-104.678303,148,0,0,0,0 },
{ "Pikes Peak CW",0,38.590954,-104.678303,334,0,0,0,0 },
{ "Pikes Peak Hill",0,38.921042,-105.037452,171,38.839780,-105.045265,91,2 },
{ "PittRace",0,40.850725,-80.347769,248,0,0,0,0 },
{ "Pocono",0,41.052625,-75.511769,112,0,0,0,1 },
{ "Pocono E",0,41.057027,-75.504476,323,0,0,0,0 },
{ "Pocono N",0,41.054636,-75.514705,25,0,0,0,0 },
{ "Pocono Road",0,41.052625,-75.511769,292,0,0,0,0 },
{ "Pocono S",0,41.051014,-75.504585,116,0,0,0,0 },
{ "Portland",0,45.594075,-122.690914,291,0,0,0,0 },
{ "Prince George",0,-33.049546,27.870029,23,0,0,0,0 },
{ "Puebla",0,19.027844,-97.989668,30,0,0,0,1 },
{ "Pueblo",0,38.302269,-104.678163,347,0,0,0,0 },
{ "Pukekohe Park",0,-37.217819,174.916677,152,0,0,0,0 },
{ "Putnam Park",0,39.582304,-86.742320,266,0,0,0,0 },
{ "Qatar F1",0,25.489183,51.449549,331,0,0,0,0 },
{ "Queensland",0,-27.690643,152.654634,181,0,0,0,0 },
{ "Race City",0,50.945636,-113.931286,189,0,0,0,0 },
{ "Red Bull Ring",0,47.219772,14.764295,254,0,0,0,0 },
{ "Richmond",0,37.591096,-77.419661,81,0,0,0,1 },
{ "Ridge",0,47.254611,-123.191236,92,0,0,0,0 },
{ "Rijeka",0,45.382752,14.509292,264,0,0,0,0 },
{ "Riverside Qu",0,46.631967,-71.787967,123,0,0,0,1 },
{ "Road America",0,43.797902,-87.989620,179,0,0,0,0 },
{ "Road America S",0,43.803101,-87.992479,179,0,0,0,0 },
{ "Road Atlanta",0,34.150383,-83.814232,121,0,0,0,0 },
{ "Rockingham SC",0,34.975861,-79.610898,266,0,0,0,1 },
{ "Rockingham UK",0,52.514749,-0.662326,161,0,0,0,0 },
{ "Roebling Road",0,32.167044,-81.322827,286,0,0,0,0 },
{ "Sachsenring",0,50.791713,12.688259,56,0,0,0,0 },
{ "Salzburgring",0,47.823073,13.169045,256,0,0,0,0 },
{ "Sanair",0,45.530166,-72.880930,227,0,0,0,1 },
{ "Sandia",0,35.020589,-106.858852,181,0,0,0,0 },
{ "Sandown",0,-37.949350,145.164346,177,0,0,0,0 },
{ "S'Arenal",0,39.494531,2.794972,287,0,0,0,0 },
{ "Schenley Park",0,40.433028,-79.935271,256,0,0,0,0 },
{ "Sears Point",0,38.161514,-122.454714,309,0,0,0,0 },
{ "Sears Point NA",0,38.161514,-122.454714,309,0,0,0,0 },
{ "Sebring",0,27.450219,-81.353728,90,0,0,0,0 },
{ "Sebring Short",0,27.456117,-81.353969,45,0,0,0,0 },
{ "Sendai",0,38.286094,140.621891,1,0,0,0,0 },
{ "Sentul",0,-6.536454,106.856353,339,0,0,0,0 },
{ "Sepang",0,2.760759,101.738377,266,0,0,0,0 },
{ "Shannonvlle Fab",0,44.225205,-77.159847,248,0,0,0,0 },
{ "Shannonvlle Lng",0,44.225205,-77.159847,248,0,0,0,0 },
{ "Shannonvlle Nel",0,44.225205,-77.159847,248,0,0,0,0 },
{ "Shannonvlle Pro",0,44.225205,-77.159847,248,0,0,0,0 },
{ "Silverstone",0,52.078696,-1.016977,84,0,0,0,0 },
{ "Slovakia",0,48.055222,17.569885,204,0,0,0,0 },
{ "Snetterton",0,52.463392,0.944754,266,0,0,0,0 },
{ "SpaFrancorchamp",0,50.444101,5.968328,140,0,0,0,0 },
{ "Spokane",0,47.662747,-117.570878,25,0,0,0,0 },
{ "Spokane Short",0,47.662747,-117.570878,25,0,0,0,0 },
{ "Spring Mtn 1.5",0,36.175210,-115.910590,96,0,0,0,0 },
{ "Spring Mtn 2.2",0,36.168650,-115.910015,320,0,0,0,0 },
{ "Spring Mtn 2.3",0,36.175210,-115.910590,96,0,0,0,0 },
{ "Spring Mtn 2.4",0,36.168650,-115.910015,320,0,0,0,0 },
{ "Spring Mtn 3.4",0,36.168650,-115.910015,320,0,0,0,0 },
{ "St-Eustac",0,45.573496,-73.965910,159,0,0,0,1 },
{ "Stratotech Park",0,53.736376,-113.233946,330,0,0,0,0 },
{ "StreetWillowCW",0,34.874802,-118.258735,288,0,0,0,0 },
{ "StreetWillowCCW",0,34.874802,-118.258735,108,0,0,0,0 },
{ "Sugo",0,38.142514,140.774603,98,0,0,0,0 },
{ "Summit Point",0,39.235195,-77.969069,199,0,0,0,0 },
{ "Summit Pt Jeff",0,39.234140,-77.972374,195,0,0,0,0 },
{ "Summit Pt Shen",0,39.241282,-77.979745,94,0,0,0,0 },
{ "Suzuka",0,34.843210,136.540444,140,0,0,0,0 },
{ "Symmons Plains",0,-41.657567,147.250019,170,0,0,0,0 },
{ "Talladega SS",0,33.563263,-86.069212,144,0,0,0,1 },
{ "TalladegaGP",0,33.504798,-86.001450,42,0,0,0,0 },
{ "TalladegaGP CCW",0,33.504798,-86.001450,238,0,0,0,0 },
{ "Taupo",0,-38.662685,176.146937,314,0,0,0,0 },
{ "Teretonga",0,-46.440927,168.263072,11,0,0,0,0 },
{ "Texas MS",0,33.037290,-97.284238,196,0,0,0,1 },
{ "Texas MS Road",0,33.037290,-97.284238,196,0,0,0,0 },
{ "Texas World",0,30.538007,-96.224427,193,0,0,0,0 },
{ "Thailand",0,13.910889,100.167664,139,0,0,0,0 },
{ "Thompson",0,41.980394,-71.828808,292,0,0,0,0 },
{ "Thruxton",0,51.207661,-1.609004,336,0,0,0,0 },
{ "Thunderhill",0,39.538461,-122.331165,180,0,0,0,0 },
{ "Thunderhill W",0,39.537514,-122.338788,255,0,0,0,0 },
{ "Tokachi",0,42.630634,143.292285,97,0,0,0,0 },
{ "Tonfanau",0,52.617999,-4.117282,62,0,0,0,0 },
{ "TopGearUKLaunch",0,51.118672,-0.536432,250,51.118672,-0.536432,250,2 },
{ "TopGearUKRoad",0,51.118672,-0.536432,250,0,0,0,0 },
{ "TopGearUSLaunch",0,33.670966,-117.734360,180,33.670975,-117.734341,180,2 },
{ "TopGearUSRoad",0,33.670975,-117.734341,180,0,0,0,0 },
{ "Toronto",0,42.902863,-79.857999,342,0,0,0,0 },
{ "Toyota Test",0,44.172441,142.245204,320,0,0,0,1 },
{ "Trappes StQuent",0,48.759621,1.993147,207,0,0,0,0 },
{ "Traqmate AutoX",0,34.102038,-84.233587,58,34.105293,-84.231776,209,2 },
{ "Traqmate DragE",0,34.102038,-84.233587,58,34.103577,-84.232364,18,3 },
{ "Traqmate DragQ",0,34.102038,-84.233587,58,34.106390,-84.229456,18,5 },
{ "Traqmate Test",0,34.106449,-84.236869,182,0,0,0,0 },
{ "Tsukuba",0,36.150082,139.919362,13,0,0,0,0 },
{ "Tsukuba 1000",0,36.150981,139.924699,198,0,0,0,0 },
{ "Tsukuba Short",0,36.150082,139.919362,13,0,0,0,0 },
{ "Val de Vienne",0,46.197259,0.635603,104,0,0,0,0 },
{ "Valencia",0,39.483568,-0.631065,55,0,0,0,0 },
{ "Vallelunga",0,42.157709,12.368996,351,0,0,0,0 },
{ "Varano",0,44.680698,10.020905,92,0,0,0,0 },
{ "VIR Full",0,36.568850,-79.206796,83,0,0,0,0 },
{ "VIR Grand",0,36.568850,-79.206796,83,0,0,0,0 },
{ "VIR North",0,36.568850,-79.206796,83,0,0,0,0 },
{ "VIR Patriot",0,36.560435,-79.206236,116,0,0,0,0 },
{ "VIR South",0,36.553991,-79.206655,329,0,0,0,0 },
{ "Wakefield Park",0,-34.840156,149.685156,18,0,0,0,0 },
{ "Waterford Hills",0,42.708549,-83.391247,172,0,0,0,0 },
{ "Watkins Glen L",0,42.340962,-76.928917,4,0,0,0,0 },
{ "Watkins Glen S",0,42.340962,-76.928917,4,0,0,0,0 },
{ "Weissach",0,48.843212,8.906844,319,0,0,0,0 },
{ "Willow Springs",0,34.871512,-118.263686,304,0,0,0,0 },
{ "Winton",0,-36.516879,146.084197,145,0,0,0,0 },
{ "YZ Circuit",0,35.436388,137.245886,341,0,0,0,0 },
{ "Zandvoort",0,52.388851,4.540760,22,0,0,0,0 },
{ "Zhuhai",0,22.366894,113.560098,161,0,0,0,0 },
{ "Zolder",0,50.989030,5.255724,238,0,0,0,0 },
{ "Zwartkops",0,-25.810021,28.112494,333,0,0,0,0 }
};

//
// Global Resources
//

//
// External data declarations
//
extern const char TRACK_PATH[];

//
// Global Data Area
//

//
// Local defines, constants and structure definitions
//
#define	SIGNIFICANT_HEADING_DIFF	1.00			// 1 degree
#define	SIGNIFICANT_LATLON_DIFF		0.0005			// under 10 meters

//
// Local function prototypes
//

//
// Local variable declarations
//

//
// creates the Track Table by copying the system tracks from the permanent copy and then
// reading the SD card to add user tracks.
// returns TRUE if successful
// returns FALSE if too many tracks or can't read SD card
//
int CreateTracks(void) {
	int trackIndex;
	
	// clear the table
	memset(trackTable, 0, sizeof(trackTable));
	
	// copy permanent system tracks over to working table
	for (trackIndex = 0; trackIndex < NUM_SYSTEM_TRACKS; trackIndex++)
		trackTable[trackIndex] = romTrackTable[trackIndex];

	// read user tracks into table	
	return ReadTracks();
} // CreateTracks	

//
// reads the SD card and adds those user tracks to the internal data structure
// returns TRUE if successful
// returns FALSE if too many tracks or can't read SD card
//
int ReadTracks(void) {
	long size;
	char trackFileName[NORMAL_FILENAME_SIZE];
	TRACK_DATA_STRUCTURE newTrack;
	int success = FALSE;
#ifdef _WINDOWS
//	WIN32_FIND_DATA fd;
//	HANDLE hFind;
	FILE *pFile;
	DIR *dir;
	struct dirent *ent;
#else
	char longFileName[MAX_FILENAME_SIZE];
	FS_FIND_DATA fd;
	FS_FILE *pFile;
#endif
			
	// determine how many tracks are preprogrammed
	for (numberOfTracks = 0; numberOfTracks < NUM_TOTAL_TRACKS; numberOfTracks++) {
		if ('\0' == trackTable[numberOfTracks].trackName[0])
			break;
	} // for
	
	size = sizeof(TRACK_DATA_STRUCTURE);

#ifdef _WINDOWS
	//if (INVALID_HANDLE_VALUE != (hFind = FindFirstFile(TRACK_PATH, &fd))) {
	//	do {
	//		if (numberOfTracks < NUM_TOTAL_TRACKS) {	// stop when full
	//			strcpy(trackFileName, fd.cFileName);
	//			if (ValidFileName(trackFileName)) {
	//				if (pFile = fopen(fd.cFileName, "rb")) {
	//					if (1 == fread(&newTrack, size, 1, pFile)) {
	//						if (success = verifyData(TRACK_DATA, &newTrack, false)) {

	//							// truncate filename to get track name
	//							trackFileName[strlen(trackFileName) - 4] = '\0';

	//							AddTrack(trackFileName, &newTrack);
	//						} // if	
	//					} // if
	//				} // if
	//				fclose(pFile);
	//			} // if
	//		} // if
	//	} while (FindNextFile (hFind, &fd));
	//} // if
	//FindClose(&fd);

	dir = opendir(TRACK_PATH);
	if (dir != NULL)
	{
		// find the selected session file
		while ((ent = readdir(dir)) != NULL)
		{
			if (ValidFileName(ent->d_name))
			{
				char fullFileName[NORMAL_FILENAME_SIZE];

				strcpy(trackFileName, ent->d_name);
				strcpy(fullFileName, TRACK_PATH);
				strcat(fullFileName, "\\");
				strcat(fullFileName, trackFileName);

				if (pFile = fopen(fullFileName, "rb")) {
					if (1 == fread(&newTrack, size, 1, pFile)) {
						if (success = verifyData(TRACK_DATA, &newTrack, false)) {

							// truncate filename to get track name
							trackFileName[strlen(trackFileName) - 4] = '\0';

							AddTrack(trackFileName, &newTrack);
						} // if	
					} // if
				} // if
				fclose(pFile);
			}
		}
		closedir(dir);
	}

#else
	if (0 == FS_FindFirstFile(&fd, TRACK_PATH, trackFileName, sizeof(trackFileName))) {
		do {
			if (numberOfTracks < NUM_TOTAL_TRACKS) {	// stop when full
				if (ValidFileName(trackFileName)) {
					// create full path
					strcpy(longFileName,TRACK_PATH);
					strcat(longFileName, "\\");
					strcat(longFileName, trackFileName);
				
					// Attempt to read SD card track file
					if (NULL != (pFile = FS_FOpen(longFileName, "rb"))) {
						// file opened - read the data and be sure it's the right version
						if (size == FS_Read(pFile, &newTrack, size)) {
							if (success = verifyData(TRACK_DATA, &newTrack, false)) {

								// truncate filename to get track name
								trackFileName[strlen(trackFileName) - 4] = '\0';

								AddTrack(trackFileName, &newTrack);
							} // if	
						} // if
					} // if
					FS_FClose(pFile);
				} // if
			} // if
		} while (FS_FindNextFile (&fd));
	} // if
	FS_FindClose(&fd);
#endif

	return success;
} // ReadTracks

//
// if user defines a new track, call me
//
// returns index of where item was inserted if successful, -1 otherwise
//
int AddTrack( char *addedName, TRACK_DATA_STRUCTURE *addedTrack) {
	BOOL trackStored = FALSE;
	int nameindex;
	int tempindex;
	int comparison;
	char tabletrack[NAMELEN+1];
	char newtrack[NAMELEN+1];
	
	// convert string to lowercase for comparison
	tempindex = -1;
	do {
		tempindex++;
		newtrack[tempindex] = tolower(addedName[tempindex]);
	} while ((tempindex < NAMELEN) && ('\0' != newtrack[tempindex]));
								
	// if duplicate track name, replace
	for (nameindex=0; !trackStored && nameindex < numberOfTracks; nameindex++) {
		// convert table name to lowercase for comparison
		tempindex = -1;
		do {
			tempindex++;
			tabletrack[tempindex] = tolower(trackTable[nameindex].trackName[tempindex]);
		} while ((tempindex < NAMELEN) && ('\0' != tabletrack[tempindex]));

		// find track in table greater than or equal in name to added track name
		if (0 <= (comparison = strncmp(tabletrack, newtrack, NAMELEN))) {
			
			if (0 != comparison) {	// greater than the name so insert here
				// make a slot for this track in the table unless it is going at the end
				if (nameindex != numberOfTracks) {
					for (tempindex=numberOfTracks; tempindex >= nameindex; tempindex--) {
						trackTable[tempindex+1] = trackTable[tempindex];
					} // for
				} // if

				strcpy (trackTable[nameindex].trackName, addedName);	// copy new name to new slot
				numberOfTracks++;
			} // if

			trackStored = TRUE;
			break;
		} // if
	} // for
	
	if (nameindex == numberOfTracks) {		// name goes at the end
		strcpy (trackTable[nameindex].trackName, addedName);	// copy new name to new slot
		numberOfTracks++;
		trackStored = TRUE;
	} // if
		
	if (trackStored) {
		// copy rest of information to new or existing slot
		trackTable[nameindex].sdCardEntry = 1;
		trackTable[nameindex].startLineLat = addedTrack->startLineLat;
		trackTable[nameindex].startLineLon = addedTrack->startLineLong;
		trackTable[nameindex].startLineHeading = addedTrack->startLineHeading;
		trackTable[nameindex].finishLineLat = addedTrack->finishLineLat;
		trackTable[nameindex].finishLineLon = addedTrack->finishLineLong;
		trackTable[nameindex].finishLineHeading = addedTrack->finishLineHeading;
		trackTable[nameindex].courseType = addedTrack->courseType;
			
		return nameindex;
	} // if
	else
		return -1;
} // AddTrack

//
// if user wants to eliminate a user track from table, call with track name
//
// returns TRUE if successful, FALSE otherwise
//
int DeleteTrack( char *deleteName) {
	BOOL trackDeleted = FALSE;
	int nameindex;
	int tempindex;
								
	// if duplicate track name, replace
	for (nameindex=0; !trackDeleted && nameindex<numberOfTracks; nameindex++) {
		// find track in table
		if (0 == strncmp(trackTable[nameindex].trackName, deleteName, NAMELEN)) {
			if (trackTable[nameindex].sdCardEntry) {	// only user tracks can be deleted
				// move all tracks down unless this is the last one
				if (nameindex != numberOfTracks) {
//KMC					for (tempindex=numberOfTracks; tempindex >= nameindex; tempindex--) {
//KMC						trackTable[tempindex] = trackTable[tempindex+1];
					tempindex = nameindex;
					while (tempindex < numberOfTracks) {
						trackTable[tempindex] = trackTable[tempindex+1];
						tempindex++;
					} // for
					memset(&trackTable[tempindex], 0, sizeof(TRACK_LIST_TYPE));		// null out last entry
				} // if
	
				trackDeleted = TRUE;
				numberOfTracks--;
			} // if
		} // if
	} // for
	return trackDeleted;
} // DeleteTrack

//
// returns TRUE if the current track is still within close range
// returns FALSE if the user needs to choose a new track
//
int CurrentTrackInRange( void ) {
	float fdeltalat;
	float fdeltalon;
	double distance;
	
	if (0.0 == trackData.startLineLat && 0.0 == trackData.startLineLong)
		return TRUE;
	
	// find difference in meters of lat, lon
	fdeltalat = (GetValue(LATITUDE).dVal - trackData.startLineLat) * LATMET;
	fdeltalon = (GetValue(LONGITUDE).dVal - trackData.startLineLong) * LONMET(trackData.startLineLat);

	// find the distance in meters
	distance = sqrt(fdeltalat * fdeltalat + fdeltalon * fdeltalon);
	
	if (distance < CURRENT_TRACK_M)
		return TRUE;
	else
		return FALSE;

}

//
// creates a list of the closest tracks to current location
//
//	args =	distkm = distance in kilometers to be considered a 'close track'
//			maxShortTracks = max number of tracks to return a list to
//			trackShortList = pointer to array of integers with length or numShortTracks
//
//	returns	number of tracks in list
//
typedef struct {
	int trackindex;
	double howfar;
} DISTANCE_TABLE;

#define LAT_CLOSE_DEG			(distkm * 1000.0 / LATMET)			// track is a candidate if latitude this close in degrees
#define LON_CLOSE_DEG			(distkm * 1000.0 / LONMET(45.0))	// track is a candidate if longitude this close in degrees

void GetClosestTracks( int distkm) {
	int trackIdx;
	float fdeltalat;
	float fdeltalon;
	double distance;
	BOOL noTrackSF;
	int distindex, moveindex;
	DISTANCE_TABLE distTable[NUM_TOTAL_TRACKS];
	
	memset(distTable, 0, sizeof(DISTANCE_TABLE) * NUM_TOTAL_TRACKS);
	numberOfShortListTracks = 0;
	
	// go down the list and find tracks with close lat and lon
	for (trackIdx=0; trackIdx < numberOfTracks; trackIdx++) {
		noTrackSF = (0.0 == trackTable[trackIdx].startLineLat && 0.0 == trackTable[trackIdx].startLineLon);
		
		fdeltalon = GetValue(LONGITUDE).dVal - trackTable[trackIdx].startLineLon;
		fdeltalon = FLOATABS(fdeltalon);
		if (noTrackSF || fdeltalon < LON_CLOSE_DEG) {
			fdeltalat = GetValue(LATITUDE).dVal - trackTable[trackIdx].startLineLat;
			fdeltalat = FLOATABS(fdeltalat);
			if (noTrackSF || fdeltalat < LAT_CLOSE_DEG) {
				
				if (noTrackSF) {		// always include uninitialized tracks in short list
					distance = 0.0;
				} // if
				else {
					// find the distances in meters
					fdeltalat = LATMET * fdeltalat;
					fdeltalon = LONMET(trackTable[trackIdx].startLineLat) * fdeltalon;
	
					distance = sqrt(fdeltalat * fdeltalat + fdeltalon * fdeltalon);
				} // if
	
				// if close enough store in list in order
				if (distance < (distkm * 1000.0)) {
					if (numberOfShortListTracks == 0) {
						distTable[0].howfar = distance;
						distTable[0].trackindex = trackIdx;
						numberOfShortListTracks = 1;
					} // if
					else {
						for (distindex=0; distindex < numberOfShortListTracks; distindex++) {
							if (distance < distTable[distindex].howfar) {
								// move all the tracks up the list to make room
								for (moveindex = NUM_TOTAL_TRACKS - 1; moveindex > distindex; moveindex--)
									distTable[moveindex] = distTable[moveindex-1];
							
								break;
							} // if
						} // for

						if (++numberOfShortListTracks > NUM_TOTAL_TRACKS)
							numberOfShortListTracks = NUM_TOTAL_TRACKS;
						else {
							// put track into created space or at end
							distTable[distindex].howfar = distance;
							distTable[distindex].trackindex = trackIdx;
						} // else
					} // else
				} // if
			} // if
		} // if
	} // for
	
	// copy working list over to official one
	for (distindex=0; distindex < NUM_TOTAL_TRACKS; distindex++)
		trackShortList[distindex] = distTable[distindex].trackindex;
	
} // GetClosestTracks

//
// UpdateSFLines - update start/finish lines
//
// Check each file on SD card - if this track exists in the ROM list
// then check the SF coordinates - if different delete the SD card track
//
#ifdef _WINDOWS
void UpdateSFLines(void)
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;

	if (SetCurrentDirectory(TRACK_PATH))
	{
		if ((fh = FindFirstFile("*.dat", &FindFileData)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				int trackIndex;
				char tempName[MAX_FILENAME_SIZE];

				// trim file extension off name
				strcpy(tempName, FindFileData.cFileName);
				tempName[strlen(tempName) - 4] = 0;

				// See if this file is in the ROM table
				for (trackIndex = 0; trackIndex < NUM_SYSTEM_TRACKS; trackIndex++)
				{
					if (strcmp(tempName, romTrackTable[trackIndex].trackName) == 0)
					{
						HFILE hFile;
						OFSTRUCT fStruct;

						if ((hFile = OpenFile(FindFileData.cFileName, &fStruct, OF_READ)) != HFILE_ERROR)
						{
							float headDiff, latDiff, longDiff;
							int sfDifferent;
							DWORD bytesRead;
							TRACK_DATA_STRUCTURE trackData;

							// Read data and check SF value
							if (ReadFile((HANDLE)hFile, &trackData, sizeof(TRACK_DATA_STRUCTURE), &bytesRead, NULL) == TRUE)
							{
								// Check start line (finish line is not in rom table)
								// Note look for significant differences beyond what might have been done
								// by simply looking at the value in the numeric editor and saving the value
								headDiff = fabs(romTrackTable[trackIndex].startLineHeading - trackData.startLineHeading);
								latDiff = fabs(romTrackTable[trackIndex].startLineLat - trackData.startLineLat);
								longDiff = fabs(romTrackTable[trackIndex].startLineLon - trackData.startLineLong);

								if ((headDiff > SIGNIFICANT_HEADING_DIFF) ||
									(latDiff  > SIGNIFICANT_LATLON_DIFF)  ||
									(longDiff > SIGNIFICANT_LATLON_DIFF))

									sfDifferent = true;
								else sfDifferent = false;
							}
							else sfDifferent = false;

							CloseHandle((HANDLE)hFile);

							// Delete file if start finish different
							if (sfDifferent)
								DeleteFile(FindFileData.cFileName);
						}
						break;		// exit 'for' loop
					}
				}
			}
			while (FindNextFile(fh, &FindFileData) != 0);
		}
		FindClose(fh);
	}
	SetCurrentDirectory("..\\..");
}
#else
// Target hardware version
void UpdateSFLines(void)
{
	FS_DIR *pDir;
	FS_FIND_DATA FindFileData;

	if ((pDir = FS_OpenDir(TRACK_PATH)) != 0)
	{
		if (FS_FindFirstFile(&FindFileData, TRACK_PATH, "*.dat", MAX_FILENAME_SIZE) == 0)
		{
			do
			{
				int trackIndex;
				char tempName[MAX_FILENAME_SIZE];

				// Be sure this is a valid file name
				if (strlen(FindFileData.sFileName) > 3)
				{
					// trim file extension off name
					strcpy(tempName, FindFileData.sFileName);
					tempName[strlen(tempName) - 4] = 0;
	
					// See if this file is in the ROM table
					for (trackIndex = 0; trackIndex < NUM_SYSTEM_TRACKS; trackIndex++)
					{
						if (strcmp(tempName, romTrackTable[trackIndex].trackName) == 0)
						{
							FS_FILE *pFile;
							char longFileName[MAX_FILENAME_SIZE];
	
							// create full path
							strcpy(longFileName, TRACK_PATH);
							strcat(longFileName, "\\");
							strcat(longFileName, FindFileData.sFileName);
					
							if ((pFile = FS_FOpen(longFileName, "r")) != NULL)
							{
								float headDiff, latDiff, longDiff;
								int sfDifferent;
								TRACK_DATA_STRUCTURE trackData;
	
								// Read data and check SF value
								if (FS_FRead(&trackData, sizeof(TRACK_DATA_STRUCTURE), 1, pFile) != 0)
								{
									// Check start line (finish line is not in rom table)
									// Note look for significant differences beyond what might have been done
									// by simply looking at the value in the numeric editor and saving the value
									headDiff = fabs(romTrackTable[trackIndex].startLineHeading - trackData.startLineHeading);
									latDiff = fabs(romTrackTable[trackIndex].startLineLat - trackData.startLineLat);
									longDiff = fabs(romTrackTable[trackIndex].startLineLon - trackData.startLineLong);
	
									if ((headDiff > SIGNIFICANT_HEADING_DIFF) ||
										(latDiff  > SIGNIFICANT_LATLON_DIFF)  ||
										(longDiff > SIGNIFICANT_LATLON_DIFF))
	
										sfDifferent = true;
									else sfDifferent = false;
								}
								else sfDifferent = false;
	
								FS_FClose(pFile);
	
								// Delete file if start finish different
								if (sfDifferent)
									FS_Remove(longFileName);
							}
							break;		// exit 'for' loop
						}
					}
				}
			}
			while (FS_FindNextFile(&FindFileData) != 0);
		}
		FS_FindClose(&FindFileData);
		FS_CloseDir(pDir);
	}
}
#endif
