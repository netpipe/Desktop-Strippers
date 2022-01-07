
CFLAGS= -O # -Wall -g

default: readbk

readbk.o: readbk.c readbk.h

dist: readbk.tgz

readbk.tgz: readbk.c readbk.h COPYING Makefile COPYING.fr.html
	tar czvf readbk.tgz $^
