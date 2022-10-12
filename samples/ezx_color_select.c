#include <stdio.h>
#include "ezxdisp.h"

extern ezx_color_t cols[54];

int main()
  {
  int i,x,y;
  ezx_t *w;
  int col = 0;

  w = ezx_init(900,600,"ColorSelect");

  for(i=0; i<54; i++)
    {
    col = i;

    ezx_fillrect_2d(w,100*(i%9),100*(i/9),100*(i%9)+100,100*(i/9)+100,cols+col);
    }

  ezx_redraw(w);

  ezx_pushbutton(w,&x,&y);

  ezx_quit(w);

  printf("You selected color with number %d.\n",(y/100)*9+(x/100));

  return 0;

  }


#define _f(c) {(c>>16)/255.0,((c>>8)&0xFF)/255.0,(c&0xFF)/255.0}

ezx_color_t cols[54] = {_f(0xffffff),_f(0x0),_f(0xff0000),
_f(0x4eee94),_f(0xff),_f(0xffff00),_f(0xffd700),_f(0xee82ee),
_f(0xffc0cb),_f(0x0),_f(0xd0d0d),_f(0x1a1a1a),_f(0x262626),
_f(0x333333),_f(0x404040),_f(0x4d4d4d),_f(0x595959),
_f(0x666666),_f(0x737373),_f(0x7f7f7f),_f(0x8c8c8c),
_f(0x999999),_f(0xa6a6a6),_f(0xb3b3b3),_f(0xbfbfbf),
_f(0xcccccc),_f(0xd9d9d9),_f(0xe5e5e5),_f(0xf2f2f2),
_f(0xffffff),_f(0xc71585),_f(0x8968cd),_f(0x548b54),
_f(0xffff),_f(0x36648b),_f(0x8b2500),_f(0x8b864e),_f(0xced1),
_f(0x228b22),_f(0xfdf5e6),_f(0x778899),_f(0x708090),
_f(0x2f4f4f),_f(0x668b8b),_f(0x32cd32),_f(0x5f9ea0),
_f(0xff1493),_f(0xee00ee),_f(0x8b4726),_f(0xffb5c5),
_f(0x868b),_f(0x4169e1),_f(0xeeee00),_f(0xbdc1ff)};
