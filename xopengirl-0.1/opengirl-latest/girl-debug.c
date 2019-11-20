#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/xpm.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
//#include "empty.h"

/* taken from netpbm */
#define MAXCOLORS    256

#define MAXPRINTABLE 92         /* number of printable ascii chars
				 * minus \ and " for string compat
				 * and ? to avoid ANSI trigraphs. */

static char *printable = " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";

/* thanks guys !!!! */


unsigned char file_type[3];
unsigned char trash1[2];	/* addedd for future implementation */
unsigned char size[4];
unsigned char frames[2];
unsigned char trash2[3];	/* addedd for future implementation */
unsigned char frames2[2];	/* addedd for future implementation */
unsigned char trash3[21];	/* addedd for future implementation */
unsigned char frames3[2];	/* addedd for future implementation */
unsigned char trash4[21];	/* addedd for future implementation */

int count_color;
unsigned long color[256];
unsigned long xpm_color[256];
int charsPerPixel=0;

unsigned long *fptr;

char **empty_map;
int screen;
Display *display;
Window rootWin;
char *display_name = NULL;
GC gc;
int base_x, base_y;
Pixmap GirlPixmap;
Pixmap GirlMaskPixmap;

unsigned xmax = 160, ymax = 120;
char *bg = "\x00\x00\x00\x00";

char **gen_empty_map(){
	
	char **map, *ptr;
	char *info;
	char *pixmap;
	int k;
	int zoom=1;


	map = calloc(1,(sizeof(*map)) * (2 + (ymax * zoom)));
	pixmap = calloc(1,xmax * zoom * ymax * zoom);

	for(k=0; k <= ymax * zoom * zoom * xmax ; k++)
		pixmap[k] = ' ';

	ptr= info = calloc(1,30);
	sprintf(ptr,"%d %d 1 1",xmax * zoom ,ymax * zoom );
	map[0] = ptr;
	ptr+=15;
	sprintf(ptr," c None");
	map[1] = ptr;
	ptr = pixmap;
	for(k=0; k <= ymax * zoom ; k++, ptr+=(xmax * zoom))
		map[2 + k] = ptr;
	
	return map;
}

void Prepare_X(){

	int display_width, display_height;


	display = XOpenDisplay(display_name);
	if (display == NULL) {
		if (display_name == NULL) display_name = getenv("DISPLAY");
		(void) fprintf(stderr, ": cannot connect to X server %s\n", 
			       display_name ? display_name : "(default)");
		exit(1);
	}

	screen = DefaultScreen(display);
	rootWin = RootWindow(display, screen);
	display_width = DisplayWidth(display, screen);
	display_height = DisplayHeight(display, screen);
	base_x = 0; 
	base_y = display_height - ymax;
	gc = XCreateGC(display, rootWin,0,NULL);

}

void EraseGirl(){
	XClearArea(display, rootWin, base_x, base_y, xmax,ymax, False);
}


void DrawGirl() {

	int rc;

	rc = XSetClipMask(display, gc, GirlMaskPixmap);
	rc = XSetClipOrigin(display, gc, base_x,base_y);
	rc = XCopyArea(display, GirlPixmap, rootWin, gc, 
			0,0,xmax,ymax, base_x,base_y);


}


void DrawLastGirl() {

	int rc;
	Pixmap GirlPixmap;
	Pixmap GirlMaskPixmap;

	rc = XpmCreatePixmapFromData(display, rootWin, empty_map, &GirlPixmap, &GirlMaskPixmap, NULL);

	rc = XSetClipMask(display, gc, GirlMaskPixmap);
	rc = XSetClipOrigin(display, gc, base_x,base_y);
	rc = XCopyArea(display, GirlPixmap, rootWin, gc, 
			0,0,xmax,ymax, base_x,base_y);


}


unsigned char rdbyte(FILE *f){
	unsigned char b;
	
	int nread = fread(&b, 1, 1, f);
	
	if(!(nread == 1))
		perror("Short read");
	return b; 
} 

