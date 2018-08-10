/*
 * system.c
 *
 * Routines specific for non-MSDOS implementations of pgp.
 * 
 * (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
 * The author assumes no liability for damages resulting from the use
 * of this software, even if the damage results from defects in this
 * software.  No warranty is expressed or implied.
 *
 * Note that while most PGP source modules bear Philip Zimmermann's
 * copyright notice, many of them have been revised or entirely written
 * by contributors who frequently failed to put their names in their
 * code.  Code that has been incorporated into PGP from other authors
 * was either originally published in the public domain or is used with
 * permission from the various authors.
 *
 * PGP is available for free to the public under certain restrictions.
 * See the PGP User's Guide (included in the release package) for
 * important information about licensing, patent restrictions on
 * certain algorithms, trademarks, copyrights, and export controls.
 *
 *	Modified 24-Jun-92 HAJK
 *	Adapt for VAX/VMS.
 *
 *	Modified: 11-Nov-92 HAJK
 *	Add FDL Support Routines. 
 *
 *	Modified: 31-Jan-93 HAJK
 *	Misc. updates for terminal handling.
 *	Add VMS command stuff.
 *	Add fileparse routine.
 */
#include <stdio.h>
#include <unistd.h>
#include "exitpgp.h"
#include "system.h"
#include "usuals.h"

/*===========================================================================*/
/*
 * UNIX
 */

#ifdef UNIX
/*
 * Define USE_SELECT to use the select() system call to check if
 * keyboard input is available. Define USE_NBIO to use non-blocking
 * read(). If you don't define anything the FIONREAD ioctl() command
 * will be used.
 *
 * Define NOTERMIO if you don't have the termios stuff
 */
#include <sys/types.h>
#include <fcntl.h>

#ifndef	NOTERMIO
#ifndef SVR2
#include <termios.h>
#else
#include <termio.h>
#endif /* not SVR2 */
#else
#include <sgtty.h>
#endif

#ifdef	USE_SELECT
#include <sys/time.h>
#ifdef _IBMR2
#include <sys/select.h>
#endif /* _IBMR2 */
#else
#ifndef USE_NBIO
#ifndef sun
#include <sys/ioctl.h>		/* for FIONREAD */
#else /* including both ioctl.h and termios.h gives a lot of warnings on sun */
#include <sys/filio.h>
#endif /* sun */
#ifndef FIONREAD
#define	FIONREAD	TIOCINQ
#endif
#endif
#endif
#include <signal.h>
#include <unistd.h>

static void setsigs(void);
static void rmsigs(void);
static void sig1(int);
static void sig2(int);
void breakHandler(int);
static int ttyfd= -1;
#ifndef SVR2
static void (*savesig)(int);
#else
static int (*savesig)(int);
#endif

void ttycbreak(void);
void ttynorm(void);

#ifndef NEED_KBHIT
#undef USE_NBIO
#endif

#ifndef NOTERMIO
#ifndef SVR2
static struct termios itio, tio;
#else
static struct termio itio, tio;
#endif /* not SVR2 */
#else
static struct sgttyb isg, sg;
#endif

#ifdef USE_NBIO
static int kbuf= -1;	/* buffer to store char read by kbhit() */
static int fflags;
#endif

static int gottio = 0;

void ttycbreak(void)
{
	if (ttyfd == -1) {
		if ((ttyfd = open("/dev/tty", O_RDWR)) < 0) {
		    fprintf(stderr, "cannot open tty, using stdin\n");
			ttyfd = 0;
		}
	}
#ifndef NOTERMIO
#ifndef SVR2
	if (tcgetattr(ttyfd, &tio) < 0)
#else
	if (ioctl(ttyfd, TCGETA, &tio) < 0)
#endif  /* not SVR2 */
	{
		fprintf (stderr, "\nUnable to get terminal characteristics: ");
		perror("ioctl");
		exitPGP(1);
	}
	itio = tio;
	setsigs();
	gottio = 1;
#ifdef USE_NBIO
	tio.c_cc[VMIN] = 0;
#else
	tio.c_cc[VMIN] = 1;
#endif
	tio.c_cc[VTIME] = 0;
	tio.c_lflag &= ~(ECHO|ICANON);
#ifndef SVR2
#ifdef ultrix
	/* Ultrix is broken and flushes the output as well! */
	tcsetattr (ttyfd, TCSANOW, &tio);
#else
	tcsetattr (ttyfd, TCSAFLUSH, &tio);
#endif
#else
	ioctl(ttyfd, TCSETAF, &tio);
#endif /* not SVR2 */
#else
	if (ioctl(ttyfd, TIOCGETP, &sg) < 0) {
		fprintf (stderr, "\nUnable to get terminal characteristics: ");
		perror("ioctl");
		exitPGP(1);
	}
	isg = sg;
	setsigs();
	gottio = 1;
#ifdef CBREAK
    sg.sg_flags |= CBREAK;
#else
    sg.sg_flags |= RAW;
#endif
	sg.sg_flags &= ~ECHO;
    ioctl(ttyfd, TIOCSETP, &sg);
#endif	/* !NOTERMIO */
#ifdef USE_NBIO
#ifndef O_NDELAY
#define	O_NDELAY	O_NONBLOCK
#endif
	if ((fflags = fcntl(ttyfd, F_GETFL, 0)) != -1)
		fcntl(ttyfd, F_SETFL, fflags|O_NDELAY);
#endif
}


void ttynorm(void)
{	gottio = 0;
#ifdef USE_NBIO
	if (fcntl(ttyfd, F_SETFL, fflags) == -1)
		perror("fcntl");
#endif
#ifndef NOTERMIO
#ifndef SVR2
#ifdef ultrix
	/* Ultrix is broken and flushes the output as well! */
	tcsetattr (ttyfd, TCSANOW, &itio);
#else
	tcsetattr (ttyfd, TCSAFLUSH, &itio);
#endif
#else
	ioctl(ttyfd, TCSETAF, &itio);
#endif /* not SVR2 */
#else
    ioctl(ttyfd, TIOCSETP, &isg);
#endif
	rmsigs();
}

static void sig1 (int sig)
{
#ifndef NOTERMIO
#ifndef SVR2
	tcsetattr (ttyfd, TCSANOW, &itio);
#else
	ioctl(ttyfd, TCSETAW, &itio);
#endif /* not SVR2 */
#else
    ioctl(ttyfd, TIOCSETP, &isg);
#endif
	signal (sig, SIG_DFL);
	if (sig == SIGINT)
		breakHandler(SIGINT);
	kill (getpid(), sig);
}

static void sig2 (int sig)
{
	if (gottio)
		ttycbreak();
	else
		setsigs();
}

static void setsigs(void)
{
	savesig = signal (SIGINT, sig1);
#ifdef	SIGTSTP
	signal (SIGCONT, sig2);
	signal (SIGTSTP, sig1);
#endif
}

