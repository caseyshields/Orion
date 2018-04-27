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

void setCoordinates( Tracker* tracker, double latitude, double longitude, double height ) {
    tracker->site.latitude = latitude;
    tracker->site.longitude = longitude;
    tracker->site.height = height;
}
void setAtmosphere( Tracker* tracker, double temperature, double pressure ) {
    tracker->site.pressure = pressure;
    tracker->site.temperature = temperature;
}
//double getLatitude( Tracker* tracker ) { return tracker->site.latitude; }
//double getLongitude( Tracker* tracker ) { return tracker->site.longitude; }
//double getHeight( Tracker* tracker ) { return tracker->site.height; }
//double getTemperature( Tracker* tracker ) { return tracker->site.temperature; }
//double getPressure( Tracker* tracker ) { return tracker->site.pressure; }