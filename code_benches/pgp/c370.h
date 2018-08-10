#ifndef C370_H
#define C370_H

#include <stdio.h>
#include <stddef.h>

void printhex( void *buf, int len );

extern unsigned char ebcdic_ascii[];
extern unsigned char ascii_ebcdic[];

void ebcdic2ascii( unsigned char *buf, unsigned size );
void ascii2ebcdic( unsigned char *buf, unsigned size );

/* dirent.h definitions */

#define NAMELEN     8

struct dirent {
   struct dirent *d_next;
   char   d_name[NAMELEN+1];
};

typedef struct _DIR {
   struct  dirent *D_list;
   struct  dirent *D_curpos;
   char            D_path[FILENAME_MAX];
} DIR;

DIR *          opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
void           rewinddir(DIR *dirp);
int            closedir(DIR *dirp);

/* sys/types.h definitions   */

#define  off_t      long
#define  mode_t     int

/* fcntl.h     definitions   */

#define  O_RDONLY   0x0001
#define  O_WRONLY   0x0002
#define  O_RDWR     0x0004

#define  O_CREAT    0x0100
#define  O_TRUNC    0x0200
#define  O_EXCL     0x0400
#define  O_APPEND   0x0800

#define  O_BINARY   0x1000
#define  O_MEMORY   0x2000
#define  O_RECORD   0x4000
#define  O_DCB      0x8000
extern unsigned char dcb_flags[];       /* used by fopen and O_DCB */
   /* with a leading comma, eg:  ",recfm=fb,lrecl=80,blksize=6160" */

int  open(const char *path, int access);

/* stdio.h         definitions   */

#define fseek myfseek
/* #define fread myfread */
int  fileno( FILE *fp );

/* unistd.h        definitions   */

int      access( const char *filename, int how );
int      close( int fd );
int      isatty( int fd );
off_t    lseek( int fd, off_t offset, int whence );
size_t   read( int fd, void *buf, size_t size );
int      unlink( char *filename );
int      write( int fd, void *buf, size_t len );

/* io.h        definitions   */

int  eof( int fd );
int  setmode(int fd, int access);
int  tell( int fd );

/* stat.h      definitions   */

struct stat {
  short  st_dev;
  short  st_ino;
  short  st_mode;
  short  st_nlink;
  int    st_uid;
  int    st_gid;
  long   st_size;
  long   st_atime;
  long   st_mtime;
  long   st_ctime;
  FILE   *fp;
  char   fname[FILENAME_MAX];
};

int    chmod( const char* filename, mode_t mode );
int    stat(const char *filename, struct stat *buf );
int    fstat(int fd, struct stat *buf );

#define S_IFMT       0xFFFF
#define _FLDATA(m)   (*(fldata_t *) &m)
#define S_ISDIR(m)   (_FLDATA(m).__dsorgPDSdir)
#define S_ISREG(m)   (_FLDATA(m).__dsorgPO | \
                      _FLDATA(m).__dsorgPDSmem | \
                      _FLDATA(m).__dsorgPS)
#define S_ISBLK(m)   (_FLDATA(m).__recfmBlk)
#define S_ISMEM(m)   (_FLDATA(m).__dsorgMem)

/* errno.h     definitions   */

#define ENOENT   -1

#endif /* C370_H */
