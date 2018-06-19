//
// Created by Casey Shields on 5/14/2018.
//

#include <mem.h>
#include <malloc.h>
#include "h/fk6.h"

int scan_line( FILE* file, const char* header );
int get_field( char* line, int start, int end, char* dest );
//int add_field( fk6_field* field, fk6_catalog* catalog );

FK6 * fk6_create(FK6 * fk6, FILE* file) {
    if(fk6==NULL)
        fk6 = malloc( sizeof(FK6) );
    memset(fk6, 0, sizeof(FK6));
    return fk6;
}

int fk6_load(FK6 * fk6, FILE* file ) {
    FK6_Field metadata[100];
    int n = 0;

    // find the metadata section
            if( scan_line(file, FK6_1_HEADER) == -1
        || scan_line(file, FK6_1_FIELDS) == -1
        || scan_line(file, SEPARATOR) == -1 ) {
                perror( "Invalid format" );
                exit(1);
            }

            // read rows from the file
    FK6_Field *field = NULL;
            while (true) {
                char *line = NULL;
                size_t size = 0;
                char start[3], end[3];

                // check for end of file and end of record
        if (getline(&line, &size, file) == -1)
            return 1;
        else if (strcmp(line, SEPARATOR) == 0)
            break;

        // if the second column is blank the row is an addendum to the explanation
        if( !get_field(line, 5, 7, end) ) {

        }

        // otherwise create the field an parse data into it
        else {
            // flush the field
            if( field != NULL ) {
                //catalog_add( catalog, field );
            }

            field = malloc(sizeof(FK6_Field));
            assert( field!=NULL );

            field->end = atoi(end);

            // starting byte is absent in the case of single byte fields
                    get_field(line, 1, 3, start);
                    if( start == NULL )
                        field->start = field->end;
                    else field->start = atoi( start );

                    get_field( line, 8, 15, field->Format );
                    get_field( line, 16, 22, field->Units );
                    get_field( line, 23, 34, field->Label );
                    get_field( line, 35, 79, field->Explanations );

        }

        n++;
        free(line);
            }


}

/*// read each value in the row
        char* cell = strtok( line, "|" );
        while( cell != NULL ) {
*/
/** trim the substring and copy it to a newly allocated string */
int get_field( char* line, int start, int end, char* dest ) {
    while (start < end) {
        if (line[start] == ' ')
            start++;

        if (start == end)
            return 0;

        while (start < end)
            if (line[end] == ' ')
                end--;

        for (int n = 0; n <= end - start; n++)
            dest[n] = line[n + start];
        dest[end - start + 1] = '\0';

        return end - start + 1;
    }
}

int scan_line(FILE *file, const char *header) {
    char *line;
    size_t size;
    int count = 0;
    while (true) {
        if (getline(&line, &size, file) == -1)
            return -count;
        if (strcmp(line, header) == 0)
            return count;
        count++;
        free(line);
    }
}

ssize_t get_line(char **line, size_t *size) {
    if (*line) {
        free(*line);
        *line = NULL;
        *size = 0;
    }
    return getline(line, size, stdin);
}