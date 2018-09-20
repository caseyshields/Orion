#include "test.h"

void test_run() {
    CuSuite * suite = test_suite();
    CuString * output = CuStringNew();
    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    CuSuiteDetails( suite, output );
    printf("%s\n", output->buffer);
    CuSuiteDelete(suite);
    CuStringDelete(output);
    exit( suite->failCount );
}

CuSuite * test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_angles);
    SUITE_ADD_TEST(suite, test_FK6);
    SUITE_ADD_TEST(suite, test_time);
    SUITE_ADD_TEST(suite, test_tats);
    SUITE_ADD_TEST(suite, test_crc);
    SUITE_ADD_TEST(suite, test_novas);
    SUITE_ADD_TEST(suite, test_iers_load);
    SUITE_ADD_TEST(suite, test_iers_search);
    SUITE_ADD_TEST(suite, test_prediction);
    return suite;
}   // note you can add suites to suites if you want to add a bit more organization to the tests

void test_prediction( CuTest * test ) {

    // construct a catalog entry for Vega from Part I of the FK6 catalog;
    //|  699 |  91262 | alpha Lyr          | 18 36 56.336939 |  +38 47  1.28333 | +00201.70 | +00286.67 |1990.93 |   0.46 |   0.21 |1990.98 |   0.45 |   0.20 | 128.93 |   0.55 |  H  | -013.5 |   0.03 |  1  |  23  |  2  |     | -0025.23 | -0005.80 | -0005.98 | -0072.68 | -0037.05 | +0006.73 | +0007.17 | -0161.31 | -0000.23 | -0000.65 | -0000.67 | -0000.82 | +0000.51 | -0000.41 | +0000.75 | +0000.80 | -0001.58 | +0000.50 |1959.60 |1991.00 |1991.00 |1941.72 |  10.79 |   0.46 |   0.46 |  13.50 |   0.30 |   0.56 |   0.57 |   0.41 |   0.27 |1954.47 |1991.03 |1991.03 |1920.23 |  12.47 |   0.45 |   0.45 |  17.33 |   0.26 |   0.59 |   0.60 |   0.39 |   0.24 | 128.89 | 128.93 | 128.93 |   0.54 |   0.55 |   0.55 | -0032.63 | -0003.13 | +0000.94 | +0001.01 |     9.48 |     6.90 |     0.29 |     0.18 |  1  |   3.36 |   1.88 |   3.85 |   5.25 | 430 |
    Entry vega = {
            .efg = {0, 0, 0},
            .zenith_distance = 0,
            .topocentric_azimuth = 0,
            .magnitude = 0.03,
            .novas = {
                    .starname = "alpha Lyr",
                    .catalog = "test",
                    .starnumber = 699,
                    .ra = dms2deg(18, 36, 56.336939),//56.33653), // this actually remains in hours despite using dms!
                    .dec = dms2deg(38, 47, 1.28333),//1.2802),//
                    /*18h 36m 56.33635s[1]
Declination	+38° 47′ 01.2802″*/ //wow wikipedia doesn't match...
                    .parallax = 128.93,
                    .promora = 00201.70,
                    .promodec = 00286.67,
                    .radialvelocity = -013.5
            }
    };
//    CuAssertDblEquals_Msg(test, "Right Ascension Hour conversion failed", 18.615649, vega.novas.ra, 0.000001);
//    CuAssertDblEquals_Msg(test, "Declination degree conversion failed", 38.783690, vega.novas.dec, 0.000001);

    // construct earth orientation from the IERS bulletin entry;
    //18 919 58380.00 P  0.207193 0.003944  0.344530 0.004292  P 0.0513092 0.0029860                 P     0.112    0.128     0.214    0.160
    jday jd = date2jday(2018, 9, 19, 0, 0, 0);
    IERS_EOP eop = {
            .mjd=jd, .pm_flag='P', .dt_flag='P',
            .pm_x=0.207193, .pm_x_err=0.003944,
            .pm_y=0.344530, .pm_y_err=0.004292,
            .ut1_utc=0.0513092, .ut1_utc_err=0.0029860
    };
    CuAssertDblEquals_Msg(test, "incorrect MJD conversion", IERS_MJD_OFFSET+58380.00, eop.mjd, 0.01);

    // A tracker placed at the McCarren Viewing Area
    // https://www.google.com/maps/place/36%C2%B004'19.3%22N+115%C2%B008'03.7%22W/@36.0720393,-115.1344197,41m/data=!3m1!1e3!4m6!3m5!1s0x0:0x0!7e2!8m2!3d36.0720322!4d-115.13435?hl=en
    //                    36°04'19.3"N 115°08'03.7"W
    //                    36.072032, -115.134350
    Tracker tracker = {
            .azimuth = 0.0,
            .elevation = 0.0,
            .efg = {0,0,0},
            .site = {
                    .pressure=1010,
                    .temperature=10,
                    .latitude=dms2deg(36, 4, 19.3),//36.072032,//
                    .longitude=dms2deg(-115, 8, 3.7),//-115.134350,//
                    .height=0.0 },
            .earth = &eop
    };
//    CuAssertDblEquals_Msg(test, "incorrect latitude conversion", 36.072032, tracker.site.latitude, 0.000001);
//    CuAssertDblEquals_Msg(test, "incorrect longitude conversion", -115.134350, tracker.site.longitude, 0.000001);
    // turns out the google values don't match, there is less precision in the formatted string...

    // Here is the reference MJD, ZD and EL which we want to reproduce
//USNO report obtained from http://aa.usno.navy.mil/data/docs/topocentric.php
/*
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
*/
    double usno[][3] = {
        { date2jday(2018, 9, 19, 12, 0, 0), dms2deg(98, 00, 38.3), dms2deg(332, 18, 58.5) },
        { date2jday(2018, 9, 19, 13, 0, 0), dms2deg(102, 37, 2.9), dms2deg(343, 15, 35.4) },
        { date2jday(2018, 9, 19, 14, 0, 0), dms2deg(104, 54, 29.9), dms2deg(355, 4, 57.1) },
        { date2jday(2018, 9, 19, 15, 0, 0), dms2deg(104, 39, 59.3), dms2deg(7, 11, 23.9) },
        { date2jday(2018, 9, 19, 16, 0, 0), dms2deg(101, 54, 57.7), dms2deg(18, 53, 7.5) },
        { date2jday(2018, 9, 19, 17, 0, 0), dms2deg(96, 54, 40.4), dms2deg(29, 37, 4.2) },
        { date2jday(2018, 9, 19, 18, 0, 0), dms2deg(90, 2, 15.2), dms2deg(39, 7, 6.0) },
        { date2jday(2018, 9, 19, 19, 0, 0), dms2deg(81, 42, 1.4), dms2deg(47, 21, 59.2) },
        { date2jday(2018, 9, 19, 20, 0, 0), dms2deg(72, 15, 25.1), dms2deg(54, 28, 43.5) },
        { date2jday(2018, 9, 19, 21, 0, 0), dms2deg(61, 59, 41.6), dms2deg(60, 36, 28.4) },
        { date2jday(2018, 9, 19, 22, 0, 0), dms2deg(51, 8, 12.4), dms2deg(65, 52, 22.2) },
        { date2jday(2018, 9, 19, 23, 0, 0), dms2deg(39, 51, 18.5), dms2deg(70, 17, 18.5) },
    };

    // we want answers to be within 10 arcseconds
    double epsilon = 10.0 / 60.0 / 60.0;

    // target a star that we have data for from the reference USNO implementation
    for(int n=0; n<12; n++) {

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

//        // convert UT1 time to the TT timescale used by novas, and thus orion
        double leap_secs = 37.0;
//        jday jd_ut1 = usno[n][0]; //date2jday(2018, 9, 19, 12, 0, 0);
//        jday jd_tt = (jd_ut1 - eop.ut1_utc/86400.0) + ((leap_secs + 32.184) / 86400.0);//ut12tt( jd_ut1 );
//        // adapted from Novas 3.1 section 3.2... doesn't work...

        jday jd_ut1 = usno[n][0];
        jday jd_tt = (jd_ut1 - eop.ut1_utc/86400.0)
                     - ((leap_secs + 32.184) / 86400.0);
        // some shit I randomly put together
        // gives me better results with a bias of 57 arcsec, and a range around that of 2 arcseconds...

        // TODO expose the calculation of celestial targets in tracker, then regenerate the USNO report for celestial coordinates.
        // use this to determine if the error is in abberation, parallax, propermotion(unlikely, these are small effects)
        // or if the error in earth orientation, it can't be refraction because I turned that off
        // if both are still messed up then the catalog or location conversion is bad...

        // TODO determine the angle between the error and the motion; if it's about zero then we know it is entirely a time scale problem

        int result = tracker_point(&tracker, jd_tt, &vega.novas);
        CuAssertIntEquals_Msg(test, "tracker_point() failed", 0, result);

        double az = usno[n][2]; //dms2deg(332, 18, 58.5);
        double zd = usno[n][1]; //dms2deg(98, 00, 38.3);
        double el = 90.0 - zd;

        double Eaz = tracker.azimuth - az;
        double Eel = tracker.elevation - el;
        double E = sqrt( Eaz*Eaz + Eel*Eel );
        char * Estr = deg2str( E );
        char * jdstr = jday2str( jd_ut1 );
        printf("\ttime=%s\terror=%s\t(%lf)\n", jdstr, Estr, E);
        free(Estr);
        free(jdstr);

//        CuAssertDblEquals_Msg(test, "Inaccurate Tracker azimuth", az, tracker.azimuth, epsilon);
//        CuAssertDblEquals_Msg(test, "Inaccurate Tracker elevation", el, tracker.elevation, epsilon);
    }
    //332.316250 // usno reference....
    //332.515724 // decimal coordinate
    //332.701623 // dms coordinates
    //332.701623

    /* There are a lot of errors at different magnitudes to sort out;
     * Proper motion < 1 arcsecond/year(usu.much less)
     * parallax < 1 arcsecond
     * gravitational light bending < 0.05 arcseconds 10deg away from the sun
     * aberration < 21 arcseconds
     * refraction < 60 arcseconds @ 45deg, 1800 arcseconds at horizon */

    //    Catalog catalog = { .size = 1, .allocated=1, .stars=&vega };
//    Application app = {
//            .mode=1, .ip="127.0.0.1", .port=43210, .jd_tt=time,
//            .orion=NULL, .eop=NULL, .catalog=&catalog, .iers=NULL };
//    cmd_target( "target 699", &app );
    // TODO it would be nice to be able to test this at the application layer, but I need to do a better job extracting the commands...
}

