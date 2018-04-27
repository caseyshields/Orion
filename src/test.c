#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "novas.h"
#include "FK6.h"
#include "StarMap.h"
/*include "StarPlot.h"
include <GL/glut.h>*/

#define N_STARS 3
#define N_TIMES 4

int main (void) {

    // this is the current offset between atomic clock time and time derived from Earth's orientation
    double ut1_utc = 0.108644; // (UT1-UTC) obtained from IERS Apr 26 2018

    // Difference between TAI and UTC, an integral number of leap second
    double leap_secs = 37.000000; // delta AT, obtained from IERS Apr 26 2018

    // get gmt time which is expressed in UTC
    time_t unix_time;
    time( &unix_time ); // GMT seconds since January 1970 0:00
    struct tm *utc = gmtime( &unix_time ); // GMT in calendar notation

    // TODO we should get a higher resolution timestamp, possibly from NTP?

    // create the star map
    StarMap map;// = malloc(sizeof(StarMap));
    StarMap_create(&map, utc, ut1_utc, leap_secs );

    // open and load the raw catalog file
    FILE *file = fopen("../data/FK6.txt", "r");
    assert(file);
    FK6_loadMap(878, file, &map);
    assert( &map );

    // update catalog entries topocentric position
    StarMap_update( &map );

    // dump map properties to screen
    StarMap_printTime( &map );
    StarMap_printSite( &map );
    StarMap_printMap( &map );

   return 0;
}