/** @file tracker.h
 * @brief Represents a tracker on the surface of the earth, and can compute the apparent coordinates of stars.
 * @author Casey Shields
 * */

#ifndef STARTRACK_TRACKER_H
#define STARTRACK_TRACKER_H

#include <assert.h>
#include <util/vmath.h>

#include <data/iers.h>
#include "util/jday.h"

#include "../lib/novasc3.1/novas.h"
#include "../lib/cutest-1.5/CuTest.h"

/** Novas in reduced accuracy mode can be expected to have 1 arcsecond accuracy when properly configured. */
#define REDUCED_ACCURACY 1

/**  Celestial intermediate origin method used to transform between GCRS and ITRS. Argument to the Novas function cel2ter. */
#define METHOD_CIO 0
/**  Equinox based method used to transform between GCRS and ITRS. Argument to the Novas function cel2ter. */
#define METHOD_EQUINOX 1

#define OPTION_GCRS 0
#define OPTION_EQUATOR_AND_EQUINOX_OF_DATE 1

/** Disables refraction calculation when computing topocentric coordinates */
#define REFRACTION_NONE 0

/** Signals use of a simple exponential model included with NOVAS 3.1 */
#define REFRACTION_SITE 2

/** The Tracker module represents a sensor on the surface of the earth at a specific time, observing
 * a celestial object. It can transform star coordinates between celestial and horizon, using Novas
 * 3.1 to handle orbit, spin, precession, parallax, atmospheric refraction and relativistic
 * aberration, whatever that is.
 * @author Casey Shields*/
typedef struct {

    /** Novas structure holding the geodetic location of the tracker */
    on_surface site;

    /** A structure holding IERS parameters for the current orientation of the earth. Needed for high accuracy Novas calculations */
    IERS_EOP * earth;

    /** Tracker orientation on the topocentric local horizon */
    double azimuth, elevation;

    /** Tracker orientation of the celestial sphere */
    double right_ascension, declination;

    /** Tracker orientation in the ITRS frame */
    double efg[3];

    // TODO perhaps tracker should maintain a list of queued target for later, more advanced calculations?
} Tracker;

/** Initializes or allocates the given tracker structure
 * @param tracker A pointer to an existing structure or NULL. If NULL a structure is allocated.
 * @return a pointer to the initialized or allocated structure */
void tracker_create(Tracker *tracker);

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

/** @return The current earth orientation parameters
 * @param tracker */
IERS_EOP * tracker_get_earth(Tracker * tracker);

/** Sets the earth orientation parameters used for some high accuracy calculations
 * @param tracker
 * @param eop */
void tracker_set_earth(Tracker * tracker, IERS_EOP * eop);
// not really intrinsic to the tracker, I could see passing them along with the time

/** sets the given angles to the tracker's current look direction in horizon topocentric spherical coordinates.
 * @param tracker
 * @param azimuth
 * @param elevation */
void tracker_get_topocentric(Tracker * tracker, double * azimuth, double * elevation);

/** sets the given angles to the tracker's current look direction in ICRS celestial spherical coordinates.
 * @param tracker
 * @param right_ascension
 * @param declination */
void tracker_get_celestial(Tracker * tracker, double * right_ascension, double * declination);

/** Returns the tracker's current orientation as a unit vector in the ITRS frame.
 * @param tracker
 * @param vec */
void tracker_get_direction(Tracker * tracker, double vec[3]);

/** Compute the coordinates of the target, and set the tracker to point at them
 * @param tracker The tracker to compute the direction from
 * @param target A novas catalog entry to point at
 * @param jd_tt the terrestrial time expressed in julian days
 * @param refraction one of REFRACTION_SITE or REFRACTION_NONE
 * @return Zero on success, otherwise a Novas error code. */
int tracker_point(Tracker *tracker, jday jd_tt, cat_entry *target, short refraction);

/** returns the current location of the given tracker's zenith in celestial coordinates.
 * @param tracker Location used to compute the zenith vector
 * @param jd_tt Terrestrial time in Julian days.
 * @param right_ascension Celestial spherical coordinates of zenith in hours
 * @param declination Celestial spherical coordinates of zenith in degrees
 * @returns Zero on success, otherwise a Novas error code. */
int tracker_zenith(Tracker *tracker, jday jd_tt, double *right_ascension, double *declination);

void tracker_print_atmosphere(Tracker *tracker, FILE * file);

void tracker_print_location(Tracker *tracker, FILE * file);

/** Performs some calculations on a small set of stars and tests them against precomputed coordinates.
 * Directly taken from 'checkout-stars.c' from novas 3.1.
 * @param test the CuTest structure which holds test results. */
void test_novas( CuTest * test );

#endif //STARTRACK_TRACKER_H
