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

void test_angles( CuTest * test ) {
    double count = 5000;
    int d = 0, m = 0;
    double s = 0.0;
    double mas = 1.0/(60.0*60.0*1000);

    for (int n=0; n<count; n++) {
        double angle = ((double)n)*360.0/count;

        // test degrees, minutes, seconds conversion
        deg2dms( angle, &d, &m, &s );
        double deg = dms2deg( d, m, s );
//        if( fabs(angle-deg)>mas )
//            printf(DMS_OUTPUT_FORMAT, d, m, s);
        CuAssertDblEquals_Msg(test, "incorrect decimal degree conversion", angle, deg, mas);

        // test string conversion
        char * str1 = dms2str(d, m, s);
        deg = str2deg(str1);
        CuAssertDblEquals_Msg(test, "incorrect string conversion of DMS", angle, deg, 10*mas);

        char * str2 = deg2str( angle );
        CuAssertStrEquals_Msg(test, "String conversions did not match", str1, str2);

        free(str1);
        free(str2);
    }
}

void test_dms( CuTest * test ) {
    int d = 73, m = 17;
    double s = 16.1;
    double truth = d + (m/60.0) + (s/3600.0);

    double deg = dms2deg( 73, 17, 16.1 );
    CuAssertDblEquals_Msg(test, "incorrect decimal degrees conversion", truth, deg, 1.0/360000.0);

    int i,j;
    double k;
    deg2dms( deg, &i, &j, &k );
    CuAssertIntEquals_Msg(test, "incorrect conversion, degrees", d, i);
    CuAssertIntEquals_Msg(test, "incorrect conversion, minutes", m, j);
    CuAssertDblEquals_Msg(test, "incorrect conversion, seconds", s, k, 0.01);

    char * str = dms2str(d, m, s);
    CuAssertStrEquals_Msg(test, "Incorrect printing of DMS", "73d 17m 16.10s", str);

    deg = str2deg(str);
    CuAssertDblEquals_Msg(test, "incorrect string parse of DMS", truth, deg, 1.0/360000.0);

    free(str);
}

void test_time( CuTest * test ) {
    // check that lenient input formatting interprets the timestamp consistently
    char * inputs[] = {"2000/1/2 3:4:5.006",
                       "2000/1/2 03:04:05.006123"};

    // output formatting is consistent
    char * output = "2000/01/02 03:04:05.006";

    // check that the retrieved timestamp is correct for each input
    for( int n = 0; n<2; n++ ) {
        jday time = str2jday(inputs[n]);
        CuAssertTrue( test, jday_is_valid(time) );
        char *copy = jday2str(time);
        CuAssertStrEquals( test, output, copy );
        free(copy);
    }
}

void test_tats( CuTest * test ) {
    // make sure 1 byte alignment is working...
    CuAssertIntEquals( test, 22, sizeof(MIDC01) );
    CuAssertIntEquals( test, 22, sizeof(TCN_Message) );
}

void test_crc( CuTest * test ) {
    // modified from http://www.drdobbs.com/implementing-the-ccitt-cyclical-redundan/199904926
    // Written by Bob Felice

    static unsigned char string[40];
    string[0] = 'T';
    string[1] = (unsigned char)0xd9;
    string[2] = (unsigned char)0xe4;
    string[3] = '\0';

//    printf ("The crc of \"T\" is 0xD9E4. crc16 returned 0x%X.\r\n\n",
//            crc16(string, (short)1));
    CuAssertIntEquals( test, 0xD9E4, crc16(string, (short)1) );

//    printf ("The crc of \"T 0xD9 0xE4\" is %x. The value of crc_ok is 0x%X.\r\n\n",
//            crc16(string, (short)3), crc_ok);
    CuAssertIntEquals( test, crc_ok, crc16(string, (short)3) );

    strcpy(string, "THE,QUICK,BROWN,FOX,0123456789");
//    printf("The crc of \"%s\" is 0x6E20. crc16 returned 0x%X.\r\n\n",
//           string, crc16 (string, strlen(string)));
    CuAssertIntEquals( test, 0x6E20, crc16(string, strlen(string)) );

    string[0] = (unsigned char)0x03;
    string[1] = (unsigned char)0x3F;
//    puts("CCITT Recommendation X.25 (1984) Appendix I example:");
//    printf("\tThe crc of 0x03 0x3F is 0x5BEC. crc16 returned 0x%X.\r\n\n",
//           crc16(string, (short)2));
    CuAssertIntEquals( test, 0x5BEC, crc16(string, (short)2) );

//    puts("strike RETURN to continue...");
//        getchar();
}

