
/* readbk v1.00

   extract frames from .BK1 .BK2 .BK3 files and write out PPM still images
 
   Copyright (C) 2001  Mark-Jason Dominus (mjd-readbk+@plover.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

static char version[] = "readbk v1.00 - 20010429 mjd-readbk+@plover.com\n";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/wait.h>

/* For mkdir() */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
extern int errno;

#include <assert.h>

#define unless(c) if(!(c))
#include "readbk.h"

unsigned XMAX = 160, YMAX = 120;
char *bg = "\x00\x00\x00";
#include <ctype.h>
#define HEX2N(c) (assert(isascii(c) && isxdigit(c)), isdigit(c) ? (c) - '0' : tolower(c) - 'a' + 10)
unsigned outstanding = 0;

void die(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

void die_bang(const char *s) {
  fprintf(stderr, "%s: %s\n", s, strerror(errno));
  exit(1);
}

void skip(FILE *f, int b) 
{
  fseek(f, b, 1);
}

#define MAXREAD 1024
char *rd(FILE *f, int b) 
{
  static char buf[MAXREAD+1];
  unsigned nread;

  if (b > MAXREAD) die("Read too large");
  nread = fread(buf, 1, b, f);
  unless(nread == b) die("Short read");
  buf[b] = '\0';
  return buf;
}

unsigned char rdbyte(FILE *f) 
{
  unsigned char b;
  int nread = fread(&b, 1, 1, f);
  unless(nread == 1) die("Short read");
  return b;
}

unsigned char color[256*3];
unsigned long *fptr;
char *out;                      /* Name of output directory */

int main(int argc, char **argv) 
{
  FILE *f;
  short int n_frames, frame;
  off_t base_pos;

  if (strcmp(argv[1], "-b") == 0) {
    unsigned g = atoi(argv[2]);
    char *b = malloc(4);
    unsigned i;

    for (i=0; i<3; i++) {
      b[i] =  g * 17;
    }
    b[3] = '\0';
    bg = b;

    argc -= 2;
    argv += 2;
  }

  if (argc != 2) {
    fprintf(stderr, "Usage: %s [-b bgcolor] filename\n", argv[0]);
    exit(1);
  }

  if ((f = fopen(argv[1], "r")) == NULL)
    die_bang("Couldn't open input file");
  create_output_dir(argv[1]);
  check_magic(f);
  skip(f, 2);
  read_color_table(f);
  skip(f, 4);
  n_frames = read_n_frames(f);

  skip(f,3);
  { 
    char *b2;                   /* skip up to "\x00\x90"  */
    while ((b2 = rd(f, 2)) && (b2[0] != '\x00' || b2[1] != '\x90')) 
      /* do nothing */ ;
  }
  
  read_pointer_table(f, n_frames);

  base_pos = ftell(f);

  for (frame = 0; frame < n_frames; frame++) {
    char *canvas = make_canvas(XMAX, YMAX);
    read_frame(f, frame, base_pos, canvas);
    write_frame_as_ppm(frame, canvas);
    free(canvas);
    fprintf(stderr, "Frame %hu/%hu written.  (%u outstanding)\r",
            frame+1, n_frames, outstanding);
  }
  fprintf(stderr, "\n");
  fclose(f);
  exit(0);
}

void write_frame_as_ppm(unsigned short n, char *canvas)
{
  static char *filename;
  FILE *o;
  unsigned long pos;
  pid_t pid;

  if (filename == NULL) {
    filename = malloc(strlen(out) + 35);
    if (filename == NULL) 
      die("Out of memory for output filename");
  }

  pid = fork();
  if (pid == -1) die_bang("Couldn't fork");

  if (pid == 0) {               /* child */
    sprintf(filename, "ppmtogif > %s/frame%04hu 2>/dev/null", out, n);
    if ((o = popen(filename, "w")) == NULL) 
      die_bang("Couldn't open output frame file for writing");


    fprintf(o, "P6\n%u %u\n255\n", XMAX, YMAX);
    for (pos = 0; pos < XMAX*YMAX*3; pos++)
      putc(canvas[pos], o);

    fclose(o);
    exit(0);
  }
  outstanding++;

  /* parent */
  while (waitpid(-1, 0, WNOHANG) > 0) 
    outstanding-- ;
}

void read_frame(FILE *f, unsigned short n, off_t base, char *canvas) 
{
  unsigned end = base + fptr[n+1];
  unsigned t, l, b, r;
  unsigned x, y;

  fseek(f, base + fptr[n], 0);
  t = rdbyte(f);
  b = rdbyte(f);
  l = rdbyte(f);
  r = rdbyte(f);

  x = l; y = t;

  while (1) {
    char C = rdbyte(f);
    char N = C & 0x7f;
    if (C & 0x80) {             /* colored pixels */
      unsigned i;
      for (i = 0; i < N; i++) {
        unsigned char color = rdbyte(f);
        assert(x<XMAX);
        assert(y<YMAX);
        assert(x>=0);
        assert(y>=0);
        assert(color < 256);
        set_pixel(canvas, x, y++, color);
        if (y >= b) { y = t; x++; }
      }
    } else {                    /* background pixels */
      unsigned i;
      for (i = 0; i < C; i++) {
        assert(x<XMAX);
        assert(y<YMAX);
        assert(x>=0);
        assert(y>=0);
        set_pixel_bgcolor(canvas, x, y++);
        if (y >= b) { y = t; x++; }
      }
    }
    if (ftell(f) >= end) break;
  }
}

void set_pixel(char *canvas, unsigned x, unsigned y, 
               unsigned char color_table_index)
{
  memcpy(canvas + 3*(y*XMAX + x), color+color_table_index*3, 3);
}

void set_pixel_bgcolor(char *canvas, unsigned x, unsigned y)
{
  memcpy(canvas + 3*(y*XMAX + x), bg, 3);
}

char *make_canvas(unsigned x, unsigned y)
{
  char *c = malloc(x * y * 3);
  if (c == NULL) die("Out of memory for canvas allocation");
  memset(c, bg[0], x*y*3);
  return c;
}

void read_pointer_table(FILE *f, unsigned short n) 
{
  unsigned frame;
  unsigned long prev_fp = 0;

  if ((fptr = malloc(n * sizeof(unsigned long))) == NULL)
    die("Out of memory for frame pointer table");

  for (frame = 0; frame < n; frame++) {
    char *b4 = rd(f, 4);
    memcpy(fptr+frame, b4, 4);
    if (fptr[frame] < prev_fp && frame > 0)
      fprintf(stderr, "Frame pointer %u descends.\n", frame);
    prev_fp = fptr[frame];
  }
}

short int read_n_frames(FILE *f)
{
  short int n;
  char *s = rd(f, 2);
  memcpy(&n, s, 2);
  return n;
}

void read_color_table(FILE *f) 
{
  unsigned cidx;
  for (cidx = 0; cidx < 256; cidx++) {
    unsigned long c = 0xffffff & *(unsigned long *)rd(f, 4);
    unsigned 
      r = ((c&0xff0000)/0x10000),
      g = (c % 0x10000) & 0xff00,
      b = (c & 0xff) * 0x10000;
    unsigned cc = r+g+b;

    color[cidx*3+2] = (c & 0x0000ff) >>  0;
    color[cidx*3+1] = (c & 0x00ff00) >>  8;
    color[cidx*3+0] = (c & 0xff0000) >> 16;
  }
}

void check_magic(FILE *f) 
{
  char *mn = rd(f, 3);
  if (mn[0] != 'B' || mn[1] != 'K'
      || mn[2] != '1' && mn[2] != '2' && mn[2] != '3') {
    die("Bad magic number in file");
  }
}

void create_output_dir(const char *in) 
{ 
  char *dot;
  
  out = strdup(in);
  if ((dot = strchr(out, '.')) == NULL) {
    die("Filename did not have a suffix.");
  } else if (dot[1] != 'b' && dot[1] != 'B' ||
             dot[2] != 'k' && dot[2] != 'K' ||
             dot[3] != '1' && dot[3] != '2' && dot[3] != '3' ||
             dot[4] != '\0') {
    die("Filename had improper suffix; should be .bk1, .bk2, or .bk3");
  }
  *dot = '\0';
  unless (mkdir(out, 0777) == 0) {
    if (errno != EEXIST) {
      die_bang("mkdir");
    }
  }
}



