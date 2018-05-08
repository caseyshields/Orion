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
#include <unistd.h>
#include <stdbool.h>
//#include <sys/thread.h>

double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

char* get_arg( int argc, char *argv[], char *name ) {
    for( int n=0; n<argv; n++ )
        if( !strcmp(name, argv[n]) )
            return argv[n+1];
    return NULL;
}

int XXmain (void) {
    // create and load a catalog
    FILE *file = fopen("../data/FK6.txt", "r");
    Catalog catalog;
    init( &catalog, 64 );
    load( &catalog, file );

    print_catalog( &catalog );

    freeCatalog( &catalog, 1 );
}

int main( int argc, char *argv[] ) {
    char *arg;
    const int SIZE = 878;
    const int TRIALS = 100;

    // geodetic coordinates in degrees
    double latitude = 38.88972222222222;
    arg = get_arg( argc, argv, "-latitude" );
    if( arg ) latitude = atof( arg );

    double longitude = -77.0075;
    arg = get_arg( argc, argv, "-longitude" );
    if( arg ) longitude = atof( arg );

    // geodetic height in meters
    double height = 125.0;
    arg = get_arg( argc, argv, "-height" );
    if( arg ) height = atof( arg );

    // site temperature in degrees celsius
    double celsius = 10.0;
    arg = get_arg( argc, argv, "celsius" );
    if( arg ) celsius = atof( arg );

    // atmospheric pressure at site in millibars
    double millibars = 1010.0;
    arg = get_arg( argc, argv, "millibars" );
    if( arg ) millibars = atof( arg );

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    double ut1_utc = 0.108644;
    arg = get_arg( argc, argv, "ut1_utc" );
    if( arg ) ut1_utc = atof( arg );

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    double leap_secs = 37.000000;
    arg = get_arg( argc, argv, "leap_secs" );
    if( arg ) leap_secs = atof( arg );

    // create the tracker
    Tracker tracker;
    create(&tracker, ut1_utc, leap_secs );

    // set the tracker's time in UTC
    setTime( &tracker, get_time() );
//    print_time( &tracker );

    // set the location
    setCoordinates( &tracker, latitude, longitude, height );
    setAtmosphere( &tracker, celsius, millibars );
//    print_site( &tracker );

    // create and load a catalog
    FILE *file = fopen("../data/FK6.txt", "r");
    Catalog catalog;
    init( &catalog, SIZE );
    load( &catalog, file );

//    print_catalog( &catalog );

    // start the timer
    double start = get_time();

    // track every star in the FK6 catalog
    double tracks [SIZE][2]; //double latitude, longitude;
    for( int t=0; t< TRIALS; t++ ) {
        for (int n = 0; n < catalog.size; n++) {
            Entry *entry = catalog.stars[n];
            setTarget(&tracker, entry);
            getTopocentric(&tracker, &tracks[n][0], &tracks[n][1]);
        }
    }

    // get the time
    double end = get_time();
    double duration = end - start;
    printf( "time: %lf\nspeed: %lf\n", duration, duration/(TRIALS*SIZE) );

    // print the catalog with corresponding tracks
    for( int n=0; n<catalog.size; n++ ) {
        Entry* entry = catalog.stars[n];
        print_entry( entry );
        printf( "appears at(%f, %f)\n\n", tracks[n][0], tracks[n][1] );
    }

    char line[1024];
    while( getline( line, 1024, stdin ) != -1 ) {
        if( strcmp(line, "exit") ) {
            break;
        }
        else if( strcmp(line, "search") ) {
            printf("right ascension hours : ");
            getline( line, 1024, stdin );
            double ra = atof( line );

            printf("declination degrees : ");
            getline( line, 1024, stdin );
            double dec = atof( line );

            printf("radius degrees : ");
            getline( line, 1024, stdin );
            double rad = atof( line );

            Catalog results;
            search( &catalog, ra, dec, rad, &results );

        }
        else if( strcmp(line, "") ) {

        } else {

        }
    };

    // clean up the program components
    freeCatalog( &catalog, true );
}


int get_search() {

}