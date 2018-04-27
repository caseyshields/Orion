/* StarPlot.h
*/

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef STARMAP_H_
#define STARMAP_H_

#include "novas.h"
#include <time.h>

#define SECONDS_IN_DAY 86400.0
#define DELTA_TT 32.184

//Structure for holding the topocentric coordinates of stars
typedef struct {
	cat_entry entry;   // the corresponding catalog entry
	float v_mag; // visual magnitude of the star

	// TODO use the novas structure sky_pos?
	double position[3]; // a graphical position //TODO remove
	double right_ascension;// coordinate
	double declination;// coordinate

	double angle;      // used to measure distance from query points
} Star;

// represents a section of the sky
typedef struct {
	double right_ascension;// coordinate of aperture
	double declination;// coordinate of aperture
	double radius;     // radius of the aperture in degrees
	int count;         // number of stars inside the aperture
	Star** inside;     // stars inside the aperture
} Aperture;

// transforms a catalog of stars to topocentric coordinates
typedef struct {
    double date;       // julian day
    double ut1_utc;    //
    double leap_secs;  //
	on_surface site;   // geodetic location
	object earth;      // location in the solar system
	Aperture aperture; // current view of the sky TODO allow multiple apertures
	float visibleMag;  // minimum brightness visible
	int starcount;     // number of stars
	Star *stars;       // transformed catalog entries
	// star** index; // TODO name, number and position indices
} StarMap;

int StarMap_create( StarMap* map, struct tm* utc, double ut1_utc, double leap_secs );
void StarMap_setTime( StarMap* map, struct tm* calendar );
void StarMap_setSite( StarMap* map, on_surface* site );
void StarMap_setAperture( StarMap* map, double ra, double de, double ap );
void StarMap_update( StarMap* map );
void StarMap_printTime( StarMap * map );
void StarMap_printSite( StarMap * map );
void StarMap_printStar( Star * star );
void StarMap_printMap( StarMap * map );

//void StarMap_test(StarMap *map);
//Star* StarMap_closest(double ra, double de, StarMap *map);
//StarMap * StarMap_inside_aperture( double ra, double de, double aperture, StarMap *map);
void StarMap_destroy(StarMap *map);

#endif
