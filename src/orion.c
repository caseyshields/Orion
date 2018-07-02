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

#include "h/vmath.h"
#include "h/orion.h"

Orion* orion_create( Orion * orion ) {
    if (!orion)
        orion = malloc(sizeof(orion));
    if (orion) {
        memset( orion, 0, sizeof(Orion) );
        orion->mode = ORION_MODE_OFF;
        orion->socket = INVALID_SOCKET;
        pthread_mutex_init( &(orion->lock), NULL);
        orion->rate = ORION_RATE; //(int)(SLEEP_RESOLUTION / ORION_RATE);
    }
    return orion;
}

int orion_connect ( Orion * orion, char * ip, unsigned short port ) {
    char * error = NULL;
    int status = 0;

    // sanity check
    assert( orion->socket == INVALID_SOCKET );
    assert( orion->mode == ORION_MODE_OFF );

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
    } // yes, nested error handling is a valid use of goto's in C. Gotta make do without throws...

    // create a TCP socket for the client
    orion->socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( orion->socket == INVALID_SOCKET ) {
        error = "Failed to create socket";
        goto CLEANUP_WINSOCK;
    }

    // connect the socket to the server
    status = connect( orion->socket, (struct sockaddr*)&(address), sizeof(address) );
    if( status == SOCKET_ERROR ) {
        error = "Failed to connect to the server";
        goto CLEANUP_SOCKET;
    }

    status = 0;
    goto EXIT;

    CLEANUP_SOCKET:
#ifdef WIN32
    status = WSAGetLastError();
    closesocket( orion->socket );
#else
    status = 1;
    close( orion->socket );
#endif

    CLEANUP_WINSOCK:
#ifdef WIN32
    WSACleanup();
#endif

    EXIT:
    if (error)
        sprintf(orion->error, "[%d] %s\n\0", status, error);
    return status;
}

void * orion_control_loop( void * arg ) {
    Orion * orion = (Orion*)arg;
    double zd, az;
    char buffer[1024];
    memset(&buffer, 0, 1024);

    // sanity check
    assert( orion->mode == ORION_MODE_ON );
    assert( orion->socket != INVALID_SOCKET );

    // loop indefinitely
    do {
        // obtain lock
        pthread_mutex_lock( &(orion->lock) );

        // check if the server has been stopped
        if (orion->mode == ORION_MODE_OFF)
            break;

        // update the current time
        double last_time = orion_mark_time(orion);
        // todo We should have a mode that allows setting a historical time

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
        pthread_mutex_unlock( &(orion->lock) );

        // send the tracking message
        int length = strlen( buffer );
        int sent = send( orion->socket, buffer, length, 0 );

        // set error and exit if there was a transmission error
        if (sent < length) {
            sprintf(orion->error, "[%d] Failed to send entire message, sent %d bytes\n\0", WSAGetLastError(), sent);
            break;
        }

        // enter a idle state
//ifdef WIN32
        sleep( orion->rate ); // in Windows this is in Milliseconds
//else
//        struct timespec ts;
//        nanosleep( &ts, );
        //todo tats itself uses nanosleep...
//endif
// TODO use running average to set heartbeat rate

    } while( TRUE );

    // release the lock after we abort the control loop
    pthread_mutex_unlock( &(orion->lock) );

    return (void*) orion;
}

int orion_start( Orion * orion ) {
    // ensure the sensor is connected
    assert( orion->socket != INVALID_SOCKET );

    // check if server is already running
    pthread_mutex_lock( &(orion->lock) );
    if ( orion->mode == ORION_MODE_ON ) {
        sprintf(orion->error, "[%d] %s\n\0", 1, "Server is already running.");
        pthread_mutex_unlock( &(orion->lock) );
        return 1;
    }

    // start the server control thread
    orion->mode = ORION_MODE_ON;
    pthread_create( &(orion->control), NULL, orion_control_loop, orion);
    pthread_mutex_unlock( &(orion->lock) );
    return 0;
}

void orion_track( Orion * orion, cat_entry target ) {
    // can this be called regardless if the server is on or not?
    // assert( orion->mode == ORION_MODE_ON );

    // safely change the target
    pthread_mutex_lock( &(orion->lock) );
    orion->target = target;
    pthread_mutex_unlock( &(orion->lock) );
}

int orion_stop( Orion * orion ) {

    // obtain lock since we need to change the mode
    pthread_mutex_lock( &(orion->lock) );

    // abort if the server isn't running
    if (orion->mode == ORION_MODE_OFF) {
        sprintf(orion->error, "[%d] %s\n\0", 1, "server is not running");
        pthread_mutex_unlock( &(orion->lock) );
        return 1;
    }

    // turn it off
    orion->mode = ORION_MODE_OFF;

    // release the lock so the control thread can complete gracefully
    pthread_mutex_unlock( &(orion->lock) );

    // wait for the control thread to join
    void * result = NULL;
    pthread_join( orion->control, &result);

    return 0;
}

void orion_disconnect( Orion * orion ) {

    // sanity check, the control thread should be off and the socket valid
    assert( orion->mode == ORION_MODE_OFF );
    assert( orion->socket != INVALID_SOCKET );

//    // abort if the server is running
//    if (orion_is_running( orion )) {
//        sprintf( orion->error, "[%d] %s\n\0", 1, "Cannot disconnect server while it is running");
//        return 1;
//    }
//
//    // abort if the sensor is not connected
//    if(orion->socket == INVALID_SOCKET) {
//        sprintf( orion->error, "[%d] %s\n\0", 1, "Server is not connected");
//        return 1;
//    }

#ifdef WIN32
    // winsock specific, close the network connection
    if( shutdown(orion->socket, SD_SEND) == SOCKET_ERROR )
        ;// well we tried

    // close the socket
    closesocket( orion->socket );

    // unloads the winsock libraries
    WSACleanup(); // no posix equivalent
#else
    // posix version
    close( orion->socket );
#endif
}

double orion_time( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    double time = orion->tracker.date;
    pthread_mutex_unlock( &(orion->lock) );
    return time;
}

double orion_mark_time( Orion * orion ) {
    struct timeval time;
    gettimeofday( &time, NULL ); // UTC timestamp in unix epoch
    double julian_hours = (time.tv_sec + (time.tv_usec / 1000000.0)) / 3600.0;
    double last_time = orion->tracker.date;
    orion->tracker.date = julian_hours;
    return last_time;
} // we might want to lock the tracker time...

int orion_is_connected (Orion * orion) {
    return orion->socket != INVALID_SOCKET;
}

int orion_is_running ( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    int running = orion->mode;
    pthread_mutex_unlock( &(orion->lock) );
    return running == ORION_MODE_ON;
}

void orion_clear_error( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    memset( &(orion->error), 0, 128);
    pthread_mutex_unlock( &(orion->lock) );
}
