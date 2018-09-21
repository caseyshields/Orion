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

void test_prediction( CuTest * test ) {

    // we want answers to be within 10 arcseconds
    double epsilon = 10.0 / 60.0 / 60.0;

    // here's a hand jammed copy of the reports. There's probably no typos.
    double usno[][5] = { // UT1,  zd,  az,  ra,  dec
        { date2jday(2018, 9, 19, 0, 0, 0), dms2deg(29, 03, 16.6), dms2deg(73, 25, 14.7), dms2deg(18, 37, 34.189), dms2deg(38, 48, 28.55) },
        { date2jday(2018, 9, 19, 1, 0, 0), dms2deg(17, 21, 12.4), dms2deg(74, 25, 47.5), dms2deg(18, 37, 34.191), dms2deg(38, 48, 28.59) },
        { date2jday(2018, 9, 19, 2, 0, 0), dms2deg(5, 54, 21.6), dms2deg(60, 26, 08.8), dms2deg(18, 37, 34.192), dms2deg(38, 48, 28.63) },
        { date2jday(2018, 9, 19, 3, 0, 0), dms2deg(7, 14, 20.0), dms2deg(294, 43, 39.0), dms2deg(18, 37, 34.190), dms2deg(38, 48, 28.68) },
        { date2jday(2018, 9, 19, 4, 0, 0), dms2deg(18, 47, 42.0), dms2deg(285, 26, 05.4), dms2deg(18, 37, 34.188), dms2deg(38, 48, 28.72) },
        { date2jday(2018, 9, 19, 5, 0, 0), dms2deg(30, 29, 13.5), dms2deg(286, 54, 56.1), dms2deg(18, 37, 34.184), dms2deg(38, 48, 28.76) },
        { date2jday(2018, 9, 19, 6, 0, 0), dms2deg(42, 00, 29.9), dms2deg(290, 28, 25.1), dms2deg(18, 37, 34.179), dms2deg(38, 48, 28.79) },
        { date2jday(2018, 9, 19, 7, 0, 0), dms2deg(53, 13, 17.5), dms2deg(295, 03, 24.9), dms2deg(18, 37, 34.173), dms2deg(38, 48, 28.81) },
        { date2jday(2018, 9, 19, 8, 0, 0), dms2deg(63, 58, 54.3), dms2deg(300, 28, 45.5), dms2deg(18, 37, 34.166), dms2deg(38, 48, 28.82) },
        { date2jday(2018, 9, 19, 9, 0, 0), dms2deg(74, 06, 29.8), dms2deg(306, 46, 58.5), dms2deg(18, 37, 34.159), dms2deg(38, 48, 28.82) },
        { date2jday(2018, 9, 19, 10, 0, 0), dms2deg(83, 22, 02.1), dms2deg(314, 05, 51.9), dms2deg(18, 37, 34.153), dms2deg(38, 48, 28.81) },
        { date2jday(2018, 9, 19, 11, 0, 0), dms2deg(91, 27, 27.9), dms2deg(322, 34, 32.6), dms2deg(18, 37, 34.147), dms2deg(38, 48, 28.79) },
        { date2jday(2018, 9, 19, 12, 0, 0), dms2deg(98, 00, 38.3), dms2deg(332, 18, 58.5), dms2deg(18, 37, 34.142), dms2deg(38, 48, 28.76) },
        { date2jday(2018, 9, 19, 13, 0, 0), dms2deg(102, 37, 2.9), dms2deg(343, 15, 35.4), dms2deg(18, 37, 34.138), dms2deg(38, 48, 28.73) },
        { date2jday(2018, 9, 19, 14, 0, 0), dms2deg(104, 54, 29.9), dms2deg(355, 4, 57.1), dms2deg(18, 37, 34.136), dms2deg(38, 48, 28.69) },
        { date2jday(2018, 9, 19, 15, 0, 0), dms2deg(104, 39, 59.3), dms2deg(7, 11, 23.9), dms2deg(18, 37, 34.135), dms2deg(38, 48, 28.65) },
        { date2jday(2018, 9, 19, 16, 0, 0), dms2deg(101, 54, 57.7), dms2deg(18, 53, 7.5), dms2deg(18, 37, 34.135), dms2deg(38, 48, 28.61) },
        { date2jday(2018, 9, 19, 17, 0, 0), dms2deg(96, 54, 40.4), dms2deg(29, 37, 4.2), dms2deg(18, 37, 34.137), dms2deg(38, 48, 28.57) },
        { date2jday(2018, 9, 19, 18, 0, 0), dms2deg(90, 2, 15.2), dms2deg(39, 7, 6.0), dms2deg(18, 37, 34.141), dms2deg(38, 48, 28.54) },
        { date2jday(2018, 9, 19, 19, 0, 0), dms2deg(81, 42, 1.4), dms2deg(47, 21, 59.2), dms2deg(18, 37, 34.145), dms2deg(38, 48, 28.52) },
        { date2jday(2018, 9, 19, 20, 0, 0), dms2deg(72, 15, 25.1), dms2deg(54, 28, 43.5), dms2deg(18, 37, 34.149), dms2deg(38, 48, 28.52) },
        { date2jday(2018, 9, 19, 21, 0, 0), dms2deg(61, 59, 41.6), dms2deg(60, 36, 28.4), dms2deg(18, 37, 34.154), dms2deg(38, 48, 28.52) },
        { date2jday(2018, 9, 19, 22, 0, 0), dms2deg(51, 8, 12.4), dms2deg(65, 52, 22.2), dms2deg(18, 37, 34.159), dms2deg(38, 48, 28.53) },
        { date2jday(2018, 9, 19, 23, 0, 0), dms2deg(39, 51, 18.5), dms2deg(70, 17, 18.5), dms2deg(18, 37, 34.163), dms2deg(38, 48, 28.55) },
    };

    Entry vega = test_getVega();
    CuAssertDblEquals_Msg(test, "Right Ascension Hour conversion failed", 18.615649, vega.novas.ra, 0.000001);
    CuAssertDblEquals_Msg(test, "Declination degree conversion failed", 38.783690, vega.novas.dec, 0.000001);

    IERS_EOP earth = test_getEarth2018Sep9();
    CuAssertDblEquals_Msg(test, "incorrect MJD conversion", IERS_MJD_OFFSET+58380.00, earth.mjd, 0.01);

    Tracker tracker = test_getMcCarrenTracker( &earth );
    CuAssertDblEquals_Msg(test, "incorrect latitude conversion", 36.072028, tracker.site.latitude, 0.000001);
    CuAssertDblEquals_Msg(test, "incorrect longitude conversion", -115.134361, tracker.site.longitude, 0.000001);

    // target a star that we have data for from the reference USNO implementation
    double Uaz, Uel, Ura, Udc; // USNO coordinates
    double Taz, Tel, Tra, Tdc; // Tracker Coordinates
    double Eaz, Eel, Era, Edc; // coordinate error
    double Eh, Ec; // error magnitude
//    double dUaz, dUel, dTaz, dTel; // horizon derivative

    printf( "TT\tE_horizon\tE_celestial\tUSNO elevation\n" );
//    printf("\tTT\tUSNO_AZ\tUSNO_EL\tORION_AZ\tORION_EL\t|E|\tEaz\tEel\tEdms\n");

    for(int n=0; n<24; n++) {

        // convert UT1 time to the TT timescale used by novas, and thus orion
        double leap_secs = 37.0;
        jday jd_ut1 = usno[n][0];
        jday jd_utc = jd_ut1 - (earth.ut1_utc/86400.0);
        jday jd_tt = jd_utc + ((leap_secs + 32.184) / 86400.0);
        // adapted from Novas 3.1 section 3.2

        // point the tracker at the star, ignoring refraction.
        int result = tracker_point(&tracker, jd_ut1, &vega.novas, REFRACTION_NONE);
        CuAssertIntEquals_Msg(test, "tracker_point() failed", 0, result);
        // NOTE: Something is up with the USNO web application;
        // I can only match it when I pass the UT1 time to the topo_star() routine.
        // According to Novas documentation, topo_star() requires terrestrial time!

        // compute error
        Uaz = usno[n][2];
        Uel = 90.0 - usno[n][1];
        Ura = usno[n][3];
        Udc = usno[n][4];

        Taz = tracker.azimuth;
        Tel = tracker.elevation;
        Tra = tracker.right_ascension;
        Tdc = tracker.declination;

        Eaz = Taz - Uaz;
        Eel = Tel - Uel;
        Era = Tra - Ura;
        Edc = Tdc - Udc;

        Eh = sqrt( Eaz*Eaz + Eel*Eel );
        Ec = sqrt( Era*Era + Edc*Edc );

//        if(n>0) {
//            dUaz = usno[n][2] - usno[n-1][2];
//            dUel = usno[n][1] - usno[n-1][1];
//            dTaz = Taz - tracker.azimuth;
//            dTel = Tel = tracker.elevation;
//        }

        // print the results for the user
        char * jdstr = jday2str( jd_ut1 );
        char * EhStr = deg2str( Eh );
        char * EcStr = deg2str( Ec );
        printf("\t%17s\t%17s\t%17s\t%lf\n", jdstr, EhStr, EcStr, Uel);
        // print it all
//        printf("\t%s\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%s\n",
//               jdstr, Uaz, Uel, Taz, Tel, E, Eaz, Eel, Estr);
        // print just the basic values
//        printf("\t%s\t%lf\t%lf\t%lf\t%lf\n",
//               jdstr, Uaz, Uel, Taz, Tel);
        free(EhStr);
        free(EcStr);
        free(jdstr);

        // compare the sizeof the error in the celestial placement and the terrestrial placement
        CuAssertDblEquals_Msg(test, "Inaccurate Tracker Right Ascension", Ura, tracker.right_ascension, epsilon);
        CuAssertDblEquals_Msg(test, "Inaccurate Tracker Declination", Udc, tracker.declination, epsilon);
        CuAssertDblEquals_Msg(test, "Inaccurate Tracker azimuth", Uaz, tracker.azimuth, epsilon);
        CuAssertDblEquals_Msg(test, "Inaccurate Tracker elevation", Uel, tracker.elevation, epsilon);
    }
}

