
#include "StarMap.h"
#include "Heap.h"
#include "FK6.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "novas.h"

void StarMap_test(StarMap *map);

//double dt;         // Terrestrial time offset TT-UT1 = 32.184+ dAT -(UT1-UTC)

/**
 * jd_utc : julian date in days
 * ut1_utc : current difference between UT1 and UTC time
 * leap_secs: current number of leap seconds in TAI*/
int StarMap_create(StarMap* map, struct tm* utc, double ut1_utc, double leap_secs ) {

    map->starcount = 0;
    map->stars = 0;

    map->ut1_utc = ut1_utc;
    map->leap_secs = leap_secs;
    //map->delta_t = 32.184 + map->leap_secs - map->ut1_utc; // Novas typically deals with the sum of the offsets, might want to cache it...
	StarMap_setTime( map, utc );

	// set default site //38 53'23"N , 77 00'27"W;
	make_on_surface(38.88972222222222, -77.0075, 125.0, 10.0, 1010.0, &(map->site));
	//site_info capitol = {38.88972222222222, -77.0075, 125.0, 10.0, 1010.0};
	//on_surface geo_loc = {45.0, -75.0, 0.0, 10.0, 1010.0};
	//StarMap_setSite(map, &capitol);

	// set default planet
	return make_object (0, 2, "Earth", (cat_entry*)NULL, &(map->earth) );
}

void StarMap_setTime(StarMap* map, struct tm* utc) {
    // convert it to a julian date, which is days since noon, Jan 1, 4713 BC
    map->date = julian_date(
                (short) (utc->tm_year + 1900),
                (short) (utc->tm_mon + 1),
                (short) utc->tm_mday,
                ((double)utc->tm_hour)
                +((double)utc->tm_min)/60.0
                + (double) utc->tm_sec / 3600.0
        );
}

/** Returns terrestrial time in julian days.
 * TT = UTC + leap_seconds + 32.184. */
double StarMap_getTT(StarMap *map) {
    return map->date + (map->leap_secs + DELTA_TT) / SECONDS_IN_DAY;
}

/** Returns UT1, a time scale which depends on the non-uniform rotation of the earth.
 * Derived by adding an empirically determined offset to UTC
 * */
double Starmap_getUT1( StarMap *map ) {
    return map->date + map->ut1_utc / SECONDS_IN_DAY;
}

void StarMap_setSite(StarMap* map, on_surface* site) {
	map->site.longitude = site->longitude;
	map->site.latitude = site->latitude;
	map->site.height = site->height;
	map->site.pressure = site->pressure;
	map->site.temperature = site->temperature;
}
void StarMap_setAperture(StarMap* map, double ra, double de, double ap) {
	map->aperture.right_ascension = ra;
	map->aperture.declination = de;
	map->aperture.radius = ap;
}

void StarMap_update(StarMap * map) {
	Star * star;
	short int error = 0;
	int i;
	// transcribe the catalog data
	for(i=0; i<map->starcount; i++) {
		star = &(map->stars[i]);
		error = topo_star(
				map->date,
				60.0,//66.2,//deltat,
		        &(star->entry),//(cat_entry*)
		        &(map->site),
		        1,//0,//accuracy mode
		        &(star->right_ascension),
		        &(star->declination));
		if(error) {
			printf("Error in starmap update: %hi", error);
			return;
		}
		radec2vector(star->right_ascension, star->declination, 1.0, star->position);
	}

	//StarMap_test(map);
	//deltat (double)
	//        Difference TT (or TDT)-UT1 at 'tjd', in seconds.
}

Star* StarMap_closest(double ra, double de, StarMap *map) {
	int i;
	double azimuth, declination;
	double x,y,z,t;
	double maxcos;
	int max;

	azimuth = 2.0*PI*ra / 24.0;
	declination = PI*de/180.0;
	z = sin(declination);
	t = cos(declination);
	x = cos(azimuth)*t;
	y = sin(azimuth)*t;

	maxcos=-1.0;
	max=0;
	for(i=0; i<map->starcount; i++) {
		t = x*map->stars[i].position[0] +
			y*map->stars[i].position[1] +
			z*map->stars[i].position[2];
		if(t>maxcos) {
			max = i;
			maxcos = t;
		}
	}

	return &(map->stars[max]);
}

