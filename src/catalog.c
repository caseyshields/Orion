//
// Created by Casey Shields on 4/27/2018.
//

#include <novasc3.1/novas.h>
#include <stdlib.h>
#include <stdio.h>
#include "catalog.h"
#include "legacy/heap.h"
#include "vmath.h"

/** Creates a new catalog at the given pointer.
 * If the 'catalog' reference is NULL a new catalog and entries are allocated using the hint.
 * If the reference is valid and 'allocate' is positive, new entries are allocated and teh old are freed.
 * Otherwise the catalog's pre-existing references are used. This allows you to reuse previously allocated catalogs.
 * Returns the initialized catalog. */
Catalog* catalog_create(Catalog *catalog, size_t allocate) {
    // If they request a new catalog but don't provide a size hint just guess a default.
    if( !catalog && allocate<=0 )
        allocate = 64;

    // allocate a catalog if none is provided
    if( !catalog ) {
        catalog = malloc(sizeof(Catalog));
        if (!catalog) {
            perror("Catalog allocation failed");
            exit(1);
        }
    }

    // allocate the Entry index if a hint is given
    if( allocate > 0 ) {
        if( catalog->stars )
            free(catalog->stars);
        catalog->stars = calloc( (size_t)allocate, sizeof(Entry) );
        catalog->allocated = allocate;
        if( !catalog->stars ) {
            perror("Catalog directory allocation failed");
            exit(1);
        }
    }

    // clear the index by zeroing the size;
    catalog->size = 0;

    return catalog;
}

/**  */
Catalog* catalog_load_fk5( Catalog *catalog, FILE *f ) {
    char buf[1024], *s;
    double hour, min, sec;
    double deg, arcmin, arcsec;
    int n=0;

    // create a catalog if no reference was given
    if( !catalog )
        catalog = catalog_create(NULL, 128);

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
        catalog_add( catalog, entry );

        n++;
    } while(1);
    return catalog;
}

/** Adds the given entry to the catalog, doubling the allocated space if necessary. */
void catalog_add( Catalog *catalog, Entry *entry ) {
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

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * catalog: the catalog to be searched.
 * ra: right ascension of axis of search cone volume in hours.
 * dec: declination of axis of search cone volume in degrees.
 * r: angle between axis and edge of search cone volume in degrees.
 * results: A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * */
Catalog* catalog_search_dome( Catalog *catalog, double ra, double dec, double r, Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, catalog->size / 4 );

    // find a direction vector for the axis of the search cone
    double A[3], S[3];
    spherical2cartesian(
            hours2radians(ra),
            degrees2radians(dec),
            A );

    // determine the maximum angular separation
    double max = cos( degrees2radians(r) );

    // for all catalog entries
    for( int n=0; n<catalog->size; n++ ) {
        Entry* entry = catalog->stars[n];

        // if the entry's unit vectors is within the cone, add it to the results
        spherical2cartesian(
                hours2radians(entry->ra),
                degrees2radians(entry->dec),
                S );
        if( dot(A, S) > max )
            catalog_add( results, entry );
    }

    return results;
}

Catalog* catalog_orange( Catalog* c, double ra_min, double ra_max, Catalog* results ) {
    // wrap right ascension
    volatile double min = fmod( ra_min, 24.0);
    volatile double max = fmod( ra_max, 24.0);
    int orange(Entry* e) {
        // check for wrap-around assuming min to max direction is clockwise
        if( min < max ) {
            // continuous bound test
            if( min <= (e->ra) && (e->ra) <= max )
                return 1;
        } else {
            // disjoint bound test
            if (min <= (e->ra) || (e->ra) <= max)
                return 1;
        }
        return 0;
    }
    results = catalog_filter( c, orange, results );

    return results;
}

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * catalog: The catalog to be searched.
 * ra_min, ra_max: Right ascension bounds, inclusive.
 * dec_min, dec_max: Declination bounds, inclusive.
 * results: A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * */
Catalog* catalog_search_patch( Catalog *catalog, double min_ra, double max_ra, double min_dec, double max_dec,
                              Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, catalog->size / 4 );

    // if the entry's unit vectors is within or on the patch, add it to the results
    for( int n=0; n<catalog->size; n++ ) {
        Entry* entry = catalog->stars[n];
        // check right ascension bounds remembering 0 to 23 wraps around


        if( min_ra <= entry->ra && entry->ra <= max_ra
            && min_dec <= entry->dec && entry->dec <= max_dec )
            catalog_add( results, entry );
    }

    return results;
}

/** Searches a catalog for Entries for which the predicate function returns true.
 * No effort is taken to remove duplicates from the results.
 * catalog: The catalog to be searched.
 * predicate: a pointer to a boolean function of an Entry
 * results: A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * */
Catalog* catalog_filter( Catalog *catalog, int (*predicate)(Entry *), Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, catalog->size / 4 );

    for( int n=0; n<catalog->size; n++ )
        if( (*predicate)(catalog->stars[n]) )
            catalog_add( results, catalog->stars[n] );

    return results;
}

/** Releases the Catalog and it's directory, but not the actual Entries. */
void catalog_free( Catalog *catalog ) {
    free( catalog->stars );
    free( catalog );
}

/** Releases the Entries underlying the Catalog.  */
void catalog_free_entries( Catalog *catalog ) {
    catalog_each( catalog, (EntryFunction)free );
}

void catalog_print( Catalog *catalog ) {
    catalog_each( catalog, entry_print );
}

void catalog_each( Catalog *catalog, void (*function)(Entry *) ) {
    for( int n=0; n<catalog->size; n++ )
        function( catalog->stars[n] );
}

void entry_print( Entry *star ) {
    printf( "%s.%li: %s (ra:%lf, dec:%lf, p:%lf, v=%lf)\n",
            star->catalog,
            star->starnumber,
            star->starname,
            star->ra,
            star->dec,
            star->parallax,
            star->magnitude );
    fflush(0);
}

// doesn't work because the stack values that affect the behavior of the stack
// are released before the function is called. C does not have closures.
//EntryPredicate patch_predicate( double ra_min, double ra_max, double dec_min, double dec_max ) {
//    // wrap right ascension
//    volatile double min = fmod( ra_min, 24.0);
//    volatile double max = fmod( ra_max, 24.0);
//    int f(Entry* e) {
//        // check for wrap-around assuming min to max direction is clockwise
//        if( min < max ) {
//            // continuous bound test
//            if( min <= (e->ra) && (e->ra) <= max )
//                return 1;
//        } else {
//            // disjoint bound test
//            if (min <= (e->ra) || (e->ra) <= max)
//                return 1;
//        }
//        return 0;
//    }
//    return f;
//}
//Catalog* catalog_orange( Catalog* c, double min, double max, Catalog* results) {
//    EntryPredicate p = patch_predicate( min, max, 0.0, 0.0 );
//    results = catalog_filter( c, p, results );
//    return results;
//}