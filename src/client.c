#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "novas.h"
#include "tracker.h"
#include "catalog.h"
#include "vmath.h"
#include <unistd.h>
#include <stdbool.h>

double get_time();
char* get_arg( int argc, char *argv[], char *name, char* default_value );
void get_input(char* prompt, char** buffer );
void benchmark( Catalog* catalog, Tracker* tracker, int trials );

/** a simple CLI interface for exercising various orion components. */
int main( int argc, char *argv[] ) {
    double latitude, longitude, height;
    double celsius, millibars;
    double ut1_utc, leap_secs;
    char *arg, *path;

    // geodetic coordinates in degrees
    arg = get_arg( argc, argv, "-latitude", "38.88972222222222" );
    if( arg ) latitude = atof( arg );

    arg = get_arg( argc, argv, "-longitude", "-77.0075" );
    if( arg ) longitude = atof( arg );

    // geodetic height in meters
    arg = get_arg( argc, argv, "-height", "125.0" );
    if( arg ) height = atof( arg );

    // site temperature in degrees celsius
    arg = get_arg( argc, argv, "-celsius", "10.0" );
    if( arg ) celsius = atof( arg );

    // atmospheric pressure at site in millibars
    arg = get_arg( argc, argv, "-millibars", "1010.0" );
    if( arg ) millibars = atof( arg );

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    arg = get_arg( argc, argv, "-ut1_utc", "0.108644" );
    if( arg ) ut1_utc = atof( arg );

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    arg = get_arg( argc, argv, "-leap_secs", "37.000000" );
    if( arg ) leap_secs = atof( arg );

    // get the location of the catalog data
    path = get_arg( argc, argv, "-catalog", "../data/FK6.txt");

    // create the tracker
    Tracker tracker;
    create(&tracker, ut1_utc, leap_secs );

    // set the tracker's time in UTC
    setTime( &tracker, get_time() );
    print_time( &tracker );

    // set the location
    setCoordinates( &tracker, latitude, longitude, height );
    setAtmosphere( &tracker, celsius, millibars );
    print_site( &tracker );

    // create and load a catalog
    FILE *file = fopen(path, "r");
    Catalog catalog;
    init( &catalog, 64 );
    load( &catalog, file );

    char line[1024];
    while(true) {

        // update the current time and print a prompt
        setTime( &tracker, get_time() );
        print_time( &tracker );
        get_input( ">", &line );

        // clean up the program components and exit the program
        if( strcmp( "exit", line )==0 ) {
            freeCatalog( &catalog, true );
            exit(0);
        }

        // search within a lesser circle of the catalog
        else if( strcmp( "circle", line )==0 ) {
            get_input( "right ascension hours", &line );
            double ra = hours2radians( atof( line ) );

            get_input( "declination degrees", &line );
            double dec = degrees2radians( atof( line ) );

            get_input( "radius degrees", &line );
            double rad = degrees2radians( atof( line ) );

            Catalog results;
            init( &results, 64 );
            search( &catalog, ra, dec, rad, &results );
            print_catalog( &results );
            freeCatalog( &results, true );
        }

        // print the entire catalog contents
        else if( strcmp( "catalog", line )==0 ) {
            print_catalog( &catalog );
        }

        //
        else if( strcmp(line, "")==0 ) {
        }

        // print available commands
        else {
            printf( "Commands include \n\tcircle\n\tbench\n\texit" );
        }
    }
}

int get_search() {

}

/** uses the GNU gettime of day method to get a accurate system time, then converts it to seconds since the unix epoch */
double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

/** retrieves the value subsequent to the specified option. If the default_value is supplied, the function will return
 * it if the option is not included. otherwise the method will print an error message and abort. */
char* get_arg( int argc, char *argv[], char *name, char* default_value ) {
    for( int n=0; n<argc; n++ )
        if( !strcmp(name, argv[n]) )
            return argv[n+1];
    if( default_value ) {
        printf("Parameter '%s' defaulting to '%s'", name, default_value);
        return default_value;
    }
    printf( "Error: missing parameter '%s'", name );
    exit( 1 );
}

void get_input(char* prompt, char** buffer ) {
    printf("%s : ", prompt);
    if( getline( buffer, (size_t) 1024, stdin ) == -1 ) {
        printf("Error: input stream closed");
        exit( 2 );
    }
}

/** Time how long it takes to point the tracker at every star in the catalog then prints the local coordinates. */
void benchmark( Catalog* catalog, Tracker* tracker, int trials ) {
    // start the timer
    double start = get_time();
    int size = catalog->size;

    // track every star in the FK6 catalog
    double tracks [catalog->size][2]; //double latitude, longitude;
    for( int t=0; t<trials; t++ ) {
        for (int n = 0; n < catalog->size; n++) {
            Entry *entry = catalog->stars[n];
            setTarget(&tracker, entry);
            getTopocentric(&tracker, &tracks[n][0], &tracks[n][1]);
        }
    }

    // get the time
    double end = get_time();
    double duration = end - start;
    printf( "time: %lf\nspeed: %lf\n", duration, duration/(trials*catalog->size) );

    // print the catalog with corresponding tracks
    for( int n=0; n<catalog->size; n++ ) {
        Entry* entry = catalog->stars[n];
        print_entry( entry );
        printf( "appears at(%f, %f)\n\n", tracks[n][0], tracks[n][1] );
    }
}