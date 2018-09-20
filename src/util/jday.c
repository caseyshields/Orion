#include "util/jday.h"

double LEAP_SECONDS = 37.0;

int jday_is_valid(jday time) {
    return time != NAN;
}

jday jday_now() {
    // get the seconds since the unix epoch(January 1, 1970) in Universal Coordinated Time
    struct timeval time;
    gettimeofday( &time, NULL );
    return unix2jday( &time );
}

jday unix2jday( struct timeval * time) {
    // Novas's conversion routines take dates, hence this detour...
    struct tm* utc = gmtime( &(time->tv_sec) );

    // set the date, correcting the idiosyncrasies of the tm struct, and adding back in the fractional time
    return date2jday(
                  utc->tm_year + 1900,
                  utc->tm_mon + 1,
                  utc->tm_mday,
                  utc->tm_hour,
                  utc->tm_min,
                  (double) utc->tm_sec + ( (double)time->tv_usec / 1000000.0 )
    );
}

void jday2date(jday jd_utc,
                   short int *year, short int *month, short int *day,
                   short int *hour, short int *minute, double *seconds)
{
    double h, m;
    cal_date( jd_utc, year, month, day, seconds);
    *seconds = modf(*seconds, &h) * 60;
    *seconds = modf(*seconds, &m) * 60;
    *hour = (short int)h;
    *minute = (short int)m;
} // TODO debug this!!!

jday date2jday(int year, int month, int day, int hour, int min, double seconds) {
    // compute the fractional hours novas requires
    double hours = (double)hour + (double)min / 60.0 + (seconds / 3600.0);

    // convert it to a julian date, which is days since noon, Jan 1, 4713 BC
    return julian_date(
            (short) year,
            (short) month,
            (short) day,
            hours );
}

char * jday2str(jday jd_utc) {
    short int year, month, day, hour, minute;
    double seconds;
    jday2date(jd_utc, &year, &month, &day, &hour, &minute, &seconds);
    char * stamp = calloc( 24, sizeof(char) );
    sprintf(stamp, TIMESTAMP_OUTPUT, year, month, day, hour, minute, seconds );
    return stamp;
}

jday str2jday(char *stamp) {
    int result, year, month, day, hour, min;
    double seconds;

    // scan the stamp from the buffer
    result = sscanf(stamp, TIMESTAMP_INPUT,
                    &year, &month, &day, &hour, &min, &seconds );

    // abort if the formatting was wrong, returning a negative number
    if (result < 6)
        return NAN;
    // todo also check for bad inputs!!!

    // set the time
    return date2jday(year, month, day, hour, min, seconds);
}