static void rmsigs(void)
{	signal (SIGINT, savesig);
#ifdef	SIGTSTP
	signal (SIGCONT, SIG_DFL);
	signal (SIGTSTP, SIG_DFL);
#endif
}

#ifdef NEED_KBHIT
#ifndef CRUDE
int kbhit(void)
/* Return TRUE if there is a key to be read */
{
#ifdef USE_SELECT		/* use select() system call */
	struct timeval t;
	fd_set n;
	int r;

	timerclear(&t);
	FD_ZERO(&n);
	FD_SET(ttyfd, &n);
	r = select(32, &n, NULL, NULL, &t);
	if (r == -1) {
		perror("select");
		exitPGP(1);
	}
	return r > 0;
#else
#ifdef	USE_NBIO		/* use non-blocking read() */
	unsigned char ch;
	if (kbuf >= 0) 
		return(1);
	if (read(ttyfd, &ch, 1) == 1) {
		kbuf = ch;
		return(1);
	}
	return(0);
#else
	long lf;
	if (ioctl(ttyfd, FIONREAD, &lf) == -1) {
		perror("ioctl: FIONREAD");
		exitPGP(1);
	}
	return(lf);
#endif
#endif
}
#endif	/* !CRUDE */
#endif

int getch(void)
{
	char c;
#ifdef USE_NBIO
	while (!kbhit());	/* kbhit() does the reading */
	c = kbuf;
	kbuf = -1;
#else
 	c = 0;
	read(ttyfd, &c, 1);
#endif
 	return(c & 0xFF);
}

#if defined(_BSD) && !defined(__STDC__)

VOID *memset(s, c, n)
VOID *s;
register int c, n;
{
	register char *p = s;
	++n;
	while (--n)
		*p++ = c;
	return(s);
}
int memcmp(s1, s2, n)
register unsigned char *s1, *s2;
register int n;
{
	if (!n)
		return(0);
	while (--n && *s1 == *s2) {
		++s1;
		++s2;
	}
	return(*s1 - *s2);
}
VOID *memcpy(s1, s2, n)
register char *s1, *s2;
register int n;
{
	char *p = s1;
	++n;
	while (--n)
		*s1++ = *s2++;
	return(p);
}
#endif /* _BSD */

#if (defined(MACH) || defined(SVR2) || defined(_BSD)) && !defined(NEXT) \
&& !defined(AUX) && !defined(__MACHTEN__) || (defined(sun) && defined(i386))
int remove(name)
char *name;
{
	return unlink(name);
}
#endif

#if defined(SVR2) && !defined(AUX)
int rename(old, new)
register char *old, *new;
{
	unlink(new);
	if (link(old, new) < 0)
		return -1;
	if (unlink(old) < 0) {
		unlink(new);
		return -1;
	}
	return 0;
}
#endif /* SVR2 */

/* not all unices have clock() */
long
Clock()	/* not a replacement for clock(), just for random number generation */
{
#if defined(_BSD) || (defined(sun) && !defined(SOLARIS)) || \
defined(MACH) || defined(linux)
#include <sys/time.h>
#include <sys/resource.h>
	struct rusage ru;

	getrusage(RUSAGE_SELF, &ru);
	return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec +
		ru.ru_stime.tv_sec + ru.ru_stime.tv_usec +
		ru.ru_minflt + ru.ru_majflt +
		ru.ru_inblock + ru.ru_oublock +
		ru.ru_maxrss + ru.ru_nvcsw + ru.ru_nivcsw;

#else	/* no getrusage() */
#include <sys/times.h>
	struct tms tms;

	times(&tms);
	return(tms.tms_utime + tms.tms_stime);
#endif
}
#endif /* UNIX */


/*===========================================================================*/
/*
 * VMS
 */

#ifdef VMS			/* kbhit()/getch() equivalent */

/*
 * This code defines an equivalent version of kbhit() and getch() for
 * use under VAX/VMS, together with an exit handler to reset terminal
 * characteristics.
 *
 * This code assumes that kbhit() has been invoked to test that there
 * are characters in the typeahead buffer before getch() is invoked to
 * get the answer.
 */

#include <signal.h>
#include <string.h>
#include <file.h>
#include <ctype.h>
#include "pgp.h"
#include "mpilib.h"
#include "mpiio.h"
#include "fileio.h"
extern byte textbuf[DISKBUFSIZE];   /*	Defined in FILEIO.C */

/*	  
**  VMS Private Macros
*/	  
#include <descrip.h>
#include <devdef>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <dcdef.h>
#include <climsgdef.h>
#include <rms.h>
#include <hlpdef.h>

#define MAX_CMDSIZ	256  /*  Maximum command size */
#define MAX_FILENM	255 /* Mamimum file name size */

#define FDL$M_FDL_STRING    2		/* Use string for fdl text */
#define FDLSIZE		    4096	/* Maximum possible file size */

#ifdef _USEDCL_

/*
 * Declare some external procedure prototypes (saves me confusion!)
 */
extern int lib$get_input(
	    struct dsc$descriptor *resultant,
	    struct dsc$descriptor *prompt, 
	    unsigned short *resultant_length);
extern int lib$put_output(
	    struct dsc$descriptor *output);
extern int lib$sig_to_ret();
/*	  
**  The CLI routines are documented in the system routines manual.
*/	  
extern int cli$dcl_parse(
	    struct dsc$descriptor *command,
	    char cmd_table[],
	    int (*get_command)(
		struct dsc$descriptor *resultant,
		struct dsc$descriptor *prompt, 
		unsigned short *resultant_length),
	    int (*get_parameter)(
		struct dsc$descriptor *resultant,
		struct dsc$descriptor *prompt, 
		unsigned short *resultant_length),
	    struct dsc$descriptor *prompt);
extern int cli$present( struct dsc$descriptor *object);
extern int cli$_get_value(
	    struct dsc$descriptor *object,
	    struct dsc$decsriptor *value,
	    unsigned short *value_len);
/*
 * Static Data
 */
static $DESCRIPTOR (cmdprmt_d, "DROPSAFE> ");  /*  Prompt string */

#endif /* _USEDCL_ */

static volatile short	_kbhitChan_ = 0;

static volatile struct IOSB {
	unsigned short sts;
	unsigned short byteCount;
	unsigned short terminator;
	unsigned short terminatorSize;
	} iosb;

static $DESCRIPTOR (kbdev_desc, "SYS$COMMAND:");

static volatile struct {
	char Class;
	char Type;
	unsigned short BufferSize;
	unsigned int Mode;
	int ExtChar;
  } CharBuf, OldCharBuf;

static $DESCRIPTOR (out_file_descr, "SYS$DISK:[]"); /* Default Output
						       File Descr */

static int flags = FDL$M_FDL_STRING;

/*
 * **-kbhit_handler-This exit handler restores the terminal characteristics
 *
 * Description:
 *
 * This procedure is invoked to return the the terminal to normality (depends
 * on what you think is normal!). Anyway, it gets called to restore
 * characteristics either through ttynorm or via an exit handler.
 */
