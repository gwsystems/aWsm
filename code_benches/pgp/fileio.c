/*      fileio.c  - I/O routines for PGP.
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

   Modified 16 Apr 92 - HAJK
   Mods for support of VAX/VMS file system

   Modified 17 Nov 92 - HAJK
   Change to temp file stuff for VMS.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef _BSD
#include <sys/param.h>
#endif
extern int errno;
#endif				/* UNIX */
#ifdef VMS
#include <file.h>
#include <assert.h>
#endif
#include "random.h"
#include "mpilib.h"
#include "mpiio.h"
#include "fileio.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
#include "charset.h"
#include "system.h"
#if defined(MSDOS) || defined(OS2) || defined (WIN32)
#include <io.h>
#include <fcntl.h>
#endif
#ifdef MACTC5
#include "crypto.h" 	/* for get_header_info_from_file() */
#include "AEStuff.h"
#include "AppGlobals.h"
#include "MacPGP.h"
#include "Macutil2.h"
#include "Macutil3.h"
#define MULTIPLE_DOTS
extern Boolean AEProcessing;
pascal Boolean idleProc(EventRecord * eventIn, long *sleep, RgnHandle * mouseRgn);
#endif

char *ck_dup_output(char *, boolean, boolean);

#ifndef F_OK
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4
#endif				/* !F_OK */

/*
 * DIRSEPS is a string of possible directory-separation characters
 * The first one is the preferred one, which goes in between
 * PGPPATH and the file name if PGPPATH is not terminated with a
 * directory separator.
 */

#if defined(MSDOS) || defined(__MSDOS__) || defined(OS2) || defined (WIN32)
static char const DIRSEPS[] = "\\/:";
#define BSLASH

#elif defined(ATARI)
static char const DIRSEPS[] = "\\/:";
#define BSLASH

#elif defined(UNIX)
static char const DIRSEPS[] = "/";
#define MULTIPLE_DOTS

#elif defined(AMIGA)
static char const DIRSEPS[] = "/:";
#define MULTIPLE_DOTS

#elif defined(VMS)
static char const DIRSEPS[] = "]:";

#elif defined(EBCDIC)
static char const DIRSEPS[] = "("; 	/* Any more? */
#define MULTIPLE_DOTS

#elif defined(MACTC5)
#define MULTIPLE_DOTS
static char const DIRSEPS[] = ":";

#else
/* #error is not portable, this has the same effect */
#include "Unknown OS"
#endif

#ifdef __PUREC__
#include <ext.h>
int access(const char *name,int flag)
{
struct ffblk dummy;
	return findfirst(name,&dummy,-1);
}
#endif

/* 1st character of temporary file extension */
#define	TMP_EXT	'$'		/* extensions are '.$##' */

/* The PGPPATH environment variable */

static char PGPPATH[] = "PGPPATH";

/* Disk buffers, used here and in crypto.c */
byte textbuf[DISKBUFSIZE];
static unsigned textbuf2[2 * DISKBUFSIZE / sizeof(unsigned)];

boolean file_exists(char *filename)
/*      Returns TRUE iff file exists. */
{
#ifdef MACTC5
	FILE *f;
	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(filename,FOPRBIN)) == NULL)
		return(FALSE);
	fclose(f);
	return(TRUE);
#else
    return access(filename, F_OK) == 0;
#endif
}				/* file_exists */

static boolean is_regular_file(char *filename)
{
#ifdef S_ISREG
    struct stat st;
    return stat(filename, &st) != -1 && S_ISREG(st.st_mode);
#else
    return TRUE;
#endif
}


/*
 * This wipes a file with pseudo-random data.  The purpose of this is to
 * make sure no sensitive information is left on the disk.  The use
 * of pseudo-random data is to defeat disk compression drivers (such as
 * Stacker and dblspace) so that we are guaranteed that the entire file
 * has been overwritten.
 *
 * Note that the file MUST be open for read/write.
 *
 * It may not work to eliminate everything from non-volatile storage
 * if the OS you're using does its own paging or swapping.  Then
 * it's an issue of how the OS's paging device is wiped, and you can
 * only hope that the space will be reused within a few seconds.
 *
 * Also, some file systems (in particular, the Logging File System
 * for Sprite) do not write new data in the same place as old data,
 * defeating this wiping entirely.  Fortunately, such systems 
 * usually don't need a swap file, and for small temp files, they
 * do write-behind, so if you create and delete a file fast enough,
 * it never gets written to disk at all.
 */

/*
 * The data is randomly generated with the size of the file as a seed.
 * The data should be random and not leak information.  If someone is
 * examining deleted files, presumably they can reconstruct the file size,
 * so that's not a secret.  H'm... this wiping algorithm makes it easy to,
 * given a block of data, find the size of the file it came from
 * and the offset of this block within it.  That in turn reveals
 * something about the state of the disk's allocation tables when the
 * file was used, possibly making it easier to find other files created
 * at neaby times - such as plaintext files.  Is this acceptable?
 */

/*
 * Theory of operation: We use the additive congruential RNG
 * r[i] = r[i-24] + r[i-55], from Knuth, Vol. 2.  This is fast
 * and has a long enough period that there should be no repetitions
 * in even a huge file.  It is seeded with r[-55] through r[-1]
 * using another polynomial-based RNG.  We seed a linear feedback
 * shift register (CRC generator) with the size of the swap file,
 * and clock in 0 bits.  Each 32 bits, the value of the generator is
 * taken as the next integer.  This is just to ensure a reasonably
 * even mix of 1's and 0's in the initialization vector.
 */

/*
 * This is the CRC-32 polynomial, which should be okay for random
 * number generation.
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
 * = 1 0000 0100 1100 0001 0001 1101 1011 0111
 * = 0x04c11db7
 */
#define POLY 0x04c11db7

