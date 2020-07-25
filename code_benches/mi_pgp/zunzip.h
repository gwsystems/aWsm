/*---------------------------------------------------------------------------

  unzip.h

  This header file is used by all of the unzip source files.  Its contents
  were divided into seven more-or-less separate sections but have been
  hacked to death to minimize the total size and maximize compile speed by
  not including stuff not needed for the inflate-only compile.

  Modified 25 Jun 92 - HAJK
  Fix support for use in PGP/VMS
  ---------------------------------------------------------------------------*/

/***************************/
/*  OS-Dependent Includes  */
/***************************/

#include	<stdlib.h>
#include        <unistd.h>
#include	"usuals.h"
#include	"system.h"

#ifndef MINIX            /* Minix needs it after all the other includes (?) */
#  include <stdio.h>     /* this is your standard header for all C compiles */
#endif

/*---------------------------------------------------------------------------
    Next, a word from our Unix (mostly) sponsors:
  ---------------------------------------------------------------------------*/

#ifdef UNIX
#  ifdef AMIGA
#    include <libraries/dos.h>
#  else /* !AMIGA */
#    ifndef NO_PARAM_H
#      include <sys/param.h>   /* conflict with <sys/types.h>, some systems? */
#    endif /* !NO_PARAM_H */
#  endif /* ?AMIGA */

#  ifndef BSIZE
#    ifdef MINIX
#      define BSIZE   1024
#    else /* !MINIX */
#      define BSIZE   DEV_BSIZE  /* assume common for all Unix systems */
#    endif /* ?MINIX */
#  endif

#  ifndef BSD
#    if !defined(AMIGA) && !defined(MINIX)
#      define NO_MKDIR           /* for mapped_name() */
#    endif /* !AMIGA && !MINIX */
#    include <time.h>
#  else   /* BSD */
#    include <sys/time.h>
#    include <sys/timeb.h>
#  endif

#else   /* !UNIX */
#  define BSIZE   512               /* disk block size */
#endif /* ?UNIX */

/*---------------------------------------------------------------------------
    And now, our MS-DOS and OS/2 corner:
  ---------------------------------------------------------------------------*/

#if defined(__TURBOC__) && !defined(__PUREC__)
#  define DOS_OS2             /* Turbo C under DOS, MSC under DOS or OS2    */
#  ifndef __BORLANDC__        /* There appears to be a bug ?? in Borland's  */
#	include <alloc.h>
#  endif
#else                         /* NOT Turbo C...                             */
#  ifdef MSDOS                /*   but still MS-DOS, so we'll assume it's   */
#    ifndef MSC               /*   Microsoft's compiler and fake the ID, if */
#      define MSC             /*   necessary (it is in 5.0; apparently not  */
#    endif                    /*   in 5.1 and 6.0)                          */
#    include <dos.h>          /* _dos_setftime()                            */
#  endif
#  ifdef OS2                  /* stuff for DOS and OS/2 family version */
#    ifndef MSC
#      define MSC
#    endif
#    define INCL_BASE
#    define INCL_NOPM
#    include <os2.h>          /* DosQFileInfo(), DosSetFileInfo()? */
#  endif
#endif

#ifdef MSC                    /* defined for all versions of MSC now         */
#  define DOS_OS2             /* Turbo C under DOS, MSC under DOS or OS/2    */
#  if defined(_MSC_VER) && (_MSC_VER >= 600)      /* new with 5.1 or 6.0 ... */
#    undef DECLARE_ERRNO      /* errno is now a function in a dynamic link   */
#  endif                      /*   library (or something)--incompatible with */
#endif                        /*   the usual "extern int errno" declaration  */

#ifdef DOS_OS2                /* defined for both Turbo C, MSC */
#  include <io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#  include <time.h>           /* localtime() */
#endif

/*---------------------------------------------------------------------------
    Followed by some VMS (mostly) stuff:
  ---------------------------------------------------------------------------*/

#ifdef VMS
#  include <time.h>             /* the usual non-BSD time functions */
#  include <file.h>             /* same things as fcntl.h has */
#  include <rmsdef.h>           /* RMS error codes */
#  define UNIX			/* can share most of same code from now on */
#  define RETURN   return	/* Don't Fake return	*/
#else /* !VMS */
#  define RETURN   return       /* only used in main() */
#  ifdef V7
#    define O_RDONLY  0
#    define O_WRONLY  1
#    define O_RDWR    2
#  else /* !V7 */
#    ifdef MTS
#      include <sys/file.h>     /* MTS uses this instead of fcntl.h */
#    else /* !MTS */
#      ifdef COHERENT           /* Coherent 3.10/Mark Williams C */
#        include <sys/fcntl.h>
#        define SHORT_NAMES
#        define tzset  settz
#      else /* !COHERENT */
#       ifndef __PUREC__
#        ifndef C370             /* not defined in C370 library */
#         include <fcntl.h>      /* #define O_BINARY 0x8000 (no CR/LF */
#        endif /* !C370 */
#       endif
#      endif /* ?COHERENT */    /*   translation), as used in open() */
#    endif /* ?MTS */
#  endif /* ?V7 */
#endif /* ?VMS */

/*---------------------------------------------------------------------------
    And some Mac stuff for good measure:
  ---------------------------------------------------------------------------*/

#ifdef MACTC5
#  define MACOS
#  ifndef __STDC__            /* THINK_C isn't truly ANSI-standard, */
#    define __STDC__ 1        /*   but it understands prototypes...so */
#  endif                      /*   it's close enough for our purposes */
#  include <time.h>
#  include <unix.h>
#endif
#ifdef MPW                    /* not tested yet - should be easy enough tho */
#  define MACOS
#  include <time.h>
#  include <fcntl.h>
#  include "macstat.h"
#endif

