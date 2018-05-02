//
// Created by Casey Shields on 4/27/2018.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "novas.h"
#include "tracker.h"
#include "catalog.h"

double getTime( ) {
    time_t coarse;
    time( &coarse );

    struct timeval time;
    gettimeofday( &time, NULL );

    printf( "iso: %d\ngnu: %d\n", (long)coarse, (long)time.tv_sec );

    return time.tv_sec + (double) (time.tv_usec / 1000000.0);
}

int main (void) {

    const int SIZE = 878;
    const int TRIALS = 100;

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
    setTimeCoarse(&tracker, utc);
    print_time( &tracker );

    // set the tracker's time in UTC
    setTime( &tracker, getTime() );
    print_time( &tracker );

    // set the location
    setCoordinates( &tracker, 38.88972222222222, -77.0075, 125.0 );
    setAtmosphere( &tracker, 10.0, 1010.0);
    print_site( &tracker );

    // create and load a catalog
    FILE *file = fopen("../data/FK6.txt", "r");
    Catalog catalog;
    init( &catalog, SIZE );
    load( &catalog, file );
//    print_catalog( &catalog );

    // start the timer
    double start = getTime();
//    clock_t start = clock();

    // track every star in the FK6 catalog
    double tracks [SIZE][2]; //double latitude, longitude;
    for( int t=0; t< TRIALS; t++ ) {
        for (int n = 0; n < catalog.size; n++) {
            Entry *entry = &catalog.stars[n];
            setTarget(&tracker, entry);
            getTopocentric(&tracker, &tracks[n][0], &tracks[n][1]);
        }
    }

    // get the time
//    clock_t end = clock();
//    double duration = (double)(end-start)/CLOCKS_PER_SEC;
    double end = getTime();
    double duration = end - start;
    printf( "time: %lf\nspeed: %lf\n", duration, duration/(TRIALS*SIZE) );

    // print the catalog with correspnding tracks
    for( int n=0; n<catalog.size; n++ ) {
        Entry* entry = &catalog.stars[n];
        print_entry( entry );
        printf( "appears at(%f, %f)\n\n", tracks[n][0], tracks[n][1] );
    }
}