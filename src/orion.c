#include <unistd.h>
#include <assert.h>
#include <h/tats.h>
#include <h/orion.h>

#include "h/orion.h"
#include "h/vmath.h"

MIDC01 * create_tracking_message( Orion * orion, MIDC01 * midc01 );

Orion* orion_create( Orion * orion, unsigned short int id ) {
    if (!orion)
        orion = malloc(sizeof(orion));
    if (orion) {
        memset( orion, 0, sizeof(Orion) );
        orion->id = id;
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
    int length = 0;
    char buffer[1024];
    memset(&buffer, 0, 1024);

    // sanity check
    assert( orion->mode == ORION_MODE_ON );
    assert( orion->socket != INVALID_SOCKET );

    // set the current time
    double last_time = orion_mark_time(orion);

    // loop indefinitely
    do {
        // obtain lock
        pthread_mutex_lock( &(orion->lock) );

        // check if the server has been stopped
        if (orion->mode == ORION_MODE_OFF)
            break;

        // update the current time if we are in real time mode
        //else if (orion->mode == ORION_MODE_REAL_TIME)
            last_time = orion_mark_time(orion);

        // create a tracking message
        MIDC01 * message = (void*) buffer;
        create_tracking_message(orion, message);
        length = sizeof( MIDC01 );

        // we no longer need tracker internals or mode, so we can release the lock
        pthread_mutex_unlock( &(orion->lock) );

        // send the tracking message
        //int length = strlen( buffer );
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
} // helpful tutorial: https://www.cs.nmsu.edu/~jcook/Tools/pthreads/library.html

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

void orion_date( Orion * orion, int year, int month, int day, int hour, int min, double second ) {
    //tracker_set_time();
}

void orion_set_time( Orion * orion, double time ) {
    // obtain the lock
    pthread_mutex_lock( &(orion->lock) );

    // if the server is in real time mode, set it to static
    if ( orion->mode == ORION_MODE_REAL_TIME )
        orion->mode = ORION_MODE_ON;

    // set the time and release the lock
    tracker_set_time( &(orion->tracker), time );
    pthread_mutex_unlock( &(orion->lock) );
}

double orion_time( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    double time = orion->tracker.utc;
    pthread_mutex_unlock( &(orion->lock) );
    return time;
}

double orion_mark_time( Orion * orion ) {
    struct timeval time;
    gettimeofday( &time, NULL ); // UTC timestamp in unix epoch
    double julian_hours = (time.tv_sec + (time.tv_usec / 1000000.0)) / 3600.0;
    double last_time = orion->tracker.utc;
    orion->tracker.utc = julian_hours;
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

MIDC01 * create_tracking_message( Orion * orion, MIDC01 * midc01 ) {
    double zd, az;

    // either allocate of initialize the provided pointer
    if(midc01)
        memset(midc01, 0, sizeof(MIDC01));
    else
        midc01 = calloc( 1, sizeof(MIDC01));

    // Tracking Message id
    midc01->midc = TATS_TRK_DATA;

    // we assume this simlator is posing as a RIU
    midc01->riu_sensor_id = (TATS_TIDC_RIU & orion->id);

    // compute timer value from sensor time...
    unsigned short int milliseconds = (unsigned short int)
            (1000 * fmod( orion->tracker.utc * SECONDS_IN_DAY, 1.0 ) );
    midc01->tcn_time = milliseconds;

    // if there is an assigned coordinate
    if( orion->target.starnumber ) {

        // calculate the current location of the target
        tracker_to_horizon(&(orion->tracker), &(orion->target), &zd, &az);
        midc01->E = (int) (zd * 3600); // converting to arcseconds as a stopgap...
        midc01->F = (int) (az * 3600);
        midc01->G = 0;
        // TODO I need a proxy range...
        // TODO I need a local to EFG conversion...

        // set the tracker status
        midc01->track_status = (TATS_STATUS_SIM | TATS_STATUS_POS_DATA);
        // we may want to set quality flag, or acquiring flag depending on orion state...
    }

    // otherwise supply an idle message
    else {
        midc01->E = 0;
        midc01->F = 0;
        midc01->G = 0;
        midc01->track_status = (TATS_STATUS_AQUIRE);
    }

    // IFF code of the target
    midc01->symbol_type = TATS_NONPLAYER;
        // right now we're just calibrating stars...

    // For single target sensors, track ID is always zero
    midc01->track_id = 0;
        // we may eventually simulate multiple targets...

    // compute the checksum
    midc01->crc = 0;
    // TODO find a good CRC-16 implementation

    return midc01;
    //TODO check byte orders!
}

void orion_print_status(Orion * orion, FILE * file) {
    //        if (orion->target.starnumber) {
//            // calculate the current location of the target
//            tracker_to_horizon( &(orion->tracker), &(orion->target), &zd, &az);
//
//            // format the current time
//            time_t julian_seconds = (time_t)(3600.0 * orion->tracker.utc);
//            int microseconds = (int)(1000000*fmod( 3600.0 * orion->tracker.utc, 1.0 ));
//            struct tm * date = gmtime( &julian_seconds );
//            char time[32];
//            strftime( time , 32, "%Y/%m/%d %H:%M:%S\0", date );
//
//            // devise the tracking message
//            sprintf(buffer, "%s.%d %4s%ld %s (%lf, %lf)\n\0",
//                    time,
//                    microseconds,
//                    orion->target.catalog,
//                    orion->target.starnumber,
//                    orion->target.starname,
//                    az, zd);
//        } else { // otherwise send an idle message
//            sprintf(buffer, "IDLE\n\0");
//        }
}