static void wipeout(FILE * f)
{
    unsigned *p1, *p2, *p3;
    unsigned long len;
    unsigned long t;
    int i;

    /* Get the file size */
    fseek(f, 0L, SEEK_END);
    len = ftell(f);
#ifdef MACTC5 
	len = len + 4096 - (len % 4096);
#endif 
    rewind(f);

    /* Seed of first RNG.  Inverted to get more 1 bits */
    t = ~len;

    /* Initialize first 55 words of buf with pseudo-random stuff */
    p1 = (unsigned *) textbuf2 + 55;
    do {
	for (i = 0; i < 32; i++)
	    t = (t & 0x80000000) ? t << 1 ^ POLY : t << 1;
	*--p1 = (unsigned) t;
    } while (p1 > (unsigned *) textbuf2);

    while (len) {
	/* Fill buffer with pseudo-random integers */

	p3 = (unsigned *) textbuf2 + 55;
	p2 = (unsigned *) textbuf2 + 24;
	p1 = (unsigned *) textbuf2 + sizeof(textbuf2) / sizeof(*p1);
	do {
	    *--p1 = *--p2 + *--p3;
	} while (p2 > (unsigned *) textbuf2);

	p2 = (unsigned *) textbuf2 + sizeof(textbuf2) / sizeof(*p1);
	do {
	    *--p1 = *--p2 + *--p3;
	} while (p3 > (unsigned *) textbuf2);

	p3 = (unsigned *) textbuf2 + sizeof(textbuf2) / sizeof(*p3);
	do {
	    *--p1 = *--p2 + *--p3;
	} while (p1 > (unsigned *) textbuf2);

	/* Write it out - yes, we're ignoring errors */
	if (len > sizeof(textbuf2)) {
	    fwrite((char const *) textbuf2, sizeof(textbuf2), 1, f);
	    len -= sizeof(textbuf2);
#ifdef MACTC5
		mac_poll_for_break();
#endif
	} else {
	    fwrite((char const *) textbuf2, len, 1, f);
	    len = 0;
	}
    }
}


/*
 * Completely overwrite and erase file, so that no sensitive
 * information is left on the disk.
 */
int wipefile(char *filename)
{
    FILE *f;
    /* open file f for read/write, in binary (not text) mode... */
    if ((f = fopen(filename, FOPRWBIN)) == NULL)
	return -1;		/* error - file can't be opened */
    wipeout(f);
    fclose(f);
    return 0;			/* normal return */
}				/* wipefile */

/*
 * Returns the part of a filename after all directory specifiers.
 */
char *file_tail(char *filename)
{
    char *p;
    char const *s = DIRSEPS;

    while (*s) {
	p = strrchr(filename, *s);
	if (p)
	    filename = p + 1;
	s++;
    }

    return filename;
}


/* return TRUE if extension matches the end of filename */
boolean has_extension(char *filename, char *extension)
{
    int lf = strlen(filename);
    int lx = strlen(extension);

    if (lf <= lx)
	return FALSE;
    return !strcmp(filename + lf - lx, extension);
}

/* return TRUE if path is a filename created by tempfile() */
/* Filename matches "*.$[0-9][0-9]" */
boolean is_tempfile(char *path)
{
    char *p = strrchr(path, '.');

    return p != NULL && p[1] == TMP_EXT &&
	isdigit(p[2]) && isdigit(p[3]) && p[4] == '\0';
}

/*
 * Returns TRUE if user left off file extension, allowing default.
 * Note that the name is misleading if multiple dots are allowed.
 * not_pgp_extension or something would be better.
 */
boolean no_extension(char *filename)
{
#ifdef MULTIPLE_DOTS		/* filename can have more than one dot */
    if (has_extension(filename, ASC_EXTENSION) ||
	has_extension(filename, PGP_EXTENSION) ||
	has_extension(filename, SIG_EXTENSION) ||
#ifdef MACTC5
		has_extension(filename,".tmp") ||
#endif
	is_tempfile(filename))
	return FALSE;
    else
	return TRUE;
#else
    filename = file_tail(filename);

    return strrchr(filename, '.') == NULL;
#endif
}				/* no_extension */


/* deletes trailing ".xxx" file extension after the period. */
void drop_extension(char *filename)
{
    if (!no_extension(filename))
	*strrchr(filename, '.') = '\0';
}				/* drop_extension */


/* append filename extension if there isn't one already. */
void default_extension(char *filename, char *extension)
{
    if (no_extension(filename))
	strcat(filename, extension);
}				/* default_extension */

#ifndef MAX_NAMELEN
#if defined(AMIGA) || defined(NeXT) || (defined(BSD) && BSD > 41) || (defined(sun) && defined(i386))
#define	MAX_NAMELEN	255
#else
#ifdef MACTC5
#define MAX_NAMELEN 31
#else
#include <limits.h>
#endif
#endif
#endif

/* truncate the filename so that an extension can be tacked on. */
static void truncate_name(char *path, int ext_len)
{
#ifdef UNIX			/* for other systems this is a no-op */
    char *p;
#ifdef MAX_NAMELEN		/* overrides the use of pathconf() */
    int namemax = MAX_NAMELEN;
#else
    int namemax;
#ifdef _PC_NAME_MAX
    char dir[MAX_PATH];

    strcpy(dir, path);
    if ((p = strrchr(dir, '/')) == NULL) {
	strcpy(dir, ".");
    } else {
	if (p == dir)
	    ++p;
	*p = '\0';
    }
    if ((namemax = pathconf(dir, _PC_NAME_MAX)) <= ext_len)
	return;
#else
#ifdef NAME_MAX
    namemax = NAME_MAX;
#else
    namemax = 14;
#endif				/* NAME_MAX */
#endif				/* _PC_NAME_MAX */
#endif				/* MAX_NAMELEN */

    if ((p = strrchr(path, '/')) == NULL)
	p = path;
    else
	++p;
    if (strlen(p) > namemax - ext_len) {
	if (verbose)
	    fprintf(pgpout, "Truncating filename '%s' ", path);
	p[namemax - ext_len] = '\0';
	if (verbose)
	    fprintf(pgpout, "to '%s'\n", path);
    }
#else
#ifdef MACTC5
	char *p;
	p = file_tail(path);
	if (verbose)
		fprintf(pgpout, LANG("Truncating filename '%s' "), path);
	if (strlen(p) + ext_len > MAX_NAMELEN) p[MAX_NAMELEN - ext_len] = '\0';
#endif  /* MACTC5 */ 
#endif				/* UNIX */
}

/* change the filename extension. */
void force_extension(char *filename, char *extension)
{
    drop_extension(filename);	/* out with the old */
    truncate_name(filename, strlen(extension));
    strcat(filename, extension);	/* in with the new */
}				/* force_extension */


