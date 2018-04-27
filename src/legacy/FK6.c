/* fk6.c
 *	Loads the FK6 catalog from a text file lifted from their site.
 *	I should really contact them for the raw data. But this will work for a
 *	mock up.
 *      Author: Casey Shields
 */
#include "FK6.h"
#include "StarMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

void FK6_print_entry(Star *star);

void FK6_loadMap(int entries, FILE* f, StarMap * map) {
	Star * star;
	char buf[1024], *s;
	double hour, min, sec;
	double deg, arcmin, arcsec;
	int n=0;

	if(!map) return;

	// initialize catalog
	map->starcount = entries;
	map->stars = calloc(entries, sizeof(Star));
	if(!map->stars){
		free(map);
		return;
	}

	// parse FK6 entries from file
	while(n<10) { // skip first ten lines
		s = fgets(buf, 1024, f);
		n++;
	}
	n=0;
	do {
		// get a line from the catalog
		s = fgets(buf, 1024, f);
		if(!s) {
			map->starcount = n-1;
			break;
		}

		star = &(map->stars[n]);

		// uhh, I might want to make a smarter parser...
		sscanf(buf+1, " %li ", &(star->entry.starnumber)); // col 1
		sscanf(buf+17, " %19c ", star->entry.starname); // col 3
		strcpy(star->entry.catalog, "FK6"); // FK6
		sscanf(buf+38, " %lf %lf %lf ", &hour, &min, &sec); // col 4
		sscanf(buf+59, " %lf %lf %lf ", &deg, &arcmin, &arcsec); // col 5
		sscanf(buf+77, " %lf ", &(star->entry.promora)); // col 6?
		sscanf(buf+89, " %lf ", &(star->entry.promodec)); // col 7?
		sscanf(buf+153, " %lf ", &(star->entry.parallax)); // col 14
		sscanf(buf+179, " %lf ", &(star->entry.radialvelocity)); // col
		sscanf(buf+186, " %f ", &(star->v_mag)); // col 18

		// combine hours minutes and seconds
		star->entry.ra = hour+(min/60.0)+(sec/3600.0);
		if(buf[58]=='+')
			star->entry.dec = deg+(arcmin/60.0)+(arcsec/3600.0);
		else
			star->entry.dec = -deg-(arcmin/60.0)-(arcsec/3600.0);

		if(buf[76]=='-')
			star->entry.promora*=-1.0;
		if(buf[88]=='-')
			star->entry.promodec*=-1.0;
		if(buf[178]=='-')
			star->entry.radialvelocity*=-1.0;

		// null and default the non catalog fields
		star->angle=0.0;
		star->position[0] = 0.0;
		star->position[1] = 0.0;
		star->position[2] = 0.0;
		star->right_ascension = 0.0;
		star->declination = 0.0;

		// debug
		//StarMap_print_star(star);

		n++;
	} while(1);
	return;// map;
}

// faster parsing than scanf...
void split(char* line, int n, char* values[]) {
	int i=0, c=0;
	while(c<n) {
		values[c] = &line[i];
		while(line[i]!='|')
			i++;
		line[i]='\0';
		c++;
		i++;
	}
}
char * advance(char * s, int columns) {
	int i=0, c=0;

	while(c<columns) {
		if(s[i] == '|')
			c++;
		i++;
	}
	return &s[i];
}
