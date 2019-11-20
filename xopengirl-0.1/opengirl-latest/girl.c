#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


extern void set_img_size(void);

extern unsigned xmax; 
extern unsigned ymax;
extern int zoom;
extern int is_stripping;

unsigned long color[256];
unsigned long xpm_color[256];


char **make_xpm_frame(unsigned long *fpntr, int charsperpixel, int count_color, 
		FILE *f, unsigned short n, off_t base,  char **color_map, char **img_color, char **img_pixmap ){
	
	char *ptr;
	unsigned end = base + fpntr[n+1];
	unsigned char t, l, b, r;
	unsigned x, y;
	int i,k, yzoom, xzoom;
	char **data;
	unsigned long match_color;


	if( (ptr = *img_color = calloc(1,15 + 13 * (count_color + 1) )) == NULL){
		perror("img_color");
		exit(2);
	}
	sprintf(ptr,"%d %d %d %d",xmax * zoom ,ymax * zoom ,count_color + 1,charsperpixel);
	ptr+=15;
	sprintf(ptr,"%s c None",color_map[0]);
	ptr+=13;

	for(i=0; i < count_color ; i++, ptr+=13){
		sprintf(ptr,"%s c #%02X%02X%02X",color_map[i + 1],
					(unsigned int) (xpm_color[i] & 0x00FF0000) >> 16,
					(unsigned int) (xpm_color[i] & 0x0000FF00) >> 8, 
					(unsigned int) (xpm_color[i] & 0x000000FF));
	}

	if( (ptr = *img_pixmap = calloc(1,xmax * zoom * ymax * zoom * charsperpixel )) == NULL){
		perror("img_pixmap");
		exit(2);
	}
	for(k=0; k < ymax * zoom * xmax * zoom * charsperpixel ; k+=charsperpixel){
		for(i=0; i < charsperpixel ; i++)
			(*img_pixmap)[k + i] = color_map[0][i];
	}

	fseek(f, base + fpntr[n], 0);
	if (fread(&t,sizeof(t),1,f) != 1){
		perror("fread t");
		exit(2);
	}
	if (fread(&b,sizeof(b),1,f) != 1){
		perror("fread b");
		exit(2);
	}
	if (fread(&l,sizeof(l),1,f) != 1){
		perror("fread l");
		exit(2);
	}
	if (fread(&r,sizeof(r),1,f) != 1){
		perror("fread r");
		exit(2);
	}

	x = (l * zoom); 
	y = (t * zoom);
	while (1) {
		//char C = rdbyte(f);
		//char N = C & 0x7f;
		char C, N;
		if (fread(&C,sizeof(C),1,f) != 1){
			perror("fread C");
			exit(2);
		}
		N = C & 0x7f;
		if (C & 0x80) {         
			unsigned m;
			for (m = 0; m < N; m++) {
				//unsigned char color2 = rdbyte(f);
				unsigned char color2;
				if (fread(&color2,sizeof(color2),1,f) != 1){
					perror("fread color2");
					exit(2);
				}
				if(( 0 <=  x < (xmax * zoom)) && (0 <= y < (ymax * zoom))){
					if(color2 < 256)
						match_color = color[color2];
					for(k =0; k < (count_color + 1) && match_color != xpm_color[k] ; k++);
					for(yzoom =0; yzoom < zoom ; yzoom++){
						for(xzoom =0; xzoom < zoom ; xzoom++){
							memcpy(ptr + ((xmax * zoom) * (y + yzoom) + xzoom + x) * charsperpixel ,color_map[k + 1],charsperpixel);
						}
					}
					y+=zoom;
				}
				if (y >= (b * zoom)){ 
					y = (t * zoom); 
					x+= zoom; 
				}
			}
		} else {
			unsigned m;
			for (m = 0; m < C; m++) {
				if(( 0 <=  x < (xmax * zoom)) && (0 <= y < (ymax * zoom))){
					for(yzoom =0; yzoom < zoom ; yzoom++){
						for(xzoom =0; xzoom < zoom ; xzoom++){
							memcpy(ptr + ((xmax * zoom) * (y + yzoom) + xzoom + x) * charsperpixel ,color_map[0],charsperpixel);
						}
					}
				}
				y+=zoom;
				if (y >= (b * zoom)){ 
					y = (t * zoom);
					x+= zoom; 
				}
			}
		}
		if (ftell(f) >= end) break;
	}
	data = calloc(1,sizeof(*data) * (count_color + 3 + (ymax * zoom)) );
	
	ptr = *img_color;
	data[0] = ptr;
	ptr += 15;
	for(i=0; i < count_color + 1; i++, ptr+=13){
		data[i + 1] = ptr;
	}
	ptr = *img_pixmap;
	for(; i <= (count_color + 1 + (ymax * zoom)) ; i++, ptr+=(xmax * zoom * charsperpixel)){
		data[i + 1] = ptr;
	}
	return data;
}
	

int parse_file(unsigned long **fpntr, FILE *f, short int *frame){

	int i, c=1,k, ccolor=0;
	char buf[2];
	unsigned char file_type[3];
	unsigned char strip[2];	
	unsigned char size[4];	
	unsigned char frames[2];
	unsigned char trash3[3];	/* addedd for future implementation */

	bzero(file_type,sizeof(file_type));
	if(fread(file_type,1,sizeof(file_type),f) == 0)
		ferror(f);
	
	if(fread(strip,1,sizeof(strip),f) == 0)
		ferror(f);
	
	if(strip[1] != 0)
		is_stripping++;
	else 
		is_stripping=0;
	/* set color palette */
	  
	bzero(xpm_color,256);
	bzero(color,256);

	for(i=0; i<256; i++){
		if(fread(&(color[i]),1,sizeof(color[i]),f) == 0)
			ferror(f);
		for(k=0; k < ccolor && color[k] != color[i] ; k++);
		if(k == ccolor)
			memcpy(&(xpm_color[ccolor++]),&(color[i]),sizeof(color[i]));
	}
	
	if(fread(size,1,sizeof(size),f) == 0 )
		ferror(f);

	xmax = size[0];
	ymax = size[2];
	set_img_size();

	if(fread(frames,1,sizeof(frames),f) == 0)
		ferror(f);
	memcpy(frame,frames,2);

	if(fread(trash3,1,sizeof(trash3),f) == 0)
		ferror(f);

	while( (c != 0 ) && (( buf[0] != '\x00' ) || ( buf[1] != '\x90' )) ){
		if((c = fread(buf,1,sizeof(buf),f)) ==0)
			ferror(f);
	}
	
	if ((*fpntr = calloc(1, ( *frame +1 ) * sizeof(unsigned long))  ) == NULL){
		perror("calloc on frame:");
		exit(2);
	}
	return ccolor;
}
