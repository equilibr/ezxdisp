#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "ezxdisp.h"

int main(int argc, char *argv[])
{
  int i;
  ezx_t *e;
  double a;

  e = ezx_init(640, 480, "3D clock");

  ezx_set_background(e, &ezx_white);

  ezx_select_layer(e, 3);

  ezx_line_3d(e, 120, 120, 20, -120, 120, 20, &ezx_green, 1);
  ezx_line_3d(e, -120, 120, 20, -120, -120, 20, &ezx_green, 1);
  ezx_line_3d(e, -120, -120, 20, 120, -120, 20, &ezx_green, 1);
  ezx_line_3d(e, 120, -120, 20, 120, 120, 20, &ezx_green, 1);

  ezx_line_3d(e, 120, 120, -20, -120, 120, -20, &ezx_blue, 1);
  ezx_line_3d(e, -120, 120, -20, -120, -120, -20, &ezx_blue, 1);
  ezx_line_3d(e, -120, -120, -20, 120, -120, -20, &ezx_blue, 1);
  ezx_line_3d(e, 120, -120, -20, 120, 120, -20, &ezx_blue, 1);

  for (i = 0; i < 12; i++) {
    double x, y, z;

    x = 100 * cos(2 * M_PI * i / 12 - M_PI / 2);
    y = 100 * sin(2 * M_PI * i / 12 - M_PI / 2);
    z = 0;

    ezx_circle_3d(e, x, y, z, i % 3 == 0 ? 20 : 15,
		  i == 0 ? &ezx_black : &ezx_red);
  }

  ezx_select_layer(e, 4);

  for (a = 0.001; ; a+=0.01) {
    double x, y, z, r;
    time_t ti;
    struct tm t;

    time(&ti);
    t = *localtime(&ti);
    
    ezx_wipe_layer(e, 4);

    x = 40 * cos(-2 * M_PI * (t.tm_hour + t.tm_min * (1 / 60.0)) / 12 - M_PI / 2);
    y = 40 * sin(-2 * M_PI * (t.tm_hour + t.tm_min * (1 / 60.0)) / 12 - M_PI / 2);
    z = 0;

    ezx_line_3d(e, 0, 0, 0, x, y, z, &ezx_black, 6);

    x = 80 * cos(-2 * M_PI * (t.tm_min + t.tm_sec * (1 / 60.0)) / 60 - M_PI / 2);
    y = 80 * sin(-2 * M_PI * (t.tm_min + t.tm_sec * (1 / 60.0)) / 60 - M_PI / 2);
    z = 0;

    ezx_line_3d(e, 0, 0, 0, x, y, z, &ezx_black, 6);

    x = 80 * cos(-2 * M_PI * t.tm_sec / 60 - M_PI / 2);
    y = 80 * sin(-2 * M_PI * t.tm_sec / 60 - M_PI / 2);
    z = 0;

    ezx_line_3d(e, 0, 0, 0, x, y, z, &ezx_black, 1);

    x = sin(a * 0.12);
    y = sin(a * 0.79);
    z = cos(a * 1.02) * 0.5 + 0.5;

    r = sqrt(x * x + y * y + z * z);

    x = x * 400 / r;
    y = y * 400 / r;
    z = z * 400 / r;

    ezx_set_view_3d(e, x, y, z, 0, 0, 0, 5);

    ezx_redraw(e);

    usleep(10000);

    if (ezx_isclosed(e)) break;
  }

  ezx_quit(e);

  exit(EXIT_SUCCESS);
}
