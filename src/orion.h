//
// Created by Casey Shields on 6/6/2018.
//

#ifndef STARTRACK_ORION_H
#define STARTRACK_ORION_H

#include <pthread.h>
#include "catalog.h"

/** A tracker controller which issues TATS commands over ethernet, given star information.
 * @author Casey Shields
 */

typedef struct {
    Tracker tracker; // a representation of the sensor being controlled
    cat_entry target; // current target of the tracker

    struct sockaddr_in address; // address of the sensor
    unsigned int client; // socket for the sensor

    volatile int mode; // current state of the control thread
    pthread_t control; // thread which runs the control loop for the sensor
    pthread_mutex_t mutex;

    char error[128]; // error message set if a server method fails
} Orion;

Orion * orion_create( Orion * orion );

int orion_connect( Orion * orion, char * ip, unsigned short port );

int orion_start( Orion * orion );//int * mode, pthread_t * control, Tracker * tracker);

int orion_track( cat_entry target );

int orion_stop(int * mode, pthread_t * control);

int orion_disconnect( );

#endif //STARTRACK_ORION_H