static void kbhit_handler(int *sts)
{
  ttynorm();
  (void) sys$dassgn (
	  _kbhitChan_);
  _kbhitChan_ = 0;
}

/*
 * Data Structures For Linking Up Exit Handler 
 */
unsigned int exsts;

static struct {
	int link;
	VOID *rtn;
	int argcnt;
	int *stsaddr;
   } exhblk = { 0, &(kbhit_handler), 1, &(exsts)};
/*
 * **-kbhit_Getchn-Get Channel
 *
 * Functional Description:
 *
 * Private routine to get a terminal channel and save the terminal
 * characteristics.
 *
 * Arguments:
 *
 *  None.
 *
 * Returns:
 *
 *  If 0, channel already assigned. If odd, then assign was successful
 * otherwise returns VMS error status.
 *
 * Implicit Inputs:
 *
 * _kbhitChan_	Channel assigned to the terminal (if any).
 *
 * Implicit Outputs:
 *
 *  OldCharBuf	Initial terminal characteristics.
 *  _kbhitChan_	Channel assigned to the terminal.
 *
 * Side Effects:
 *
 *  Establishes an exit handler to restore characteristics and deassign
 * terminal channel.
 */
static int kbhit_Getchn()
{
    int sts = 0;

    if (_kbhitChan_ == 0) {
	if ((sts = sys$assign (
			   &kbdev_desc,
			   &_kbhitChan_,
			   0,
			   0)) & 1) {
	    if ((sts = sys$qiow (
			       0,
			       _kbhitChan_,
			       IO$_SENSEMODE,
			       &iosb,
			       0,
			       0,
			       &OldCharBuf,
			       12,
			       0,
			       0,
			       0,
			       0)) & 01) sts = iosb.sts;
	    if (sts & 01) {
	      if (!(OldCharBuf.Class & DC$_TERM)) {
		fprintf(stderr,"\nNot running on a terminal");
		exitPGP(1);
	      }
	      (void) sys$dclexh (&exhblk);
	    }
	}
    }
    return(sts);
}
/*	  
 * **-ttynorm-Restore initial terminal characteristics
 *
 * Functional Description:
 *
 * This procedure is invoked to restore the initial terminal characteristics.
 */
void ttynorm()
/*
 * Arguments:
 *
 *  None.
 *
 * Implicit Inputs:
 *
 *  OldCharBuf	Initial terminal characteristics.
 *  _kbhitChan_	Channel assigned to the terminal.
 *
 * Implicit Outputs:
 *
 *  None.
 */	  
{
  int sts;

  if (_kbhitChan_ != 0) {
      CharBuf.Mode = OldCharBuf.Mode;
      CharBuf.ExtChar = OldCharBuf.ExtChar;
    /*
      CharBuf.Mode &= ~TT$M_NOECHO;
      CharBuf.ExtChar &= ~TT2$M_PASTHRU;
    */
      if ((sts = sys$qiow (
			       0,
			       _kbhitChan_,
			       IO$_SETMODE,
			       &iosb,
			       0,
			       0,
			       &OldCharBuf,
			       12,
			       0,
			       0,
			       0,
			       0)) & 01) sts = iosb.sts;
      if (!(sts & 01)) {
	    fprintf(stderr,"\nFailed to reset terminal characteristics!");
	    (void) lib$signal(sts);
      }
   }
   return;
}
/*
 * **-kbhit-Find out if a key has been pressed
 *
 * Description:
 *
 * Make the terminal noecho and sense the characters coming in by looking at
 * the typeahead count. Note that the character remains in the typeahead buffer
 * untill either read, or that the user types a Control-X when not in 'passall'
 * mode.
 */
int kbhit()
/*
 * Arguments:
 *
 *  None.
 *
 * Returns:
 *
 *  TRUE  if there is a character in the typeahead buffer.
 *  FALSE if there is no character in the typeahead buffer.
 */


{
  int sts;

  struct {
	unsigned short TypAhdCnt;
	char FirstChar;
	char Reserved[5];
  } TypCharBuf;

  /*
  **  Get typeahead count
  */
  if ((sts = sys$qiow (
			   0,
			   _kbhitChan_,
			   IO$_SENSEMODE | IO$M_TYPEAHDCNT,
			   &iosb,
			   0,
			   0,
			   &TypCharBuf,
			   8,
			   0,
			   0,
			   0,
			   0)) & 01) sts = iosb.sts;
  if (sts & 01) return(TypCharBuf.TypAhdCnt>0);
  (void) lib$signal(sts);
  exitPGP(1);
}

static int NoTerm[2] = { 0, 0};  /*  TT Terminator Mask (Nothing) */

/*
 * **-getch-Get a character and return it
 *
 * Description:
 *
 * Get a character from the keyboard and return it. Unlike Unix, the character
 * will be explicitly echoed unless ttycbreak() has been called first. If the
 * character is in the typeahead, that will be read first.
 */
int getch()
/*
 * Arguments:
 *
 *  None.
 *
 * Returns:
 *
 *  Character Read.
 */
{
  unsigned int sts;
  volatile char CharBuf;

  if (((sts = kbhit_Getchn()) & 01) || sts == 0) {
      if ((sts = sys$qiow (
			      0,
			      _kbhitChan_,
			      IO$_READVBLK,
			      &iosb,
			      0,
			      0,
			      &CharBuf,
			      1,
			      0,
			      &NoTerm,
			      0,
			      0)) & 01) sts = iosb.sts;
  }
  if (sts & 01) return ((int) CharBuf);
  fprintf(stderr,"\nFailed to get character");
  (void) lib$signal(sts);
}
/*
 * **-putch-Put Character To 'Console' Device
 *
 * This procedure is a companion to getch, outputing a character to the
 * terminal with a minimum of fuss (no VAXCRTLK, no RMS!). This routine
 * simply gets a channel (if there isn't one already and uses QIO to
 * output.
 *
 */
int putch(int chr)
/*
 * Arguments:
 *  chr		Character to output.
 *
 * Returns:
 *
 *  Status return from Getchn and qio.
 *
 * Side Effects
 *
 * May assign a channel to the terminal.
 */
{
  unsigned int sts;

  if (((sts = kbhit_Getchn()) & 01) || sts == 0) {
      if ((sts = sys$qiow (
			      0,
			      _kbhitChan_,
			      IO$_WRITEVBLK,
			      &iosb,
			      0,
			      0,
			      &chr,
			      1,
			      0,
			      0,
			      0,
			      0)) & 01) sts = iosb.sts;
  }
  if (sts & 01) return (sts);
  fprintf(stderr,"\nFailed to put character");
  (void) lib$signal(sts);
}
/*
 * **-ttycbreak-Set Unix-like Cbreak mode
 *
 * Functional Description:
 *
 * This code must be invoked to produce the Unix-like cbreak operation which
 * disables echo, allows control character input.
 */
