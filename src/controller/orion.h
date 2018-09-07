/** @file orion.h
 * @brief Server which asynchronously broadcasts apparent coordinates of celetial targets to a TATS sensor.
 * @author Casey Shields */

#ifndef STARTRACK_ORION_H
#define STARTRACK_ORION_H

#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#include "util/sockets.h"
#include "util/crc.h"
#include "util/jday.h"
#include "data/tats.h"
#include "engine/catalog.h"
#include "engine/tracker.h"

#define ORION_MODE_OFF 0

#define ORION_MODE_STATIC 1

#define ORION_MODE_REAL_TIME 2

#define ORION_MODE_KILL_SIM 3

#define ORION_RATE 1
//todo eventually put it to 50 hertz...

/** The Orion server connects to a TATS sensor over TCP and aims it at a selected celestial target.
 * This incarnation runs the control loop in a thread, using a mutex to synchronise access to the
 * server controls.
 * @author Casey Shields
 */
typedef struct {

    /** A RIU id number for the simulator to emit */
    unsigned short id;

    /** A representation of the sensor being controlled */
    Tracker tracker;

    /** Current target of the tracker */
    Entry target;

    /** Address of the sensor */
    struct sockaddr_in address;

    /** Socket for the sensor */
    unsigned int socket;

    /** Used to prevent race conditions between control loop and ui thread */
    pthread_mutex_t lock;

    /** Thread which runs the sensor control loop */
    pthread_t control;

    /** Current state of the control thread. May equal ORION_MODE_ON or ORION_MODE_OFF. */
    volatile int mode;

//    jday jd_tt;

    /** Update rate of the control thread */
    unsigned int rate;

    /** Estimate of TATS control network latency. This positive bias is only applied to TATS messages */
    double latency;

    /** An error buffer for messages from a failed method */
    char error[128]; // error message set if a server method fails

} Orion;
// todo I should probably just make every method thread-safe to ease use

/** Terrestrial time is meant to be a smooth timescale and is derived from UTC by removing leap seconds and adding an
 * experimentally measured offset available from the IERS service, which combines data from the Lunar Laser Ranger,
 * Very long baseline radio inferometry of quasars, the GPS constellation, etc.
 * @return The tracker's current Terrestrial Time in Julian day format. */
jday tracker_get_time(Tracker *tracker);

/** Sets the current time for the star tracker.
 * @param tracker
 * @param utc The desired tracker's Terrestrial Time in julian day format */
void tracker_set_time(Tracker *tracker, jday jd_tt);

void tracker_print_time(const Tracker *tracker, FILE * file);

/** Instantiates the given Orion structure, allocating space if the argument is NULL.
 * @param orion pointer to structure to instantiate, or NULL if a structure should be allocated
 * @param id A sensor ID for the Orion server to use when comunicating with Tats devices
 * @return a pointer to the instantiated structure regardless if it was allocated. */
Orion * orion_create( Orion * orion, unsigned short int id );

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

/** Safely stops the control loop and blocks until it returns. Thread safe. */
int orion_stop( Orion * orion );

/** Disconnects the socket and disposes of resources. */
void orion_disconnect( Orion * orion );

/** Best guess of whether the server is running. Thread safe.
 * @return True if the orion server was running recently. False otherwise.*/
int orion_get_mode( Orion * orion );

/** @return True if the sensor is connected to the orion server. */
int orion_is_connected( Orion * orion );

/** Clears the internal error buffer. Thread safe. */
void orion_clear_error( Orion * orion);

///** gets a millisecond accurate timestamp from the system, converts it to Julian hours(see Novas
// * 3.1 documentation), then sets the current tracker time.
// * @returns the last marked timestamp.*/
//jday orion_set_time( Orion *orion, jday time );
//
///** @return the current julian date in UTC in days @see jday.h */
//jday orion_get_time( Orion *orion );

/** @return the configured TATS control network latency in seconds.
 * note: Something to keep in mind is it should be possible to estimate latency by polling the sensor.
 * For right now though we just manually configure it... */
double orion_get_latency( Orion * orion );

/** Sets the TATS control network latency. */
void orion_set_latency( Orion * orion, double latency );

/** @return a thread-safe copy of the orion server's current tracker */
Tracker orion_get_tracker( Orion * orion );

/** sets the location of the tracker on earth
 * @param orion
 * @param lat Latitude in degrees
 * @param lon Longitude in degrees
 * @param h Height in meters*/
void orion_set_location( Orion * orion, double lat, double lon, double h );

/** Sets the local weather conditions of the tracker for Novas' reraction calculation.
 * @param orion
 * @param celsius Temperature in degrees Celsius
 * @param millibars Air pressure in millibars. 1 Atmosphere of pressure is approximately 1010 millibars. */
void orion_set_weather( Orion * orion, double celsius, double millibars );

/** @return a thread safe copy of the current target of the Orion server. */
Entry orion_get_target( Orion * orion );

/** Specify a target which the control thread will instruct the sensor to track. Thread safe.
 * @param orion The server which will track the target
 * @param target A novas structure holding the celestial location of the target. */
void orion_set_target( Orion * orion, const Entry * entry );

#endif //STARTRACK_ORION_H

