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
#define UT1_UTC "0.06809"

/** delta AT, Difference between TAI and UTC. Obtained from IERS June 20 2018 */
#define TAI_UTC "37.000000"

/*!
\mainpage Orion

\section building Building Orion

 Orion is built using a CMake script(./CMakeLists.txt) which produces a number of executables;

  - checkout : The test program packaged with Novas for testing the low accuracy mode. Output should match './data/novas/checkout-stars-usno.txt'.
  - tester : An interactive command line program for running benchmarks and testing features. Only for development use.
  - sensor : An server meant to mimic a TATS sensor. Will interact with orion if ip/port is configured.
  - orion : The Orion server with a concurrent command-line interface.

 The orion project is built using MinGW tools and libraries. MinGW is a very lightweight and thin
 API, however it is not meant to be a full posix implementation. This leads to some difficulty
 linking socket and threading code, as well as limiting Orion to a 32 bit executable. Since
 efficiency doesn't appear to be a bottleneck, The cygwin toolchain will be investigated to improve
 portability to linux.

\section running Running Orion

 Orion's main entry point is in main.c. Configuration of sensor location, atmospheric conditions and so forth, is
 provided by command line arguments;

 - latitude [degrees] : geodetic location of sensor
 - longitude [degrees]
 - height [meters]
 - ip [000.000.000.000] : ipv4 address of TATS sensor
 - port [20000-60000] : port number of TATS sensor
 - temperature [celsius] : temperature at sensor location
 - pressure [millibars] : atmospheric pressure at sensor
 - UT1_UTC [fractional seconds] : current offset between TAI and TT
 - TAI_UTC [integer seconds] : Difference between TAI and UTC
 - catalog [path] : A path to the FK6 dataset to be loaded.

 If not specified they will revert to default values defined in main.c.

\section using Using Orion
 Once started Orion enters an interactive command line mode which accepts
 the following commands;

\subsection search Search <min magnitude> <min az> <max az> <min zd> <max zd>
 Searches through the catalog for all bright stars currently within the given
 patch of sky in local horizon coordinates.

\subsection start Start
Connects to the TATS sensor, and starts the server.

\subsection track Track <FK ID>
Sets a new target which the Orion server will direct the TATS sensor at

\subsection exit Exit
Closes the sensor connection, shuts down the Orion server, releases the
 catalogs, and exits the program.

*/

/** Provides an interactive command line interface to the Orion server. */
int main( int argc, char * argv[] );

/** Builds a tracker object using the given commandline arguments. */
void configure_tracker( int argc, char* argv[], Tracker* tracker );

/** Builds a catalog using the given commandline arguments */
void configure_catalog( int argc, char* argv[], Catalog* catalog );

/** Transforms the catalog into local coordinates using the tracker, then filters them by the given criteria.
 * The results are then printed to stdout.
 * @return The number of stars fitting the criteria*/
int search( Catalog * catalog, Tracker * tracker,
            double az_min, double az_max, double zd_min, double zd_max, float mag_min);

#endif //STARTRACK_MAIN_H

//Henderson Location:  W114°58'48.0", N36°03'00.0",     0m