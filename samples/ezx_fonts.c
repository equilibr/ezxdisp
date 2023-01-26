#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ezxdisp.h"

#ifdef WIN32
char *fonts[11] = {
  "Arial", "Consolas", "Courier", "FrankRuehl", "Georgia", "Lucida Console", "Microsoft Sans Serif", "Miriam",
  "Tahoma", "Times New Roman", "Wingdings"
};
#else
char *fonts[11] = {
  "fixed", "4x7", "5x8", "6x10", "7x13", "8x16", "9x14", "9x15", "9x15bold",
  "10x19", "10x20",
};
#endif

void tst(ezx_t *e, int x, int y, char *fontname, const ezx_color_t *col)
{
  char buf[1000];
  sprintf(buf, "%s %sfound", fontname, ezx_set_font(e,fontname) ? "" : "not ");
  ezx_str_2d(e,x,y,buf,col);
}

int main(int argc, char *argv[])
{
  ezx_t *e = ezx_init(270, 300, "font test");

  for(int i=0; i<11; ++i)  tst(e,50,50+20*i,fonts[i],&ezx_black);

  ezx_redraw(e);

  ezx_pushbutton(e, NULL, NULL);
  
  ezx_quit(e);
  return 0;
}
