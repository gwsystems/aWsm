/*      more.c  - Unix-style "more" paging output for PGP.
   PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

   (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
   The author assumes no liability for damages resulting from the use
   of this software, even if the damage results from defects in this
   software.  No warranty is expressed or implied.

   Note that while most PGP source modules bear Philip Zimmermann's
   copyright notice, many of them have been revised or entirely written
   by contributors who frequently failed to put their names in their
   code.  Code that has been incorporated into PGP from other authors
   was either originally published in the public domain or is used with
   permission from the various authors.

   PGP is available for free to the public under certain restrictions.
   See the PGP User's Guide (included in the release package) for
   important information about licensing, patent restrictions on
   certain algorithms, trademarks, copyrights, and export controls.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#ifdef UNIX
#include <sys/types.h>
#endif
#ifdef sco
#include <sys/stream.h>
#include <sys/ptem.h>
FILE *popen();
#endif
#include "system.h"
#include "mpilib.h"
#include "language.h"
#include "fileio.h"
#include "pgp.h"
#include "more.h"
#include "charset.h"

#ifdef MACTC5
#include "Macutil3.h"
#include "MacPGP.h"
#define DEFAULT_LINES	23
#define DEFAULT_COLUMNS	77
#else
#if defined(MSDOS) || defined(WIN32)
#ifndef __GO32__
#include <conio.h>
#endif
#define DEFAULT_LINES	25	/* MSDOS actually has a 25-line screen */
#else
#define DEFAULT_LINES	24
#endif				/* MSDOS */
#define DEFAULT_COLUMNS	80
#endif /* MACTC5 */

static int screen_lines = DEFAULT_LINES, screen_columns = DEFAULT_COLUMNS;

#ifndef EBCDIC                  /* already defined in usuals.h */
#define TAB		0x09	/* ASCII tab char */
#define CR		'\r'	/* Carriage return char */
#define LF		'\n'	/* Linefeed */
#endif

/* Get the screen size for 'more'.  The environment variables $LINES and
   $COLUMNS will be used if they exist.  If not, then the TIOCGWINSZ call to
   ioctl() is used (if it is defined).  If not, then the TIOCGSIZE call to
   ioctl() is used (if it is defined).  If not, then the WIOCGETD call to
   ioctl() is used (if it is defined).  If not, then get the info from
   terminfo/termcap (if it is there).  Otherwise, assume we have a 24x80
   model 33.

   That was for Unix.

   For DOS, just assume 24x80. */

#ifdef UNIX
/* Try to access terminfo through the termcap-interface in the curses library
   (which requires linking with -lcurses) or use termcap directly (which
   requires linking with -ltermcap) */

#ifndef USE_TERMCAP
#ifdef USE_TERMINFO
#define USE_TERMCAP
#endif
#ifdef USE_CURSES
#define USE_TERMCAP
#endif
#endif

#ifdef USE_TERMCAP
#define TERMBUFSIZ    1024
#define UNKNOWN_TERM  "unknown"
#define DUMB_TERMBUF  "dumb:co#80:hc:"

extern int tgetent(), tgetnum();
#endif

/* Try to get TIOCGWINSZ from termios.h, then from sys/ioctl.h */
#ifndef NOTERMIO
#ifdef SVR2
#include <termio.h>
#else
#include <termios.h>
#endif				/* SVR2 */
#endif

#ifdef __FreeBSD__
#include <sys/ioctl.h>          /* for ioctl() prototype too */
#else
#ifndef SVR2
#ifndef TIOCGWINSZ
#ifndef TIOCGSIZE
#ifndef WIOCGETD
#include <sys/ioctl.h>
#endif				/* not WIOCGETD */
#endif				/* not TIOCGSIZE */
#endif				/* not TIOCGWINSZ */
#endif

