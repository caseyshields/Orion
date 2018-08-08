#ifndef STARTRACK_UTIL_H
#define STARTRACK_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Retrieves the value subsequent to the specified option in the given command line options. If
 * the default_value is supplied, the function will return it. otherwise the method will print an
 * error message and abort.
 * @param argc
 * @param argv
 * @param name
 * @param default_value*/
char* get_arg( int argc, char *argv[], char *name, char* default_value );

/** @param argc
 * @param argv
 * @param name
 * @return True, or 1, when the given tag is included in the argument list, 0 otherwise. */
int has_arg( int argc, char*argv[], char *name );

/** Frees any data 'line' is pointing to, then prompts the user, allocates a buffer, and reads the
 * input, trimming trailing whitespace. Be sure to free the buffer after your last call to get_input!
 * @param prompt
 * @param line
 * @param size */
ssize_t get_input(char* prompt, char **line, size_t *size );

/** Trim the substring and copy it to a newly allocated string, return the size of the resulting
 * string. The indexing conventions match the FK6 Readme idiom, that is; 1-indexed, inclusive.
 * @param line
 * @param start
 * @param end
 * @param dest */
int get_value(const char *line, int start, int end, char *dest);

/** Reads lines from the given file until a line matching the header is reached.
 * @param header A pointer to the line of text we are searching for.
 * @param file Filed handle to the file to be scanned
 * @return returns the number of line read, or a negative number if the line is not found.*/
int scan_line(FILE *file, const char *header);

#endif //STARTRACK_UTIL_H
