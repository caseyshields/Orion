//
// Created by Casey Shields on 4/27/2018.
//

#include <novasc3.1/novas.h>
#include <stdlib.h>
#include "catalog.h"
#include "legacy/heap.h"

int init(Catalog* catalog, const int allocate) {
    catalog->allocated = allocate;
    catalog->size = 0;
    catalog->stars = calloc( (size_t)allocate, sizeof(Entry) );
    if( catalog->stars )
        return 1;
    return 0;
}

int load( Catalog* catalog, FILE* f) {
    char buf[1024], *s;
    double hour, min, sec;
    double deg, arcmin, arcsec;
    int n=0;

    if(!catalog) return 1;

    // parse FK6 entries from file
    while(n<10) { // skip first ten lines
        s = fgets(buf, 1024, f);
        n++;
    }
    n=0;
    do {
        // get a line from the catalog
        s = fgets(buf, 1024, f);
        if(!s) break;

        // allocate a new entry
        Entry* entry = malloc( sizeof(Entry) );

        // uhh, I might want to make a smarter parser...
        sscanf(buf+1, " %li ", &(entry->starnumber)); // col 1
        sscanf(buf+17, " %19c ", entry->starname); // col 3
        strcpy(entry->catalog, "FK6"); // FK6
        sscanf(buf+38, " %lf %lf %lf ", &hour, &min, &sec); // col 4
        sscanf(buf+59, " %lf %lf %lf ", &deg, &arcmin, &arcsec); // col 5
        sscanf(buf+77, " %lf ", &(entry->promora)); // col 6?
        sscanf(buf+89, " %lf ", &(entry->promodec)); // col 7?
        sscanf(buf+153, " %lf ", &(entry->parallax)); // col 14
        sscanf(buf+179, " %lf ", &(entry->radialvelocity)); // col
        sscanf(buf+186, " %f ", &(entry->magnitude)); // col 18
        /*TODO use make_cat_entry(
                          char star_name[SIZE_OF_OBJ_NAME],
                          char catalog[SIZE_OF_CAT_NAME],
                          long int star_num, double ra, double dec,
                          double pm_ra, double pm_dec, double parallax,
                          double rad_vel,
                          cat_entry *star)*/

        // combine hours minutes and seconds
        entry->ra = hour+(min/60.0)+(sec/3600.0);
        if(buf[58]=='+')
            entry->dec = deg+(arcmin/60.0)+(arcsec/3600.0);
        else
            entry->dec = -deg-(arcmin/60.0)-(arcsec/3600.0);

        if(buf[76]=='-')
            entry->promora*=-1.0;
        if(buf[88]=='-')
            entry->promodec*=-1.0;
        if(buf[178]=='-')
            entry->radialvelocity*=-1.0;

        // actually add the Entry to the catalog
        add( catalog, entry );
        // debug
        //StarMap_print_star(star);

        n++;
    } while(1);

    return catalog->size;
}

void add( Catalog* catalog, Entry* entry ) {
    // if full, copy Entries into larger array
    if( catalog->size == catalog->allocated ) {
        Entry** old = catalog->stars;
        catalog->stars = calloc( (size_t)(catalog->allocated*2), sizeof(Entry*) );
        for( int n=0; n<catalog->allocated; n++ )
            catalog->stars[n] = old[n];
        catalog->allocated *= 2;
        free(old);
    }

    // add the new entry
    catalog->stars[ catalog->size++ ] = entry;
}

/** catalog: the catalog to be searched.
 * ra: right ascension of axis of search cone volume in hours.
 * dec: declination of axis of search cone volume in degrees.
 * r: angle between axis and edge of search con volume in degrees.
 * result: a subset of the catalog which is in the search volume. references are shared
 * */
int search(Catalog* catalog, double ra, double dec, double r, Catalog *result) {
    // since catalog is ordered by right ascension;
    // binary search for the entry s.t. entry.ra >= ra-r;
    int low=0, mid, high;


    // while entry.ra < ra+r
        // compute angular separation
        // original method was the dot product of unit vectors of the spherical coordinates
        // might be able to compute it more directly...
        // if angular separation is less than r
            // add to results
    /* x^2 + y^2 + z^2 = p^2
     * x = p sin(theta)cos(phi)
     * y = p sin(theta)sin(phi)
     * z = p cos(theta)*/
}

void filter(Catalog* catalog, int (*predicate)(Entry*), Catalog* results) {
    for( int n=0; n<catalog->size; n++ )
        if( (*predicate)(catalog->stars[n]) )
            add( results, catalog->stars[n] );
}

/** Deallocates the catalog, including the underlying Entries if 'free_entrees' is true.
 * This requires some care when cleaning up resources. One strategy is making sure catalogs are completely disjoint.
 * Another is keeping a master catalog which is a superset of all the others, and only deallocating it's entries. */
void freeCatalog( Catalog* catalog, int free_entries ) {
    if( free_entries )
        for( int n=0; n<catalog->allocated; n++ )
            free( catalog->stars[n] );
    free( catalog->stars );
} // TODO another less-efficient option is making queries perform a deep copy on entries

void print_catalog( const Catalog *catalog ) {
    int n;
    for(n=0; n<catalog->size; n++) {
        print_entry( catalog->stars[n] );
    }
    fflush(0);
}

void print_entry( const Entry *star ) {
    printf("%s.%li: %s (ra:%lf, dec:%lf, p:%lf, v=%lf)\n",
           star->catalog,
           star->starnumber,
           star->starname,
           star->ra,
           star->dec,
           star->parallax,
           star->magnitude);
}
