#include "iers.h"

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
    if( iers->size==0 || eop->time < iers->eops[iers->size-1].time )
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
    while ( -1 != getline( &line, &size, finals2000A) ) {

        // skip lines that are too short to be an IERS Bulletin A record
        if( strlen( line ) < IERS_RECORD_LENGTH )
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
        eop->time = atof(buf) + 2400000.5;

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

        // There are more values if we need them;
        // Length of Day, nutation(polar velocities), Bulletin B values...

        count++;
    }
    free(line);
    return count;
}

IERS_EOP * iers_get_orientation( IERS * iers, jday time ) {

}

void iers_free( IERS * iers ) {
    iers->size = 0;
    free(iers->eops);
    iers->eops = NULL;
}

void iers_print_eop( IERS_EOP * eop, FILE * stream ) {
    char * stamp = jday2stamp( eop->time );
    fprintf( stream, "t:%s\tpmX:%lf\tpmY:%lf\tdt:%lf",
             time, eop->pm_x, eop->pm_y, eop->ut1_utc );
    free( stamp );
}