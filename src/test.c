#include <assert.h>

#include "h/tracker.h"
#include "h/catalog.h"
#include "h/vmath.h"
#include "h/util.h"

// todo now that I have the interactive command line figured out, this should be turned into a battery of unit tests- or whatever is available in C

/** A simple command line app for testing and exercising different features of
 * the star tracker. Accepts configuration parameters as command line arguments;
 * <ul>
 * <li>-latitude (tracker location in degrees)</li>
 * <li>-longitude (tracker location in degrees)</li>
 * <li>-height (tracker location in meters)</li>
 * <li>-celsius (temp at tracker location)</li>
 * <li>-millibars (pressure at tracker location)</li>
 * <li>-ut1_utc (difference between TAI and TT in seconds)</li>
 * <li>-leap_secs (difference between UT1 and TAI in integer seconds)</li>
 * <li>-catalog (path of the catalog file)</li>
 * </ul>
 * @author Casey Shields
 */

void benchmark( Catalog* catalog, Tracker* tracker, int trials );
void test_conversions();
void test_FK6();
void test_BSC5();
void test_time();

/** a simple CLI interface for exercising various orion components. */
int main( int argc, char *argv[] ) {
    double latitude, longitude, height;
    double celsius, millibars;
    double ut1_utc, leap_secs;
    char *arg;//, *path;

    // geodetic coordinates in degrees
    arg = get_arg( argc, argv, "-latitude", "38.88972222222222" );
    if( arg ) latitude = atof( arg );

    arg = get_arg( argc, argv, "-longitude", "-77.0075" );
    if( arg ) longitude = atof( arg );

    // geodetic height in meters
    arg = get_arg( argc, argv, "-height", "125.0" );
    if( arg ) height = atof( arg );

    // site temperature in degrees celsius
    arg = get_arg( argc, argv, "-celsius", "10.0" );
    if( arg ) celsius = atof( arg );

    // atmospheric pressure at site in millibars
    arg = get_arg( argc, argv, "-millibars", "1010.0" );
    if( arg ) millibars = atof( arg );

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    arg = get_arg( argc, argv, "-ut1_utc", "0.108644" );
    if( arg ) ut1_utc = atof( arg );

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    arg = get_arg( argc, argv, "-leap_secs", "37.000000" );
    if( arg ) leap_secs = atof( arg );

//    // get the location of the catalog data
//    path = get_arg( argc, argv, "-catalog", "../data/FK6.txt");

    // create the tracker
    Tracker tracker;
    tracker_create(&tracker, ut1_utc, leap_secs);

    // set the tracker's time in UTC
    tracker_set_time(&tracker, jday_current());
    tracker_print_time(&tracker, stdout);

    // set the location
    tracker_set_location(&tracker, latitude, longitude, height);
    tracker_set_weather(&tracker, celsius, millibars);
    tracker_print_site(&tracker, stdout);

    Catalog * catalog = catalog_create( NULL, 1024 );

    // load the first part of FK6
    FILE * readme = fopen( "../data/fk6/ReadMe", "r" );
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields(fk6_1, readme, FK6_1_HEADER);
    FILE * data1 = fopen( "../data/fk6/fk6_1.dat", "r" );
    catalog_load_fk6(catalog, fk6_1, data1);
    fk6_free( fk6_1 );
    fclose( data1 );
    fclose( readme );

    // load the third part
    readme = fopen( "../data/fk6/ReadMe", "r" );
    FK6 * fk6_3 = fk6_create();
    fk6_load_fields(fk6_3, readme, FK6_3_HEADER);
    FILE * data3 = fopen( "../data/fk6/fk6_3.dat", "r" );
    catalog_load_fk6(catalog, fk6_3, data3);
    fk6_free( fk6_3 );
    fclose( data3 );
    fclose( readme );

    char *line = NULL;
    size_t size = 0 ;
    ssize_t read = 0;
    while(1) {

        // update the current time and print a prompt
        tracker_set_time(&tracker, jday_current());
        printf( "orion[%s]", jday2stamp(tracker.utc) );
        read = get_input( "", &line, &size );

        // select a specific star by number
        if( strncmp( "star", line, 4 ) == 0 ) {
//            // filter by ID
//            get_input("catalog number", &line, &size);
//            int catalog_id = atoi(line);
//            int check_id( Entry *entry ) {
//                return entry->starnumber == catalog_id ? 1 : 0;
//            }
//            Catalog* results = catalog_filter(catalog, &check_id, NULL);

            // filter by name
            get_input("catalog name", &line, &size);
            int check_name( Entry *entry ) {
                return NULL != strstr(entry->novas.starname, line );
            }
            Catalog* results = catalog_filter(catalog, &check_name, NULL);

            // transform each star to local coordinates
            void process( Entry *entry ) {
                double zd=0, az=0;
                tracker_to_horizon(&tracker, &(entry->novas), &zd, &az);
                entry_print( entry );
                printf( "\tlocal : { zd:%lf, az:%lf}\n", zd, az );
            }
            catalog_each( results, process );

            catalog_free(results);
        }

        // search within a lesser circle of the catalog
        else if( strncmp( "dome", line, 4 )==0 ) {
            get_input( "right ascension hours", &line, &size );
            double ra = atof( line );//hours2radians( atof( line ) );

            get_input( "declination degrees", &line, &size );
            double dec = atof( line );//degrees2radians( atof( line ) );

            get_input( "radius degrees", &line, &size );
            double rad = atof( line );//degrees2radians( atof( line ) );

            Catalog* results = catalog_search_dome(catalog, ra, dec, rad, NULL);
            catalog_print(results);
            printf( "\n%d stars found.\n", results->size );
            catalog_free(results);
        }

        // search within a lesser circle of the catalog
        else if( strncmp( "patch", line, 5 )==0 ) {
            get_input( "minimum right ascension hours", &line, &size );
            double ra_min = atof( line );

            get_input( "maximum right ascension hours", &line, &size );
            double ra_max = atof( line );

            get_input( "minimum declination degrees", &line, &size );
            double dec_min = atof( line );

            get_input( "maximum declination degrees", &line, &size );
            double dec_max = atof( line );

            Catalog* results = catalog_search_patch(catalog, ra_min, ra_max, dec_min, dec_max, NULL);
            catalog_print(results);
            printf( "\n%d stars found.\n", results->size );
            catalog_free(results);
        }

        // figure out spherical celestial coordinates of the local zenith
        else if( strncmp( "zenith", line, 6 )==0 ) {
            double ra = 0, dec = 0;
            tracker_zenith(&tracker, &ra, &dec);
            printf( "Current zenith coodinates : (ra:%lf, dec:%lf)\n", ra, dec );
        }

        // print the entire catalog contents
        else if( strncmp( "print", line, 5 )==0 ) {
            catalog_print(catalog);
        }

        // run the benchmark
        else if( strncmp( "bench", line, 5 )==0 ) {
            benchmark( catalog, &tracker, 100 );
        }

        else if(strncmp( "convert", line, 7)==0 ) {
            test_conversions();
        }

        else if( strncmp("fk6", line, 3)==0) {
            test_FK6();
        }

        // clean up the program components and exit the program
        else if( strncmp( "exit", line, 4 )==0 ) {
            catalog_free_entries(catalog);
            catalog_free(catalog);
            free( line );
            exit(0);
        }

        // print available commands
        else {
            printf( "Commands include \n\tstar\n\tzenith\n\tdome\n\tpatch\n\tprint\n\tbench\n\texit\n" );
        }
    }
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
        entry_print(entry);
        printf( "Horizon : (zd:%lf, az:%lf)\n\n", tracks[n][0], tracks[n][1] );
    }

    printf( "stars: %d\ntrials: %d\ntime: %lf\nspeed: %lf\n\n", catalog->size, trials, duration, duration/(trials*catalog->size) );
}

