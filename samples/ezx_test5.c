#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  double a;
  ezx_point3d_t points[3];
  
  e = ezx_init(300, 300, "This is a test");

  ezx_line_3d(e, 0, 0, 0, 0, 0, 100, &ezx_black, 1);
  ezx_line_3d(e, 0, 0, 0, 0, 100, 0, &ezx_blue, 1);
  ezx_line_3d(e, 0, 0, 0, 100, 0, 0, &ezx_green, 1);
  ezx_line_3d(e, 0, 100, 0, 100, 0, 0, &ezx_red, 1);
  ezx_line_3d(e, 0, 100, 0, 0, 0, 100, &ezx_yellow, 1);
  ezx_line_3d(e, 100, 0, 0, 0, 0, 100, &ezx_purple, 1);
  
  points[0].x = 100;
  points[0].y = 0;
  points[0].z = 0;
  points[1].x = 0;
  points[1].y = 100;
  points[1].z = 0;
  points[2].x = 0;
  points[2].y = 0;
  points[2].z = 100;
  ezx_poly_3d(e, points, 150,150,150, 3, &ezx_red);

  a = 0.0;
  for (;;) {
    ezx_event_t ev;
    double x, y;

    x=300*cos(a);
    y=300*sin(a);
          
    ezx_set_view_3d(e, x, y, 300, 0, 0, 0, 4);
    ezx_redraw(e);
    
    ezx_next_event(e, &ev);
    switch (ev.type) {
    case eet(EZX_BUTTON_PRESS):
      printf("x=%f, y=%f\n", x, y);
      switch (ev.button.b) {
      case EZX_BUTTON_LEFT:
        a += 0.1;
        break;
      case EZX_BUTTON_RIGHT:
        a -= 0.1;
        break;
      }
    default:
      break;
    }

    ezx_redraw(e);
  }
  
  return 0;
}
