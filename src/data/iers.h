/** @file iers.h
 * @brief WIP. Module for loading earth orientation parameters from IERS Bulletin A.
 * */

#ifndef STARTRACK_IERS_H
#define STARTRACK_IERS_H

#include <util/jday.h>

/** Earth orientation parameters, either measured or predicted. */
typedef struct {
    jday time;
    double px, pxe;
    double py, pye;
    double dt, dte;
} Orientation;
// note: observed entries provide actual error, the predictions provide estimated error bounds

/** Holds a summary of the contents of the IERS BUlletin A. */
typedef struct {
    int size;
    Orientation * orientations;
    double utc_ut1;
    double leapseconds;
    // might want to add some more state representing coefficients of predictive functions, time offsets, etc.
} IERS;

/** Stub. Loads a series of earth orientation parameters from the given file.
 * @param bulletinA a file containing IERS Bulletin A data, such as that obtained at http://maia.usno.navy.mil/ser7/ser7.dat
 * @return A pointer to an allocated IERS object, containing the parameters form the file.*/
IERS * iers_load( FILE * bulletinA );

/** Stub. returns the orientation parameters at the specified Novas Julian date.
 * @param iers The structure containing a time sequence of earth orientation parameters
 * @param time date of teh orientation of interest. */
Orientation * iers_get_orientation( IERS * iers, jday time );




// should we linearly interpolate between data points?
//Orientation * iers_interpolate_orientation( IERS * iers, jday time );

#endif //STARTRACK_IERS_H
