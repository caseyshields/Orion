//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_CATALOG_H
#define STARTRACK_CATALOG_H

#include <stdio.h>
#include <novasc3.1/novas.h>

/** Represents a entry in a star catalog. Extended from Novas's struct cat_entry */
typedef struct entry_struct {

    struct cat_entry_struct;
//    char starname[SIZE_OF_OBJ_NAME]; // name of star
//    char catalog[SIZE_OF_CAT_NAME];  // name of catalog
//    long int starnumber;             // catalog number
//    double ra;                       // celestial right ascention in hours
//    double dec;                      // celestial declination in degrees
//    double promora;                  // proper motion right ascension in mas/yr
//    double promodec;                 // proper motion declination in mas/yr
//    double parallax;                 // displacement due to parallax in mas
//    double radialvelocity;           // radial velocity in m/s
    // NOTE: I had to modify novas.h to extend the 'cat_entry' struct
    // specifically I added 'cat_entry_struct' to the structure definition

    float magnitude; // TODO add more catalog information...
    double x, y, z;
    double az, el;
    double E, F, G;
} Entry;

void entry_print( Entry *e );

/** A catalog of stars. provides methods for loading, searching and printing. */
typedef struct catalog_struct {
    size_t size;
    size_t allocated;
    Entry** stars;
} Catalog;

typedef void (*EntryFunction)(Entry*);

typedef int (*EntryPredicate)(Entry*);

//typedef struct {
//    double ra, dec, r;
//} Aperture; // might be useful for some queries...

Catalog* catalog_create(Catalog* c, size_t s);
Catalog* catalog_load_fk5(Catalog* c, FILE* f);
Catalog* catalog_search_dome( Catalog* c, double right_ascension, double declination, double radius, Catalog* results );
Catalog* catalog_search_patch( Catalog* c, double ra_min, double ra_max, double dec_min, double dec_max, Catalog* results );
Catalog* catalog_filter(Catalog* c, EntryPredicate p, Catalog *result );
void catalog_add( Catalog* c, Entry *e );
void catalog_each( Catalog* c, EntryFunction f );
//void catalog_sort( Catalog* c, int (*comparison)(Entry*, Entry*) );
void catalog_print( Catalog *c );
void catalog_free_entries( Catalog *c );
void catalog_free( Catalog *c );

Catalog* catalog_orange( Catalog* c, double min, double max, Catalog* results);

#endif //STARTRACK_CATALOG_H