void test_FK6( CuTest * test ) {
    Catalog * catalog = catalog_create( 0, 1024 );

    // load metadata for the first part of FK6
    FILE * readme = fopen("../data/fk6/ReadMe", "r");
    CuAssertPtrNotNullMsg( test, "Could not find FK6 readme", readme );
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields( fk6_1, readme, FK6_1_HEADER );
//    for(int n=0; n<fk6_1->cols; n++)
//        fk6_print_field( &(fk6_1->fields[n]), stdout );
    CuAssertIntEquals( test, 93, fk6_1->cols );

    // load first part of FK6
    FILE * data1 = fopen("../data/fk6/fk6_1.dat", "r");
    CuAssertPtrNotNullMsg( test, "Could not find FK6 Part I", data1);
    catalog_load_fk6( catalog, fk6_1, data1 );
    fclose( data1 );
    CuAssertIntEquals( test, 878, catalog->size );

    // How can I test the validity of this data a bit more thoroughly...

    // release all objects
    fk6_free( fk6_1 );
    free( fk6_1 );
    catalog_free( catalog );
    free( catalog );
}

#define N_STARS 3
#define N_TIMES 4
/** Adapted from Novas' checkout-stars.c */
void test_novas( CuTest * test ) {
    double answers[N_TIMES][N_STARS][2] = {
            {{2.446989227,89.24635169},{5.530110735,-0.30571717},{10.714525532,-64.38130568}},
            {{2.446989227,89.24635169},{5.530110735,-0.30571717},{10.714525532,-64.38130568}},
            {{2.509479607,89.25196807},{5.531195895,-0.30301953},{10.714444762,-64.37366521}},
            {{2.481178365,89.24254418},{5.530372302,-0.30231627},{10.713575398,-64.37966984}}
    };

    /*
   Main function to check out many parts of NOVAS-C by calling
   function 'topo_star' with version 3 of function 'solarsystem'.

   For use with NOVAS-C Version 3.1.
*/

    short int error = 0;
    short int accuracy = 1;
    short int i, j;

/*
   'deltat' is the difference in time scales, TT - UT1.

    The array 'tjd' contains four selected Julian dates at which the
    star positions will be evaluated.
*/

    double deltat = 60.0;
    double tjd[N_TIMES] = {2450203.5, 2450203.5, 2450417.5, 2450300.5};
    double ra, dec;

/*
   Hipparcos (ICRS) catalog data for three selected stars.
*/

    cat_entry stars[N_STARS] = {
            {"POLARIS", "HIP",   0,  2.530301028,  89.264109444,
                    44.22, -11.75,  7.56, -17.4},
            {"Delta ORI", "HIP", 1,  5.533444639,  -0.299091944,
                    1.67,   0.56,  3.56,  16.0},
            {"Theta CAR", "HIP", 2, 10.715944806, -64.394450000,
                    -18.87, 12.06,  7.43,  24.0}};

/*
   The observer's terrestrial coordinates (latitude, longitude, height).
*/

    on_surface geo_loc = {45.0, -75.0, 0.0, 10.0, 1010.0};

/*
   Compute the topocentric places of the three stars at the four
   selected Julian dates.
*/

    for (i = 0; i < N_TIMES; i++)
    {
        for (j = 0; j < N_STARS; j++)
        {
            error = topo_star (tjd[i],deltat,&stars[j],&geo_loc, accuracy, &ra,&dec);

//                printf ("Error %d from topo_star. Star %d  Time %d\n",
//                        error, j, i);
            CuAssertIntEquals_Msg( test, "topo_star failed", 0, error );

//            printf ("JD = %f  Star = %s\n", tjd[i], stars[j].starname);
//            printf ("RA = %12.9f  Dec = %12.8f\n", ra, dec);
//            printf ("\n");
            CuAssertDblEquals(test, answers[i][j][0], ra, 0.0000001);
            CuAssertDblEquals(test, answers[i][j][1], dec, 0.0000001);
        }
        printf ("\n");
    }
}

