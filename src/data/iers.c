#include "iers.h"

IERS_EOP MISSING_EOP = {0.0,' ',0.0,0.0,0.0,0.0,' ',0.0,0.0};

size_t inline iers_get_index(IERS * iers, IERS_EOP * eop) {
    return (iers->eops - eop) / sizeof(IERS_EOP);
}

IERS * iers_create( IERS * iers ) {
    if( !iers )
        iers = malloc(sizeof(IERS));
    if( iers ) {
        iers->size = 0;
        iers->eops = calloc( 32, sizeof(IERS_EOP) );
        if( !iers->eops ) {
            free(iers);
            iers = NULL;
        }
    }
    return iers;
}

// internal method for adding an EOP and returning a pointer to it, rather than copying a structure
IERS_EOP * iers_new_eop( IERS * iers ) {

    // if size has reached a power of 2 we double the size of the array
    size_t n = iers->size;
    if( n>16 && !(n&(n-1)) ) {
        IERS_EOP * eops = realloc(
                iers->eops, 2 * iers->size * sizeof(IERS_EOP));
        if( !eops )
            return NULL;
        iers->eops = eops;
    }

    // increase the size and return a pointer to the trailing entry.
    return &( iers->eops[ iers->size++ ] );
}

int iers_add( IERS * iers, IERS_EOP * eop ) {

    // test that parameters are being added in chronological order
    if( iers->size>0 && eop->mjd < iers->eops[iers->size-1].mjd )
        return -2;

    // obtain a new entry
    IERS_EOP * new = iers_new_eop( iers );
    if( !new )
        return -1;

    // copy the earth orientation parameters
    memcpy( new, eop, sizeof(IERS_EOP) );
    return 0;
}

int iers_load( IERS *iers, FILE * finals2000A ) {
    size_t size = 0, count = 0;
    char * line = NULL;
    char buf[16]; // stack variables should be null filled...
    const int IERS_RECORD_LENGTH = 188;

    // for every line of the file
    while (-1 != getline( &line, &size, finals2000A )) {

        // skip lines that are too short to be an IERS Bulletin A record, or are missing needed parameters
        if (strlen( line ) < IERS_RECORD_LENGTH || line[16]==' ' || line[57]==' ')
            continue;

        // allocate a new earth orientation parameter object
        IERS_EOP * eop = iers_new_eop( iers );
        if( !eop )
            break; // we should probably shut down instead...

        // clear buffer, note that copies happen in order of increasing length,
        // so this is only done once a loop
        memset( buf, 0, 16 );

        // compute the modified julian date
        strncpy(buf, line+7, 8);
        eop->mjd = atof(buf) + IERS_MJD_OFFSET;

        // get the prediction flag for polar offsets
        strncpy( &(eop->pm_flag), line+16, 1 );

        // obtain polar offsets
        strncpy( buf, line + 18, 9 );
        eop->pm_x = atof( buf );

        strncpy( buf, line + 27, 9 );
        eop->pm_x_err = atof( buf );

        strncpy( buf, line + 37, 9 );
        eop->pm_y = atof( buf );

        strncpy( buf, line + 46, 9 );
        eop->pm_y_err = atof( buf );

        // obtain UTC offset
        strncpy( &(eop->dt_flag), line + 57, 1 );

        strncpy( buf, line + 58, 10 );
        eop->ut1_utc = atof( buf );

        strncpy( buf, line + 68, 10 );
        eop->ut1_utc_err = atof( buf );

        // this structure is described in 'readme.finals2000A.txt'
        // There are more values if we need them;
        //  Length of Day, nutation(polar velocities), Bulletin B values...

        count++;
    }
    free(line);
    return count;
}

IERS_EOP * iers_search( IERS * iers, jday time ) {

    // search for upper bound with binary search
    size_t low = 0, size = iers->size;
    while (size>0) {
        size_t half = size / 2;
        size_t probe = low + half;
        size_t other = low + size - half;
        size = half;
        low = (time <= iers->eops[probe].mjd) ? low : other;
    } // an optimized search with fewer comparisons, the latter of which can be compiled to a cmovaeq instruction
    // adapted from https://academy.realm.io/posts/how-we-beat-cpp-stl-binary-search/

    // check bounds, we only want to return values in a measured interval
    if (low==iers->size) // low ==-1)
        return NULL;
    else if (low==0 && time < iers->eops[low].mjd)
        return NULL;

    // TODO should we interpolate orientation between the two adjacent dates?

    // right now we just return the first subsequent parameters
    return &(iers->eops[low]);
} // TODO ugh, duh, since the bulletin records are perfectly spaced we should use an interpolation search in basically O(1)

