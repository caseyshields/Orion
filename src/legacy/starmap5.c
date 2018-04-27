#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "StarPlot.h"
#include "novas.h"
#include "Heap.h"
#include "FK6.h"
#include <GL/glut.h>

#define N_STARS 3
#define N_TIMES 4

#ifndef PI
#define PI 3.14159265358979323846
#endif

void keyboard(unsigned char key, int x, int y);
void display(void);
void reshape(int w, int h);
int createDisplayList(StarMap *map, int selected);

StarMap *map = NULL;
StarPlot *plot = NULL;

float ascension=0.0f, declination=0.0f;
float inc = 2.5;//PI/10.0f;
int mode = 1;

int main (int argc, char** argv) {
	FILE *file;
	//double TT_UT1 = 66.2; // obtained from http://maia.usno.navy.mil/ser7/deltat.preds
	//int error;

	glutInit(&argc, argv);
	glutInitWindowSize(600,600);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);//|GLUT_DEPTH);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutCreateWindow("GLUT Test");
	glutKeyboardFunc(&keyboard);
	//glutMouseFunc(int, int, int, int)
	//glutMotionFunc(int, int)
	//void glutKeyboardUpFunc(void (*func)(unsigned char key,int x,int y));
	//void glutSpecialUpFunc(void (*func)(int key,int x, int y));
	glutSpecialFunc(&keyboard);
	glutDisplayFunc(&display);
	glutReshapeFunc(&reshape);
	glutIdleFunc(&display);
	//glEnable(GL_POINT_SMOOTH);
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_DEPTH_TEST);

	// allocate map
	map = StarMap_create();

	// parse FK6
	file = fopen("C:/dev/starmap3/data/FK6.txt", "r");
	FK6_loadMap(878, file, map);
	printf("\tDone.\n");

	// initialize map
	//StarMap_setSite(map, &capitol);
	//StarMap_setTime(map, 2011, 1, 22, 10.0);
	//StarMap_setAperture(map, 0.0, 90.0, 10.0);
	StarMap_update(map);
	StarMap_printMap(map);

	// initialize plot
	plot = StarPlot_create(map);

	glutMainLoop();

	printf("Closing file.\n");fflush(0);
	fclose(file);

	printf("Freeing plot.\n");fflush(0);
	if(map)StarMap_destroy(map);
	//if(view)StarMap_destroy(view);

	printf("\tDone.\n");

	return EXIT_SUCCESS;
}

void keyboard(unsigned char key, int x, int y) {
	//printf("asc:%f, dec:%f",ascension, declination);
	switch (key) {
	case GLUT_KEY_LEFT:
		//ascension+=inc;
		map->date -= 100.0;//(1.0/24.0);
		StarMap_update(map);
		//StarPlot_update(plot);
		StarMap_printTime(map);

		break;
	case GLUT_KEY_RIGHT:
		//ascension-=inc;
		map->date += 90.0;//(1.0/24.0);
		StarMap_update(map);
		//StarPlot_update(plot);
		StarMap_printTime(map);
		break;
	case GLUT_KEY_DOWN:
		declination+=inc;
		break;
	case GLUT_KEY_UP:
		declination-=inc;
		break;
    case '\x1B':
    // TODO quit the game loop, not the app!
      //exit(EXIT_SUCCESS);
    	mode = 0;
      break;
  }
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	//glRotatef(anglex,1.0,0.0,0.0);
	//glRotatef(angley,0.0, 1.0, 0.0);
	glRotatef(ascension,0.0, 0.0, 1.0);
	glRotatef(declination, cos(PI*ascension/180.0), sin(PI*ascension/180.0), 0.0);
	StarPlot_illustrate(plot);
	glPopMatrix();
	glutSwapBuffers();
	glFinish();
}

void reshape(int w, int h) {
	if(h == 0)
		h = 1;
	float ratio = 1.0* w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	gluPerspective(45,ratio,3,5);//0.1,3);
	// Set the correct perspective.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,0.0,3.0,
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);
}



//glutWireSphere();

/*
GL_POINTS : individual points
GL_LINES : pairs of vertices interpreted as individual line segments
GL_LINE_STRIP : series of connected line segments
GL_LINE_LOOP : same as above, with a segment added between last and first vertices
GL_TRIANGLES : triples of vertices interpreted as triangles
GL_TRIANGLE_STRIP : linked strip of triangles
GL_TRIANGLE_FAN : linked fan of triangles
GL_QUADS : quadruples of vertices interpreted as four-sided polygons
GL_QUAD_STRIP : linked strip of quadrilaterals
GL_POLYGON : boundary of a simple, convex polygon
*/
