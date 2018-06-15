//
// Created by Casey Shields on 5/14/2018.
//

#ifndef STARTRACK_FK6_H
#define STARTRACK_FK6_H


#include <stdbool.h>
#include <assert.h>
#include "catalog.h"

const char* FK6_1_HEADER = "Byte-by-byte Description of file: fk6_1.dat\n";
const char* FK6_1_FIELDS = "   Bytes Format Units    Label     Explanations\n";
const char* SEPARATOR = "--------------------------------------------------------------------------------";

typedef struct fk6_field {
    int start;
    int end;
    char Format[5];
    char Units[7];
    char Label[13];
    char Explanations[80];
} fk6_field;

typedef struct fk6_catalog {
    int width, height;
    fk6_field* cols;
    Entry rows;
} fk6_catalog;

int fk6_load( Catalog* catalog, FILE* file );
int scan_line( FILE* file, const char* header );
int get_field( char* line, int start, int end, char* dest );
int add_field( fk6_field* field, fk6_catalog* catalog );

#endif //STARTRACK_FK6_H
