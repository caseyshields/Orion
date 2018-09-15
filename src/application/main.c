#include "application/main.h"

// Application Structure //////////////////////////////////////////////////////
Application app = {
        .mode=0,
        .port=0,
        .ip=NULL,
        .orion=NULL,
        .catalog=NULL,
        .iers=NULL,
        .time=0.0,
        .eop=NULL
};

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
    app.orion = orion_create( NULL, 1 );
    app.catalog = catalog_create( NULL, 1024 );
    app.iers = iers_create( NULL );

    configure_app( argc, argv, &app );
    configure_orion( argc, argv, app.orion );
    configure_iers( argc, argv, app.iers );
    configure_catalog( argc, argv, app.catalog );

    // find the current earth orientation
    app.eop = iers_search( app.iers, app.time );
    if(!app.eop) {
        app.eop = &MISSING_EOP;
    } // TODO notify user of predicted mode, or outdated catalogs?
    //TODO set eop to current time or load the null eop?

    // Main loop
    while( app.mode ) {

        // print prompt and get next user command
        printf( "\n" );
        char * stamp = jday2str(tt2utc(app.time));
        char *line = NULL;
        size_t size = 0 ;
        ssize_t read = get_input( stamp, &line, &size );
        free( stamp );

        // configuration commands
        if( strncmp( "time", line, 4 ) == 0 )
            cmd_time( line, &app );
        else if( strncmp( "location", line, 8 ) == 0 )
            cmd_location( line, app.orion );
        else if( strncmp( "weather", line, 7 ) == 0 )
            cmd_weather( line, app.orion );

        // catalog commands
        else if( strncmp( "name", line, 4 ) == 0 )
            cmd_name( line, app.catalog);
        else if( strncmp( "search", line, 6 ) == 0 )
            cmd_search( line, &app );

        // sensor commands
        else if( strncmp( "connect", line, 7 ) == 0 )
            cmd_connect( line, &app );
        else if( strncmp( "target", line, 6 ) == 0 )
            cmd_target( line, &app );

        // Diagnostic commands
        else if( strncmp( "status", line, 6) == 0 )
            cmd_status( line, &app, stdout );
        else if( strncmp( "report", line, 6) == 0 )
            cmd_report( line, &app, stdout );
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

    if (app.iers)
        iers_free( app.iers );

    if( app.ip && app.ip==LOCALHOST)
        free( app.ip );
}

// Initialization /////////////////////////////////////////////////////////////

void configure_app( int argc, char* argv[], Application * app ) { //struct sockaddr_in* address) {
    app->mode = 1;
    app->time = utc2tt(jday_now());

    // (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
    char * arg = get_arg( argc, argv, "-ut1_utc", UT1_UTC );
    double ut1_utc = atof( arg );
    MISSING_EOP.ut1_utc = ut1_utc;

    // delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
    arg = get_arg( argc, argv, "-leap_secs", TAI_UTC );
    LEAP_SECONDS = (jday) atof( arg );

    // get server address, should be in dotted quad notation
    app->ip = get_arg(argc, argv, "-ip", LOCALHOST);

    // get server socket port number
    char * default_port = "43210";
    arg = get_arg(argc, argv, "-port", default_port);
    app->port = (unsigned short) atoi(arg);

    if( arg!=default_port )
        free(arg);

//    // construct server address structure
//    address->sin_family = AF_INET; // internet address family
//    address->sin_addr.s_addr = inet_addr( ip ); // server ip
//    address->sin_port = htons( port ); // server port
}

void configure_orion( int argc, char* argv[], Orion * orion ) {

    Tracker * tracker = &(orion->tracker);

    // create the tracker
    tracker_create(tracker);

    // geodetic coordinates
    char * arg = get_arg(argc, argv, "-latitude", LATITUDE);
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

    // notify user of the loaded time
//    tracker_print_site( tracker, stdout );

    // TODO move this to app configuration so it can be displayed with other network settings?
    // Tats Control network latency
    arg = get_arg( argc, argv, "-latency", LATENCY );
    double latency = atof( arg );
    // TODO do rate as well...

    orion_set_latency( orion, latency );
}

void configure_iers(int argc, char* argv[], IERS * iers ) {
    char * path = "../data/iers/finals2000A.data";
    // TODO derive path from root directory!!! Do this for all the file based components!

    printf("Loading IERS Bulletin \"%s\"... ", path);
    FILE * file = fopen(path, "r");
    int read = iers_load( iers, file );

    if (!read) {
        alert( "Failed to read IERS Bulletin data" );
        exit(1);
    }

    char * start = jday2str( iers->eops[0].time );
    char * end = jday2str( iers->eops[iers->size-1].time );
    printf("%s to %s\n", start, end);
    free(start);
    free(end);
}

