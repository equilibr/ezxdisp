#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "ezxdisp.h"
#include "tsplib.h"

#define NCITY 1000

#define EPS 0.00001

typedef struct {
  int num;
  double key;
} sort_t;

typedef struct {
  int city1, city2;
} edge_t;

ezx_t *e;
int city_x[NCITY], city_y[NCITY];

int compare(const void *a, const void *b)
{
  if (((sort_t *) a)->key == ((sort_t *) b)->key)
    return 0;
  if (((sort_t *) a)->key < ((sort_t *) b)->key)
    return -1;
  return 1;
}

static int calccnt = 0;

double calc_dist(tsplib_t * prob, edge_t * edge)
{
  double x[3], y[3];

  calccnt++;

  x[0] = prob->coord[edge->city1].r[0];
  x[1] = prob->coord[edge->city1].r[1];
  x[2] = prob->coord[edge->city1].r[2];

  y[0] = prob->coord[edge->city2].r[0];
  y[1] = prob->coord[edge->city2].r[1];
  y[2] = prob->coord[edge->city2].r[2];

  return sqrt((x[0] - y[0]) * (x[0] - y[0]) +
	      (x[1] - y[1]) * (x[1] - y[1]) + (x[2] - y[2]) * (x[2] - y[2]));
}

static int *tour, *tourp;
static int *t;
static edge_t *x, *y;
static double *g;
static int dim;
static int *tflag, *tflag2;

#define RV_NormalTerm 0
#define RV_GoToStep2  1

#define BreakLoop 10000000

void showtour(void)
{
  int i;

  for (i = 0; i < dim; i++)
    fprintf(stderr, "%2d ", tour[i]);
  fprintf(stderr, "\n");
  for (i = 0; i < dim; i++)
    fprintf(stderr, "%2d ", tflag[i]);
  fprintf(stderr, "\n");
  for (i = 0; i < dim; i++)
    fprintf(stderr, "%2d ", tflag2[i]);
  fprintf(stderr, "\n");
}

void showtour2(int *tour, int eval)
{
  int i, j, dir;
  char str[100];

  for (j = 0; j < dim; j++)
    if (tour[j] == 0)
      break;

  if (tour[(j + 1) % dim] < tour[(j + dim - 1) % dim])
    dir = 1;
  else
    dir = dim - 1;

  if (eval == 0) {
    ezx_select_layer(e, 2);
    ezx_wipe_layer(e, 2);
  } else {
    ezx_select_layer(e, 1);
    ezx_wipe_layer(e, 1);
  }

  for (i = 0; i < dim; i++) {
    ezx_line_2d(e, city_x[tour[(i * dir + j) % dim]],
		city_y[tour[(i * dir + j) % dim]],
		city_x[tour[(i * dir + j + dir) % dim]],
		city_y[tour[(i * dir + j + dir) % dim]],
		eval == 0 ? &ezx_black : &ezx_grey75,
		eval == 0 ? 1 : 3);
  }

  sprintf(str, "eval=%d", eval);

  if (eval != 0)
    ezx_str_2d(e, 550, 470, str, &ezx_black);

  //printf("\n");

  ezx_redraw(e);

  //for(i=0;i<dim;i++) fprintf(stderr,"%2d ",tour[(i*dir+j) % dim]);
  //fprintf(stderr,"\n");
}

double calcEval(tsplib_t * prob)
{
  edge_t e;
  int i;
  double dsum = 0;

  for (i = 0; i < dim; i++) {
    e.city1 = tour[i];
    e.city2 = tour[(i + 1) % dim];
    dsum += calc_dist(prob, &e);
  }

  return dsum;
}

void showEval(tsplib_t * prob)
{
  fprintf(stderr, "eval : %.10g\n", calcEval(prob));
}

