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
//todo eventually put it to 50...

#ifdef WIN32
#define SLEEP_RESOLUTION 1000
#else
#define SLEEP_RESOLUTION 1000000
#endif

/** A tracker controller which issues TATS commands over ethernet, given star information.
 * @author Casey Shields
 */
typedef struct {
    Tracker tracker; // a representation of the sensor being controlled
    cat_entry target; // current target of the tracker

    struct sockaddr_in address; // address of the sensor
    unsigned int client; // socket for the sensor

    pthread_mutex_t mutex;
    pthread_t control; // thread which runs the control loop for the sensor
    volatile int mode; // current state of the control thread
    int rate;

    char* error; // error message set if a server method fails
} Orion;

int orion_is_running( Orion * orion );

int orion_is_connected( Orion * orion );

char* orion_poll_error( Orion * orion);

Orion * orion_create( Orion * orion );

int orion_connect( Orion * orion, char * ip, unsigned short port );

int orion_start( Orion * orion );

int orion_track( cat_entry target );

int orion_stop( Orion * orion );

int orion_disconnect( Orion * orion );

#endif //STARTRACK_ORION_H

