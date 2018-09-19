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
        eop->mjd = atof(buf) + 2400000.5;

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
} // TODO ugh, duh, since the bulletin records are perfectly spaces we should use an interpolation search in basically O(1)!

jday iers_get_UT1( IERS_EOP * eop, jday utc ) {
    return utc + eop->ut1_utc / SECONDS_IN_DAY;
    //TODO ensure utc is within a day of the eop entry?
}

double iers_get_DeltaT( IERS_EOP * eop ) {
    return DELTA_TT + LEAP_SECONDS - eop->ut1_utc;
}

void iers_free( IERS * iers ) {
    iers->size = 0;
    free(iers->eops);
    iers->eops = NULL;
}

void iers_print_eop( IERS_EOP * eop, FILE * stream ) {
    char * stamp = jday2str(eop->mjd);
    fprintf( stream, "t:%s\tpmX:%lf\tpmY:%lf\tdt:%lf\te={%lf,%lf,%lf}",
             stamp, eop->pm_x, eop->pm_y, eop->ut1_utc,
             eop->pm_x_err, eop->pm_y_err, eop->ut1_utc_err);
    free( stamp );
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