/*
 * Get yes/no answer from user, returns TRUE for yes, FALSE for no. 
 * First the translations are checked, if they don't match 'y' and 'n'
 * are tried.
 */
#ifdef MACTC5

boolean getyesno(char default_answer)
{ 
  extern FILE *logfile;
  short  alertid,i,large,err;
  char dfault[8], ndfault[8];
  ProcessSerialNumber psn;
  if (batchmode)
  	return(default_answer == 'y' ? TRUE : FALSE);
  if (strlen(Yes_No_Message)<72) large=0;
  else large=100;
  strcpy(dfault,default_answer == 'y' ? LANG("y") : LANG("n"));
  strcpy(ndfault,default_answer == 'n' ? LANG("y") : LANG("n"));
  for(i=0;i<strlen(Yes_No_Message);i++)
  if (Yes_No_Message[i]<' ' && Yes_No_Message[i]>=0) Yes_No_Message[i]=' ';	/* It's a signed char! */
  InitCursor();
  alertid=(default_answer == 'n' ? 211+large : 212+large);
  c2pstr(Yes_No_Message);
  ParamText((uchar *)Yes_No_Message,(uchar *)"", \
  		(uchar *)"",(uchar *)"");
  if (AEProcessing) {
  	if (gHasProcessManager)
  		GetFrontProcess(&psn);
  	if(MyInteractWithUser())
  		return default_answer;
  	if (gHasProcessManager)
    	SetFrontProcess(&psn);
  }
  if (CautionAlert(alertid,nil)==1){
   p2cstr((uchar *)Yes_No_Message);
   fputs(strcat(Yes_No_Message,dfault),stderr);
   fputc('\n',stderr);
   fflush(stderr);
   return(default_answer == 'y' ? TRUE : FALSE);
   }
  p2cstr((uchar *)Yes_No_Message);
  fputs(strcat(Yes_No_Message,ndfault),stderr);
  fputc('\n',stderr);
  fflush(stderr);
  return(default_answer == 'n' ? TRUE : FALSE);
  } 
#else

boolean getyesno(char default_answer)
{
    char buf[8];
    static char yes[8], no[8];

    if (yes[0] == '\0') {
	strncpy(yes, LANG("y"), 7);
	strncpy(no, LANG("n"), 7);
    }
    if (!batchmode) {		/* return default answer in batchmode */
	getstring(buf, 6, TRUE);	/* echo keyboard input */
	strlwr(buf);
	if (!strncmp(buf, no, strlen(no)))
	    return FALSE;
	if (!strncmp(buf, yes, strlen(yes)))
	    return TRUE;
	if (buf[0] == 'n')
	    return FALSE;
	if (buf[0] == 'y')
	    return TRUE;
    }
    return default_answer == 'y' ? TRUE : FALSE;
}				/* getyesno */
#endif

/* if user consents to it, change the filename extension. */
char *maybe_force_extension(char *filename, char *extension)
{
    static char newname[MAX_PATH];
    if (!has_extension(filename, extension)) {
	strcpy(newname, filename);
	force_extension(newname, extension);
	if (!file_exists(newname)) {
	    fprintf(pgpout, LANG("\nShould '%s' be renamed to '%s' (Y/n)? "),
		    filename, newname);
	    if (getyesno('y'))
		return newname;
	}
    }
    return NULL;
}				/* maybe_force_extension */

/*
 * Add a trailing directory separator to a name, if absent.
 */
static void addslash(char *name)
{
    int i = strlen(name);

    if (i != 0 && !strchr(DIRSEPS, name[i - 1])) {
	name[i] = DIRSEPS[0];
	name[i + 1] = '\0';
    }
}

/*
 * Builds a filename with a complete path specifier from the environmental
 * variable PGPPATH.
 */
char *buildfilename(char *result, char *fname)
{
#ifdef MACTC5
	char const *s;
#else
    char const *s = "."; /* getenv(PGPPATH); */
#endif
    result[0] = '\0';
#ifdef MACTC5
    return(strcpy(result,fname));
#endif

    if (s && strlen(s) <= 50) {
	strcpy(result, s);
    }
#ifdef UNIX
    /* On Unix, default to $HOME/.pgp, otherwise, current directory. */
    else {
	s = getenv("HOME");
	if (s && strlen(s) <= 50) {
	    strcpy(result, s);
	    addslash(result);
	    strcat(result, ".pgp");
	}
    }
#endif				/* UNIX */

    addslash(result);
    strcat(result, fname);
    return result;
}				/* buildfilename */

char *buildsysfilename(char *result, char *fname)
{
#ifdef PGP_SYSTEM_DIR
    strcpy(result, PGP_SYSTEM_DIR);
    strcat(result, fname);
    if (file_exists(result))
	return result;
#endif
    buildfilename(result, fname);	/* Put name back for error */
    return result;
}


/* Convert filename to canonical form, with slashes as separators */
void file_to_canon(char *filename)
{
#ifdef EBCDIC
    CONVERT_TO_CANONICAL_CHARSET(filename);
#endif
#ifdef BSLASH
    while (*filename) {
	if (*filename == '\\')
	    *filename = '/';
	++filename;
    }
#else	/* 203a */
#ifdef MACTC5
	while (*filename) {
		if (*filename == ':')
			*filename = '/';
		++filename;
		}
#endif
#endif
}

#ifdef EBCDIC
/* Convert filename from canonical form */
void file_from_canon(char *filename)
{
   strcpy( filename, LOCAL_CHARSET(filename) );
}
#endif /* EBCDIC */


int write_error(FILE * f)
{
    fflush(f);
    if (ferror(f)) {
#ifdef ENOSPC
	if (errno == ENOSPC)
	    fprintf(pgpout, LANG("\nDisk full.\n"));
	else
#endif
	    fprintf(pgpout, LANG("\nFile write error.\n"));
	return -1;
    }
    return 0;
}

