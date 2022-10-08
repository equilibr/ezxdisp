#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  ezx_t *e;

  int city_x[10], city_y[10];
  int mouse_x, mouse_y;
  int x1, x2, y1, y2;
  int i;
  int root[10];
  double distance, new_distance;

  e = ezx_init(900, 400, "This is a tsp test");

  ezx_set_background(e, &ezx_white);

  for (i = 0; i < 10; i++) {
    ezx_pushbutton(e, &mouse_x, &mouse_y);
    x1 = mouse_x;
    y1 = mouse_y;
    city_x[i] = x1;
    city_y[i] = y1;

    ezx_line_2d(e, x1 - 5, y1 - 5, x1 + 5, y1 + 5, &ezx_black, 1);
    ezx_line_2d(e, x1 + 5, y1 - 5, x1 - 5, y1 + 5, &ezx_black, 1);

    ezx_redraw(e);

    root[i] = i;
  }

  distance = 0;

  for (i = 0; i < 9; i++) {
    ezx_line_2d(e, city_x[root[i]], city_y[root[i]],
		city_x[root[i + 1]], city_y[root[i + 1]], &ezx_black,
		1);

    distance +=
      sqrt((pow((city_x[root[i]] - city_x[root[i + 1]]), 2)) +
	   (pow((city_y[root[i]] - city_y[root[i + 1]]), 2)));
  }

  ezx_line_2d(e, city_x[root[i]], city_y[root[i]],
	      city_x[root[0]], city_y[root[0]], &ezx_black, 1);

  distance += sqrt((pow((city_x[root[i]] - city_x[root[0]]), 2)) +
		   (pow((city_y[root[i]] - city_y[root[0]]), 2)));

  ezx_redraw(e);

  while (1) {
    x2 = rand() % 10;
    y2 = rand() % 10;

    i = root[x2];
    root[x2] = root[y2];
    root[y2] = i;
    new_distance = 0;

    for (i = 0; i < 9; i++) {
      new_distance +=
	sqrt((pow((city_x[root[i]] - city_x[root[i + 1]]), 2)) +
	     (pow((city_y[root[i]] - city_y[root[i + 1]]), 2)));
    }

    new_distance +=
      sqrt((pow((city_x[root[i]] - city_x[root[0]]), 2)) +
	   (pow((city_y[root[i]] - city_y[root[0]]), 2)));

    if (new_distance < distance) {
      distance = new_distance;

      ezx_wipe(e);
      for (i = 0; i < 9; i++) {
	ezx_line_2d(e, city_x[root[i]], city_y[root[i]],
		    city_x[root[i + 1]], city_y[root[i + 1]],
		    &ezx_black, 1);
      }

      ezx_line_2d(e, city_x[root[i]], city_y[root[i]],
		  city_x[root[0]], city_y[root[0]], &ezx_black, 1);
      ezx_redraw(e);

      ezx_pushbutton(e, NULL, NULL);
    } else {
      i = root[x2];
      root[x2] = root[y2];
      root[y2] = i;
    }

    i = ezx_pushbutton(e, NULL, NULL);
    if (i == 3) break;
  }

  ezx_quit(e);

  return 0;
}
