#include "engine/catalog.h"

Catalog* catalog_create(Catalog *catalog, size_t allocate) {
    // allocate a catalog if none is provided
    if( !catalog )
        catalog = malloc(sizeof(Catalog));
    if( catalog )
        memset( catalog, 0, sizeof(Catalog) );
    else {
        perror("Catalog allocation failed");
        return 0;
    }

    // If they don't provide a size hint just guess a default.
    if( allocate>0 )
        catalog->allocated = allocate;
    else catalog->allocated = 64;
    catalog->size = 0;

    // allocate the Entry index if a hint is given
    catalog->stars = calloc( (size_t)catalog->allocated, sizeof(Entry) );
    if( !catalog->stars ) {
        catalog->allocated = 0;
        // we should probably allocate this first so we can fail fast...
    }

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
} // TODO use realloc

Catalog* catalog_search_dome( Catalog *catalog, double ra, double dec, double r, Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, 0 );

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

Catalog * catalog_search_name( const Catalog * catalog, const char * substring, Catalog * results ) {
    // create a catalog to hold the results if the user did not supply one
    if( !results )
        results = catalog_create( NULL, 0 );

    // Add each catalog entry whose name contains the search phrase
    for( int n=0; n<catalog->size; n++ ) {
        Entry * entry = catalog->stars[n];
        if( strstr( entry->novas.starname, substring )!=NULL )
            catalog_add( results, entry );
    }

    return results;
}

Catalog * catalog_brighter(const Catalog *catalog, double min_mag, Catalog *results) {
    // create an output catalog if no catalog was given
    if( !results )
        results = catalog_create( NULL, 64 );

    // Add each catalog entry to the results which exceeds the minimum brightness
    for( int n=0; n<catalog->size; n++ ) {
        Entry * entry = catalog->stars[n];
        if( entry->magnitude < min_mag )
            catalog_add( results, entry );
    }
    return results;
}

Catalog* catalog_search_patch( const Catalog *catalog, double min_ra, double max_ra, double min_dec, double max_dec, Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, 0 );

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

Entry * catalog_get(const Catalog *catalog, unsigned long fkid) {
    for(int n=0; n<catalog->size; n++)
        if( catalog->stars[n]->novas.starnumber == fkid )
            return catalog->stars[n];
    return NULL;
}

void catalog_clear( Catalog *catalog ) {
    assert(catalog && catalog->stars);
    memset(catalog->stars, 0, catalog->size * sizeof(Entry*) );
    catalog->size = 0;
}

/** Frees all Catalog Entries, then frees the Catalog's list. */
void catalog_free( Catalog * catalog ) {
    assert( catalog && catalog->stars );

    // free every allocated catalog entry
    for( int n=0; n<catalog->size; n++ ) {
        Entry * entry = catalog->stars[n];
        free( entry );
        catalog->stars[n] = 0;
    }

    // then free the catalog's array of pointers
    free( catalog->stars );

    // zero out the Catalog's structure
    catalog->stars = 0;
    catalog->size = 0;
    catalog->allocated = 0;

    // I don't free the catalog pointer because it might be in the stack not the heap...
}

void catalog_print( Catalog *catalog ) {
    catalog_each( catalog, entry_print );
}

void catalog_each( Catalog *catalog, void (*function)(Entry *) ) {
    int n = 0;
    while ( n < catalog->size) {
        function(catalog->stars[n]);
        n++;
    }
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
    fflush( stdout );
}

Catalog * catalog_load_fk6(Catalog * catalog, FK6 *fk6, FILE *file) {
    int count = 0;
    size_t size = 0;
    char * record = NULL;

    // create a catalog if no reference was given
    if( !catalog )
        catalog = catalog_create(NULL, 0);

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
    assert( starname );
    assert( starnumber );
    assert( rah );
    assert( ram );
    assert( ras );
    assert( sign );
    assert( decd );
    assert( decm );
    assert( decs );
    assert( promodec );
    assert( promora );
    assert( parallax );
    assert( radialvelocity );
    assert( magnitude );

    // read lines from the input
    while (1) {

        // check for end of file
        int result = getline(&record, &size, file);
        if( result == -1 )
            break;

        // create a catalog entry from the FK6 catalog
        Entry * entry = calloc( 1, sizeof(Entry) );
        fk6_get_value( record, starname, entry->novas.starname );
        strcpy( entry->novas.catalog, "FK6" ); // fk6_get_value( record, survey, entry->novas.catalog );
        fk6_get_value( record, starnumber, &(entry->novas.starnumber) );
        long int d=0, h=0, m=0;
        double s=0;
        fk6_get_value( record, rah, &h );
        fk6_get_value( record, ram, &m );
        fk6_get_value( record, ras, &s );
        entry->novas.ra = h + (m/60.0) + (s/3600.0);
        char p[2] = "\0";
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
        fk6_get_value( record, radialvelocity, &(entry->novas.radialvelocity) );
        fk6_get_value( record, magnitude, &(entry->magnitude) );

        catalog_add(catalog, entry);
    }

    free( record );
    return catalog;
}