/* copy file f to file g, for longcount bytes */
int copyfile(FILE * f, FILE * g, word32 longcount)
{
    int count, status = 0;
    do {			/* read and write the whole file... */
	if (longcount < (word32) DISKBUFSIZE)
	    count = (int) longcount;
	else
	    count = DISKBUFSIZE;
	count = fread(textbuf, 1, count, f);
	if (count > 0) {
	    if (CONVERSION != NO_CONV) {
		int i;
		for (i = 0; i < count; i++)
		    textbuf[i] = (CONVERSION == EXT_CONV) ?
			EXT_C(textbuf[i]) :
			INT_C(textbuf[i]);
	    }
	    if (fwrite(textbuf, 1, count, g) != count) {
		/* Problem: return error value */
		status = -1;
		break;
	    }
	    longcount -= count;
#ifdef MACTC5
			mac_poll_for_break();
#endif
	}
	/* if text block was short, exit loop */
    } while (count == DISKBUFSIZE);
    burn(textbuf);		/* burn sensitive data on stack */
    return status;
}				/* copyfile */

/*
 * Like copyfile, but takes a position for file f.  Returns with
 * f and g pointing just past the copied data.
 */
int copyfilepos(FILE * f, FILE * g, word32 longcount, word32 fpos)
{
    fseek(f, fpos, SEEK_SET);
    return copyfile(f, g, longcount);
}


/* copy file f to file g, for longcount bytes.  Convert to
 * canonical form as we go.  f is open in text mode.  Canonical
 * form uses crlf's as line separators.
 */
int copyfile_to_canon(FILE * f, FILE * g, word32 longcount)
{
    int count, status = 0;
    byte c, *tb1, *tb2;
    int i, nbytes;
    int nspaces = 0;
#ifdef MACTC5
    Boolean warning = true; /* MACTC5 */
#endif
    do {			/* read and write the whole file... */
	if (longcount < (word32) DISKBUFSIZE)
	    count = (int) longcount;
	else
	    count = DISKBUFSIZE;
	count = fread(textbuf, 1, count, f);
	if (count > 0) {
	    /* Convert by adding CR before LF */
	    tb1 = textbuf;
	    tb2 = (byte *) textbuf2;
	    for (i = 0; i < count; ++i) {
		switch (CONVERSION) {
		case EXT_CONV:
		    c = EXT_C(*tb1++);
		    break;
		case INT_CONV:
		    c = INT_C(*tb1++);
		    break;
		default:
		    c = *tb1++;
		}
#ifdef MACTC5
		if ( (((uchar) c) < ' ') && (c != '\n') && (c != '\t') && warning) {
			warning = false;
			fprintf(stdout, "\aWarning text file contains control characters!\n");
		}
#endif
		if (strip_spaces) {
		    if (c == ' ') {
			/* Don't output spaces yet */
			nspaces += 1;
		    } else {
			if (c == '\n') {
			    *tb2++ = '\r';
			    nspaces = 0;	/* Delete trailing spaces */
			}
			if (nspaces) {
			    /* Put out spaces now */
			    do
				*tb2++ = ' ';
			    while (--nspaces);
			}
			*tb2++ = c;
		    }
		} else {
		    if (c == '\n')
			*tb2++ = '\r';
		    *tb2++ = c;
		}
	    }
	    nbytes = tb2 - (byte *) textbuf2;
	    if (fwrite(textbuf2, 1, nbytes, g) != nbytes) {
		/* Problem: return error value */
		status = -1;
		break;
	    }
	    longcount -= count;
	}
	/* if text block was short, exit loop */
    } while (count == DISKBUFSIZE);
    burn(textbuf);		/* burn sensitive data on stack */
    burn(textbuf2);
    return status;
}				/* copyfile_to_canon */


/* copy file f to file g, for longcount bytes.  Convert from
 * canonical to local form as we go.  g is open in text mode.  Canonical
 * form uses crlf's as line separators.
 */
int copyfile_from_canon(FILE * f, FILE * g, word32 longcount)
{
    int count, status = 0;
    byte c, *tb1, *tb2;
    int i, nbytes;
    do {			/* read and write the whole file... */
	if (longcount < (word32) DISKBUFSIZE)
	    count = (int) longcount;
	else
	    count = DISKBUFSIZE;
	count = fread(textbuf, 1, count, f);
	if (count > 0) {
	    /* Convert by removing CR's */
	    tb1 = textbuf;
	    tb2 = (byte *) textbuf2;
	    for (i = 0; i < count; ++i) {
		switch (CONVERSION) {
		case EXT_CONV:
		    c = EXT_C(*tb1++);
		    break;
		case INT_CONV:
		    c = INT_C(*tb1++);
		    break;
		default:
		    c = *tb1++;
		}
		if (c != '\r')
		    *tb2++ = c;
	    }
	    nbytes = tb2 - (byte *) textbuf2;
	    if (fwrite(textbuf2, 1, nbytes, g) != nbytes) {
		/* Problem: return error value */
		status = -1;
		break;
	    }
	    longcount -= count;
	}
	/* if text block was short, exit loop */
    } while (count == DISKBUFSIZE);
    burn(textbuf);		/* burn sensitive data on stack */
    burn(textbuf2);
    return status;
}				/* copyfile_from_canon */

/*      Copy srcFile to destFile  */
int copyfiles_by_name(char *srcFile, char *destFile)
{
    FILE *f, *g;
    int status = 0;
    long fileLength;

    f = fopen(srcFile, FOPRBIN);
    if (f == NULL)
	return -1;
    g = fopen(destFile, FOPWBIN);
    if (g == NULL) {
	fclose(f);
	return -1;
    }
    /* Get file length and copy it */
    fseek(f, 0L, SEEK_END);
    fileLength = ftell(f);
    rewind(f);
    status = copyfile(f, g, fileLength);
    fclose(f);
    if (write_error(g))
	status = -1;
    fclose(g);
    return status;
}				/* copyfiles_by_name */

/* Copy srcFile to destFile, converting to canonical text form  */
int make_canonical(char *srcFile, char *destFile)
{
    FILE *f, *g;
    int status = 0;
    long fileLength;

    if (((f = fopen(srcFile, FOPRTXT)) == NULL) ||
	((g = fopen(destFile, FOPWBIN)) == NULL))
	/* Can't open files */
	return -1;

    /* Get file length and copy it */
    fseek(f, 0L, SEEK_END);
    fileLength = ftell(f);
    rewind(f);
    CONVERSION = INT_CONV;
    status = copyfile_to_canon(f, g, fileLength);
    CONVERSION = NO_CONV;
    fclose(f);
    if (write_error(g))
	status = -1;
    fclose(g);
    return status;
}				/* make_canonical */

