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
</pre>
  Newer version;
<pre>
18 919 58380.00 I  0.212071 0.000091  0.348723 0.000091  I 0.0558907 0.0000075  0.0222 0.0279  P     0.111    0.128     0.219    0.160
</pre>
 */
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

                   Henderson
         Location:  W115°08'03.7", N36°04'19.3",     0m
            (Longitude referred to Greenwich meridian)

   Date        Time                Zenith               Azimuth
        (UT1)                     Distance              (E of N)
             h  m   s              °  '   "             °  '   "
2018 Sep 19 00:00:00.0            29 03 16.6           73 25 14.7
2018 Sep 19 01:00:00.0            17 21 12.4           74 25 47.5
2018 Sep 19 02:00:00.0             5 54 21.6           60 26 08.8
2018 Sep 19 03:00:00.0             7 14 20.0          294 43 39.0
2018 Sep 19 04:00:00.0            18 47 42.0          285 26 05.4
2018 Sep 19 05:00:00.0            30 29 13.5          286 54 56.1
2018 Sep 19 06:00:00.0            42 00 29.9          290 28 25.1
2018 Sep 19 07:00:00.0            53 13 17.5          295 03 24.9
2018 Sep 19 08:00:00.0            63 58 54.3          300 28 45.5
2018 Sep 19 09:00:00.0            74 06 29.8          306 46 58.5
2018 Sep 19 10:00:00.0            83 22 02.1          314 05 51.9
2018 Sep 19 11:00:00.0            91 27 27.9          322 34 32.6
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
    <p>The calculation is very long and the inputs are numerous, and their sources conflicting. This
    makes troubleshooting very difficult. First you need to think about the magnitudes of the
    various effects;
    <ul><li>Proper motion < 1 arcsecond/year(usu.much less)</li>
    <li>parallax < 1 arcsecond</li>
    <li>gravitational light bending < 0.05 arcseconds 10deg away from the sun</li>
    <li>aberration < 21 arcseconds</li>
    <li>refraction < 60 arcseconds @ 45deg, 1800 arcseconds at horizon</li>
    </ul></p>

    <p>We can begin to separate these by separating the calculation into celestial coordinates and local
    topocentric coordinates. If the error is found celestial coordinates, then the error is likely
    a difference of catalog. Errors in abberation, parallax, propermotion are unlikely, as these
    are really small effects. If the error is in topocentric horizon coordinates, we know the
    error is in earth orientation or site location. Refraction is also possible but you can turn
    that calculation off to isolate it.</p>

    <p>Another useful instrumentation could be looking at the angle or dot product of the topocentric
    star motion and the direction of the error. If the angle is always small, it means the error is likely
    in your time scale.</p>

    <p> Here are the topocentric celestial coordinates to try to separate the error source.</p>
<pre>
                               Vega

                  Apparent Topocentric Positions
                 True Equator and Equinox of Date

                   Henderson
         Location:  W115°08'03.7", N36°04'19.3",     0m
            (Longitude referred to Greenwich meridian)

   Date        Time           Right Ascension          Declination
        (UT1)
             h  m   s            h  m   s                °  '   "
2018 Sep 19 00:00:00.0          18 37 34.189          + 38 48 28.55
2018 Sep 19 01:00:00.0          18 37 34.191          + 38 48 28.59
2018 Sep 19 02:00:00.0          18 37 34.192          + 38 48 28.63
2018 Sep 19 03:00:00.0          18 37 34.190          + 38 48 28.68
2018 Sep 19 04:00:00.0          18 37 34.188          + 38 48 28.72
2018 Sep 19 05:00:00.0          18 37 34.184          + 38 48 28.76
2018 Sep 19 06:00:00.0          18 37 34.179          + 38 48 28.79
2018 Sep 19 07:00:00.0          18 37 34.173          + 38 48 28.81
2018 Sep 19 08:00:00.0          18 37 34.166          + 38 48 28.82
2018 Sep 19 09:00:00.0          18 37 34.159          + 38 48 28.82
2018 Sep 19 10:00:00.0          18 37 34.153          + 38 48 28.81
2018 Sep 19 11:00:00.0          18 37 34.147          + 38 48 28.79
2018 Sep 19 12:00:00.0          18 37 34.142          + 38 48 28.76
2018 Sep 19 13:00:00.0          18 37 34.138          + 38 48 28.73
2018 Sep 19 14:00:00.0          18 37 34.136          + 38 48 28.69
2018 Sep 19 15:00:00.0          18 37 34.135          + 38 48 28.65
2018 Sep 19 16:00:00.0          18 37 34.135          + 38 48 28.61
2018 Sep 19 17:00:00.0          18 37 34.137          + 38 48 28.57
2018 Sep 19 18:00:00.0          18 37 34.141          + 38 48 28.54
2018 Sep 19 19:00:00.0          18 37 34.145          + 38 48 28.52
2018 Sep 19 20:00:00.0          18 37 34.149          + 38 48 28.52
2018 Sep 19 21:00:00.0          18 37 34.154          + 38 48 28.52
2018 Sep 19 22:00:00.0          18 37 34.159          + 38 48 28.53
2018 Sep 19 23:00:00.0          18 37 34.163          + 38 48 28.55
</pre>
 */
void test_prediction( CuTest * test );

/** Time how long it takes to point the tracker at every star in the catalog then prints the local coordinates. */
void test_benchmark( Catalog* catalog, Tracker* tracker, int trials );

#endif //STARTRACK_TEST_H

// NOVAS documentation example of calculating time scales
//        const short int year = 2008;
//        const short int month = 4;
//        const short int day = 24;
//        const short int leap_secs = 33;
//        const double hour = 10.605;
//        const double ut1_utc = -0.387845;
//        const double x_pole = -0.002;
//        const double y_pole = +0.529;
//        jd_utc = julian_date (year,month,day,hour); //the output argument, jd_utc, will have a value of 2454580.9441875
//        jd_tt = jd_utc + ((double) leap_secs + 32.184) / 86400.0;
//        jd_ut1 = jd_utc + ut1_utc / 86400.0;
//        delta_t = 32.184 + leap_secs - ut1_utc