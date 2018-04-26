#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "novas.h"
#include "FK6.h"
#include "StarMap.h"
/*include "StarPlot.h"
include <GL/glut.h>*/

#define N_STARS 3
#define N_TIMES 4

int main (void) {
    FILE *file;
    StarMap * map;
//    StarPlot * plot;

   printf("Building star map.");
   file = fopen("../data/FK6.txt", "r");
   assert(file);

   map = StarMap_create();
   FK6_loadMap(878, file, map);
   assert(map);

   StarMap_update(map);
   StarMap_printTime( map );
   StarMap_printSite( map );
   StarMap_printMap( map );

   //   plot = StarPlot_create(map);

   return 1;
}