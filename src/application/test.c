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
    return suite;
}   // note you can add suites to suites if you want to add a bit more organization to the tests

void test_angles( CuTest * test ) {
    int d = 0, m = 0;
    double s = 0.0;
    for (long seconds=0; seconds<360*60*60; seconds++) {
        double degrees = ((double)seconds)/(60.0*60.0);
        degrees2dms(degrees, &d, &m, &s);
        double d2 = dms2degrees(d, m, s);
        char * str = dms2str(d, m, s);
//        printf("%lds\t=\t%f°\t=\t%s\n", seconds, degrees, str);
        free(str);
        CuAssertDblEquals( test, degrees, d2, 0.0000001 );
    }
}

void test_time( CuTest * test ) {
    // check that lenient input formatting interprets the timestamp consistently
    char * inputs[] = {"2000/1/2 3:4:5.006",
                       "2000/1/2 03:04:05.006123"};

    // output formatting is consistent
    char * output = "2000/01/02 03:04:05.006";

    // check that the retrieved timestamp is correct for each input
    for( int n = 0; n<2; n++ ) {
        jday time = stamp2jday(inputs[n]);
        CuAssertTrue( test, jday_is_valid(time) );
        char *copy = jday2stamp(time);
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

        avg.time+=eop->time;
        avg.pm_x+=eop->pm_x;
        avg.pm_x_err+=eop->pm_x_err;
        avg.pm_y+=eop->pm_y;
        avg.pm_y_err+=eop->pm_y_err;
        avg.ut1_utc+=eop->ut1_utc;
        avg.ut1_utc_err+=eop->ut1_utc_err;
    }
    avg.time/=iers.size;
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
        if (time <= iers->eops[n].time)
            break;
        else n++;
    // check bounds
    if (n==0 || n==iers->size)
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
        temp.time = offset + n*step;
        iers_add( &iers, &temp );
    }

    // test bounds
    IERS_EOP * eop;
    eop = iers_search( &iers, offset - step );
    CuAssertPtrEquals_Msg(test,"searches before first item should return null", NULL, eop);
    eop = iers_search( &iers, offset + (count+1) * step );
    CuAssertPtrEquals_Msg(test,"searches after last item should return null", NULL, eop);

    // search for exact matches to every time
    for (int n; n<count; n++) {
        jday time = offset + step * n;

        // search for exact match
        IERS_EOP * leop = linear_search( &iers, time );
        IERS_EOP * beop = iers_search( &iers, time );
        CuAssertPtrEquals_Msg(test,
                "binary search for upper bound of matching time does not return same result as a linear search",
                leop, beop);
        CuAssertDblEquals_Msg(test, "Returned search Time does not match", time, beop->time, 0.0);

        // search for midpoints of intervals
        if(n==count-1)
            continue;
        leop = linear_search( &iers, time+(step/2) );
        beop = iers_search( &iers, time+(step/2) );
        CuAssertPtrEquals_Msg(test,
                  "binary search for upper bound of interspersed time does not return same result as a linear search",
                  leop, beop);
        CuAssert(test, "Returned search time is not an upper bound", time<=beop->time);
        // should probably test the lower element as well...

        // NOTE : might want to extend this to test interpolation or rounding...
    }
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
    jday start = jday_utc();
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
    jday end = jday_utc();
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

