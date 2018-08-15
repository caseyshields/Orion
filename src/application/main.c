#include "application/main.h"

// interrupt handler for abnormal program termination
void stop( int signal );

// releases all the resources
void cleanup();

int main( int argc, char *argv[] ) {

    // divert to running all unit test if flagged
    if( has_arg(argc, argv, "-test") )
        test_run();

    // create and configure the Orion server
    orion_create( &orion, 1 );
    configure_tracker( argc, argv, &(orion.tracker) );

    // create and configure the FK6 catalog
    catalog_create( &catalog, 1024 );
    configure_catalog( argc, argv, &catalog );
    if (!catalog.size)
        goto CLEANUP;

    // get server address, should be in dotted quad notation
    char *ip = get_arg(argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port;
    char *arg = get_arg(argc, argv, "-port", "43210");
    port = (unsigned short) atoi(arg);

//    // connect and start the orion server
//    if (orion_connect( &orion, ip, port ))
//        goto EXIT;
//    if (orion_start( &orion ))
//        goto DISCONNECT;

    // start the main ui loop
    int result = 0;
    do {

        // print prompt and get next user command
        char * stamp = jday2stamp( orion_get_time( &orion ) );
        char *line = NULL;
        size_t size = 0 ;
        ssize_t read = get_input( stamp, &line, &size );
        free( stamp );

        // identify the command by the prefix

        // configuration commands
        if( strncmp("time", line, 4)==0 )
            cmd_time(line, &orion);
        else if( strncmp("location", line, 8)==0 )
            cmd_location( line, &orion );
        else if(strncmp("weather", line, 7)==0 )
            cmd_weather( line, &orion );

        // catalog commands
        else if( strncmp( "name", line, 4)==0 )
            cmd_name( line, &catalog);
        else if (strncmp("search", line, 6) == 0)
            cmd_search(line, &orion, &catalog);

        // start the sensor control thread ////////////////////////////////////
        if (strncmp( "start", line, 5 ) == 0) {
            // connect and start the orion server
            if (orion_connect( &orion, ip, port ))
                goto EXIT;
            if (orion_start( &orion ))
                goto DISCONNECT;
        }

        // select a target ////////////////////////////////////////////////////
        if (strncmp("track", line, 5) == 0) {
            long id = 0;
            result = sscanf( line, "track %ld\n", &id );
            Entry * entry = catalog_get(&catalog, id);
            if( entry ) {
                orion_track(&orion, *entry);
            } else {
                printf("Could not find star %ld in catalog\n", id);
            }
        }

        // Print out the current status of the tracker
        if( strncmp("status", line, 6)==0 ) {
            orion_print_status( &orion, stdout );
        }

        if( strncmp("report", line, 6)==0 )
            cmd_report( line, orion );

        // stop the sensor control thread and exit ////////////////////////////
        if( strncmp("exit", line, 4)==0 ) {
            orion_stop( &orion );
            break;
        }

        // get help with the commands /////////////////////////////////////////
        if( strncmp( "help", line, 4 )==0 ) {
            printf("Commands:\nname <substr>\nsearch <> <> <> <> <>\ntrack <id>\nexit\n");
        }
    } while( !result );

    DISCONNECT:
    orion_disconnect( &orion );

    CLEANUP:
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

// configuration commands /////////////////////////////////////////////////////

/** interprets a time command which sets the current time;  */
int cmd_time(char * line, Orion * orion) {
    int year, month, day, hour, min, count;
    double secs, step;
    int result = sscanf(line, "time %u/%u/%u %u:%u:%lf\n",
                        &year, &month, &day, &hour, &min, &secs);
    if (result < 6) {
        printf("usage: report <YYYY>/<MM>/<DD> <hh>:<mm>:<ss.sss>\nnote time should be int UTC\n\n");
        return 1;
    } else {
        jday time = date2jday(year, month, day, hour, min, secs);
        orion_set_time(orion, time);
        return 0;
    }
} // todo add leap seconds and ut1 offset as optional parameters

int cmd_location( char * line, Orion * orion ) {
    double lat=0, lon=0, height=0;
    int result = sscanf(line, "location %lf %lf %lf\n", &lat, &lon, &height );
    if(result < 3) {
        printf( "usage: location <Lat:-90.0 to 90.0> <Lon:0.0 to 360> <height: meters>\n\n" );
        return 1;
    }
    orion_set_location( orion, lat, lon, height );
    return 0;
}

int cmd_weather(char * line, Orion * orion) {
    double temperature=0, pressure=0;
    int result = sscanf(line, "weather %lf %lf\n", &temperature, &pressure );
    if( result<2 ) {
        printf("usage: weather <temperature:celsius> <pressure:millibars>\n\n");
        return 1;
    } else {
        orion_set_weather( orion, temperature, pressure );
        return 0;
    }
}

// catalog commands ///////////////////////////////////////////////////////////

int cmd_name( char * line, Catalog * catalog ) {
    char name[32];
    int result = sscanf( line, "name %32s\n", name);
    if( result==0 ) {
        printf( "usage: search <name>\n\n");
        return 1;
    } else {
        int check_name(Entry *entry) {
            return NULL != strstr(entry->novas.starname, name);
        } // TODO nested functions are Gnu C specific...
        Catalog *results = catalog_filter( catalog, check_name, NULL );
        catalog_each( results, entry_print );
        free( results );
    }
}

/** Currently just hardcoded to load the first and third parts of the FK6 catalog from the default data directory.  */
int cmd_catalog( Catalog * catalog ) {
//    char * data_root = "../data/fk6/";
//    char * metadata = "ReadMe";
//    char * data[2] = {"fk6_1.dat", "fk6_3.dat"};

    // load the FK6 metadata
    FILE * readme = fopen( "../data/fk6/ReadMe", "r" );
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields(fk6_1, readme, FK6_1_HEADER);
    FK6 * fk6_3 = fk6_create();
    fk6_load_fields(fk6_3, readme, FK6_3_HEADER);
    fclose( readme );

    // load the first part of FK6
    FILE * data1 = fopen( "../data/fk6/fk6_1.dat", "r" );
    catalog_load_fk6(catalog, fk6_1, data1);
    fk6_free( fk6_1 );
    free( fk6_1 );
    fclose( data1 );

    // load the third part
    FILE * data3 = fopen( "../data/fk6/fk6_3.dat", "r" );
    catalog_load_fk6(catalog, fk6_3, data3);
    fk6_free( fk6_3 );
    free( fk6_3 );
    fclose( data3 );

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

//int cmd_search(char * line, Orion * orion, Catalog * catalog) {
//    float mag_min;
//    double az_min, az_max, zd_min, zd_max;
//
//    int result = sscanf(line, "search %f %lf %lf %lf %lf\n", &mag_min, &az_min, &az_max, &zd_min, &zd_max );
//    if( result != 1 && result != 5 ) {
//        printf( "usage: search <min magnitude> [<min az> <max az> <min zd> <max zd>]\n");
//        return 1;
//    } else {
//        // obtain a copy of the current tracker state
//        Tracker tracker = orion_get_tracker( orion );
//
//        // first find the stars which are bright enough
//        Catalog * bright_stars = catalog_brighter(catalog, mag_min, NULL);
//
//        // further filter the stars if the command included patch dimensions
//        if ( result == 5 ) {
//            // transform the entire catalog of bright stars using the tracker
//            void to_horizon( Entry * entry ) {
//                tracker_to_horizon( &tracker,
//                                    &(entry->novas),
//                                    &(entry->zenith_distance),
//                                    &(entry->topocentric_azimuth) );
//            } // Gnu C specific: nested functions
//            catalog_each( bright_stars, &to_horizon );
//
//            // filter the stars by their horizon coordinates
////      int in_patch(Entry *entry) {
////          return entry->zenith_distance > zd_min
////                 && entry->zenith_distance < zd_max
////                 && entry->topocentric_azimuth > az_min
////                 && entry->topocentric_azimuth < az_max;
////      }
////      Catalog * results = catalog_filter( bright_stars, in_patch, NULL );
//            Catalog * results = catalog_create( NULL, 64 );
//            for (int n = 0; n<catalog->size; n++) {
//                Entry * entry = catalog->stars[n];
//                if( entry->zenith_distance > zd_min
//                    && entry->zenith_distance < zd_max
//                    && entry->topocentric_azimuth > az_min
//                    && entry->topocentric_azimuth < az_max )
//                    catalog_add( results, entry );
//            }
//
//            catalog_clear( results );
//            catalog_free( results );
//            free( results );
//
//            return count;
//        }
//        // todo sort the remaining stars by brightness
//        //
//
//        // print the search results for the user
//        void print_star( Entry * entry ) {
//            cat_entry * novas = &(entry->novas);
//            printf( "%s%ld\t%s\t%f\t%lf\t%lf\n",
//                    novas->catalog, novas->starnumber,
//                    novas->starname, entry->magnitude,
//                    entry->topocentric_azimuth, entry->zenith_distance);
//        }
//        catalog_each( results, print_star );
//        printf("\n");
//
//        int count = results->size;
//
//        // release catalogs
//        catalog_clear( bright_stars );
//        catalog_free( bright_stars );
//        free( bright_stars );
//
//    }
//
//}

int cmd_report( char * line, Orion * orion, FILE * stream ) {
    double step=0, az=0, zd=0;
    int count=0;

    int result = sscanf(line, "report %lf %u\n", &step, &count );
    if( result < 8) {
        printf( "usage: report <step> <count>\n");
        return 0;
    }

    // get thread safe copies of the tracker and target from the orion server
    Tracker tracker = orion_get_tracker( &orion );
    Entry target = orion_get_target( &orion );
    jday start = orion_get_time( &orion );

    // print tracker information
    tracker_print_site( &tracker, stream );
    // TODO print target information

    // print the header
    fprintf( stream, "UTC\tAZ\tZD\n" );

    // step over the given time interval
    step /= SECONDS_IN_DAY;
    jday end = start + (step*count);
    while( start < end ) {

        // calculate coordinates at the given time
        tracker_set_time( &tracker, start );
        tracker_to_horizon( &tracker, &(target.novas), &az, &zd );

        // print report entry
        char * ts = jday2stamp( start );
        fprintf( stream, "%s\t%010.6lf\t%010.6lf\n", ts, az, zd );
        free( ts );
        start += step;
    }
}

void stop( int signal ) {
    // probably shouldn't do IO in a signal handler
//    fprintf(stdout, "signal handler invoked [%d]\n", signal );
//    fflush( stdout );
    mode = 0;
}

void cleanup() {
    fprintf( stderr, "entering cleanup\n" );

    // release resources
//    if( buffer != NULL )
//        free( buffer );

    if( orion_is_running( &orion ) )
        orion_stop( &orion );

    if( orion_is_connected( &orion ) )
        orion_disconnect( &orion );

    if( catalog )
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
    tracker_set_time(tracker, jday_current());

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

    // load the FK6 metadata
    FILE * readme = fopen( "../data/fk6/ReadMe", "r" );
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields(fk6_1, readme, FK6_1_HEADER);
    FK6 * fk6_3 = fk6_create();
    fk6_load_fields(fk6_3, readme, FK6_3_HEADER);
    fclose( readme );

    // load the first part of FK6
    FILE * data1 = fopen( "../data/fk6/fk6_1.dat", "r" );
    catalog_load_fk6(catalog, fk6_1, data1);
    fk6_free( fk6_1 );
    free( fk6_1 );
    fclose( data1 );

    // load the third part
    FILE * data3 = fopen( "../data/fk6/fk6_3.dat", "r" );
    catalog_load_fk6(catalog, fk6_3, data3);
    fk6_free( fk6_3 );
    free( fk6_3 );
    fclose( data3 );

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
