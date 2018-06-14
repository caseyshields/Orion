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

Orion* orion_create( Orion * orion ) {
    if (!orion)
        orion = malloc(sizeof(orion));
    if (orion) {
        memset(orion, 0, sizeof(orion));
        orion->client = INVALID_SOCKET;
        pthread_mutex_init( &(orion->mutex), NULL);
        orion->rate = (int)(SLEEP_RESOLUTION / ORION_RATE);//(int)(1000.0 / 50.0);
    }
    return orion;
}

/** gets a millisecond accurate timestamp from the system, converts it to Julian hours(see Novas
 * 3.1 documentation), then sets the current tracker time.
 * @returns the last marked timestamp.*/
double orion_mark_time( Orion * orion ) {
    struct timeval time;
    gettimeofday( &time, NULL ); // UTC timestamp in unix epoch
    double julian_hours = time.tv_sec + (time.tv_usec / 1000000.0);
    double last_time = orion->tracker.date;
    orion->tracker.date = julian_hours;
    return last_time;
}

int orion_is_connected (Orion * orion) {
    return orion->client != INVALID_SOCKET;
}

int orion_is_running ( Orion * orion ) {
    pthread_mutex_lock( &(orion->mutex) );
    int running = orion->mode;
    pthread_mutex_unlock( &(orion->mutex) );
    return running == ORION_MODE_ON;
} //https://www.cs.nmsu.edu/~jcook/Tools/pthreads/library.html

//char* orion_set_error( Orion *orion, char* error ) {
//    orion->error = error;
//}
//
//char* orion_poll_error( Orion* orion ) {
//
//}

int orion_connect ( Orion * orion, char * ip, unsigned short port ) {
    char * error = NULL;
    int status = 0;

    // construct server address structure
    struct sockaddr_in address = {
            .sin_zero = {0,0,0,0,0,0,0,0},
            .sin_family = AF_INET, // internet address family
            .sin_addr.s_addr = inet_addr(ip), // server ip
            .sin_port = htons(port) // server port
    };

    // load the winsock library
    WSADATA wsadata;
    status = WSAStartup( MAKEWORD(2, 2), &wsadata);
    if( status != 0 ) {
        error = "Failed to initialize winsock";
        goto EXIT;
    }

    // create a TCP socket for the client
    orion->client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( orion->client == INVALID_SOCKET ) {
        error = "Failed to create socket";
        goto CLEANUP_WINSOCK;
    }

    // connect the socket to the server
    status = connect( orion->client, (struct sockaddr*)&(address), sizeof(address) );
    if( status == SOCKET_ERROR ) {
        error = "Failed to connect to the server";
        goto CLEANUP_SOCKET;
    }

    status = 0;
    goto EXIT;

    CLEANUP_SOCKET:
    status = WSAGetLastError();
    closesocket(orion->client); // just close in POSIX

    CLEANUP_WINSOCK:
    WSACleanup(); // no posix equivalent

    EXIT:
    if (error)
        sprintf(orion->error, "[%d] %s\n", status, error);
    return status;
}

void * orion_control_loop( void * arg ) {
    Orion * orion = (Orion*)arg;
    double zd, az;
    char buffer[1024];
    memset(&buffer, 0, 1024);

    // loop indefinately
    do {
        // obtain lock
        pthread_mutex_lock( &(orion->mutex) );

        // check if the server has been stopped
        if (orion->mode)
            break;

        // update the current time
        double last_time = orion_mark_time(orion);

        // create a tracking message if we have a target
        if (orion->target.starnumber) {

            // calculate the current location of the target
            tracker_to_horizon( &(orion->tracker), &(orion->target), &zd, &az);

            // devise the tracking message
            sprintf(buffer, "%4s%ld %s (%lf, %lf)\n\0",
                    orion->target.catalog,
                    orion->target.starnumber,
                    orion->target.starname,
                    az, zd);
            // todo use TATS MIDC01

        } else { // otherwise send an idle message
            sprintf(buffer, "IDLE\n\0");
        }

        // we no longer need tracker internals or mode, so we can release the lock
        pthread_mutex_unlock( &(orion->mutex) );

        // send the tracking message
        int length = strlen( buffer );
        int sent = send( orion->client, buffer, length, 0 );

        // set error and exit if there was a transmission error
        if (sent < length) {
            sprintf(orion->error, "[%d] Failed to send entire message, sent %d\0", WSAGetLastError(), sent);
            break;
        }

        // enter a idle state
        // TODO use running average to set heartbeat rate
        sleep( orion->rate ); // in Windows this is in Milliseconds

    } while( TRUE );

    // release the lock after we abort the control loop
    pthread_mutex_unlock( &(orion->mutex) );

    return (void*) orion;
}

int orion_start( Orion * orion ) {
    // get mutex lock
    pthread_mutex_lock( &(orion->mutex) );

    // check server is off
    int mode = orion->mode;
    if ( mode ) {
        orion->mode = ORION_MODE_ON;
        pthread_create(&orion->control, NULL, orion_control_loop, &orion);
    } else {
        sprintf(orion->error, "[%d] %s", 1, "Orion server is not initialized or has erred");
    }

    pthread_mutex_unlock( &(orion->mutex) );
    return mode;
}

int orion_select( int id ) {
    // check sensor control thread mode
    // obtain mutex
    // set target
    // release mutex
    return -1;
}

int orion_stop(Orion * orion) {

    // obtain lock since we need to change the mode
    pthread_mutex_lock( orion->mutex );

    // abort if the server isn't running
    if (orion->mode == ORION_MODE_OFF) {
        sprintf(orion->error, "[%ld] %s", 1, "server is not running");
        pthread_mutex_unlock(orion->mutex);
        return 1;
    }

    // turn it off
    orion->mode = ORION_MODE_OFF;

    // release the lock so the control thread can complete gracefully
    pthread_mutex_unlock(orion->mutex);

    // wait for the control thread to join
    void * result = NULL;
    pthread_join( orion->control, &result);

    return 0;
}

int orion_disconnect( Orion * orion ) {

    // abort if the server is running
    if (orion_is_running( orion )) {
        sprintf( orion->error, "[%d] %s\n\0", 1, "Cannot disconnect server while it is running");
        return 1;
    }

    // abort if the sensor is not connected
    if(orion->client == INVALID_SOCKET) {
        sprintf( orion->error, "[%d] %s\n\0", 1, "Server is not connected");
        return 1;
    }

    // winsock specific, close the network connection
    if( shutdown(orion->client, SD_SEND) == SOCKET_ERROR )
        ;// well we tried

    // close the socket
    closesocket( orion->client ); // just close(...) in POSIX

    // unloads the winsock libraries
    WSACleanup(); // no posix equivalent

    return 0;
}
