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

    /** Bulletin A UTC offset. */
    double ut1_utc, ut1_utc_err;

//    double lod, lod_err;
//    double dX, dX_err;
//    double dY, dY_err;

} IERS_EOP;

// note: observed entries provide actual error, the predictions provide estimated error bounds

/** <p>Holds a summary of the contents of the IERS BUlletin A. There are also the Bulletin B values in these files, but
 * I don't think I need it. While I think they are more accurate, they span less time.</p>
 *
 * <p>I had considered making a loader similar to the vizier catalog loader, but it seemed like overkill; this stuff
 * really doesn't change fast.</p> */
typedef struct {
    size_t size;
    IERS_EOP * eops;
} IERS;

/** Optionally allocates, then initializes an IERS structure with no orientations.
 * @param iers location of an existing structure, or null if the structure should be allocated
 * @return a pointer to the initialized structure, or null if memory could not be allocated*/
IERS * iers_create( IERS * iers );

/** Append a copy of the given orientation into the IERS records.
 * @return false if the addition succeded with no errors, true otherwise
 * @param iers A pointer to the IERS structure we wish to add the parameters to
 * @param eop A pointer to a structure containing Earth Orientation Parameters */
int iers_add(IERS * iers, IERS_EOP * eop);

/** Loads a series of earth orientation parameters from the given file.
 * @return the number of records read from the file
 * @param iers A pointer to an IERS structure, which the contents of the bulletin will be added to
 * @param bulletinA a file containing IERS Bulletin A data, such as that obtained at http://maia.usno.navy.mil/ser7/ser7.dat */
int iers_load( IERS * iers, FILE * bulletinA );

/** @return The orientation parameters at the specified Novas Julian date.
 * @param iers The structure containing a time sequence of earth orientation parameters
 * @param time date of the earth orientation of interest. */
IERS_EOP * iers_search( IERS * iers, jday time );

// should we linearly interpolate between data points?
//Orientation * iers_interpolate_orientation( IERS * iers, jday time );

void iers_print_eop( IERS_EOP * eop, FILE * file );

/** releases all contained earth orientation parameter records. */
void iers_free( IERS * iers );

#endif //STARTRACK_IERS_H
