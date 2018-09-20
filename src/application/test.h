/** @file test.h
 * @brief A test suite which invokes modules' unit tests, as well as higher level integration tests.
 * @author Casey Shields */

#ifndef STARTRACK_TEST_H
#define STARTRACK_TEST_H

#include <assert.h>
#include "util/crc.h"
#include "util/vmath.h"
#include "util/jday.h"
#include "util/io.h"
#include "data/tats.h"
#include "data/iers.h"
#include "engine/tracker.h"
#include "engine/catalog.h"
#include "../lib/cutest-1.5/CuTest.h"

/** Runs the entire test suite and prints the results to standard out.
 * Program then exits, returning the number of failed tests. */
void test_run();

/** @return a CuTest suite of tests for Orion. */
CuSuite * test_suite();

/** construct a catalog entry for Vega from Part I of the FK6 catalog;
<pre>
|  699 |  91262 | alpha Lyr          | 18 36 56.336939 |  +38 47  1.28333 | +00201.70 | +00286.67 |1990.93 |   0.46 |   0.21 |1990.98 |   0.45 |   0.20 | 128.93 |   0.55 |  H  | -013.5 |   0.03 |  1  |  23  |  2  |     | -0025.23 | -0005.80 | -0005.98 | -0072.68 | -0037.05 | +0006.73 | +0007.17 | -0161.31 | -0000.23 | -0000.65 | -0000.67 | -0000.82 | +0000.51 | -0000.41 | +0000.75 | +0000.80 | -0001.58 | +0000.50 |1959.60 |1991.00 |1991.00 |1941.72 |  10.79 |   0.46 |   0.46 |  13.50 |   0.30 |   0.56 |   0.57 |   0.41 |   0.27 |1954.47 |1991.03 |1991.03 |1920.23 |  12.47 |   0.45 |   0.45 |  17.33 |   0.26 |   0.59 |   0.60 |   0.39 |   0.24 | 128.89 | 128.93 | 128.93 |   0.54 |   0.55 |   0.55 | -0032.63 | -0003.13 | +0000.94 | +0001.01 |     9.48 |     6.90 |     0.29 |     0.18 |  1  |   3.36 |   1.88 |   3.85 |   5.25 | 430 |
</pre> */
Entry test_getVega();

/** construct earth orientation from the IERS bulletin entry;
<pre>
18 919 58380.00 P  0.207193 0.003944  0.344530 0.004292  P 0.0513092 0.0029860                 P     0.112    0.128     0.214    0.160
</pre> */
IERS_EOP test_getEarth2018Sep9();

/** https://www.google.com/maps/place/36%C2%B004'19.3%22N+115%C2%B008'03.7%22W/@36.0720393,-115.1344197,41m/data=!3m1!1e3!4m6!3m5!1s0x0:0x0!7e2!8m2!3d36.0720322!4d-115.13435?hl=en
<pre>
                    36°04'19.3"N 115°08'03.7"W
                    36.072032, -115.134350
</pre>
 turns out the google values don't match, there is less precision in the formatted string...
 @param eop the current earth orientation
 @return A tracker placed at the McCarren Viewing Area */
Tracker test_getMcCarrenTracker( IERS_EOP * eop );

/** An integration test which compares Orion results against USNO reference implementation results.
 * Data obtained from http://aa.usno.navy.mil/data/docs/topocentric.php
<pre>
                               Vega

                  Apparent Topocentric Positions
                    Local Zenith and True North

                   McCarren
         Location:  W115°08'03.7", N36°04'19.3",     0m
            (Longitude referred to Greenwich meridian)

   Date        Time                Zenith               Azimuth
        (UT1)                     Distance              (E of N)
             h  m   s              °  '   "             °  '   "
2018 Sep 19 12:00:00.0            98 00 38.3          332 18 58.5
2018 Sep 19 13:00:00.0           102 37 02.9          343 15 35.4
2018 Sep 19 14:00:00.0           104 54 29.9          355 04 57.1
2018 Sep 19 15:00:00.0           104 39 59.3            7 11 23.9
2018 Sep 19 16:00:00.0           101 54 57.7           18 53 07.5
2018 Sep 19 17:00:00.0            96 54 40.4           29 37 04.2
2018 Sep 19 18:00:00.0            90 02 15.2           39 07 06.0
2018 Sep 19 19:00:00.0            81 42 01.4           47 21 59.2
2018 Sep 19 20:00:00.0            72 15 25.1           54 28 43.5
2018 Sep 19 21:00:00.0            61 59 41.6           60 36 28.4
2018 Sep 19 22:00:00.0            51 08 12.4           65 52 22.2
2018 Sep 19 23:00:00.0            39 51 18.5           70 17 18.5
</pre>
    <p>The calculation is very long and the inputs are numerous, and their sources conflicting. Making
    troubleshooting very difficult. First you need to think about the magnitudes of the various
    effects;
    <ul>
    <li>Proper motion < 1 arcsecond/year(usu.much less)</li>
    <li>parallax < 1 arcsecond</li>
    <li>gravitational light bending < 0.05 arcseconds 10deg away from the sun</li>
    <li>aberration < 21 arcseconds</li>
    <li>refraction < 60 arcseconds @ 45deg, 1800 arcseconds at horizon</li>
    </ul></p>

    <p>We can isolate these by separating the calculation into celestial coordinates and local
    topocentric coordinates. If the error is found celestial coordinates, then the error is likely
    a difference of catalog. Errors in abberation, parallax, propermotion are unlikely, as these
    are really small effects. If the error is in topocentric horizon coordinates, we know the
    error is in earth orientation or site location. Refraction is also possible but you can turn
    that calculation off to isolate it.</p>

    <p>Another useful instrumentation could be looking at the angle or dot product of the topocentric
    star motion and the direction of the error. If the angle is always small, it means the error is likely
    in your time scale.</p>
 */
void test_prediction( CuTest * test );

/** Time how long it takes to point the tracker at every star in the catalog then prints the local coordinates. */
void test_benchmark( Catalog* catalog, Tracker* tracker, int trials );

#endif //STARTRACK_TEST_H
