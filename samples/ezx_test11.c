#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifdef WIN32
#include "winlib.h"
#endif

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  ezx_point2d_t *points;
  int i, npoints, width, height;
  ezx_color_t color;

  srand(time(NULL));

  width = height = 300;
  e = ezx_init(width, height, "this is a ezx_poly_2d test");

  npoints = 10;
  points = (ezx_point2d_t*) malloc(sizeof(ezx_point2d_t)*npoints);

  for (;;) {
    ezx_wipe(e);
    
    for (i = 0; i < npoints; i++) {
      points[i].x = rand() % width;
      points[i].y = rand() % height;
    }

    color.r = (double)rand()/RAND_MAX;
    color.g = (double)rand()/RAND_MAX;
    color.b = (double)rand()/RAND_MAX;
    
    ezx_poly_2d(e, points, npoints, &color);
    
    ezx_redraw(e);

    usleep(10000);
    
    if (ezx_isclosed(e)) break;
  }

  free(points);
  
  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
