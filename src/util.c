#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/util.h"

double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

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
    (*line)[read] = '\0'; // trim trailing
    return read;
}
