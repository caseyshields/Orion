//
// Created by Casey Shields on 5/14/2018.
//

#ifndef STARTRACK_FK6_H
#define STARTRACK_FK6_H


#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#define FK6_1_HEADER "Byte-by-byte Description of file: fk6_1.dat\n"
#define FK6_1_FIELDS "   Bytes Format Units    Label     Explanations\n"
#define SEPARATOR "--------------------------------------------------------------------------------\n"

typedef struct {
    int start;
    int end;
    char Format[5];
    char Units[7];
    char Label[13];
    char Explanations[80];
} FK6_Field;

typedef struct {
    size_t rows, cols;
    FK6_Field * fields;
    //char* data[][100];
} FK6;

FK6 * fk6_create();
int fk6_load( FK6* fk6, FILE* file );
void fk6_free( FK6 * fk6 );

#endif //STARTRACK_FK6_H
