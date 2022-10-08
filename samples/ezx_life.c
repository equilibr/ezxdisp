#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#ifdef WIN32
#include "winlib.h"
#endif

#include "ezxdisp.h"

typedef struct {
  enum {
    NORMAL,
    DUMMY
  } type;
  enum {
    DEAD,
    ALIVE
  } state, next_state;
  int num_neighbors;
} life_t;

static void life_evaluate_next_state(life_t *life, int num_neighbors)
{
  if (life->type == NORMAL) {
    if (num_neighbors == 2) {
      if (life->state == ALIVE) life->next_state = ALIVE;
      else life->next_state = DEAD;
    } else if (num_neighbors == 3) life->next_state = ALIVE;
    else life->next_state = DEAD;
  }
}

static void update_num_neighbors(life_t **life, int x, int y)
{
  int i, j;
  
  for (i = x-1; i <= x+1; i++) {
    for (j = y-1; j <= y+1; j++) {
      if (i == x && j == y) continue;
      life[i][j].num_neighbors++;
    }
  }
}

int main(int argc, char *argv[])
{
  int i, j, x, y;
  int width=500, height=500;
  int cell_width=1, cell_height=1;
  int nrow=height/cell_height+2, ncol=width/cell_width+2;
  int gen, maxgen=0;
  life_t **life;
  ezx_t *e;

  if (argc >= 2 && atoi(argv[1]) != 0) srand(atoi(argv[1]));
  else srand(time(NULL));
  if (argc >= 3) maxgen = atoi(argv[2]);

  e = ezx_init(width, height, "Conway's Game of Life");

  life = malloc(sizeof(life_t *) * nrow);
  for (i = 0; i < nrow; i++) life[i] = calloc(1, sizeof(life_t) * ncol);

  for (i = 0; i < nrow; i++) {
    for (j = 0; j < ncol; j++) {
      if (i == 0 || j == 0 || i == nrow-1 || j == ncol-1) {
	life[i][j].type = DUMMY;
	life[i][j].state = DEAD;
      } else {
	life[i][j].type = NORMAL;
	life[i][j].state = rand()%2;
	if (life[i][j].state == ALIVE) update_num_neighbors(life, i, j);
      }
    }
  }
  
  for (gen = 1; ; gen++) {
    ezx_wipe(e);
    
    for (i = 1; i < nrow-1; i++) {
      for (j = 1; j < ncol-1; j++) {
	life_evaluate_next_state(&life[i][j], life[i][j].num_neighbors);
	life[i][j].num_neighbors = 0;
      }
    }

    for (i=1, y=0; i<nrow-1; i++, y+=cell_height) { 
      for (j=1, x=0; j<ncol-1; j++, x+=cell_width) {
	life[i][j].state = life[i][j].next_state;
	if (life[i][j].state == ALIVE) {
	  update_num_neighbors(life, i, j);
	  ezx_fillrect_2d(e, x, y, x+cell_width, y+cell_height, &ezx_blue);	  
	}
      }
    }
    
    ezx_redraw(e);
    
    usleep(10000);

    if ((maxgen > 0 && gen == maxgen) || ezx_isclosed(e)) break;
  }

  for (i = 0; i < nrow; i++) free(life[i]);
  free(life);

  ezx_quit(e);
  
  exit(EXIT_SUCCESS);
}
