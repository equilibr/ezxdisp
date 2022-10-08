#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef WIN32
#include "winlib.h"
#endif

#include "ezxdisp.h"

void alpha_blending(const ezx_color_t *s0, const ezx_color_t *s1, double alpha,
		    ezx_color_t *d)
{
  if (alpha < 1.0) {
    d->r = (s0->r - s1->r) * alpha + s1->r;
    d->g = (s0->g - s1->g) * alpha + s1->g;
    d->b = (s0->b - s1->b) * alpha + s1->b;
  }
}

int main(int argc, char *argv[])
{
  ezx_t *e;
  int x0=50, y0=50, x1=250, y1=250;
  int d=0;
  ezx_color_t ec0, ec1;
  double alpha=0.0;

  e = ezx_init(500, 500, "test");
  ezx_set_background(e, &ezx_white);

  while (alpha < 1.0) {
    ezx_wipe(e);
    ezx_fillrect_2d(e, x0, y0, x1, y1, &ezx_red);
    alpha_blending(&ezx_white, &ezx_blue, alpha, &ec0);
    ezx_fillrect_2d(e, x0+d, y0+d, x1+d, y1+d, &ec0);
    alpha_blending(&ezx_red, &ezx_blue, alpha, &ec1);
    ezx_fillrect_2d(e, x0+d, y0+d, x1, y1, &ec1);
    ezx_redraw(e);
    alpha += 0.005;
    d += 1;
    usleep(30000);
  }
  
  ezx_pushbutton(e, NULL, NULL);
  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
