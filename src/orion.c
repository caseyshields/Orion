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

Orion orion;

/** A tracker controller which issues TATS commands over ethernet, given star information.
 * @author Casey Shields
 */
int main( int argc, char *argv[] ) {
    char *line = NULL;
    size_t size = 0 ;
    ssize_t read = 0;

    //Orion orion;//orion = malloc( sizeof(Orion) );
    memset( &orion, 0, sizeof(orion) );

    // create main components according to program arguments
    configure_tracker( argc, argv, &(orion.tracker) );
    configure_address( argc, argv, &(orion.address) );
    configure_catalog( argc, argv, &(orion.catalog) );

    tracker_print_site( &(orion.tracker) );

    int result;
    while( true ) {

        // update and print out the time
        tracker_set_time( &(orion.tracker), get_time() );
        tracker_print_time( &(orion.tracker) );

        // print prompt and get next user command

        read = get_input( "", &line, &size );

        // mark time
        // set time
        // set orientation ( ut1-utc, leapsecs, x, y )
        // set location ( lat, longitude, height )
        // set weather ( temperature, pressure )
        // set sensor ( ip, port )

        // load catalog ( path )
        if( strncmp( "load ", line, 5)==0) {
            char path[256];
            result = sscanf( line, "load %256s\n", path );
            if(result==0) {
                printf("usage: load <catalog path>");
                continue;
            }
            if( orion.catalog.size>0)
                catalog_free_entries( &(orion.catalog) );
            FILE *file = fopen( path, "r" );
            catalog_load_fk5( &(orion.catalog), file );
        }

        // search name
        if( strncmp( "search ", line, 7)==0 ) {
            char name[32];
            result = sscanf( line, "search %32s\n", name);
            if( result==0 ) {
                printf( "usage: search <name>\n");
                continue;
            }
            int check_name( Entry *entry ) {
                return NULL != strstr(entry->novas.starname, name );
            }
            Catalog* results = catalog_filter(&(orion.catalog), check_name, NULL);
            catalog_each( results, entry_print );
            free( results );
        } // todo by number as well?

//        // search horizon patch
//        double azimuth_0, azimuth_1, elevation_0, elevation_1;
//        result = scanf("search %lf %lf %lf %lf\n", azimuth_0, azimuth_1, elevation_0, elevation_1 );

        // start the sensor control thread
        if( strncmp( "start", line, 5 ) == 0 ) {
            if(orion_start(&orion))
                printf("Sensor control thread already started.\n");
        }

        // select a target
        if( strncmp("select", line, 6)==0 ) {
            int id = 0;
            result = sscanf( line, "select %d\n", &id );
            if( orion_select(&orion, id) )
                printf( "select failed");
        }

        // stop the sensor control thread
        if( strncmp("stop", line, 4)==0 ) {
            orion_stop( &(orion) );
        } // reference : https://www.cs.nmsu.edu/~jcook/Tools/pthreads/library.html

        // exit orion
        if( strncmp("exit", line, 4)==0 ) {
            break;
        }

        // get help with the commands
        if( strncmp( "help", line, 4 )==0 ) {
            result = sscanf(line, "help%s\n");
            if (result == 0) {
                printf("Commands:load <path>\nsearch <name>\nstart\nselect <id>\nstop\nexit\n");
            } else {
                // todo print specific help
            }
        }
    }

    catalog_free_entries( &(orion.catalog) );
}