void configure_catalog( int argc, char* argv[], Catalog* catalog ) {
//    char * data_root = "../data/";
//    char * metadata = "/fk6/ReadMe";
//    char * data[2] = {"fk6_1.dat", "fk6_3.dat"};
// TODO add some arguments to control the catalog loaded

char * path = "../data/fk6/ReadMe";

    // load the FK6 metadata
    printf( "loading FK6 I metadata \"%s\"...", path);
    FILE * readme = fopen( path, "r" );
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields(fk6_1, readme, FK6_1_HEADER);
    printf(" %u fields loaded.\n", fk6_1->cols);

    printf( "loading FK6 III metadata \"%s\"...", path);
    FK6 * fk6_3 = fk6_create();
    fk6_load_fields(fk6_3, readme, FK6_3_HEADER);
    fclose( readme );
    printf(" %u fields loaded.\n", fk6_3->cols);

    // load the first part of FK6
    char * path1 = "../data/fk6/fk6_1.dat";
    printf( "loading catalog \"%s\"...", path1);
    FILE * data1 = fopen( path1, "r" );
    catalog_load_fk6(catalog, fk6_1, data1);
    fk6_free( fk6_1 );
    free( fk6_1 );
    fclose( data1 );
    printf(" %u entries total.\n", catalog->size);

    // load the third part
    char * path3 = "../data/fk6/fk6_3.dat";
    printf( "loading catalog \"%s\"...", path1);
    FILE * data3 = fopen( path3, "r" );
    catalog_load_fk6(catalog, fk6_3, data3);
    fk6_free( fk6_3 );
    free( fk6_3 );
    fclose( data3 );
    printf(" %u entries total.\n", catalog->size);

    if( !catalog->size ) {
        alert( "Failed to load catalog, exiting" );
        exit(1); // maybe we don't exit? they could still target, they just couldn't catalog
    }
}// TODO should we add some other bright stars as a stop gap, or just finish the YBS Catalog loader?

// configuration commands /////////////////////////////////////////////////////
int cmd_time(char * line, Application * cli) {
    int year, month, day, hour, min, count;
    double secs, step;

    Orion * orion = cli->orion;

    int result = sscanf(line, "time %u/%u/%u %u:%u:%lf\n",
                        &year, &month, &day, &hour, &min, &secs);

    //TODO if there is no argument set the current time and set the mode to NRT?

    if (result < 6) {
        alert("usage: report <YYYY>/<MM>/<DD> <hh>:<mm>:<ss.sss>\nnote time should be int UTC");
        return 1;

    } else {
        // convert the user supplied string stamp to a jday, which we assume is utc
        jday utc = date2jday(year, month, day, hour, min, secs);

        // convert that time to terrestrial time
        cli->time = utc2tt(utc);

        // look up the earth orientation parameters for the given day
        cli->eop = iers_search( app.iers, cli->time );
        if (!cli->eop) {
            cli->eop = &MISSING_EOP;
            // TODO warn user about out of bound times?
        }// TODO warn user about predicted mode?

        return 0;
    }
}

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

int cmd_search(char * line, Application * cli) {
    float mag_min;
    double az_min, az_max, el_min, el_max;
    Orion * orion = cli->orion;
    Catalog * catalog = cli->catalog;
    Catalog * stars = NULL;

    // try to parse both formats
    int result = sscanf(line, "search %f %lf %lf %lf %lf\n", &mag_min, &az_min, &az_max, &el_min, &el_max );
    if( result != 5 )
        result = sscanf(line, "search %f\n", &mag_min );

    // only filter stars by brightness
    if( result == 1 ) {
        stars = catalog_brighter(catalog, mag_min, NULL);

    // filter by both brightness and location
    } else if ( result==5 ) {

        if(az_min>az_max) {
            alert("Azimuth minimum must be less than maximum");
            return 1;
        } else if(el_min>el_max) {
            alert("Elevation must be less than maximum");
            return 1;
        }

        // first find the stars which are bright enough
        Catalog * bright = catalog_brighter(catalog, mag_min, NULL);

        // obtain a copy of the current tracker state
        Tracker tracker = orion_get_tracker( orion );

        // for all bright stars
        stars = catalog_create( NULL, 64 );
        for (int n = 0; n<bright->size; n++) {
            Entry * entry = bright->stars[n];

            // transform the catalog coordinates to topocentric coordinates in spherical and rectilinear.
            tracker_point( &tracker, app.time, &(entry->novas) );

            // if the coordinates are within the patch, add them to the results
            if( tracker.elevation > el_min
                && tracker.elevation < el_max
                && tracker.azimuth > az_min
                && tracker.azimuth < az_max )
                catalog_add( stars, entry );
        }
        catalog_clear( bright );
        catalog_free( bright );
        free( bright );

    // alert the user if the command could not be read
    } else {
        alert( "usage: search <min magnitude> [<min az> <max az> <min zd> <max zd>]");
        return 1;
    }

    // print the answer if there is one
    if( stars!=NULL ) {
        // todo sort the remaining stars by brightness

        // print the search results for the user
        for (int n = 0; n<stars->size; n++) {
            Entry * entry = stars->stars[n];

            cat_entry *novas = &(entry->novas);
            printf("%s%ld\t%s\t%f\t%lf\t%lf\n",
                   novas->catalog, novas->starnumber,
                   novas->starname, entry->magnitude,
                   entry->topocentric_azimuth, entry->zenith_distance);
        }

        printf( "%u stars found.\n", (unsigned int)stars->size );

        // release catalogs
        catalog_clear(stars);
        catalog_free(stars);
        free(stars);
    }
    return 0;
}

