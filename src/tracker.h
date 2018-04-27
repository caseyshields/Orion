//
// Created by Casey Shields on 4/27/2018.
//


#ifndef STARTRACK_TRACKER_H
#define STARTRACK_TRACKER_H

#include "novas.h"
#include <time.h>

#define SECONDS_IN_DAY 86400.0
#define DELTA_TT 32.184

typedef struct {
    double date;       // julian days; the number of days since noon, January 1, 4713 BC //Which Joseph Justice Scaliger estimated to be the beginning of recorded time.
    double ut1_utc;    // current observed discrepancy between earth's non-uniform rotation and Universal Coordinated Time
    double leap_secs;  // Number of leap seconds added in International Atomic Time
    on_surface site;   // geodetic location
    object earth;      // location in the solar system
} Tracker;

int create( Tracker* tracker, double ut1_utc, double leap_secs );

void setTime( Tracker* tracker, struct tm* calendar );
double getTT( Tracker *map );
double getUT1( Tracker *map );
double getUTC( Tracker *tracker );

void setCoordinates( Tracker* tracker, double latitude, double longitude, double height );
void setAtmosphere( Tracker* tracker, double temperature, double pressure );
//double getLatitude( Tracker* tracker );
//double getLongitude( Tracker* tracker );
//double getHeight( Tracker* tracker );
//double getTemperature( Tracker* tracker );
//double getPressure( Tracker* tracker );


#endif //STARTRACK_TRACKER_H
