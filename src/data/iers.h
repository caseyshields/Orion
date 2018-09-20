/** @file iers.h
 * @brief Module for loading earth orientation parameters from IERS Bulletin A.
 * */

#ifndef STARTRACK_IERS_H
#define STARTRACK_IERS_H

#include <util/jday.h>

/** Time difference in julian days in which to switch to a linear search. I need to set this experimentally...
 * right now I just guessed the log2 of the number of items in the default dataset. */
#define IERS_LINEAR_THRESHOLD 14.0

/** In the IERS bulletins, Major Julian Date is represented as a constant offset from Julian Day. */
#define IERS_MJD_OFFSET (2400000.5)

/** A summary of the IERS Earth Orientation Parameters. These parameters are produced by combining
 * measurements from Very Long Baseline Interferometry (VLBI), Satellite Laser Ranging (SLR), the
 * Global Positioning System (GPS) satellites, Lunar Laser Ranging (LLR), and meteorological
 * predictions of variations in Atmospheric Angular Momentum (AAM). */
typedef struct {
    /** Major Julian Date of parameters */
    jday mjd;

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
    // Current observed discrepancy between earth's non-uniform rotation and Universal Coordinated Time?

//    double lod, lod_err;
//    double dX, dX_err;
//    double dY, dY_err;

} IERS_EOP;

/** A EOP object to hold default values for these offsets. */
extern IERS_EOP MISSING_EOP;

/** <p>Holds the contents of the IERS BUlletin A, obtained from
 * 'https://datacenter.iers.org/eop/-/somos/5Rgv/getTX/10/finals2000A.data'. Each record of the data is uniformly
 * spaced a day apart. I don't think this ever changes. More versions are available at
 * 'https://www.iers.org/IERS/EN/DataProducts/EarthOrientationData/eop.html'</p>
 * <p>I had considered making a loader similar to the vizier catalog loader, but it seemed like overkill; this stuff
 * really doesn't change fast. There are also the Bulletin B values in these files I don't load. I think they are more
 * accurate, they span less time and predictions are not available.</p> */
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

/** @return The orientation parameters at or immediately after the specified Novas Julian date.
 * @param iers The structure containing a time sequence of earth orientation parameters
 * @param time The UTC JDAY of the earth orientation of interest. */
IERS_EOP * iers_search( IERS * iers, jday time );
// should we linearly interpolate between data points?

/* finds an orientation subsequent to a given recent orientation using linear search */
//IERS_EOP * iers_update(IERS * iers, IERS_EOP recent, jday time);

/** @return The time in UT1, a time scale which depends on the non-uniform rotation of the earth.
 * It is derived by adding an empirically determined offset to UTC. If the Major Julian date of the
 * earth orientation parameter is more then a day from the requested time, an invalid jday is returned. */
jday iers_get_UT1( IERS_EOP * eop, jday utc );

/** @return The time in Universal Coordinated Time. If the Major Julian date of the
 * earth orientation parameter is more then a day from the requested time, an invalid jday is returned. */
jday iers_get_UTC( IERS_EOP * eop, jday ut1 );

/** @returns The time offset in seconds of the given earth orientation. */
double iers_get_DeltaT( IERS_EOP * eop );

void iers_print_eop( IERS_EOP * eop, FILE * file );


/** Terrestrial time is meant to be a smooth timescale and is derived from UT1 by removing leap
 * seconds and adding an experimentally measured offset available from the IERS service
 * @param ut1 The Universal Coordinated Time in Julian Days
 * @returns The Terrestrial Time in julian days, a somewhat obscure Novas convention. TT = UTC + leap_seconds + 32.184. */
jday ut12tt( jday ut1 );

jday tt2ut1( jday tt );

/** releases all contained earth orientation parameter records. */
void iers_free( IERS * iers );

#endif //STARTRACK_IERS_H
