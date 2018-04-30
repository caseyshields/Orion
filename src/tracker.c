//
// Created by Casey Shields on 4/27/2018.
//

#include <novasc3.1/novas.h>
#include "tracker.h"

/**
 * jd_utc : julian date in days
 * ut1_utc : current difference between UT1 and UTC time
 * leap_secs: current number of leap seconds in TAI*/
int create(Tracker* tracker, double ut1_utc, double leap_secs ) {

    tracker->ut1_utc = ut1_utc;
    tracker->leap_secs = leap_secs;

    // Novas typically deals with the sum of the offsets, might want to cache it...
    //map->delta_t = 32.184 + map->leap_secs - map->ut1_utc;

    // create null novas surface location
    make_on_surface( 0.0, 0.0, 0.0, 10.0, 1010.0, &(tracker->site) );

    // create an earth to put the tracker on
    return make_object (0, 2, "Earth", (cat_entry*)NULL, &(tracker->earth) );
}

void setTime(Tracker* tracker, struct tm* utc) {
    // convert it to a julian date, which is days since noon, Jan 1, 4713 BC
    tracker->date = julian_date(
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
double getTT( Tracker *map ) { return map->date + (map->leap_secs + DELTA_TT) / SECONDS_IN_DAY; }

/** Returns UT1, a time scale which depends on the non-uniform rotation of the earth.
 * Derived by adding an empirically determined offset to UTC */
double getUT1( Tracker *map ) { return map->date + map->ut1_utc / SECONDS_IN_DAY; }

/** Returns the Universal Coordinated Time in Julian days. */
double getUTC( Tracker *tracker ) { return tracker->date; }

double getDeltaT( Tracker *tracker ) { return 32.184 + tracker->leap_secs - tracker->ut1_utc; }

void setCoordinates( Tracker* tracker, double latitude, double longitude, double height ) {
    tracker->site.latitude = latitude;
    tracker->site.longitude = longitude;
    tracker->site.height = height;
}
void setAtmosphere( Tracker* tracker, double temperature, double pressure ) {
    tracker->site.pressure = pressure;
    tracker->site.temperature = temperature;
}

//on_surface getLocation( Tracker* tracker ) { return tracker->site; }

void setTarget( Tracker* tracker, Entry* entry ) {
    tracker->target = entry;
}

int getTopocentric(Tracker* tracker, double *latitude, double *longitude) {
    short int error;
    error = topo_star(
                tracker->date,
                getDeltaT( tracker ),
                (cat_entry*) tracker->target,
                &tracker->site,
                REDUCED_ACCURACY,
                latitude,
                longitude
        );
    return error;
}

void print_time( Tracker* tracker ) {
    short int year, month, day;
    double hour;

    cal_date( getUTC(tracker), &year, &month, &day, &hour );
    printf( "UTC : %hi/%hi/%hi %f\n", year, month, day, hour );

    cal_date( getUT1(tracker), &year, &month, &day, &hour );
    printf( "UT1 : %hi/%hi/%hi %f\n", year, month, day, hour );

    cal_date( getTT(tracker), &year, &month, &day, &hour );
    printf( "TT  : %hi/%hi/%hi %f\n\n", year, month, day, hour );

    fflush(0);
}

void print_site( Tracker* tracker ) {
    printf("latitude:\t%f hours\n", tracker->site.latitude);
    printf("longitude:\t%f degrees\n", tracker->site.longitude);
    printf("elevation:\t%f meters\n", tracker->site.height);
    printf("temperature:\t%f Celsius\n", tracker->site.temperature);
    printf("pressure:\t%f millibars\n\n", tracker->site.pressure);
    //Aperture* aperture = tracker->aperture;
    //printf("aperture: (asc:%f, dec:%f rad:%f)\n", aperture.right_ascension, aperture.declination, aperture.radius);
    fflush(0);
}
