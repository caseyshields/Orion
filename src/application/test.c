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

    // add unit tests
    SUITE_ADD_TEST(suite, test_angles);
    SUITE_ADD_TEST(suite, test_FK6);
    SUITE_ADD_TEST(suite, test_time);
    SUITE_ADD_TEST(suite, test_tats);
    SUITE_ADD_TEST(suite, test_crc);
    SUITE_ADD_TEST(suite, test_novas);
    SUITE_ADD_TEST(suite, test_iers_load);
    SUITE_ADD_TEST(suite, test_iers_search);

    // add higher level tests which integrate modules
    SUITE_ADD_TEST(suite, test_prediction);

    return suite;
}   // note you can add suites to suites if you want to add a bit more organization to the tests

Entry test_getVega( CuTest * test ) {
    Entry vega = {
            .efg = {0, 0, 0},
            .zenith_distance = 0,
            .topocentric_azimuth = 0,
            .magnitude = 0.03,
            .novas = {
                    .starname = "alpha Lyr",
                    .catalog = "test",
                    .starnumber = 699,
                    .ra = dms2deg(18, 36, 56.336939), // this actually remains in hours despite using dms!
                    .dec = dms2deg(38, 47, 1.28333),
                    .parallax = 128.93,
                    .promora = 00201.70,
                    .promodec = 00286.67,
                    .radialvelocity = -013.5
            }
    };
    // Wow, Wikipedia doesn't match... apperently they get it from some coaxed version of FK5?
    // Right Ascension  18h 36m 56.33635s[1]
    // Declination	+38° 47′ 01.2802″
//    CuAssertDblEquals_Msg(test, "Right Ascension Hour conversion failed", 18.615649, vega.novas.ra, 0.000001);
//    CuAssertDblEquals_Msg(test, "Declination degree conversion failed", 38.783690, vega.novas.dec, 0.000001);
    return vega;
}

IERS_EOP test_getEarth2018Sep9( CuTest * test ) {
    jday jd = date2jday(2018, 9, 19, 0, 0, 0);
    IERS_EOP eop = {
            .mjd=jd, .pm_flag='P', .dt_flag='P',
            .pm_x=0.207193, .pm_x_err=0.003944,
            .pm_y=0.344530, .pm_y_err=0.004292,
            .ut1_utc=0.0513092, .ut1_utc_err=0.0029860
    };
    CuAssertDblEquals_Msg(test, "incorrect MJD conversion", IERS_MJD_OFFSET+58380.00, eop.mjd, 0.01);
    return eop;
}

Tracker test_getMcCarrenTracker( CuTest * test, IERS_EOP * eop ) {

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
            .earth = eop
    };
    CuAssertDblEquals_Msg(test, "incorrect latitude conversion", 36.072032, tracker.site.latitude, 0.000001);
    CuAssertDblEquals_Msg(test, "incorrect longitude conversion", -115.134350, tracker.site.longitude, 0.000001);
    return tracker;
}

void test_prediction( CuTest * test ) {

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

    Entry vega = test_getVega(test);

    IERS_EOP earth = test_getEarth2018Sep9(test);

    Tracker tracker = test_getMcCarrenTracker( test, &earth );

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
        jday jd_tt = (jd_ut1 - earth.ut1_utc/86400.0)
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

/** Time how long it takes to point the tracker at every star in the catalog then prints the local coordinates. */
void test_benchmark( Catalog* catalog, Tracker* tracker, int trials ) {
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

