#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <X11/xpm.h>
#include <signal.h>


extern int main_loop(char *);
extern void virtua_help(char *);
extern void exit_vg(int );
extern void next_girl(int );

typedef struct list_t{
	struct list_t *succ;
	char filename[PATH_MAX + FILENAME_MAX];
} filelist_t;


int zoom=1, writefile=0;
int exit_flag = 0;
int x_off=0, y_off=0;
unsigned xmax = 160 , ymax = 140;
char *outdir;
char **empty_map;

extern Display *display;
extern Window rootWin;

char **gen_empty_map(){
	
	char **map, *ptr;
	char *info;
	char *pixmap;
	int k;


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

int main(int argc, char **argv){
	int i, counter,k;
	char *directory = NULL;
	DIR *girl_dir;
	struct dirent *thedir;
	int secs = 180;
	filelist_t *base = NULL, *filename = NULL, *newitem;
	int X,Y;
	

	while ((i = getopt(argc, argv, "X:Y:d:s:o:x:y:zh")) != EOF){
		switch (i){
			case 'd':
				directory = optarg;      /* type of DOS */
				break;
			case 's':
				secs = atoi(optarg);
				break;
			case 'x':
				x_off = atoi(optarg);
				break;
			case 'y':
				y_off = atoi(optarg);
			case 'X':
				X = atoi(optarg);
				x_off = x_off - X;
				break;
			case 'Y':
				Y = atoi(optarg);
				y_off = y_off - Y;
				break;
			case 'o':
				writefile = 1;
				outdir = optarg;
				break;
			case 'h':               /* quiet mode */
				virtua_help(argv[0]);
				break;
			case 'z':
				zoom++;
				break;
			default :
				virtua_help(argv[0]);
				break;
		}
	}
	if(argc == 1)
		virtua_help(argv[0]);
	
	exit_flag = 0;
	empty_map = gen_empty_map();
	signal(SIGINT,exit_vg);
	signal(SIGUSR1,next_girl);

	if (directory == NULL){
		for(i=optind ; i < argc  && exit_flag != 1 ; i++){
			main_loop(argv[i]);
			sleep(secs);
		}
	} else {
		if ((girl_dir = opendir(directory)) == NULL){	
			perror("opendir");
			exit(2);
		}
		i=0;
		while( (thedir = readdir(girl_dir)) != NULL){
			
			/* no hidden files */
			if(thedir->d_name[0] != '.' && (
					thedir->d_name[strlen(thedir->d_name) - 1] == '1' ||
					thedir->d_name[strlen(thedir->d_name) - 1] == '2' ||
					thedir->d_name[strlen(thedir->d_name) - 1] == '3') &&
					(
					thedir->d_name[strlen(thedir->d_name) - 2] == 'k' ||
					thedir->d_name[strlen(thedir->d_name) - 2] == 'K' ) 
					
						){ 
				/* file */
				if(thedir->d_type == 8){
					newitem = malloc(sizeof(filelist_t));
					sprintf(newitem->filename,"%s/%s",directory,thedir->d_name);
					if(filename == NULL){
						base = newitem;
					} else {
						filename->succ = newitem;
					}
					filename = newitem;
					i++;
				}
			}
		}
		closedir(girl_dir);
		srand(time(NULL));
		while(exit_flag != 1){
			counter = ( rand() % i);
			newitem = base;
			for(k=0; k != counter ; k++){
				newitem = newitem->succ;
			}
			main_loop(newitem->filename);
			if(exit_flag !=1)
				sleep(secs);
		}
	}
	
	return 0;
}
