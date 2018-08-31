/** @file iers.h
 * @brief WIP. Module for loading earth orientation parameters from IERS Bulletin A.
 * */

#ifndef STARTRACK_IERS_H
#define STARTRACK_IERS_H

#include <util/jday.h>

/** IERS Earth Orientation Parameters, either measured or predicted. */
typedef struct {
    /** julian date of parameters */
    jday time;

    /** Flags whether the pole orientation is predicted of part of the IERS, that is 'I' or 'P' respectively */
    char pm_flag;

    /** Bulletin A Polar motion x in seconds of arc. */
    double pm_x, pm_x_err;

    /** Bulletin A Polar motion y in seconds of arc. */
    double pm_y, pm_y_err;

    /** Flags when the UTC offset is predicted or official. */
    char dt_flag;

    /** Bulletin A Polar UTC offset. */
    double ut1_utc, ut1_utc_err;

    // there's a bit more stuff in these files, but I don't need it
    // I had considered making a loader similar to the vizier catalog loader, but it seemed like overkill;
    // this stuff really doesn't change fast.
} IERS_EOP;

// note: observed entries provide actual error, the predictions provide estimated error bounds

/** Holds a summary of the contents of the IERS BUlletin A. */
typedef struct {
    int size;
    IERS_EOP * orientations;
//    double utc_ut1;
//    double leapseconds;
//    // might want to add some more state representing coefficients of predictive functions, time offsets, etc.
} IERS;

/** Stub. Loads a series of earth orientation parameters from the given file.
 * @param bulletinA a file containing IERS Bulletin A data, such as that obtained at http://maia.usno.navy.mil/ser7/ser7.dat
 * @return A pointer to an allocated IERS object, containing the parameters form the file.*/
IERS * iers_load( FILE * bulletinA );

/** Stub. returns the orientation parameters at the specified Novas Julian date.
 * @param iers The structure containing a time sequence of earth orientation parameters
 * @param time date of the earth orientation of interest. */
IERS_EOP * iers_get_orientation( IERS * iers, jday time );

// should we linearly interpolate between data points?
//Orientation * iers_interpolate_orientation( IERS * iers, jday time );

#endif //STARTRACK_IERS_H