// Sensor Commands ////////////////////////////////////////////////////////////
int cmd_connect( char * line, Application * cli ) {
    unsigned int ip1=0, ip2=0, ip3=0, ip4=0;
    unsigned short port = 0;

    // abort if we are already connected
    if( orion_is_connected( cli->orion ) ) {
        alert( "Sensor is already connected" );
        return 1;
    }

    // read the arguments
    int result = sscanf(line, "connect %u.%u.%u.%u:%hu\n", &ip1, &ip2, &ip3, &ip4, &port );
    // TODO add an optional time bias parameter?

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
    if( orion_connect( cli->orion, app.ip, app.port) ) {
        alert( "Could not connect to TATS sensor");
        return 1;
    } if( orion_start( cli->orion ) ) {
        alert( "Failed to start TATS control thread" );
        return 1;
    }

    return 0;
}

int cmd_target(char * line, Application * cli ) {//Orion * orion, Catalog * catalog ) {
    unsigned long id = 0;
    int result = sscanf( line, "target %lu\n", &id );

    if( result != 1 ) {
        alert( "usage: track <FK6 ID>");
        return 1;
    }

    Entry * entry = catalog_get(cli->catalog, id);
    if( entry ) {

        // set earth orientation parameters for the current time
        jday utc = jday_now();
        IERS_EOP * eop = iers_search( cli->iers, utc );
        orion_set_earth_orientation( cli->orion, eop );
        // this will be good as long as the user doesn't track a target for a day. Then it will be slightly less good...

        orion_set_target( cli->orion, entry);
    } else {
        fprintf( stderr, "Could not find star %ld in catalog\n", id );
        fflush( stderr );
    }
    return 0;
}

// Diagnostic Commands ////////////////////////////////////////////////////////
int cmd_status(char * line, Application * cli, FILE * stream ) {
    Orion * orion = cli->orion;

    char * stamp = jday2str(cli->time);
    fprintf( stream, "time:\t%s UTC\t(%+05.3lf TT)\n", stamp, cli->time );

    Tracker tracker = orion_get_tracker( orion );
    tracker_print_site( &tracker, stream );

    Entry target = orion_get_target( orion );
    if( target.novas.starnumber ) {

        // TODO get and print tracker time since it is now decoupled from the cli time!

        tracker_point( &tracker, cli->time, &(target.novas) );
        fprintf( stream, "target:\n\t%s %ld: %s\n\t%8.4lf°az % 8.4lf°el\n\t(%lf, %lf, %lf)\n\tVmag: %3.1lf\n",
                target.novas.catalog, target.novas.starnumber, target.novas.starname,
                tracker.azimuth, tracker.elevation,
                tracker.efg[0], tracker.efg[1], tracker.efg[2], target.magnitude);
//        // print out an example midc01 message
//        MIDC01 midc01;
//        create_tracking_message(orion, &midc01);
//        tats_print_midc01(&midc01, file);
    }
}

int cmd_report( char * line, Application * cli, FILE * stream ) {
    double step=0;
    int count=0;
    Orion * orion = cli->orion;

    int result = sscanf(line, "report %lf %u\n", &step, &count );
    if( result != 2 ) {
        alert( "usage: report <step> <count>" );
        return 0;
    }

    // get thread safe copies of the tracker and target from the orion server
    Tracker tracker = orion_get_tracker( orion );
    Entry target = orion_get_target( orion );
    jday start = cli->time;

    // print tracker information
    tracker_print_site( &tracker, stream );
    entry_print( &target ); // TODO print to the supplied stream

    // print the header
    fprintf( stream, "UTC\tAZ\tEL\tE\tF\tG\n" );

    // step over the given time interval
    step /= SECONDS_IN_DAY;
    jday end = start + (step*count);
    while( start < end ) {

        // look up the earth orientation parameters
        IERS_EOP * earth = iers_search( cli->iers, start ); // TODO this can really be sped up to a linear search after the first iteration...
        if (!earth) {
            // TODO should we print a warning if missing or predicted?
            earth = &MISSING_EOP;
        }
        tracker_set_earth( &tracker, earth );

        // calculate coordinates at the given time
        tracker_point( &tracker, start, &(target.novas) );

        // print report entry
        int r = TATS_CELESTIAL_SPHERE_RADIUS;
        char * ts = jday2str(tt2utc(start));
        fprintf( stream, "%s\t%010.6lf\t%010.6lf\t%u\t%u\t%u\n", ts,
                tracker.azimuth, tracker.elevation,
                 (int)(tracker.efg[0]*r), (int)(tracker.efg[1]*r), (int)(tracker.efg[2]*r) );
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