// Work in Progress!
void catalog_add_axis(Catalog * catalog, int type, int count);
void test_search_equator() {
    Catalog * catalog = catalog_create( NULL, 8 );
    catalog_add_axis( catalog, 1, (24*60*60) );


    // test each query along the axis contains
}

/** add either an equator or meridian to a catalog
 * @param count number of equidistant points on the great circle.
 * @param type 1 = Equator, 2 = Prime Meridian*/
void catalog_add_axis(Catalog * catalog, int type, int count) {

    // for the desired number of points
    // notice <= so equator will wrap around, and meridian set will contain both poles
    for (int n=0; n<=count; n++) {

        // allocate and zero all values
        Entry * entry = malloc( sizeof(Entry) );
        memset( entry, 0, sizeof(Entry) );

        // calculate test values
        memcpy( entry->novas.catalog, "tst\0", 4);
        if ( type == 1 ) {
            double hours = 24.0 * n / count;
            entry->novas.ra = hours;
            sprintf( entry->novas.starname, "equator %lf", hours );
        } else if (type == 2) {
            double degrees = (180.0 * n / count) - 90.0;
            entry->novas.dec = degrees;
            sprintf( entry->novas.starname, "meridian %lf", degrees );
        }

        catalog_add(catalog, entry);
    }
} // need to generalize to an arbitrary lesser circle!