void ttycbreak ()
/*
 * Arguments:
 *
 *  None.
 *
 * Returns:
 *
 *  None.
 *
 * Side Effects
 *
 * May assign a channel to the terminal.
 */
{
    struct {
	unsigned short TypAhdCnt;
	char FirstChar;
	char Reserved[5];
    } TypCharBuf;
    char buf[80];
    int sts;

    if (((sts = kbhit_Getchn()) & 01) || sts == 0) {
/*
 * Flush any typeahead before we change characteristics
 */
	if ((sts = sys$qiow (
			       0,
			       _kbhitChan_,
			       IO$_SENSEMODE | IO$M_TYPEAHDCNT,
			       &iosb,
			       0,
			       0,
			       &TypCharBuf,
			       8,
			       0,
			       0,
			       0,
			       0)) & 01) sts = iosb.sts;
	if (sts) {
	    if (TypCharBuf.TypAhdCnt>0) {
		if ((sts = sys$qiow (
			    0,
			   _kbhitChan_,
			   IO$_READVBLK | IO$M_NOECHO | IO$M_TIMED,
			   &iosb,
			   0,
			   0,
			   &buf,
			   (TypCharBuf.TypAhdCnt >= 80 ? 80 :
			    TypCharBuf.TypAhdCnt),
			   1,
			   &NoTerm,
			   0,
			   0)) & 01) sts = iosb.sts;
			   
		if (sts)
		    TypCharBuf.TypAhdCnt -= iosb.byteCount;
	    }
	}
	if (!(sts & 01)) TypCharBuf.TypAhdCnt = 0;
/*
 * Modify characteristics
 */
	CharBuf = OldCharBuf;
	CharBuf.Mode = (OldCharBuf.Mode | TT$M_NOECHO) & ~TT$M_NOTYPEAHD;
	CharBuf.ExtChar = OldCharBuf.ExtChar | TT2$M_PASTHRU;
	if ((sts = sys$qiow (
		       0,
		       _kbhitChan_,
		       IO$_SETMODE,
		       &iosb,
		       0,
		       0,
		       &CharBuf,
		       12,
	    	       0,
		       0,
		       0,
		       0)) & 01) sts = iosb.sts;
	if (!(sts & 01)) {
	  fprintf(stderr,
		  "\nttybreak()- Failed to set terminal characteristics!");
	  (void) lib$signal(sts);
	  exitPGP(1);
	}
    }
}


#ifdef _USEDCL_

/*
 * **-vms_getcmd-Get VMS Style Foreign Command
 *
 * Functional Description:
 *
 *  Get command from VAX/VMS foreign command line interface and parse
 * according to DCL rules. If the command line is ok, it can then be
 * parsed according to the rules in the DCL command language table.
 *
 */
int vms_GetCmd( char *cmdtbl)
/*
 * Arguments:
 *
 *  cmdtbl	Pointer to command table to parse.
 *
 * Returns:
 *
 *  ...TBS...
 *
 * Implicit Inputs:
 *
 *  Command language table defined in DROPDCL.CLD
 */
{
    int sts;
    char cmdbuf[MAX_CMDSIZ];
    unsigned short cmdsiz;
    struct dsc$descriptor cmdbuf_d = {0,0,0,0};
    struct dsc$descriptor infile_d = {0,0,0,0};
    char filenm[MAX_FILENM];
    unsigned short filenmsiz;
    unsigned short verb_size;

    /*	  
    **  DCL Parse Expects A Command Verb Prefixing The Argumnents
    **	fake it!
    */	  
    verb_size = cmdprmt_d.dsc$w_length - 2;  /*  Loose '> ' characters */
    cmdbuf_d.dsc$w_length = MAX_CMDSIZ-verb_size-1;
    cmdbuf_d.dsc$a_pointer = strncpy(cmdbuf,cmdprmt_d.dsc$a_pointer,verb_size)
      +	verb_size+1;
    cmdbuf[verb_size++]=' ';
    if ((sts = lib$get_foreign (  /*  Recover command line from DCL */
	           &cmdbuf_d, 
	           0, 
	           &cmdsiz, 
	           0)) & 01) {
	cmdbuf_d.dsc$a_pointer = cmdbuf;
	cmdbuf_d.dsc$w_length = cmdsiz + verb_size;
	VAXC$ESTABLISH(lib$sig_to_ret);   /*  Force unhandled exceptions
					      to return */
        sts = cli$dcl_parse(  /*  Parse Command Line */
		    &cmdbuf_d,
		    cmdtbl,			
		    lib$get_input,
		    lib$get_input,
		    &cmdprmt_d);
    }
    return(sts);
}
/*
 * **-vms_TstOpt-Test for command qualifier present
 *
 * Functional Description:
 *
 * This procedure is invoked to test whether an option is present. It is
 * really just a jacket routine for the system routine CLI$PRESENT
 * converting the argument and result into 'C' speak.
 *
 */
vms_TstOpt(char opt)
/*
 * Arguments:
 *
 *  opt	    Character label of qualifier to test for.
 *
 * Returns:
 *
 *  +1	Option present.
 *  0	Option absent.
 *  -1	Option negated.
 *
 * Implicit Inputs:
 *
 * Uses DCL command line context established by vms_GetOpt.
 */
{
    int sts;
    char buf;
    struct dsc$descriptor option_d = { 1, 0, 0, &buf};

    buf = _toupper(opt);
    VAXC$ESTABLISH(lib$sig_to_ret);   /*  Force unhandled exceptions
					  to return */
    switch (sts=cli$present(&option_d))
    {

	case CLI$_PRESENT :
	    return(1);
	case CLI$_ABSENT:
	    return(0);
	case CLI$_NEGATED:
	    return(-1);
    	default:
	    return(0);
    }    
}
/*
 * **-vms_GetVal-Get Qualifier Value.
 *
 * Functional Description:
 *
 * This procedure is invoked to return the value associated with a
 * qualifier that exists (See TstOpt).
 */
vms_GetVal( char opt, char *resval, unsigned short maxsiz)
/*
 * Arguments:
 *
 *  opt	    Character label of qualifier to test for.
 *  resval  Pointer to resulting value string.
 *  maxsiz  Maximum size of string.
 *
 * Returns:
 *
 *  ...TBS...
 *
 * Implicit Inputs:
 *
 * Uses DCL command line context established by vms_GetOpt.
 */
{
    int sts;
    char buf;
    struct dsc$descriptor option_d = { 1, 0, 0, &buf};
    struct dsc$descriptor value_d = {maxsiz-1, 0, 0, resval };
    unsigned short valsiz;

    VAXC$ESTABLISH(lib$sig_to_ret);   /*  Force unhandled exceptions
					  to return */
    buf = _toupper(opt);
    if ((sts = cli$get_value( 
	    &option_d,
	    &value_d,
	    &valsiz)) & 01) resval[valsiz] = '\0';
    return(sts);
}
/*
 * **-vms_GetArg-Get Argument Value.
 *
 * Functional Description:
 *
 * This procedure is invoked to return the value associated with an
 * argument.
 */