jday iers_get_UT1( IERS_EOP * eop, jday utc ) {
    if( fabs(eop->mjd - utc) > 1)
        return (jday)NAN;
    return utc + eop->ut1_utc / SECONDS_IN_DAY;
}

jday iers_get_UTC( IERS_EOP * eop, jday ut1 ) {
    if( fabs(eop->mjd - ut1) > 1)
        return (jday)NAN;
    return ut1 - eop->ut1_utc / SECONDS_IN_DAY;
}

double iers_get_DeltaT( IERS_EOP * eop ) {
    return DELTA_AT + LEAP_SECONDS - eop->ut1_utc;
}

void iers_free( IERS * iers ) {
    iers->size = 0;
    free(iers->eops);
    iers->eops = NULL;
}

int iers_print_time( IERS_EOP * eop, jday jd_utc, FILE * stream ) {
    fprintf( stream, "Time\n" );

    // abort if given UTC time is incorrect
    if( !jday_is_valid(jd_utc) ) {
        fprintf( stream, "UTC:\tinvalid Julian Date\n");
        return 1;
    }

    // summarize Coordinated time and Terrestrial time
    jday jd_tt = utc2tt(jd_utc);
    char * utc = jday2str(jd_utc);
    char * tt = jday2str( jd_tt );
    fprintf( stream,
             "\tUTC:\t%s\t(%lf)\n"
             "\tTT:\t%s\t(%lf)\n",
             utc, jd_utc, tt, jd_tt );
    free(tt);
    free(utc);

    // Abort if the Earth Orientation is bad
    if( eop==NULL || !jday_is_valid(eop->mjd) ) {
        printf( stream, "UT1:\tInvalid Earth Orientation\n");
        return 2;
    }

    // Abort if the EOP isn't within a day of the given UTC time
    if( fabs(eop->mjd - jd_utc)>1.0 ) {
        printf( stream, "UTC:\tEOP is out of date for current time\n");
        return 3;
    }

    // Summarize the Universal Time
    double dt = iers_get_DeltaT(eop);
    jday jd_ut1 = iers_get_UT1( eop, jd_utc );
    char * ut1 = jday2str( jd_ut1 );
    char * method = (eop->dt_flag=='I') ? "measured" : "predicted";
    fprintf( stream,
            "\tUT1:\t%s\t(%lf)\n"
            "\tdT:\t%lf\t%s\n",
            ut1, jd_ut1, dt, method );
    free(ut1);

    return 0;
}

void iers_print_eop( IERS_EOP * eop, FILE * stream ) {
    char * stamp = jday2str(eop->mjd);
    fprintf( stream,
            "Earth Orientation\n"
            "\tMJD:%s\t(%lf)\n"
            "\tpm_x:%lf\t(e=%lf)\n"
            "\tpm_y:%lf\t(e=%lf)\n"
            "\tut1_utc:%lf\t(e=%lf)\n",
             stamp, eop->mjd, eop->pm_x, eop->pm_x_err,
             eop->pm_y, eop->pm_y_err, eop->ut1_utc, eop->ut1_utc_err);
    free( stamp );
}

// test ///////////////////////////////////////////////////////////////////////

void test_iers_load( CuTest * test ) {
    FILE * bulletinA = fopen( "../data/iers/finals2000A.data", "r" );
    CuAssertPtrNotNullMsg(test, "could not open default iers file", bulletinA );

    IERS iers;

    iers_create( &iers );
    CuAssertPtrNotNullMsg(test, "failed to allocate EOPs", iers.eops);

    int result = iers_load( &iers, bulletinA );
    CuAssertIntEquals( test, 10105, result );

    // compute and print the average assuming samples are evenly spaced...
    IERS_EOP avg = {0.0,'M',0.0,0.0,0.0,0.0,'M',0.0,0.0};
    for(int n=0; n<iers.size; n++) {
        IERS_EOP * eop = &(iers.eops[n]);

        // ensure flags are valid
        CuAssert(test, "Invalid flag for polar offset",
                 eop->pm_flag=='I' || eop->pm_flag=='P' || eop->pm_flag==' ' );

        CuAssert(test, "Invalid flag for utc-ut1",
                 eop->dt_flag=='I' || eop->dt_flag=='P' || eop->dt_flag==' ' );

        avg.mjd+=eop->mjd;
        avg.pm_x+=eop->pm_x;
        avg.pm_x_err+=eop->pm_x_err;
        avg.pm_y+=eop->pm_y;
        avg.pm_y_err+=eop->pm_y_err;
        avg.ut1_utc+=eop->ut1_utc;
        avg.ut1_utc_err+=eop->ut1_utc_err;
    }
    avg.mjd/=iers.size;
    avg.pm_x/=iers.size;
    avg.pm_x_err/=iers.size;
    avg.pm_y/=iers.size;
    avg.pm_y_err/=iers.size;
    avg.ut1_utc/=iers.size;
    avg.ut1_utc_err/=iers.size;
    fprintf( stdout, "Average Earth Orientation:\n");
    iers_print_eop( &avg, stdout );
    fprintf( stdout, "\n");
    fflush( stdout );

    iers_free( &iers );

    fclose( bulletinA );
}

