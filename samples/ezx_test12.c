#include <stdio.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;
  int i;
  
  e = ezx_init(200, 350, "this is a test");

  for (i = 1; i <= 10; i++)
    ezx_line_2d(e, 30, 5+(i*20), 100, 70+(i*20), &ezx_red, i);

  ezx_redraw(e);

  for (;;) {
    ezx_next_event(e, NULL);
    if (ezx_isclosed(e)) break;
  }

  ezx_quit(e);

  return 0;
}
