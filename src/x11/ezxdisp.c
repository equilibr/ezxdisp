/*
 * x11/ezxdisp.c
 * This file is part of the ezxdisp library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "ezxdisp.h"

struct ezx_s {
  int size_x, size_y;
  ezx_color_t bgcolor;
  int closed;

  Display     *display;
  Window       top;
  XFontStruct *fontst;
  GC           gc;
  Colormap     cmap;
  XColor       black, white, rgb;
  Pixmap       pixmap;
  XSizeHints   size_hints;
  Atom         wm_protocols, wm_delete_window;

  int                 cur_layer;
  struct ezx_figures *fig_head[EZX_NLAYER];
  struct ezx_figures *fig_tail[EZX_NLAYER];

  // 3d stuffs
  double scrdist, mag;
  double eye_x, eye_y, eye_z, eye_r;
  double cmat[3][3];
  double light_x, light_y, light_z;

  struct color_table_s *color_table;
};

typedef struct ezx_figures {
  enum {
    FPOINT2D,
    FLINE2D,
    FLINES2D,
    FLINE3D,
    FPOLY2D,
    FPOLY3D,
    FSTR2D,
    FSTR3D,
    FRECT2D,
    FFILLEDRECT2D,
    FCIRCLE2D,
    FFILLEDCIRCLE2D,
    FFILLEDCIRCLE3D,
    FARC2D,
    FFILLEDARC2D,
  } type;

  int x0, y0, z0, x1, y1, z1, r;
  double dx0, dy0, dz0, dx1, dy1, dz1, dr;
  double angle1, angle2;
  char *str;

  int npoints;
  ezx_point2d_t *points_2d;
  ezx_point3d_t *points_3d;

  ezx_color_t         col;
  int                 width;
  struct ezx_figures *next;
} ezx_figures;

typedef struct ezx_pfigures {
  double               z;
  struct ezx_figures  *fig;
  struct ezx_pfigures *next;
} ezx_pfigures;

typedef struct color_table_entry_s {
  unsigned short red, green, blue;
  XColor *data;
} color_table_entry_t;

typedef struct color_table_s {
  int size;
  int maxsize;
  struct color_table_entry_s *entry;
} color_table_t;

const ezx_color_t ezx_black  = {0, 0, 0};
const ezx_color_t ezx_white  = {1, 1, 1};
const ezx_color_t ezx_grey25 = {0.25, 0.25, 0.25};
const ezx_color_t ezx_grey50 = {0.5, 0.5, 0.5};
const ezx_color_t ezx_grey75 = {0.75, 0.75, 0.75};
const ezx_color_t ezx_blue   = {0, 0, 1};
const ezx_color_t ezx_red    = {1, 0, 0};
const ezx_color_t ezx_green  = {0, 1, 0};
const ezx_color_t ezx_yellow = {1, 1, 0};
const ezx_color_t ezx_purple = {1, 0, 1};
const ezx_color_t ezx_pink   = {1, 0.5, 0.5};
const ezx_color_t ezx_cyan   = {0.5, 0.5, 1};
const ezx_color_t ezx_brown  = {0.5, 0, 0};
const ezx_color_t ezx_orange = {1, 0.5, 0};

static const char * const fontname = "fixed";

static void error_exit(const char *fmt, ...)
{
  va_list params;

  fprintf(stderr, "ezxdisp: ");
  va_start(params, fmt);
  vfprintf(stderr, fmt, params);
  va_end(params);
  fprintf(stderr, "\n");
  fflush(stderr);
  exit(EXIT_FAILURE);
}

static void sys_error_exit(const char *fmt, ...)
{
  int errno_save = errno;
  va_list params;

  fprintf(stderr, "ezxdisp: ");
  va_start(params, fmt);
  vfprintf(stderr, fmt, params);
  va_end(params);
  fprintf(stderr, ": %s\n", strerror(errno_save));
  fflush(stderr);
  exit(EXIT_FAILURE);
}

static inline void *xmalloc(size_t n)
{
  void *p;

  p = malloc(n);
  if (!p) sys_error_exit("malloc failed");
  
  return p;
}

static inline void *xcalloc(size_t n, size_t s)
{
  void *p;

  p = calloc(n, s);
  if (!p) sys_error_exit("calloc failed");

  return p;
}

static color_table_t *color_table_new()
{
  int i;
  color_table_t *table;

  table = xmalloc(sizeof(color_table_t));
  table->size = 0;
  table->maxsize = 8191;
  //table->maxsize = 32749;
  table->entry = xmalloc(sizeof(color_table_entry_t) * table->maxsize);
  for (i = 0; i < table->maxsize; i++)
    table->entry[i].data = NULL;

  return table;
}

static int color_hash(unsigned short red, unsigned short green,
		      unsigned short blue, int maxsize)
{
  long long hash = ((long long)red << 32) |
    ((long long)green << 16) | ((long long)blue);
  return hash % maxsize;
}

static void color_table_insert(color_table_t *table, unsigned short red,
			       unsigned short green, unsigned short blue,
			       Display *display, XColor *col)
{
  int i = color_hash(red, green, blue, table->maxsize);

  if (table->size == table->maxsize) {
    XFreeColors(display, DefaultColormap(display, DefaultScreen(display)),
		&(table->entry[i].data->pixel), 1, 0);
    free(table->entry[i].data);
    table->entry[i].data = NULL;
    table->size--;
  }
  
  while (table->entry[i].data) i = (i+1) % table->maxsize;
  table->entry[i].red = red;
  table->entry[i].green = green;
  table->entry[i].blue = blue;
  table->entry[i].data = col;
  table->size++;
}

static XColor *color_table_search(color_table_t *table, unsigned short red,
				  unsigned short green, unsigned short blue)
{
  int h, i;

  h = i = color_hash(red, green, blue, table->maxsize);

  while (table->entry[i].data) {
    if (table->entry[i].red == red &&
	table->entry[i].green == green &&
	table->entry[i].blue == blue)
      return table->entry[i].data;
    else i = (i+1) % table->maxsize;
    
    if (h == i) break;
  }

  return NULL;
}

static void color_table_free(color_table_t *table)
{
  int i;
  
  for (i = 0; i < table->maxsize; i++)
    if (table->entry[i].data) free(table->entry[i].data);
  free(table->entry);
  free(table);
}

static void set_fgcolor(ezx_t *e, const ezx_color_t *col)
{
  XColor lc={0}, *c;
  
  lc.red = (unsigned short) (col->r * 65535);
  lc.green = (unsigned short) (col->g * 65535);
  lc.blue = (unsigned short) (col->b * 65535);
  lc.flags = DoRed | DoGreen | DoBlue;
  
  c = color_table_search(e->color_table, lc.red, lc.green, lc.blue);
  if (!c) {
    c = xmalloc(sizeof(XColor));
    *c = lc;
    XAllocColor(e->display, e->cmap, c);
    color_table_insert(e->color_table, lc.red, lc.green, lc.blue, e->display, c);
  }
  
  XSetForeground(e->display, e->gc, c->pixel);
}

void ezx_wipe(ezx_t *e)
{
  int i;
  ezx_figures *f, *nf;

  for (i = 0; i < EZX_NLAYER; i++) {
    for (f = e->fig_head[i]; f != NULL; f = nf) {
      nf = f->next;
      if (f->type == FSTR3D) free(f->str);
      if (f->type == FSTR2D) free(f->str);
      free(f);
    }

    e->fig_head[i] = NULL;
    e->fig_tail[i] = NULL;
  }
}

void ezx_wipe_layer(ezx_t *e, int lay)
{
  ezx_figures *f, *nf;

  if (lay < 0 || EZX_NLAYER <= lay)
    error_exit("ezx_wipe_layer: invalid layer number %d", lay);
  
  for (f = e->fig_head[lay]; f != NULL; f = nf) {
    nf = f->next;
    if (f->type == FSTR3D) free(f->str);
    if (f->type == FSTR2D) free(f->str);
    free(f);
  }

  e->fig_head[lay] = NULL;
  e->fig_tail[lay] = NULL;
}

void ezx_select_layer(ezx_t *e, int lay)
{
  if (lay < 0 || EZX_NLAYER <= lay)
    error_exit("ezx_select_layer: invalid layer number %d", lay);

  e->cur_layer = lay;
}

void ezx_set_light_3d(ezx_t *e, double ex, double ey, double ez)
{
  double s = sqrt(ex * ex + ey * ey + ez * ez);

  if (s != 0) {
    e->light_x = ex / s;
    e->light_y = ey / s;
    e->light_z = ez / s;
  }
}

void ezx_set_view_3d(ezx_t *e, double ex, double ey, double ez, double vx,
		     double vy, double vz, double m)
{
  double x = vx - ex, y = vy - ey, z = vz - ez;
  double theta = atan2(y, x);
  double tmp = x * x + y * y;
  double phi;
  double st, ct, sp, cp;

  e->mag = m;

  e->eye_x = ex;
  e->eye_y = ey;
  e->eye_z = ez;
  e->eye_r = sqrt(x * x + y * y + z * z);

  if (tmp == 0) phi = M_PI / 2;
  else phi = acos(tmp / (sqrt(tmp) * e->eye_r));

  if (z < 0) phi = -phi;

  st = sin(theta);
  ct = cos(theta);
  sp = sin(phi);
  cp = cos(phi);

  e->cmat[0][0] = -st;
  e->cmat[0][1] = -ct * sp;
  e->cmat[0][2] = ct * cp;
  e->cmat[1][0] = ct;
  e->cmat[1][1] = -st * sp;
  e->cmat[1][2] = st * cp;
  e->cmat[2][0] = 0;
  e->cmat[2][1] = cp;
  e->cmat[2][2] = sp;
}

static double getz(ezx_t *e, double sx, double sy, double sz)
{
  sx -= e->eye_x;
  sy -= e->eye_y;
  sz -= e->eye_z;

  return sx * e->cmat[0][2] + sy * e->cmat[1][2] + sz * e->cmat[2][2];
}

void ezx_c3d_to_2d(ezx_t *e, double sx, double sy, double sz, double *dx,
		   double *dy)
{
  double x2, y2, z2, rz;

  sx -= e->eye_x;
  sy -= e->eye_y;
  sz -= e->eye_z;

  x2 = sx * e->cmat[0][0] + sy * e->cmat[1][0] + sz * e->cmat[2][0];
  y2 = sx * e->cmat[0][1] + sy * e->cmat[1][1] + sz * e->cmat[2][1];
  z2 = sx * e->cmat[0][2] + sy * e->cmat[1][2] + sz * e->cmat[2][2];

  rz = e->scrdist - z2;
  *dx = e->mag * e->scrdist * x2 / rz;
  *dy = e->mag * e->scrdist * y2 / rz;
}

static void clip_line(int *x0, int *y0, int *x1, int *y1, int width, int height)
{
  if (*x0 > *x1) {
    int t;
    t = *x0;
    *x0 = *x1;
    *x1 = t;
    t = *y0;
    *y0 = *y1;
    *y1 = t;
  }

  if (*x0 < 0 && *x1 >= 0) {
    *y0 = *y1 + (*y0 - *y1) * (0 - *x1) / (*x0 - *x1);
    *x0 = 0;
  }
  if (*x1 >= width && *x0 < width) {
    *y1 = *y0 + (*y1 - *y0) * (width - *x0) / (*x1 - *x0);
    *x1 = width;
  }

  if (*y0 > *y1) {
    int t;
    t = *x0;
    *x0 = *x1;
    *x1 = t;
    t = *y0;
    *y0 = *y1;
    *y1 = t;
  }

  if (*y0 < 0 && *y1 >= 0) {
    *x0 = *x1 + (*x0 - *x1) * (0 - *y1) / (*y0 - *y1);
    *y0 = 0;
  }
  if (*y1 >= height && *y0 < height) {
    *x1 = *x0 + (*x1 - *x0) * (height - *y0) / (*y1 - *y0);
    *y1 = height;
  }
}

static void figure_list_add_tail(ezx_t *e, ezx_figures *nf)
{
  int lay = e->cur_layer;
  
  if (e->fig_head[lay] == NULL)
    e->fig_head[lay] = e->fig_tail[lay] = nf;
  else {
    e->fig_tail[lay]->next = nf;
    e->fig_tail[lay] = nf;
  }
}

void ezx_point_2d(ezx_t *e, int x, int y, const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FPOINT2D;
  nf->x0 = x;
  nf->y0 = y;
  nf->col = *col;

  figure_list_add_tail(e, nf);
}

void ezx_line_2d(ezx_t *e, int x0, int y0, int x1, int y1, const ezx_color_t *col,
		 int width)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FLINE2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->x1 = x1;
  nf->y1 = y1;
  nf->col = *col;
  nf->width = width;
  
  figure_list_add_tail(e, nf);
}

void ezx_lines_2d(ezx_t *e, ezx_point2d_t *points, int npoints,
		  const ezx_color_t *col, int width)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FLINES2D;
  nf->points_2d = points;
  nf->npoints = npoints;
  nf->col = *col;
  nf->width = width;
  
  figure_list_add_tail(e, nf);
}

void ezx_poly_2d(ezx_t *e,ezx_point2d_t *points, int npoints,
		 const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FPOLY2D;
  nf->points_2d = points;
  nf->npoints = npoints;
  nf->col = *col;
  
  figure_list_add_tail(e, nf);
}

void ezx_arc_2d(ezx_t *e, int x0, int y0, int w, int h, double angle1,
		double angle2, const ezx_color_t *col, int width)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FARC2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->x1 = w;
  nf->y1 = h;
  nf->angle1 = angle1;
  nf->angle2 = angle2;
  nf->col = *col;
  nf->width = width;

  figure_list_add_tail(e, nf);
}

void ezx_fillarc_2d(ezx_t *e, int x0, int y0, int w, int h, double angle1,
		    double angle2, const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FFILLEDARC2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->x1 = w;
  nf->y1 = h;
  nf->angle1 = angle1;
  nf->angle2 = angle2;
  nf->col = *col;
  nf->width = 0;

  figure_list_add_tail(e, nf);
}

void ezx_line_3d(ezx_t *e, double x0, double y0, double z0, double x1,
		 double y1, double z1, const ezx_color_t *col, int width)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FLINE3D;
  nf->dx0 = x0;
  nf->dy0 = y0;
  nf->dz0 = z0;
  nf->dx1 = x1;
  nf->dy1 = y1;
  nf->dz1 = z1;
  nf->col = *col;
  nf->width = width;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_str_3d(ezx_t *e, double x0, double y0, double z0, char *str,
		const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FSTR3D;
  nf->dx0 = x0;
  nf->dy0 = y0;
  nf->dz0 = z0;
  nf->str = xcalloc(strlen(str) + 1, sizeof(char));
  strcpy(nf->str, str);
  nf->col = *col;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_str_2d(ezx_t *e, int x0, int y0, char *str, const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FSTR2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->str = xcalloc(strlen(str) + 1, sizeof(char));
  strcpy(nf->str, str);
  nf->col = *col;
  nf->width = 0;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_fillrect_2d(ezx_t *e, int x0, int y0, int x1, int y1,
		     const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FFILLEDRECT2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->x1 = x1;
  nf->y1 = y1;
  nf->col = *col;
  nf->width = 0;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_rect_2d(ezx_t *e, int x0, int y0, int x1, int y1, const ezx_color_t *col,
		 int width)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FRECT2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->x1 = x1;
  nf->y1 = y1;
  nf->col = *col;
  nf->width = width;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_poly_3d(ezx_t *e, ezx_point3d_t *points, double hx, double hy,
		 double hz, int npoints, const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));
  int i;

  nf->type = FPOLY3D;
  nf->points_3d = points;
  nf->npoints = npoints;
  nf->col = *col;
  nf->next = NULL;
  nf->dx0 = hx;
  nf->dy0 = hy;
  nf->dz0 = hz;

  nf->dx1 = nf->dy1 = nf->dz1 = 0;
  for (i = 0; i < npoints; i++) {
    nf->dx1 += points[i].x;
    nf->dy1 += points[i].y;
    nf->dz1 += points[i].z;
  }

  nf->dx1 /= npoints;
  nf->dy1 /= npoints;
  nf->dz1 /= npoints;

  figure_list_add_tail(e, nf);
}

void ezx_fillcircle_2d(ezx_t *e, int x0, int y0, int r, const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FFILLEDCIRCLE2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->r = r;
  nf->col = *col;
  nf->width = 0;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_circle_2d(ezx_t *e, int x0, int y0, int r, const ezx_color_t *col,
		   int width)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FCIRCLE2D;
  nf->x0 = x0;
  nf->y0 = y0;
  nf->r = r;
  nf->col = *col;
  nf->width = width;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_circle_3d(ezx_t *e, double x0, double y0, double z0, double r,
		   const ezx_color_t *col)
{
  ezx_figures *nf = xcalloc(1, sizeof(ezx_figures));

  nf->type = FFILLEDCIRCLE3D;
  nf->dx0 = x0;
  nf->dy0 = y0;
  nf->dz0 = z0;
  nf->dr = r;
  nf->col = *col;
  nf->next = NULL;

  figure_list_add_tail(e, nf);
}

void ezx_set_background(ezx_t *e, const ezx_color_t *col)
{
  e->bgcolor = *col;
}

void ezx_redraw(ezx_t *e)
{
  ezx_figures  *f;
  ezx_pfigures *p;
  ezx_pfigures *pf;

  XSetBackground(e->display, e->gc, e->black.pixel);
  set_fgcolor(e, &e->bgcolor);
  XFillRectangle(e->display, e->pixmap, e->gc, 0, 0, e->size_x, e->size_y);

  // z-sorting
  pf = NULL;
  for (f = e->fig_head[e->cur_layer]; f != NULL; f = f->next) {
    double z;
    ezx_pfigures **p;

    if (f->type != FPOLY3D) continue;

    z = getz(e, f->dx1, f->dy1, f->dz1);

    for (p = &pf; *p != NULL; p = &(*p)->next) {
      ezx_pfigures *np;

      if (z <= (*p)->z) continue;

      np = xmalloc(sizeof(ezx_pfigures));
      np->z = z;
      np->fig = f;
      np->next = *p;
      *p = np;
      break;
    }

    if (*p == NULL) {
      ezx_pfigures *np = xmalloc(sizeof(ezx_pfigures));
      np->z = z;
      np->fig = f;
      np->next = NULL;
      *p = np;
    }
  }

  for (p = pf; p != NULL; p = p->next) {
    ezx_figures *f = p->fig;
    double hx, hy, hz;
    double cl;
    XPoint *xp;
    int i;

    if (f->dx0 != 0 || f->dy0 != 0 || f->dz0 != 0) {
      hx = f->dx0;
      hy = f->dy0;
      hz = f->dz0;

      if (hx * (e->eye_x - f->dx1) +
	  hy * (e->eye_y - f->dy1) +
	  hz * (e->eye_z - f->dz1) < 0) {
	continue;
      }
    } else {
      for (i = 2;; i++) {
	double x0 = f->points_3d[1].x - f->points_3d[0].x;
	double y0 = f->points_3d[1].y - f->points_3d[0].y;
	double z0 = f->points_3d[1].z - f->points_3d[0].z;
	double x1 = f->points_3d[i].x - f->points_3d[0].x;
	double y1 = f->points_3d[i].y - f->points_3d[0].y;
	double z1 = f->points_3d[i].z - f->points_3d[0].z;

	hx = y0 * z1 - y1 * z0;
	hy = z0 * x1 - z1 * x0;
	hz = x0 * y1 - x1 * y0;

	if (hx != 0 || hy != 0 || hz != 0) break;
      }
    }

    cl = (e->light_x * f->dx0 + e->light_y * f->dy0 +
	  e->light_z * f->dz0) / sqrt(f->dx0 * f->dx0 +
					  f->dy0 * f->dy0 +
					  f->dz0 * f->dz0);
    if (cl < 0) {
      cl = -cl;
      f->dx0 = -f->dx0;
      f->dy0 = -f->dy0;
      f->dz0 = -f->dz0;
    }
    if (f->dx0 * (e->eye_x - f->dx1) +
	f->dy0 * (e->eye_y - f->dy1) +
	f->dz0 * (e->eye_z - f->dz1) < 0)
      cl = 0;

    xp = xmalloc(sizeof(XPoint) * f->npoints);
    for (i = 0; i < f->npoints; i++) {
      double sx0, sy0;
      ezx_c3d_to_2d(e, f->points_3d[i].x, f->points_3d[i].y,
		    f->points_3d[i].z, &sx0, &sy0);
      xp[i].x = (int)sx0 + (e->size_x / 2);
      xp[i].y = (int)sy0 + (e->size_y / 2);
    }

    set_fgcolor(e, &f->col);
    
    XFillPolygon(e->display, e->pixmap, e->gc, xp, f->npoints,
		 Complex, CoordModeOrigin);

    free(xp);
  }

  {
    ezx_pfigures *np, *p;
    for (p = pf; p != NULL; p = np) {
      np = p->next;
      free(p);
    }

    pf = NULL;
  }

  {
    ezx_figures *f;
    int i;

    for (i = 0; i < EZX_NLAYER; i++) {
      for (f = e->fig_head[i]; f != NULL; f = f->next) {
	set_fgcolor(e, &f->col);

	switch (f->type) {
	case FPOINT2D:
	  XDrawPoint(e->display, e->pixmap, e->gc, f->x0, f->y0);
	  break;
	case FLINE2D:
	  {
	    int width = f->width;
	    if (width <= 0) break;
	    if (width == 1) width = 0;
	    XSetLineAttributes(e->display, e->gc, width, LineSolid,
			       CapRound, JoinRound);
	    XDrawLine(e->display, e->pixmap, e->gc, f->x0, f->y0, f->x1, f->y1);
	  }
	  break;
	case FLINES2D:
	  {
	    int j, width = f->width;
	    if (width <= 0) break;
	    if (width == 1) width = 0;
	    XSetLineAttributes(e->display, e->gc, width, LineSolid,
			       CapRound, JoinRound);
	    XPoint *xp = xmalloc(sizeof(XPoint) * f->npoints);
	    for (j = 0; j < f->npoints; j++) {
	      xp[j].x = f->points_2d[j].x;
	      xp[j].y = f->points_2d[j].y;
	    }
	    XDrawLines(e->display, e->pixmap, e->gc, xp, f->npoints,
		       CoordModeOrigin);
	    free(xp);
	  }
	  break;
	case FPOLY2D:
	  {
	    int j;
	    XPoint *xp = xmalloc(sizeof(XPoint) * f->npoints);
	    for (j = 0; j < f->npoints; j++) {
	      xp[j].x = f->points_2d[j].x;
	      xp[j].y = f->points_2d[j].y;
	    }
	    XFillPolygon(e->display, e->pixmap, e->gc, xp, f->npoints,
			 Complex, CoordModeOrigin);
	    free(xp);
	  }
	  break;
	case FLINE3D:
	  {
	    int width = f->width;
	    double sx0, sy0, sx1, sy1;
	    int x0, y0, x1, y1;
	    if (width <= 0) break;
	    if (width == 1) width = 0;
	    ezx_c3d_to_2d(e, f->dx0, f->dy0, f->dz0, &sx0, &sy0);
	    ezx_c3d_to_2d(e, f->dx1, f->dy1, f->dz1, &sx1, &sy1);
	    x0 = (int)sx0 + (e->size_x / 2);
	    y0 = (int)sy0 + (e->size_y / 2);
	    x1 = (int)sx1 + (e->size_x / 2);
	    y1 = (int)sy1 + (e->size_y / 2);
	    clip_line(&x0, &y0, &x1, &y1, e->size_x, e->size_y);
	    XSetLineAttributes(e->display, e->gc, width, LineSolid,
			       CapRound, JoinRound);
	    XDrawLine(e->display, e->pixmap, e->gc, x0, y0, x1, y1);
	  }
	  break;
	case FCIRCLE2D:
	  {
	    int width = f->width;
	    if (width <= 0) break;
	    if (width == 1) width = 0;
	    XSetLineAttributes(e->display, e->gc, width, LineSolid,
			       CapRound, JoinRound);
	    XDrawArc(e->display, e->pixmap, e->gc, f->x0 - f->r, f->y0 - f->r,
		     f->r * 2, f->r * 2, 0, 64 * 360);
	  }
	  break;
	case FFILLEDCIRCLE2D:
	  XFillArc(e->display, e->pixmap, e->gc, f->x0 - f->r, f->y0 - f->r,
		   f->r * 2, f->r * 2, 0, 64 * 360);
	  break;
	case FFILLEDCIRCLE3D:
	  {
	    double sx0, sy0;
	    ezx_c3d_to_2d(e, f->dx0, f->dy0, f->dz0, &sx0, &sy0);
	    XFillArc(e->display, e->pixmap, e->gc,
		     (int)sx0 + (e->size_x / 2) - (int)f->dr,
		     (int)sy0 + (e->size_y / 2) - (int)f->dr,
		     (int)f->dr * 2, (int)f->dr * 2, 0, 64 * 360);
	  }
	  break;
	case FSTR2D:
	  XDrawString(e->display, e->pixmap, e->gc, f->x0, f->y0, f->str,
		      strlen(f->str));
	  break;
	case FSTR3D:
	  {
	    double sx0, sy0;
	    ezx_c3d_to_2d(e, f->dx0, f->dy0, f->dz0, &sx0, &sy0);
	    XDrawString(e->display, e->pixmap, e->gc,
			(int)sx0 + (e->size_x / 2), (int)sy0 + (e->size_y / 2),
			f->str, strlen(f->str));
	  }
	  break;
	case FRECT2D:
	  {
	    int width = f->width;
	    if (width <= 0) break;
	    if (width == 1) width = 0;
	    XSetLineAttributes(e->display, e->gc, width, LineSolid,
			       CapRound, JoinRound);
	    XDrawRectangle(e->display, e->pixmap, e->gc, f->x0, f->y0,
			   f->x1 - f->x0, f->y1 - f->y0);
	  }
	  break;
	case FFILLEDRECT2D:
	  XFillRectangle(e->display, e->pixmap, e->gc, f->x0, f->y0,
			 f->x1 - f->x0, f->y1 - f->y0);
	  break;
	case FARC2D:
	  {
	    int width = f->width;
	    if (width <= 0) break;
	    if (width == 1) width = 0;
	    XSetLineAttributes(e->display, e->gc, width, LineSolid,
			       CapRound, JoinRound);
	    XDrawArc(e->display, e->pixmap, e->gc, f->x0 - f->x1/2, f->y0 - f->y1/2,
		     f->x1, f->y1, f->angle1 * 64, f->angle2 * 64);
	  }
	  break;
	case FFILLEDARC2D:
	  XFillArc(e->display, e->pixmap, e->gc, f->x0 - f->x1/2, f->y0 - f->y1/2,
		   f->x1, f->y1, f->angle1 * 64, f->angle2 * 64);
	  break;
	case FPOLY3D:
	  break;
	}
      }
    }
  }

  XCopyArea(e->display, e->pixmap, e->top, e->gc, 0, 0, e->size_x, e->size_y,
	    0, 0);

  XFlush(e->display);
}

void ezx_window_name(ezx_t *e, char *window_name)
{
  if (window_name != NULL)
    XStoreName(e->display, e->top, window_name);
}

int ezx_isclosed(ezx_t *e)
{
  XEvent event;

  if (!e->closed &&
      XCheckTypedWindowEvent(e->display, e->top, ClientMessage, &event) != 0) {
    if (event.xclient.message_type == e->wm_protocols &&
	event.xclient.data.l[0] == e->wm_delete_window) {
      e->closed = 1;
    }      
  }

  return e->closed;
}

void ezx_quit(ezx_t *e)
{
  ezx_wipe(e);

  XFreeFont(e->display, e->fontst);
  XFreeGC(e->display, e->gc);
  XCloseDisplay(e->display);

  color_table_free(e->color_table);

  free(e);
}

ezx_t *ezx_init(int size_x, int size_y, char *window_name)
{
  char  *server;
  ezx_t *e;
  XSetWindowAttributes atr;

  e = xcalloc(1, sizeof(ezx_t));

  server = (char *)getenv("DISPLAY");
  if (server == NULL) server = "localhost:0.0";

  e->display = XOpenDisplay(server);
  if (e->display == NULL) error_exit("can't open display \"%s\"", server);

  e->size_x = size_x;
  e->size_y = size_y;
  e->closed = 0;
  e->scrdist = 100;
  e->mag = 20;
  e->cmap = DefaultColormap(e->display, DefaultScreen(e->display));

  e->color_table = color_table_new();

  XAllocNamedColor(e->display, e->cmap, "black", &e->black, &e->rgb);
  XAllocNamedColor(e->display, e->cmap, "white", &e->white, &e->rgb);

  e->fontst = XLoadQueryFont(e->display, fontname);
  if (e->fontst == NULL) error_exit("can't load font \"%s\"", fontname);

  e->top =
    XCreateSimpleWindow(e->display, DefaultRootWindow(e->display), 0,
			0, e->size_x, e->size_y, 2,
			BlackPixel(e->display, DefaultScreen(e->display)),
			WhitePixel(e->display, DefaultScreen(e->display)));

  if (window_name != NULL)
    XStoreName(e->display, e->top, window_name);

  e->pixmap = XCreatePixmap(e->display, e->top, e->size_x, e->size_y, 
			    DefaultDepth(e->display, DefaultScreen(e->display)));
  
  e->gc = XCreateGC(e->display, e->top, 0, 0);
  XSetGraphicsExposures(e->display, e->gc, False);

  e->wm_protocols = XInternAtom(e->display, "WM_PROTOCOLS", True);
  e->wm_delete_window = XInternAtom(e->display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(e->display, e->top, &e->wm_delete_window, 1);
  
  e->size_hints.flags = PMinSize | PMaxSize;
  e->size_hints.min_width = e->size_x;
  e->size_hints.min_height = e->size_y;
  e->size_hints.max_width = e->size_x;
  e->size_hints.max_height = e->size_y;
  XSetNormalHints(e->display, e->top, &e->size_hints);

  atr.backing_store = WhenMapped;
  XChangeWindowAttributes(e->display, e->top, CWBackingStore, &atr);

  XSetWindowBackgroundPixmap(e->display, e->top, e->pixmap);

  XSelectInput(e->display, e->top,
	       ExposureMask | KeyPressMask | KeyReleaseMask | ButtonMotionMask |
	       OwnerGrabButtonMask | ButtonPressMask | ButtonReleaseMask);

  XResizeWindow(e->display, e->top, e->size_x, e->size_y);

  XMapWindow(e->display, e->top);

  XFlush(e->display);

  {
    XEvent ev;
    do {
      XNextEvent(e->display, &ev);
    } while (ev.type != Expose);
  }

  ezx_set_background(e, &ezx_white);
  ezx_set_view_3d(e, 1000, 0, 0, 0, 0, 0, 5);
  ezx_set_light_3d(e, 1000, 900, 800);
  ezx_redraw(e);
  
  return e;
}

static inline unsigned int get_state_mask(unsigned int xstate)
{
  unsigned int state = 0;

  if (xstate & ShiftMask) state |= EZX_SHIFT_MASK;
  if (xstate & ControlMask) state |= EZX_CONTROL_MASK;
  if (xstate & Button1Mask) state |= EZX_BUTTON_LMASK;
  if (xstate & Button2Mask) state |= EZX_BUTTON_MMASK;
  if (xstate & Button3Mask) state |= EZX_BUTTON_RMASK;

  return state;
}

int ezx_sensebutton(ezx_t *e, int *x, int *y)
{
  Window root, child;
  int root_x, root_y, win_x, win_y;
  unsigned int s;

  if (x == NULL) x = &win_x;
  if (y == NULL) y = &win_y;

  XQueryPointer(e->display, e->top, &root, &child, &root_x, &root_y, x, y, &s);
  
  return get_state_mask(s);
}

int ezx_pushbutton(ezx_t *e, int *x, int *y)
{
  XEvent xevent;

  for (;;) {
    XMaskEvent(e->display, ExposureMask | ButtonPressMask, &xevent);
    
    if (xevent.type == Expose) {
      ezx_redraw(e);
    } else if (xevent.type == ButtonPress) {
      if (x) *x = xevent.xbutton.x;
      if (y) *y = xevent.xbutton.y;
      return xevent.xbutton.button;
    }
  }
}

void ezx_next_event(ezx_t *e, ezx_event_t *event)
{
  XEvent xevent;

  for (;;) {
    XNextEvent(e->display, &xevent);
    
    if (xevent.type == Expose) {
      ezx_redraw(e);
    } else if (xevent.type == ButtonPress || xevent.type == ButtonRelease) {
      if (event) {
	if (xevent.type == ButtonPress) event->button.type = EZX_BUTTON_PRESS;
	else event->button.type = EZX_BUTTON_RELEASE;
	event->button.b = xevent.xbutton.button;
	event->button.x = xevent.xbutton.x;
	event->button.y = xevent.xbutton.y;
	event->button.state = get_state_mask(xevent.xbutton.state);
      }
      break;
    } else if (xevent.type == KeyPress || xevent.type == KeyRelease) {
      int ret;
      char c;
      KeySym keysym;
      if ((ret=XLookupString((XKeyEvent *)&xevent, &c, 1, &keysym, NULL)) == 1 ||
	  (XK_Home <= keysym && keysym <= XK_Down)) {
	if (event) {
	  if (xevent.type == KeyPress) event->key.type = EZX_KEY_PRESS;
	  else event->key.type = EZX_KEY_RELEASE;
	  if (ret == 1) event->key.k = c;
	  else event->key.k = keysym;
	  event->key.x = xevent.xkey.x;
	  event->key.y = xevent.xkey.y;
	  event->key.state = get_state_mask(xevent.xkey.state);
	}
	break;
      }
    } else if (xevent.type == MotionNotify) {
      if (event) {
	event->motion.type = EZX_MOTION_NOTIFY;
	event->motion.x = xevent.xmotion.x;
	event->motion.y = xevent.xmotion.y;
	event->motion.state = get_state_mask(xevent.xmotion.state);
      }
      break;
    } else if (xevent.type == ClientMessage) {
      if (xevent.xclient.message_type == e->wm_protocols &&
	  xevent.xclient.data.l[0] == e->wm_delete_window) {
	e->closed = 1;
	if (event) event->type = EZX_CLOSE;
	break;
      }
    }
  }
}
