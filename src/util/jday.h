/** @file jday.h
 * @brief provides methods for calculating and converting time into the Novas convention.
 * @author Casey Shields */

#ifndef STARTRACK_JDAY_H
#define STARTRACK_JDAY_H

#include <sys/time.h>

#include "../lib/novasc3.1/novas.h"

#define SECONDS_IN_DAY 86400.0

#define DELTA_TT 32.184

#define TIMESTAMP_OUTPUT "%04u/%02u/%02u %02u:%02u:%06.3lf"

#define TIMESTAMP_INPUT "%u/%u/%u %u:%u:%lf"

/** Julian day of the J2000 epoch */
#define J2000_EPOCH 2400000.5

/** Julian days; the number of days since noon, January 1, 4713 BC- Which Joseph Justice Scaliger
 * estimated to be the beginning of history. This is a common time format used by Novas, so we
 * provide an explicit typing, and a set of convenience conversion methods built atop novas' own
 * routines.
 * @author Casey Shields*/
typedef double jday;

/** Number of leap seconds added in International Atomic Time. Using this parameter we can convert to 'continuous' time scales. */
extern double LEAP_SECONDS;

/** @return False when the given julian day is not valid, true otherwise. */
int jday_is_valid(jday time);

/** @return A julian day holding the current time in UTC. */
jday jday_now();

/** @param utc The Universal Coordinated Time in Julian Days
 * @returns The Terrestrial Time in julian days, a somewhat obscure Novas convention. TT = UTC + leap_seconds + 32.184. */
jday utc2tt( jday utc );

jday tt2utc( jday tt );

/** @param time A posix structure holding the unix seconds with a fractional part
 * @return A Julian day equivalent to the given timeval. */
jday unix2jday(struct timeval * time);

/** @param time The input julian day in UTC
 * @param year output The AD year
 * @param month output [1-12]
 * @param day output [1-31]
 * @param hour output [0-23]
 * @param minute output [0-59]
 * @param seconds output [0.0-61.0] */
void jday2date(jday time,
                   short int *year, short int *month, short int *day,
                   short int *hour, short int *minute, double *seconds);

/**@param year The AD year
 * @param month input [1-12]
 * @param day input [1-31]
 * @param hour input [0-23]
 * @param minute input [0-59]
 * @param seconds input [0.0-61.0]
 * @return the jday equivalent to the given date in UTC. */
jday date2jday(int year, int month, int day, int hour, int minute, double seconds);

/** Converts Julian days to a human readable string
 * @param utc A UTC time in julian days
 * @return a string in the format TIMESTAMP_OUTPUT */
char * jday2str(jday utc);

/** Converts a human readable string into Julian days.
 * @param stamp A string in the format TIMESTAMP_INPUT
 * @return the equivalent UTC Julian day or NAN if the string was invalid*/
jday str2jday(char *stamp);

#endif //STARTRACK_JDAY_H
