CC = gcc
CFLAGS= -O2  -Wall -g -c $(GTKFLAGS)
DESTDIR= 
INSTDIR=$(DESTDIR)/usr/bin
MANDIR=$(DESTDIR)/usr/share/man/man1

default: opengirl #girl-debug

opengirl: main.o video.o girl.o misc.o 
	gcc main.o girl.o video.o misc.o -L/usr/X11R6/lib -lXext -lX11 -lm -lXpm -o opengirl

girl-debug: girl-debug.o
	gcc girl-debug.o -L/usr/X11R6/lib -lXext -lX11 -lm -lXpm -o girl-debug

main.o: main.c
	$(CC) $(CFLAGS) main.c -o main.o

video.o: video.c
	$(CC) $(CFLAGS) video.c -o video.o

girl.o: girl.c 
	$(CC) $(CFLAGS) girl.c -o girl.o
	
misc.o: misc.c
	$(CC) $(CFLAGS) misc.c -o misc.o

girl-debug.o: girl-debug.c
	$(CC) $(CFLAGS) girl-debug.c -o girl-debug.o

clean:
	rm -rf *.o opengirl *core* girl-debug

distclean:
	$(MAKE) clean
	rm build-stamp configure-stamp debian/opengirl -rf
install:
	install opengirl $(INSTDIR)
	install opengirl.1 $(MANDIR)
	#install girl-debug $(INSTDIR)
	#install girl-debug.1 $(MANDIR)
