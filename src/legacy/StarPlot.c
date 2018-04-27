/* StarPlot.c
*/
#include "StarPlot.h"
#include <GL/glut.h>
#include <assert.h>
#include "novas.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

void StarPlot_drawStars(StarPlot *plot);
//void StarPlot_starList(StarPlot *plot);
void StarPlot_gridList(StarPlot *plot);
void sphereToRect(double ascension, double declination, double * position);

StarPlot * StarPlot_create(StarMap * map) {
	assert(map);
	StarPlot * plot = NULL;
	plot = malloc(sizeof(StarPlot));
	plot->map = map;

	//plot->quadric = gluNewQuadric();
	//gluQuadricDrawStyle(plot->quadric, GLU_LINE);

	//StarPlot_starList(plot);
	StarPlot_gridList(plot);

	return plot;
}

void StarPlot_illustrate(StarPlot * plot) {
	glCallList(plot->gridList);
	StarPlot_drawStars(plot);//glCallList(plot->starList);//StarPlot_drawStars(plot);//
	// TODO render aperature
}

/*void StarPlot_update(StarPlot *plot) {
	glDeleteLists(plot->starList, 1);
	glDeleteLists(plot->gridList, 1);
	StarPlot_starList(plot);
	StarPlot_gridList(plot);
}*/

void StarPlot_drawStars(StarPlot *plot) {
	int n;
	float mag;
	Star * star;

	glPointSize(3.0f);
	glBegin(GL_POINTS);
	for(n=0; n<plot->map->starcount; n++) {
		star = &(plot->map->stars[n]);

		//set the brightness
		mag = (10.0-star->v_mag)/10.0;
		if(mag<0.0) mag = 0.0;
		else if(mag>1) mag = 1.0;
		glColor3f(mag,mag,mag);

		// plot
		glVertex3dv(star->position);
	}
	glEnd();
}

/*void StarPlot_starList(StarPlot *plot) {
	int i;
	Star *star;
	float mag = 0.0;

	plot->starList = glGenLists(1);
	if(plot->starList == 0)
		return;

	//glEnable(GL_POINT_SMOOTH );
	glNewList(plot->starList, GL_COMPILE);

	glPointSize(3.0f);

	glBegin(GL_POINTS);

	glColor3f(1.0,1.0,1.0);
	for(i=0; i<plot->map->starcount; i++) {
		star = &(plot->map->stars[i]);

		//set the brightness
		mag = (10.0-star->v_mag)/10.0;
		if(mag<0.0) mag = 0.0;
		else if(mag>1) mag = 1.0;
		glColor3f(mag,mag,mag);

		/*sphereToRect(
				2.0*PI*star->right_ascension/24.0,
				2.0*PI*star->declination/360.0,
				star->position);* /
		//radec2vector(star->entry.ra, star->entry.dec, 1.0, star->position);
		//radec2vector(star->right_ascension, star->declination, 1.0, star->position);
		glVertex3dv(star->position);
				/*glVertex3d(star->position[0],
				star->position[2],
				star->position[1]);* /
		//gluDisk(plot->quadric, 0, star->entry->visual_magnitude, 6,1);
	}
	glEnd();

	glEndList();
}*/

void StarPlot_gridList(StarPlot *plot) {
	double asc, dec, asc_inc, dec_inc, pos[3];

	plot->gridList = glGenLists(1);
	if(plot->gridList == 0)
		return;

	//glEnable(GL_POINT_SMOOTH );
	glNewList(plot->gridList, GL_COMPILE);

	glLineWidth(0.25f);
	//glColor3f(1.0,1.0,1.0);
	glColor3f(0.2,0.2,0.2);
	// draw slices of right ascension
	asc_inc = PI / 12.0; // increment by hour
	dec_inc = PI / 100;
	for(asc=0; asc<=2.0*PI; asc+=asc_inc) {
		glBegin(GL_LINE_STRIP);
		for(dec=0; dec<=PI/2.0; dec+=dec_inc) {
			sphereToRect(asc,dec,pos);
			glVertex3dv(pos);
		}
		glEnd();
		//for(dec=PI; dec)
	}

	// draw stacks of declination
	asc_inc = PI / 100; // increment by hour
	dec_inc = PI / 18;
	for(dec=0; dec<PI/2.0; dec+=dec_inc) {
		glBegin(GL_LINE_STRIP);
		for(asc=0; asc<=2.0*PI; asc+=asc_inc) {
			sphereToRect(asc,dec,pos);
			glVertex3dv(pos);
		}
		glEnd();
		//for(dec=PI; dec)
	}

	glEndList();
}

void sphereToRect(double ascension, double declination, double * position) {
	/*position[2] = -sin(declination);
	double t = cos(declination);
	position[0] = sin(ascension)*t;
	position[1] = cos(ascension)*t;*/
	position[2] = sin(declination);
	double t = cos(declination);
	position[0] = cos(ascension)*t;
	position[1] = sin(ascension)*t;
}

void StarPlot_destroy(StarPlot *plot) {
	//gluDeleteQuadric(plot->quadric);
}
