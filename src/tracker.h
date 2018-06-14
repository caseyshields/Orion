//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_TRACKER_H
#define STARTRACK_TRACKER_H

#include "novas.h"

#define SECONDS_IN_DAY 86400.0
#define DELTA_TT 32.184
#define REDUCED_ACCURACY 1

/** An object representing a tracker on the surface of the earth */
typedef struct {
    double date;       // julian days; the number of days since noon, January 1, 4713 BC // Which Joseph Justice Scaliger estimated to be the beginning of history.
    double ut1_utc;    // current observed discrepancy between earth's non-uniform rotation and Universal Coordinated Time
    double leap_secs;  // Number of leap seconds added in International Atomic Time
    on_surface site;   // geodetic location
    object earth;      // location in the solar system
} Tracker;

/** creates a tracker, allocating space if
 * jd_utc : julian date in days
 * ut1_utc : current difference between UT1 and UTC time
 * leap_secs: current number of leap seconds in TAI*/
int tracker_create(Tracker *tracker, double ut1_utc, double leap_secs);

/** Sets the time for the star tracker
 * seconds: seconds since the unix epoch(January 1, 1970) in UTC */
void tracker_set_time(Tracker *tracker, double utc_unix_seconds); // set UTC

// todo julian hours is pretty obscure, might want to return in unix seconds...
/** @returns terrestrial time in julian hours. TT = UTC + leap_seconds + 32.184. */
double tracker_get_TT(Tracker *tracker);

/** @return The time in UT1, a time scale which depends on the non-uniform rotation of the earth.
 * It is derived by adding an empirically determined offset to UTC */
double tracker_get_UT1(Tracker *tracker);

/** @return The Universal Coordinated Time in Julian hours. */
double tracker_get_UTC(Tracker *tracker);

/**  */
double tracker_get_DeltaT(Tracker *tracker);

/** sets the location of the tracker on planet earth
 * @param tracker object whose coordinates are set
 * @param latitude in degrees
 * @param longitude in degrees
 * @param height in meters */
void tracker_set_location(Tracker *tracker, double latitude, double longitude, double height);
on_surface tracker_get_location(Tracker *tracker);

void tracker_set_weather(Tracker *tracker, double temperature, double pressure);

/** the local horizon coordinates of the target relative to the tracker
 * @param tracker The tracker to compute the direction from
 * @param target A novas catalog entry to point at
 * @param zenith_distance Output argument returning the angular offset from the local zenith in degrees
 * @param topocentric_azimuth Output argument returning the clockwise angular offset from north in degrees
 * @returns Zero on success, otherwise a Novas error code. */
int tracker_to_horizon(Tracker *tracker, cat_entry *target, double *zenith_distance, double *topocentric_azimuth);

/** returns the current location of the given tracker's zenith in celestial coordinates.
 * @param tracker Location used to compute the zenith vector
 * @param right_ascension Celestial spherical coordinates of zenith in hours
 * @param declination Celestial spherical coordinates of zenith in degrees
 * @returns Zero on success, otherwise a Novas error code. */
int tracker_zenith(Tracker *tracker, double *right_ascension, double *declination);

// todo should take a stream
void tracker_print_time(Tracker *tracker);

void tracker_print_site(Tracker *tracker);

#endif //STARTRACK_TRACKER_H
