//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_TRACKER_H
#define STARTRACK_TRACKER_H

#include "novas.h"
#include "catalog.h"
#include <time.h>
#include <sys/time.h>

#define SECONDS_IN_DAY 86400.0
#define DELTA_TT 32.184
#define REDUCED_ACCURACY 1

typedef struct {
    double date;       // julian days; the number of days since noon, January 1, 4713 BC // Which Joseph Justice Scaliger estimated to be the beginning of history.
    double ut1_utc;    // current observed discrepancy between earth's non-uniform rotation and Universal Coordinated Time
    double leap_secs;  // Number of leap seconds added in International Atomic Time
    on_surface site;   // geodetic location
    object earth;      // location in the solar system
    Entry* target;      // the current target of the tracker, extended from novas cat_entry;
} Tracker;

int create( Tracker* tracker, double ut1_utc, double leap_secs );
//void free_tracker( Tracker* tracker );

void setTime( Tracker* tracker, double seconds ); // set UTC
double getTT( Tracker *map );
double getUT1( Tracker *map );
double getUTC( Tracker *tracker );
double getDeltaT( Tracker *tracker );

void setCoordinates( Tracker* tracker, double latitude, double longitude, double height );
void setAtmosphere( Tracker* tracker, double temperature, double pressure );
on_surface getLocation( Tracker* tracker );

void setTarget( Tracker* tracker, Entry* entry );
int local(Tracker *tracker, double *zenith_distance, double *topocentric_azimuth);
int zenith( Tracker* tracker, double* right_ascension, double* declination );

void print_time( Tracker* tracker );
void print_site( Tracker* tracker );

#endif //STARTRACK_TRACKER_H