// test ///////////////////////////////////////////////////////////////////////

void test_FK6( CuTest * test ) {
    Catalog * catalog = catalog_create( 0, 1024 );

    // load metadata for the first part of FK6
    FILE * readme = fopen("../data/fk6/ReadMe", "r");
    CuAssertPtrNotNullMsg( test, "Could not find FK6 readme", readme );
    FK6 * fk6_1 = fk6_create();
    fk6_load_fields( fk6_1, readme, FK6_1_HEADER );
//    for(int n=0; n<fk6_1->cols; n++)
//        fk6_print_field( &(fk6_1->fields[n]), stdout );
    CuAssertIntEquals( test, 93, fk6_1->cols );

    // load first part of FK6
    FILE * data1 = fopen("../data/fk6/fk6_1.dat", "r");
    CuAssertPtrNotNullMsg( test, "Could not find FK6 Part I", data1);
    catalog_load_fk6( catalog, fk6_1, data1 );
    fclose( data1 );
    CuAssertIntEquals( test, 878, catalog->size );

    // How can I test the validity of this data a bit more thoroughly...

    // release all objects
    fk6_free( fk6_1 );
    free( fk6_1 );
    catalog_free( catalog );
    free( catalog );
}

// TODO Need to come up with a dummy catalog generator which I can test the filters against...
// Work in Progress!
void catalog_add_axis(Catalog * catalog, int type, int count);
void test_search_equator() {
    Catalog * catalog = catalog_create( NULL, 8 );
    catalog_add_axis( catalog, 1, (24*60*60) );


    // test each query along the axis contains
}

/** add either an equator or meridian to a catalog
 * @param count number of equidistant points on the great circle.
 * @param type 1 = Equator, 2 = Prime Meridian*/
void catalog_add_axis(Catalog * catalog, int type, int count) {

    // for the desired number of points
    // notice <= so equator will wrap around, and meridian set will contain both poles
    for (int n=0; n<=count; n++) {

        // allocate and zero all values
        Entry * entry = malloc( sizeof(Entry) );
        memset( entry, 0, sizeof(Entry) );

        // calculate test values
        memcpy( entry->novas.catalog, "tst\0", 4);
        if ( type == 1 ) {
            double hours = 24.0 * n / count;
            entry->novas.ra = hours;
            sprintf( entry->novas.starname, "equator %lf", hours );
        } else if (type == 2) {
            double degrees = (180.0 * n / count) - 90.0;
            entry->novas.dec = degrees;
            sprintf( entry->novas.starname, "meridian %lf", degrees );
        }

        catalog_add(catalog, entry);
    }
} // need to generalize to an arbitrary lesser circle!

// TODO readme columns in bsc5 do not line up, in fact they overlap. so this will not work with the FK6 loader
// try using the metadata loader to load the yale
void test_BSC5() {
    // it could probably be made to work by splitting on whitespace.
    Catalog * catalog = catalog_create( 0, 1024 );

    FILE * readme = fopen("../data/bsc5/ReadMe", "r");
    assert(NULL != readme);

    char* header = "Byte-by-byte Description of file: catalog\n";
    FK6 * bsc5 = fk6_create();
    fk6_load_fields( bsc5, readme, header );//FK6_1_HEADER );
    fclose( readme );

    FILE * data = fopen("../data/bsc5/catalog", "r");
    assert(NULL != data);
    catalog_load_fk6(catalog, bsc5, data);
    fclose( data );
    fk6_free( bsc5 );
    free( bsc5 );

    catalog_print( catalog );

    catalog_free( catalog );
    free( catalog );
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

// another option is GNU C nested functions
//Catalog * human_visible( Catalog * catalog ) {
//    double min = 6.0;
//    int is_bright(Entry *entry) {
//        return entry->magnitude <= 6.0;
//    }
//    Catalog *result = catalog_filter(catalog, is_bright, NULL);
//    return result;
//}

// Unfortunately Nested functions are not portable.
// You can simulate them by externally defining the function, and passing an object holding relevant state.

//int is_bright( Entry * entry, void * context ) {
//    int mag_min = *((int*)context); // context is simply a pointer to an int
//    return entry->magnitude <= mag_min;
//}
// int mag_min = 6.0;
// Catalog * bright_stars = catalog_filter( catalog, is_bright, &mag_min );

// This gets messy quickly. I'm opting to just using a more old fashioned approach..
// So I'm removing all the methods which rely on nested functions

Catalog* catalog_filter( const Catalog *catalog, const EntryPredicate predicate, Catalog *results ) {
    // create an output catalog if no reference was given
    if( !results )
        results = catalog_create( NULL, 0 );

    for( int n=0; n<catalog->size; n++ )
        if( (*predicate)(catalog->stars[n]) )
            catalog_add( results, catalog->stars[n] );

    return results;
}