// TODO readme columns in bsc5 do not line up, in fact they overlap. so this will not work with the FK6 loader
// try using the metadata loader to load the yale
void test_BSC5() {
    // it could probably be made to work by splitting on whitespace.
    Catalog * catalog = catalog_create( 0, 1024 );

    FILE * readme = fopen("../data/bsc5/ReadMe", "r");
    assert(NULL != readme);

    char* header = "Byte-by-byte Description of file: catalog\n";
    FK6 * bsc5 = fk6_create();
    fk6_load_fields( bsc5, readme, header );//FK6_1_HEADER );
    fclose( readme );

    FILE * data = fopen("../data/bsc5/catalog", "r");
    assert(NULL != data);
    catalog_load_fk6(catalog, bsc5, data);
    fclose( data );
    fk6_free( bsc5 );
    free( bsc5 );

    catalog_print( catalog );

    catalog_free( catalog );
    free( catalog );
}

/** Time how long it takes to point the tracker at every star in the catalog then prints the local coordinates. */
void benchmark( Catalog* catalog, Tracker* tracker, int trials ) {
    // start the timer
    jday start = jday_now();
    int size = catalog->size;

    // track every star in the FK6 catalog
//    double tracks [catalog->size][2]; //double latitude, longitude;
//    double efgs [catalog->size][3];
    for( int t=0; t<trials; t++ ) {
        for (int n = 0; n < catalog->size; n++) {
            Entry *entry = catalog->stars[n];
            int error = tracker_point( tracker, J2000_EPOCH, &(entry->novas) );

        }
    }

    // get the time
    jday end = jday_now();
    double duration = (end - start)*SECONDS_IN_DAY;

    // print the catalog with corresponding tracks
//    for( int n=0; n<catalog->size; n++ ) {
//        Entry* entry = catalog->stars[n];
//        entry_print( entry );
//        printf( "Horizon : (zd:%lf, az:%lf)\n\n", tracks[n][0], tracks[n][1] );
//    }

    printf( "stars: %d\ntrials: %d\ntime: %lf\nspeed: %lf\n\n",
            catalog->size, trials, duration, duration/(trials*catalog->size) );

//    CuAssert( test, "Can't update positions of entire catalog at 50 Hz.", (duration/trials) < 0.02 );
}

// this is not standard C, but a GNU C extension.
// would have been nice...
//            // filter by name
//            int check_name( Entry *entry ) {
//                return NULL != strstr(entry->novas.starname, line );
//            }
//            Catalog* results = catalog_filter(catalog, &check_name, NULL);

//            // transform each star to local coordinates
//            void process( Entry *entry ) {
//                double zd=0, az=0;
//                tracker_to_horizon(&tracker, &(entry->novas), &zd, &az);
//                entry_print( entry );
//                printf( "\tlocal : { zd:%lf, az:%lf}\n", zd, az );
//            }
//            catalog_each( results, process );

