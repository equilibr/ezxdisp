#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "ezxdisp.h"

#define RAD(x) ((x)*M_PI/180.0)
#define NPOINTS 30

static int width, height;
static double base_x, base_y;
static double space;
static double gear0_r, gear1_r, roter_r, outline_r;
static unsigned long usec_sleep = 10000;
static ezx_point2d_t outline_point[NPOINTS+1];

const ezx_color_t color_outline = {0.4,0.4,0.4};
const ezx_color_t color_roter  = {0.0,0.6,1.0};
const ezx_color_t color_grey35 = {0.25,0.25,0.25};

static double get_time(void)
{
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}

static void init_outline_points()
{
  int i;
  double angle, theta, step = 360.0/NPOINTS;
  
  for (i=0, angle=0; i<NPOINTS; i++, angle+=step) {
    theta = RAD(angle);
    outline_point[i].x = base_x + cos(theta*3)*space + cos(theta)*outline_r;
    outline_point[i].y = base_y + sin(theta*3)*space + sin(theta)*outline_r;
  }
  outline_point[NPOINTS].x = outline_point[0].x;
  outline_point[NPOINTS].y = outline_point[0].y;
}

static void draw_gear(ezx_t *e, double x, double y, double r, double angle0,
		      double step, double alpha, const ezx_color_t *col)
{
  double angle, theta;

  ezx_fillcircle_2d(e, x, y, r, col);
  for (angle = 0; angle < 180; angle += step) {
    theta = RAD(angle0+angle);
    ezx_line_2d(e,
		x+alpha*r*cos(theta), y+alpha*r*sin(theta),
		x-alpha*r*cos(theta), y-alpha*r*sin(theta), col, 5);
  }
}

static int draw_rotary_engine(ezx_t *e, char *rpm)
{
  int angle;
  double x, y, theta;

  for (angle = 0; angle < 360; angle += 4) {
    ezx_wipe(e);
    
    theta = RAD(angle);

    x = base_x + cos(theta*3)*space;
    y = base_y + sin(theta*3)*space;
    
    ezx_fillarc_2d(e, x + cos(theta)*outline_r, y + sin(theta)*outline_r,
		   roter_r*2, roter_r*2, (-angle+150), 60, &color_roter);
    ezx_fillarc_2d(e, x + cos(theta+RAD(120))*outline_r,
		   y + sin(theta+RAD(120))*outline_r, roter_r*2, roter_r*2,
		   (-angle+30), 60, &color_roter);
    ezx_fillarc_2d(e, x + cos(theta+RAD(240))*outline_r,
		   y + sin(theta+RAD(240))*outline_r, roter_r*2, roter_r*2,
		   (-angle+270), 60, &color_roter);
    
    ezx_lines_2d(e, outline_point, NPOINTS+1, &color_outline, 1);
    
    draw_gear(e, x, y, gear1_r, angle, 22.5/3*2, 1.13, &ezx_black);
    draw_gear(e, base_x, base_y, gear0_r, 0.0, 22.5, 1.20, &color_grey35);
    ezx_fillcircle_2d(e, base_x, base_y, gear0_r/3, &ezx_black);
    
    ezx_str_2d(e, width-7*strlen(rpm), height-5, rpm, &ezx_white);
    
    ezx_redraw(e);

    if (usec_sleep) usleep(usec_sleep);
    if (ezx_isclosed(e)) return 1;
  }

  return 0;
}

static void usage(char *arg0)
{
  printf("usage: %s [-a <n>] [-u <usec>] [-b] [-h]\n"
         "\t -a \t calculate an average rpm from n samples\n"
         "\t -u \t set the sleep time in usec\n"
         "\t -b \t for benchmarking usage (n=100, usec=0)\n"
         , arg0);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  int n, cnt, avgcnt = 10;
  double start, end;
  char rpm[256];
  ezx_t *e;

  while ((n = getopt(argc, argv, "a:bu:h")) != EOF) {
    switch (n) {
    case 'a':
      avgcnt = atoi(optarg);
      break;
    case 'u':
      usec_sleep = strtoul(optarg, NULL, 10);
      break;
    case 'b':
      avgcnt = 100;
      usec_sleep = 0;
      break;
    case 'h':
    default:
      usage(argv[0]);
    }
  }

  width = height = 200;
  base_x = width / 2;
  base_y = height / 2;
  space = 9;
  outline_r = 82;
  gear0_r = space * 2;
  gear1_r = gear0_r + space;
  roter_r = outline_r * sin(RAD(60)) / sin(RAD(30));
  init_outline_points();

  e = ezx_init(width, height, "ezx_rotary");
  ezx_set_background(e, &ezx_black);

  cnt = 0;
  start = get_time();
  rpm[0] = '\0';
  for (;;) {
    if (draw_rotary_engine(e, rpm)) break;
    cnt++;
    if (cnt == avgcnt) {
      end = get_time();
      sprintf(rpm, "%4.1f rpm", (cnt*60.0)/(end-start));
      start = end;
      cnt = 0;
    }
  }

  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
