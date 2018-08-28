#include "engine/tracker.h"

int tracker_create(Tracker *tracker, double ut1_utc, double leap_secs) {

    tracker->ut1_utc = ut1_utc;
    tracker->leap_secs = leap_secs;
    // Novas typically deals with the sum of the offsets, might want to cache it...
    //map->delta_t = 32.184 + map->leap_secs - map->ut1_utc;

    // create null novas surface location
    make_on_surface( 0.0, 0.0, 0.0, 10.0, 1010.0, &(tracker->site) );

    // create an earth to put the tracker on
    return make_object (0, 2, "Earth", (cat_entry*)NULL, &(tracker->earth) );
}

jday tracker_get_time(Tracker *tracker) {
    return tracker->utc;
}

void tracker_set_time( Tracker * tracker, jday utc ) {
    tracker->utc = utc;
}

/**@param bias A bias to be added to the raw time in seconds
 * @returns terrestrial time in julian days, a somewhat obscure Novas convention. TT = UTC + leap_seconds + 32.184. */
jday tracker_get_terrestrial_time( const Tracker *tracker, double bias ) {
    return tracker->utc + (tracker->leap_secs + DELTA_TT + bias) / SECONDS_IN_DAY;
}

/** @return The time in UT1, a time scale which depends on the non-uniform rotation of the earth.
 * It is derived by adding an empirically determined offset to UTC */
jday tracker_get_UT1(const Tracker *tracker) {
    return tracker->utc + tracker->ut1_utc / SECONDS_IN_DAY;
}

/** @return The Universal Coordinated Time in Julian hours. */
jday tracker_get_UTC(const Tracker *tracker) {
    return tracker->utc;
}

/** @returns The time offset in seconds. */
double tracker_get_DeltaT(const Tracker *tracker) {
    return DELTA_TT + tracker->leap_secs - tracker->ut1_utc;
}

void tracker_set_location(Tracker *tracker, double latitude, double longitude, double height) {
    tracker->site.latitude = latitude;
    tracker->site.longitude = longitude;
    tracker->site.height = height;
}
void tracker_set_weather(Tracker *tracker, double temperature, double pressure) {
    tracker->site.pressure = pressure;
    tracker->site.temperature = temperature;
}

on_surface tracker_get_location(Tracker *tracker) {
    return tracker->site;
}

int tracker_to_horizon(
        Tracker *tracker,
        cat_entry *target,
        double *topocentric_azimuth, double *zenith_distance,
        double *efg )
{
    return tracker_find( tracker, target, tracker_get_TT(tracker),
            topocentric_azimuth, zenith_distance, efg );
}

int tracker_find(
        Tracker *tracker,
        cat_entry *target,
        jday time,
        double *topocentric_azimuth, double *zenith_distance,
        double *efg )
{
    // Derive the Terestrial time from the current UTC time
    double jd_tt = time;//tracker_get_TT( tracker );
    double deltaT = tracker_get_DeltaT( tracker );

    // TODO we should configure these or scrub them from the IERS bullitin A...
    double xp = 0.0, yp = 0.0;

    // Apply proper motion, parallax, gravitational deflection, relativistic
    // aberration and get the coordinates of the star in the true equator and equinox of date
    double right_ascension=0, declination=0;
    short int error;
    error = topo_star(
                jd_tt,
                deltaT,
                target, // The FK6 catalog entries I believe are specified in the J2000 epoch
                        // thus they should be consistent to within 0.02 arcseconds of the BCRS
                &tracker->site, // The input location is supplied in WGS-84, which is within centimeters of the ITRS axis
                REDUCED_ACCURACY,
                &right_ascension, // these are specified in term of the current orientations of the equator and ecliptic
                &declination // that is, they don't make sense unless you have a date, because both of those things are moving...
        );
    if( error )
        return error;

    // Apply refraction and convert Equatorial coordinates to horizon coordinates
    double ra, dec;
    equ2hor(
            jd_tt,
            deltaT,
            REDUCED_ACCURACY,
            xp, yp,
            &tracker->site,
            right_ascension, declination,
            REFRACTION_SITE, // simple refraction model based on site atmospheric conditions
            zenith_distance, topocentric_azimuth,
            &ra, &dec // TODO do I need to expose these?
    );

    // convert the spherical coordinates to rectilinear
    double equ[3];
    // what should the default distance be?
    double tats_distance = 2<<16; // square root of maximum TATS resolution?
    // = AU; // astronomical unit used by novas
    // = AU*2*10^14; // where novas places the celestial sphere
    // = (2^32)/256 = 16777216 meters // max distance representable by TATS
    radec2vector( right_ascension, declination, tats_distance, equ );

    // now transform the equatorial coordinates into ITRS coordinates
    error = cel2ter(
            jd_tt, 0, // previous novas routines don't support the level of precision available
            deltaT,
            METHOD_EQUINOX,
            REDUCED_ACCURACY,
            OPTION_EQUATOR_AND_EQUINOX_OF_DATE, // only compatible with the equinox method
            xp, yp,
            equ,
            efg
    );
    // the answer will have to be scaled to fit TATS eventually...

    return error;
}

