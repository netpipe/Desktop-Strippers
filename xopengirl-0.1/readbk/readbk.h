void die(const char *) ;
void die_bang(const char *) ;
void skip(FILE *, int ) ;
char *rd(FILE *, int ) ;
unsigned char rdbyte(FILE *) ;
char *make_canvas(unsigned , unsigned );
void read_pointer_table(FILE *, unsigned short ) ;
short int read_n_frames(FILE *);
void read_color_table(FILE *) ;
void check_magic(FILE *) ;
void create_output_dir(const char *) ;
void set_pixel_bgcolor(char *, unsigned, unsigned);
void set_pixel(char *, unsigned, unsigned, unsigned char);
void read_frame(FILE *, unsigned short, off_t, char *) ;
void write_frame_as_ppm(unsigned short, char *);

