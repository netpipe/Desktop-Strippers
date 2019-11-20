CC = gcc
CFLAGS= -O2  -Wall -g -c $(GTKFLAGS)
DESTDIR= 
INSTDIR=$(DESTDIR)/usr/X11R6/bin
MANDIR=$(DESTDIR)/usr/X11R6/man/man1

default: Xopengirl 

Xopengirl: xmain.o xvideo.o xgirl.o xmisc.o 
	gcc xmain.o xgirl.o xvideo.o xmisc.o -L/usr/X11R6/lib -lXext -lX11 -lm -lXpm -lXrender -o Xopengirl

xmain.o: xmain.c
	$(CC) $(CFLAGS) xmain.c -o xmain.o

xmisc.o: xmisc.c
	$(CC) $(CFLAGS) xmisc.c -o xmisc.o

xgirl.o: xgirl.c 
	$(CC) $(CFLAGS) xgirl.c -o xgirl.o

xvideo.o: xvideo.c 
	$(CC) $(CFLAGS) xvideo.c -o xvideo.o

clean:
	rm -rf *.o Xopengirl *core*

distclean:
	$(MAKE) clean
	rm build-stamp configure-stamp debian/xopengirl -rf

install:
	install Xopengirl $(INSTDIR)
	install Xopengirl.1 $(MANDIR)
