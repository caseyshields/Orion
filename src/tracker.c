#include <assert.h>
#include <time.h>

#include "h/tracker.h"
#include "novasc3.1/novas.h"

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

void tracker_set_time(Tracker *tracker, double utc_unix_seconds) {

    // separate fractional seconds
    long s = (long) utc_unix_seconds;
    double f = utc_unix_seconds - s;

    // get the calendar date
    struct tm* utc = gmtime( &s );

    // figure out julian hours by adding back in the fractional seconds
    double hours = ((double)utc->tm_hour)
                   + (double) utc->tm_min / 60.0
                   + (utc->tm_sec + f) / 3600.0;

    // convert it to a julian date, which is days since noon, Jan 1, 4713 BC
    tracker->date = julian_date(
            (short) (utc->tm_year + 1900),
            (short) (utc->tm_mon + 1),
            (short) utc->tm_mday,
            hours );
}

double tracker_get_TT(Tracker *tracker) {
    return tracker->date + (tracker->leap_secs + DELTA_TT) / SECONDS_IN_DAY;
}

double tracker_get_UT1(Tracker *tracker) {
    return tracker->date + tracker->ut1_utc / SECONDS_IN_DAY;
}

double tracker_get_UTC(Tracker *tracker) {
    return tracker->date;
}

double tracker_get_DeltaT(Tracker *tracker) {
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
    double deltaT = tracker_get_DeltaT(tracker);
    double right_ascension=0, declination=0;

    // get the GCRS coordinates
    error = topo_star(
                tracker->date,
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
            tracker->date,
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
            tracker->date, 0.0, tracker_get_DeltaT(tracker),
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

void tracker_print_time(Tracker *tracker) {
    short int year, month, day;
    double hour;

    cal_date(tracker_get_UTC(tracker), &year, &month, &day, &hour );
    printf( "UTC : %hi/%hi/%hi %f\n", year, month, day, hour );

    cal_date(tracker_get_UT1(tracker), &year, &month, &day, &hour );
    printf( "UT1 : %hi/%hi/%hi %f\n", year, month, day, hour );

    cal_date(tracker_get_TT(tracker), &year, &month, &day, &hour );
    printf( "TT  : %hi/%hi/%hi %f\n\n", year, month, day, hour );

    fflush(0);
}

void tracker_print_site(Tracker *tracker) {
    printf("latitude:\t%f hours\n", tracker->site.latitude);
    printf("longitude:\t%f degrees\n", tracker->site.longitude);
    printf("elevation:\t%f meters\n", tracker->site.height);
    printf("temperature:\t%f Celsius\n", tracker->site.temperature);
    printf("pressure:\t%f millibars\n\n", tracker->site.pressure);
    //Aperture* aperture = tracker->aperture;
    //printf("aperture: (asc:%f, dec:%f rad:%f)\n", aperture.right_ascension, aperture.declination, aperture.radius);
    fflush(0);
}
