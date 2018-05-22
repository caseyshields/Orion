//
// Created by Casey Shields on 5/3/2018.
//
#include <math.h>
#include "vmath.h"

void scale( double U[3], double k ) {
    U[0] *= k;
    U[1] *= k;
    U[2] *= k;
}

double dot( double U[3], double V[3] ) {
    return U[0]*V[0] + U[1]*V[1] + U[2]*V[2];
}

void cross( double U[3], double V[3], double W[3] ) {
    W[0] = U[1]*V[2] - U[2]*V[1];
    W[1] = U[0]*V[2] - U[2]*V[0];
    W[2] = U[0]*V[1] - U[1]*V[0];
}

/** returns the cartesian length of the vector */
double magnitude( double U[3] ) {
    return sqrt( U[0]*U[0] + U[1]*U[1] + U[2]*U[2] );
}

/** normalizes the given vector and returns the magnitude. */
double normalize( double U[3] ) {
    double m = magnitude(U);
    scale( U, 1.0/m );
    return m;
}

double hours2radians( double h ) { return (h/24.0)*M_PI; }

double degrees2radians( double d ) {return (d/360.0)*M_PI; }

void spherical2cartesian(double theta, double phi, double C[3] ) {
    C[0] = sin(theta)*cos(phi);
    C[1] = sin(theta)*sin(phi);
    C[2] = cos(theta);
}

/** Determines the angle between two vectors which lie on the surface of the sphere, by taking the arcsin of the resulting chord. */
double angular_separation( double theta_1, double phi_1, double theta_2, double phi_2 ) {
    double dX = cos(phi_2) * cos(theta_2) - cos(phi_1) * cos(theta_1);
    double dY = cos(phi_2) * sin(theta_2) - cos(phi_1) * sin(theta_1);
    double dZ = sin(phi_2) - sin(phi_1);
    double C = sqrt( dX*dX + dY*dY + dZ*dZ );
    double dA = 2.0 * asin( C/2.0 );
    return dA;
}

/** Determines the great-circle distance between two points on a sphere. arctan( ||VxU|| / V*U ) */
double orthodromic_distance( double theta_1, double phi_1, double theta_2, double phi_2 ) {
    double U[3] = {0,0,0};
    spherical2cartesian( theta_1, phi_1, U );

    double V[3] = {0,0,0};
    spherical2cartesian( theta_2, phi_2, V );

    double UxV[3] = {0,0,0};
    cross( U, V, UxV );

    double sind = magnitude( UxV );
    double cosd = dot( U, V );

    double angle = atan( sind / cosd );
}
