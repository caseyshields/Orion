//
// Created by Casey Shields on 4/27/2018.
//

#include <novasc3.1/novas.h>
#include "catalog.h"

int init(Catalog* catalog, const int size) {
    catalog->size = size;
    catalog->stars = calloc( (size_t)size, sizeof(Entry) );
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
        if(!s) {
            catalog->size = n-1;
            break;
        }

        Entry* entry = &(catalog->stars[n]);

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

        // debug
        //StarMap_print_star(star);

        n++;
    } while(1);

    return catalog->size;
}

void print_catalog( const Catalog *catalog ) {
    int n;
    Entry *star;
    for(n=0; n<catalog->size; n++) {
        star = &catalog->stars[n];
        print_entry( star );
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
