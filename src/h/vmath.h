#ifndef STARTRACK_VMATH_H
#define STARTRACK_VMATH_H

void scale( double U[3], double k );

double dot( double U[3], double V[3] );

void cross( double U[3], double V[3], double W[3] );

/** returns the cartesian length of the vector */
double magnitude( double U[3] );

/** normalizes the given vector to unit length, and returns the original magnitude. */
double normalize( double U[3] );

double hours2radians( double h );

double degrees2radians( double d );

void spherical2cartesian(double theta, double phi, double C[3] );

void degrees2hms(double degrees, int * hour, int * min, double * sec);

void degrees2dms(double degrees, int * deg, int * min, double * sec);

double dms2degrees(int degrees, int minutes, double seconds);

double hms2degrees(int hours, int minutes, double seconds);

char* degrees2str(double degrees);

char* dms2str(int degrees, int minutes, double seconds);

char* hms2str(int hours, int minutes, double seconds);

/** Determines the angle between two vectors which lie on the surface of the sphere, by taking the arcsin of the resulting chord. */
double angular_separation( double theta_1, double phi_1, double theta_2, double phi_2 );

/** Determines the great-circle distance between two points on a sphere. arctan( ||VxU|| / V*U ) */
double orthodromic_distance( double theta_1, double phi_1, double theta_2, double phi_2 );

#endif //STARTRACK_VMATH_H
