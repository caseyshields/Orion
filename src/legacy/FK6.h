
#ifndef FK6_H_
#define FK6_H_

#include <stdio.h>
#include "StarMap.h"

/*// Reads FK6 stars into the novas structure:
typedef struct {
	char starname[51];
	char catalog[4];
	long int starnumber;
	double ra;
	double dec;
	double promora;
	double promodec;
	double parallax;
	double radialvelocity;
} cat_entry;

where:
	starname[51] = name of celestial object
	catalog[4] = 3-character catalog designator
	starnumber = integer identifier assigned to object
	ra = ICRS right ascension (hours)
	dec = ICRS declination (degrees)
	promora = ICRS proper motion in right ascension
	(milliarcseconds/year)
	promodec = ICRS proper motion in declination
	(milliarcseconds/year)
	parallax = parallax (milliarcseconds)
	radialvelocity = radial velocity (km/s)
*/

void FK6_loadMap(int entries, FILE* f, StarMap *map);

#endif
