/*
 * StarPlot.h
 *
 *  Created on: Jun 23, 2009
 *      Author: User
 */
#include "StarMap.h"
#include <GL/glut.h>

#ifndef STARPLOT_H_
#define STARPLOT_H_

// TODO make projection configurable
static const int POLAR;
static const int PLANAR;

typedef struct {
	StarMap* map; // the star map to plot
	//GLUquadricObj* quadric;
	GLuint gridList;
	GLuint starList;
} StarPlot;

StarPlot * StarPlot_create(StarMap *map);
void StarPlot_illustrate(StarPlot * plot);
//void StarPlot_update(StarPlot *plot);
void StarPlot_destroy(StarPlot *plot);

#endif /* STARPLOT_H_ */
