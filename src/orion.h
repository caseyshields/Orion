//
// Created by Casey Shields on 6/6/2018.
//

#ifndef STARTRACK_ORION_H
#define STARTRACK_ORION_H

#include <pthread.h>
#include "catalog.h"

// default latitude of sensor in degrees
#define LATITUDE "38.88972222222222"

// default longitude of sensor in degrees
#define LONGITUDE "-77.0075"

// default geodetic height of sensor in meters
#define HEIGHT "125.0"

// default site temperature in degrees celsius
#define TEMPERATURE "10.0"

// default atmospheric pressure at site in millibars
#define PRESSURE "1010.0"

// (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
#define UT1_UTC "0.108644"

// delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
#define TAI_UTC "37.000000"


typedef struct {
    Tracker tracker; // a representation of the sensor being controlled
    Catalog catalog; // A catalog of stars to choose targets from
    struct sockaddr_in address; // address of the sensor
    unsigned int client; // socket for the sensor
    pthread_t control; // thread which runs the control loop for the sensor
} Orion;

/** extracts tracker information from the program arguments and constructs a model of a tracker */
void configure_tracker( int argc, char* argv[], Tracker* tracker );

void configure_address(int argc, char* argv[], struct sockaddr_in* address);

void configure_catalog( int argc, char* argv[], Catalog* catalog );

void * orion_start_sensor( void * data );

double get_time();

char* get_arg( int argc, char *argv[], char *name, char* default_value );

// some winsock crap if you're trying to figure this out on posix;
//define INVALID_SOCKET (unsigned int)(~0)
//define SOCKET_ERROR -1

// cleans up orion program resources and exits the program, emmiting a message in msg is non-null.
//void orion_terminate( Orion* orion, int status, char* msg );


#endif //STARTRACK_ORION_H
