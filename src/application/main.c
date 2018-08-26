#include "application/main.h"

// Application Structure //////////////////////////////////////////////////////

Application app = { .mode=0, .port=0, .ip=NULL, .orion=NULL, .catalog=NULL };

int main( int argc, char *argv[] ) {

    // divert to running test suite if flagged
    if( has_arg(argc, argv, "-test") )
        test_run();

    // register cleanup routines and interrupt handlers
    atexit( cleanup );
    signal(SIGTERM, interrupt_handler);
    signal(SIGINT, interrupt_handler);
    signal(SIGABRT, interrupt_handler);

    // create and configure the application
    app.mode = 1;
    app.orion = orion_create( NULL, 1 );
    app.catalog = catalog_create( NULL, 1024 );

    configure_address( argc, argv, &app );
    configure_tracker( argc, argv, &(app.orion->tracker) );
    configure_catalog( argc, argv, app.catalog );
    if( !app.catalog->size ) {
        alert( "Failed to load catalog" );
        return 1;
    }

    // Main loop
    while( app.mode ) {

        // print prompt and get next user command
        printf( "\n" );
        char * stamp = jday2stamp( orion_get_time( app.orion ) );
        char *line = NULL;
        size_t size = 0 ;
        ssize_t read = get_input( stamp, &line, &size );
        free( stamp );

        // configuration commands
        if( strncmp( "time", line, 4 ) == 0 )
            cmd_time( line, app.orion );
        else if( strncmp( "location", line, 8 ) == 0 )
            cmd_location( line, app.orion );
        else if( strncmp( "weather", line, 7 ) == 0 )
            cmd_weather( line, app.orion );

        // catalog commands
        else if( strncmp( "name", line, 4 ) == 0 )
            cmd_name( line, app.catalog);
//        else if( strncmp( "search", line, 6 ) == 0 )
//            cmd_search( line, orion, catalog );

        // sensor commands
        else if( strncmp( "connect", line, 7 ) == 0 )
            cmd_connect( line, app.orion );
        else if( strncmp( "target", line, 6 ) == 0 )
            cmd_target( line, app.orion, app.catalog );

        // Diagnostic commands
        else if( strncmp( "status", line, 6) == 0 )
            orion_print_status( app.orion, stdout );
        else if( strncmp( "report", line, 6) == 0 )
            cmd_report( line, app.orion, stdout );
        else if( strncmp( "help", line, 4 ) == 0 )
            cmd_help( line );

        else if( strncmp("exit", line, 4)==0 )
            app.mode = 0;
        else
            alert( "Unrecognized command. enter 'help' for a list of commands" );
    }
    return 0;
}

void interrupt_handler(int signal) {
    // flag the main loop to exit
    app.mode = 0;
}

void cleanup() {
    // print error message
    if ( app.orion && strlen(app.orion->error)) {
        fprintf(stderr, "%s\n", app.orion->error);
        fflush(stderr);
    }

    // release components
    if( app.orion ) {
        if( orion_get_mode(app.orion) != ORION_MODE_OFF )
            orion_stop(app.orion);
        if( orion_is_connected(app.orion) )
            orion_disconnect(app.orion);
        free( app.orion );
    }

    if( app.catalog )
        catalog_free( app.catalog );

    if( app.ip && app.ip==LOCALHOST)
        free( app.ip );
}

// Initialization /////////////////////////////////////////////////////////////

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

void configure_address( int argc, char* argv[], Application * app ) { //struct sockaddr_in* address) {

    // get server address, should be in dotted quad notation
    app->ip = get_arg(argc, argv, "-ip", LOCALHOST);

    // get server socket port number
    char * default_port = "43210";
    char *arg = get_arg(argc, argv, "-port", default_port);
    app->port = (unsigned short) atoi(arg);
    if( arg!=default_port )
        free(arg);

//    // construct server address structure
//    address->sin_family = AF_INET; // internet address family
//    address->sin_addr.s_addr = inet_addr( ip ); // server ip
//    address->sin_port = htons( port ); // server port
}

void configure_catalog( int argc, char* argv[], Catalog* catalog ) {
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


// should we add some other bright stars as a stop gap, or just finish the YBS Catalog loader?
//    cat_entry polaris = {"alpha UMi", "HIP",   0,  2.530301028,  89.264109444,
//                    44.22, -11.75,  7.56, -17.4};
//    stars[N_STARS] = {
//            {"Delta ORI", "HIP", 1,  5.533444639,  -0.299091944,
//                    1.67,   0.56,  3.56,  16.0},
//            {"Theta CAR", "HIP", 2, 10.715944806, -64.394450000,
//                    -18.87, 12.06,  7.43,  24.0}};
//    catalog_add( catalog, polaris );
}