// a helper method which serches for the upper bound with a linear search. used to test correctness of binary search
IERS_EOP * linear_search( IERS * iers, jday time ) {
    // linear search for upper bound
    int n = 0;
    while (n < iers->size)
        if (time <= iers->eops[n].mjd)
            break;
        else n++;
    // check bounds
    if (n==iers->size)
        return NULL;
    else
        return &(iers->eops[n]);
}

void test_iers_search( CuTest * test ) {
    // create the IERS structure
    IERS iers;
    if (!iers_create( &iers ))
        CuFail(test, "failed to initialize iers structure");

    // generate a contiguous sequence of days like the IERS datasets
    int count = 50;
    jday offset = 2400000.5;
    double step = 0.5;
    IERS_EOP temp;
    memset(&temp, 0, sizeof(IERS_EOP));
    for(int n=0; n<count; n++) {
        temp.mjd = offset + n*step;
        int result = iers_add( &iers, &temp );
        CuAssertIntEquals_Msg(test, "iers_add failed", 0, result);
    }

    // test bounds
    IERS_EOP * eop;
    eop = iers_search( &iers, offset - step );
    CuAssertPtrEquals_Msg(test,"searches before first item should return null", NULL, eop);
    eop = iers_search( &iers, offset + (count+1) * step );
    CuAssertPtrEquals_Msg(test,"searches after last item should return null", NULL, eop);

    // search for exact matches to every time
    for (int n=0; n<count; n++) {
        jday time = offset + step * n;

        // search for exact match
        IERS_EOP * leop = linear_search( &iers, time );
        IERS_EOP * beop = iers_search( &iers, time );
        CuAssertPtrEquals_Msg(test,
                              "binary search for upper bound of matching time does not return same result as a linear search",
                              leop, beop);
        CuAssertDblEquals_Msg(test, "Returned search Time does not match", time, beop->mjd, 0.0);

        // search for midpoints of intervals
        if(n==count-1)
            continue;
        leop = linear_search( &iers, time+(step/2) );
        beop = iers_search( &iers, time+(step/2) );
        CuAssertPtrEquals_Msg(test,
                              "binary search for upper bound of interspersed time does not return same result as a linear search",
                              leop, beop);
        CuAssert(test, "Returned search time is not an upper bound", time<=beop->mjd);
        // should probably test the lower element as well...

        // NOTE : might want to extend this to test interpolation or rounding...
    }
}

//size_t iers_bsearch( IERS * iers, jday time, size_t low, size_t size ) {
//    // search for upper bound with binary stride search
//    while (size > 0) {
//        size_t half = size / 2;
//        size_t probe = low + half;
//        size_t other = low + size - half;
//        size = half;
//        low = (time < iers->eops[probe].time) ? low : other;
//    } // an optimized search with fewer comparisons, the latter of which can be compiled to a cmovaeq instruction
//    // adapted from https://academy.realm.io/posts/how-we-beat-cpp-stl-binary-search/
//    return low;
//}
//
//size_t iers_lsearch( IERS * iers, jday time, size_t low ) {
//    while (low < iers->size)
//        if (time <= iers->eops[low].time)
//            break;
//        else low++;
//    return low;
//}
//
//IERS_EOP * iers_search( IERS * iers, jday time ) {
//    size_t n = iers->cache;
//    // check cache and dt to see if we should should linear search
//    if ( n==0 )
//        n = iers_bsearch(iers, time, 0, iers->size);
//    else if ( time < iers->eops[n].time )
//        n = iers_bsearch(iers, time, 0, n);
//    else if ( time < iers->eops[n].time + IERS_LINEAR_THRESHOLD)
//        n = iers_lsearch(iers, time, cache);
//    else
//        n = iers_bsearch(iers, time, cache, iers->size);
//
//    // check bounds, we only want to return values in a measured interval
//    if (n == 0 || n == iers->size)
//        return NULL;
//
//    // should we interpolate between the two adjacent values?
//
//    return &(iers->eops[low]);
//}