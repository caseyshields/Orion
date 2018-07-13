#ifndef STARTRACK_ORION_H
#define STARTRACK_ORION_H

#include <pthread.h>
#include "h/catalog.h"
#include "h/tracker.h"
#include "h/sockets.h"

#define ORION_MODE_OFF 0

#define ORION_MODE_ON 1

#define ORION_MODE_REAL_TIME 2

#define ORION_RATE 1
//todo eventually put it to 50 hertz...

/** The Orion server connects to a TATS sensor over TCP and aims it at a selected celestial target.
 * This incarnation runs the control loop in a thread, using a mutex to synchronise access to the
 * server controls.
 * @author Casey Shields
 */
typedef struct {

    /** A representation of the sensor being controlled */
    Tracker tracker;

    /** Current target of the tracker */
    cat_entry target;

    /** Address of the sensor */
    struct sockaddr_in address;

    /** Socket for the sensor */
    unsigned int socket;

    /** Used to prevent race conditions between main loop and ui thread */
    pthread_mutex_t lock;

    /** Thread which runs the sensor control loop */
    pthread_t control;

    /** Current state of the control thread. May equal ORION_MODE_ON or ORION_MODE_OFF. */
    volatile int mode;

    /** Update rate of the control thread */
    unsigned int rate;

    /** An error buffer for messages from a failed method */
    char error[128]; // error message set if a server method fails

} Orion;
// todo I should probably just make every method thread-safe to ease use

/** Instantiates the given Orion structure, allocating space if the argument is NULL.
 * @param orion pointer to structure to instantiate, or NULL if a structure should be allocated
 * @return a pointer to the instantiated structure regardless if it was allocated. */
Orion * orion_create( Orion * orion );

/** Connect a socket to the specified sensor using TCP.
 * @param orion pointer to an initialized orion structure
 * @param ip a string holding the ip address of the sensor in dotted quad notation
 * @param port the port of the sensor's socket
 * @return 0 on success, 1 otherwise. Check error buffer after a failure for cause. */
int orion_connect( Orion * orion, char * ip, unsigned short port );

/** Starts the main control loop of the Orion sensor in a separate thread. Thread safe.
 * @param orion Pointer to a connected orion server
 * @return 0 on success, 1 on failure. Check error buffer after a failure for cause. */
int orion_start( Orion * orion );

/** Specify a target which the control thread will instruct the sensor to track. Thread safe.
 * @param target A novas structure holding the celestial location of the target. */
void orion_track( Orion * orion, cat_entry target );

/** Safely stops the control loop and blocks until it returns. Thread safe. */
int orion_stop( Orion * orion );

/** Disconnects the socket and disposes of resources. */
void orion_disconnect( Orion * orion );

/* Frees anciliary structures allocated by orion. */
//int orion_free( Orion * orion );

/** Best guess of whether the server is running. Thread safe.
 * @return True if the orion server was running recently. False otherwise.*/
int orion_is_running( Orion * orion );

/** @return True if the sensor is connected to the orion server. */
int orion_is_connected( Orion * orion );

/** gets a millisecond accurate timestamp from the system, converts it to Julian hours(see Novas
 * 3.1 documentation), then sets the current tracker time.
 * @returns the last marked timestamp.*/
double orion_mark_time( Orion * orion );

/** Clears the internal error buffer. Thread safe. */
void orion_clear_error( Orion * orion);

double orion_time( Orion * orion );

#endif //STARTRACK_ORION_H

