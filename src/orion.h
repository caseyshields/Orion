//
// Created by Casey Shields on 6/6/2018.
//

#ifndef STARTRACK_ORION_H
#define STARTRACK_ORION_H

// we only allow one controller and one sensor
#define MAX_CONNECTIONS 2

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

// some winsock crap if you're trying to figure this out on posix;
//define INVALID_SOCKET (unsigned int)(~0)
//define SOCKET_ERROR -1

// ORION program state
Tracker * tracker = NULL; // a representation of the sensor being controlled
unsigned int server = INVALID_SOCKET; // socket for giving targets to the tracker
unsigned int client = INVALID_SOCKET; // socket for the actual sensor
//unsigned int sensor = INVALID_SOCKET;

// ORION methods
Tracker * configure_tracker( int argc, char *argv[] );

unsigned int configure_server( int argc, char *argv[] );

double get_time();

char* get_arg( int argc, char *argv[], char *name, char* default_value );

void terminate( int status, char* msg );

#endif //STARTRACK_ORION_H