int pureLKsub(tsplib_t * prob, int i)
{
  int *flag7;
  int s6, s7;
  int ret = 0;

  int j, k, l;

  flag7 = (int*) malloc(sizeof(int) * (dim + 1));

  // STEP 5. Let i = i + 1;

  //i = i + 1;

  // STEP 6. Choose x_i = (t_{2i-1},t_{2i}) \in T such that
  //         (a) if t_{2i} is joined to t_1, the resulting configuration is a tour, T', and
  //         (b) x_i \neq x_s for all s < i.
  //         If T' is a better tour than T, let T=T' and go to Step 2.

  x[i].city1 = tour[t[2 * i - 1]];

  for (s6 = 0;; s6++) {
    if (s6 != 0) {
      if (tflag[t[2 * i]] == 2 * i)
	tflag[t[2 * i]] = 0;
      if (tflag2[t[2 * i]] == 2 * i)
	tflag2[t[2 * i]] = 0;
    }

    if (!(s6 < 2))
      break;

    if (s6 == 1) {
      t[2 * i] = (t[2 * i - 1] + 1) % dim;
    } else {
      t[2 * i] = (t[2 * i - 1] + dim - 1) % dim;
    }

    if (tflag[t[2 * i]] != 0)
      tflag2[t[2 * i]] = 2 * i;
    else
      tflag[t[2 * i]] = 2 * i;
    x[i].city2 = tour[t[2 * i]];

    // check condition (b)

    for (j = 0; j < i; j++) {
      if ((x[j].city1 == x[i].city1 && x[j].city2 == x[i].city2) ||
	  (x[j].city1 == x[i].city2 && x[j].city2 == x[i].city1))
	break;
    }

    if (j < i)
      continue;

    // check condition (a)

    {
      int ocp = t[1];
      int ocn = 1;
      int cp = t[2 * i];
      int cn = 2 * i;
      int cnt = 0;

      tourp[cnt++] = tour[t[2 * i]];

      for (; cnt < dim;) {
	if (cn == 1)
	  break;

	if (tflag2[cp] != 0) {
	  if (tflag[cp] == cn) {
	    ocn = tflag2[cp];
	    ocp = cp;
	  } else {
	    ocn = tflag[cp];
	    ocp = cp;
	  }
	  if (ocn & 1)
	    cn = ocn - 1;
	  else
	    cn = ocn + 1;
	  cp = t[cn];

	  tourp[cnt++] = tour[cp];
	} else {
	  int dir;

	  if (cn & 1) {
	    if (tflag[(cp + 1) % dim] == cn + 1)
	      dir = dim - 1;
	    else
	      dir = 1;
	  } else {
	    if (tflag[(cp + 1) % dim] == cn - 1)
	      dir = dim - 1;
	    else
	      dir = 1;
	  }

	  ocp = cp;
	  ocp = (ocp + dir >= dim ? ocp + dir - dim : ocp + dir);

	  for (; tflag[ocp] == 0;
	       ocp =
		 (ocp + dir >=
		  dim ? ocp + dir - dim : ocp + dir)) {
	    tourp[cnt++] = tour[ocp];
	    if (cnt >= dim)
	      break;
	  }

	  tourp[cnt++] = tour[ocp];

	  ocn = tflag[ocp];
	  if (ocn == 1) {
	    cn = 1;
	    break;
	  }
	  if (ocn & 1)
	    cn = ocn - 1;
	  else
	    cn = ocn + 1;
	  cp = t[cn];

	  tourp[cnt++] = tour[cp];
	}
      }

      if (cnt == dim && cn == 1) {
	// condition (a) is met
	double dx, dy;

	y[i].city1 = tour[t[2 * i]];
	y[i].city2 = tour[t[1]];

	dx = calc_dist(prob, &x[i]);
	dy = calc_dist(prob, &y[i]);

	g[i] = g[i - 1] + dx - dy;

	if (g[i] > EPS) {
	  for (j = 0; j < dim; j++)
	    tour[j] = tourp[j];

	  showtour2(tour, 0);

	  ret = RV_GoToStep2;
	  goto EndOfSub;	// goto Step2;
	}
	// STEP 7. Choose y_i = (t_{2i},t_{2i+1}) not in T such that
	//         (a) G_i > 0,
	//         (b) y_i \neq y_s for all s \neq i, and
	//         (c) x_{i+1} exists.

	for (j = 0; j < dim; j++)
	  flag7[j] = 0;


	flag7[t[2 * i - 1]] = 1;
	flag7[t[2 * i]] = 1;

	if ((t[2 * i - 1] + 1) % dim == t[2 * i])
	  flag7[(t[2 * i] + 1) % dim] = 1;
	else
	  flag7[(t[2 * i] + dim - 1) % dim] = 1;

	for (s7 = 3;; s7++) {
	  if (s7 > 3) {
	    if (tflag[t[2 * i + 1]] == 2 * i + 1)
	      tflag[t[2 * i + 1]] = 0;
	    if (tflag2[t[2 * i + 1]] == 2 * i + 1)
	      tflag2[t[2 * i + 1]] = 0;
	  }

	  if (!(s7 < dim))
	    break;

	  k = rand() % (dim - s7) + 1;
	  for (l = 0; l < dim; l++) {
	    if (flag7[l] == 0)
	      k--;
	    if (k == 0)
	      break;
	  }

	  assert(l != dim);

	  flag7[l] = 1;

	  if (tflag[l] != 0 && tflag2[l] != 0)
	    continue;

	  t[2 * i + 1] = l;
	  if (tflag[l] == 0)
	    tflag[l] = 2 * i + 1;
	  else
	    tflag2[l] = 2 * i + 1;

	  y[i].city1 = tour[t[2 * i]];
	  y[i].city2 = tour[t[2 * i + 1]];
	  dy = calc_dist(prob, &y[i]);

	  g[i] = g[i - 1] + dx - dy;
	  if (g[i] <= 0)
	    continue;

	  j = pureLKsub(prob, i + 1);

	  if (j == RV_GoToStep2) {
	    ret = RV_GoToStep2;
	    goto EndOfSub;
	  }

	  if (j == RV_NormalTerm && i > 2) {
	    s6 = s7 = BreakLoop;
	  }
	}
      }
    }
  }

 EndOfSub:

  free(flag7);

  return ret;
}

