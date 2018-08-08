#include <assert.h>
#include "util/crc.h"
#include "util/vmath.h"
#include "util/io.h"
#include "data/tats.h"
#include "engine/tracker.h"
#include "engine/catalog.h"
#include "../../lib/cutest-1.5/CuTest.h"

void benchmark( Catalog* catalog, Tracker* tracker, int trials );
void test_conversions( CuTest * test );
void test_time( CuTest * test );
void test_tats( CuTest * test );
void test_crc( CuTest * test );
void test_FK6( CuTest * test );
void test_BSC5();

int main() {
    CuSuite * suite = CuSuiteNew();
    SUITE_ADD_TEST( suite, test_conversions );
    SUITE_ADD_TEST( suite, test_FK6 );
    SUITE_ADD_TEST( suite, test_time );
    SUITE_ADD_TEST( suite, test_tats );
    SUITE_ADD_TEST( suite, test_crc );
    // note you can add suites to suites if you want to add a bit more organization to the tests

    CuString * output = CuStringNew();
    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    CuSuiteDetails( suite, output );
    printf("%s\n", output->buffer);
}

void test_conversions( CuTest * test ) {
    int d = 0, m = 0;
    double s = 0.0;
    for (long seconds=0; seconds<360*60*60; seconds++) {
        double degrees = ((double)seconds)/(60.0*60.0);
        degrees2dms(degrees, &d, &m, &s);
        double d2 = dms2degrees(d, m, s);
        char * str = dms2str(d, m, s);
//        printf("%lds\t=\t%f°\t=\t%s\n", seconds, degrees, str);
        free(str);
        CuAssertTrue( test, fabs(degrees-d2) < 0.0000001 );
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
    double start = jday_current();
    int size = catalog->size;

    // track every star in the FK6 catalog
    double tracks [catalog->size][2]; //double latitude, longitude;
    for( int t=0; t<trials; t++ ) {
        for (int n = 0; n < catalog->size; n++) {
            Entry *entry = catalog->stars[n];
            tracker_to_horizon(tracker, &(entry->novas), &tracks[n][0], &tracks[n][1]);
        }
    }

    // get the time
    double end = jday_current();
    double duration = (end - start)*SECONDS_IN_DAY;

    // print the catalog with corresponding tracks
    for( int n=0; n<catalog->size; n++ ) {
        Entry* entry = catalog->stars[n];
        entry_print( entry );
        printf( "Horizon : (zd:%lf, az:%lf)\n\n", tracks[n][0], tracks[n][1] );
    }

    printf( "stars: %d\ntrials: %d\ntime: %lf\nspeed: %lf\n\n", catalog->size, trials, duration, duration/(trials*catalog->size) );
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


/* a simple CLI interface for exercising various orion components. */
//int xmain( int argc, char *argv[] ) {
//    double latitude, longitude, height;
//    double celsius, millibars;
//    double ut1_utc, leap_secs;
//    char *arg;//, *path;
//
//    // geodetic coordinates in degrees
//    arg = get_arg( argc, argv, "-latitude", "38.88972222222222" );
//    if( arg ) latitude = atof( arg );
//
//    arg = get_arg( argc, argv, "-longitude", "-77.0075" );
//    if( arg ) longitude = atof( arg );
//
//    // geodetic height in meters
//    arg = get_arg( argc, argv, "-height", "125.0" );
//    if( arg ) height = atof( arg );
//
//    // site temperature in degrees celsius
//    arg = get_arg( argc, argv, "-celsius", "10.0" );
//    if( arg ) celsius = atof( arg );
//
//    // atmospheric pressure at site in millibars
//    arg = get_arg( argc, argv, "-millibars", "1010.0" );
//    if( arg ) millibars = atof( arg );
//
//    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
//    arg = get_arg( argc, argv, "-ut1_utc", "0.108644" );
//    if( arg ) ut1_utc = atof( arg );
//
//    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
//    arg = get_arg( argc, argv, "-leap_secs", "37.000000" );
//    if( arg ) leap_secs = atof( arg );
//
////    // get the location of the catalog data
////    path = get_arg( argc, argv, "-catalog", "../data/FK6.txt");
//
//    // create the tracker
//    Tracker tracker;
//    tracker_create(&tracker, ut1_utc, leap_secs);
//
//    // set the tracker's time in UTC
//    tracker_set_time(&tracker, jday_current());
//    tracker_print_time(&tracker, stdout);
//
//    // set the location
//    tracker_set_location(&tracker, latitude, longitude, height);
//    tracker_set_weather(&tracker, celsius, millibars);
//    tracker_print_site(&tracker, stdout);
//
//    Catalog * catalog = catalog_create( NULL, 1024 );
//    Catalog * results = catalog_create( NULL, 64 );
//
//    // load metadata from readme
//    FILE * readme = fopen("../data/fk6/ReadMe", "r");
//    assert(NULL != readme);
//    FK6 * fk6_1 = fk6_create();
//    fk6_load_fields( fk6_1, readme, FK6_1_HEADER );
//    FK6 * fk6_3 = fk6_create();
//    fk6_load_fields( fk6_3, readme, FK6_3_HEADER );
//    fclose(readme);
//
//    // load first part of FK6
//    FILE * data1 = fopen("../data/fk6/fk6_1.dat", "r");
//    assert(NULL != data1);
//    catalog_load_fk6( catalog, fk6_1, data1 );
//    fclose( data1 );
//    fk6_free( fk6_1 );
//    free( fk6_1 );
//
//    // load third part of FK6
//    FILE * data3 = fopen("../data/fk6/fk6_3.dat", "r");
//    assert(NULL != data3);
//    catalog_load_fk6(catalog, fk6_3, data3);
//    fclose( data3 );
//    fk6_free( fk6_3 );
//    free( fk6_3 );
//
//    char *line = NULL;
//    size_t size = 0 ;
//    ssize_t read = 0;
//    while(1) {
//
//        // update the current time and print a prompt
//        tracker_set_time(&tracker, jday_current());
//        printf("orion[%s]", jday2stamp(tracker.utc));
//        read = get_input("", &line, &size);
//
//        // select stars whose name contains a given substring
//        if( strncmp( "name", line, 4 ) == 0 ) {
//            get_input("containing", &line, &size);
//            catalog_search_name(catalog, line, results);
//            catalog_print(results);
//            catalog_clear(results);
//        }
//
//            // search within a lesser circle of the catalog
//        else if( strncmp( "dome", line, 4 )==0 ) {
//            get_input( "right ascension hours", &line, &size );
//            double ra = atof( line );//hours2radians( atof( line ) );
//
//            get_input( "declination degrees", &line, &size );
//            double dec = atof( line );//degrees2radians( atof( line ) );
//
//            get_input( "radius degrees", &line, &size );
//            double rad = atof( line );//degrees2radians( atof( line ) );
//
//            catalog_search_dome(catalog, ra, dec, rad, results);
//            catalog_print(results);
//            printf( "\n%d stars found.\n", results->size );
//            catalog_clear( results );
//        }
//
//            // search within a lesser circle of the catalog
//        else if( strncmp( "patch", line, 5 )==0 ) {
//            get_input( "minimum right ascension hours", &line, &size );
//            double ra_min = atof( line );
//
//            get_input( "maximum right ascension hours", &line, &size );
//            double ra_max = atof( line );
//
//            get_input( "minimum declination degrees", &line, &size );
//            double dec_min = atof( line );
//
//            get_input( "maximum declination degrees", &line, &size );
//            double dec_max = atof( line );
//
//            catalog_search_patch(catalog, ra_min, ra_max, dec_min, dec_max, results);
//            catalog_print( results );
//            printf( "\n%d stars found.\n", results->size );
//            catalog_clear( results );
//        }
//
//            // figure out spherical celestial coordinates of the local zenith
//        else if( strncmp( "zenith", line, 6 )==0 ) {
//            double ra = 0, dec = 0;
//            tracker_zenith(&tracker, &ra, &dec);
//            printf( "Current zenith coodinates : (ra:%lf, dec:%lf)\n", ra, dec );
//        }
//
//            // print the entire catalog contents
//        else if( strncmp( "print", line, 5 )==0 )
//            catalog_print( catalog );
//
//            // run the benchmark
//        else if( strncmp( "bench", line, 5 )==0 )
//            benchmark( catalog, &tracker, 100 );
//
//        else if( strncmp( "convert", line, 7)==0 )
//            test_conversions();
//
//        else if( strncmp("fk6", line, 3)==0 )
//            test_FK6();
//
//        else if( strncmp("jday", line, 4)==0 )
//            test_time();
//
//            // clean up the program components and exit the program
//        else if( strncmp( "exit", line, 4 )==0 ) {
//
//            catalog_clear( results );
//            catalog_free( results );
//            free( results );
//            results = 0;
//
//            catalog_free( catalog );
//            free( catalog );
//            catalog = 0;
//
//            free( line );
//            line = 0;
//
//            exit(0);
//        }
//
//            // print available commands
//        else {
//            printf( "Commands include \n\tstar\n\tzenith\n\tdome\n\tpatch\n\tprint\n\tbench\n\texit\n" );
//        }
//    }
//}
