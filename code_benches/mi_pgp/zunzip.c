/*---------------------------------------------------------------------------

  unzip.c

  Highly butchered minimum unzip code for inflate.c

  ---------------------------------------------------------------------------*/

#include "zunzip.h"              /* includes, defines, and macros */
#include "language.h"            /* for LANG() */

#define VERSION  "v4.20p BETA of 2-18-92"

/**********************/
/*  Global Variables  */
/**********************/

#if 0
longint csize;        /* used by list_files(), ReadByte(): must be signed */
static longint ucsize;       /* used by list_files(), unReduce(),
				unImplode() */
#endif

ULONG mask_bits[] =
{0x00000000L,
 0x00000001L, 0x00000003L, 0x00000007L, 0x0000000fL,
 0x0000001fL, 0x0000003fL, 0x0000007fL, 0x000000ffL,
 0x000001ffL, 0x000003ffL, 0x000007ffL, 0x00000fffL,
 0x00001fffL, 0x00003fffL, 0x00007fffL, 0x0000ffffL,
 0x0001ffffL, 0x0003ffffL, 0x0007ffffL, 0x000fffffL,
 0x001fffffL, 0x003fffffL, 0x007fffffL, 0x00ffffffL,
 0x01ffffffL, 0x03ffffffL, 0x07ffffffL, 0x0fffffffL,
 0x1fffffffL, 0x3fffffffL, 0x7fffffffL, 0xffffffffL};

/*---------------------------------------------------------------------------
    Input file variables:
  ---------------------------------------------------------------------------*/

byte *inbuf = NULL, *inptr;     /* input buffer (any size is legal)
				   and pointer */
int incnt;

ULONG bitbuf;
int bits_left;
boolean zipeof;

int zipfd;               /* zipfile file handle */

/*---------------------------------------------------------------------------
    Output stream variables:
  ---------------------------------------------------------------------------*/

byte *outbuf;                   /* buffer for rle look-back */
byte *outptr;
byte *outout;                   /* scratch pad for ASCII-native trans */
longint outpos;                 /* absolute position in outfile */
int outcnt;                     /* current position in outbuf */
int outfd;

/*---------------------------------------------------------------------------
    unzip.c static global variables (visible only within this file):
  ---------------------------------------------------------------------------*/

static byte *hold;

/*******************/
/* Main unzip code */
/*******************/

int unzip( FILE *inFile, FILE *outFile )        /* return PK-type error code
						   (except under VMS) */
{
	int status = 0;
	outfd = fileno( outFile );
	zipfd = fileno( inFile );

	inbuf = (byte *) (malloc(INBUFSIZ + 4));    /* 4 extra for hold[]
						       (below) */
	outbuf = (byte *) (malloc(OUTBUFSIZ + 1));  /* 1 extra for string
						       termination */
	outout = outbuf;        /*  else just point to outbuf */

	if ((inbuf == NULL) || (outbuf == NULL) || (outout == NULL)) {
		fprintf(stderr, "error:  can't allocate unzip buffers\n");
		RETURN(4);              /* 4-8:  insufficient memory */
	}
	hold = &inbuf[INBUFSIZ];    /* to check for boundary-spanning
				       signatures */

	bits_left = 0;
	bitbuf = 0;
	outpos = 0L;
	outcnt = 0;
	outptr = outbuf;
	zipeof = 0;

	/* Set output buffer to initial value */
	memset(outbuf, 0, OUTBUFSIZ);

	/* Go from high- to low-level I/O */
	lseek( zipfd, ftell(inFile), SEEK_SET );

	if ((incnt = read(zipfd, (char *) inbuf,INBUFSIZ)) <= 0) {
		fprintf(stderr, LANG("\nERROR: unexpected end of compressed data input.\n"));
		status = -1;               /*  can still do next file   */
	}
	inptr = inbuf;

#if 0
	/* Read in implode information */
	csize = 1000L;			/* Dummy size just to get input bits */

	/* Get compressed, uncompressed file sizes */
	csize = ucsize = 1000000000L;	/* Make sure we can read in anything */
#endif
	if (status == 0)
		status = inflate();	/* Ftoomschk! */

	/* Flush output buffer before returning */
	if (status == 0 && FlushOutput())
		status = -1;
	free(inbuf);
	free(outbuf);
	inbuf = outbuf = outout = NULL;
	return(status);
}
