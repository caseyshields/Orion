#ifndef STARTRACK_MAIN_H
#define STARTRACK_MAIN_H

#include <signal.h>

#include "util/io.h"
#include "util/sockets.h"
#include "engine/tracker.h"
#include "engine/catalog.h"
#include "controller/orion.h"
#include "application/test.h"

/*!
\mainpage Orion

 Orion is a command line application for finding and tracking stars with TATS sensors.
 @author Casey Shields

---

\section building Building Orion

Orion is built using a CMake script(./CMakeLists.txt) which should run on either windows or Unix platforms.
The script produce two executables;

 - sensor : An server meant to mimic a TATS sensor.
 - orion : A command line application which controls an Orion server.

---

\section running Running Orion

Orion's main entry point is in main.c.
The configuration is provided by command line arguments;

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

 if orion is run with the 'test' flag, the test suit is run.

 > ./orion -test

 This can be used to diagnose some types of problems in production.

 Otherwise, once started Orion enters an interactive command line mode which accepts
 the following commands;

 ---
\section configuration Configuration Commands

\subsection time time <year>/<month>/<day> <hour>:<min>:<second>
Sets the current time of the Orion server.
  - year : the 4 digit year
  - month : 1 to 12
  - day : 1 to 31
  - hour : 1 to 24
  - min : 0 to 59
  - second : 0.000000 to 61.999999
### Example
> time 2000/1/1 12:0:0

\subsection location location <latitude> <longitude> <height>
 Sets the tracker's location on earth
  - latitude : decimal degrees latitude, -90.0 to 90.0
  - longitude : decimal degrees longitude, 0.0 to 360.0
  - height : ellipsoidal height in meters

\subsection weather weather <temperature> <pressure>
 sets the local weather conditions for use in refraction calculations
  - temperature : in degrees celsius
  - pressure : in integer millibars

---
\section catalog Catalog Commands

\subsection name name <substring>
 Search the loaded catalog for all stars whoose Bayer-Flamesteed designation contains the given substring.

\subsection search search <min magnitude> [<min az> <max az> <min zd> <max zd>]
Searches through the catalog for all bright stars currently within the given
patch of sky in local horizon coordinates.

 ---
\section sensor Sensor Commands

\subsection connect connect [<ip>:<port>]
Connects to the TATS sensor, using the default address if not supplied
 - ip : IPv4 address of TATS sensor in dotted quad notation
 - port : port number the sensor is listening on

\subsection target target <FK6 ID>
Sets a new target which the Orion server will direct the TATS sensor at.
The FK6 ID can be obtained through the catalog search commands.

\subsection exit exit
Closes the sensor connection, shuts down the Orion server, releases the
catalogs, and exits the program.

 ---
\section diagnostic Diagnostic Commands

\subsection status status
Prints the current status of the orion server to the screen, including the control thread state, tracker location, time, current target, and example tracking message

\subsection report report <step> <count>
 Using the current time, location, weather, and target, a report is generated describing the targets
 apparent location over time in local coordinates
 - step : amount to increase time each step
 - count : number of steps to take

*/

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

typedef struct {

/** Current mode of the user interface,  */
    volatile int mode;

/**  */
    unsigned short port;

/**  */
    char *ip;

/** The Orion server which steers a TATS sensor at a designated star */
    Orion *orion;

/** The Star catalog which the user can search for star targets */
    Catalog *catalog;

} Application;

/** Provides an interactive command line interface to the Orion server. */
int main(int argc, char *argv[]);

void interrupt_handler(int signal);

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

/** Currently just hardcoded to load the first and third parts of the FK6 catalog from the default data directory. */
int cmd_load(Catalog *catalog);

int cmd_name(char *line, Catalog *catalog);
//int cmd_search( char * line, Orion * orion, Catalog * catalog );

int cmd_connect(char *line, Orion *orion);

int cmd_target(char *line, Orion *orion, Catalog *catalog);

int cmd_report(char *line, Orion *orion, FILE *stream);

int cmd_help(char *line);

/** Transforms the catalog into local coordinates using the tracker, then filters them by the given criteria.
 * The results are then printed to stdout.
 * @return The number of stars fitting the criteria*/
int search(Catalog *catalog, Tracker *tracker,
           double az_min, double az_max, double zd_min, double zd_max, float mag_min);

/** produces a tsv report of a targets coordinates over the specified time interval
 * @param tracker
 * @param target a catalog entry of a desired celestial target
 * @param start the beginning time of the report in UTC
 * @param step the number of fractional seconds to increment the report time by
 * @param count the total number of steps to take
 * @param stream the file to write the report to*/
void report(Tracker *tracker, Entry *target, jday start, double step, int count, FILE *stream);

#endif //STARTRACK_MAIN_H

//Henderson Location:  W114°58'48.0", N36°03'00.0",     0m