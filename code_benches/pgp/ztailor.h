/* tailor.h -- Not copyrighted 1991 Mark Adler */

/* Use prototypes and ANSI libraries if __STDC__ */
#ifdef __STDC__
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* __STDC__ */


/* Use prototypes and ANSI libraries */
#if defined(sgi) || defined(mips)
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* sgi */


/* Define MSDOS for Turbo C as well as Microsoft C */
#ifdef __POWERC                 /* For Power C too */
#  define __TURBOC__
#endif /* __POWERC */


/* Use prototypes and ANSI libraries if Microsoft or Borland C */
#ifdef MSDOS
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* MSDOS */


/* Turn off prototypes if requested */
#ifdef NOPROTO
#  ifdef PROTO
#    undef PROTO
#  endif /* PROTO */
#endif /* NOPROT */


/* Used to remove arguments in function prototypes for non-ANSI C */
#ifdef PROTO
#  define OF(a) a
#else /* !PROTO */
#  define OF(a) ()
#endif /* ?PROTO */


/* Allow far and huge allocation for small model (Microsoft C or Turbo C) */
/* unless NOFAR defined (needed for ANSI mode compilations) */
/* But if we're using DJGPP, and want to use the optimized 386 assembler  */
/* zmatch routines, we can't use DYN_ALLOC. But gcc has virtual memory... */
#ifdef MSDOS
#  ifndef __GNUC__
#    ifndef DYN_ALLOC
#      define DYN_ALLOC
#    endif
#  endif
#  ifdef __TURBOC__
#    include <alloc.h>
#    define fcalloc   calloc  /* Assumes that all arrays are < 64K for MSDOS */
#    define fcfree    free
#  else /* !__TURBOC__ */
#    ifndef __GNUC__
#     include <malloc.h>
#     define farmalloc _fmalloc
#     define farfree   _ffree
#     define fcalloc(nitems,itemsize) halloc((long)(nitems),(itemsize))
#     define fcfree hfree
#    endif /* __GNUC__ */
#  endif /* ?__TURBOC__ */
#  ifdef NOFAR
#  define huge
#  define far
#  define near
#  endif
#else /* !MSDOS */
#  ifdef __QNX__
#  undef huge
#  undef far
#  undef near
#  undef farmalloc
#  undef farfree
#  undef fcalloc
#  endif /* __QNX__ */
#  define huge
#  define far
#  define near
#  define farmalloc malloc
#  define farfree   free
#  define fcalloc   calloc
#  define fcfree    free
#endif /* ?MSDOS */
#ifdef __GNUC__
#  ifdef MSDOS /* MSDOS GNU C == DJGPP */
#    define huge
#    define far
#    define near
#    define farmalloc malloc
#    define farfree   free
#    define fcalloc   calloc
#    define fcfree    free
#  endif /* MSDOS */
#endif /* __GNUC__ */

/* Define MSVMS if either MSDOS or VMS defined */
#ifdef MSDOS
#  define MSVMS
#else /* !MSDOS */
#  ifdef VMS
#    define MSVMS
#  endif /* VMS */
#endif /* ?MSDOS */


#include <stdio.h>
#include <stdlib.h>
typedef unsigned int extent;

/* Get types and stat */
#ifdef VMS
#  include <types.h>
#  include <stat.h>
#else /* !VMS */
#ifndef __PUREC__
# ifndef C370
#  ifndef MACTC5
#   include <sys/types.h>
#   include <sys/stat.h>
#  endif /* MACTC5 */
# endif /* C370 */
#endif
#endif /* ?VMS */


/* Cheap fix for unlink on VMS */
#ifdef VMS
#  define unlink delete
#endif /* VMS */


/* For Pyramid */
#ifdef pyr
#  define strrchr rindex
#endif /* pyr */


/* File operations--use "b" for binary if allowed */
#ifdef MODERN
#  define FOPR "rb"
#  define FOPM "r+b"
#  define FOPW "w+b"
#else /* !MODERN */
#  define FOPR "r"
#  define FOPM "r+"
#  define FOPW "w+"
#endif /* ?MODERN */


/* Define this symbol if your target allows access to unaligned data.
 * This is not mandatory, just a speed optimization. The compressed
 * output is strictly identical.
 */
#if defined(MSDOS) || defined(M_XENIX) || defined(i386) || \
defined(mc68020) || defined(vax)
#   define UNALIGNED_OK
#endif


/* Under MSDOS we may run out of memory when processing a large number
 * of files. Compile with MEDIUM_MEM to reduce the memory requirements or
 * with MIN_MEM to use as little memory as possible.
 */
#ifdef SMALL_MEM
#   define BSZ 2048   /* Buffer size for files */
#else
#ifdef MEDIUM_MEM
#   define BSZ 8192
#else
#   define BSZ 16384
#endif
#endif

/* end of tailor.h */