vms_GetArg( unsigned short arg, char *resval, unsigned short maxsiz)
/*
 * Arguments:
 *
 *  arg	    Argument Number (1-9)
 *  resval  Pointer to resulting value string.
 *  maxsiz  Maximum size of string.
 *
 * Returns:
 *
 *  ...TBS...
 *
 * Implicit Inputs:
 *
 * Uses DCL command line context established by vms_GetOpt.
 */
{
    int sts;
    char buf[2] = "P";
    struct dsc$descriptor option_d = { 2, 0, 0, buf};
    struct dsc$descriptor value_d = {maxsiz-1, 0, 0, resval };
    unsigned short valsiz;

    VAXC$ESTABLISH(lib$sig_to_ret);   /*  Force unhandled exceptions
					  to return */
    buf[1] = arg + '0';
    if ((sts = cli$present(&option_d)) & 01) {
	if ((sts = cli$get_value( 
	    &option_d,
	    &value_d,
	    &valsiz)) & 01) resval[valsiz] = '\0';
    } else return(0);
    return(sts);
}



/*
 * **-do_help-Invoke VMS Help Processor
 *
 * Functional Description:
 *
 * This procedure is invoked to display a suitable help message to the caller
 * using the standard VMS help library.
 *
 */
do_help(char *helptext, char *helplib)
/*
 * Arguments:
 *
 *  helptext	Text of help request.
 *  helplib	Help library.
 *
 * Returns:
 *
 * As for kbhit_Getchn and lbr$output_help.
 *
 * Side Effects:
 *
 * A channel may be opened to the terminal. A library is opened.
 */
{
    int sts;
    int helpflags;
    struct dsc$descriptor helptext_d = { strlen(helptext), 0, 0, helptext};
    struct dsc$descriptor helplib_d = { strlen(helplib), 0, 0, helplib};

    VAXC$ESTABLISH(lib$sig_to_ret);   /*  Force unhandled
					  exceptions to return */
    if (((sts = kbhit_Getchn()) & 01) || sts == 0) {
	helpflags = HLP$M_PROMPT|HLP$M_SYSTEM|HLP$M_GROUP|HLP$M_PROCESS;    
	sts = lbr$output_help(
		    lib$put_output,
		    &OldCharBuf.BufferSize,
		    &helptext_d,
		    &helplib_d,
		    &helpflags,
		    lib$get_input);
    }
    return(sts);
}
#endif /* _USEDCL_ */
unsigned long	vms_clock_bits[2];	/* VMS Hardware Clock */
const long	vms_ticks_per_update = 100000L; /* Clock update int. */

/*
 * FDL Stuff For Getting & Setting File Characteristics
 * This code was derived (loosely!) from the module LZVIO.C in the public 
 * domain LZW compress routine as found on the DECUS VAX SIG tapes (no author
 * given, so no credits!) 
 */

/*
 * **-fdl_generate-Generate An FDL
 *
 * Description:
 *
 * This procedure takes the name of an existing file as input and creates
 * an fdl. The FDL is retuned by pointer and length. The FDL space should be
 * released after use with a call to free();
 */
int fdl_generate(char *in_file, char **fdl, short *len)
/*
 * Arguments:
 *
 *	in_file	    char*   Filename of file to examine (Zero terminated).
 *
 *	fdl	    char*   Pointer to FDL that was created.
 *
 *	len	    short   Length of FDL created.
 *
 * Status Returns:
 *
 * VMS style. lower bit set means success.
 */
{

    struct dsc$descriptor fdl_descr = { 0,
				DSC$K_DTYPE_T,
				DSC$K_CLASS_D,
				0};
    struct FAB fab, *fab_addr;
    struct RAB rab, *rab_addr;
    struct NAM nam;
    struct XABFHC xab;
    int sts;
    int badblk;

/*
 * Build FDL Descriptor
 */
    if (!(sts = str$get1_dx(&FDLSIZE,&fdl_descr)) & 01) return(0);
/*
 * Build RMS Data Structures
 */
    fab = cc$rms_fab;
    fab_addr = &fab;
    nam = cc$rms_nam;
    rab = cc$rms_rab;
    rab_addr = &rab;
    xab = cc$rms_xabfhc;
    fab.fab$l_nam = &nam;
    fab.fab$l_xab = &xab;
    fab.fab$l_fna = in_file;
    fab.fab$b_fns = strlen(in_file);
    rab.rab$l_fab = &fab;
    fab.fab$b_fac = FAB$M_GET | FAB$M_BIO; /* This open block mode only */
/*
 * Attempt to Open File
 */
    if (!((sts = sys$open(&fab)) & 01)) {
	if (verbose) {
	    fprintf(stderr,"\n(SYSTEM) Failed to $OPEN %s\n",in_file);
	    (void) lib$signal(fab.fab$l_sts,fab.fab$l_stv);
	}
	return(sts);
    }
    if (fab.fab$l_dev & DEV$M_REC) {
	fprintf(stderr,"\n(SYSTEM) Attempt to read from output only device\n");
	sts = 0;
    } else {
	rab.rab$l_rop = RAB$M_BIO;
	if (!((sts = sys$connect(&rab)) & 01)) {
	    if (verbose) {
		fprintf(stderr,"\n(SYSTEM) Failed to $CONNECT %s\n",in_file);
		(void) lib$signal(fab.fab$l_sts,fab.fab$l_stv);
	    }
	} else {
	    if (!((sts = fdl$generate(
			&flags,
			&fab_addr,
			&rab_addr,
			NULL,NULL,
			&fdl_descr,
			&badblk,
			len)) & 01)) {
		if (verbose)
		  fprintf(stderr,"\n(SYSTEM) Failed to generate FDL\n",
			  in_file);
		free(fdl);
	    } else {
		if (!(*fdl = malloc(*len))) return(0);
		memcpy(*fdl,fdl_descr.dsc$a_pointer,*len);
	    }
	    (void) str$free1_dx(&fdl_descr);
	}
        sys$close(&fab);
    }
    return(sts);	    
}

/*	  
 * **-fdl_close-Closes files created by fdl_generate
 *  
 * Description:
 *
 * This procedure is invoked to close the file and release the data structures
 * allocated by fdl$parse.
 */
void fdl_close(void* rab)
/*
 * Arguments:
 *
 *	rab	VOID *	Pointer to RAB (voided to avoid problems for caller).
 *
 * Returns:
 *
 *	None.
 */
{
    struct FAB *fab;

    fab = ((struct RAB *) rab)->rab$l_fab;
    if (fab) {  /*  Close file if not already closed */
	if (fab->fab$w_ifi) sys$close(fab);
    }
    fdl$release( NULL, &rab);	  
}

/*
 * **-fdl_create-Create A File Using the recorded FDL (hope we get it right!)
 *
 * Description:
 *
 * This procedure accepts an FDL and uses it create a file. Unfortunately
 * there is no way we can easily patch into the back of the VAX C I/O
 * subsystem.
 */
