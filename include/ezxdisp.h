/*
 * ezxdisp.h
 * This file is part of the ezxdisp library.
 */

#ifndef _EZXDISP_H_
#define _EZXDISP_H_

#if defined(__cplusplus)
extern "C" {
#define eet(e) ezx_event_t::e
#else
#define eet(e) e
#endif


#ifdef WIN32
#define main ezx_main
#endif

#define EZX_NLAYER 8

#define EZX_BUTTON_LEFT      1
#define EZX_BUTTON_MIDDLE    2
#define EZX_BUTTON_RIGHT     3
#define EZX_BUTTON_WHEELUP   4
#define EZX_BUTTON_WHEELDOWN 5

#define EZX_KEY_HOME  0xff50
#define EZX_KEY_LEFT  0xff51
#define EZX_KEY_UP    0xff52
#define EZX_KEY_RIGHT 0xff53
#define EZX_KEY_DOWN  0xff54

#define EZX_SHIFT_MASK   (1<<0)
#define EZX_CONTROL_MASK (1<<1)
#define EZX_BUTTON_LMASK (1<<2)
#define EZX_BUTTON_MMASK (1<<3)
#define EZX_BUTTON_RMASK (1<<4)

typedef struct ezx_s ezx_t;

typedef struct {
  double r, g, b;
} ezx_color_t;

typedef struct {
  int x, y;
} ezx_point2d_t;

typedef struct {
  double x, y, z;
} ezx_point3d_t;

typedef union {
  enum ezx_event_type {
    EZX_BUTTON_PRESS,
    EZX_BUTTON_RELEASE,
    EZX_KEY_PRESS,
    EZX_KEY_RELEASE,
    EZX_MOTION_NOTIFY,
    EZX_CLOSE
  } type;
  
  struct ezx_button_event {
    enum ezx_event_type type; /* EZX_BUTTON_PRESS or EZX_BUTTON_RELEASE */
    unsigned int b;           /* The mouse button code */
    int x, y;                 /* The x-y coordinates of the mouse pointer */
    unsigned int state;       /* The key or button mask */
  } button;
  
  struct ezx_key_event {
    enum ezx_event_type type; /* EZX_KEY_PRESS or EZX_KEY_RELEASE */
    unsigned int k;           /* The key code */
    int x, y;                 /* The x-y coordinates of the mouse pointer */
    unsigned int state;       /* The key or button mask */
  } key;
  
  struct ezx_motion_event {
    enum ezx_event_type type; /* EZX_MOTION_NOTIFY */
    int x, y;                 /* The x-y coordinates of the mouse pointer */
    unsigned int state;       /* The key or button mask */
  } motion;
} ezx_event_t;

extern const ezx_color_t ezx_black, ezx_white;
extern const ezx_color_t ezx_grey25, ezx_grey50, ezx_grey75;
extern const ezx_color_t ezx_blue, ezx_red, ezx_green, ezx_yellow;
extern const ezx_color_t ezx_purple, ezx_pink, ezx_cyan, ezx_brown, ezx_orange;

ezx_t *ezx_init(int size_x, int size_y, char *window_name);
void   ezx_quit(ezx_t *e);
void   ezx_redraw(ezx_t *e);
void   ezx_wipe(ezx_t *e);
void   ezx_wipe_layer(ezx_t *e, int lay);
void   ezx_select_layer(ezx_t *e, int lay);
void   ezx_set_background(ezx_t *e, const ezx_color_t *col);
void   ezx_window_name(ezx_t *e, char *window_name);
int    ezx_isclosed(ezx_t *e);
int    ezx_sensebutton(ezx_t *e, int *x, int *y);
int    ezx_pushbutton(ezx_t *e, int *x, int *y);
void   ezx_next_event(ezx_t *ezx, ezx_event_t *event);

// 2D graphics
void ezx_point_2d(ezx_t *e, int x, int y, const ezx_color_t *col);
void ezx_line_2d(ezx_t *e, int x0, int y0, int x1, int y1,
		 const ezx_color_t *col, int width);
void ezx_lines_2d(ezx_t *e, ezx_point2d_t *points, int npoints,
		  const ezx_color_t *col, int width);
void ezx_poly_2d(ezx_t *e,ezx_point2d_t *points, int npoints,
		 const ezx_color_t *col);
void ezx_str_2d(ezx_t *e, int x, int y, char *str, const ezx_color_t *col);
void ezx_rect_2d(ezx_t *e, int x0, int y0, int x1, int y1,
		 const ezx_color_t *col, int width);
void ezx_fillrect_2d(ezx_t *e, int x0, int y0, int x1, int y1,
		     const ezx_color_t *col);
void ezx_circle_2d(ezx_t *e, int x, int y, int r, const ezx_color_t *col,
		   int width);
void ezx_fillcircle_2d(ezx_t *e, int x, int y, int r, const ezx_color_t *col);
void ezx_arc_2d(ezx_t *e, int x, int y, int w, int h, double angle1,
		double angle2, const ezx_color_t *col, int width);
void ezx_fillarc_2d(ezx_t *e, int x, int y, int w, int h, double angle1,
		    double angle2, const ezx_color_t *col);

// 3D graphics
void ezx_c3d_to_2d(ezx_t *e, double sx, double sy, double sz, double *dx,
		   double *dy);
void ezx_line_3d(ezx_t *e, double x0, double y0, double z0, double x1,
		 double y1, double z1, const ezx_color_t *col, int width);
void ezx_set_light_3d(ezx_t *e, double ex, double ey, double ez);
void ezx_set_view_3d(ezx_t *e, double ex, double ey, double ez, double vx,
		     double vy, double vz, double m);
void ezx_str_3d(ezx_t *e, double x0, double y0, double z0, char *str,
		const ezx_color_t *col);
void ezx_poly_3d(ezx_t *e, ezx_point3d_t *points, double hx, double hy,
		 double hz, int npoints, const ezx_color_t *col);
void ezx_circle_3d(ezx_t *e, double x0, double y0, double z0, double r,
		   const ezx_color_t *col);

#if defined(__cplusplus)
}
#endif

#endif
