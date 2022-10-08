#ifndef _WINLIB_H_
#define _WINLIB_H_

#include <windows.h>

void usleep(unsigned long usec)
{
  if (usec < 1000) usec = 1000;
  Sleep(usec / 1000);
}

#endif