int tracker_zenith(Tracker *tracker, double *right_ascension, double *declination) {
    on_surface site = tracker->site;

    // calculate a geocentric earth fixed vector as in Novas C-39
    double lon_rad = site.longitude * DEG2RAD;
    double lat_rad = site.latitude * DEG2RAD;
    double sin_lon = sin(lon_rad);
    double cos_lon = cos(lon_rad);
    double sin_lat = sin(lat_rad);
    double cos_lat = cos(lat_rad);
    double terestrial[3] = {0.0,0.0,0.0};
    terestrial[0] = cos_lat * cos_lon;
    terestrial[1] = cos_lat * sin_lon;
    terestrial[2] = sin_lat;
    //  this is a spherical approximation so it can have up to about 1% error...

    // TODO scrub or configure pole movement
    double px=0.0, py=0.0;

    // convert to a celestial vector
    double celestial[3] = {0.0,0.0,0.0};
    int error = ter2cel(
            tracker->utc, 0.0, tracker_get_DeltaT(tracker),
            METHOD_EQUINOX, // equinox method (0= CIO method)
            REDUCED_ACCURACY,
            OPTION_EQUATOR_AND_EQUINOX_OF_DATE,
            px, py,
            terestrial,
            celestial
    );
    assert(error==0);

    // convert to spherical celestial components
    error = vector2radec( celestial, right_ascension, declination );
    assert(error==0);
}

void tracker_print_time( const Tracker *tracker, FILE * file ) {
    // format the time
    char * stamp = jday2stamp( tracker->utc );
    fprintf( file, "time:\t%s UTC\t(%+05.3lf UT1)\n", stamp, tracker->ut1_utc );

    // do we want to expose any other time conventions?
//    double utc = tracker_get_UTC(tracker);
//    double ut1 = tracker_get_UT1(tracker);
//    double tt = tracker_get_TT(tracker);
//    fprintf( file, "UTC : %s\nUT1 : %s\nTT  : %s\n",
//             jday2stamp(utc),
//             jday2stamp(ut1),
//             jday2stamp(tt)
//    );

    fflush( file );
}

void tracker_print_site(const Tracker *tracker, FILE * file) {

    // print out location details
    on_surface * site = &(tracker->site);
    char ns = (char) ((site->latitude>0.0) ? 'N' : 'S');
    char ew = (char) ((site->longitude>0.0) ? 'E' : 'W');
    fprintf( file, "location:\t%10.6lf %c, %10.6lf %c, %6.1lf m\n",
             site->latitude, ns, site->longitude, ew, site->height );

    // print atmospheric details
    fprintf( file, "weather:\t%5.1lf°C, %4.3f bar\n", site->temperature, site->pressure/1000.0 );

    fflush(file);
}