// heap function
int compare(Star *a, Star *b) {
	double d = a->angle - b->angle;
	if(d<0) return -1;
	if(d>0) return 1;
	else return 0;
}

Aperture * StarMap_inside_aperture(
		double ra, double de, double aperture,
		StarMap *map) {
	double azimuth, declination;
	double x,y,z,t;
	double mincos;
	StarMap *subset; // will hold all stars inside aperture, ordered by angular offset from heading
	Star *star;
	Heap *heap;
	int i;

	// create heap
	heap = Heap_create(10, compare,0);
	assert(heap);

	// find the cosine of the aperture angle
	mincos = cos(PI*aperture/180.0);

	// convert angles to 3-vector
	azimuth = 2.0*PI*ra / 24.0;
	declination = PI*de/180.0;
	z = sin(declination);
	t = cos(declination);
	x = cos(azimuth)*t;
	y = sin(azimuth)*t;

	// add stars within aperture to heap
	for(i=0; i<map->starcount; i++) {
		star = &(map->stars[i]);
		star->angle = x*star->position[0] +
			y*star->position[1] +
			z*star->position[2];
		if(star->angle >= mincos) {
			Heap_push(star, heap);
			//printf("\tstar %li angle %lf\n",star->entry->starnumber, 180*acos(star->angle)/PI );
		}
	}

	//create new StarPlot and fill it with contents of the heap
	subset = malloc(sizeof(StarMap));
	if(!subset) {
		Heap_destroy(heap);
		return NULL;
	}
	subset->starcount = heap->size;
	subset->stars = calloc(subset->starcount, sizeof(Star*));
	if(!subset->stars) {
		Heap_destroy(heap);
		free(subset);
		return NULL;
	}

	for(i=0; i<subset->starcount; i++) {
		/*star = (Star*)Heap_pull(heap);
		printf("\tstar %li angle %lf\n",
				star->entry->starnumber,
				180*acos(star->angle)/PI );*/

		//subset->stars[i] = *Heap_pull(heap);
	}
	//printf("Stars found: %i\n", subset->length);
	/*for(i=0; i<subset->length; i++) {
		printf("\tstar %li angle %lf\n",
				subset->stars[i]->entry->starnumber,
				180*acos(subset->stars[i]->angle)/PI );
	}*/

	Heap_destroy(heap);
	return NULL;//subset;
}

void StarMap_test(StarMap *map) {
	int i;
	Star *star;
	double ra, dec;//, pos[3];
	for(i=0 ; i<map->starcount; i++) {
		star = &(map->stars[i]);
		ra = star->entry.ra;
		dec = star->entry.dec;
		radec2vector(ra, dec, 1.0, star->position);
		printf("[%1.4f, %1.4f, %1.4f]\n",
				star->position[0],// - pos[0],
				star->position[1],// - pos[1],
				star->position[2]);// - pos[2]);
	}
}

void StarMap_printTime(StarMap * map) {
	short int year, month, day;
	double hour;
	cal_date (map->date, &year, &month, &day, &hour);
	printf("\ntime : %hi/%hi/%hi %f\n", year, month, day, hour);
	fflush(0);
}

void StarMap_printSite(StarMap * map) {
	printf("latitude:\t%f\n", map->site.latitude);
	printf("longitude:\t%f\n", map->site.longitude);
	printf("elevation:\t%f\n", map->site.height);
	printf("temperature:\t%fC\n", map->site.temperature);
	printf("pressure:\t%f millibars\n", map->site.pressure);
	printf("aperture: (asc:%f, dec:%f rad:%f)\n", map->aperture.right_ascension, map->aperture.declination, map->aperture.radius);
	fflush(0);
}

void StarMap_printMap(StarMap *map) {
	int n;
	Star *star;
	for(n=0; n<map->starcount; n++) {
		star = &map->stars[n];
		StarMap_printStar(star);
	}
	fflush(0);
}

void StarMap_printStar(Star *star) {
	printf("%s.%li: %s (ra:%lf, dec:%lf, p:%lf, v=%lf)\n",
			star->entry.catalog,
			star->entry.starnumber,
			star->entry.starname,
			star->entry.ra,
			star->entry.dec,
			star->entry.parallax,
			star->v_mag);
}

void StarMap_destroy(StarMap *map) {
	/*int i;
	for(i=0; i<map->numStar; i++) {
		free(map->stars[i]);
	}*/
	free(map->stars);
	free(map);
}