/* If we still dont have TIOCGWINSZ (or TIOCGSIZE) try for WIOCGETD */
#ifndef TIOCGWINSZ
#ifndef TIOCGSIZE
#ifndef WIOCGETD
#include <sgtty.h>
#endif				/* not WIOCGETD */
#endif				/* not TIOCGSIZE */
#endif				/* not TIOCGWINSZ */
#endif				/* not SVR2 */
#endif				/* UNIX */
#ifdef __PUREC__
#include <ext.h>
#endif

static void getScreenSize(void)
{				/* Rot bilong kargo */
    /* Return the screen size */
    char *envLines, *envColumns;
    long rowTemp = 0, colTemp = 0;
#ifdef UNIX
#ifdef USE_TERMCAP
    char termBuffer[TERMBUFSIZ], *termInfo;
#endif
#ifdef TIOCGWINSZ
    struct winsize windowInfo;
#else
#ifdef TIOCGSIZE
    struct ttysize windowInfo;
#else
#ifdef WIOCGETD
    struct uwdata windowInfo;
#endif				/* WIOCGETD */
#endif				/* TIOCGSIZE */
#endif				/* TIOCGWINSZ */

    /* Make sure that we're outputting to a terminal */
    if (!isatty(fileno(stderr))) {
	screen_lines = DEFAULT_LINES;
	screen_columns = DEFAULT_COLUMNS;
	return;
    }
    screen_lines = screen_columns = 0;
#endif				/* UNIX */

    /* LINES & COLUMNS environment variables override everything else */
    envLines = getenv("LINES");
    if (envLines != NULL && (rowTemp = atol(envLines)) > 0)
	screen_lines = (int) rowTemp;

    envColumns = getenv("COLUMNS");
    if (envColumns != NULL && (colTemp = atol(envColumns)) > 0)
	screen_columns = (int) colTemp;

#ifdef UNIX
#ifdef TIOCGWINSZ
    /* See what ioctl() has to say (overrides terminfo & termcap) */
    if ((!screen_lines || !screen_columns) &&
	ioctl(fileno(stderr), TIOCGWINSZ, &windowInfo)
	!= -1) {
	if (!screen_lines && windowInfo.ws_row > 0)
	    screen_lines = (int) windowInfo.ws_row;

	if (!screen_columns && windowInfo.ws_col > 0)
	    screen_columns = (int) windowInfo.ws_col;
    }
#else
#ifdef TIOCGSIZE
    /* See what ioctl() has to say (overrides terminfo & termcap) */
    if ((!screen_lines || !screen_columns) &&
	ioctl(fileno(stderr), TIOCGSIZE, &windowInfo) != -1) {
	if (!screen_lines && windowInfo.ts_lines > 0)
	    screen_lines = (int) windowInfo.ts_lines;

	if (!screen_columns && windowInfo.ts_cols > 0)
	    screen_columns = (int) windowInfo.ts_cols;
    }
#else
#ifdef WIOCGETD
    /* See what ioctl() has to say (overrides terminfo & termcap) */
    if ((!screen_lines || !screen_columns) &&
	ioctl(fileno(stderr), WIOCGETD, &windowInfo) != -1) {
	if (!screen_lines && windowInfo.uw_height > 0)
	    screen_lines = (int) (windowInfo.uw_height / windowInfo.uw_vs);

	if (!screen_columns && windowInfo.uw_width > 0)
	    screen_columns = (int) (windowInfo.uw_width / windowInfo.uw_hs);
    }				/* You are in a twisty maze of standards,
				   all different */
#endif
#endif
#endif

#ifdef USE_TERMCAP
    /* See what terminfo/termcap has to say */
    if (!screen_lines || !screen_columns) {
	if ((termInfo = getenv("TERM")) == (char *) NULL)
	    termInfo = UNKNOWN_TERM;

	if ((tgetent(termBuffer, termInfo) <= 0))
	    strcpy(termBuffer, DUMB_TERMBUF);

	if (!screen_lines && (rowTemp = tgetnum("li")) > 0)
	    screen_lines = (int) rowTemp;

	if (!screen_columns && (colTemp = tgetnum("co")) > 0)
	    screen_columns = (int) colTemp;
    }
#endif
    if (screen_lines == 0)	/* nothing worked, use defaults */
	screen_lines = DEFAULT_LINES;
    if (screen_columns == 0)
	screen_columns = DEFAULT_COLUMNS;
#endif				/* UNIX */
}

