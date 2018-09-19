/** @file vmath.h
 * @brief A colletion of vector and spherical math routines*/

#ifndef STARTRACK_VMATH_H
#define STARTRACK_VMATH_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define DMS_OUTPUT_FORMAT "% 3d°% 2d'% 6.3lf\""
//"% 2dd % 2dm % 6.3lfs"

#define DMS_INPUT_FORMAT "%d%c%u%c%lf%c"

void scale( double U[3], double k );

double dot( double U[3], double V[3] );

void cross( double U[3], double V[3], double W[3] );

/** returns the cartesian length of the vector */
double magnitude( double U[3] );

/** normalizes the given vector to unit length, and returns the original magnitude. */
double normalize( double U[3] );

/** Determines the angle between two vectors which lie on the surface of the sphere, by taking the arcsin of the resulting chord. */
double angular_separation( double theta_1, double phi_1, double theta_2, double phi_2 );

/** Determines the great-circle distance between two points on a sphere. arctan( ||VxU|| / V*U ) */
double orthodromic_distance( double theta_1, double phi_1, double theta_2, double phi_2 );

double hours2radians( double h );

double degrees2radians( double d );

void spherical2cartesian(double theta, double phi, double C[3] );

double deg2dms( double degrees, int * d, int * m, double * s );
//void degrees2dms(double degrees, int * deg, int * min, double * sec);

/** @param degrees
 * @param minutes
 * @param seconds
 * @return the equivalant angle in decimal degrees*/
double dms2deg( int degrees, int minutes, double seconds );
//double dms2degrees(int degrees, int minutes, double seconds);

char* deg2str(double degrees);////////

char * dms2str(int d, int m, double s);
//char* dms2str(int degrees, int minutes, double seconds);

void str2dms(char * str, int * d, int *m, double * s);

double str2deg(char * str);

// TODO add conversion routines for llh? //W114°58'48.0", N36°03'00.0",     0m

void degrees2hms(double degrees, int * hour, int * min, double * sec);
double hms2degrees(int hours, int minutes, double seconds);
char* hms2str(int hours, int minutes, double seconds);

#endif //STARTRACK_VMATH_H
