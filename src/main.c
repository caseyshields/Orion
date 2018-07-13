#include <sys/time.h>

#include "h/tracker.h"
#include "h/orion.h"
#include "h/catalog.h"
#include "h/main.h"
#include "h/util.h"
#include "h/sockets.h"

int main( int argc, char *argv[] ) {
    // create and configure the Orion server
    Orion orion;
    orion_create( &orion );
    configure_tracker( argc, argv, &(orion.tracker) );

    // create and configure the FK6 catalog
    Catalog catalog;
    memset(&catalog, 0, sizeof(catalog));
    catalog_create( &catalog, 1024 );
    configure_catalog( argc, argv, &catalog);
    if (!catalog.size)
        goto CLEANUP;

    // get server address, should be in dotted quad notation
    char *ip = get_arg(argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port;
    char *arg = get_arg(argc, argv, "-port", "43210");
    port = (unsigned short) atoi(arg);

    // connect and start the orion server
    if (orion_connect( &orion, ip, port ))
        goto EXIT;
    if (orion_start( &orion ))
        goto DISCONNECT;

    // start the main ui loop
    while( TRUE ) {

//        // update and print out the time
//        tracker_set_time( &tracker, get_time() );
//        tracker_print_time( &tracker );

        // todo print prompt and get next user command
        char *line = NULL;
        int result;
        size_t size = 0 ;
        ssize_t read = get_input( "", &line, &size );

        // set current time ///////////////////////////////////////////////////
        if( strncmp("time ", line, 5)==0 ) {
            int year, month, day, hour, minute;
            double seconds;

            char stamp[32];
            result = sscanf( line, "time %s\n", stamp);
            if( result==0 ) {
                char * stamp = tracker_get_stamp( &(orion.tracker) );
                printf( "current time %s\n", stamp );
                continue;
            } else {

            }
        }

        // search name ////////////////////////////////////////////////////////
        if( strncmp( "name ", line, 5)==0 ) {
            char name[32];
            result = sscanf( line, "name %32s\n", name);
            if( result==0 ) {
                printf( "usage: search <name>\n");
                continue;
            }
            int check_name( Entry *entry ) {
                return NULL != strstr(entry->novas.starname, name );
            }
            Catalog* results = catalog_filter(&catalog, check_name, NULL);
            catalog_each( results, entry_print );
            free( results );
        }

        // search horizon patch ///////////////////////////////////////////////
        if (strncmp("search ", line, 7) == 0) {
            float mag;
            double az_0, az_1, zd_0, zd_1;
            result = scanf("search %f %lf %lf %lf %lf\n", &mag, &az_0, &az_1, &zd_0, &zd_1 );
            if( result == 0) {
                printf( "usage: search <min magnitude> <min az> <max az> <min zd> <max zd>\n");
                continue;
            }
            search( &catalog, &(orion.tracker), az_0, az_1, zd_0, zd_1, mag );
        }

        // start the sensor control thread ////////////////////////////////////
        if (strncmp( "start", line, 5 ) == 0) {
            if (orion_start( &orion ))
                printf("Sensor control thread already started.\n");
        }

        // select a target ////////////////////////////////////////////////////
        if (strncmp("track", line, 5) == 0) {
            long id = 0;
            result = sscanf( line, "track %ld\n", &id );
            Entry * entry = catalog_select( &catalog, id );
            if( entry ) {
                cat_entry target = entry->novas;
                orion_track(&orion, target);
            } else {
                printf("Could not find star %ld in catalog\n", id);
            }
        }

        // stop the sensor control thread and exit ////////////////////////////
        if( strncmp("exit", line, 4)==0 ) {
            orion_stop( &orion );
            break;
        }

        // get help with the commands /////////////////////////////////////////
        if( strncmp( "help", line, 4 )==0 ) {
            printf("Commands:\nname <substr>\nsearch <> <> <> <> <>\ntrack <id>\nexit\n");
        }
    }

    DISCONNECT:
    orion_disconnect( &orion );

    CLEANUP:
    catalog_free_entries( &catalog );
    catalog_free( &catalog );
//    orion_free( &orion );

    EXIT:
    if (strlen(orion.error)) {
        fprintf(stdout, orion.error);
        fflush(stdout);
        return 1;
    }
    return 0;
}

int search(
        Catalog * catalog,
        Tracker * tracker,
        double az_min, double az_max,
        double zd_min, double zd_max,
        float mag_min)
{
    // first find the stars which are bright enough
    int is_bright( Entry * entry ) {
        return entry->magnitude >= mag_min;
    }
    Catalog * bright_stars = catalog_filter( catalog, is_bright, NULL );

    // transform the entire catalog of bright stars
    void to_horizon(Entry * entry) {
        tracker_to_horizon( tracker,
                            &(entry->novas),
                            &(entry->zenith_distance),
                            &(entry->topocentric_azimuth) );
    }
    catalog_each( bright_stars, &to_horizon );

    // filter the stars by their horizon coordinates
    int in_patch(Entry *entry) {
        return entry->zenith_distance > zd_min
               && entry->zenith_distance < zd_max
               && entry->topocentric_azimuth > az_min
               && entry->topocentric_azimuth < az_max;
    }
    Catalog * results = catalog_filter( bright_stars, in_patch, NULL );
    // todo if catalog == results, filter the catalog in place...

    // todo sort the remaining stars by brightness
    //

    // print the search results for the user
    void print_star( Entry * entry ) {
        cat_entry * novas = &(entry->novas);
        printf( "%s%ld\t%s\t%f\t%lf\t%lf\n",
                novas->catalog, novas->starnumber,
                novas->starname, entry->magnitude,
                entry->topocentric_azimuth, entry->zenith_distance);
    }
    catalog_each( results, print_star );

    int count = results->size;

    // release catalogs
    catalog_free_entries( bright_stars );
    free( bright_stars );
    catalog_free_entries( results );
    free( results );

    return count;
}

/** extracts tracker information from the program arguments and constructs a model of a tracker */
void configure_tracker( int argc, char* argv[], Tracker* tracker ) {

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    char* arg = get_arg( argc, argv, "-ut1_utc", UT1_UTC );
    double ut1_utc = atof( arg );

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    arg = get_arg( argc, argv, "-leap_secs", TAI_UTC );
    double leap_secs = atof( arg );

    // create the tracker
    tracker_create(tracker, ut1_utc, leap_secs);

    // set the tracker's time in UTC
    tracker_set_time(tracker, get_time());

    // geodetic coordinates
    arg = get_arg(argc, argv, "-latitude", LATITUDE);
    double latitude = atof(arg);

    arg = get_arg(argc, argv, "-longitude", LONGITUDE);
    double longitude = atof(arg);

    arg = get_arg(argc, argv, "-height", HEIGHT);
    double height = atof(arg);

    // set the location of the tracker
    tracker_set_location(tracker, latitude, longitude, height);

    // site temperature in degrees celsius
    arg = get_arg( argc, argv, "-celsius", TEMPERATURE );
    double celsius = atof( arg );

    // atmospheric pressure at site in millibars
    arg = get_arg( argc, argv, "-millibars", PRESSURE );
    double millibars = atof( arg );

    // set the weather
    tracker_set_weather(tracker, celsius, millibars);
}

///** Creates an ip address for the sensor from the arguments. */
//void configure_address(int argc, char* argv[], struct sockaddr_in* address) {
//
//    // get server address, should be in dotted quad notation
//    char *ip = get_arg(argc, argv, "-ip", "127.0.0.1");
//
//    // get server socket port number
//    unsigned short port; // port
//    char *arg = get_arg(argc, argv, "-port", "43210");
//    port = (unsigned short) atoi(arg);
//
//    // construct server address structure
//    address->sin_family = AF_INET; // internet address family
//    address->sin_addr.s_addr = inet_addr( ip ); // server ip
//    address->sin_port = htons( port ); // server port
//}

void configure_catalog( int argc, char* argv[], Catalog* catalog ) {

    FILE * readme = fopen( "../data/fk6/ReadMe", "r" );

    // load the first part of FK6
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields(fk6_1, readme, FK6_1_HEADER);
    FILE * data1 = fopen( "../data/fk6/fk6_1.dat", "r" );
    catalog_load_fk6(catalog, fk6_1, data1);
    fk6_free( fk6_1 );
    fclose( data1 );

    // load the third part
    FK6 * fk6_3 = fk6_create();
    fk6_load_fields(fk6_3, readme, FK6_3_HEADER);
    FILE * data3 = fopen( "../data/fk6/fk6_3.dat", "r" );
    catalog_load_fk6(catalog, fk6_3, data3);
    fk6_free( fk6_3 );
    fclose( data3 );
    fclose( readme );

    // TODO add some arguments to control the catalog loaded
//    // get the location of the catalog data
//    char *path = get_arg(argc, argv, "-catalog", "../data/FK6.txt");
//    // create and load a catalog
//    FILE *file = fopen(path, "r");
//    catalog_load_fk5(catalog, file);


    // TODO add some other notable stars, such as polaris
//    cat_entry polaris = {"alpha UMi", "HIP",   0,  2.530301028,  89.264109444,
//                    44.22, -11.75,  7.56, -17.4};
//    stars[N_STARS] = {
//            {"Delta ORI", "HIP", 1,  5.533444639,  -0.299091944,
//                    1.67,   0.56,  3.56,  16.0},
//            {"Theta CAR", "HIP", 2, 10.715944806, -64.394450000,
//                    -18.87, 12.06,  7.43,  24.0}};
//    catalog_add( catalog, polaris );

}

// todo eventually move this configuration code to interactive commands and make a script instead...
// mark time
// set time
// set orientation ( ut1-utc, leapsecs, x, y )
// set location ( lat, longitude, height )
// set weather ( temperature, pressure )
// set sensor ( ip, port )
