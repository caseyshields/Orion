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

    /** Novas structure holding the geodetic location of the tracker */
    on_surface site;

    /** A structure holding IERS parameters for the current orientation of the earth. Needed for high accuracy Novas calculations */
    IERS_EOP * earth;

    /** Tracker orientation on the local horizon */
    double azimuth, elevation;

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

/** sets the given angles to the tracker's current look direction in topocentric coordinates.
 * @param tracker
 * @param azimuth
 * @param elevation */
void tracker_get_angles(Tracker * tracker, double * azimuth, double * elevation);

/** Returns the tracker's current orientation as a unit vector in the ITRS frame.
 * @param tracker
 * @param vec */
void tracker_get_direction(Tracker * tracker, double vec[3]);

/** Compute the coordinates of the target, and set the tracker to point at them
 * @param tracker The tracker to compute the direction from
 * @param target A novas catalog entry to point at
 * @param jd_tt the terrestrial time expressed in julian days
 * @return Zero on success, otherwise a Novas error code. */
int tracker_point(Tracker *tracker, jday jd_tt, cat_entry *target);

/** returns the current location of the given tracker's zenith in celestial coordinates.
 * @param tracker Location used to compute the zenith vector
 * @param jd_tt Terrestrial time in Julian days.
 * @param right_ascension Celestial spherical coordinates of zenith in hours
 * @param declination Celestial spherical coordinates of zenith in degrees
 * @returns Zero on success, otherwise a Novas error code. */
int tracker_zenith(Tracker *tracker, jday jd_tt, double *right_ascension, double *declination);

void tracker_print_site(Tracker *tracker, FILE * file);

#endif //STARTRACK_TRACKER_H
