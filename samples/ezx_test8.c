#include <stdio.h>
#include <stdlib.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  int quit=0, moving=0, w0=0, h0=0;
  struct rect_s {
    int x0, y0;
    int w, h;
  } rect = {100,100,30,30};

  e = ezx_init(300, 300, "ezxdisp test");
  ezx_select_layer(e, 1);
  ezx_str_2d(e, 5, 15, "Drag-and-drop test.", &ezx_black);
  ezx_str_2d(e, 5, 30, "Press 'q' to exit.", &ezx_black);
  ezx_str_2d(e, 5, 45, "Press 'Home' to initialize.", &ezx_black);
  ezx_select_layer(e, 0);

  while (!quit) {
    ezx_event_t ev;
    
    ezx_wipe_layer(e, 0);
    if (!moving)
      ezx_fillrect_2d(e, rect.x0, rect.y0, rect.x0+rect.w, rect.y0+rect.h,
		      &ezx_blue);
    else
      ezx_rect_2d(e, rect.x0, rect.y0, rect.x0+rect.w, rect.y0+rect.h,
		  &ezx_blue, 1);
    ezx_redraw(e);

    ezx_next_event(e, &ev);
    switch (ev.type) {
    case eet(EZX_BUTTON_PRESS):
      if (ev.button.b == EZX_BUTTON_LEFT &&
	  rect.x0 <= ev.button.x && ev.button.x <= rect.x0+rect.w &&
	  rect.y0 <= ev.button.y && ev.button.y <= rect.y0+rect.h) {
	moving = 1;
	w0 = ev.button.x - rect.x0;
	h0 = ev.button.y - rect.y0;
      }
      break;
    case eet(EZX_BUTTON_RELEASE):
      if (ev.button.b == EZX_BUTTON_LEFT &&
	  rect.x0 <= ev.button.x && ev.button.x <= rect.x0+rect.w &&
	  rect.y0 <= ev.button.y && ev.button.y <= rect.y0+rect.h)
	moving = 0;
      break;
    case eet(EZX_KEY_PRESS):
      if (ev.key.k == EZX_KEY_HOME) {
	rect.x0 = 100;
	rect.y0 = 100;
      } else if (ev.key.k == 'q') quit = 1;
      break;
    case eet(EZX_MOTION_NOTIFY):
      if (moving) {
	rect.x0 = ev.motion.x - w0;
	rect.y0 = ev.motion.y - h0;
      }
      break;
    case eet(EZX_CLOSE):
      quit = 1;
      break;
    default:
      break;
    }
  }
  
  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
