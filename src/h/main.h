//
// Created by Casey Shields on 6/15/2018.
//

#ifndef STARTRACK_MAIN_H
#define STARTRACK_MAIN_H

/** default latitude of sensor in degrees */
#define LATITUDE "38.88972222222222"

/** default longitude of sensor in degrees */
#define LONGITUDE "-77.0075"

/** default geodetic height of sensor in meters */
#define HEIGHT "125.0"

/** default site temperature in degrees celsius */
#define TEMPERATURE "10.0"

/** default atmospheric pressure at site in millibars */
#define PRESSURE "1010.0"

/** (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation */
#define UT1_UTC "0.108644"

/** delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018 */
#define TAI_UTC "37.000000"

#ifndef WIN32
/** some winsock constants I use */
#define INVALID_SOCKET (unsigned int)(~0)
#define SOCKET_ERROR -1
#endif

/*!
\mainpage Orion

\section intro_sec Introduction

Star hunting software. Contains a planning component for loading astrometric catalogs and selecting suitable stars.
The other component is a tracker which transforms catalog coordinates into a useable format and broadcasts them to client(s) over TATS.
The [Novas 3.1](http://aa.usno.navy.mil/software/novas/novas_info.php) library is used to perform the transformations.
The first part of [FK5](http://www-kpno.kpno.noao.edu/Info/Caches/Catalogs/FK5/fk5.html) or [FK6](http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/264) is currently used as a catalog.
The front end is still in the conceptual phases, however a [UI experiment](https://caseyshields.github.io/starlog/index.html) has been written in [D3](https://d3js.org/).

\section component_sec Components
 - vmath : utility library of vector and spherical math routines
 - novas : USNO's astrometric software package
 - fk6 : loads raw FK6 catalogs into a catalog
 - catalog : module for filtering and sorting desired stars.
 - tracker : performs coordinate transforms according to the current time and location on earth
 - sensor : a dummy server which simulates a slaved sensor
 - orion : main program which integrates all features

\subsection vmath VMath
##vmath

## Requirements
 - Orion is developed in CLion using MinGW
*/

/** Provides an interactive command line interface to the Orion server. */
int main( int argc, char * argv[] );

void configure_tracker( int argc, char* argv[], Tracker* tracker );

void configure_catalog( int argc, char* argv[], Catalog* catalog );

int search( Catalog * catalog, Tracker * tracker,
            double az_min, double az_max, double zd_min, double zd_max, float mag_min);

double get_time();

char* get_arg( int argc, char *argv[], char *name, char* default_value );

ssize_t get_input(char* prompt, char **line, size_t *size );

#endif //STARTRACK_MAIN_H
