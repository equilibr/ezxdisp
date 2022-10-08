#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  int i, width=300, height=150;
  ezx_t *e;
  ezx_color_t grey95 = {0.95,0.95,0.95};

  e = ezx_init(width, height, "This is a test");

  for (i=0; i<width; i+=10)
    ezx_line_2d(e, i, 0, i, height, &grey95, 1);
  for (i=0; i<height; i+=10)
    ezx_line_2d(e, 0, i, width, i, &grey95, 1);

  ezx_str_2d(e, 20, 20, "hello, world!", &ezx_black);

  ezx_rect_2d(e, 20, 30, 50, 60, &ezx_black, 1);
  ezx_fillrect_2d(e, 60, 30, 90, 60, &ezx_black);
  ezx_fillrect_2d(e, 20, 70, 50, 100, &ezx_black);

  ezx_circle_2d(e, 115, 45, 15, &ezx_black, 1);
  ezx_fillcircle_2d(e, 155, 45, 15, &ezx_black);
  ezx_fillcircle_2d(e, 115, 85, 15, &ezx_black);

  ezx_arc_2d(e, 200, 50, 30, 40, 0, 150, &ezx_black, 1);
  ezx_fillarc_2d(e, 230, 50, 30, 40, 0, 150, &ezx_black);
  ezx_fillarc_2d(e, 200, 90, 30, 40, 0, 150, &ezx_black);
  
  ezx_redraw(e);

  ezx_pushbutton(e, NULL, NULL);
  
  ezx_quit(e);

  return 0;
}