#ifdef ATARI
#define reverse_attr()	printf("\033p")
#define norm_attr()	printf("\033q")
#else
#define reverse_attr()
#define norm_attr()
#endif


#ifdef VMS
char pager[80] = "Type/Page";	/* default pager for VMS */
#else				/* not VMS */
char pager[80] = "";
#endif				/* not VMS */

/* Blort a file to the screen with page breaks, intelligent handling of line
   terminators, truncation of overly long lines, and zapping of illegal
   chars */
int more_file(char *fileName, boolean eyes_only)
{
    FILE *inFile;
    int lines = 0, ch, i, chars = 0, c;
    long fileLen;
    char cmd[MAX_PATH];
    char buf[16];
    int lineno;
    char *p;

    if ((inFile = fopen(fileName, FOPRBIN)) == NULL)
	/* Can't see how this could fail since we just created the file */
	return -1;

    fread(buf, 1, 16, inFile);
    if (compressSignature((byte *) buf) >= 0) {
	fprintf(pgpout,
		LANG("\n\007File '%s' is not a text file; cannot display.\n"),
		fileName);
	return -1;
    }
    /* PAGER set in config.txt overrides environment variable, 
       set PAGER in config.txt to 'pgp' to use builtin pager */
    if (pager[0] == '\0') {
	if ((p = getenv("PAGER")) != NULL)
	    strncpy(pager, p, sizeof(pager) - 1);
    }
    if (strcmp(pager, "cat") == 0) {
	fclose(inFile);
	writePhantomOutput(fileName);
	return 0;
    }
    /* Use built-in pager if PAGER is not set or if the message
       is for your eyes only */
    if (!eyes_only && (strlen(pager) != 0) && strcmp("pgp", pager)) {
	fclose(inFile);
#ifdef UNIX
	if (strchr(fileName, '\'') != NULL)
	    return -1;
	sprintf(cmd, "%s '%s'", pager, fileName);
#else
	sprintf(cmd, "%s %s", pager, fileName);
#if defined(MSDOS) || defined(WIN32)
	for (p = cmd; *p; ++p)
	    if (*p == '/')
		*p = '\\';
#endif
#endif
	fflush(pgpout);
#ifdef MACTC5
	return 0;
#else
	return system(cmd);
#endif
    }
#ifdef UNIX
    if (!isatty(fileno(stdout))) {
	fclose(inFile);
	writePhantomOutput(fileName);
	return 0;
    }
#endif				/* UNIX */

    getScreenSize();

    /* Get file length */
    fseek(inFile, 0L, SEEK_END);
    fileLen = ftell(inFile);
    rewind(inFile);
    lineno = 1;

    ttycbreak();

    putchar('\n');
    for (;;) {
	ch = getc(inFile);
	if (ch == LF) {
	    lines++;
	    putchar('\n');
	    chars = 0;
	    ++lineno;
	} else if (ch == CR) {
	    lines++;
	    putchar('\n');
	    chars = 0;
	    ++lineno;

	    /* Skip following LF if there is one */
	    if ((ch = getc(inFile)) != LF && ch != EOF)
		ungetc(ch, inFile);
	} else if (((unsigned char) ch >= ' ' && ch != EOF) || ch == TAB) {
	    /* Legal char or tab, print it */
	    putchar(ch);
	    chars += (ch == TAB) ? 8 : 1;
	}
	/* If we've reach the max.no of columns we can handle, skip the
	   rest of the line */
	if (chars == screen_columns - 1) {
	    chars = 0;
	    while ((ch = getc(inFile)) != CR && ch != LF && ch != EOF);
	    if (ch != EOF)
		ungetc(ch, inFile);
	}
	/* If we've reached the max.no of rows we can handle, wait for the
	   user to hit a key */
	while (ch == EOF || lines == screen_lines - 1) {
	    /* Print prompt at end of screen */
	    reverse_attr();
	    if (ch == EOF)
		printf(LANG("\nDone...hit any key\r"));
	    else
#ifdef MACTC5	/* 203a : ftell() tells us nothing useful */
		printf(LANG("-- More -- Space: next screen, Enter: next line\
, 'B': back, 'Q': quit --\r"));
#else
		printf(
LANG("More -- %d%% -- Space: next screen, Enter: next line\
, 'B': back, 'Q': quit --\r"),
		       (100 * ftell(inFile)) / fileLen);
#endif /* MACTC5 */
	    norm_attr();
	    fflush(stdout);
	    c = getch();
	    c = toupper(c);

	    /* Blank out prompt */
	    for (i = 0; i < 79; i++)
		putchar(' ');
	    putchar('\r');
	    fflush(stdout);
	    if (c == 'B' && lineno > screen_lines) {
		/* go Back a page */
		int seek_line = lineno - 2 * screen_lines + 3;
		lineno = 1;
		rewind(inFile);
		if (seek_line > 1) {
		    printf("...skipping\n");
		    while ((ch = getc(inFile)) != EOF)
			if (ch == '\n')
			    if (++lineno == seek_line)
				break;
		}
		ch = '\0';
		lines = 0;
	    } else {
#if defined(MSDOS) || defined(__MSDOS__)
		if (c == 'Q' || c == 0x1B || ch == EOF)
#else
		if (c == 'Q' || ch == EOF)
#endif
		    goto done;
		if (c == ' ' || c == '\n' || c == '\r' || c == 'J')
		    lines -= (c == ' ') ? screen_lines - 2 : 1; /* Do n more
								   lines */
	    }
	}
    }
  done:
    ttynorm();

    fclose(inFile);
    return 0;
}				/* more_file */


