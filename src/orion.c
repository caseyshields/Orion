#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pthread.h>
#include <winsock.h>

#include "novasc3.1/novas.h"
#include "tracker.h"
#include "vmath.h"
#include "orion.h"

/** A tracker controller which issues TATS commands over ethernet, given star information.
 * @author Casey Shields
 */
int main( int argc, char *argv[] ) {

    // create main components according to program arguments
    tracker = configure_tracker( argc, argv );
    client = configure_client( argc, argv );
    catalog = configure_catalog( argc, argv );

    // test connection
    char * teststr = "testy tester testing tests testily";
    int length = strlen( teststr );
    int sent = send( client, teststr, length, 0 );
    if( sent < length )
        terminate( sent, "Failed to send entire message");

    // TODO start control thread

    // TODO take user input and relay commands

    terminate( 0, NULL );
}

/** Gets an accurate UTC timestamp from the system in seconds since the unix epoch */
double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

/** extracts tracker information from the program arguments and constructs a model */
Tracker * configure_tracker( int argc, char *argv[] ) {

    // tracker configuration
    double latitude, longitude, height;
    double celsius, millibars;

    // Earth orientation/time configuration
    double ut1_utc, leap_secs;

    // geodetic coordinates in degrees
    char* arg = get_arg( argc, argv, "-latitude", LATITUDE );
    latitude = atof( arg );

    arg = get_arg( argc, argv, "-longitude", LONGITUDE );
    longitude = atof( arg );

    // geodetic height in meters
    arg = get_arg( argc, argv, "-height", HEIGHT );
    height = atof( arg );

    // site temperature in degrees celsius
    arg = get_arg( argc, argv, "-celsius", TEMPERATURE );
    celsius = atof( arg );

    // atmospheric pressure at site in millibars
    arg = get_arg( argc, argv, "-millibars", PRESSURE );
    millibars = atof( arg );

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    arg = get_arg( argc, argv, "-ut1_utc", UT1_UTC );
    ut1_utc = atof( arg );

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    arg = get_arg( argc, argv, "-leap_secs", TAI_UTC );
    leap_secs = atof( arg );

    // create the tracker
    Tracker* tracker = malloc( sizeof(Tracker) );
    tracker_create(tracker, ut1_utc, leap_secs);

    // set the tracker's time in UTC
    tracker_set_time(tracker, get_time());
    tracker_print_time(tracker);

    // set the location
    tracker_set_location(tracker, latitude, longitude, height);
    tracker_set_weather(tracker, celsius, millibars);
    tracker_print_site(tracker);

    return tracker;
}

/** Connects to a socket to the server specified in the arguments. */
unsigned int configure_client(int argc, char* argv[]) {
    int result;

    // get server address, should be in dotted quad notation
    char* ip = get_arg( argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port; // port
    char* arg = get_arg( argc, argv, "-port", "43210" );//8080" );
    port = (unsigned short)atoi( arg );

    // load the winsock library
    WSADATA wsadata;
    result = WSAStartup( MAKEWORD(2, 2), &wsadata);
    if( result != 0 )
        terminate( result, "Failed to initialize winsock" );

      // retrieve a list of available sockets for the server
//    struct sockaddr_in hints;
//    mwmser(&hints, 0, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//    hints.ai_flags = AI_PASSIVE;
//
//    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, server);
//    if( result!=0 ) {
//        result = WSAGetLastError();
//        freeaddrinfo(server);
//        orion_exit( result, "failed to get server socket information" );
//    }

    // create a TCP socket for the client
    unsigned int server = INVALID_SOCKET; // listening socket descriptor
    server = socket(PF_INET/*AF_UNSPEC*/, SOCK_STREAM, IPPROTO_TCP);
    if( server == INVALID_SOCKET )
        terminate( WSAGetLastError(), "Failed to create socket\n");


    // construct server address structure
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; // internet address family
    address.sin_addr.s_addr = inet_addr( ip ); // server ip
    address.sin_port = htons( port ); // sserver port


    // connect the socket to the server
    result = connect( server, (struct sockaddr *) &address, sizeof(address) );
    if( result == SOCKET_ERROR )
        terminate( WSAGetLastError(), "Failed to connect to the server");

    return server;
}

Catalog * configure_catalog( int argc, char* argv[] ) {
    // get the location of the catalog data
    char* path = get_arg( argc, argv, "-catalog", "../data/FK6.txt");

    // create and load a catalog
    FILE *file = fopen(path, "r");
    Catalog* catalog = catalog_load_fk5(NULL, file);

    return catalog;
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

void terminate(int status, char* msg) {
    if( msg!=NULL )
        printf( "Error : %s : %d", msg, status );

    // release resources
    if( tracker != NULL )
        free( tracker );

    if( catalog != NULL ) {
        catalog_free_entries( catalog );
        catalog_free( catalog );
    }

    if( client != INVALID_SOCKET ) {
        int result = shutdown( client, SD_SEND ); // winsock specific
        if( result == SOCKET_ERROR )
            ;// well we tried
        closesocket(client);
    }

    // winsock cleanup routine
    WSACleanup();

    exit( status );
}
