//
// Created by Casey Shields on 4/27/2018.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <h/catalog.h>

#include "novasc3.1/novas.h"
#include "h/catalog.h"
#include "h/vmath.h"
#include "h/fk6.h"

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

Catalog* catalog_search_dome( Catalog *catalog, double ra, double dec, double r, Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, catalog->size / 4 );

//    // find a direction vector for the axis of the search cone
//    double A[3], S[3];
//    spherical2cartesian(
//            hours2radians(ra),
//            degrees2radians(dec),
//            A );
//
//    // determine the maximum angular separation
//    double max = cos( degrees2radians(r) );
//
//    // for all catalog entries
//    for( int n=0; n<catalog->size; n++ ) {
//        Entry* entry = catalog->stars[n];
//
//        // if the entry's unit vectors is within the cone, add it to the results
//        spherical2cartesian(
//                hours2radians(entry->novas.ra),
//                degrees2radians(entry->novas.dec),
//                S );
//        if( dot(A, S) > max )
//            catalog_add( results, entry );
//    }

    double max = degrees2radians( r );
    for( int n=0; n<catalog->size; n++ ) {
        Entry *entry = catalog->stars[n];
        double d = orthodromic_distance(
                hours2radians(ra),
                degrees2radians(dec),
                hours2radians(entry->novas.ra),
                degrees2radians(entry->novas.dec) );
        if( d < max )
            catalog_add( results, entry );
    }

    return results;
}

//Catalog* catalog_orange( Catalog* c, double ra_min, double ra_max, Catalog* results ) {
//    // wrap right ascension
//    volatile double min = fmod( ra_min, 24.0);
//    volatile double max = fmod( ra_max, 24.0);
//    int orange(Entry* e) {
//        // check for wrap-around assuming min to max direction is clockwise
//        if( min < max ) {
//            // continuous bound test
//            if( min <= (e->novas.ra) && (e->novas.ra) <= max )
//                return 1;
//        } else {
//            // disjoint bound test
//            if (min <= (e->novas.ra) || (e->novas.ra) <= max)
//                return 1;
//        }
//        return 0;
//    }
//    results = catalog_filter( c, orange, results );
//
//    return results;
//}

Catalog* catalog_search_patch( Catalog *catalog, double min_ra, double max_ra, double min_dec, double max_dec,
                              Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, catalog->size / 4 );

    // if the entry's unit vectors is within or on the patch, add it to the results
    for( int n=0; n<catalog->size; n++ ) {
        Entry* entry = catalog->stars[n];
        // check right ascension bounds remembering 0 to 23 wraps around


        if( min_ra <= entry->novas.ra && entry->novas.ra <= max_ra
            && min_dec <= entry->novas.dec && entry->novas.dec <= max_dec )
            catalog_add( results, entry );
    }

    return results;
}

Catalog* catalog_filter( Catalog *catalog, int (*predicate)(Entry *), Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, catalog->size / 4 );

    for( int n=0; n<catalog->size; n++ )
        if( (*predicate)(catalog->stars[n]) )
            catalog_add( results, catalog->stars[n] );

    return results;
}

Entry * catalog_select( Catalog * catalog, unsigned long fkid ) {
    for(int n=0; n<catalog->size; n++)
        if( catalog->stars[n]->novas.starnumber == fkid )
            return catalog->stars[n];
    return NULL;
}

void catalog_free( Catalog *catalog ) {
    free( catalog->stars );
    free( catalog );
}

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
            star->novas.catalog,
            star->novas.starnumber,
            star->novas.starname,
            star->novas.ra,
            star->novas.dec,
            star->novas.parallax,
            star->magnitude );
    fflush(0);
}

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
        assert( entry );
        memset( entry, '\0', sizeof(Entry) );

        // uhh, I might want to make a smarter parser...
        sscanf(buf+1, " %li ", &(entry->novas.starnumber)); // col 1
        sscanf(buf+17, " %19c ", entry->novas.starname); // col 3
        strcpy(entry->novas.catalog, "FK6"); // FK6
        sscanf(buf+38, " %lf %lf %lf ", &hour, &min, &sec); // col 4
        sscanf(buf+59, " %lf %lf %lf ", &deg, &arcmin, &arcsec); // col 5
        sscanf(buf+77, " %lf ", &(entry->novas.promora)); // col 6?
        sscanf(buf+89, " %lf ", &(entry->novas.promodec)); // col 7?
        sscanf(buf+153, " %lf ", &(entry->novas.parallax)); // col 14
        sscanf(buf+179, " %lf ", &(entry->novas.radialvelocity)); // col
        sscanf(buf+186, " %f ", &(entry->magnitude)); // col 18
        /*TODO use make_cat_entry(
                          char star_name[SIZE_OF_OBJ_NAME],
                          char catalog[SIZE_OF_CAT_NAME],
                          long int star_num, double ra, double dec,
                          double pm_ra, double pm_dec, double parallax,
                          double rad_vel,
                          cat_entry *star)*/

        // combine hours minutes and seconds
        entry->novas.ra = hour+(min/60.0)+(sec/3600.0);
        if(buf[58]=='+')
            entry->novas.dec = deg+(arcmin/60.0)+(arcsec/3600.0);
        else
            entry->novas.dec = -deg-(arcmin/60.0)-(arcsec/3600.0);

        if(buf[76]=='-')
            entry->novas.promora*=-1.0;
        if(buf[88]=='-')
            entry->novas.promodec*=-1.0;
        if(buf[178]=='-')
            entry->novas.radialvelocity*=-1.0;

        // actually add the Entry to the catalog
        catalog_add( catalog, entry );

        n++;
    } while(1);
    return catalog;
}