#define MESH 20

void genInitTour(tsplib_t * prob, int *tour)
{
  int j, k;
  double min[3], max[3];

  int head[MESH][MESH];
  int *next;

  next = (int*) malloc(dim * sizeof(int));

  for (j = 0; j < dim; j++)
    next[j] = -1;

  for (j = 0; j < 2; j++)
    min[j] = max[j] = prob->coord[0].r[j];

  for (j = 1; j < dim; j++) {
    for (k = 0; k < 2; k++) {
      if (min[k] > prob->coord[j].r[k])
	min[k] = prob->coord[j].r[k];
      if (max[k] < prob->coord[j].r[k])
	max[k] = prob->coord[j].r[k];
    }
  }

  for (j = 0; j < dim; j++) {
    int c[3];

    for (k = 0; k < 2; k++)
      c[k] =
	(prob->coord[j].r[k] -
	 min[k]) / ((max[k] - min[k]) / MESH);

    if (head[c[0]][c[1]] != -1)
      next[j] = head[c[0]][c[1]];
    head[c[0]][c[1]] = j;

  }

  free(next);
}

void pureLK(tsplib_t * prob, int *result)
{
  int i;
  int chain;

  int j, k, l, s2, s3, s4;
  int *flag2, *flag4;

  double bestEval = -1;

  assert(prob->type == TL_TYPE_NONE || prob->type == TL_TYPE_TSP);
  assert(prob->edge_weight_type == TL_EWT_NONE
	 || prob->edge_weight_type == TL_EWT_EUC_2D
	 || prob->edge_weight_type == TL_EWT_EUC_3D);

  dim = prob->dimension;

  tour = (int*) calloc(dim, sizeof(int));
  tourp = (int*) calloc(dim + 2, sizeof(int));

  t = (int*) calloc((dim + 1) * 2, sizeof(int));

  x = (edge_t*) calloc(dim + 1, sizeof(edge_t));
  y = (edge_t*) calloc(dim + 1, sizeof(edge_t));
  g = (double*) calloc(dim + 1, sizeof(double));

  flag2 = (int*) malloc(sizeof(int) * (dim + 1));
  flag4 = (int*) malloc(sizeof(int) * (dim + 1));
  tflag = (int*) calloc((dim + 1), sizeof(int));
  tflag2 = (int*) calloc((dim + 1), sizeof(int));

  // STEP 1. Generate the initial tour t

  for (j = 0; j < dim; j++)
    tour[j] = result[j];

  for (chain = 0; chain < 30; chain++) {
    // STEP 2. Let i = 1. Choose t_1.

  Step2:

    for (j = 0; j < dim; j++)
      flag2[j] = 0;

    for (s2 = 0; s2 < dim; s2++) {
      for (j = 0; j < dim + 1; j++)
	tflag[j] = tflag2[j] = 0;

      k = rand() % (dim - s2) + 1;
      for (l = 0; l < dim; l++) {
	if (flag2[l] == 0)
	  k--;
	if (k == 0)
	  break;
      }

      assert(l != dim);

      t[1] = l;

      i = 1;

      tflag[t[1]] = 1;

      // STEP 3. Choose x_1 = (t_1,t_2) \in T

      for (s3 = 0;; s3++) {
	if (s3 != 0) {
	  assert(tflag[t[2]] == 2 && tflag2[t[2]] == 0);
	  tflag[t[2]] = 0;
	}

	if (!(s3 < 2))
	  break;

	if (s3 == 1) {
	  x[1].city1 = tour[t[1]];
	  t[2] = (t[1] + 1) % dim;
	  x[1].city2 = tour[t[2]];
	} else {
	  x[1].city1 = tour[t[1]];
	  t[2] = (t[1] + dim - 1) % dim;
	  x[1].city2 = tour[t[2]];
	}

	assert(tflag[t[2]] == 0 && tflag2[t[2]] == 0);
	tflag[t[2]] = 2;

	// STEP 4. Choose y_1 = (t_2,t_3) not in T such that G_1 > 0
	//         If this is not possible, go to Step 12.

	{
	  double dx, dy;

	  dx = calc_dist(prob, &x[1]);

	  for (j = 0; j < dim; j++)
	    flag4[j] = 0;

	  flag4[t[1]] = 1;
	  flag4[t[2]] = 1;
	  if ((t[1] + 1) % dim == t[2])
	    flag4[(t[2] + 1) % dim] = 1;
	  else
	    flag4[(t[2] + dim - 1) % dim] = 1;

	  for (s4 = 3;; s4++) {
	    if (s4 > 3) {
	      assert(tflag[t[3]] == 3 && tflag2[t[3]] == 0);
	      tflag[t[3]] = 0;
	    }

	    if (!(s4 < dim))
	      break;

	    k = rand() % (dim - s4) + 1;
	    for (l = 0; l < dim; l++) {
	      if (flag4[l] == 0)
		k--;
	      if (k == 0)
		break;
	    }

	    assert(l != dim);

	    flag4[l] = 1;

	    t[3] = l;
	    assert(tflag[t[3]] == 0 && tflag2[t[3]] == 0);
	    tflag[t[3]] = 3;

	    y[1].city1 = tour[t[2]];
	    y[1].city2 = tour[t[3]];
	    dy = calc_dist(prob, &y[1]);

	    g[1] = dx - dy;

	    j = 1;

	    if (g[1] <= 0)
	      continue;

	    j = pureLKsub(prob, i + 1);

	    if (j == RV_GoToStep2) {
	      goto Step2;
	    }
	  }
	}
      }
    }

    {
      double e = calcEval(prob);

      //fprintf(stderr,"eval : %.10g",e);

      if (bestEval == -1 || bestEval > e) {
	//fprintf(stderr,"\tnew record");
	for (j = 0; j < dim; j++)
	  result[j] = tour[j];
	bestEval = e;

	showtour2(tour, bestEval);
      }
      //fprintf(stderr,"\n");
    }

    {
      // double-bridge

      sort_t a[4];

      for (j = 0; j < 4; j++)
	a[j].key = rand() % (dim - 8);

      qsort(a, 4, sizeof(sort_t), compare);

      a[1].key += 2;
      a[2].key += 4;
      a[3].key += 6;

      k = 0;
      for (j = 0; j <= a[0].key; j++)
	tourp[k++] = tour[j];
      for (j = a[2].key + 1; j <= a[3].key; j++)
	tourp[k++] = tour[j];
      for (j = a[1].key + 1; j <= a[2].key; j++)
	tourp[k++] = tour[j];
      for (j = a[0].key + 1; j <= a[1].key; j++)
	tourp[k++] = tour[j];
      for (j = a[3].key + 1; j < dim; j++)
	tourp[k++] = tour[j];

      assert(k == dim);

      for (j = 0; j < dim; j++)
	tour[j] = tourp[j];
    }
  }

  free(tourp);
  free(tour);

  free(tflag2);
  free(tflag);
  free(flag4);
  free(flag2);

  free(g);
  free(y);
  free(x);

  free(t);
}

