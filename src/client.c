#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "novasc3.1/novas.h"
#include "tracker.h"
#include "catalog.h"
#include "vmath.h"

/** A tracker controller which issues TATS commands over ethernet, given star information.
 * @author Casey Shields
 */

double get_time();
char* get_arg( int argc, char *argv[], char *name, char* default_value );

int main( int argc, char *argv[] ) {
    char *arg;

    // tracker configuration
    double latitude, longitude, height;
    double celsius, millibars;

    // Earth orientation/time configuration
    double ut1_utc, leap_secs;

    // socket communication
    int sock; // socket descriptor
    struct sockaddr_in server; // server address
    unsigned short port; // port
    char* ip; // in dotted quad notation
    WSADATA wsadata; // winsock setup thing?

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

    // socket address
    ip = get_arg( argc, argv, "-ip", "127.0.0.1");
    arg = get_arg( argc, argv, "-port", "8080" );
    if( arg ) port = atoi( arg );


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

    // TODO the tracker should just a cat_entry structure as an argument rather than maintaining a catalog!...
//    // create and load a catalog
//    FILE *file = fopen(path, "r");
//    Catalog* catalog = catalog_load_fk5(NULL, file);

    // create the socket
    if( WSAStartup( MAKEWORD(2, 0), wsadata) != 0 )
        orion_exit( 1, "Failed to initialize winsock" );

    if( (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) )
        orion_exit(1, "Failed to create socket" );

    // currently using
    //http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/winsock.html
    // for a simple example

    char *line = NULL;
    size_t size = 0 ;
    ssize_t read = 0;
    while(true) {

        // update the current time and print a prompt
        setTime( &tracker, get_time() );
        //printf( "orion[%lf]", getUTC(&tracker) );

    }
}

/** Gets an accurate UTC timestamp from the system in seconds since the unix epoch */
double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

/** retrieves the value subsequent to the specified option. If the default_value
 * is supplied, the function will return it. otherwise the method will print an
 * error message and abort. */
char* get_arg( int argc, char *argv[], char *name, char* default_value ) {
    for( int n=0; n<argc; n++ )
        if( !strcmp(name, argv[n]) )
            return argv[n+1];
    if( default_value ) {
        printf("Parameter '%s' defaulting to '%s'\n", name, default_value);
        return default_value;
    }
    sprintf( "Error: missing parameter '%s'\n", name );
    exit( 1 );
}

void orion_exit(int status, char* msg) {

    if( msg!=NULL )
        printf( msg );
    exit( status );
}
// filter by name
//get_input("catalog number", &line, &size);
//int check_name( Entry *entry ) {
//    return NULL != strstr(entry->novas.starname, line );
//}
//Catalog* results = catalog_filter(catalog, &check_name, NULL);

// transform each star to local coordinates
void process( Entry *entry ) {
    double zd=0, az=0;
    setTarget( &tracker, entry );
    local( &tracker, &zd, &az );
    entry_print( entry );
    printf( "\tlocal : { zd:%lf, az:%lf}\n", zd, az );
}

void zenith() {
    double ra = 0, dec = 0;
    zenith( &tracker, &ra, &dec );
    printf( "Current zenith coodinates : (ra:%lf, dec:%lf)\n", ra, dec );
}

void close() {
    catalog_free_entries(catalog);
    catalog_free(catalog);
    free( line );
    exit(0);
}