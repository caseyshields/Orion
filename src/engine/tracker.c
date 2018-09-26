#include "engine/tracker.h"

void tracker_create(Tracker *tracker) {

    // clear out direction coordinates
    tracker->azimuth = 0;
    tracker->elevation = 0;
    memset( &(tracker->efg), 0, 3*sizeof(double) );

    // create null novas surface location
    make_on_surface( 0.0, 0.0, 0.0, 10.0, 1010.0, &(tracker->site) );

    // default to empty Earth orientation parameters ?
    tracker->earth = &MISSING_EOP;
}

void tracker_set_location(Tracker *tracker, double latitude, double longitude, double height) {
    tracker->site.latitude = latitude;
    tracker->site.longitude = longitude;
    tracker->site.height = height;
}
void tracker_set_weather(Tracker *tracker, double temperature, double pressure) {
    tracker->site.pressure = pressure;
    tracker->site.temperature = temperature;
}

on_surface tracker_get_location(Tracker *tracker) {
    return tracker->site;
}

IERS_EOP * tracker_get_earth(Tracker * tracker) {
    return tracker->earth;
}

void tracker_set_earth(Tracker * tracker, IERS_EOP * eop) {
    tracker->earth = eop;
}

void tracker_get_topocentric(Tracker * tracker, double * azimuth, double * elevation) {
    *azimuth = tracker->azimuth;
    *elevation = tracker->elevation;
}

void tracker_get_celestial(Tracker * tracker, double * right_ascension, double * declination) {
    *right_ascension = tracker->right_ascension;
    *declination = tracker->declination;
}

void tracker_get_direction(Tracker * tracker, double vec[3]) {
    memcpy(vec, tracker->efg, 3*sizeof(double));
}

int tracker_point(
        Tracker *tracker,
        jday jd_utc,
        cat_entry *target,
        short refraction)
{
    // terrestrial time is needed for computing celestial coordinates, Universal time is needed for eath
    jday jd_tt = utc2tt( jd_utc );
    jday jd_ut1 = iers_get_UT1( tracker->earth, jd_utc );

    // Apply proper motion, parallax, gravitational deflection, relativistic
    // aberration and get the coordinates of the star in the true equator and equinox of date
    short int error;
    error = topo_star(
            jd_tt,
            iers_get_DeltaT( tracker->earth ),
            target, // The FK6 catalog entries I believe are specified in the J2000 epoch
            // thus they should be consistent to within 0.02 arcseconds of the BCRS
            &tracker->site, // The input location is supplied in WGS-84, which is within centimeters of the ITRS axis
            REDUCED_ACCURACY,
            &tracker->right_ascension, // these are specified in term of the current orientations of the equator and ecliptic
            &tracker->declination // that is, they don't make sense unless you have a date, because both of those things are moving...
    );
    if( error )
        return error;

    // Apply refraction and convert Equatorial coordinates to horizon coordinates
    equ2hor(
            jd_ut1,
            iers_get_DeltaT( tracker->earth ),
            REDUCED_ACCURACY,
            tracker->earth->pm_x,
            tracker->earth->pm_y,
            &tracker->site,
            tracker->right_ascension, tracker->declination,
            refraction,
            &(tracker->elevation), &(tracker->azimuth),
            &tracker->right_ascension, &tracker->declination
    );

    // the elevation is actually zenith distance so convert that real quick
    tracker->elevation = 90.0 - tracker->elevation;

    // convert the spherical coordinates to rectilinear
    double equatorial[3];
    // double celestial_sphere = 2<<31; // half TATS resolution?
    radec2vector( tracker->right_ascension, tracker->declination, 1.0, equatorial );

    // now transform the equatorial coordinates into ITRS coordinates
    error = cel2ter(
            jd_ut1, 0, // I do not need/have enough precision to use the low order UT1
            iers_get_DeltaT( tracker->earth ),
            METHOD_EQUINOX,
            REDUCED_ACCURACY,
            OPTION_EQUATOR_AND_EQUINOX_OF_DATE, // only compatible with the equinox method
            tracker->earth->pm_x,
            tracker->earth->pm_y,
            equatorial,
            tracker->efg
    );

    return error;
}

int tracker_zenith(Tracker *tracker, jday jd_tt, double *right_ascension, double *declination) {
    on_surface site = tracker->site;

    // calculate a geocentric earth fixed vector as in Novas C-39
    double lon_rad = site.longitude * DEG2RAD;
    double lat_rad = site.latitude * DEG2RAD;
    double sin_lon = sin(lon_rad);
    double cos_lon = cos(lon_rad);
    double sin_lat = sin(lat_rad);
    double cos_lat = cos(lat_rad);
    double terestrial[3] = {0.0,0.0,0.0};
    terestrial[0] = cos_lat * cos_lon;
    terestrial[1] = cos_lat * sin_lon;
    terestrial[2] = sin_lat;
    //  this is a spherical approximation so it can have up to about 1% error...

    // convert to a celestial vector
    double celestial[3] = {0.0,0.0,0.0};
    int error = ter2cel(
            jd_tt, 0.0,
            iers_get_DeltaT( tracker->earth ),
            METHOD_EQUINOX, // equinox method (0= CIO method)
            REDUCED_ACCURACY,
            OPTION_EQUATOR_AND_EQUINOX_OF_DATE,
            tracker->earth->pm_x,
            tracker->earth->pm_y,
            terestrial,
            celestial
    );
    assert(error==0);

    // convert to spherical celestial components
    error = vector2radec( celestial, right_ascension, declination );
    assert(error==0);
}

