//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_CATALOG_H
#define STARTRACK_CATALOG_H

#include <stdio.h>
#include "novasc3.1/novas.h"
#include "h/fk6.h"

/** Represents celestial object in a star catalog. Extended from Novas's struct cat_entry to add
 * some more parameters and transformed coordinates.*/
typedef struct {

    /** The Novas structure which contains all the information needed to perform astrometric calculations. */
    cat_entry novas;

    /** The visual magnitude of the star. */
    float magnitude;

    /** Scratch-space for transformed horizon coordinates- only makes sense in the context of a Tracker */
    double zenith_distance, topocentric_azimuth;

    /** A pointing vector in geocentric coordinates, only makes sense in the context of a Tracker */
    double E, F, G;
    // todo To follow better encapsulation principles we may want to extract transformed coordinates into a structure which references the catalog entry.

} Entry;

void entry_print( Entry *e );

/** A catalog of stars. provides methods for loading, searching, filtering and printing entries. */
typedef struct {

    /** Current number of stars in the catalog */
    size_t size;

    /** Tracks space currently allocated for the catalog. */
    size_t allocated;

    /** An array of catalog entries. */
    Entry** stars;

} Catalog;

/** Creates a new catalog at the given pointer.
 * If the 'catalog' reference is NULL a new catalog and entries are allocated using the hint.
 * If the reference is valid and 'allocate' is positive, new entries are allocated and the old are freed.
 * Otherwise the catalog's pre-existing references are used. This allows you to reuse previously allocated catalogs.
 * Returns the initialized catalog. */
Catalog* catalog_create(Catalog* c, size_t s);

/** Adds the given entry to the catalog, doubling the allocated space if necessary. */
void catalog_add( Catalog* c, Entry *e );

/** Loads a FK5 catalog printout from the given file. */
Catalog* catalog_load_fk5(Catalog * c, FILE * f);

/**  */
Catalog * catalog_load_fk6_1( Catalog * catalog, FK6 * fk6, FILE * file );

typedef void (*EntryFunction)(Entry*);

/** Applies a void function to every entry in the catalog. */
void catalog_each( Catalog * c, EntryFunction f );

/** A predicate used in catalog filters to test individual entries. */
typedef int (*EntryPredicate)(Entry*);

/** Searches a catalog for Entries for which the predicate function returns true.
 * No effort is taken to remove duplicates from the results.
 * @param catalog The catalog to be searched.
 * @param predicate a pointer to a boolean function of an Entry
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * @return a pointer to the resulting catalog, regardless if the result parameter was set to NULL. */
Catalog* catalog_filter(Catalog * catalog, EntryPredicate predicate, Catalog * result );

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * @param catalog The catalog to be searched.
 * @param right_ascension Right ascension of axis of search cone volume in hours.
 * @param declination Declination of axis of search cone volume in degrees.
 * @param radius Angle between axis and edge of search cone volume in degrees.
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * @return a pointer to the resulting catalog, regardless if the result parameter was set to NULL. */
Catalog* catalog_search_dome( Catalog* catalog, double right_ascension, double declination, double radius, Catalog* results );

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * @param catalog The catalog to be searched.
 * @param ra_min Right ascension lower bound, inclusive.
 * @param ra_max Right ascension upper bound, inclusive.
 * @param dec_min Declination lower bound, inclusive.
 * @param dec_max Declination upper bound, inclusive.
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!*/
Catalog* catalog_search_patch( Catalog* catalog, double ra_min, double ra_max, double dec_min, double dec_max, Catalog* results );

//Catalog* catalog_orange( Catalog* c, double min, double max, Catalog* results);

/** Selects the first entry with the given Fundamental Catalog ID.
 * @param catalog the catalog to search
 * @param fkid The Hipparcos/Tycho catalog ID
 * @return a pointer to the star's entry, or NULL if it is not in the catalog */
Entry * catalog_select( Catalog * catalog, unsigned long fkid );

/** Prints the given catalog to stdout in a default format. */
void catalog_print( Catalog *c );

/** A function for ordering catalog entries */
typedef void (*EntryComparison)(Entry*, Entry*);

//void catalog_sort( Catalog* c, int (*comparison)(Entry*, Entry*) );
//todo implement a sort function for ranking results

/** Releases the Entries underlying the Catalog. */
void catalog_free_entries( Catalog *c );

/** Releases the Catalog and it's directory, but not the actual Entries. */
void catalog_free( Catalog *c );

// todo we might want to flesh out the model to include the current pointing direction of the tracker, which we can obtain through TATS sources...
//typedef struct {
//    double ra, dec, r;
//} Aperture; // might be useful for some queries...

#endif //STARTRACK_CATALOG_H
