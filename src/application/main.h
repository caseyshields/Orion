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
#include "util/jday.h"
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
#define UT1_UTC "0.06496"
// updated from IERS predictions for 8/27
//"0.06809"

/** delta AT, Difference between TAI and UTC. Obtained from IERS June 20 2018 */
#define TAI_UTC "37.000000"

/** A positive time bias to account for the network latency between Orion and it's slaved TATS sensor. */
#define LATENCY "0.0"

#define LOCALHOST "127.0.0.1"

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
    Orion * orion;

    /** This time is used for catalog commands such as search and report. It does not affect the time broadcasted to
     * TATS sensors. Only the current time is used for that. */
    jday time;

    /** The Star catalog which the user can search for star targets */
    Catalog * catalog;

    /** A database of earth orientation parameters, searchable by time*/
    IERS * iers;

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
void configure_orion(int argc, char *argv[], Orion * orion);

/** Loads IERS Bulletin A data file */
void configure_iers(int argc, char * argv[], IERS * iers );

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

//tracker->jd_tt = J2000_EPOCH;
//// Novas typically deals with the sum of the offsets, might want to cache it...
////map->delta_t = 32.184 + map->leap_secs - map->ut1_utc;
////jday orion_get_time(Orion *orion) {
////    pthread_mutex_lock( &(orion->lock) );
////    double time = tt2utc( tracker_get_time( &(orion->tracker) ) );
////            // = orion->tracker.utc;
////    pthread_mutex_unlock( &(orion->lock) );
////    return time;
////}
//jday tracker_get_time(Tracker *tracker) {
//    return tracker->jd_tt;
//}
//
//void tracker_set_time( Tracker * tracker, jday jd_tt ) {
//    tracker->jd_tt = jd_tt;
//}
//void tracker_print_time( const Tracker *tracker, FILE * file ) {
//    // format the time
//    char * stamp = jday2stamp( tracker->jd_tt );
//    fprintf( file, "time:\t%s UTC\t(%+05.3lf UT1)\n", stamp, tracker->jd_tt );
//
//    // do we want to expose any other time conventions?
////    double utc = tracker_get_UTC(tracker);
////    double ut1 = tracker_get_UT1(tracker);
////    double tt = tracker_get_TT(tracker);
////    fprintf( file, "UTC : %s\nUT1 : %s\nTT  : %s\n",
////             jday2stamp(utc),
////             jday2stamp(ut1),
////             jday2stamp(tt)
////    );
//
//    fflush( file );
//}