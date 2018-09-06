/** @file tracker.h
 * @brief Represents a tracker on the surface of the earth, and can compute the apparent coordinates of stars.
 * @author Casey Shields
 * */

#ifndef STARTRACK_TRACKER_H
#define STARTRACK_TRACKER_H

#include <assert.h>
#include <data/iers.h>

#include "../lib/novasc3.1/novas.h"
#include "util/jday.h"

/** Novas in reduced accuracy mode can be expected to have 1 arcsecond accuracy when properly configured. */
#define REDUCED_ACCURACY 1

/**  Celestial intermediate origin method used to transform between GCRS and ITRS. Argument to the Novas function cel2ter. */
#define METHOD_CIO 0
/**  Equinox based method used to transform between GCRS and ITRS. Argument to the Novas function cel2ter. */
#define METHOD_EQUINOX 1

#define OPTION_GCRS 0
#define OPTION_EQUATOR_AND_EQUINOX_OF_DATE 1

#define REFRACTION_NONE 0
#define REFRACTION_SITE 2

/** The Tracker module represents a sensor on the surface of the earth at a specific time, observing
 * a celestial object. It can transform star coordinates between celestial and horizon, using Novas
 * 3.1 to handle orbit, spin, precession, parallax, atmospheric refraction and relativistic
 * aberration, whatever that is.
 * @author Casey Shields*/
typedef struct {

    /** The current Terrestrial time in Julian Days. This is the format use by Novas. */
    jday jd_tt;

    /** Novas structure holding the geodetic location of the tracker */
    on_surface site;

    /** A structure holding IERS parameters for the current orientation of the earth. Needed for high accuracy Novas calculations */
    IERS_EOP * earth;

    // TODO perhaps tracker should maintain a list of queued target for later, more advanced calculations?
} Tracker;

/** Initializes or allocates the given tracker structure
 * @param tracker A pointer to an existing structure or NULL. If NULL a structure is allocated.
 * @return a pointer to the initialized or allocated structure */
void tracker_create(Tracker *tracker);

/** Terrestrial time is meant to be a smooth timescale and is derived from UTC by removing leap seconds and adding an
 * experimentally measured offset available from the IERS service, which combines data from the Lunar Laser Ranger,
 * Very long baseline radio inferometry of quasars, the GPS constellation, etc.
 * @return The tracker's current Terrestrial Time in Julian day format. */
jday tracker_get_time(Tracker *tracker);

/** Sets the current time for the star tracker.
 * @param tracker
 * @param utc The desired tracker's Terrestrial Time in julian day format */
void tracker_set_time(Tracker *tracker, jday jd_tt);

/** Sets the location of the tracker on earth geoid.
 * @param tracker object whose coordinates are set
 * @param latitude in degrees
 * @param longitude in degrees
 * @param height in meters */
void tracker_set_location(Tracker *tracker, double latitude, double longitude, double height);

/** @return The current geodetic location using the novas convention */
on_surface tracker_get_location(Tracker *tracker);

/** A simple model is used to estimate atmospheric refraction by novas. This sets it's coefficients.
 * @param tracker
 * @param temperature the temperature at the sensor in degrees Celsius.
 * @param pressure the air pressure at the sensor in millibars.*/
void tracker_set_weather(Tracker *tracker, double temperature, double pressure);

/** the local horizon coordinates of the target relative to the tracker
 * @param tracker The tracker to compute the direction from
 * @param target A novas catalog entry to point at
 * @param zenith_distance Output argument returning the angular offset from the local zenith in degrees
 * @param topocentric_azimuth Output argument returning the clockwise angular offset from north in degrees
 * @return Zero on success, otherwise a Novas error code. */
int tracker_to_horizon(Tracker *tracker, cat_entry *target, double *zenith_distance, double *topocentric_azimuth, double *efg);

/** Compute the coordinates of the target in local horizon coordinates and a unit ITRS vector at the specified time
 * // TODO should time be moved out of target? and into a general terrestrial time clock module?
 * @param tracker The tracker to compute the direction from
 * @param target A novas catalog entry to point at
 * @param time The UT1 time in the Novas Julian day convention
 * @param zenith_distance Output argument returning the angular offset from the local zenith in degrees
 * @param topocentric_azimuth Output argument returning the clockwise angular offset from north in degrees
 * @return Zero on success, otherwise a Novas error code. */
int tracker_find(Tracker *tracker, cat_entry *target, jday time, double *zenith_distance, double *topocentric_azimuth, double *efg);

/** returns the current location of the given tracker's zenith in celestial coordinates.
 * @param tracker Location used to compute the zenith vector
 * @param right_ascension Celestial spherical coordinates of zenith in hours
 * @param declination Celestial spherical coordinates of zenith in degrees
 * @returns Zero on success, otherwise a Novas error code. */
int tracker_zenith(Tracker *tracker, double *right_ascension, double *declination);

void tracker_print_time(const Tracker *tracker, FILE * file);

void tracker_print_site(Tracker *tracker, FILE * file);

#endif //STARTRACK_TRACKER_H
