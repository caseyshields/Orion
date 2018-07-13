#include <assert.h>

#include "h/tracker.h"

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

/** @returns terrestrial time in julian days, a somewhat obscure Novas convention. TT = UTC + leap_seconds + 32.184. */
jday tracker_get_TT(const Tracker *tracker) {
    return tracker->utc + (tracker->leap_secs + DELTA_TT) / SECONDS_IN_DAY;
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

int tracker_to_horizon(Tracker *tracker, cat_entry *target, double *zenith_distance, double *topocentric_azimuth) {
    short int error;
    double jd_tt = tracker_get_TT( tracker );
    double deltaT = tracker_get_DeltaT( tracker );
    double right_ascension=0, declination=0;

    // get the GCRS coordinates
    error = topo_star(
                jd_tt,//tracker->date,
                deltaT,
                target,
                &tracker->site,
                REDUCED_ACCURACY,
                &right_ascension,
                &declination
        );

    // then convert them to horizon coordinates
    double ra, dec;
    equ2hor(
            jd_tt,//tracker->date,
            deltaT,
            REDUCED_ACCURACY,
            0.0, 0.0, // TODO ITRS poles... scrub from bulletin!
            &tracker->site,
            right_ascension, declination,
            2, // simple refraction model based on site atmospheric conditions

            zenith_distance, topocentric_azimuth,
            &ra, &dec // TODO do I need to expose these?
    );

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

    // convert to a celestial vector
    double celestial[3] = {0.0,0.0,0.0};
    int error = ter2cel(
            tracker->utc, 0.0, tracker_get_DeltaT(tracker),
            1, // equinox method (0= CIO method)
            REDUCED_ACCURACY,
            0, // output in GCRS axes (1=equator & equinox of date)
            0.0, 0.0, // TODO add in pole offsets!
            terestrial,
            celestial
    );
    assert(error==0);

    // convert to spherical celestial components
    error = vector2radec( celestial, right_ascension, declination );
    assert(error==0);
}

void tracker_print_time( const Tracker *tracker, FILE * file ) {
    double utc = tracker_get_UTC(tracker);
    double ut1 = tracker_get_UT1(tracker);
    double tt = tracker_get_TT(tracker);
    fprintf( file, "UTC : %s\nUT1 : %s\nTT  : %s\n",
             jday2stamp(utc),
             jday2stamp(ut1),
             jday2stamp(tt)
    );
    fflush( file );
}

void tracker_print_site(const Tracker *tracker, FILE * file) {
    fprintf(file, "latitude:\t%f hours\n", tracker->site.latitude);
    fprintf(file, "longitude:\t%f degrees\n", tracker->site.longitude);
    fprintf(file, "elevation:\t%f meters\n", tracker->site.height);
    fprintf(file, "temperature:\t%f Celsius\n", tracker->site.temperature);
    fprintf(file, "pressure:\t%f millibars\n\n", tracker->site.pressure);
    //Aperture* aperture = tracker->aperture;
    //printf("aperture: (asc:%f, dec:%f rad:%f)\n", aperture.right_ascension, aperture.declination, aperture.radius);
    fflush(file);
}

//void tracker_set_time(Tracker *tracker, double utc_unix_seconds) {
//
//    // separate fractional seconds
//    long s = (long) utc_unix_seconds;
//    double f = utc_unix_seconds - s;
//
//    // Novas uses a different epoch and scale which I don't want to recreate
//    // and it's conversion routines take dates, hence this detour
//    struct tm* utc = gmtime( &s );
//
//    // set the date, correcting the idiosyncrasies of the tm struct, and adding back in the fractional time
//    tracker_set_date( tracker,
//                      utc->tm_year + 1900,
//                      utc->tm_mon + 1,
//                      utc->tm_mday,
//                      utc->tm_hour,
//                      utc->tm_min,
//                      (double)utc->tm_sec + f
//    );
//}
//
//void tracker_get_date(const Tracker * tracker,
//                      short int * year, short int * month, short int * day,
//                      short int * hour, short int * minute, double * seconds )
//{
//    double h, m;
//    cal_date( tracker->utc, year, month, day, seconds);
//    *seconds = modf(*seconds, &h) * 60;
//    *seconds = modf(*seconds, &m) * 60;
//    *hour = (short int)h;
//    *minute = (short int)m;
//}
//
//void tracker_set_date( Tracker * tracker, int year, int month, int day, int hour, int min, double seconds ) {
//    // compute the fractional hours novas requires
//    double hours = (double)hour + (double)min / 60.0 + (seconds / 3600.0);
//
//    // convert it to a julian date, which is days since noon, Jan 1, 4713 BC
//    tracker->utc = julian_date(
//            (short) year,
//            (short) month,
//            (short) day,
//            hours );
//}
//
//char * tracker_get_stamp( const Tracker * tracker ) {
//    short int year, month, day, hour, minute;
//    double seconds;
//    tracker_get_date( tracker, &year, &month, &day, &hour, &minute, &seconds );
//    char * stamp = calloc( 24, sizeof(char) );
//    sprintf(stamp, TIMESTAMP_OUTPUT, year, month, day, hour, minute, seconds );
//    return stamp;
//}
//
//int tracker_set_stamp( Tracker * tracker, char * stamp ) {
//    int result, year, month, day, hour, min;
//    double seconds;
//
//    // scan the stamp from the buffer
//    result = sscanf(stamp, TIMESTAMP_INPUT,
//                    &year, &month, &day, &hour, &min, &seconds );
//
//    // abort if the formatting was wrong, returning a negative number
//    if (result < 6)
//        return result;
//    // todo also check for bad inputs!!!
//
//    // set the time
//    tracker_set_date( tracker, year, month, day, hour, min, seconds );
//
//    return 0;
//}