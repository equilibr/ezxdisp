#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  
  e = ezx_init(500, 500, "This is a test");

  ezx_line_2d(e,100,50,300,50,&ezx_blue,1);
  ezx_line_2d(e,150,70,350,70,&ezx_blue,3);
  ezx_str_2d(e,50,50,"hello",&ezx_black);
  ezx_rect_2d(e,50,100,150,200,&ezx_red,1);
  ezx_fillrect_2d(e,100,150,200,250,&ezx_grey75);
  ezx_circle_2d(e,300,200,50,&ezx_green,1);
  ezx_fillcircle_2d(e,350,250,50,&ezx_purple);
  ezx_arc_2d(e,200,350,200,100,90,270,&ezx_cyan,1);
  ezx_fillarc_2d(e,280,400,200,100,90,270,&ezx_pink);

  ezx_redraw(e);

  ezx_pushbutton(e, NULL, NULL);
  
  return 0;
}
