#include <sys/time.h>
#include <winsock.h>
#include "tracker.h"
#include "orion.h"

// default latitude of sensor in degrees
#define LATITUDE "38.88972222222222"

// default longitude of sensor in degrees
#define LONGITUDE "-77.0075"

// default geodetic height of sensor in meters
#define HEIGHT "125.0"

// default site temperature in degrees celsius
#define TEMPERATURE "10.0"

// default atmospheric pressure at site in millibars
#define PRESSURE "1010.0"

// (UT1-UTC); current offset between atomic clock time and time derived from Earth's orientation
#define UT1_UTC "0.108644"

// delta AT, Difference between TAI and UTC. Obtained from IERS Apr 26 2018
#define TAI_UTC "37.000000"

// some winsock constant I use
#ifndef WIN32
#define INVALID_SOCKET (unsigned int)(~0)
#define SOCKET_ERROR -1
#endif

double get_time();
char* get_arg( int argc, char *argv[], char *name, char* default_value );
ssize_t get_input(char* prompt, char **line, size_t *size );
void configure_tracker( int argc, char* argv[], Tracker* tracker );
//void configure_address(int argc, char* argv[], struct sockaddr_in* address);
void configure_catalog( int argc, char* argv[], Catalog* catalog );
int search( Catalog * catalog, Tracker * tracker,
            double az_min, double az_max, double zd_min, double zd_max, float mag_min);

/** Provides an interactive command line interface to the Orion server. */
int main( int argc, char *argv[] ) {
    char *line = NULL;
    size_t size = 0 ;
    ssize_t read = 0;
    int result;

    // create the Orion server structure
    Orion orion;
    orion_create( &orion );

    // create and load a catalog of stars
    char *path = get_arg(argc, argv, "-catalog", "../data/FK6.txt");
    Catalog catalog = {.size=0, .allocated=0, .stars = NULL};
    catalog_create( &catalog, 1024 );
    FILE *file = fopen(path, "r");
    catalog_load_fk5(&catalog, file);
    if (!catalog.size)
        goto CLEANUP;

    // create main components according to program arguments
    configure_tracker( argc, argv, &(orion.tracker) );
    // TODO eventually move this configuration code to interactive commands and make a script instead...

    // get server address, should be in dotted quad notation
    char *ip = get_arg(argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port; // port
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

        read = get_input( "", &line, &size );

        // mark time
        // set time
        // set orientation ( ut1-utc, leapsecs, x, y )
        // set location ( lat, longitude, height )
        // set weather ( temperature, pressure )
        // set sensor ( ip, port )

        // load catalog ( path ) //////////////////////////////////////////////
//        if( strncmp( "load ", line, 5)==0) {
//            char path[256];
//            result = sscanf( line, "load %256s\n", path );
//            if(result==0) {
//                printf("usage: load <catalog path>");
//                continue;
//            }
//            if( catalog.size>0 )
//                catalog_free_entries( &catalog );
//            FILE *file = fopen( path, "r" );
//            catalog_load_fk5( &catalog, file );
//        }

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

        // start the sensor control thread
        if (strncmp( "start", line, 5 ) == 0) {
            if (orion_start( &orion ))
                printf("Sensor control thread already started.\n");
        }

//        // select a target
//        if (strncmp("select", line, 6) == 0) {
//            int id = 0;
//            result = sscanf( line, "select %d\n", &id );
//            if( orion_track( id, target ) )
//                printf( "select failed");
//        }

        // stop the sensor control thread and exit
        if( strncmp("exit", line, 4)==0 ) {
            orion_stop( &orion );
            break;
        }

        // get help with the commands
        if( strncmp( "help", line, 4 )==0 ) {
            char command[11] = {0,0,0,0,0,0,0,0,0,0,0};
            result = sscanf(line, "help%10s\n", command);
            if (strlen(command)==0) {//result == 0) {
                printf("Commands:\nname <substr>\nsearch <> <> <> <> <>\ntrack <id>\nexit\n");
            } else {
                // todo print specific help
            }
        }
    }

    DISCONNECT:
    orion_disconnect( &orion );

    CLEANUP:
    catalog_free_entries( &catalog );
    catalog_free( &catalog );

    EXIT:
    if (orion.error) {
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
        float mag_min) {
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
                            &(entry->topocentri_azimiuth) );
    }
    catalog_each( bright_stars, &to_horizon );

    // filter the stars by their horizon coordinates
    int in_patch(Entry *entry) {
        return entry->zenith_distance > zd_min
               && entry->zenith_distance < zd_max
               && entry->topocentri_azimiuth > az_min
               && entry->topocentri_azimiuth < az_max;
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
                entry->topocentri_azimiuth, entry->zenith_distance);
    }
    catalog_each( results, print_star );

    // release catalogs
    catalog_free_entries( bright_stars );
    free( bright_stars );
    catalog_free_entries( results );
    free( results );

    return 0;
}

/** Gets an accurate UTC timestamp from the system in seconds since the unix epoch */
double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

/** retrieves the value subsequent to the specified option. If the default_value
 * is supplied, the function will return it. otherwise the method will print an
 * error message and abort. */
char* get_arg( int argc, char *argv[], char *name, char* default_value ) {
    for( int n=0; n<argc; n++ )
        if( !strcmp(name, argv[n]) )
            return argv[n+1];
    if( default_value ) {
        printf("Parameter '%s' defaulting to '%s'\n", name, default_value);
        fflush(stdout);
        return default_value;
    }
    sprintf( "Error: missing parameter '%s'\n", name );
    exit( 1 );
}

/** Frees any data line is pointing to, then prompts the user, allocates a buffer, and reads the input.
 * Be sure to free the buffer after your last call to get_input! */
ssize_t get_input(char* prompt, char **line, size_t *size ) {
    if( *line ) {
        free( *line );
        *line = NULL;
        *size = 0;
    }
    printf("%s : ", prompt);
    fflush( stdout );
    ssize_t read = getline( line, size, stdin );

    if( read == -1 ) {
        printf("Error: input stream closed");
        exit( 2 );
    }
    (*line)[read] = '\0'; // trim trailing
    return read;
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

//void configure_catalog( int argc, char* argv[], Catalog* catalog ) {
//    // get the location of the catalog data
//    char *path = get_arg(argc, argv, "-catalog", "../data/FK6.txt");
//
//    // create and load a catalog
//    FILE *file = fopen(path, "r");
//    catalog_load_fk5(catalog, file);
//}