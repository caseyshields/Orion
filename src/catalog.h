//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_CATALOG_H
#define STARTRACK_CATALOG_H

#include <stdio.h>
#include "novasc3.1/novas.h"

/** Represents a entry in a star catalog. Extended from Novas's struct cat_entry */
typedef struct entry_struct {
    cat_entry novas;
    // TODO add more catalog information...
    float magnitude;
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

/** Creates a new catalog at the given pointer.
 * If the 'catalog' reference is NULL a new catalog and entries are allocated using the hint.
 * If the reference is valid and 'allocate' is positive, new entries are allocated and teh old are freed.
 * Otherwise the catalog's pre-existing references are used. This allows you to reuse previously allocated catalogs.
 * Returns the initialized catalog. */
Catalog* catalog_create(Catalog* c, size_t s);

/** Adds the given entry to the catalog, doubling the allocated space if necessary. */
void catalog_add( Catalog* c, Entry *e );

Catalog* catalog_load_fk5(Catalog* c, FILE* f);

typedef void (*EntryFunction)(Entry*);
/** Applies a void function to every entry in the catalog. */
void catalog_each( Catalog* c, EntryFunction f );

typedef int (*EntryPredicate)(Entry*);
/** Searches a catalog for Entries for which the predicate function returns true.
 * No effort is taken to remove duplicates from the results.
 * @param catalog : The catalog to be searched.
 * @param predicate : a pointer to a boolean function of an Entry
 * @param results : A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * @return a pointer to the resulting catalog, regardless if the result parameter was set to NULL. */
Catalog* catalog_filter(Catalog* c, EntryPredicate p, Catalog *result );

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * @param catalog : The catalog to be searched.
 * @param ra : Right ascension of axis of search cone volume in hours.
 * @param dec : Declination of axis of search cone volume in degrees.
 * @param r : Angle between axis and edge of search cone volume in degrees.
 * @param results : A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * @return a pointer to the resulting catalog, regardless if the result parameter was set to NULL. */
Catalog* catalog_search_dome( Catalog* c, double right_ascension, double declination, double radius, Catalog* results );

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * catalog: The catalog to be searched.
 * ra_min, ra_max: Right ascension bounds, inclusive.
 * dec_min, dec_max: Declination bounds, inclusive.
 * results: A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!*/
Catalog* catalog_search_patch( Catalog* c, double ra_min, double ra_max, double dec_min, double dec_max, Catalog* results );

Catalog* catalog_orange( Catalog* c, double min, double max, Catalog* results);

void catalog_print( Catalog *c );

//TODO implemant a sort function for ranking results
//void catalog_sort( Catalog* c, int (*comparison)(Entry*, Entry*) );

/** Releases the Entries underlying the Catalog.  */
void catalog_free_entries( Catalog *c );

/** Releases the Catalog and it's directory, but not the actual Entries. */
void catalog_free( Catalog *c );

// we might want to flesh out the model to include the current pointing direction of the tracker...
//typedef struct {
//    double ra, dec, r;
//} Aperture; // might be useful for some queries...

#endif //STARTRACK_CATALOG_H
