#ifndef STARTRACK_JDAY_H
#define STARTRACK_JDAY_H

#include <sys/time.h>

#include "novasc3.1/novas.h"

#define SECONDS_IN_DAY 86400.0

#define DELTA_TT 32.184

#define TIMESTAMP_OUTPUT "%04u/%02u/%02u %02u:%02u:%06.3lf"

#define TIMESTAMP_INPUT "%u/%u/%u %u:%u:%lf"

/** Julian days; the number of days since noon, January 1, 4713 BC- Which Joseph Justice Scaliger
 * estimated to be the beginning of history. This is a common time format used by Novas, so we
 * provide an explicit typing, and a set of convenience conversion methods built atop novas' own
 * routines.
 * @author Casey Shields*/
typedef double jday;

/** @return False when the given julian day is not valid, true otherwise. */
int jday_is_valid(jday time);

/** @return A julian day holding the current time. */
jday jday_current();

/** @param time A posix structure holding the unix seconds with a fractional part
 * @return A Julian day equivalent to the given timeval. */
jday unix2jday(struct timeval * time);

/** @param time The input julian day
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
 * @return the jday equivalent to the given date. */
jday date2jday(int year, int month, int day, int hour, int minute, double seconds);

/** Converts Julian days to a human readable string
 * @param time A time in julian days
 * @return a string in the format TIMESTAMP_OUTPUT */
char * jday2stamp(jday time);

/** Converts a human readable string into Julian days.
 * @param stamp A string in the format TIMESTAMP_INPUT
 * @return the equivalent Julian day or NAN if the string was invalid*/
jday stamp2jday(char * stamp);

#endif //STARTRACK_JDAY_H
