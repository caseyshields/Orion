//
// Created by Casey Shields on 6/6/2018.
//

#ifndef STARTRACK_ORION_H
#define STARTRACK_ORION_H

#include <pthread.h>
#include "catalog.h"

#define ORION_MODE_OFF 0

#define ORION_MODE_ON 1

#define ORION_RATE 1
//todo eventually put it to 50 hertz...

#ifdef WIN32
#define SLEEP_RESOLUTION 1000
#else
#define SLEEP_RESOLUTION 1
#endif

/** A tracker controller which issues TATS commands over ethernet, given star information.
 * @author Casey Shields
 */
typedef struct {
    Tracker tracker; // a representation of the sensor being controlled
    cat_entry target; // current target of the tracker

    struct sockaddr_in address; // address of the sensor
    unsigned int socket; // socket for the sensor

    pthread_mutex_t lock; // used to prevent race conditions on both the mode and tracker
    pthread_t control; // thread which runs the control loop for the sensor
    volatile int mode; // current state of the control thread
    unsigned int rate;

    char error[128]; // error message set if a server method fails
} Orion;

Orion * orion_create( Orion * orion );

int orion_connect( Orion * orion, char * ip, unsigned short port );

int orion_start( Orion * orion );

void orion_track( Orion * orion, cat_entry target );

int orion_stop( Orion * orion );

void orion_disconnect( Orion * orion );

int orion_free( Orion * orion );

int orion_is_running( Orion * orion );

int orion_is_connected( Orion * orion );

/** gets a millisecond accurate timestamp from the system, converts it to Julian hours(see Novas
 * 3.1 documentation), then sets the current tracker time.
 * @returns the last marked timestamp.*/
double orion_mark_time( Orion * orion );

void orion_clear_error( Orion * orion);

#endif //STARTRACK_ORION_H

