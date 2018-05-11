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
ssize_t get_input(char* prompt, char **line, size_t *size );

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
    Catalog* catalog = catalog_load_fk5(NULL, file);

    char *line = NULL;
    size_t size = 0 ;
    ssize_t read = 0;
    while(true) {

        // update the current time and print a prompt
        setTime( &tracker, get_time() );
        printf( "orion[%lf]", getUTC(&tracker) ); //print_time( &tracker );
        read = get_input( "", &line, &size );

        // select a specific star by number
        if( strncmp( "star", line, 4 ) == 0 ) {
            get_input("catalog number", &line, &size);
            int catalog_id = atoi(line);

            int check_id( Entry *entry ) {
                return entry->starnumber == catalog_id ? 1 : 0;
            }

            Catalog* results = catalog_filter(catalog, &check_id, NULL);
            catalog_print(results);
            catalog_free(results);
        }

        // search within a lesser circle of the catalog
        else if( strncmp( "dome", line, 4 )==0 ) {
            get_input( "right ascension hours", &line, &size );
            double ra = atof( line );//hours2radians( atof( line ) );

            get_input( "declination degrees", &line, &size );
            double dec = atof( line );//degrees2radians( atof( line ) );

            get_input( "radius degrees", &line, &size );
            double rad = atof( line );//degrees2radians( atof( line ) );

            Catalog* results = catalog_search_dome(catalog, ra, dec, rad, NULL);
            catalog_print(results);
            printf( "\n%d stars found.\n", results->size );
            catalog_free(results);
        }

        // search within a lesser circle of the catalog
        else if( strncmp( "patch", line, 5 )==0 ) {
            get_input( "minimum right ascension hours", &line, &size );
            double ra_min = atof( line );

            get_input( "maximum right ascension hours", &line, &size );
            double ra_max = atof( line );

            get_input( "minimum declination degrees", &line, &size );
            double dec_min = atof( line );

            get_input( "maximum declination degrees", &line, &size );
            double dec_max = atof( line );

            Catalog* results = catalog_search_patch(catalog, ra_min, ra_max, dec_min, dec_max, NULL);
            catalog_print(results);
            printf( "\n%d stars found.\n", results->size );
            catalog_free(results);
        }

        // test an orange slice shaped section of sky
        else if( strncmp("orange", line, 5)==0 ) {
            get_input( "minimum right ascension hours", &line, &size );
            double ra_min = atof( line );

            get_input( "maximum right ascension hours", &line, &size );
            double ra_max = atof( line );

            Catalog* results = catalog_orange(catalog, ra_min, ra_max, NULL);
            catalog_print(results);
            printf( "\n%d stars found.\n", results->size );
            catalog_free(results);
        }

        // print the entire catalog contents
        else if( strncmp( "print", line, 5 )==0 ) {
            catalog_print(catalog);
        }

        // run the benchmark
        else if( strncmp( "bench", line, 5 )==0 ) {
            benchmark( catalog, &tracker, 100 );
        }

        // clean up the program components and exit the program
        else if( strncmp( "exit", line, 4 )==0 ) {
            catalog_free_entries(catalog);
            catalog_free(catalog);
            free( line );
            exit(0);
        }

        // print available commands
        else {
            printf( "Commands include \n\tstar\n\tdome\n\tpatch\n\tprint\n\tbench\n\texit\n" );
        }
    }
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
        printf("Parameter '%s' defaulting to '%s'\n", name, default_value);
        return default_value;
    }
    printf( "Error: missing parameter '%s'\n", name );
    exit( 1 );
}

/** Frees any data line is pointing to, then prompts the user, allocates a buffer, and reads the input.
 * Be sure to free the buffer after your last call to get_input! */
ssize_t get_input(char* prompt, char **line, size_t *size ) {
    if( *line ) {
        free( *line );
        *line = NULL;
        *size = 0;
    }
    printf("%s : ", prompt);
    fflush( stdout );
    ssize_t read = getline( line, size, stdin );
    if( read == -1 ) {
        printf("Error: input stream closed");
        exit( 2 );
    }
    return read;
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
            setTarget( tracker, entry );
            local(tracker, &tracks[n][0], &tracks[n][1]);
        }
    }

    // get the time
    double end = get_time();
    double duration = end - start;

    // print the catalog with corresponding tracks
    for( int n=0; n<catalog->size; n++ ) {
        Entry* entry = catalog->stars[n];
        entry_print(entry);
        printf( "appears at(%f, %f)\n\n", tracks[n][0], tracks[n][1] );
    }

    printf( "trials: %d\ntime: %lf\nspeed: %lf\n\n", trials, duration, duration/(trials*catalog->size) );
}

void search_dome() {

}