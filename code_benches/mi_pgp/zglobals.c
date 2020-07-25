/*

 Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup Gailly.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included
 unmodified, that it is not sold for profit, and that this copyright notice
 is retained.

*/

/*
 *  globals.c by Mark Adler.
 */

#define GLOBALS         /* include definition of errors[] in zip.h */
#include "zip.h"

/* Argument processing globals */
int method = BEST;      /* one of BEST, DEFLATE (only), or STORE (only) */
#ifdef MACTC5
int level = 9;          /* 0=fastest compression, 9=best compression */
#else
int level = 5;          /* 0=fastest compression, 9=best compression */
#endif
char *special = (char *)NULL;   /* List of special suffixes */