Catalog * catalog_load_fk6_1( Catalog * catalog, FK6 * fk6, FILE * file ) {
    int count = 0;
    size_t size = 0;
    char * record = NULL;

    // create a catalog if no reference was given
    if( !catalog )
        catalog = catalog_create(NULL, 128);

    // first find the needed columns in the FK6 metadata
    FK6_Field * starname = fk6_get_field( fk6, "Name" );
    FK6_Field * starnumber = fk6_get_field( fk6, "FK6" );
    FK6_Field * rah = fk6_get_field( fk6, "RAh" ); // hours
    FK6_Field * ram = fk6_get_field( fk6, "RAm" ); // minutes
    FK6_Field * ras = fk6_get_field( fk6, "RAs" ); // seconds
    FK6_Field * sign = fk6_get_field( fk6, "DE-" ); // [-, ]
    FK6_Field * decd = fk6_get_field( fk6, "DEd" ); // degrees
    FK6_Field * decm = fk6_get_field( fk6, "DEm" ); // arcmin
    FK6_Field * decs = fk6_get_field( fk6, "DEs" ); // arcsec
    FK6_Field * promora = fk6_get_field( fk6, "pmRA*" ); // mas/yr
    FK6_Field * promodec = fk6_get_field( fk6, "pmDE" ); // mas/yr
    FK6_Field * parallax = fk6_get_field( fk6, "plx" );
    FK6_Field * radialvelocity = fk6_get_field( fk6, "RV" );
    FK6_Field * magnitude = fk6_get_field( fk6, "Vmag" );

//    // function for transforming a FK6 data record into a Catalog Entry
//    Entry * create_entry( char * record ) {
//        Entry * entry = calloc( 1, sizeof(Entry) );
//        fk6_get_value( record, starname, entry->novas.starname );
//        strcpy( "FK6", entry->novas.catalog );//fk6_get_value( record, survey, entry->novas.catalog );
//        fk6_get_value( record, starnumber, &(entry->novas.starnumber) );
//        // todo finish the rest of the fields
//        return entry;
//    }

    // read lines from the input
    while (true) {

        // check for end of file
        int result = getline(&record, &size, file);
        if( result == -1 )
            break;

        // create a catalog entry from the FK6 catalog
        Entry * entry = calloc( 1, sizeof(Entry) );
        fk6_get_value( record, starname, entry->novas.starname );
        strcpy( entry->novas.catalog, "FK6" ); // fk6_get_value( record, survey, entry->novas.catalog );
        fk6_get_value( record, starnumber, &(entry->novas.starnumber) );
        double d=0, h=0, m=0, s=0;
        fk6_get_value( record, rah, &h );
        fk6_get_value( record, ram, &m );
        fk6_get_value( record, ras, &s );
        entry->novas.ra = h + (m/60.0) + (s/3600.0);
        char p[2] = "\0\0";
        fk6_get_value( record, sign, &p );
        fk6_get_value( record, decd, &d );
        fk6_get_value( record, decm, &m );
        fk6_get_value( record, decs, &s );
        entry->novas.dec = d + (m/60.0) + (s/3600.0);
        if( strcmp("-", p)==0 )
            entry->novas.dec*=-1;
        fk6_get_value( record, promora, &(entry->novas.promora) );
        fk6_get_value( record, promodec, &(entry->novas.promodec) );
        fk6_get_value( record, parallax, &(entry->novas.parallax) );
        fk6_get_value( record, radialvelocity, &(entry->novas.ra) );
        fk6_get_value( record, magnitude, &(entry->magnitude) );

        catalog_add(catalog, entry);
    }

    free( record );
    return 0;
}


// doesn't work because the stack variables that affect the behavior of the function
// are out of scope when the function is called. C does not have closures.
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