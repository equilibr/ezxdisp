#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  int b, x, y;
  char str[256];
  
  e = ezx_init(300, 300, "This is a test");

  for (;;) {
    ezx_wipe(e);
    b = ezx_sensebutton(e, &x, &y);
    sprintf(str, "b=%d", b);
    ezx_str_2d(e, 260, 290, str, &ezx_black);
    if (b&EZX_BUTTON_LMASK) ezx_fillcircle_2d(e, x, y, 10, &ezx_red);
    else if (b&EZX_BUTTON_MMASK) ezx_fillcircle_2d(e, x, y, 10, &ezx_yellow);
    else if (b&EZX_BUTTON_RMASK) ezx_fillcircle_2d(e, x, y, 10, &ezx_green);
    else ezx_fillcircle_2d(e, x, y, 10, &ezx_blue);
    if (b&EZX_SHIFT_MASK) ezx_circle_2d(e, x, y, 10, &ezx_black, 3);
    if (b&EZX_CONTROL_MASK) ezx_circle_2d(e, x, y, 6, &ezx_white, 3);
    ezx_redraw(e);
    usleep(1);
  }
  
  return 0;
}
