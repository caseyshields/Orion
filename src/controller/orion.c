#include "controller/orion.h"

MIDC01 * create_tracking_message(Orion * orion, jday time, MIDC01 * midc01 );

Orion* orion_create( Orion * orion, unsigned short int id ) {
    // allocate if space isn't provided
    if (!orion)
        orion = malloc(sizeof(Orion));

    // initialize structure
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
    status = socket_load();
    if( status != 0 ) {
        error = "Failed to initialize winsock";
        goto EXIT;
    }

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
    status = socket_error();
    socket_close( orion->socket );

    CLEANUP_WINSOCK:
    socket_unload();

    EXIT:
    if (error)
        sprintf(orion->error, "[%d] %s\n", status, error);
    return status;
}

void * orion_control_loop( void * arg ) {
    Orion * orion = (Orion*)arg;
    int length = 0;
    char buffer[1024];
    memset(&buffer, 0, 1024);

    // sanity check
    assert( orion->mode != ORION_MODE_OFF );
    assert( orion->socket != INVALID_SOCKET );

    // loop indefinitely
    do {
        // obtain lock
        pthread_mutex_lock( &(orion->lock) );

        // check if the server has been stopped
        if (orion->mode == ORION_MODE_OFF)
            break;

        // calculate the expected time of arrival
        jday jd_tt = utc2tt( jday_now() ) + (orion->latency/SECONDS_IN_DAY);

        // create a tracking message
        MIDC01 * message = (void*) buffer;
        create_tracking_message(orion, jd_tt, message);
        length = sizeof( MIDC01 );

        // we no longer need tracker internals or mode, so we can release the lock
        pthread_mutex_unlock( &(orion->lock) );

        // send the tracking message
        int sent = send( orion->socket, buffer, length, 0 );

        // set error and exit if there was a transmission error
        if (sent < length) {
            sprintf(orion->error, "[%d] Failed to send entire message, sent %d bytes\n", socket_error(), sent);
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

    } while( 1 );

    // release the lock after we abort the control loop
    pthread_mutex_unlock( &(orion->lock) );

    return (void*) orion;
}

int orion_start( Orion * orion ) {
    // ensure the sensor is connected
    assert( orion->socket != INVALID_SOCKET );

    // check if server is already running
    pthread_mutex_lock( &(orion->lock) );
    if ( orion->mode != ORION_MODE_OFF ) {
        sprintf(orion->error, "[%d] %s\n", 1, "Server is already running.");
        pthread_mutex_unlock( &(orion->lock) );
        return 1;
    }

    // start the server control thread
    orion->mode = ORION_MODE_ON;
    pthread_create( &(orion->control), NULL, orion_control_loop, orion);
    pthread_mutex_unlock( &(orion->lock) );
    return 0;
} // helpful tutorial: https://www.cs.nmsu.edu/~jcook/Tools/pthreads/library.html

int orion_stop( Orion * orion ) {
    // obtain lock since we need to change the mode
    pthread_mutex_lock( &(orion->lock) );

    // abort if the server isn't running
    if (orion->mode == ORION_MODE_OFF) {
        sprintf(orion->error, "[%d] %s\n", 1, "server is not running");
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
// TODO add some better error handling than asserts!
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

    // close the socket
    socket_close( orion->socket );
    socket_unload();
}

double orion_get_latency( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    double latency = orion->latency;
    pthread_mutex_unlock( &(orion->lock) );
    return latency;
}

void orion_set_latency( Orion * orion, double latency ) {
    pthread_mutex_lock( &(orion->lock) );
    orion->latency = latency;
    pthread_mutex_unlock( &(orion->lock) );
}

void orion_set_location( Orion * orion, double lat, double lon, double h ) {
    pthread_mutex_lock( &(orion->lock) );
    orion->tracker.site.latitude = lat;
    orion->tracker.site.longitude = lon;
    orion->tracker.site.height = h;
    pthread_mutex_unlock( &(orion->lock) );
}

void orion_set_weather( Orion * orion, double celsius, double millibars ) {
    pthread_mutex_lock( &(orion->lock) );
    orion->tracker.site.temperature = celsius;
    orion->tracker.site.pressure = millibars;
    pthread_mutex_unlock( &(orion->lock) );
}

Tracker orion_get_tracker( Orion * orion ) {
    Tracker tracker;
    pthread_mutex_lock( &(orion->lock) );
    memcpy( &tracker, &(orion->tracker), sizeof( Tracker ) );
    pthread_mutex_unlock( &(orion->lock) );
    return tracker;
}

Entry orion_get_target( Orion * orion ) {
    Entry entry;
    pthread_mutex_lock( &(orion->lock) );
    memcpy( &entry, &(orion->target), sizeof(Entry) );
    pthread_mutex_unlock( &(orion->lock) );
    return entry;
}

void orion_set_target( Orion * orion, const Entry * entry ) {
    pthread_mutex_lock( &(orion->lock) );
    memcpy( &(orion->target), entry, sizeof(Entry) );
    pthread_mutex_unlock( &(orion->lock) );
}

int orion_is_connected (Orion * orion) {
    return orion->socket != INVALID_SOCKET;
}

int orion_get_mode ( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    int mode = orion->mode;
    pthread_mutex_unlock( &(orion->lock) );
    return mode;
}

void orion_clear_error( Orion * orion ) {
    pthread_mutex_lock( &(orion->lock) );
    memset( &(orion->error), 0, 128);
    pthread_mutex_unlock( &(orion->lock) );
}

MIDC01 * create_tracking_message( Orion * orion, jday jd_tt, MIDC01 * midc01 ) {
    // either allocate or initialize the provided pointer
    if(midc01)
        memset(midc01, 0, sizeof(MIDC01));
    else
        midc01 = calloc( 1, sizeof(MIDC01));

    // Tracking Message id
    midc01->midc = TATS_TRK_DATA;

    // we assume this simulator is posing as a RIU
    midc01->sensor_type = TATS_TIDC_RIU;
    midc01->sensor_id = orion->id;

    // compute timer value from sensor time...
    unsigned short int milliseconds = (unsigned short int)
            (1000 * fmod( jd_tt * SECONDS_IN_DAY, 1.0 ) );
    midc01->tcn_time = milliseconds;

    // if there is an assigned target
    if( orion->target.novas.starnumber ) {

        // calculate the current location of the target
        tracker_point( &(orion->tracker), jd_tt, &(orion->target.novas) );
        double efg[3];
        tracker_get_direction( &(orion->tracker), efg);
        midc01->E = (int)efg[0];
        midc01->F = (int)efg[1];
        midc01->G = (int)efg[2];

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
    midc01->symbol_type = 0; // TATS_NONPLAYER;
        // not an IFF system right now we're just calibrating stars...

    // For single target sensors, track ID is always zero
    midc01->track_id = (unsigned short int) orion->target.novas.starnumber;
        // but we're going to put in the catalog ID, since we technically can track any one
        // watch for overflow!

    // compute the checksum
    midc01->crc = crc16( (char*)midc01, sizeof(MIDC01)-2 );

    return midc01;
}
