#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

char* get_arg( int argc, char *argv[], char *name, char* default_value ) {
    for( int n=0; n<argc; n++ )
        if( !strcmp(name, argv[n]) )
            return argv[n+1];
    if( default_value ) {
        printf("Parameter '%s' defaulting to '%s'\n", name, default_value);
        fflush(stdout);
        return default_value;
    }
    sprintf( "Error: missing parameter '%s'\n", name );
    exit( 1 );
}

ssize_t get_input(char* prompt, char **line, size_t *size ) {
    if( *line ) {
        free( *line );
        *line = NULL;
        *size = 0;
    }
    printf("%s : ", prompt);
    fflush( stdout );
    ssize_t read = getline( line, size, stdin );

    if( read == -1 ) {
        printf("Error: input stream closed");
        exit( 2 );
    }

    // trim trailing whitespace.
    while (read>0 &&
           ((*line)[read-1]=='\r' || (*line)[read-1]=='\n' || (*line)[read-1]==' ') )
        (*line)[--read] = '\0'; // trim trailing
    // Apparently getline's return value seems to differ by one between my test compilers. Fun.

    return read;
}

int scan_line(FILE *file, const char *header) {
    int count = 0;
    while (1) {
        char *line = NULL;
        size_t size = 0;
        if (getline(&line, &size, file) == -1)
            return -count;
        int len = strlen(header);
        if( len<=strlen(line) && strncmp(line, header, len) == 0)
            return count;
        count++;
        free(line);
    }
}

int get_value(const char *line, int start, int end, char *dest) {
    // trim leading whitspace
    while (start < end)
        if (line[start] == ' ' || line[start] == '\n' || line [start] == '\r')
            start++;
        else break;

    // trim trailing whitespace
    while (start < end)
        if (line[end-1] == ' ' || line[end-1] == '\n' || line [start] == '\r')
            end--;
        else break;

    // copy characters over
    int len = end - start;
    for (int n = 0; n < len; n++)
        dest[n] = line[n + start];

    // terminate the c-style string
    dest[len] = '\0';

    // return the number of characters copied
    return len;
}