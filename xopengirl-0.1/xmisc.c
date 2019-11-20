#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <X11/xpm.h>


extern void EraseGirl(Display *, Window, int );
extern void DrawGirl(Display *, Window, Pixmap, Pixmap, int);
extern void open_window(Display **, Window *, Window *);
extern void close_window(Display *, Window);
extern char **make_xpm_frame(unsigned long *, int, int, FILE *, unsigned short, off_t,char **, char **, char **);
extern int parse_file(unsigned long **, FILE *, short int *);
extern void exit_vg(int);


extern int exit_flag;
extern unsigned xmax; 
extern unsigned ymax;
extern int zoom; 
extern int writefile;
extern char **empty_map;

/* taken from netpbm */
/* thanks guys !!!! */
#define MAXPRINTABLE 92         
static char *printable = " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";

char *outdir;
int next_girl_flag;
int is_stripping=0;



void next_girl(int signum){
	next_girl_flag = 1;
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

void write_file(char *outdir, int frame,int count_color, char **data){

	char *filename;
	int posy,posx;
	FILE *o;

    	filename = malloc(35);

	sprintf(filename, "%s/frame%04d.xpm",outdir, frame);
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
	free(filename);

}

void virtua_help(char *arg){

	fprintf(stderr,"usage %s -s secs -d dir \n",arg);
	fprintf(stderr,"or %s file1 file2 .... \n",arg);
	fprintf(stderr,"-d directory specify the dir in which resides virtua girl animation\n");
	fprintf(stderr,"-o outdir (if you want to save them \n");
	fprintf(stderr,"-s seconds to sleep ( default 180)  \n");
	fprintf(stderr,"-x num: x-offset from bottom-left of the screen\n");
	fprintf(stderr,"-y num: y-offset from bottom-left of the screen\n");
	fprintf(stderr,"-z zoom them\n");
	fprintf(stderr,"-h: shows help\n");
	fprintf(stderr,"\n");
	exit(0);
}

void get_info_anim(char *filename){
	
	char info_file[128];
	char *name, *type;
	char buf[128];
	FILE *f;
	int i;
	
	printf("using %s\n",filename);
	for(i=0; ((f = fopen(info_file,"r")) == NULL) && i < strlen(filename); i++ ){
		strncpy(info_file,filename,strlen(filename));
		info_file[strlen(filename)-i] = '\0';
		sprintf(info_file,"%s.inf",info_file);
	}
	if ((f = fopen(info_file,"r")) != NULL){
		fread(buf,1,128,f);
		name = strtok(buf,",");
		type = strtok(NULL,",");

		printf("\n%s  series \"%s\"",name,type);
		if(is_stripping != 0)
			printf(" (Striptease)\n\n");
		else
			printf("\n\n");
		fclose(f);
	}

}


void main_loop( char *filename) {

	int i, rc, count_color;
	FILE *f;
	char buf2[4];
	unsigned frame ;
	off_t base_pos;
	char **color_map, **data ;
	unsigned long prev_fp = 0, *fptr;
	int charsPerPixel=0;
	short int n_frame=0;
	char *img_color=NULL , *img_pixmap=NULL ;
 	Pixmap GirlPixmap, GirlMaskPixmap;
	long delay;
	struct timeval oldtime, newtime;
	struct timezone tz1, tz2;
	Display *display;
	Window window, rootWin;

	
	if ((f = fopen(filename,"r")) == NULL){
		perror("fopen");
		exit(1);
	}

	count_color = parse_file(&fptr, f, &n_frame);
	get_info_anim(filename);

	for (frame = 0; frame < n_frame; frame++) {
		if(fread(buf2,1,sizeof(buf2),f) == 0)
			ferror(f);
		memcpy(fptr+frame, buf2,sizeof(buf2));
		if ( (fptr[frame] < prev_fp) && frame > 0)
			fprintf(stderr, "Frame pointer %u descends.\n", frame);
		prev_fp = fptr[frame];
	}


	base_pos = ftell(f);
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


	open_window(&display, &window, &rootWin);
	for (frame = 0; frame < n_frame && exit_flag != 1 && next_girl_flag != 1; frame++) {
		gettimeofday(&oldtime,&tz1);
		data = make_xpm_frame(fptr, charsPerPixel, count_color, f, frame, base_pos, color_map, &img_color, &img_pixmap);
		if(writefile == 1)
			write_file(outdir, frame,count_color, data);
		rc = XpmCreatePixmapFromData(display, window, data, &GirlPixmap, &GirlMaskPixmap, NULL);
		EraseGirl(display, window, zoom);
		DrawGirl(display, window, GirlPixmap, GirlMaskPixmap, zoom);
		XFreePixmap(display,GirlPixmap);
		XFreePixmap(display,GirlMaskPixmap);
		free(data);
		free(img_color);
		free(img_pixmap);
		gettimeofday(&newtime,&tz2);
		if (next_girl_flag != 1 && exit_flag != 1 && (delay = (newtime.tv_sec * 1000000 + newtime.tv_usec)  - ( oldtime.tv_sec * 1000000 + oldtime.tv_usec)) < 80000 )
			usleep(80000 - delay );
	}
	EraseGirl(display, window, zoom);
	rc = XpmCreatePixmapFromData(display, window, empty_map, &GirlPixmap, &GirlMaskPixmap, NULL);
	DrawGirl(display, window, GirlPixmap, GirlMaskPixmap, zoom);
	XFreePixmap(display,GirlPixmap);
	XFreePixmap(display,GirlMaskPixmap);
	fclose(f);
	free(color_map);
	free(fptr);
	next_girl_flag = 0;
	close_window(display, window);
}
