/*
 * This program is a clone of xengine which was originally developed
 * by Kazuhiko Shutoh. The original code can be found at
 * http://packages.debian.org/stable/x11/xengine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#ifdef WIN32
#include "winlib.h"
#endif

#include "ezxdisp.h"

#define RAD(x) ((x)*M_PI/180.0)

static const ezx_color_t color_piston = {0.745,0.745,0.745};
static const ezx_color_t color_shaft = {0.125,0.698,0.667};
static const ezx_color_t color_roter = {0.133,0.545,0.133};
static const ezx_color_t color_back = {0.000,0.000,0.000};
static const ezx_color_t color_dep = {1.000,0.000,0.000};
static const ezx_color_t color_pre = {0.000,1.000,1.000};
static const ezx_color_t color_engine = {0.698,0.133,0.133};

static int width, height;
static unsigned long usec_sleep = 10000;

static double get_time(void)
{
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}

static void inline fillrect(ezx_t *e, double x, double y, double w, double h,
			    const ezx_color_t *col)
{
  ezx_fillrect_2d(e, rint(x), rint(y), rint(x+w), rint(y+h), col);
}

static void inline fillarc(ezx_t *e, double x, double y, double w, double h,
			   int angle1, int angle2, const ezx_color_t *col)
{
  ezx_fillarc_2d(e, rint(x+w/2), rint(y+h/2), rint(w), rint(h),
		 angle1, angle2, col);
}

static int draw_engine(ezx_t *e, char *rpm)
{
  static int sw = 1;
  double angle, theta;
  double piston_x1, piston_y1;
  double piston_x2, piston_y2;

  sw ^= 1;

  for (angle = -180; angle < 180; angle += 18) {
    theta = angle * (M_PI / 180.0);
    piston_x1 = sin(theta) * (width * 0.1) + (width / 2);
    piston_y1 = cos(theta) * (height * 0.1) + (height * 0.7);
    piston_x2 = width / 2;
    piston_y2 = piston_y1 - (height * 0.3);

    /* Crear Window */
    ezx_wipe(e);
    
    /* Draw heat sink */
    fillrect(e, width*0.35, height*0.3,  width*0.3, height*0.02, &color_engine);
    fillrect(e, width*0.35, height*0.35, width*0.3, height*0.02, &color_engine);
    fillrect(e, width*0.35, height*0.4,  width*0.3, height*0.02, &color_engine);

    /* Draw Cylinder */
    fillrect(e, width*0.42, height*0.24, width*0.16, height*0.5, &color_engine);
    fillarc(e, piston_x2-width*0.15, height*0.7-height*0.15, width*0.3,
	    height*0.3, 0, 360, &color_engine);
    fillarc(e, piston_x2-width*0.14, height*0.7-height*0.14, width*0.28,
	    height*0.28, 0, 360, &color_back);
    fillrect(e, width*0.43, height*0.25, width*0.14, height*0.5, &color_back);

    /* Draw Gas */
    if (sw != 0) 
      fillrect(e, width*0.435, height*0.25, width*0.13,
	       piston_y2-height*0.05-height*0.26+height*0.1, &color_pre);
    else 
      fillrect(e, width*0.435, height*0.25, width*0.13,
	       piston_y2-height*0.05-height*0.26+height*0.1, &color_dep);

    /* Draw Piston arm */
    ezx_line_2d(e, piston_x1, piston_y1, piston_x2, piston_y2, &color_shaft, 2);
    ezx_line_2d(e, piston_x1, piston_y1, piston_x2, height*0.7, &color_shaft, 2);

    /* Draw Piston & ring */
    fillrect(e, width*0.435, piston_y2-height*0.05, width*0.13, height*0.1,
	     &color_piston);
    fillrect(e, width*0.435, piston_y2-height*0.045, width*0.13, height*0.01,
	     &color_shaft);
    fillarc(e, piston_x2-width*0.015, piston_y2-height*0.015, width*0.03,
	    height*0.03, 0, 360, &color_shaft);

    /* Draw Roter */
    fillarc(e, piston_x2-width*0.1, height*0.7-height*0.1, width*0.2,
	    height*0.2, angle+30, 120, &color_roter);
    fillarc(e, piston_x2-width*0.025, height*0.7-height*0.025, width*0.05,
	    height*0.05, 0, 360, &color_piston);

    /* Display rpm */
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
	 "\t -b \t for benchmarking usage (n=1000, usec=0)\n"
	 , arg0);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  int n, cnt, avgcnt = 25;
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
      avgcnt = 1000;
      usec_sleep = 0;
      break;
    case 'h':
    default:
      usage(argv[0]);
    }
  }
  
  width = height = 200;
  e = ezx_init(width, height, "a clone of xengine");
  ezx_set_background(e, &ezx_black);

  cnt = 0;
  start = get_time();
  rpm[0] = '\0';
  for (;;) {
    if (draw_engine(e, rpm)) break;
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