/*
 * Like rename() but will try to copy the file if the rename fails.
 * This is because under OS's with multiple physical volumes if the
 * source and destination are on different volumes the rename will fail
 */
int rename2(char *srcFile, char *destFile)
{
    FILE *f, *g;
#ifdef MACTC5
	int copy=-1;
#endif
    int status = 0;
    long fileLength;

#ifdef MACTC5
	copy=MoveRename(srcFile,destFile);
if (copy)
#else
#if defined(VMS) || defined(C370)
    if (rename(srcFile, destFile) != 0)
#else
    if (rename(srcFile, destFile) == -1)
#endif
#endif
    {
	/* Rename failed, try a copy */
	if (((f = fopen(srcFile, FOPRBIN)) == NULL) ||
	    ((g = fopen(destFile, FOPWBIN)) == NULL))
	    /* Can't open files */
	    return -1;

#ifdef MACTC5
		{
		FInfo finderInfo;
		c2pstr(srcFile);
		c2pstr(destFile);
		if(GetFInfo((uchar *)srcFile,0,&finderInfo)==0)
			SetFInfo((uchar *)destFile,0,&finderInfo);
		p2cstr((uchar *)srcFile);
		p2cstr((uchar *)destFile);
		}
#endif

	/* Get file length and copy it */
	fseek(f, 0L, SEEK_END);
	fileLength = ftell(f);
	rewind(f);
	status = copyfile(f, g, fileLength);
	if (write_error(g))
	    status = -1;

	/* Zap source file if the copy went OK, otherwise zap the (possibly
	   incomplete) destination file */
	if (status >= 0) {
	    wipeout(f);		/* Zap source file */
	    fclose(f);
	    remove(srcFile);
	    fclose(g);
	} else {
	    if (is_regular_file(destFile)) {
		wipeout(g);	/* Zap destination file */
		fclose(g);
		remove(destFile);
	    } else {
		fclose(g);
	    }
	    fclose(f);
	}
    }
    return status;
}

/* read the data from stdin to the phantom input file */
int readPhantomInput(char *filename)
{
    FILE *outFilePtr;
    byte buffer[512];
    int bytesRead, status = 0;

    if (verbose)
	fprintf(pgpout, "writing stdin to file %s\n", filename);
    if ((outFilePtr = fopen(filename, FOPWBIN)) == NULL)
	return -1;

#if defined(MSDOS) || defined(OS2) || defined (WIN32)
    /* Under DOS must set input stream to binary mode to avoid data mangling */
    setmode(fileno(stdin), O_BINARY);
#endif				/* MSDOS || OS2 */
    while ((bytesRead = fread(buffer, 1, 512, stdin)) > 0)
	if (fwrite(buffer, 1, bytesRead, outFilePtr) != bytesRead) {
	    status = -1;
	    break;
	}
    if (write_error(outFilePtr))
	status = -1;
    fclose(outFilePtr);
#if defined(MSDOS) || defined(OS2) || defined (WIN32)
    setmode(fileno(stdin), O_TEXT);	/* Reset stream */
#endif				/* MSDOS || OS2 */
    return status;
}

/* write the data from the phantom output file to stdout */
int writePhantomOutput(char *filename)
{
    FILE *outFilePtr;
    byte buffer[512];
    int bytesRead, status = 0;

    if (verbose)
	fprintf(pgpout, "writing file %s to stdout\n", filename);
    /* this can't fail since we just created the file */
    outFilePtr = fopen(filename, FOPRBIN);

#if defined(MSDOS) || defined(OS2) || defined (WIN32)
    setmode(fileno(stdout), O_BINARY);
#endif				/* MSDOS || OS2 */
    while ((bytesRead = fread(buffer, 1, 512, outFilePtr)) > 0)
	if (fwrite(buffer, 1, bytesRead, stdout) != bytesRead) {
	    status = -1;
	    break;
	}
    fclose(outFilePtr);
    fflush(stdout);
    if (ferror(stdout)) {
	status = -1;
	fprintf(pgpout, LANG("\007Write error on stdout.\n"));
    }
#if defined(MSDOS) || defined(OS2) || defined (WIN32)
    setmode(fileno(stdout), O_TEXT);
#endif				/* MSDOS || OS2 */

    return status;
}

/* Return the size from the current position of file f to the end */
word32 fsize(FILE * f)
{
    long fpos = ftell(f);
    long fpos2;

    fseek(f, 0L, SEEK_END);
    fpos2 = ftell(f);
    fseek(f, fpos, SEEK_SET);
    return (word32) (fpos2 - fpos);
}

/* Return TRUE if file filename looks like a pure text file */
int is_text_file (char *filename) /* EWS */
{
    FILE *f = fopen(filename,"r");      /* FOPRBIN gives problem with VMS */
    int i, n, lfctr = 0;
    unsigned char buf[512];
    unsigned char *bufptr = buf;
    unsigned char c;

    if (!f)
        return FALSE;          /* error opening it, so not a text file */
    i = n = fread (buf, 1, sizeof(buf), f);
    fclose(f);
    if (n <= 0)
        return FALSE;          /* empty file or error, not a text file */
    if (compressSignature(buf) >= 0)
        return FALSE;
    while (i--) {
        c = *bufptr++;
        if (c == '\n' || c == '\r')
            lfctr=0;
        else /* allow BEL BS HT LF VT FF CR EOF ESC control characters */
        {
#ifdef EBCDIC
	    if (iscntrl(c) && c!=BEL && c!=BS && c!=HT && c!=LF && c!=VT && c!=FF && c!=CR && c!=EOF && c!=ESC)
#else
            if (c < '\007' || (c > '\r' && c < ' ' && c != '\032' && c != '\033'))
#endif
                return FALSE;  /* not a text file */
            lfctr++;
/*          if (lfctr>132) return FALSE; /* line too long. Not a text file */
        }
    }
    return TRUE;
}                               /* is_text_file */

VOID *xmalloc(unsigned size)
{
    VOID *p;
    if (size == 0)
	++size;
    p = malloc(size);
    if (p == NULL) {
	fprintf(stderr, LANG("\n\007Out of memory.\n"));
	exitPGP(1);
    }
    return p;
}

/*----------------------------------------------------------------------
 *	temporary file routines
 */


#define MAXTMPF 8

#define	TMP_INUSE	2