void tracker_print_atmosphere(Tracker *tracker, FILE * file) {
    on_surface * site = &(tracker->site);

    // print atmospheric details
    fprintf( file,
            "Atmosphere\n"
            "\ttemperature:\t%5.1lf°C\n"
            "\tpressure:\t%4.3f millibars\n",
            site->temperature, site->pressure );
    fflush(file);
}

void tracker_print_location(Tracker *tracker, FILE * file) {
    int deg = 0, min = 0;
    double sec = 0.0;
    on_surface * site = &(tracker->site);
    fprintf( file, "Location\n" );

    char ew = (char) ((site->longitude>0.0) ? 'E' : 'W');
    deg2dms(site->longitude, &deg, &min, &sec);
    fprintf(file, "\tlongitude:\t%d %d' %lf\" %c\t(%lf)\n",
            abs(deg), abs(min), fabs(sec), ew, site->longitude);

    char ns = (char) ((site->latitude>0.0) ? 'N' : 'S');
    deg2dms(site->latitude, &deg, &min, &sec);
    fprintf(file, "\tlatitude:\t%d %d' %lf\" %c\t(%lf)\n",
            abs(deg), abs(min), fabs(sec), ns, site->latitude);

    fprintf(file, "\theight:\t%lf meters\n");
}

void tracker_print_heading(Tracker * tracker, FILE * file) {
    int deg = 0, min = 0;
    double sec = 0.0;

    fprintf( file, "Tracker Heading\n");

    deg2dms(tracker->right_ascension, &deg, &min, &sec);
    fprintf( file,
            "\tright ascension:\t%d %u' %lf\" (%9.5lf hours)\n",
            deg, min, sec, tracker->right_ascension);

    deg2dms(tracker->declination, &deg, &min, &sec);
    fprintf( file,
             "\tdeclination:\t%d %u' %lf\" (%9.5lf degrees)\n", //°
             deg, min, sec, tracker->declination);

    deg2dms(tracker->azimuth, &deg, &min, &sec);
    fprintf( file,
             "\tazimuth:\t%d %u' %lf\" (%9.5lf degrees)\n",
             deg, min, sec, tracker->azimuth);

    deg2dms(tracker->elevation, &deg, &min, &sec);
    fprintf( file,
             "\televation:\t%d %u' %lf\" (%9.5lf degrees)\n",
             deg, min, sec, tracker->elevation);

    fprintf( file,
             "\tEFG:\t[%lf, %lf, %lf]\n",
            tracker->efg[0], tracker->efg[1], tracker->efg[2]);
}

// test ///////////////////////////////////////////////////////////////////////

#define N_STARS 3
#define N_TIMES 4
/** Adapted from Novas' checkout-stars.c */
void test_novas( CuTest * test ) {
    double answers[N_TIMES][N_STARS][2] = {
            {{2.446989227,89.24635169},{5.530110735,-0.30571717},{10.714525532,-64.38130568}},
            {{2.446989227,89.24635169},{5.530110735,-0.30571717},{10.714525532,-64.38130568}},
            {{2.509479607,89.25196807},{5.531195895,-0.30301953},{10.714444762,-64.37366521}},
            {{2.481178365,89.24254418},{5.530372302,-0.30231627},{10.713575398,-64.37966984}}
    };

    /*
   Main function to check out many parts of NOVAS-C by calling
   function 'topo_star' with version 3 of function 'solarsystem'.

   For use with NOVAS-C Version 3.1.
*/

    short int error = 0;
    short int accuracy = 1;
    short int i, j;

/*
   'deltat' is the difference in time scales, TT - UT1.

    The array 'tjd' contains four selected Julian dates at which the
    star positions will be evaluated.
*/

    double deltat = 60.0;
    double tjd[N_TIMES] = {2450203.5, 2450203.5, 2450417.5, 2450300.5};
    double ra, dec;

/*
   Hipparcos (ICRS) catalog data for three selected stars.
*/

    cat_entry stars[N_STARS] = {
            {"POLARIS", "HIP",   0,  2.530301028,  89.264109444,
                    44.22, -11.75,  7.56, -17.4},
            {"Delta ORI", "HIP", 1,  5.533444639,  -0.299091944,
                    1.67,   0.56,  3.56,  16.0},
            {"Theta CAR", "HIP", 2, 10.715944806, -64.394450000,
                    -18.87, 12.06,  7.43,  24.0}};

/*
   The observer's terrestrial coordinates (latitude, longitude, height).
*/

    on_surface geo_loc = {45.0, -75.0, 0.0, 10.0, 1010.0};

/*
   Compute the topocentric places of the three stars at the four
   selected Julian dates.
*/

    for (i = 0; i < N_TIMES; i++)
    {
        for (j = 0; j < N_STARS; j++)
        {
            error = topo_star (tjd[i],deltat,&stars[j],&geo_loc, accuracy, &ra,&dec);

//                printf ("Error %d from topo_star. Star %d  Time %d\n",
//                        error, j, i);
            CuAssertIntEquals_Msg( test, "topo_star failed", 0, error );

//            printf ("JD = %f  Star = %s\n", tjd[i], stars[j].starname);
//            printf ("RA = %12.9f  Dec = %12.8f\n", ra, dec);
//            printf ("\n");
            CuAssertDblEquals(test, answers[i][j][0], ra, 0.0000001);
            CuAssertDblEquals(test, answers[i][j][1], dec, 0.0000001);
        }
        printf ("\n");
    }
}