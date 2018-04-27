//
// Created by Casey Shields on 4/27/2018.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "novas.h"
#include "tracker.h"

int main (void) {

    // this is the current offset between atomic clock time and time derived from Earth's orientation
    double ut1_utc = 0.108644; // (UT1-UTC) obtained from IERS Apr 26 2018

    // Difference between TAI and UTC, an integral number of leap second
    double leap_secs = 37.000000; // delta AT, obtained from IERS Apr 26 2018


    // TODO we should get a higher resolution timestamp, possibly from NTP?

    // create the tracker
    Tracker tracker;
    create(&tracker, ut1_utc, leap_secs );

    // set the tracker's time in UTC
    time_t unix_time;
    time( &unix_time ); // GMT seconds since January 1970 0:00
    struct tm *utc = gmtime( &unix_time );
    setTime(&tracker, utc);

    // set the location
    setCoordinates( &tracker, 38.88972222222222, -77.0075, 125.0 );
    setAtmosphere( &tracker, 10.0, 1010.0);

}