#if 0
int main(int argc, char **argv)
{
  int i, j;
  tsplib_t *t;
  int *tour;

  t = tsplib_alloc();
  tsplib_loadfrom(t, argv[1]);

  if (argc > 2)
    srand(atoi(argv[2]));
  else
    srand(time(NULL));

  printf("name : %s\n", t->name);
  printf("comment : %s\n", t->comment);

  tour = malloc(sizeof(int) * t->dimension);

  dim = t->dimension;

#if 0
  genInitTour(t, tour);
#else
  for (j = 0; j < dim; j++)
    tour[j] = j;

  for (j = 0; j < dim * 2; j++) {
    int p, q, s;
    p = j % dim;
    q = rand() % dim;

    s = tour[p];
    tour[p] = tour[q];
    tour[q] = s;
  }
#endif

  pureLK(t, tour);

  showtour2(tour, 0);

  printf("calccnt : %d\n", calccnt);
}
#endif

int main(int argc, char *argv[])
{
  int mouse_x, mouse_y;
  int x1, y1;
  int i, j;

  //

  tsplib_t tl;
  int *tour;
  int dir;

  //

  tl.coord = (tsplib_coord_t*) malloc(sizeof(tsplib_coord_t) * NCITY);
  tl.type = TL_TYPE_TSP;
  tl.edge_weight_type = TL_EWT_EUC_2D;

  //

  e = ezx_init(640, 480, "Solve TSP with LK algorithm");

  ezx_set_background(e, &ezx_white);

  ezx_select_layer(e, 3);

  for (i = 0; i < NCITY; i++) {
    int b;

    b = ezx_pushbutton(e, &mouse_x, &mouse_y);
    tl.coord[i].node_num = i;
    tl.coord[i].r[0] = mouse_x;
    tl.coord[i].r[1] = mouse_y;

    x1 = mouse_x;
    y1 = mouse_y;
    city_x[i] = x1;
    city_y[i] = y1;

    ezx_line_2d(e, x1 - 5, y1 - 5, x1 + 5, y1 + 5, &ezx_red, 3);
    ezx_line_2d(e, x1 + 5, y1 - 5, x1 - 5, y1 + 5, &ezx_red, 3);

    ezx_redraw(e);


    if (i >= 8 && b == 3)
      break;
  }

  tl.dimension = i + 1;

  //

  tour = (int*) malloc(sizeof(int) * tl.dimension);

  dim = tl.dimension;

#if 0
  genInitTour(t, tour);
#else
  for (j = 0; j < dim; j++)
    tour[j] = j;

  for (j = 0; j < dim * 2; j++) {
    int p, q, s;
    p = j % dim;
    q = rand() % dim;

    s = tour[p];
    tour[p] = tour[q];
    tour[q] = s;
  }
#endif

  pureLK(&tl, tour);

  //showtour2(tour);

#if 1
  for (j = 0; j < dim; j++)
    if (tour[j] == 0)
      break;

  if (tour[(j + 1) % dim] < tour[(j + dim - 1) % dim])
    dir = 1;
  else
    dir = dim - 1;

  ezx_select_layer(e, 1);

  ezx_wipe_layer(e, 1);
  ezx_wipe_layer(e, 2);

  for (i = 0; i < dim; i++) {
    ezx_line_2d(e, city_x[tour[(i * dir + j) % dim]],
		city_y[tour[(i * dir + j) % dim]],
		city_x[tour[(i * dir + j + dir) % dim]],
		city_y[tour[(i * dir + j + dir) % dim]], &ezx_black,
		2);
  }

  //printf("\n");

  ezx_redraw(e);
#endif

  ezx_pushbutton(e, NULL, NULL);

  ezx_quit(e);

  return 0;
}