int orion_start(Orion * orion) {
    if( !orion->mode ) {
        pthread_create(&(orion->control), NULL, control_loop, orion);
        return 0;
    } else {
        printf("Sensor control thread already started.\n");
        return 1;
    }
//    int* status = malloc( sizeof(int) );
//    *status = 0; // should we just cast the return pointer to simplify things?
//
//    // check if the control thread is already launched
//    if( orion->mode ) {
//        printf("Sensor control thread already started.\n");
//        return 1;
//    }
//
//    // load the winsock library
//    WSADATA wsadata;
//    *status = WSAStartup( MAKEWORD(2, 2), &wsadata);
//    if( *status != 0 ) {
//        printf( "Error [%d] Failed to initialize winsock\n", *status);
//        goto ABORT;
//    }
//
//    // create a TCP socket for the client
//    orion->client = socket(PF_INET/*AF_UNSPEC*/, SOCK_STREAM, IPPROTO_TCP);
//    if( orion->client == INVALID_SOCKET ) {
//        *status = WSAGetLastError();
//        printf( "Error [%d] Failed to create socket\n", *status );
//        goto CLEANUP_WINSOCK;
//    }
//
//    // connect the socket to the server
//    *status = connect( orion->client, (struct sockaddr*)&(orion->address), sizeof(orion->address) );
//    if( *status == SOCKET_ERROR ) {
//        *status = WSAGetLastError();
//        printf( "Error [%d] Failed to connect to the server\n", *status );
//        goto CLEANUP_SOCKET;
//    }
//
//    // now start the control thread
//    pthread_create(&(orion->control), NULL, control_loop, orion);
//    status = 0;
//    goto ABORT;
//
//
//
//    CLEANUP_SOCKET:
//    closesocket(orion->client); // just close in POSIX
//
//    CLEANUP_WINSOCK:
//    WSACleanup(); // no posix equivalent
//
//    ABORT:
//    return *status;
}

int orion_select(Orion * orion, int id) {
    // check sensor control thread mode
    // obtain mutex
    // set target
    // release mutex
    return -1;
}

int orion_stop(Orion * orion) {
    if( orion->mode ) {
        int* value = NULL;
        pthread_join( orion->control, (void**)&value );
        printf( "thread returned with value %d", *value );
        free(value);
        return 0;
    } else {
        printf( "Sensor control thread not running.\n");
        return 1;
    }
}

void * control_loop( void * data ) {
//    //Orion* orion = (Orion*) data;
//    int* status = malloc( sizeof(int) );
//    *status = 0; // should we just cast the return pointer to simplify things?
//
//    // enter a loop where the target is continually updated...
//    while( orion->mode ) {
//
//        // test connection
//        char *teststr = "testy tester testing tests testily";
//        int length = strlen(teststr);
//        int sent = send(orion->client, teststr, length, 0);
//        if (sent < length) {
//            *status = WSAGetLastError();
//            printf("Error [%d] Failed to send entire message, sent %d\n", *status, sent);
//            goto CLEANUP_CONNECTION;
//        }
//
//    }
//
//    CLEANUP_CONNECTION:
//    if( shutdown(orion->client, SD_SEND) == SOCKET_ERROR ) // winsock specific
//        ;// well we tried
//}

    // remember to synchronize!
    //Orion* orion = (Orion*) data;
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
    orion.client = socket(PF_INET/*AF_UNSPEC*/, SOCK_STREAM, IPPROTO_TCP);
    if( orion.client == INVALID_SOCKET ) {
        *status = WSAGetLastError();
        printf( "Error [%d] Failed to create socket\n", *status );
        goto CLEANUP_WINSOCK;
    }

    // connect the socket to the server
    *status = connect( orion.client, (struct sockaddr*)&(orion.address), sizeof(orion.address) );
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
    int sent = send( orion.client, teststr, length, 0 );
    if( sent < length ) {
        *status = WSAGetLastError();
        printf("Error [%d] Failed to send entire message, sent %d\n", *status, sent);

        goto CLEANUP_CONNECTION;
    }
//    }

    CLEANUP_CONNECTION:
    if( shutdown(orion.client, SD_SEND) == SOCKET_ERROR ) // winsock specific
        ;// well we tried

    CLEANUP_SOCKET:
    closesocket(orion.client); // just close in POSIX

    CLEANUP_WINSOCK:
    WSACleanup(); // no posix equivalent

    ABORT:
    fflush(stdout);
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
        fflush(stdout);
        return default_value;
    }
    sprintf( "Error: missing parameter '%s'\n", name );
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
    (*line)[read] = '\0'; // trim trailing
    return read;
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