VOID * fdl_create( char *fdl, short len, char *outfile, char *preserved_name)
/*
 * Arguments:
 *
 *	fdl	char*	FDL string descriptor.
 *
 *	len	short	Returned string length.
 *
 *	outfile	char*	Output filename.
 *
 *	preserved_name char*	Name from FDL.
 *
 * Returns:
 *
 *     0 in case of error, or otherwise the RAB pointer.
 */
{
    VOID *sts;
    int sts2;
    struct FAB *fab;
    struct RAB *rab;
    struct NAM nam;
    int badblk;
    char *resnam;

    struct dsc$descriptor fdl_descr = {
			    len,
			    DSC$K_DTYPE_T,
			    DSC$K_CLASS_S,
			    fdl
			    };

    sts = NULL;
/*
 * Initialize RMS NAM Block
 */
    nam = cc$rms_nam;
    nam.nam$b_rss = NAM$C_MAXRSSLCL;
    nam.nam$b_ess = NAM$C_MAXRSSLCL;
    if (!(resnam = nam.nam$l_esa = malloc(NAM$C_MAXRSSLCL+1))) {
	fprintf(stderr,"\n(FDL_CREATE) Out of memory!\n");
	return(NULL);
    }
/*
 * Parse FDL
 */
    if (!((sts2 = fdl$parse( &fdl_descr,
				&fab,
				&rab,
				&flags)) & 01)) {
	fprintf(stderr,"\nCreating (fdl$parse)\n");
	(void) lib$signal(sts2);
    } else {
/*
 * Extract & Return Name of FDL Supplied Filename
 */
	memcpy (preserved_name,fab->fab$l_fna,fab->fab$b_fns);
	preserved_name[fab->fab$b_fns] = '\0';
/*
 * Set Name Of Temporary File
 */
	fab->fab$l_fna = outfile;
	fab->fab$b_fns = strlen(outfile);
/*
 * Connect NAM Block
 */
	fab->fab$l_nam = &nam;
	fab->fab$l_fop |= FAB$M_NAM | FAB$M_CIF;
	fab->fab$b_fac |= FAB$M_BIO | FAB$M_PUT;
/*
 * Create File
 */
	if (!(sys$create(fab) & 01)) {
	    fprintf(stderr,"\nCreating (RMS)\n");
	    (void) lib$signal(fab->fab$l_sts,fab->fab$l_stv);
	    fdl_close(rab);
	} else {
	    if (verbose) {
		resnam[nam.nam$b_esl+1] = '\0';
		fprintf(stderr,"\nCreated %s successfully\n",resnam);
	    }
	    rab->rab$l_rop = RAB$M_BIO;
	    if (!(sys$connect(rab) & 01)) {
		fprintf(stderr,"\nConnecting (RMS)\n");
		(void) lib$signal(rab->rab$l_sts,rab->rab$l_stv);
		fdl_close(rab);
	    } else sts = rab;
	}
	fab->fab$l_nam = 0; /* I allocated NAM block,
			       so I must deallocate it! */
    }
    free(resnam);
    return(sts);		
}

/*
 * **-fdl_copyfile2bin-Copies the input file to a 'binary' output file
 *
 * Description:
 *
 * This procedure is invoked to copy from an opened file f to a file opened
 * directly through RMS. This allows us to make a block copy into one of the
 * many esoteric RMS file types thus preserving characteristics without blowing
 * up the C RTL. This code is based directly on copyfile from FILEIO.C.
 *
 * Calling Sequence:
 */
int fdl_copyfile2bin( FILE *f, VOID *rab, word32 longcount)
/*
 * Arguments:
 *
 *	f	    FILE*	Pointer to input file
 *
 *	rab	    RAB*	Pointer to output file RAB
 * 
 *	longcount   word32	Size of file
 *
 * Returns:
 *
 *	0   If we were successful.
 *	-1  We had an error on the input file (VAXCRTL).
 *	+1  We had an error on the output file (direct RMS).
 */
{
    int status = 0;
    word32 count;
    ((struct RAB *) rab)->rab$l_rbf = &textbuf;
    ((struct RAB *) rab)->rab$l_bkt = 0;
    do { /*  Read and write longcount bytes */
	if (longcount < (word32) DISKBUFSIZE)
	    count = longcount;
	else
	    count = DISKBUFSIZE;
	count = fread(textbuf,1,count,f);
	if (count > 0) {
/*	  
 *  No byte order conversion required, source and target system are both
 *  VMS so have the same byte ordering.
 */	  
	    ((struct RAB *) rab)->rab$w_rsz = (unsigned short) count;
	    if (!(sys$write (
		       rab, 
		       NULL, 
		       NULL) & 01)) {
		  lib$signal(((struct RAB *) rab)->rab$l_sts,
			     ((struct RAB *) rab)->rab$l_stv);
		  status = 1;
		  break;
	    }
	    longcount -= count;
	}
    } while (count==DISKBUFSIZE);
    burn(textbuf);
    return(status);
}
/*
 * **-vms_fileparse-Parse A VMS File Specification
 *
 * Functional Description:
 *
 * This procedure is invoked to parse a VMS file specification using default 
 * and related specifications to fill in any missing components. This works a 
 * little like DCL's F$PARSE function with the syntax check only specified
 * (that is we don't check the device or the directory). The related file
 * spec is really for when we want to use the name of an input file (w/o the
 * directory) to supply the name of an output file.
 *
 * Note that we correctly handle the situation where the output buffer overlays
 * the input filespec by testing for the case and then handling it by copying
 * the primary input specification to a temporary buffer before parsing.
 */
