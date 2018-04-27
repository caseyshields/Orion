//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_CATALOG_H
#define STARTRACK_CATALOG_H

#include <stdio.h>
#include <novasc3.1/novas.h>

typedef struct {
    // these are lifted from novas.h
//    char starname[SIZE_OF_OBJ_NAME]; // name of star
//    char catalog[SIZE_OF_CAT_NAME];  // name of catalog
//    long int starnumber;             // catalog number
//    double ra;                       // celestial right ascention in hours
//    double dec;                      // celestial declination in degrees
//    double promora;                  // proper motion right ascension in mas/yr
//    double promodec;                 // proper motion declination in mas/yr
//    double parallax;                 // displacement due to parallax in mas
//    double radialvelocity;           // radial velocity in m/s
    struct cat_entry_struct;
    // NOTE: I had to add 'cat_entry_struct' to the structure definition in novas.h!

    float magnitude; // TODO add more catalog information...
} Entry;

typedef struct {
    int size;
    Entry* stars;
} Catalog;

int init( Catalog* catalog, const int size );
int load( Catalog* catalog, FILE* file );
//Aperture* search(double right_ascension, double declination, double radius);


#endif //STARTRACK_CATALOG_H
