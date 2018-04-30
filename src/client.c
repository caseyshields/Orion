//
// Created by Casey Shields on 4/27/2018.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "novas.h"
#include "tracker.h"
#include "catalog.h"

int main (void) {

    // this is the current offset between atomic clock time and time derived from Earth's orientation
    double ut1_utc = 0.108644; // (UT1-UTC) obtained from IERS Apr 26 2018

    // Difference between TAI and UTC, an integral number of leap second
    double leap_secs = 37.000000; // delta AT, obtained from IERS Apr 26 2018

    // create the tracker
    Tracker tracker;
    create(&tracker, ut1_utc, leap_secs );

    // set the tracker's time in UTC
    time_t unix_time; // TODO we should get a higher resolution timestamp, possibly from NTP?
    time( &unix_time ); // GMT seconds since January 1970 0:00
    struct tm *utc = gmtime( &unix_time );
    setTime(&tracker, utc);

    // set the location
    setCoordinates( &tracker, 38.88972222222222, -77.0075, 125.0 );
    setAtmosphere( &tracker, 10.0, 1010.0);

    print_time( &tracker );
    print_site( &tracker );

    // create and load a catalog
    FILE *file = fopen("../data/FK6.txt", "r");
    Catalog catalog;
    init( &catalog, 878 );
    load( &catalog, file );
//    print_catalog( &catalog );

    // track every star in the FK6 catalog
    double latitude, longitude;
    for( int n=0; n<catalog.size; n++ ) {
        Entry* entry = &catalog.stars[n];
        setTarget( &tracker, entry );
        getTopocentric(&tracker, &latitude, &longitude);
        print_entry( entry );
        printf( "appears at(%f, %f)\n\n", longitude, latitude );
    }
}