/*---------------------------------------------------------------------------
    And finally, some random extra stuff:
  ---------------------------------------------------------------------------*/

#include <stdlib.h>      /* standard library prototypes, malloc(), etc. */
#include <string.h>      /* defines strcpy, strcmp, memcpy, etc. */

#ifdef MINIX
#  include <stdio.h>
#endif


/*************/
/*  Defines  */
/*************/

#define INBUFSIZ          BUFSIZ   /* same as stdio uses */
#define OUTBUFSIZ       0x2000     /* unImplode needs power of 2, >= 0x2000 */

#define MAX_BITS      13                 /* used in unShrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */

#ifdef EBCDIC             /* already defined in usuals.h */
#undef LF
#undef CR
#else
#define LF   10   /* '\n' on ASCII machines.  Must be 10 due to EBCDIC */
#define CR   13   /* '\r' on ASCII machines.  Must be 13 due to EBCDIC */
#endif

#ifdef AMIGA
#  define FFLUSH    fflush(stderr);
#else /* !AMIGA */
#  define FFLUSH
#endif /* ?AMIGA */

#ifndef TRUE
#  define TRUE      1   /* sort of obvious */
#  define FALSE     0
#endif

#ifndef SEEK_SET        /* These should all be declared in stdio.h!  But   */
#  define SEEK_SET  0   /*  since they're not (in many cases), do so here. */
#  define SEEK_CUR  1
#  define SEEK_END  2
#endif

/**************/
/*  Typedefs  */
/**************/

typedef long             longint;
typedef unsigned short   UWORD;
#ifndef OS2
typedef unsigned long    ULONG;
#endif

/*************************/
/*  Function Prototypes  */
/*************************/

#ifdef PROTO

/* The following is for non-ansi compilers supporting prototypes */
/* (e.g. old SGI compilers and Borland C in non-ansi mode */

#define __(X)   X   /*  Inc.  Should probably give them a call and see   */
#endif

#ifndef __              /* This amusing little construct was swiped without  */
#  if __STDC__          /*  permission from the fine folks at Cray Research, */
#    define __(X)   X   /*  Inc.  Should probably give them a call and see   */
#  else                 /*  if they mind, but....  Then again, I can't think */
#    define __(X)   ()  /*  of any other way to do this, so maybe it's an    */
#  endif                /*  algorithm?  Whatever, thanks to CRI.  (Note:     */
#endif                  /*  keep interior stuff parenthesized.)              */
/*
 * Toad Hall Note:  Not to worry:  I've seen this somewhere else too,
 * so obviously it's been stolen more than once.
 * That makes it public domain, right?
 */

/*---------------------------------------------------------------------------
    Functions in file_io.c and crypt.c:
  ---------------------------------------------------------------------------*/

int    open_input_file           __( (void) );
int    readbuf                   __( (char *buf, register unsigned size) );
int    create_output_file        __( (void) );
#if 0
int    FillBitBuffer             __( (void) );
int    ReadByte                  __( (UWORD *x) );
#else
int    FillInBuf                 __( (void) );
#endif
int    FlushOutput               __( (void) );

/*---------------------------------------------------------------------------
    Uncompression functions (all internal compression routines, enclosed in
    comments below, are prototyped in their respective files and are invisi-
    ble to external functions):
  ---------------------------------------------------------------------------*/

int inflate			__( (void) );            /* inflate.c */
int unzip			__( ( FILE *inFile, FILE *outFile ) );

/************/
/*  Macros  */
/************/

#ifndef min    /* MSC defines this in stdlib.h */
#  define min(a,b)   ((a) < (b) ? (a) : (b))
#endif

#define OUTB(intc) {*outptr++=intc; if (++outcnt==OUTBUFSIZ) FlushOutput();}

/*
 *  macro OUTB(intc)
 *  {
 *      *outptr++ = intc;
 *      if (++outcnt == OUTBUFSIZ)
 *          FlushOutput();
 *  }
 *
 */


#if 0

#define READBIT(nbits,zdest) \
	do \
	{ \
		if(nbits>bits_left) \
			FillBitBuffer(); \
		zdest=(int)(bitbuf&mask_bits[nbits]); \
		bitbuf>>=nbits; \
		bits_left-=nbits; \
	} while(0)

/*
 * macro READBIT(nbits,zdest)
 *  {
 *      if (nbits > bits_left)
 *          FillBitBuffer();
 *      zdest = (int)(bitbuf & mask_bits[nbits]);
 *      bitbuf >>= nbits;
 *      bits_left -= nbits;
 *  }
 *
 */

#define PEEKBIT(nbits) ( nbits > bits_left ? \
(FillBitBuffer(), bitbuf & mask_bits[nbits]) : bitbuf & mask_bits[nbits] )

#endif

/*************/
/*  Globals  */
/*************/

#if 0 /* FIX */
   extern longint   csize;
#endif

   extern ULONG     mask_bits[];

   extern byte      *inbuf;
   extern byte      *inptr;
   extern int       incnt;
   extern ULONG     bitbuf;
   extern int       bits_left;
   extern boolean   zipeof;
   extern int       zipfd;
   extern char      zipfn[];

   extern byte      *outbuf;
   extern byte      *outptr;
   extern byte      *outout;
   extern longint   outpos;
   extern int       outcnt;
   extern int       outfd;
   extern int       disk_full;
