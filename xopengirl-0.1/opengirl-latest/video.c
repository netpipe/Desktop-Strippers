#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <signal.h>


extern int exit_flag;
extern int zoom;
extern char **empty_map;
extern unsigned xmax;
extern unsigned ymax;
extern int x_off;
extern int y_off;

int base_x, base_y;


Display *display=0;
Window rootWin=0;
GC gc;

void set_img_size(){

	int display_width, display_height;
	int screen;
	
	
	screen = DefaultScreen(display);
	display_width = DisplayWidth(display, screen);
	display_height = DisplayHeight(display, screen);
	base_x = x_off + 0;
	base_y = display_height - ((ymax * zoom) + y_off);
}

void close_X(void){

	XFreeGC(display,gc);
	XCloseDisplay(display);
}

void Prepare_X(void){

	char *display_name = NULL;
	int screen;

	display = XOpenDisplay(display_name);
	if (display == NULL) {
		if (display_name == NULL) display_name = getenv("DISPLAY");
		(void) fprintf(stderr, ": cannot connect to X server %s\n", 
			       display_name ? display_name : "(default)");
		exit(1);
	}

	screen = DefaultScreen(display);
	rootWin = RootWindow(display, screen);
	gc = XCreateGC(display, rootWin, 0, NULL);

}


void EraseGirl(Display *display, Window rootWin, int zoom){
	XClearArea(display, rootWin, base_x, base_y, xmax * zoom, ymax *zoom, True);
}


void GetRootBG(Display *display, Window rootWin, Pixmap *pixmap, int zoom){

	int screen;

	screen = DefaultScreen(display);
	*pixmap = XCreatePixmap(display, rootWin, xmax * zoom, ymax * zoom, DefaultDepth(display,screen));
	XCopyArea(display, rootWin, 
			*pixmap, gc, 
			base_x, base_y,
			xmax * zoom, ymax * zoom, 
			0, 0);

}


void DrawBG(Display *display, Window rootWin, Pixmap bgmap, int zoom){

	XCopyArea(display, bgmap, rootWin, gc, 
			0, 0, 
			xmax * zoom, ymax * zoom, 
			base_x, base_y);

}


void DrawGirl(Display *display, Window rootWin, 
		Pixmap girlpixmap, Pixmap girlmaskpixmap, int zoom){

	XSetClipMask(display, gc, girlmaskpixmap);
	XSetClipOrigin(display, gc, base_x, base_y);
	XCopyArea(display, girlpixmap, rootWin, gc, 
			0, 0, 
			xmax * zoom, ymax * zoom, 
			base_x, base_y);

}

void exit_vg(int signum){
	exit_flag = 1;
}