char *gen_numstr(int const input, int const digits) {
	char *str, *p;
	int d;
	int i;

	/* Allocate memory for printed number.  Abort if error. */
	if (!(str = (char *) malloc(digits + 1))){
		fprintf(stderr,"out of memory");
		exit(2);
	}
	i = input;
	/* Generate characters starting with least significant digit. */
	p = str + digits;
	*p-- = '\0';            /* nul terminate string */
	while (p >= str) {
		d = i % MAXPRINTABLE;
		i /= MAXPRINTABLE;
		*p-- = printable[d];
	}

	if (i != 0){
		fprintf(stderr,"Overflow converting %d to %d digits in base %d", input, digits, MAXPRINTABLE);
		exit(2);
	}

	return str;
}



char **make_xpm_frame(FILE *f, unsigned short n, off_t base,  char **color_map ){
	
	char *img_color, *img_pixmap;
	char *ptr;
	unsigned end = base + fptr[n+1];
	unsigned t, l, b, r;
	unsigned x, y;
	int i,k;
	char **data;
	unsigned long match_color;


	if( (ptr = img_color = calloc(1,15 + 13 * (count_color + 1) )) == NULL){
		perror("img_color");
		exit(2);
	}
	
	sprintf(ptr,"%d %d %d %d",xmax,ymax,count_color + 1,charsPerPixel);
	ptr+=15;
	sprintf(ptr,"%s c None",color_map[0]);
	ptr+=13;

	for(i=0; i < count_color ; i++, ptr+=13){
		sprintf(ptr,"%s c #%02X%02X%02X",color_map[i + 1],
					(unsigned int)( xpm_color[i] & 0x00FF0000) >> 16,
					(unsigned int)( xpm_color[i] & 0x0000FF00) >> 8, 
					(unsigned int)(xpm_color[i] & 0x000000FF));
	}

	if( (ptr = img_pixmap = calloc(1,ymax * xmax * charsPerPixel )) == NULL){
		perror("img_pixmap");
		exit(2);
	}
	for(k=0; k < ymax * xmax * charsPerPixel; k+=charsPerPixel){
		for(i=0; i < charsPerPixel; i++)
			img_pixmap[k + i] = color_map[0][i];
	}

	fseek(f, base + fptr[n], 0);
	t = rdbyte(f);
	b = rdbyte(f);
	l = rdbyte(f);
	r = rdbyte(f);

	x = l; 
	y = t;
	while (1) {
		char C = rdbyte(f);
		char N = C & 0x7f;
		if (C & 0x80) {         
			unsigned m;
			for (m = 0; m < N; m++) {
				unsigned char color2 = rdbyte(f);
				assert(x<xmax);
				assert(y<ymax);
				assert(x>=0);
				assert(y>=0);
				assert(color2 < 256);
				match_color = color[color2];
				for(k =0; k < (count_color + 1) && match_color != xpm_color[k] ; k++);
				memcpy(ptr + charsPerPixel * (xmax * y + x) ,color_map[k + 1],charsPerPixel);
				y++;
				if (y >= b) { y = t; x++; }
			}
		} else {
			unsigned m;
			for (m = 0; m < C; m++) {
				assert(x<xmax);
				assert(y<ymax);
				assert(x>=0);
				assert(y>=0);
				memcpy(ptr + charsPerPixel * (y * xmax + x),color_map[0],charsPerPixel);
				y++;
				if (y >= b) { y = t; x++; }
			}
		}
		if (ftell(f) >= end) break;
	}
	
	data = calloc(1,sizeof(*data) * (count_color + 1) * ymax);
	
	ptr = img_color;
	data[0] = ptr;
	ptr += 15;
	for(i=0; i < count_color + 1; i++, ptr+=13)
		data[i + 1] = ptr;
	ptr = img_pixmap;
	for(; i < (count_color + 1 ) * ymax ; i++, ptr+=(xmax * charsPerPixel)){
		data[i + 1] = ptr;
	}

	return data;
}
	

