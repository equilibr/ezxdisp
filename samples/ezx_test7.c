#include <stdio.h>
#include <stdlib.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  int closed = 0;

  e = ezx_init(300, 300, "This is a test.");
  
  while (!closed) {
    ezx_event_t ev;
    
    ezx_next_event(e, &ev);
    switch (ev.type) {
    case EZX_BUTTON_PRESS:
      fprintf(stderr,"button pressed (b=%d, state=%d)\n",
	      ev.button.b, ev.button.state);
      break;
    case EZX_BUTTON_RELEASE:
      fprintf(stderr,"button released (b=%d, state=%d)\n",
	      ev.button.b, ev.button.state);
      break;
    case EZX_KEY_PRESS:
      fprintf(stderr,"key pressed (k=%d, state=%d)\n",
	      ev.key.k, ev.key.state);
      break;
    case EZX_KEY_RELEASE:
      fprintf(stderr,"key released (k=%d, state=%d)\n",
	      ev.key.k, ev.key.state);
      break;
    case EZX_MOTION_NOTIFY:
      fprintf(stderr,"pointer moved (x=%d, y=%d, state=%d)\n",
	      ev.motion.x, ev.motion.y, ev.motion.state);
      break;
    case EZX_CLOSE:
      fprintf(stderr,"window closed\n");
      closed = 1;
      break;
    }
    fflush(stderr);
  }

  ezx_quit(e);
  
  return 0;
}
