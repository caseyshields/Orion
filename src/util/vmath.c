#include "util/vmath.h"

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

double magnitude( double U[3] ) {
    return sqrt( U[0]*U[0] + U[1]*U[1] + U[2]*U[2] );
}

double normalize( double U[3] ) {
    double m = magnitude(U);
    scale( U, 1.0/m );
    return m;
}

double angular_separation( double theta_1, double phi_1, double theta_2, double phi_2 ) {
    double dX = cos(phi_2) * cos(theta_2) - cos(phi_1) * cos(theta_1);
    double dY = cos(phi_2) * sin(theta_2) - cos(phi_1) * sin(theta_1);
    double dZ = sin(phi_2) - sin(phi_1);
    double C = sqrt( dX*dX + dY*dY + dZ*dZ );
    double dA = 2.0 * asin( C/2.0 );
    return dA;
}

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

double hours2radians( double h ) { return (h/24.0)*M_PI; }

double degrees2radians( double d ) { return (d/360.0)*M_PI; }

void spherical2cartesian(double theta, double phi, double C[3] ) {
    C[0] = sin(theta)*cos(phi);
    C[1] = sin(theta)*sin(phi);
    C[2] = cos(theta);
}

void deg2dms( double degrees, int *d, int * m, double * s ) {
    double dd, dm, ds;
    ds = modf( degrees, &dd) * 60.0;
    ds = modf( ds, &dm) * 60.0;
    *d = (int)dd;
    *m = (int)dm;
    *s = ds;
}

double dms2deg( int degrees, int minutes, double seconds ) {
    return seconds/3600.0 + minutes/60.0 + degrees;
}

char* deg2str(double degrees) {
    int d, h, m;
    double s;
    deg2dms(degrees, &d, &m, &s);
    return dms2str(d, m, s);
}

char * dms2str(int d, int m, double s) {
    char * str = calloc(32, sizeof(char));
    sprintf(str, DMS_OUTPUT_FORMAT, d, m, s);
    return str;
}

void str2dms(char * str, int * d, int *m, double * s) {
    sscanf( str, DMS_INPUT_FORMAT, d, m, s );
}

double str2deg(char * str) {
    int d, m;
    double s;
    str2dms(str, &d, &m, &s);
    return dms2deg(d, m, s);
}

char* hms2str(int hours, int minutes, double seconds) {
    char * str = calloc(64, sizeof(char));
//    memset(str, 0, sizeof(str));
    sprintf(str, "%d:%d:%1.3lf", hours, minutes, seconds);
    return str;
}

void degrees2hms(double degrees, int * hour, int * min, double * sec) {
    double t, h, m;
    t *= (24.0/360.0);
    t = modf( t, &h );
    t *= 60.0;
    t = modf( t, &m );
    t *= 60.0;

    *hour = (int) h;
    *min = (int) m;
    *sec = t;
}

double hms2degrees(int hours, int minutes, double seconds) {
    double angle = seconds;
    angle /= 60.0;
    angle += minutes;
    angle /= 60.0;
    angle += hours;
    angle *= (360.0/24.0);
    return angle;
}

// tests //////////////////////////////////////////////////////////////////////

void test_angles( CuTest * test ) {
    double count = 5000;
    int d = 0, m = 0;
    double s = 0.0;
    double mas = 1.0/(60.0*60.0*1000);

    for (int n=0; n<count; n++) {
        double angle = ((double)n)*360.0/count;

        // test degrees, minutes, seconds conversion
        deg2dms( angle, &d, &m, &s );
        double deg = dms2deg( d, m, s );
//        if( fabs(angle-deg)>mas )
//            printf(DMS_OUTPUT_FORMAT, d, m, s);
        CuAssertDblEquals_Msg(test, "incorrect decimal degree conversion", angle, deg, mas);

        // test string conversion
        char * str1 = dms2str(d, m, s);
        deg = str2deg(str1);
        CuAssertDblEquals_Msg(test, "incorrect string conversion of DMS", angle, deg, 10*mas);

        char * str2 = deg2str( angle );
        CuAssertStrEquals_Msg(test, "String conversions did not match", str1, str2);

        double angle2 = str2deg( str1 );
        CuAssertDblEquals_Msg( test, "String parse of decimal degrees failed", angle, angle2, mas );

        free(str1);
        free(str2);
    }
}