#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/xpm.h>
#include <X11/extensions/Xrender.h>
#include <signal.h>


extern int exit_flag;
extern int zoom;
extern char **empty_map;
extern unsigned xmax;
extern unsigned ymax;
extern int x_off;
extern int y_off;

int base_x, base_y;

GC gc;
XRenderPictFormat *format, *format_color;



typedef struct
{
  long flags;
  long functions;
  long decorations;
  long input_mode;
  long state;
} MotifWmHints;


#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_CLOSE          (1L << 5)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)


void decoration (Display *display,Window w) {

	MotifWmHints MotifWHints;
	Atom MotifHints  = None;

	MotifHints=XInternAtom( display,"_MOTIF_WM_HINTS",0 );
	if (MotifHints != None){
		memset(&MotifWHints,0,sizeof(MotifWHints) );
		MotifWHints.flags= MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
		MotifWHints.functions= MWM_FUNC_MOVE | MWM_FUNC_CLOSE | MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE;
		MotifWHints.decorations= 0 | (MWM_DECOR_MENU);
		XChangeProperty(display,w,MotifHints,MotifHints,32,
				PropModeReplace,(unsigned char *)&MotifWHints,5);
	}
}


void close_window(Display *display, Window w){
	XDestroyWindow(display,w);
	XFlush(display);
	XFreeGC(display,gc);
	XCloseDisplay(display);
}

void open_window(Display **display, Window *MyWindow, Window *rootWin){

	char *display_name = NULL;
	int screen;
	int tmp;
	XSetWindowAttributes att;
	

	*display = XOpenDisplay(display_name);

	if (display == NULL) {
		fprintf(stderr, "cannot connect to X server :0\n");
		exit(2);
	}
	/*
	if(!XShapeQueryExtension(*display,&tmp,&tmp)) {
		fprintf(stderr, "SHAPE extension not supported\n");
		exit(2);
	}
	*/
	if(!XRenderQueryExtension(*display,&tmp,&tmp)){
		fprintf(stderr, "XRender extension not supported\n");
		exit(2);
	}

	screen = DefaultScreen(*display);
	*rootWin = RootWindow(*display, screen);

	base_x = x_off + 0;
	base_y = DisplayHeight(*display, screen) - ((ymax * zoom) + y_off);
	
	att.override_redirect = True;
	*MyWindow = XCreateWindow(*display,RootWindow(*display,screen),
			base_x, base_y, xmax * zoom, ymax * zoom, 0,
			CopyFromParent, 	// depth
			CopyFromParent, 	// class
			CopyFromParent, 	// visual
			CWOverrideRedirect,     // valuemask
			&att);             	// attributes

	format = XRenderFindVisualFormat(*display, DefaultVisual(*display, screen));

	decoration(*display,*MyWindow);
	XMapWindow(*display,*MyWindow);
	XSelectInput(*display,*MyWindow,NoEventMask );
	XFlush(*display);
	XSync(*display,False);
	gc = XCreateGC(*display, *MyWindow,0,NULL);
}


void EraseGirl(Display *display, Window win, int zoom){
	
       XClearArea(display, win, 0, 0, xmax * zoom, ymax *zoom, False);
}

void DrawGirl(Display *display, Window win, Pixmap girlpixmap, Pixmap girlmaskpixmap, int zoom){
	
	/*
	Drawable d;
	Picture picture;
	
	d = girlpixmap;

	picture = XRenderCreatePicture (display, d, format, 0, 0);

	XSetClipMask(display, gc, girlmaskpixmap);
	XSetClipOrigin(display, gc, 0,0);
*/
	//XSetClipMask(display, gc, girlmaskpixmap);
	XCopyArea(display, girlpixmap, win, gc,0,0,xmax * zoom,ymax * zoom, 0,0);

}

void exit_vg(int signum){
	exit_flag = 1;
}
