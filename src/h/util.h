//
// Created by Casey Shields on 7/12/2018.
//

#ifndef STARTRACK_UTIL_H
#define STARTRACK_UTIL_H

#include <sys/types.h>

/** Gets an accurate UTC timestamp from the system in seconds since the unix epoch */
double get_time();

/** retrieves the value subsequent to the specified option. If the default_value
 * is supplied, the function will return it. otherwise the method will print an
 * error message and abort. */
char* get_arg( int argc, char *argv[], char *name, char* default_value );

/** Frees any data line is pointing to, then prompts the user, allocates a buffer, and reads the input.
 * Be sure to free the buffer after your last call to get_input! */
ssize_t get_input(char* prompt, char **line, size_t *size );

#endif //STARTRACK_UTIL_H
