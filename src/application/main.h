/** @file main.h
 * @brief Command line application for interacting with Orion components.
 *
 * The main application first sets parameters using arguments and defaults, loads catalogs and trackers,
 * then enters an interactive command line mode.
 * */


#ifndef STARTRACK_MAIN_H
#define STARTRACK_MAIN_H

#include <signal.h>

#include "util/io.h"
#include "util/sockets.h"
#include "engine/tracker.h"
#include "engine/catalog.h"
#include "controller/orion.h"
#include "application/test.h"

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

/** The Application structure holds all components of a interactive command line application which
 * interfaces with the Orion server and associated star catalogs. */
typedef struct {

    /** Flag for whether the UI loop should be running.  */
    volatile int mode;

    /** Slaved TCN sensor ip address in doted quad notation. */
    char *ip;

    /** Slaved TCN sensor port. */
    unsigned short port;

    /** The Orion server which steers a TATS sensor at a designated star */
    Orion *orion;

    /** The Star catalog which the user can search for star targets */
    Catalog *catalog;

} Application;

/** Application entry point. After initialization, the program enters an interactive loop which
 * interprets user commands. This allows the user to alter configuration, search star catalogs,
 * drive TCN sensors, and perform various diagnostics. */
int main(int argc, char *argv[]);

/** Sets a flag for the main loop to exit. */
void interrupt_handler(int signal);

/** Releases all application resources.  */
void cleanup();

/** Builds a tracker object using the given commandline arguments. */
void configure_tracker(int argc, char *argv[], Tracker *tracker);

/** Builds a catalog using the given commandline arguments */
void configure_catalog(int argc, char *argv[], Catalog *catalog);

/** Creates an ip address for the sensor from the arguments. */
void configure_address(int argc, char *argv[], Application * app);

int cmd_time(char *time, Orion *orion);

int cmd_location(char *line, Orion *orion);

int cmd_weather(char *line, Orion *orion);

int cmd_name(char *line, Catalog *catalog);

/** Transforms the catalog into local coordinates using the tracker, then filters them by the given criteria.
 * The results are then printed to stdout.
 * @return The number of stars fitting the criteria*/
int cmd_search( char * line, Orion * orion, Catalog * catalog );

int cmd_connect(char *line, Orion *orion);

int cmd_target(char *line, Orion *orion, Catalog *catalog);

/** produces a tsv report of a targets coordinates over the specified time interval */
int cmd_status(char *line, Orion * orion, FILE * stream);

int cmd_report(char *line, Orion *orion, FILE *stream);

int cmd_help(char *line);

#endif //STARTRACK_MAIN_H

//Henderson Location:  W114°58'48.0", N36°03'00.0",     0m