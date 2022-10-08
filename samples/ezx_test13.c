#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#ifdef WIN32
#include "winlib.h"
#endif

#include "ezxdisp.h"

static int width=300, height=200;

static ezx_color_t *rand_color()
{
  static ezx_color_t color;
  
  color.r = (double)rand()/RAND_MAX/3.0+0.66;
  color.g = (double)rand()/RAND_MAX/3.0+0.66;
  color.b = (double)rand()/RAND_MAX/3.0+0.66;

  return &color;
}

static void draw_triangle(ezx_t *e)
{
  static ezx_point2d_t points[4];
  int x = rand()%width, y = rand()%height, w = rand()%35;

  points[0].x = x;
  points[0].y = y-w;
  points[1].x = x-w*cos(M_PI/6);
  points[1].y = y+w*sin(M_PI/6);
  points[2].x = x+w*cos(M_PI/6);
  points[2].y = y+w*sin(M_PI/6);
  points[3].x = points[0].x;
  points[3].y = points[0].y;

  ezx_poly_2d(e, points, 3, rand_color());
  ezx_lines_2d(e, points, 4, &ezx_black, 1);
}

static void draw_circle(ezx_t *e)
{
  int x = rand()%width, y = rand()%height, r = rand()%25;
  ezx_fillcircle_2d(e, x, y, r, rand_color()); 
  ezx_circle_2d(e, x, y, r, &ezx_black, 1);
}

static void draw_rect(ezx_t *e)
{
  int x = rand()%width, y = rand()%height, w = rand()%40;
  ezx_fillrect_2d(e, x-w/2, y-w/2, x+w/2, y+w/2, rand_color());
  ezx_rect_2d(e, x-w/2, y-w/2, x+w/2, y+w/2, &ezx_black, 1);
}

int main(int argc, char *argv[])
{
  ezx_t *e;
  int i, n=40;

  srand(time(NULL));

  e = ezx_init(width, height, "ezxdisp - yet another simple graphics library");

  for (;;) {
    ezx_wipe(e);

    for (i=0; i<n; i++) {
      draw_triangle(e);
      draw_circle(e);
      draw_rect(e);
    }
    
    ezx_redraw(e);

    usleep(10000);
    
    if (ezx_isclosed(e)) break;
  }

  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
