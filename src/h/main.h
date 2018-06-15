//
// Created by Casey Shields on 6/15/2018.
//

#ifndef STARTRACK_MAIN_H
#define STARTRACK_MAIN_H

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

// some winsock constant I use
#ifndef WIN32
#define INVALID_SOCKET (unsigned int)(~0)
#define SOCKET_ERROR -1
#endif

/** Provides an interactive command line interface to the Orion server. */
int main( int argc, char * argv[] );

double get_time();
char* get_arg( int argc, char *argv[], char *name, char* default_value );
ssize_t get_input(char* prompt, char **line, size_t *size );
void configure_tracker( int argc, char* argv[], Tracker* tracker );
//void configure_address(int argc, char* argv[], struct sockaddr_in* address);
void configure_catalog( int argc, char* argv[], Catalog* catalog );
int search( Catalog * catalog, Tracker * tracker,
            double az_min, double az_max, double zd_min, double zd_max, float mag_min);

#endif //STARTRACK_MAIN_H