static struct {
    char path[MAX_PATH];
    int flags;
    int num;
} tmpf[MAXTMPF];

static char tmpdir[256];	/* temporary file directory */
static char outdir[256];	/* output directory */
static char tmpbasename[64] = "pgptemp";	/* basename for
						   temporary files */


/*
 * set directory for temporary files.  path will be stored in
 * tmpdir[] with an appropriate trailing path separator.
 */
void settmpdir(char *path)
{
    char *p;

    if (path == NULL || *path == '\0') {
	tmpdir[0] = '\0';
	return;
    }
    strcpy(tmpdir, path);
    p = tmpdir + strlen(tmpdir) - 1;
#ifdef MACTC5
	if (*p != '/' && *p != '\\' && *p != ']' && *p != ':')
	{	/* append path separator, either / or \ */
		if ((p = strchr(tmpdir, '/')) == NULL &&
			(p = strchr(tmpdir, '\\')) == NULL &&
			(p = strchr(tmpdir, ':')) == NULL)
			p = ":";	/* path did not contain / or \ or :, use : */
		strncat(tmpdir, p, 1);
#else
    if (*p != '/' && *p != '\\' && *p != ']' && *p != ':') {
	/* append path separator, either / or \ */
	if ((p = strchr(tmpdir, '/')) == NULL &&
	    (p = strchr(tmpdir, '\\')) == NULL)
	    p = "/";		/* path did not contain / or \, use / */
	strncat(tmpdir, p, 1);
#endif
    }
}

/*
 * set output directory to avoid a file copy when temp file is renamed to
 * output file.  the argument filename must be a valid path for a file, not
 * a directory.
 */
void setoutdir(char *filename)
{
    char *p;

    if (filename == NULL) {
	strcpy(outdir, tmpdir);
	return;
    }
    strcpy(outdir, filename);
    p = file_tail(outdir);
    strcpy(tmpbasename, p);
    *p = '\0';
    drop_extension(tmpbasename);
#if !defined(BSD42) && !defined(BSD43) && !defined(sun)
    /*
     *  we don't depend on pathconf here, if it returns an incorrect value
     * for NAME_MAX (like Linux 0.97 with minix FS) finding a unique name
     * for temp files can fail.
     */
    tmpbasename[10] = '\0';	/* 14 char limit */
#endif
}

/*
 * return a unique temporary file name
 */
char *tempfile(int flags)
{
    int i, j;
    int num;
    int fd;
#ifndef UNIX
    FILE *fp;
#endif

    for (i = 0; i < MAXTMPF; ++i)
	if (tmpf[i].flags == 0)
	    break;

    if (i == MAXTMPF) {
	/* message only for debugging, no need for LANG */
	fprintf(stderr, "\n\007Out of temporary files\n");
	return NULL;
    }
  again:
    num = 0;
    do {
	for (j = 0; j < MAXTMPF; ++j)
	    if (tmpf[j].flags && tmpf[j].num == num)
		break;
	if (j < MAXTMPF)
	    continue;		/* sequence number already in use */
	sprintf(tmpf[i].path, "%s%s.%c%02d",
		((flags & TMP_TMPDIR) && *tmpdir ? tmpdir : outdir),
		tmpbasename, TMP_EXT, num);
	if (!file_exists(tmpf[i].path))
	    break;
    }
    while (++num < 100);

    if (num == 100) {
	fprintf(pgpout, "\n\007tempfile: cannot find unique name\n");
	return NULL;
    }
#if defined(UNIX) || defined(VMS)
    if ((fd = open(tmpf[i].path, O_EXCL | O_RDWR | O_CREAT, 0600)) != -1)
	close(fd);
#else
    if ((fp = fopen(tmpf[i].path, "w")) != NULL)
	fclose(fp);
    fd = (fp == NULL ? -1 : 0);
#endif

    if (fd == -1) {
	if (!(flags & TMP_TMPDIR)) {
	    flags |= TMP_TMPDIR;
	    goto again;
	}
#ifdef UNIX
	else if (tmpdir[0] == '\0') {
	    strcpy(tmpdir, "/tmp/");
	    goto again;
	}
#endif
    }
    if (fd == -1) {
	fprintf(pgpout, LANG("\n\007Cannot create temporary file '%s'\n"),
		tmpf[i].path);
	user_error();
    }
#if defined(VMS) || defined(C370)
    remove(tmpf[i].path);
#endif

    tmpf[i].num = num;
    tmpf[i].flags = flags | TMP_INUSE;
    if (verbose)
	fprintf(pgpout, "tempfile: created '%s'\n", tmpf[i].path);
    return tmpf[i].path;
}				/* tempfile */

/*
 * remove temporary file, wipe if necessary.
 */
void rmtemp(char *name)
{
    int i;

    for (i = 0; i < MAXTMPF; ++i)
	if (tmpf[i].flags && strcmp(tmpf[i].path, name) == 0)
	    break;

    if (i < MAXTMPF) {
	if (strlen(name) > 3 && name[strlen(name) - 3] == TMP_EXT) {
	    /* only remove file if name hasn't changed */
	    if (verbose)
		fprintf(pgpout, "rmtemp: removing '%s'\n", name);
	    if (tmpf[i].flags & TMP_WIPE)
		wipefile(name);
	    if (!remove(name)) {
		tmpf[i].flags = 0;
	    } else if (verbose) {
		fprintf(stderr, "\nrmtemp: Failed to remove %s", name);
		perror("\nError");
	    }
	} else if (verbose)
	    fprintf(pgpout, "rmtemp: not removing '%s'\n", name);
    }
}				/* rmtemp */

/*
 * make temporary file permanent, returns the new name.
 */
char *savetemp(char *name, char *newname)
{
    int i, overwrite;

    if (strcmp(name, newname) == 0)
	return name;

    for (i = 0; i < MAXTMPF; ++i)
	if (tmpf[i].flags && strcmp(tmpf[i].path, name) == 0)
	    break;

    if (i < MAXTMPF) {
	if (strlen(name) < 4 || name[strlen(name) - 3] != TMP_EXT) {
	    if (verbose)
		fprintf(pgpout, "savetemp: not renaming '%s' to '%s'\n",
			name, newname);
	    return name;	/* return original file name */
	}
    }

    newname = ck_dup_output(newname, FALSE, TRUE);
    if (newname==NULL)
        return(NULL);

    if (verbose)
	fprintf(pgpout, "savetemp: renaming '%s' to '%s'\n", name, newname);
    if (rename2(name, newname) < 0) {
	/* errorLvl = UNKNOWN_FILE_ERROR; */
	fprintf(pgpout, LANG("Can't create output file '%s'\n"), newname);
	return NULL;
    }
    if (i < MAXTMPF)
	tmpf[i].flags = 0;
    return newname;
} /* savetemp */

char *ck_dup_output(char *newname, boolean notest, boolean delete_dup)
{
    int overwrite;
    static char buf[MAX_PATH];

    while (file_exists(newname)) {
	if (batchmode && !force_flag) {
            fprintf(pgpout,LANG("\n\007Output file '%s' already exists.\n"),
                    newname);
	    return NULL;
	}
	if (is_regular_file(newname)) {	
	    if (force_flag) {
		/* remove without asking */
		if (delete_dup) remove(newname);
		break;
	    }
	    fprintf(pgpout,
	    LANG("\n\007Output file '%s' already exists.  Overwrite (y/N)? "),
		 newname);
	    overwrite = getyesno('n');
	} else {
	    fprintf(pgpout,
		    LANG("\n\007Output file '%s' already exists.\n"),newname);
	    if (force_flag)	/* never remove special file */
		return NULL;
	    overwrite = FALSE;
	}

	if (!overwrite) {
	    fprintf(pgpout, "\n");
	    fprintf(pgpout, LANG("Enter new file name:"));
	    fprintf(pgpout, " ");
#ifdef MACTC5
			if (!GetFilePath(LANG("Enter new file name:"), buf, PUTFILE))
				return(NULL);
			strcpy(newname, buf);
			fprintf(pgpout, "%s\n",buf);
#else
	    getstring(buf, MAX_PATH - 1, TRUE);
	    if (buf[0] == '\0')
		return(NULL);
	    newname = buf;
#endif
	} else if (delete_dup)
            remove(newname);
        else
            break;

        if (notest) break;
    }
    return(newname);
} /* ck_dup_output */

/*
 * like savetemp(), only make backup of destname if it exists
 */
int savetempbak(char *tmpname, char *destname)
{
    char bakpath[MAX_PATH];
#ifdef MACTC5
	byte header[8];
#endif
#ifdef UNIX
    int mode = -1;
#endif

    if (is_tempfile(destname)) {
	remove(destname);
    } else {
	if (file_exists(destname)) {
#ifdef UNIX
	    struct stat st;
	    if (stat(destname, &st) != -1)
		mode = st.st_mode & 07777;
#endif
	    strcpy(bakpath, destname);
	    force_extension(bakpath, BAK_EXTENSION);
	    remove(bakpath);
#if defined(VMS) || defined(C370)
	    if (rename(destname, bakpath) != 0)
#else
	    if (rename(destname, bakpath) == -1)
#endif
		return -1;
#ifdef MACTC5
		get_header_info_from_file(bakpath, header, 8 );
		if (header[0] == CTB_CERT_SECKEY)
			PGPSetFinfo(bakpath,'SKey','MPGP');
		if (header[0] == CTB_CERT_PUBKEY)
			PGPSetFinfo(bakpath,'PKey','MPGP');
#endif
	}
    }
    if (savetemp(tmpname, destname) == NULL)
	return -1;
#if defined(UNIX)
    if (mode != -1)
	chmod(destname, mode);
#elif defined(MACTC5)
	get_header_info_from_file(destname, header, 8 );
	if (header[0] == CTB_CERT_SECKEY)
		PGPSetFinfo(destname,'SKey','MPGP');
	if (header[0] == CTB_CERT_PUBKEY)
		PGPSetFinfo(destname,'PKey','MPGP');
#endif
    return 0;
}

/*
 * remove all temporary files and wipe them if necessary
 */
void cleanup_tmpf(void)
{
    int i;

    for (i = 0; i < MAXTMPF; ++i)
	if (tmpf[i].flags)
	    rmtemp(tmpf[i].path);
}				/* cleanup_tmpf */

#ifdef MACTC5
void mac_cleanup_tmpf(void)
{
	int i,err;
    HFileParam pb;
    char fname[256];
	for (i = 0; i < MAXTMPF; ++i)
		if (tmpf[i].flags)
	       {
	        strcpy(fname,tmpf[i].path);
	        pb.ioCompletion=nil;
	        c2pstr(fname);
	        pb.ioNamePtr=(uchar *)fname;
	        pb.ioVRefNum=0;
	        pb.ioFDirIndex=0;
	        pb.ioFRefNum=0;
	        pb.ioDirID=0;
	        err=PBHGetFInfo((HParmBlkPtr)&pb,false);
	        if (pb.ioFRefNum!=0){
	            strcpy(fname,tmpf[i].path);
	            pb.ioCompletion=nil;
	            c2pstr(fname);
	            pb.ioNamePtr=(uchar *)fname;
	            pb.ioVRefNum=0;
	            pb.ioDirID=0;
	            err=PBClose((ParmBlkPtr)&pb,false);
	            }
			rmtemp(tmpf[i].path);
			}
}	/* mac_cleanup_tmpf */
#endif

/* 
 * Routines to search for the manuals.
 *
 * Why all this code?
 *
 * Some people may object to PGP insisting on finding the manual somewhere
 * in the neighborhood to generate a key.  They bristle against this
 * seemingly authoritarian attitude.  Some people have even modified PGP
 * to defeat this feature, and redistributed their hotwired version to
 * others.  That creates problems for me (PRZ).
 * 
 * Here is the problem.  Before I added this feature, there were maimed
 * versions of the PGP distribution package floating around that lacked
 * the manual.  One of them was uploaded to Compuserve, and was
 * distributed to countless users who called me on the phone to ask me why
 * such a complicated program had no manual.  It spread out to BBS systems
 * around the country.  And a freeware distributor got hold of the package
 * from Compuserve and enshrined it on CD-ROM, distributing thousands of
 * copies without the manual.  What a mess.
 * 
 * Please don't make my life harder by modifying PGP to disable this
 * feature so that others may redistribute PGP without the manual.  If you
 * run PGP on a palmtop with no memory for the manual, is it too much to
 * ask that you type one little extra word on the command line to do a key
 * generation, a command that is seldom used by people who already know
 * how to use PGP?  If you can't stand even this trivial inconvenience,
 * can you suggest a better method of reducing PGP's distribution without
 * the manual?
 */

static unsigned ext_missing(char *prefix)
{
    static char const *const extensions[] =
#ifdef VMS
	{ ".doc", ".txt", ".man", ".tex", ".", 0 };
#else
	{ ".doc", ".txt", ".man", ".tex", "", 0 };
#endif
    char const *const *p;
    char *end = prefix + strlen(prefix);

    for (p = extensions; *p; p++) {
	strcpy(end, *p);
#if 0				/* Debugging code */
	fprintf(pgpout, "Looking for \"%s\"\n", prefix);
#endif
	if (file_exists(prefix))
	    return 0;
    }
    return 1;
}

/*
 * Returns mask of files missing
 */
static unsigned files_missing(char *prefix)
{
/* This changed to incorporate the changes in the Documentation subdirectory */
#ifdef MACTC5
	static char const *const names[] = 
	{"Volume I", "Volume II", 0};
#else
    static char const *const names[] =
    {"pgpdoc1", "pgpdoc2", 0};
#endif
    char const *const *p;
    unsigned bit, mask = 3;
    int len = strlen(prefix);

#ifndef MACTC5
	/* Cannot do this on the macintosh because file_exists returns false on
	   directories */
#ifndef VMS
    /*
     * Optimization: if directory doesn't exist, stop.  But access()
     * (used internally by file_exists()) doesn't work on dirs under VMS.
     */
    if (prefix[0] && !file_exists(prefix))	/* Directory doesn't exist? */
	return mask;
#endif /* VMS */
#endif /* MACTC5 */
    if (len && strchr(DIRSEPS, prefix[len - 1]) == 0)
	prefix[len++] = DIRSEPS[0];
    for (p = names, bit = 1; *p; p++, bit <<= 1) {
	strcpy(prefix + len, *p);
	if (!ext_missing(prefix))
	    mask &= ~bit;
    }

    return mask;		/* Bitmask of which files exist */
}

/*
 * Search prefix directory and doc subdirectory.
 */
static unsigned doc_missing(char *prefix)
{
    unsigned mask;
    int len = strlen(prefix);

    mask = files_missing(prefix);
    if (!mask)
	return 0;
#if defined(VMS)
    if (len && prefix[len - 1] == ']') {
	strcpy(prefix + len - 1, ".doc]");
    } else {
	assert(!len || prefix[len - 1] == ':');
	strcpy(prefix + len, "[doc]");
    }
#elif defined(MACTC5)
	/* on the macintosh we must look for the documents in 
	   Documentation:PGP User's Guide: folder  */
	if (len && prefix[len - 1] != DIRSEPS[0])
		prefix[len++] = DIRSEPS[0];
	strcpy(prefix + len, "Documentation");
	len = strlen(prefix);
	mask &= files_missing(prefix);
	if (!mask)
		return 0;
	if (len && prefix[len - 1] != DIRSEPS[0])
		prefix[len++] = DIRSEPS[0];
	strcpy(prefix + len, "PGP User's Guide");
	mask &= files_missing(prefix);
	if (!mask)
		return 0;
#else
    if (len && prefix[len - 1] != DIRSEPS[0])
	prefix[len++] = DIRSEPS[0];
    strcpy(prefix + len, "doc");
#endif

    mask &= files_missing(prefix);

    prefix[len] = '\0';
    return mask;
}

/*
 * Expands a leading environment variable.  Returns 0 on success;
 * <0 if there is an error.
 */
static int expand_env(char const *src, char *dest)
{
    char const *var, *suffix;
    unsigned len;

    if (*src != '$') {
	strcpy(dest, src);
	return 0;
    }
    /* Find end of variable */
    if (src[1] == '{') {	/* ${FOO} form */
	var = src + 2;
	len = strchr(var, '}') - (char*) var;
	suffix = src + 2 + len + 1;
    } else {			/* $FOO form - allow $ for VMS */
	var = src + 1;
	len = strspn(var, "ABCDEFGHIJKLMNOPQRSTUVWXYZ$_");
	suffix = src + 1 + len;
    }

    memcpy(dest, var, len);	/* Copy name */
    dest[len] = '\0';		/* Null-terminate */

    var = getenv(dest);
    if (!var || !*var)
	return -1;		/* No env variable */

    /* Copy expanded form to destination */
    strcpy(dest, var);

    /* Add tail */
    strcat(dest, suffix);

    return 0;
}

/* Don't forget to change 'pgp26' whenever you update rel_version past 2.6 */
char const *const manual_dirs[] =
{
#if defined(VMS)
    "$PGPPATH", "", "[pgp]", "[pgp26]", "[pgp263]",
    PGP_SYSTEM_DIR, "SYS$LOGIN:", "SYS$LOGIN:[pgp]",
    "SYS$LOGIN:[pgp26]", "SYS$LOGIN:[pgp263]", "[-]",
#elif defined(UNIX)
    "$PGPPATH", "", "pgp", "pgp26", "pgp263", PGP_SYSTEM_DIR,
    "$HOME/.pgp", "$HOME", "$HOME/pgp", "$HOME/pgp26", "..",
#elif defined(AMIGA)
    "$PGPPATH", "", "pgp", "pgp26", ":pgp", ":pgp26", ":pgp263", 
    ":", "/",
#else				/* MSDOS or ATARI */
    "$PGPPATH", "", "pgp", "pgp26", "\\pgp", "\\pgp26", "\\pgp263", 
    "\\", "..", "c:\\pgp", "c:\\pgp26",
#endif
    0};

#ifdef MACTC5
extern char appPathName[];
#endif

unsigned manuals_missing(void)
{
    char buf[256];
    unsigned mask = ~((unsigned)0);
    char const *const *p;

#ifdef MACTC5
	strcpy(buf, appPathName);
	mask &= doc_missing(buf);
	return mask;
#endif /* MACTC5 */
    for (p = manual_dirs; *p; p++) {
	if (expand_env(*p, buf) < 0)
	    continue;		/* Ignore */
	mask &= doc_missing(buf);
	if (!mask)
	    break;
    }

    return mask;
}

/* 
 * Why all this code?
 *
 * See block of comments above.
 */