Entry test_getVega() {
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
    // Wow, Wikipedia doesn't match;
    // Right Ascension  18h 36m 56.33635s[1]
    // Declination	+38° 47′ 01.2802″
    // ... apparently they get it from some coaxed version of FK5?
    return vega;
}

IERS_EOP test_getEarth2018Sep9() {
    jday jd = date2jday(2018, 9, 19, 0, 0, 0);
    IERS_EOP eop = {
            .mjd=jd, .pm_flag='I', .dt_flag='I',
            .pm_x=0.212071, .pm_x_err=0.000091,
            .pm_y=0.348723, .pm_y_err=0.000091,
            .ut1_utc=0.0558907, .ut1_utc_err=0.0000075
    };
    return eop;
}

// prediction from older version of the IERS
IERS_EOP test_getEarth2018Sep9P() {
    jday jd = date2jday(2018, 9, 19, 0, 0, 0);
    IERS_EOP eop = {
            .mjd=jd, .pm_flag='P', .dt_flag='P',
            .pm_x=0.207193, .pm_x_err=0.003944,
            .pm_y=0.344530, .pm_y_err=0.004292,
            .ut1_utc=0.0513092, .ut1_utc_err=0.0029860
    };
    return eop;
}

Tracker test_getMcCarrenTracker( IERS_EOP * eop ) {
    Tracker tracker = {
            .azimuth = 0.0,
            .elevation = 0.0,
            .efg = {0,0,0},
            .site = {
                    .pressure=1010,
                    .temperature=10,
                    .latitude = dms2deg(36, 4, 19.3),
                    .longitude = -dms2deg(115, 8, 3.7),
                    .height=0.0 },
            .earth = eop
    };
    return tracker;
}

// TODO it would be nice to be able to test this at the application layer, but I need to do a better job extracting the commands...
//void test_application() {
//    Catalog catalog = { .size = 1, .allocated=1, .stars=&vega };
//    Application app = {
//            .mode=1, .ip="127.0.0.1", .port=43210, .jd_tt=time,
//            .orion=NULL, .eop=NULL, .catalog=&catalog, .iers=NULL };
//    cmd_target( "target 699", &app );
//    ...
//}

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
            int error = tracker_point( tracker, J2000_EPOCH, &(entry->novas), REFRACTION_SITE );

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