int vms_fileparse( char *outbuf, char *filespec, char *defspec, char *relspec)
/*
 * Arguments:
 *
 *  outbuf	Returned file specification.
 *  filespec	Primary file specification (optional).
 *  defspec	Default file specification (optional).
 *  relspec	Related file specification (optional).
 *
 * Returns:
 *
 *  As for SYS$PARSE.
 *
 * Implicit Inputs:
 *
 *  None.
 *
 * Implicit Outputs:
 *
 *  None.
 *
 * Side Effects:
 *
 *  ...TBS...
 */
{
    struct FAB fab = cc$rms_fab;
    struct NAM nam = cc$rms_nam;
    struct NAM rlnam = cc$rms_nam;
    int sts = 1;
    int len;
    char tmpbuf[NAM$C_MAXRSSLCL];
    char expfnam2[NAM$C_MAXRSSLCL];

    if (outbuf != NULL) {
	outbuf[0] = '\0';
	fab.fab$l_fop != FAB$M_NAM;  /*  Enable RMS NAM block processing */
	nam.nam$b_nop |= NAM$M_PWD | NAM$M_SYNCHK;
	/*	  
	**  Handle Related Spec (If reqd).
	*/	  
	if (relspec != NULL) {
	    if ((len = strlen(relspec)) > 0) {
		fab.fab$l_nam = &rlnam;
		fab.fab$b_fns = len;
		fab.fab$l_fna = relspec;
		rlnam.nam$b_ess = NAM$C_MAXRSSLCL;
		rlnam.nam$l_esa = expfnam2;
		rlnam.nam$b_nop |= NAM$M_PWD | NAM$M_SYNCHK;
		if ((sts = sys$parse (
			    &fab, 
			    0, 
			    0)) & 01) {
		    rlnam.nam$l_rsa = rlnam.nam$l_esa;
		    rlnam.nam$b_rsl = rlnam.nam$b_esl;
		    nam.nam$l_rlf = &rlnam;
		    fab.fab$l_fop |= FAB$M_OFP;
		}
	    }
	}
	if (sts) {
	    fab.fab$l_nam = &nam;
	    nam.nam$l_esa = outbuf;
	    nam.nam$b_ess = NAM$C_MAXRSSLCL;
	    /*	  
	    **  Process Default Specification:
	    */	  
	    if (defspec != NULL) {
		if ((len = strlen(defspec)) > 0) {
		    fab.fab$l_dna = defspec;
		    fab.fab$b_dns = len;
		}
	    }
	    /*	  
	    **  Process Main File Specification:
	    */	  
	    fab.fab$l_fna = NULL;
	    fab.fab$b_fns = 0;
	    if (filespec != NULL) {
		if ((len = strlen(filespec)) > 0) {
		    fab.fab$b_fns = len;
		    if (filespec == outbuf)
			fab.fab$l_fna = memcpy(tmpbuf,filespec,len);
		    else
			fab.fab$l_fna = filespec;
		}
	    }
	    if ((sts = sys$parse(
		       &fab, 
		       0, 
		       0)) && 01) outbuf[nam.nam$b_esl] = '\0';
	}
    }
    return (sts);
}
#endif /* VMS */


/*
 * ------------------------- Amiga specific routines -------------------------
 */

#ifdef AMIGA

#include <time.h>
#include <dos/var.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/types.h>
#include <libraries/dosextens.h>
#include <libraries/reqtools.h>
#include <proto/dos.h> 
#include <proto/exec.h>
#include <proto/reqtools.h>
#include "pgp.h"

/*
 * This getenv will use the WB2.0 calls if you have the 2.0
 * rom. If not, it resorts to looking in the ENV: directory.
 */

/*
 * I am sorry to report that SAS/C is buggy. :-(
 * It doesn't recognize replacement routines if they are linked
 * to the main code and not included in the file itself. I hate
 * stuff like that. :-(
 *                                            -peter
 */

char *amiga_getenv(const char *name)
{
        FILE *fp;
        char *ptr;
        static char value[256];
        static char buf[256];

        /*
         * 2.0 style?
         */
        if (DOSBase->dl_lib.lib_Version >= 36) {
                if (GetVar((char *) name, value, 256, 0L) == -1)
                        return NULL;
        }
        else {
                if (strlen(name) > 252)
                        return NULL;
                strcpy(buf, "ENV:");
                strcpy(&buf[4], name);
                if (!(fp = fopen(buf, "r")))
                        return NULL;
                for (ptr = value; (*ptr = getc(fp)) != EOF
                     && *ptr != '\n'
                     && ++ptr < &value[256];) ;
                fclose(fp);
                *ptr = 0;
        }
        return value;
}


extern FILE *pgpout;
char *requesterdesc;

/*
 * AmigaRequestString() is a trick to make PGP more usable from scripts.
 * The problem is, that most scripts don't allow user interaction over
 * the standard input. The same problem occurs when working in filter mode.
 *
 * This routine will be called by PGP's getstring() whenever user input
 * is requested but the standard input is not interactive. Because the
 * routine can't know what string to ask for, I added the Amiga-specific
 * variable requesterdesc, which holds the last string printed to pgpout
 * before getstring was called.
 *
 * This solution is not pretty, but it works.
 *                                                      Peter Simons
 */

int AmigaRequestString(char *buffer, int maxlen, int echo)
{
        struct ReqToolsBase *ReqToolsBase;
        struct TagItem ti[] = {
                {RTGS_Invisible, FALSE},
                {RTGS_TextFmt, 0L},
                {RTGS_Flags, GSREQF_CENTERTEXT},
                {TAG_DONE, 0L}
        };
        int len = 0;
        char name[64];

        if (!maxlen)
                return 0;
        if (!echo)
                ti[0].ti_Data = TRUE;
        ti[1].ti_Data = (ULONG) (requesterdesc) ? ((*requesterdesc == '\n') ? requesterdesc+1 : requesterdesc) : "Please enter required string";
                                /* This one is tricky, too. Because of the format of the
                                 * LANG() module we have a prefacing return before most
                                 * strings, which will make our beautiful requester look
                                 * a bit stupid. This way, we get rid of it. :-)
                                 */
        sprintf(name, "PGPAmiga %s", rel_version);

        if (ReqToolsBase = (struct ReqToolsBase *) OpenLibrary(REQTOOLSNAME, 38L))
        {
                *buffer = '\0';
                if (rtGetStringA(buffer, maxlen, name, NULL, ti))
                        len = strlen(buffer);
                CloseLibrary((struct Library *) ReqToolsBase);
        }
        else
        {  fprintf(stderr,"Could not open ReqTools.library!  Try using PGP "
                   "without -f.\n");
           exitPGP(7);   /* Error exit */
        }
        requesterdesc=NULL;   /* Program will re-set it before next getstring() call */
        return len;
}

sendpacket(struct MsgPort *rec,LONG action,LONG arg1) 
{
  struct StandardPacket *pkt;
  struct MsgPort *rp;
  LONG res1 = 0L;

  if (rp = (struct MsgPort *)CreatePort(NULL,0L)) {
    if (pkt = (struct StandardPacket *)\
	 AllocMem(sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR)) {
	   pkt->sp_Msg.mn_Node.ln_Name = (BYTE *)&pkt->sp_Pkt;
	   pkt->sp_Pkt.dp_Link = &pkt->sp_Msg;
	   pkt->sp_Pkt.dp_Port = rp;
	   pkt->sp_Pkt.dp_Type = action;
	   pkt->sp_Pkt.dp_Arg1 = arg1;
	   PutMsg(rec,&pkt->sp_Msg);
	   WaitPort(rp);
	   GetMsg(rp);
	   res1 = pkt->sp_Pkt.dp_Res1;
	   FreeMem((UBYTE*)pkt,sizeof(struct StandardPacket));
	 }
	 DeletePort(rp);
	}
	return(res1);

}

void ttycbreak(void)
{
  struct MsgPort *ch;

  ch = ((struct FileHandle *)BADDR(Input()))->fh_Type;
  sendpacket(ch,ACTION_SCREEN_MODE,-1L);
}

void ttynorm(void)
{
  struct MsgPort *ch;

  ch = ((struct FileHandle *)BADDR(Input()))->fh_Type;
  sendpacket(ch,ACTION_SCREEN_MODE,0L);
}

