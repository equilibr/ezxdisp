#include <stdio.h>
#include <stdlib.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;

  e = ezx_init(300,300,"this is a test");
  
  ezx_rect_2d(e, 0, 0, 100, 100, &ezx_black, 1);
  ezx_fillrect_2d(e, 0, 0, 100, 100, &ezx_yellow);

  //ezx_rect_2d(e, 100, 100, 200, 200, &ezx_black, 1);
  //ezx_fillrect_2d(e, 100, 100, 200, 200, &ezx_yellow);

  ezx_circle_2d(e, 50,50,50,&ezx_black,1);
  ezx_fillcircle_2d(e, 50,50,50, &ezx_grey75);

  //ezx_fillcircle_2d(e, 150,150,50, &ezx_grey75);

  ezx_arc_2d(e, 150,150,50,70,0,210,&ezx_black,1);
  ezx_fillarc_2d(e, 150,150,50,70,0,210,&ezx_yellow);

  ezx_redraw(e);

  for (;;) {
    ezx_next_event(e, NULL);
  }

  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
