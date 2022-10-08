#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  int t, width=360, height=360, t_step=5;
  double x, y, x_step=0.1, r_step = 1.0/(height/(double)t_step);
  ezx_color_t color = {0,0,0};

  e = ezx_init(width,height,"this is a test");

  for (t=0; t<height; t+=t_step) {
    color.r += r_step;
    for (x=0; x<width; x+=x_step) {
      y = sin(M_PI/180.0*(x+width/2.0)) * (t/2.0) + (height/2.0);
      ezx_point_2d(e, x, y, &color);
    }
  }

  ezx_redraw(e);

  for (;;) {
    ezx_next_event(e, NULL);
    if (ezx_isclosed(e)) break;
  }

  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