// configuration commands /////////////////////////////////////////////////////

int cmd_time(char * line, Orion * orion) {
    int year, month, day, hour, min, count;
    double secs, step;
    int result = sscanf(line, "time %u/%u/%u %u:%u:%lf\n",
                        &year, &month, &day, &hour, &min, &secs);
    if (result < 6) {
        alert("usage: report <YYYY>/<MM>/<DD> <hh>:<mm>:<ss.sss>\nnote time should be int UTC");
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
        alert( "usage: location <Lat:-90.0 to 90.0> <Lon:0.0 to 360> <height: meters>" );
        return 1;
    }
    orion_set_location( orion, lat, lon, height );
    return 0;
}

int cmd_weather(char * line, Orion * orion) {
    double temperature=0, pressure=0;
    int result = sscanf(line, "weather %lf %lf\n", &temperature, &pressure );
    if( result<2 ) {
        alert("usage: weather <celsius> <millibars>");
        return 1;
    } else {
        orion_set_weather( orion, temperature, pressure );
        return 0;
    }
}

// Catalog Commands ///////////////////////////////////////////////////////////
int cmd_name( char * line, Catalog * catalog ) {
    char name[32];
    int result = sscanf( line, "name %32s\n", name);
    if( result==0 ) {
        alert( "usage: search <name>");
        return 1;
    } else {
        int contains(Entry *entry) { // TODO nested functions are Gnu C specific...
            return NULL != strstr(entry->novas.starname, name);
        }
        Catalog *results = catalog_filter( catalog, contains, NULL );
        catalog_each( results, entry_print );
        free( results );
    }
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

// Sensor Commands ////////////////////////////////////////////////////////////
int cmd_connect( char * line, Orion * orion ) {
    unsigned int ip1=0, ip2=0, ip3=0, ip4=0;
    unsigned short port = 0;

    // abort if we are already connected
    if( orion_is_connected( orion ) ) {
        alert( "Sensor already connected" );
        return 1;
    }

    // read the arguments
    int result = sscanf(line, "connect %u.%u.%u.%u:%hu\n", &ip1, &ip2, &ip3, &ip4, &port );

    // overwrite default address if one is supplied
    if( result == 5 ) {
        if( app.ip )
            free( app.ip );
        app.ip = calloc( 32, sizeof(char) );
        sprintf( app.ip, "%u.%u.%u.%u", ip1, ip2, ip3, ip4 );
        app.port = port;

    // abort if command isn't the default structure either
    } else if( result !=0 ) {
        alert( "usage: connect [X.X.X.X:Y]");
        return 1;
    }

    // connect to the sensor and start the control thread
    if( orion_connect( orion, app.ip, app.port) ) {
        alert( "Could not connect to TATS sensor");
        return 1;
    } if( orion_start( orion ) ) {
        alert( "Failed to start TATS control thread" );
        return 1;
    }

    return 0;
}

int cmd_target(char * line, Orion * orion, Catalog * catalog ) {
    unsigned long id = 0;
    int result = sscanf( line, "target %lu\n", &id );

    if( result != 1 ) {
        alert( "usage: track <FK6 ID>");
        return 1;
    }

    Entry * entry = catalog_get(catalog, id);
    if( entry ) {
        orion_track( orion, *entry);
    } else {
        fprintf( stderr, "Could not find star %ld in catalog\n", id );
        fflush( stderr );
    }
    return 0;
}

// Diagnostic Commands ////////////////////////////////////////////////////////
int cmd_report( char * line, Orion * orion, FILE * stream ) {
    double step=0, az=0, zd=0;
    int count=0;

    int result = sscanf(line, "report %lf %u\n", &step, &count );
    if( result != 2 ) {
        alert( "usage: report <step> <count>" );
        return 0;
    }

    // get thread safe copies of the tracker and target from the orion server
    Tracker tracker = orion_get_tracker( orion );
    Entry target = orion_get_target( orion );
    jday start = orion_get_time( orion );

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

int cmd_help( char * line ) {
    printf("Configuration\n\ttime <YYYY/MM/DD HH:MM:SS.ssssss>\n\tlocation <lat(deg)> <lon(deg)> <height(m)>\n\tweather <temp(C)> <pressure(mBar)>\n\n");
    printf("Catalog\n\tname <substr>\n\tsearch <mag> [<> <> <> <>(deg)]\n\n");
    printf("TCN Sensor\n\tconnect [X.X.X.X:Y]\n\ttrack <fk6 id>\n\n");
    printf("Diagnostic\n\tstatus\n\treport <step(sec)> <count>\n\n");
    printf("\texit\n\n");
    printf("<> : required\t[] : optional\t() : units\n");
}
