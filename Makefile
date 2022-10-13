
all:
	cd src/x11 && make libezx.a samples

libezx.a:
	cd src/x11 && make libezx.a

install: libezx.a
	sudo cp include/ezxdisp.h /usr/local/include
	sudo cp src/x11/libezx.a /usr/local/lib

clean:
	cd src/x11 && make clean
