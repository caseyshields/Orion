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
    int rows, cols;
    FK6_Field* fields;
    char** data;
} FK6;

int fk6_load( FK6* fk6, FILE* file );


#endif //STARTRACK_FK6_H
