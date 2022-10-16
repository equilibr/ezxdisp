VPATH = ../../include
VPATH += ../../samples

CC = gcc
CFLAGS = -Wall -O2 

SAMPLES = ezx_test ezx_test2 ezx_test3 ezx_test4 ezx_test5 ezx_test6 ezx_test7 ezx_test8 ezx_test9 ezx_test10 ezx_test11 ezx_test12 ezx_test13 tsp tsp_LK 3d_clock ezx_engine ezx_rotary ezx_life ezx_color_select ezx_fonts ezx_test5a
SAMPLES_OBJS = $(SAMPLES:=.o)

X11_INC = -I/usr/X11R6/include
X11_LIB = -L/usr/X11R6/lib -lX11

all : libezx.a 

samples : $(SAMPLES)

libezx.a : ezxdisp.o
	ar -rs $@ $^

ezxdisp.o : ezxdisp.c ezxdisp.h
	$(CC) -I../../include $(CFLAGS) -c $<

$(SAMPLES) : libezx.a 
$(SAMPLES_OBJS) : ezxdisp.h

% : %.o
	$(CC) $(CFLAGS) $< -o $@ -L. -lezx $(X11_LIB) -lm

%.o : %.c
	$(CC) -I../../include $(CFLAGS) -c $<

clean :
	rm -f *.o *.a gmon.out $(SAMPLES)
