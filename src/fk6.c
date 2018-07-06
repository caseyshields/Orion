//
// Created by Casey Shields on 5/14/2018.
//

#include <malloc.h>
#include <mem.h>
#include <h/fk6.h>
#include "h/fk6.h"

int scan_line( FILE* file, const char* header );
int get_field( char* line, int start, int end, char* dest );

FK6 * fk6_create() {
    // allocate the structure if necessary
    FK6 * fk6 = malloc( sizeof(FK6) );
    memset(fk6, 0, sizeof(FK6));
    assert( fk6 );

    // allocate the metadata array
    fk6->fields = calloc( 2, sizeof(FK6_Field));
    assert(fk6->fields);

    return fk6;
}

int fk6_load_fields( FK6* fk6, FILE* readme, const char* header ) {

    // find the metadata section
    if(
        scan_line(readme, header) < 0 ||
        scan_line(readme, SEPARATOR) < 0 ||
        //scan_line(file, FK6_1_FIELDS) < 0 ||
        scan_line(readme, SEPARATOR) < 0
    ) {
        perror( "Invalid format" );
        exit(1);
    }

    // read rows from the file
    int n = 0;
    FK6_Field *field = NULL;
    while (true) {
        size_t size = 0;
        char *line = NULL;
        char start[4] = "\0\0\0", end[4] = "\0\0\0";

        // check for end of file and end of record
        if (getline(&line, &size, readme) == -1)
            return 1;
        else if (strcmp(line, SEPARATOR) == 0)
            break;

        // if the second column is blank the row is an addendum to the explanation
        if( !get_field(line, 5, 7, end) ) {
            // read and append the rest of the explanation
            char rest[100] = " ";
            get_field( line, 35, strlen(line)-1, rest+1 );
            strncat( field->Explanations, rest, 100-strlen(field->Explanations)-1 );
        }

        // otherwise create the field and parse data into it
        else {
            // flush the field
            if( field != NULL ) {
                fk6_add_field(fk6, field);
                fk6_print_field( field, stdout );
                n++;
            }

            // allocate a new field structure
            field = malloc( sizeof(FK6_Field) );
            memset( field, 0, sizeof(FK6_Field) );
            assert( field != NULL );

            // read field from file
            field->end = atoi(end);

            // starting byte is absent in the case of single byte fields
            get_field(line, 1, 3, start);
            if( strcmp("", start) == 0 )
                field->start = field->end;
            else field->start = atoi( start );

            get_field( line, 8, 15, field->Format );
            get_field( line, 16, 22, field->Units );
            get_field( line, 23, 34, field->Label );
            get_field( line, 35, strlen(line)-1, field->Explanations );
        }

        free(line);
    }
    return 0;
}

void fk6_free( FK6 * fk6 ) {
    // free metadata
    free(fk6->fields);

    // free catalog data
//    for( int n=0; n<fk6->rows; n++)
//        free( )
}

void fk6_add_field( FK6 * fk6, FK6_Field * field ) {
    size_t n = fk6->cols;

    // resize whenever n gets larger than a powers of 2
    if( n>1 && !(n&(n-1)) )
        fk6->fields = realloc(
                fk6->fields, 2*n*sizeof(FK6_Field) );
    assert(fk6->fields);

    (fk6->fields)[fk6->cols] = *field;
    fk6->cols++;
    // TODO don't allocate field, instantiate it in place
}

/*// read each value in the row
        char* cell = strtok( line, "|" );
        while( cell != NULL ) {
*/
/** trim the substring and copy it to a newly allocated string, return the size of the resulting string. */
int get_field( char* line, int start, int end, char* dest ) {
    // trim leading whitspace
    while (start <= end)
        if (line[start] == ' ' || line[start] == '\n')
            start++;
        else break;

    // trim trailing whitespace
    while (start <= end)
        if (line[end] == ' ' || line[end] == '\n')
            end--;
        else break;

    // copy characters over
    for (int n = 0; n <= end - start; n++)
        dest[n] = line[n + start];

    // terminate the c-style string
    dest[end - start + 1] = '\0';

    // return the number of characters copied
    return end - start + 1;
}

int scan_line(FILE *file, const char *header) {
    int count = 0;
    while (true) {
        char *line = NULL;
        size_t size = 0;
        if (getline(&line, &size, file) == -1)
            return -count;
        if (strcmp(line, header) == 0)
            return count;
        count++;
        free(line);
    }
}

//ssize_t get_line(char **line, size_t *size) {
//    if (*line) {
//        free(*line);
//        *line = NULL;
//        *size = 0;
//    }
//    return getline(line, size, stdin);
//}

void fk6_print_field( FK6_Field * f, FILE * file ) {
    fprintf(file, "%s (%s, %s, %d-%d) : %s\n",
           f->Label, f->Units, f->Format, f->start, f->end, f->Explanations);
    fflush(file);
}