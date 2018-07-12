//
// Created by Casey Shields on 5/14/2018.
//

#ifndef STARTRACK_FK6_H
#define STARTRACK_FK6_H


#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#define FK6_1_HEADER "Byte-by-byte Description of file: fk6_1.dat\n"
#define FK6_3_HEADER "Byte-by-byte Description of file: fk6_3.dat\n"
#define SEPARATOR "--------------------------------------------------------------------------------\n"
//define FK6_3_FIELDS "   Bytes Format Units    Label     Explanations\n"

/** A field to hold the metadata contained in Visier/CDS ReadMe files. Each field describes a
 * parameter in an ascii line of data. */
typedef struct {
    /** The starting index of the field, 1-indexed, inclusive. */
    int start;

    /** The ending index of the field, 1-indexed, inclusive. This byte is part of the value!  */
    int end;

    /** Describes the ascii format of the data with a letter denoting the type followed by a
     * number holding the precision. An 'A' is a string, 'I' is an integer, 'F' is a fixed point. */
    char Format[5];

    /** The physical units of the parameter. */
    char Units[7];

    /** The name of the parameter */
    char Label[13];

    /** An explanation of the field's meaning */
    char Explanations[100];
} FK6_Field;

/** A collection of FK6_Fields which describe a CDS record, allowing  */
typedef struct {
    size_t cols;
    FK6_Field * fields;
    //char* data[][100];
} FK6;

/** Initializes a FK6 structure. */
FK6 * fk6_create();

/** Scans to the given header in a CDS readme file, then loads the metadata in that section of the file. */
int fk6_load_fields( FK6* fk6, FILE* readme, const char* header );

/** Adds the given field to the record metadata */
void fk6_add_field( FK6 * fk6, FK6_Field * field );

/** @returns A pointer to the first field with the given name, or NULL if no such field exists */
FK6_Field * fk6_get_field( FK6 * fk6, const char * label );

/** @return A pointer to the field at the given index. */
FK6_Field * fk6_get_index( FK6 * fk6, const int index );

/** Uses the given metadata to parse the field from the record and convert it to the appropriate type.
 * Fields with a format of 'A' are converted to C strings. 'I' fields are converted to long integers.
 * 'F' fields are converted to doubles. Make sure the destination pinter is appropriately typed!
 * @param record A line of text from a Vizier/CDS record
 * @param the metadata of the desired field
 * @param a ponter whose value will be set to the value of the requested field.*/
int fk6_get_value( char * record, FK6_Field * field, void * dest );

/** Frees the metadata information. */
void fk6_free( FK6 * fk6 );

/** Prints the field's information in a human readable format to the given file.
 * @param field the field to be serialized
 * @param file desired output*/
void fk6_print_field( FK6_Field * field, FILE * file );

#endif //STARTRACK_FK6_H
