#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef AMIGA
int AmigaRequestString(char *, int, int);
extern char *requesterdesc;
#ifdef getenv
#undef getenv
#endif
#define getenv(name) amiga_getenv(name)
#endif /* AMIGA */

#ifdef UNIX
#if !defined(HAVE_UNISTD_H) && !defined(MACH) && !defined(_BSD)
#define HAVE_UNISTD_H
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#include <stdio.h>
#include <sys/types.h>
#endif

int getch();
int kbhit();

/* replacement function for obsolete clock(), just provides random data */
long Clock();

#endif /* UNIX */

#ifdef MACTC5
int 	getch(void);
int 	kbhit(void);
#endif

#ifdef WIN32
#include <stdio.h>
#include <sys/types.h>
#include <conio.h>
#ifdef min /* These are re-defined in stdlib.h */
#undef min
#endif
#ifdef max
#undef max
#endif
#include <stdlib.h>
#define sleep _sleep
#include <memory.h>
#include <io.h>
#endif

#if defined(UNIX) || defined(AMIGA) || defined(VMS)
void ttycbreak();
void ttynorm();
#else
#define ttycbreak()	/* nothing */
#define ttynorm()	/* nothing */
#endif

#if !defined(MSDOS) && !defined(ATARI)
char *strlwr(char *);
#endif

void breakHandler(int);

#endif /* SYSTEM_H */