/*
 * open_more() and close_more() redirect pgpout to the pager.
 *
 */

static char *mfile = NULL;
static boolean piping = FALSE;
static FILE *savepgpout;


int open_more(void)
{
#ifdef UNIX
    char *p;
#endif

    if (mfile || piping)
	close_more();

    savepgpout = pgpout;
#ifdef UNIX
    fflush(pgpout);
    if (pager[0] == '\0') {
	if ((p = getenv("PAGER")) != NULL)
	    strncpy(pager, p, sizeof(pager) - 1);
    }
    /* Use built-in pager if PAGER is not set or set to "pgp" */
    if ((strlen(pager) != 0) && strcmp("pgp", pager)) {
	if ((pgpout = popen(pager, "w")) != NULL) {
	    piping = TRUE;
	    return 0;
	}
	perror("popen");
	pgpout = savepgpout;
    }
#endif
    if ((mfile = tempfile(TMP_TMPDIR | TMP_WIPE)) == NULL)
	return -1;
    if ((pgpout = fopen(mfile, FOPWTXT)) == NULL) {
	pgpout = savepgpout;
	rmtemp(mfile);
	return -1;
    }
    /* user will not see anything until close_more() is called */
    fprintf(savepgpout, LANG("Just a moment..."));
    fflush(savepgpout);
    return 0;
}

int close_more(void)
{
    if (!mfile && !piping)
	return 0;

#ifdef UNIX
    if (piping)
	pclose(pgpout);
    else
#endif
	fclose(pgpout);
    pgpout = savepgpout;
    if (mfile) {
	fprintf(pgpout, "\n");
	more_file(mfile, FALSE);
	rmtemp(mfile);
	mfile = NULL;
    }
    piping = FALSE;
    return 0;
}