void test_iers_load( CuTest * test ) {
    FILE * bulletinA = fopen( "../data/iers/finals2000A.data", "r" );
    CuAssertPtrNotNullMsg(test, "could not open default iers file", bulletinA );

    IERS iers;

    iers_create( &iers );
    CuAssertPtrNotNullMsg(test, "failed to allocate EOPs", iers.eops);

    int result = iers_load( &iers, bulletinA );
    CuAssertIntEquals( test, 10105, result );

    // compute and print the average assuming samples are evenly spaced...
    IERS_EOP avg = {0.0,'M',0.0,0.0,0.0,0.0,'M',0.0,0.0};
    for(int n=0; n<iers.size; n++) {
        IERS_EOP * eop = &(iers.eops[n]);

        // ensure flags are valid
        CuAssert(test, "Invalid flag for polar offset",
                eop->pm_flag=='I' || eop->pm_flag=='P' || eop->pm_flag==' ' );

        CuAssert(test, "Invalid flag for utc-ut1",
                 eop->dt_flag=='I' || eop->dt_flag=='P' || eop->dt_flag==' ' );

        avg.mjd+=eop->mjd;
        avg.pm_x+=eop->pm_x;
        avg.pm_x_err+=eop->pm_x_err;
        avg.pm_y+=eop->pm_y;
        avg.pm_y_err+=eop->pm_y_err;
        avg.ut1_utc+=eop->ut1_utc;
        avg.ut1_utc_err+=eop->ut1_utc_err;
    }
    avg.mjd/=iers.size;
    avg.pm_x/=iers.size;
    avg.pm_x_err/=iers.size;
    avg.pm_y/=iers.size;
    avg.pm_y_err/=iers.size;
    avg.ut1_utc/=iers.size;
    avg.ut1_utc_err/=iers.size;
    fprintf( stdout, "Average Earth Orientation:\n");
    iers_print_eop( &avg, stdout );
    fprintf( stdout, "\n");
    fflush( stdout );

    iers_free( &iers );

    fclose( bulletinA );
}

// a helper method which serches for the upper bound with a linear search. used to test correctness of binary search
IERS_EOP * linear_search( IERS * iers, jday time ) {
    // linear search for upper bound
    int n = 0;
    while (n < iers->size)
        if (time <= iers->eops[n].mjd)
            break;
        else n++;
    // check bounds
    if (n==iers->size)
        return NULL;
    else
        return &(iers->eops[n]);
}

void test_iers_search( CuTest * test ) {
    // create the IERS structure
    IERS iers;
    if (!iers_create( &iers ))
        CuFail(test, "failed to initialize iers structure");

    // generate a contiguous sequence of days like the IERS datasets
    int count = 50;
    jday offset = 2400000.5;
    double step = 0.5;
    IERS_EOP temp;
    memset(&temp, 0, sizeof(IERS_EOP));
    for(int n=0; n<count; n++) {
        temp.mjd = offset + n*step;
        int result = iers_add( &iers, &temp );
        CuAssertIntEquals_Msg(test, "iers_add failed", 0, result);
    }

    // test bounds
    IERS_EOP * eop;
    eop = iers_search( &iers, offset - step );
    CuAssertPtrEquals_Msg(test,"searches before first item should return null", NULL, eop);
    eop = iers_search( &iers, offset + (count+1) * step );
    CuAssertPtrEquals_Msg(test,"searches after last item should return null", NULL, eop);

    // search for exact matches to every time
    for (int n=0; n<count; n++) {
        jday time = offset + step * n;

        // search for exact match
        IERS_EOP * leop = linear_search( &iers, time );
        IERS_EOP * beop = iers_search( &iers, time );
        CuAssertPtrEquals_Msg(test,
                "binary search for upper bound of matching time does not return same result as a linear search",
                leop, beop);
        CuAssertDblEquals_Msg(test, "Returned search Time does not match", time, beop->mjd, 0.0);

        // search for midpoints of intervals
        if(n==count-1)
            continue;
        leop = linear_search( &iers, time+(step/2) );
        beop = iers_search( &iers, time+(step/2) );
        CuAssertPtrEquals_Msg(test,
                  "binary search for upper bound of interspersed time does not return same result as a linear search",
                  leop, beop);
        CuAssert(test, "Returned search time is not an upper bound", time<=beop->mjd);
        // should probably test the lower element as well...

        // NOTE : might want to extend this to test interpolation or rounding...
    }
}

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
                    .ra = dms2deg(18, 36, 56.336939), // this actually remains in hours despite using dms!
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

