//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_CATALOG_H
#define STARTRACK_CATALOG_H

#include <novasc3.1/novas.h>

typedef struct {
    int size;
    cat_entry stars[];
} Catalog;

//int load( FILE* file );


#endif //STARTRACK_CATALOG_H
