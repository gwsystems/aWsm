#include <config.h>
#include "proto.h"
#include <stdio.h>
#include <fcntl.h>
#include <useconfig.h>
#include "getargs.h"
#include "l2u.h"
#include "hplay.h"
#include "file.h"

#define SUN_MAGIC 	0x2e736e64		/* Really '.snd' */
#define SUN_HDRSIZE	24			/* Size of minimal header */
#define SUN_UNSPEC	((unsigned)(~0))	/* Unspecified data size */
#define SUN_ULAW	1			/* u-law encoding */
#define SUN_LIN_8	2			/* Linear 8 bits */
#define SUN_LIN_16	3			/* Linear 16 bits */

file_write_p file_write = NULL;
file_term_p  file_term  = NULL;

static char *linear_file;
static char *au_file;
static int au_fd = -1;            /* file descriptor for .au ulaw file */
static int linear_fd = -1;

static unsigned au_encoding = SUN_ULAW;
static unsigned au_size = 0;

static void wblong PROTO((int fd, unsigned long x));
static void 
wblong(fd, x)
int fd;
unsigned long x;
{
 int i;
 for (i = 24; i >= 0; i -= 8)
  {
   char byte = (char) ((x >> i) & 0xFF);
   write(fd, &byte, 1);
  }
}

extern void au_header PROTO((int fd,unsigned enc,unsigned rate,unsigned size,char *comment));

void 
au_header(fd, enc, rate, size, comment)
int fd;
unsigned enc;
unsigned rate;
unsigned size;
char *comment;
{
 if (!comment)
  comment = "";
 wblong(fd, SUN_MAGIC);
 wblong(fd, SUN_HDRSIZE + strlen(comment));
 wblong(fd, size);
 wblong(fd, enc);
 wblong(fd, rate);
 wblong(fd, 1);                   /* channels */
 write(fd, comment, strlen(comment));
}

static void aufile_write PROTO((int n,short *data));

static void
aufile_write(n, data)
int n;
short *data;
{
 if (n > 0)
  {
   if (linear_fd >= 0)
    {
     unsigned size = n * sizeof(short);
     if (write(linear_fd, data, n * sizeof(short)) != size)
            perror("write");
    }
   if (au_fd >= 0)
    {
     if (au_encoding == SUN_LIN_16)
      {
       unsigned size = n * sizeof(short);
       if (write(au_fd, data, size) != size)
        perror("write");
       else
        au_size += size;
      }
     else if (au_encoding == SUN_ULAW)
      {
       unsigned char *plabuf = (unsigned char *) malloc(n);
       if (plabuf)
        {
         unsigned char *p = plabuf;
         unsigned char *e = p + n;
         while (p < e)
          {
           *p++ = short2ulaw(*data++);
          }
         if (write(au_fd, plabuf, n) != n)
          perror(au_file);
         else
          au_size += n;
         free(plabuf);
        }
       else
        {
         fprintf(stderr, "%s : No memory for ulaw data\n", program);
        }
      }
     else
      {
       abort();
      }
    }
  }
}

static void aufile_term PROTO((void));

static void
aufile_term()
{
 /* Finish ulaw file */
 if (au_fd >= 0)
  {
   off_t here = lseek(au_fd, 0L, SEEK_CUR);
   if (here >= 0)
    {
     /* can seek this file - truncate it */
     ftruncate(au_fd, here);
     /* Now go back and overwite header with actual size */
     if (lseek(au_fd, 8L, SEEK_SET) == 8)
      {
       wblong(au_fd, au_size);
      }
    }
   if (au_fd != 1)
    close(au_fd);
   au_fd = -1;
  }
 /* Finish linear file */
 if (linear_fd >= 0)
  {
   ftruncate(linear_fd, lseek(linear_fd, 0L, SEEK_CUR));
   if (linear_fd != 1)
    close(linear_fd);
   linear_fd = -1;
  }
}


int
file_init(argc, argv)
int argc;
char *argv[];
{
 argc = getargs("File output", argc, argv,
                "l", "", &linear_file, "Raw 16-bit linear pathname",
                "o", "", &au_file,     "Sun/Next audio file name",
                NULL);
 if (help_only)
  return argc;

 if (au_file)
  {
   if (strcmp(au_file, "-") == 0)
    {
     au_fd = 1;                   /* stdout */
    }
   else
    {
     au_fd = open(au_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
     if (au_fd < 0)
      perror(au_file);
    }
   if (au_fd >= 0)
    {
     if (samp_rate > 8000)
      au_encoding = SUN_LIN_16;
     else
      au_encoding = SUN_ULAW;
     au_header(au_fd, au_encoding, samp_rate, SUN_UNSPEC, "");
     au_size = 0;
    }
  }

 if (linear_file)
  {
   if (strcmp(linear_file, "-") == 0)
    {
     linear_fd = 1 /* stdout */ ;
    }
   else
    {
     linear_fd = open(linear_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
     if (linear_fd < 0)
      perror(linear_file);
    }
  }
 if (au_fd >= 0 || linear_fd >= 0)
  {
   file_write = aufile_write;
   file_term  = aufile_term;
  }
 return argc;
}


