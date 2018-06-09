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

    Orion orion;// = {
//            .tracker = NULL,
//            .client = INVALID_SOCKET
//    }; // in this guaranteed to be nulled out?
    memset( &orion, 0, sizeof(orion) );

    // create main components according to program arguments
    configure_tracker( argc, argv, &(orion.tracker) );
    configure_address( argc, argv, &(orion.address) );
    configure_catalog( argc, argv, &(orion.catalog) );

    // start sensor control thread
    // reference : https://www.cs.nmsu.edu/~jcook/Tools/pthreads/library.html
    pthread_t thread;
    pthread_create( &thread, NULL, orion_start_sensor, &orion );

    int* result = NULL;
    pthread_join( thread, (void**)&result );
    printf( "thread returned with value %d", *result );
    free(result);
    // TODO take user input and relay commands

    //tracker_print_time(tracker);
    //tracker_print_site(tracker);

    catalog_free_entries( &(orion.catalog) );
}

void * orion_start_ui( void * data ) {

}

void * orion_start_sensor( void * data ) {
    // remember to synchronize!
    Orion* orion = (Orion*)data;
    int* status = malloc( sizeof(int) );
    *status = 0; // should we just cast the return pointer to simplify things?

    // load the winsock library
    WSADATA wsadata;
    *status = WSAStartup( MAKEWORD(2, 2), &wsadata);
    if( *status != 0 ) {
        printf( "Error [%d] Failed to initialize winsock\n", *status);
        goto ABORT;
    }

    // create a TCP socket for the client
    orion->client = socket(PF_INET/*AF_UNSPEC*/, SOCK_STREAM, IPPROTO_TCP);
    if( orion->client == INVALID_SOCKET ) {
        *status = WSAGetLastError();
        printf( "Error [%d] Failed to create socket\n", *status );
        goto CLEANUP_WINSOCK;
    }

    // connect the socket to the server
    *status = connect( orion->client, (struct sockaddr*)&(orion->address), sizeof(orion->address) );
    if( *status == SOCKET_ERROR ) {
        *status = WSAGetLastError();
        printf( "Error [%d] Failed to connect to the server\n", *status );
        goto CLEANUP_SOCKET;
    }

    // enter a loop where the target is continually updated...
//    while( orion->state ) {
    // test connection
    char * teststr = "testy tester testing tests testily";
    int length = strlen( teststr );
    int sent = send( orion->client, teststr, length, 0 );
    if( sent < length ) {
        *status = WSAGetLastError();
        printf("Error [%d] Failed to send entire message, sent %d\n", *status, sent);
        goto CLEANUP_CONNECTION;
    }
//    }

    CLEANUP_CONNECTION:
    if( shutdown(orion->client, SD_SEND) == SOCKET_ERROR ) // winsock specific
        ;// well we tried

    CLEANUP_SOCKET:
    closesocket(orion->client); // just close in POSIX

    CLEANUP_WINSOCK:
    WSACleanup(); // no posix equivalent

    ABORT:
    return status;
}

/** Gets an accurate UTC timestamp from the system in seconds since the unix epoch */
double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

// Earth orientation/time configuration
void configure_tracker( int argc, char* argv[], Tracker* tracker ) {

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    char* arg = get_arg( argc, argv, "-ut1_utc", UT1_UTC );
    double ut1_utc = atof( arg );

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    arg = get_arg( argc, argv, "-leap_secs", TAI_UTC );
    double leap_secs = atof( arg );

    // create the tracker
    tracker_create(tracker, ut1_utc, leap_secs);

    // set the tracker's time in UTC
    tracker_set_time(tracker, get_time());

    // geodetic coordinates
    arg = get_arg(argc, argv, "-latitude", LATITUDE);
    double latitude = atof(arg);

    arg = get_arg(argc, argv, "-longitude", LONGITUDE);
    double longitude = atof(arg);

    arg = get_arg(argc, argv, "-height", HEIGHT);
    double height = atof(arg);

    // set the location of the tracker
    tracker_set_location(tracker, latitude, longitude, height);

    // site temperature in degrees celsius
    arg = get_arg( argc, argv, "-celsius", TEMPERATURE );
    double celsius = atof( arg );

    // atmospheric pressure at site in millibars
    arg = get_arg( argc, argv, "-millibars", PRESSURE );
    double millibars = atof( arg );

    // set the weather
    tracker_set_weather(tracker, celsius, millibars);
}

/** Creates an ip address for the sensor from the arguments. */
void configure_address(int argc, char* argv[], struct sockaddr_in* address) {

    // get server address, should be in dotted quad notation
    char *ip = get_arg(argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port; // port
    char *arg = get_arg(argc, argv, "-port", "43210");
    port = (unsigned short) atoi(arg);

    // construct server address structure
    address->sin_family = AF_INET; // internet address family
    address->sin_addr.s_addr = inet_addr( ip ); // server ip
    address->sin_port = htons( port ); // server port
}

void configure_catalog( int argc, char* argv[], Catalog* catalog ) {
    // get the location of the catalog data
    char* path = get_arg( argc, argv, "-catalog", "../data/FK6.txt" );

    // create and load a catalog
    FILE *file = fopen( path, "r" );
    catalog_load_fk5( catalog, file );
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

//void terminate(Orion* orion, int status, char* msg) {
//
//    // print message if given
//    if( msg!=NULL )
//        printf( "Error : %s : %d", msg, status );
//
//    // release resources
////    if( tracker.earth != NULL )
////        free( tracker );
//
//    if( orion->catalog.allocated != NULL ) {
//        catalog_free_entries( &(orion->catalog) );
////        catalog_free( catalog );
//    }
//
//    if( orion->client != INVALID_SOCKET ) {
//
//        int result = shutdown( orion->client, SD_SEND ); // winsock specific
//        if( result == SOCKET_ERROR )
//            ;// well we tried
//
//        closesocket(orion->client);
//    }
//
//    // winsock cleanup routine
//    WSACleanup();
//
//    exit( status );
//}

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