void test_conversions() {
    int d,h,m;
    double s, degrees, second = (1.0/24/60/60);
    for (long seconds=0; seconds<360*60*60; seconds++) {
        double degrees = ((double)seconds)/(60.0*60.0);
        degrees2dms(degrees, &d, &m, &s);
        double d2 = dms2degrees(d, m, s);
        char * str = dms2str(d, m, s);
//        printf("%lds\t=\t%f°\t=\t%s\n", seconds, degrees, str);
        free(str);
        assert( fabs(degrees-d2) < 0.0000001 );
    }
}

void test_FK6() {
    FILE * readme = fopen("../data/fk6/ReadMe", "r");
    assert(NULL != readme);

    // load first part
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields( fk6_1, readme, FK6_1_HEADER );

    FILE * data1 = fopen("../data/fk6/fk6_1.dat", "r");
    assert(NULL != data1);
    Catalog * catalog1 = catalog_load_fk6(NULL, fk6_1, data1);
    fclose( data1 );
    catalog_print( catalog1 );

    // load third part
    FK6 * fk6_3 = fk6_create();
    fk6_load_fields( fk6_3, readme, FK6_3_HEADER );

    FILE * data3 = fopen("../data/fk6/fk6_3.dat", "r");
    assert(NULL != data3);
    Catalog * catalog3 = catalog_load_fk6(NULL, fk6_3, data3);
    fclose( data3 );
    catalog_print( catalog3 );

    fclose( readme );
    fk6_free( fk6_1 );
    fk6_free( fk6_3 );
}

// try using the metadata loader to load the yale
void test_BSC5() {

    // TODO readme columns in bsc5 do not line up, in fact they overlap. so this will not work with the FK6 loader
    // it could probably be made to work by splitting on whitespace.

    FILE * readme = fopen("../data/bsc5/ReadMe", "r");
    assert(NULL != readme);

    // load first part
    char* header = "Byte-by-byte Description of file: catalog\n";
    FK6 * bsc5 = fk6_create();
    fk6_load_fields( bsc5, readme, header );//FK6_1_HEADER );

    FILE * data = fopen("../data/bsc5/catalog", "r");
    assert(NULL != data);
    Catalog * catalog = catalog_load_fk6(NULL, bsc5, data);
    fclose( data );
    catalog_print( catalog );

    fclose( readme );
    fk6_free( bsc5 );
}

void test_time() {
    // check that lenient input formatting interprets the timestamp consistently
    char * inputs[] = {"2000/1/2 3:4:5.006",
                       "2000/1/2 03:04:05.006123"};

    // output formatting is consistent
    char * output = "2000/01/02 03:04:05.006";

    // check that the retrieved timestamp is correct for each input
    for( int n = 0; n<2; n++ ) {
        jday time = stamp2jday(inputs[n]);
        assert( jday_is_valid(time) );
        char *copy = jday2stamp(time);
        assert( strcmp(output, copy) == 0);
        free(copy);
    }

    fflush(stdout);

}