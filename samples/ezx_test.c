#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  ezx_event_t event;
  double a;
  int quit;

  e = ezx_init(900, 400, "This is a test");
  ezx_set_background(e, &ezx_white);
  ezx_select_layer(e, 3);

  ezx_rect_2d(e, 0, 0, 900, 400, &ezx_black, 1);
  ezx_line_2d(e, 0, 0, 767, 511, &ezx_black, 0);
  ezx_line_2d(e, 0, 511, 767, 0, &ezx_black, 10);
  ezx_circle_2d(e, 384, 256, 20, &ezx_red, 1);
  ezx_str_2d(e, 100, 100, "This is a 2d string.", &ezx_red);
  ezx_fillrect_2d(e, 100, 100, 300, 300, &ezx_brown);

  ezx_line_3d(e, 0, 0, 0, 0, 0, 100, &ezx_black, 1);
  ezx_line_3d(e, 0, 0, 0, 0, 100, 0, &ezx_blue, 2);
  ezx_line_3d(e, 0, 0, 0, 100, 0, 0, &ezx_green, 3);
  ezx_line_3d(e, 0, 100, 0, 100, 0, 0, &ezx_red, 4);
  ezx_line_3d(e, 0, 100, 0, 0, 0, 100, &ezx_yellow, 5);
  ezx_line_3d(e, 100, 0, 0, 0, 0, 100, &ezx_purple, 6);
  ezx_str_3d(e, 0, 0, 0, "This is a 3d string.", &ezx_black);
  ezx_str_3d(e, 100, 0, 0, "X", &ezx_black);
  ezx_str_3d(e, 0, 100, 0, "Y", &ezx_black);
  ezx_str_3d(e, 0, 0, 100, "Z", &ezx_black);

  for (quit=0, a=0; !quit;) {
    char str[1024];

    ezx_set_view_3d(e, 400 * cos(a), 400 * sin(a), 200, 0, 0, 0, 5);
    ezx_redraw(e);

    ezx_next_event(e, &event);
    switch (event.type) {
    case EZX_BUTTON_PRESS:
      switch (event.button.b) {
      case EZX_BUTTON_LEFT:
	a += 0.1;
	break;
      case EZX_BUTTON_RIGHT:
	a -= 0.1;
	break;
      }
      ezx_select_layer(e, 2);
      ezx_wipe_layer(e, 2);
      ezx_fillcircle_2d(e, event.button.x, event.button.y, 10, &ezx_blue);
      sprintf(str, "%g", a);
      ezx_window_name(e, str);
      break;
    case EZX_KEY_PRESS:
      if (event.key.k != 'q') break;
    case EZX_CLOSE:
      quit = 1;
    default:
      break;
    }
  }
  
  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