void write_file(int frame, char **data){

	char *filename;
	int posy,posx;
	FILE *o;

    	filename = malloc( 35);

	sprintf(filename, "pippo/frame%04d.xpm", frame);
	if ((o = fopen(filename, "w+")) == NULL) 
		perror("fopen");
	fprintf(o,"static char *noname[] = {\n");
	for (posy= 0; posy <= count_color + 1 ; posy++)
		fprintf(o,"\"%s\",\n",data[posy]);

	for (;posy <= count_color + 1 + ymax ; posy++){
		fprintf(o,"\"");
		for (posx= 0; posx < xmax ; posx++)
			fprintf(o,"%c",data[posy][posx]);
		fprintf(o,"\",\n");
	}
	fprintf(o,"};\n");
	fclose(o);

}

int main(int argc, char **argv) {

	FILE *f;
	int i, c=1,k;
	char buf[2], buf2[4];
	short int n_frame;
	unsigned frame ;
	off_t base_pos;
	char **color_map;
	unsigned long prev_fp = 0;
	int rc;
	char **data;
	
	empty_map = gen_empty_map();
	if ((f = fopen(argv[1],"r")) == NULL){
		perror("fopen");
		exit(1);
	}
	
	printf("Using %s\n",argv[1]);

	if(fread(file_type,1,sizeof(file_type),f) == 0)
		ferror(f);
	
	if(fread(trash1,1,sizeof(trash1),f) == 0)
		ferror(f);
	
	printf("1\t%d",trash1[1]);
	if(trash1[1] == 2)
		printf(" Strip\n");
	else 
		printf("\n");
	/* set color palette */
	  
	for(i=0; i<256; i++){
		if(fread(&(color[i]),1,sizeof(color[i]),f) == 0)
			ferror(f);
	for(k=0; k < count_color && color[k] != color[i] ; k++);
	if(k == count_color)
		memcpy(&(xpm_color[count_color++]),&(color[i]),sizeof(color[i]));
	}
	
	if(fread(size,1,sizeof(size),f) == 0 )
		ferror(f);

	xmax = size[0];
	ymax = size[2];

	if(fread(frames,1,sizeof(frames),f) == 0)
		ferror(f);
	memcpy(&n_frame,frames,2);

	if(fread(trash2,1,sizeof(trash2),f) == 0)
		ferror(f);
	
	printf("2\t%d\n",trash2[2]);

	i=3;
	while( (c != 0 ) && (( buf[0] != '\x00' ) || ( buf[1] != '\x90' )) ){
		if((c = fread(buf,1,sizeof(buf),f)) ==0)
			ferror(f);
		if((i == 3 ) )
			printf("%d\t%d\n",i, buf[0]);
		if((i == 4) )
			printf("%d\t%d\n",i, buf[1]);
			i++;
	}
	
	if ((fptr = calloc(1, n_frame * sizeof(unsigned long))) == NULL){
		perror("calloc");
		exit(2);
	}

	for (frame = 0; frame < n_frame; frame++) {
		if(fread(buf2,1,sizeof(buf2),f) == 0)
			ferror(f);
		memcpy(fptr+frame, buf2,sizeof(buf2));
		if ( (fptr[frame] < prev_fp) && frame > 0)
			fprintf(stderr, "Frame pointer %u descends.\n", frame);
		prev_fp = fptr[frame];
	}


	base_pos = ftell(f);
	Prepare_X();

	/* taken from netpbm */
	{
		int j;
		for (charsPerPixel = 0, j = (count_color + 1) ; j; charsPerPixel++)
			j /= MAXPRINTABLE;
	}
	
	/* count_color + 1 color added (alpha channel) */
	
	if ((color_map = calloc(1, (count_color + 1) * sizeof(*color_map))) == NULL){
		perror("calloc");
		exit(2);
	}
	/* generation of the color_map charset */
	for(i=0; i < (count_color + 1); i++)
	    color_map[i] = gen_numstr(i, charsPerPixel);
	

	for (frame = 0; frame < n_frame; frame++) {

		data = make_xpm_frame(f, frame, base_pos, color_map);
		/* write_file(frame,data); */
		rc = XpmCreatePixmapFromData(display, rootWin, data, &GirlPixmap, &GirlMaskPixmap, NULL);
		EraseGirl();
		DrawGirl();
		free(data);
	}
	DrawLastGirl();
	fclose(f);

	return 0;
}
