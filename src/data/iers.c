#include "iers.h"

// returns a pointer to a new orientation in the IERS, allocating space if needed
IERS_EOP * iers_new_eop( IERS * iers ) {

    // if size has reached a power of 2 we resize the array
    size_t n = iers->size;
    if( n>1 && !(n&(n-1)) )
        iers->orientations = realloc(
                &(iers->orientations),
                2*iers->size*sizeof(IERS_EOP) );

    // add the orientation parameter
    //iers->orientations[ ++iers->size ] = eop;
    // this way might be more conventional, but it leads to more pointless copying or indirection.

    // add an orientation to the list and return a pointer to it for the user to configure.
    return &( iers->orientations[ ++iers->size ] );
}

IERS * iers_load( FILE * finals2000A ) {
    IERS * iers = malloc(sizeof(IERS));

    // TODO initialize the iers object...

    int size;
    char line[256], mjd[9];

    while ( -1 != getline(&line, &size, finals2000A) ) {
        IERS_EOP * eop = iers_new_eop( iers );

        // compute the modified julian date
        strncpy(mjd, line+7, 8);//7-16
        eop->time = atof(mjd) + 2400000.5;


    }


}

IERS_EOP * iers_get_orientation( IERS * iers, jday time ) {

}
