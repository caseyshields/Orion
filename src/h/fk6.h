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
#define FK6_3_HEADER "Byte-by-byte Description of file: fk6_3.dat\n"
#define FK6_3_FIELDS "   Bytes Format Units    Label     Explanations\n"
#define SEPARATOR "--------------------------------------------------------------------------------\n"

typedef struct {
    int start;
    int end;
    char Format[5];
    char Units[7];
    char Label[13];
    char Explanations[100];
} FK6_Field;

typedef struct {
    size_t rows, cols;
    FK6_Field * fields;
    //char* data[][100];
} FK6;

FK6 * fk6_create();

int fk6_load_fields( FK6* fk6, FILE* readme, const char* header );

void fk6_add_field( FK6 * fk6, FK6_Field * field );

FK6_Field * fk6_get_field( FK6 * fk6, const char * label );

FK6_Field * fk6_get_index( FK6 * fk6, const int index );

int fk6_get_value( char * line, FK6_Field * field, void * dest );

int fk6_load_entries( FK6 * fk6, FILE * file );

void fk6_free( FK6 * fk6 );

void fk6_print_field( FK6_Field * field, FILE * file );

#endif //STARTRACK_FK6_H