int getch(void)
{
  char buf;

  Read(Input(),&buf,1);
  return((int)buf);
}

int kbhit(void)
{
  if(WaitForChar(Input(), 1)) return 1;
  return 0;
}

/* GetSysTime problem with WB 1.3 fixed by A. Hartley (february@genie.com) */

extern struct timerequest *TimerIO;    /* Defined in random.c */

void am_GetSysTime(struct timeval *tv)
{
   TimerIO->tr_node.io_Command=TR_GETSYSTIME;
   DoIO((struct IORequest *) TimerIO);
   *tv=TimerIO->tr_time;
}

#ifdef __SASC

/*
 * SAS/C CTRL-C handler
 */

void __regargs _CXBRK(void)
{
  struct MsgPort *ch;

  /* it might happen we catch a ^C while in cbreak mode.
   * so always set the screen to the normal mode.
  */

  ch = ((struct FileHandle *)BADDR(Input()))->fh_Type;
  sendpacket(ch, ACTION_SCREEN_MODE, 0L);


  fprintf(pgpout, "\n*** Program Aborted.\n");
  exitPGP(6); /* INTERRUPT */
}
#endif    /* __SASC */

#endif /* AMIGA */


/*===========================================================================*/
/*
 * other stuff for non-MSDOS systems
 */

#ifdef ATARI
#ifdef __PUREC__
#include <tos.h>
#else
#include <osbind.h>		/* use GEMDOS functions for I/O */
#endif

int kbhit(void)
{
	return Cconis();	/* ret == 0 : no char available */
}

int getch(void)
{
	return (Cnecin() & 0x000000FF);	/* ASCII-Code in Bits 0..7   */
}					/* Scan-Codes in Bits 16..23 */
#endif /* ATARI */

#if !defined(MSDOS) && !defined(ATARI)
#include <ctype.h>
#include "charset.h"
char *strlwr(char *s)
{	/*
	**        Turns string s into lower case.
	*/
	int c;
	char *p = s;
	while (c = *p)
		*p++ = to_lower(c);
	return(s);
}
#endif /* !MSDOS && !ATARI */


#ifdef strstr
#undef strstr
/* Not implemented on some systems - return first instance of s2 in s1 */
char *mystrstr (char *s1, char *s2)
{	int i;
	char *strchr();

	if (!s2 || !*s2)
		return s1;
	for ( ; ; )
	{	if (!(s1 = strchr (s1, *s2)))
			return s1;
		for (i=1; s2[i] && (s1[i]==s2[i]); ++i)
			;
		if (!s2[i])
			return s1;
		++s1;
	}
}
#endif /* strstr */


#ifdef fopen
#undef fopen

#ifdef ATARI
#define F_BUF_SIZE 8192  /* seems to be a good value ... */

FILE *myfopen(const char *filename, const char *mode)
/* Open streams with larger buffer to increase disk I/O speed. */
/* Adjust F_BUF_SIZE to change buffer size.                    */
{
    FILE *f;

    if ( (f = fopen(filename, mode)) != NULL )
        if (setvbuf(f, NULL, _IOFBF, F_BUF_SIZE)) /* no memory? */
        {
            fclose(f);                 /* then close it again */
            f = fopen(filename, mode); /* and try again in normal mode */
        }
    return(f);                         /* return either handle or NULL */
}
	
#else /* ATARI */

/* Remove "b" from 2nd arg */
FILE *myfopen(char *filename, char *type)
{	char buf[10];

	buf[0] = *type++;
	if (*type=='b')
		++type;
	strcpy(buf+1,type);
	return fopen(filename, buf);
}
#endif /* not ATARI */
#endif /* fopen */


#ifndef MSDOS
#ifdef OS2

static int chr = -1;

int kbhit(void)
{
	if (chr == -1)
	  	chr = _read_kbd(0, 0, 0);
	return (chr != -1);
}

int getch(void)
{
	int c;

	if (chr >= 0) {
		c = chr;
		chr = -1;
	} else
	  	c = _read_kbd(0, 1, 0);

	return c;
}

#endif /* OS2 */
#endif /* MSDOS */

#ifdef MACTC5	/* 203a */

#include "My_console.h"

int getch(void) {
	while( !kbhit() );
	return( getc(stdin) );
}

int kbhit(void) {
	int kbuf;
	
	csetmode(C_RAW, stdin);
	kbuf = getc(stdin);
	if( kbuf != EOF ) ungetc((kbuf & 0xff), stdin);
	csetmode(C_ECHO, stdin);
	return( (kbuf == EOF) ? 0 : 1 );
}

#endif

/*EWS Fix -f lockup on passphrase prompts for TURBO C++ */
#if defined(MSDOS) && !defined(__GO32__) && defined(__TURBOC__)
#include <bios.h>
#include <signal.h>

#if !defined(_KEYBRD_READY)
#define _KEYBRD_READY 1    /* To support old versions of Turbo C */
#endif
#if !defined(_KEYBRD_READ)
#define _KEYBRD_READ 0     /* To support old versions of Turbo C */
#endif

int kbhit(void)
{
  int c;
  c=bioskey(_KEYBRD_READY);
  if (c != 0) c=1;
  return c;
} /*kbhit*/

int getch(void)
{
   int c;
   c=bioskey(_KEYBRD_READ);
   if (c==11779) raise(SIGINT);   /* Ctrl-C */
   return c & 0xff;
} /*getch*/
#endif

/*EWS Fix -f lockup on passphrase prompts for MSC */
#if defined(MSDOS) && !defined(__GO32__) && defined(_MSC_VER)
#include <bios.h>
#include <signal.h>
#include <dos.h>

int getcbrk(void)
{
    union REGS r;

    r.x.ax=0x3300;
    intdos(&r, &r);
    return(r.h.dl);
}

int setcbrk(int xx)
{
    union REGS r;

    r.x.ax=0x3301;
    r.h.dl=xx;
    intdos(&r, &r);
    return(r.h.dl);
}

int kbhit(void)
{
    int c;
    c=_bios_keybrd(_KEYBRD_READY);
    if (c != 0) c=1;
    return c;
} /*kbhit*/

int getch(void)
{
    int c;
    c=_bios_keybrd(_KEYBRD_READ);
    if (c==11779) raise(SIGINT);   /* Ctrl-C */
    return c & 0xff;
} /*getch*/
#endif
 
#ifdef EBCDIC
static int kbuf = -1;

int kbhit(void)
{
   int ch;
   if (kbuf >= 0)
      return 1;
   if (ch = getchar()) {
      kbuf = ch;
      return 1;
   }
   return 0;
}

int getch(void)
{
   int ch;
   while (!kbhit());
   ch = kbuf;
   kbuf = -1;
   return ch;
}

int c370_rename(char *from, char *to)
{
   return rename(from,to) == 0 ? 0 : -1;
}
#endif /* EBCDIC */
