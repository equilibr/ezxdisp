#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ezxdisp.h"

#define flat(p) p.x,p.y,p.z

int main(int argc, char *argv[])
{
  ezx_t *e;
  double a,b,l=50;
  ezx_point3d_t p[4] = {
    {-l, 0, -l/sqrt(2)},
    {+l, 0, -l/sqrt(2)},
    {0, -l, l/sqrt(2)},
    {0, l, l/sqrt(2)}
  };
  ezx_point3d_t f[4][3] = {
    {p[0],p[1],p[2]},
    {p[0],p[1],p[3]},
    {p[0],p[2],p[3]},
    {p[1],p[2],p[3]}
  };
  
  e = ezx_init(300, 300, "This is a test");

  ezx_line_3d(e, flat(p[0]), flat(p[3]), &ezx_black, 1);
  ezx_line_3d(e, flat(p[0]), flat(p[2]), &ezx_black, 1);
  ezx_line_3d(e, flat(p[0]), flat(p[1]), &ezx_black, 1);
  ezx_line_3d(e, flat(p[1]), flat(p[2]), &ezx_black, 1);
  ezx_line_3d(e, flat(p[1]), flat(p[3]), &ezx_black, 1);
  ezx_line_3d(e, flat(p[2]), flat(p[3]), &ezx_black, 1);

  ezx_poly_3d(e, f[0], 3*l,3*l,3*l, 3, &ezx_cyan);
  ezx_poly_3d(e, f[1], 3*l,3*l,3*l, 3, &ezx_blue);
  ezx_poly_3d(e, f[2], 3*l,3*l,3*l, 3, &ezx_green);
  ezx_poly_3d(e, f[3], 3*l,3*l,3*l, 3, &ezx_red);

  a = 0.0;
  b = 0.0;
  for (;;) {
    ezx_event_t ev;
    double x, y, z;

    x=6*l*cos(a)*cos(b);
    y=6*l*sin(a)*cos(b);
    z=6*l*sin(b);
          
    ezx_set_view_3d(e, x, y, z, 0, 0, 0, 4);
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
      case EZX_BUTTON_MIDDLE:
        b += 0.1;
        break;
      }
    default:
      break;
    }

    ezx_redraw(e);
  }
  
  return 0;
}
