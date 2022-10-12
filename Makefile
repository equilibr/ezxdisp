
all:
	cd src/x11 && make libezx.a samples

libezx.a:
	cd src/x11 && make libezx.a

clean:
	cd src/x11 && make clean
