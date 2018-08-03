extern int file_init PROTO((int argc,char *argv[]));
typedef void (*file_write_p) PROTO((int n, short *data));
typedef void (*file_term_p) PROTO((void));

extern file_write_p file_write;
extern file_term_p file_term;
