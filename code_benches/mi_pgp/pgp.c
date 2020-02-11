/* #define TEMP_VERSION /* if defined, temporary experimental
   		            version of PGP */
/* pgp.c -- main module for PGP.
   PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

   Synopsis:  PGP uses public-key encryption to protect E-mail. 
   Communicate securely with people you've never met, with no secure
   channels needed for prior exchange of keys.  PGP is well featured and
   fast, with sophisticated key management, digital signatures, data
   compression, and good ergonomic design.

   The original PGP version 1.0 was written by Philip Zimmermann, of
   Phil's Pretty Good(tm) Software.  Many parts of later versions of 
   PGP were developed by an international collaborative effort, 
   involving a number of contributors, including major efforts by:
   Branko Lankester <branko@hacktic.nl>
   Hal Finney <74076.1041@compuserve.com>
   Peter Gutmann <pgut1@cs.aukuni.ac.nz>
   Other contributors who ported or translated or otherwise helped include:
   Jean-loup Gailly in France
   Hugh Kennedy in Germany
   Lutz Frank in Germany
   Cor Bosman in The Netherlands
   Felipe Rodriquez Svensson in The Netherlands
   Armando Ramos in Spain
   Miguel Angel Gallardo Ortiz in Spain
   Harry Bush and Maris Gabalins in Latvia
   Zygimantas Cepaitis in Lithuania
   Alexander Smishlajev
   Peter Suchkow and Andrew Chernov in Russia
   David Vincenzetti in Italy
   ...and others.


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


   Philip Zimmermann may be reached at:
   Boulder Software Engineering
   3021 Eleventh Street
   Boulder, Colorado 80304  USA
   (303) 541-0140  (voice or FAX)
   email:  prz@acm.org


   PGP will run on MSDOS, Sun Unix, VAX/VMS, Ultrix, Atari ST, 
   Commodore Amiga, and OS/2.  Note:  Don't try to do anything with 
   this source code without looking at the PGP User's Guide.

   PGP combines the convenience of the Rivest-Shamir-Adleman (RSA)
   public key cryptosystem with the speed of fast conventional
   cryptographic algorithms, fast message digest algorithms, data
   compression, and sophisticated key management.  And PGP performs 
   the RSA functions faster than most other software implementations.  
   PGP is RSA public key cryptography for the masses.

   Uses RSA Data Security, Inc. MD5 Message Digest Algorithm
   as a hash for signatures.  Uses the ZIP algorithm for compression.
   Uses the ETH IPES/IDEA algorithm for conventional encryption.

   PGP generally zeroes its used stack and memory areas before exiting.
   This avoids leaving sensitive information in RAM where other users
   could find it later.  The RSA library and keygen routines also
   sanitize their own stack areas.  This stack sanitizing has not been
   checked out under all the error exit conditions, when routines exit
   abnormally.  Also, we must find a way to clear the C I/O library
   file buffers, the disk buffers, and cache buffers.

   Revisions:
   Version 1.0 -  5 Jun 91
   Version 1.4 - 19 Jan 92
   Version 1.5 - 12 Feb 92
   Version 1.6 - 24 Feb 92
   Version 1.7 - 29 Mar 92
   Version 1.8 - 23 May 92
   Version 2.0 -  2 Sep 92
   Version 2.1 -  6 Dec 92
   Version 2.2 -  6 Mar 93
   Version 2.3 - 13 Jun 93
   Version 2.3a-  1 Jul 93
   Version 2.4 -  6 Nov 93
   Version 2.5 -  5 May 94
   Version 2.6 - 22 May 94
   Version 2.6.1 - 29 Aug 94
   Version 2.6.2 - 11 Oct 94
   Version 2.6.2i - 7 May 95
   Version 2.6.3(i) - 18 Jan 96
   Version 2.6.3(i)a - 4 Mar 96

 */


#include <ctype.h>
#ifndef AMIGA
#include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "system.h"
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
#include "charset.h"
#include "getopt.h"
#include "config.h"
#include "keymaint.h"
#include "keyadd.h"
#include "rsaglue.h"
#include "noise.h"

#ifdef MACTC5
#include "Macutil.h"
#include "Macutil2.h"
#include "Macutil3.h"
#include "Macutil4.h"
#include "Macbinary.h"
#include "Binhex.h"
#include "MacPGP.h"
#include "mystr.h"
void AddOutputFiles(char *filepath);
void ReInitKeyMaint(void);
extern char appPathName[];
void 		ReInitGlobals(void);
extern int level,method;
extern Boolean explicit_plainfile;
extern long infile_line;
extern int eofonce;
extern boolean savedwashed;
extern char *special;
char *Outputfile = NULL;
void check_expiration_date(void);
#define BEST -1
#define exit Exit
void Exit(int x);
#endif

#ifdef  M_XENIX
char *strstr();
long time();
#endif

#ifdef MSDOS
#ifdef __ZTC__			/* Extend stack for Zortech C */
unsigned _stack_ = 24 * 1024;
#endif
#ifdef __TURBOC__
unsigned _stklen = 24 * 1024;
#endif
#endif
#define	STACK_WIPE	4096

#ifdef AMIGA
#ifdef __SASC_60
/* Let the compiler allocate us an appropriate stack. */
extern long __stack = 32768L;
#endif

 /* Add the appropriate AmigaOS version string, depending on the
  * compiler flags.
  */
#ifdef USA
static const char __DOSVer[] = "$VER: PGP 2.6.3a (04.03.96)"
#  ifdef _M68020
        " Amiga 68020 version by Rob Knop <rknop@mop.caltech.edu>";
#  else
        " Amiga 68000 version by Rob Knop <rknop@mop.caltech.edu>";
#  endif
#else
static const char __DOSVer[] = "$VER: PGP 2.6.3ia (04.03.96)"
#  ifdef _M68020
        " Amiga 68020 version by Peter Simons <simons@peti.rhein.de>";
#  else
        " Amiga 68000 version by Peter Simons <simons@peti.rhein.de>";
#  endif
#endif /* USA */
#endif /* AMIGA */

/* Global filenames and system-wide file extensions... */
#ifdef USA
char rel_version[] = _LANG("2.6.3a");	/* release version */
#else
char rel_version[] = _LANG("2.6.3ia");	/* release version */
#endif
char rel_date[] = "1996-03-04";		/* release date */
char PGP_EXTENSION[] = ".pgp";
char ASC_EXTENSION[] = ".asc";
char SIG_EXTENSION[] = ".sig";
char BAK_EXTENSION[] = ".bak";
static char HLP_EXTENSION[] = ".hlp";
char CONSOLE_FILENAME[] = "_CONSOLE";
#ifdef MACTC5
char HELP_FILENAME[256] = "pgp.hlp";
#else
static char HELP_FILENAME[] = "pgp.hlp";
#endif

/* These files use the environmental variable PGPPATH as a default path: */
char globalPubringName[MAX_PATH];
char globalSecringName[MAX_PATH];
char globalRandseedName[MAX_PATH];
char globalCommentString[128];

/* Flags which are global across the driver code files */
boolean verbose = FALSE;	/* -l option: display maximum information */
FILE *pgpout;			/* Place for routine messages */

static void usage(void);
static void key_usage(void);
static void arg_error(void);
static void initsigs(void);
static int do_keyopt(char);
static int do_decrypt(char *);
static void do_armorfile(char *);
char ** ParseRecipients(char **);
void hashpass (char *keystring, int keylen, byte *hash);

/* Various compression signatures: PKZIP, Zoo, GIF, Arj, and HPACK.
   Lha(rc) is handled specially in the code; it is missing from the
   compressSig structure intentionally.  If more formats are added,
   put them before lharc to keep the code consistent.

   27-Jun-95 simons@peti.rhein.de (Peter Simons)
   Added support for lh5 archive as generated by Lha. Unfortunately,
   lh5 requires special treatment also. I inserted the check right
   _before_ lharc, because lh5/lha is a special type of an lharc
   archive.
 */
static char *compressSig[] =
{"PK\03\04", "ZOO ", "GIF8", "\352\140", "Rar!",
 "HPAK", "\037\213", "\037\235", "\032\013", "\032HP%"
	/* lharc is special, must be last */ };
static char *compressName[] =
{"PKZIP", "Zoo", "GIF", "Arj", "RAR",
 "Hpack", "gzip", "compressed", "PAK", "Hyper",
 "Lha", "Lharc"};
static char *compressExt[] =
{".zip", ".zoo", ".gif", ".arj", ".rar",
 ".hpk", ".gz", ".Z", ".pak", ".hyp",
 ".lha", ".lzh"};

/* "\032\0??", "ARC", ".arc" */

/* Returns file signature type from a number of popular compression formats
   or -1 if no match */
int compressSignature(byte * header)
{
    int i;

    for (i = 0; i < sizeof(compressSig) / sizeof(*compressSig); i++)
	if (!strncmp((char *) header, compressSig[i], strlen(compressSig[i])))
	    return i;

    /* Special check for lha files */
    if (!strncmp((char *)header+2, "-lh5-", 5))
      return i;

    /* Special check for lharc files */
    if (header[2] == '-' && header[3] == 'l' &&
	(header[4] == 'z' || header[4] == 'h') &&
	header[6] == '-')
	return i+1;
    return -1;
}				/* compressSignature */

/* returns TRUE if file is likely to be compressible */
static boolean file_compressible(char *filename)
{
    byte header[8];
    get_header_info_from_file(filename, header, 8);
    if (compressSignature(header) >= 0)
	return FALSE;		/* probably not compressible */
    return TRUE;		/* possibly compressible */
}				/* compressible */


/* Possible error exit codes - not all of these are used.  Note that we
   don't use the ANSI EXIT_SUCCESS and EXIT_FAILURE.  To make things
   easier for compilers which don't support enum we use #defines */

#define EXIT_OK				0
#define INVALID_FILE_ERROR		1
#define FILE_NOT_FOUND_ERROR		2
#define UNKNOWN_FILE_ERROR		3
#define NO_BATCH			4
#define BAD_ARG_ERROR			5
#define INTERRUPT			6
#define OUT_OF_MEM			7

/* Keyring errors: Base value = 10 */
#define KEYGEN_ERROR			10
#define NONEXIST_KEY_ERROR		11
#define KEYRING_ADD_ERROR		12
#define KEYRING_EXTRACT_ERROR		13
#define KEYRING_EDIT_ERROR		14
#define KEYRING_VIEW_ERROR		15
#define KEYRING_REMOVE_ERROR		16
#define KEYRING_CHECK_ERROR		17
#define KEY_SIGNATURE_ERROR		18
#define KEYSIG_REMOVE_ERROR		19

/* Encode errors: Base value = 20 */
#define SIGNATURE_ERROR			20
#define RSA_ENCR_ERROR			21
#define ENCR_ERROR			22
#define COMPRESS_ERROR			23

/* Decode errors: Base value = 30 */
#define SIGNATURE_CHECK_ERROR		30
#define RSA_DECR_ERROR			31
#define DECR_ERROR			32
#define DECOMPRESS_ERROR		33


#ifdef SIGINT

/* This function is called if a BREAK signal is sent to the program.  In this
   case we zap the temporary files.
 */
void breakHandler(int sig)
{
#ifdef UNIX
    if (sig == SIGPIPE) {
	signal(SIGPIPE, SIG_IGN);
	exitPGP(INTERRUPT);
    }
    if (sig != SIGINT)
	fprintf(stderr, "\nreceived signal %d\n", sig);
    else
#endif
	fprintf(pgpout, LANG("\nStopped at user request\n"));
    exitPGP(INTERRUPT);
}
#endif

/* Clears screen and homes the cursor. */
static void clearscreen(void)
{
    fprintf(pgpout, "\n\033[0;0H\033[J\r           \r");  /* ANSI sequence. */
    fflush(pgpout);
}

/* We had to process the config file first to possibly select the 
   foreign language to translate the sign-on line that follows... */
static void signon_msg(void)
{    
    word32 tstamp;
    /* display message only once to allow calling multiple times */
    static boolean printed = FALSE;

    if (quietmode || printed)
	return;
    printed = TRUE;
    fprintf(stderr,
LANG("Pretty Good Privacy(tm) %s - Public-key encryption for the masses.\n"),
	    rel_version);
#ifdef TEMP_VERSION
    fputs(
"Internal development version only - not for general release.\n", stderr);
#endif
    fputs(LANG("(c) 1990-96 Philip Zimmermann, Phil's Pretty Good Software."),
    stderr);
    fprintf(stderr, " %s\n",LANG(rel_date));
#ifdef USA
    fputs(LANG(signon_legalese), stderr);
#endif
    fputs(
#ifdef USA
LANG("Export of this software may be restricted by the U.S. government.\n"),
#else
LANG("International version - not for use in the USA. Does not use RSAREF.\n"),
#endif
	  stderr);

    get_timestamp((byte *) & tstamp);	/* timestamp points to tstamp */
    fprintf(pgpout, LANG("Current time: %s\n"), ctdate(&tstamp));
}


#ifdef TEMP_VERSION		/* temporary experimental version of PGP */
#include <time.h>
#define CREATION_DATE 0x30FE3640ul
				/* CREATION_DATE is
				   Thu Jan 18, 1996 1200 hours UTC */
#define LIFESPAN	((unsigned long) 60L * (unsigned long) 86400L)
				/* LIFESPAN is 60 days */

/* If this is an experimental version of PGP, cut its life short */
void check_expiration_date(void)
{
    if (get_timestamp(NULL) > (CREATION_DATE + LIFESPAN)) {
	fprintf(stderr,
		"\n\007This experimental version of PGP has expired.\n");
	exit(-1);		/* error exit */
    }
}				/* check_expiration_date */
#else				/* no expiration date */
#define check_expiration_date()	/* null statement */
#endif				/* TEMP_VERSION */

/* -f means act as a unix-style filter */
/* -i means internalize extended file attribute information, only supported
 *          between like (or very compatible) operating systems. */
/* -l means show longer more descriptive diagnostic messages */
/* -m means display plaintext output on screen, like unix "more" */
/* -d means decrypt only, leaving inner signature wrapping intact */
/* -t means treat as pure text and convert to canonical text format */

/* Used by getopt function... */
#define OPTIONS "abcdefghiklmo:prstu:vwxz:ABCDEFGHIKLMO:PRSTU:VWX?"
extern int optind;
extern char *optarg;

#define INCLUDE_MARK "-@"
#define INCLUDE_MARK_LEN sizeof(INCLUDE_MARK)-1	/* skip the \0 */

boolean emit_radix_64 = FALSE;	/* set by config file */
static boolean sign_flag = FALSE;
boolean moreflag = FALSE;
boolean filter_mode = FALSE;
static boolean preserve_filename = FALSE;
static boolean decrypt_only_flag = FALSE;
static boolean de_armor_only = FALSE;
static boolean strip_sig_flag = FALSE;
boolean clear_signatures = TRUE;
boolean strip_spaces;
static boolean c_flag = FALSE;
static boolean u_flag = FALSE;		/* Did I get my_name from -u? */
boolean encrypt_to_self = FALSE; /* should I encrypt messages to myself? */
boolean sign_new_userids = TRUE;
boolean batchmode = FALSE;	/* if TRUE: don't ask questions */
boolean quietmode = FALSE;
boolean force_flag = TRUE;	/* overwrite existing file without asking */
#ifdef VMS			/* kludge for those stupid VMS variable-length
				   text records */
char literal_mode = MODE_TEXT;	/* MODE_TEXT or MODE_BINARY for literal
				   packet */
#else				/* not VMS */
char literal_mode = MODE_BINARY; /* MODE_TEXT or MODE_BINARY for literal
				    packet */
#endif				/* not VMS */
/* my_name is substring of default userid for secret key to make signatures */
char my_name[256] = "\0";	/* null my_name means take first userid
				   in ring */
boolean keepctx = FALSE;	/* TRUE means keep .ctx file on decrypt */
/* Ask for each key separately if it should be added to the keyring */
boolean interactive_add = FALSE;
boolean compress_enabled = TRUE; /* attempt compression before encryption */
long timeshift = 0L;		/* seconds from GMT timezone */
int version_byte = VERSION_BYTE_NEW;
boolean nomanual = 0;
/* If non-zero, initialize file to this many random bytes */
int makerandom = 0;


static char *outputfile = NULL;
#ifndef MACTC5
static int errorLvl = EXIT_OK;
#else
int errorLvl = EXIT_OK;
#endif
static char mcguffin[256];	/* userid search tag */
boolean signature_checked = FALSE;
int checksig_pass = 0;
boolean use_charset_header;
char charset_header[16] = "";
char plainfile[MAX_PATH];
int myArgc = 2;
char **myArgv;
struct hashedpw *passwds = 0, *keypasswds = 0;
static struct hashedpw **passwdstail = &passwds;

#ifdef MACTC5
extern unsigned long PGPStart, WNECalls;

void ReInitGlobals()
{
	int i;
    char scratch[64];
    WNECalls = 0;
    if (verbose)
    	PGPStart = TickCount();
    else
    	PGPStart = 0;
    Abort = FALSE;
	BreakCntl = 0;
	pgpout = stderr;
    optind = 1;
	errorLvl = EXIT_OK;
    myArgc = 2;
    myArgv = nil;
    emit_radix_64 = FALSE;		/* set by config file */
    sign_flag = FALSE;
    moreflag = FALSE;
    filter_mode = FALSE;
    preserve_filename = FALSE;
    decrypt_only_flag = FALSE;
    de_armor_only = FALSE;
    strip_sig_flag = FALSE;
    u_flag = FALSE;
	c_flag = FALSE;
    signature_checked = FALSE;
	literal_mode = MODE_BINARY;	/* MODE_TEXT or MODE_BINARY for literal packet */
    errorLvl = EXIT_OK;
    outputfile = Outputfile;
    method = BEST; 	/* one of BEST, DEFLATE (only), or STORE (only) */
    level = 9;		/* 0=fastest compression, 9=best compression */
    special = NULL;	/* List of special suffixes */
    infile_line = 0;
	eofonce = 0;
	savedwashed = FALSE;
	ReInitKeyMaint();
	settmpdir(nil);
	setoutdir(nil);
	makerandom = 0;
	if (xcli_opt[0]) {
    	if (argv[argc] == nil)
    		argv[argc] = malloc((size_t) 80);
		if (argv[argc] == nil) {
			BailoutAlert(LANG("Out of memory"));
			ExitToShell();
		}
		strcpy(argv[argc], xcli_opt);
		argc++;
		fprintf(pgpout, "         %s\n", xcli_opt);
	}
	for (i = 0; i <= 63; i++)
		scratch[i] = to_upper(xcli_opt[i]);
	if (strcmp(xcli_opt, "+NOMANUAL=ON")==0) nomanual = true;
	else nomanual = false;
 }

int init_pgp()
{
	int err=0;
	pgpout=stderr;
	/* Process the config file first.  Any command-line arguments will
	   override the config file settings */
	buildfilename( mcguffin, "config.txt");
	if ( processConfigFile( mcguffin ) < 0 )
		err=BAD_ARG_ERROR;
	init_charset();
	signon_msg();
	g_armor_flag=emit_radix_64;
	g_text_mode=(literal_mode == MODE_TEXT);
	g_clear_signatures=clear_signatures;
	PGPSetFinfo(globalRandseedName,'RSed','MPGP');
	set_precision(MAX_UNIT_PRECISION);
	return err;
}


void Exit(int x) {

	errorLvl = x;
	if (myArgv)
		free(myArgv);
	if (mcguffins)
		free(mcguffins);
	mac_cleanup_tmpf();
	longjmp(jmp_env,5);
}


int pgp_dispatch(int argc, char *argv[])
{
	int status, opt;
	char *inputfile = NULL;
	char **recipient = NULL;
/*	char **mcguffins;   zigf made global so we can free */
	boolean macbin_flag = FALSE;
#else

int main(int argc, char *argv[])
{
    int status, opt;
    char *inputfile = NULL;
    char **recipient = NULL;
    char **mcguffins;
#endif /* MACTC5 */
    char *workfile, *tempf;
    boolean nestflag = FALSE;
    boolean decrypt_mode = FALSE;
    boolean wipeflag = FALSE;
    boolean armor_flag = FALSE;	/* -a option */
    boolean separate_signature = FALSE;
    boolean keyflag = FALSE;
    boolean encrypt_flag = FALSE;
    boolean conventional_flag = FALSE;
    boolean attempt_compression; /* attempt compression before encryption */
    boolean output_stdout;	/* Output goes to stdout */
    char *clearfile = NULL;
    char *literal_file = NULL;
    char literal_file_name[MAX_PATH];
    char cipherfile[MAX_PATH];
    char keychar = '\0';
    char *p;
    byte ctb;
    struct hashedpw *hpw;

    /* Initial messages to stderr */
    pgpout = stderr;

#ifdef MACTC5
	ReInitGlobals();
#endif
#ifdef	DEBUG1
    verbose = TRUE;
#endif
    /* The various places one can get passwords from.
     * We accumulate them all into two lists.  One is
     * to try on keys only, and is stored in no particular
     * order, while the other is of unknown purpose so
     * far (they may be used for conventional encryption
     * or decryption as well), and are kept in a specific
     * order.  If any password in the general list is found
     * to decode a key, it is moved to the key list.
     * The general list is not grown after initialization,
     * so the tail pointer is not used after this.
     */

#ifndef MACTC5
    if ((p = getenv("PGPPASS")) != NULL) {
	hpw = xmalloc(sizeof(struct hashedpw));
	hashpass(p, strlen(p), hpw->hash);
	/* Add to linked list of key passwords */
	hpw->next = keypasswds;
	keypasswds = hpw;
    }
    /* The -z "password" option should be used instead of PGPPASS if
     * the environment can be displayed with the ps command (eg. BSD).
     * If the system does not allow overwriting of the command line
     * argument list but if it has a "hidden" environment, PGPPASS
     * should be used.
     */
    for (opt = 1; opt < argc; ++opt) {
	p = argv[opt];
	if (p[0] != '-' || p[1] != 'z')
	    continue;
	/* Accept either "-zpassword" or "-z password" */
	p += 2;
	if (!*p)
	    p = argv[++opt];
	/* p now points to password */
	if (!p)
	    break;		/* End of arg list - ignore */
	hpw = xmalloc(sizeof(struct hashedpw));
	hashpass(p, strlen(p), hpw->hash);
	/* Wipe password */
	while (*p)
	    *p++ = ' ';
	/* Add to tail of linked list of passwords */
	hpw->next = 0;
	*passwdstail = hpw;
	passwdstail = &hpw->next;
    }
    /*
     * If PGPPASSFD is set in the environment try to read the password
     * from this file descriptor.  If you set PGPPASSFD to 0 pgp will
     * use the first line read from stdin as password.
     */
    if ((p = getenv("PGPPASSFD")) != NULL) {
	int passfd;
	if (*p && (passfd = atoi(p)) >= 0) {
	    char pwbuf[256];
	    p = pwbuf;
	    while (read(passfd, p, 1) == 1 && *p != '\n')
		++p;
	    hpw = xmalloc(sizeof(struct hashedpw));
	    hashpass(pwbuf, p - pwbuf, hpw->hash);
	    memset(pwbuf, 0, p - pwbuf);
	    /* Add to tail of linked list of passwords */
	    hpw->next = 0;
	    *passwdstail = hpw;
	    passwdstail = &hpw->next;
	}
    }
    /* Process the config file.  The following override each other:
       - Hard-coded defaults
       - The system config file
       - Hard-coded defaults for security-critical things
       - The user's config file
       - Environment variables
       - Command-line options.
     */
    opt = 0;			/* Number of config files read */
#ifdef PGP_SYSTEM_DIR
#ifdef UNIX
    buildsysfilename(mcguffin, ".pgprc");
    if (access(mcguffin, 0) != 0)
#endif
    buildsysfilename(mcguffin, "config.txt");
    if (access(mcguffin, 0) == 0) {
	opt++;
	/*
	 * Note: errors here are NOT fatal, so that people
	 * can use PGP with a corrputed system file.
	 */
	processConfigFile(mcguffin);
    }
#endif

    /*
     * These must be personal; the system config file may not
     * influence them.
     */
    buildfilename(globalPubringName, "pubring.pgp");
    buildfilename(globalSecringName, "secring.pgp");
    buildfilename(globalRandseedName, "randseed.bin");
    my_name[0] = '\0';

    /* Process the config file first.  Any command-line arguments will
       override the config file settings */
#if defined(UNIX) || defined(MSDOS) || defined(OS2) || defined (WIN32)
    /* Try "pgp.ini" on MS-DOS or ".pgprc" on Unix */
#ifdef UNIX
    buildfilename(mcguffin, ".pgprc");
#else
    buildfilename(mcguffin, "pgp.ini");
#endif
    if (access(mcguffin, 0) != 0)
	buildfilename(mcguffin, "config.txt");
#else
    buildfilename(mcguffin, "config.txt");
#endif
    if (access(mcguffin, 0) == 0) {
	opt++;
	if (processConfigFile(mcguffin) < 0)
	    exit(BAD_ARG_ERROR);
    }
    if (!opt)
	fprintf(pgpout, LANG("\007No configuration file found.\n"));

    init_charset();
#endif /* MACTC5 */

#ifdef MSDOS			/* only on MSDOS systems */
    if ((p = getenv("TZ")) == NULL || *p == '\0') {
	fprintf(pgpout,LANG("\007WARNING: Environmental variable TZ is not \
defined, so GMT timestamps\n\
may be wrong.  See the PGP User's Guide to properly define TZ\n\
in AUTOEXEC.BAT file.\n"));
    }
#endif				/* MSDOS */

#ifdef VMS
#define TEMP "SYS$SCRATCH"
#else
#define TEMP "TMP"
#endif				/* VMS */
    if ((p = getenv(TEMP)) != NULL && *p != '\0')
	settmpdir(p);

    if ((myArgv = (char **) malloc((argc + 2) * sizeof(char **))) == NULL) {
	fprintf(stderr, LANG("\n\007Out of memory.\n"));
	exitPGP(7);
    }
    myArgv[0] = NULL;
    myArgv[1] = NULL;

    /* Process all the command-line option switches: */
    while (optind < argc) {
	/*
	 * Allow random order of options and arguments (like GNU getopt)
	 * NOTE: this does not work with GNU getopt, use getopt.c from
	 * the PGP distribution.
	 */
	if ((!strncmp(argv[optind], INCLUDE_MARK, INCLUDE_MARK_LEN)) ||
           ((opt = pgp_getopt(argc, argv, OPTIONS)) == EOF)) {
	    if (optind == argc)	/* -- at end */
		break;
	    myArgv[myArgc++] = argv[optind++];
	    continue;
	}
	opt = to_lower(opt);
	if (keyflag && (keychar == '\0' || (keychar == 'v' && opt == 'v'))) {
	    if (keychar == 'v')
		keychar = 'V';
	    else
		keychar = opt;
	    continue;
	}
	switch (opt) {
	case 'a':
	    armor_flag = TRUE;
	    emit_radix_64 = 1;
	    break;
	case 'b':
	    separate_signature = strip_sig_flag = TRUE;
	    break;
	case 'c':
	    encrypt_flag = conventional_flag = TRUE;
	    c_flag = TRUE;
	    break;
	case 'd':
	    decrypt_only_flag = TRUE;
	    break;
	case 'e':
	    encrypt_flag = TRUE;
	    break;
#ifdef MACTC5
	case 'f':
		if (macbin_flag == FALSE) filter_mode = TRUE;
		break;
#else
	case 'f':
	    filter_mode = TRUE;
	    break;
#endif
	case '?':
	case 'h':
	    usage();
	    break;
#ifdef VMS
	case 'i':
	    literal_mode = MODE_LOCAL;
	    break;
#else
#ifdef MACTC5
	case 'i':
		macbin_flag = TRUE;
		moreflag = FALSE;
		literal_mode = MODE_BINARY;
		filter_mode = FALSE;
		break;
#endif /* MACTC5 */
#endif				/* VMS */
	case 'k':
	    keyflag = TRUE;
	    break;
	case 'l':
	    verbose = TRUE;
	    break;
#ifdef MACTC5
	case 'm':
		if( macbin_flag == FALSE )
			moreflag = TRUE;
		break;
#else
	case 'm':
	    moreflag = TRUE;
	    break;
#endif
	case 'p':
	    preserve_filename = TRUE;
	    break;
	case 'o':
	    outputfile = optarg;
	    break;
	case 's':
	    sign_flag = TRUE;
	    break;
#ifdef MACTC5
	case 't':
		if( macbin_flag == FALSE )
			literal_mode = MODE_TEXT;
		break;
#else
	case 't':
	    literal_mode = MODE_TEXT;
	    break;
#endif
	case 'u':
	    strncpy(my_name, optarg, sizeof(my_name) - 1);
	    CONVERT_TO_CANONICAL_CHARSET(my_name);
	    u_flag = TRUE;
	    break;
	case 'w':
	    wipeflag = TRUE;
	    break;
	case 'z':
	    break;
	    /* '+' special option: does not require - */
	case '+':
	    if (processConfigLine(optarg) == 0) {
                if (!strncmp(optarg,"CH",2)) /* CHARSET */
                    init_charset();
		break;
	    }
	    fprintf(stderr, "\n");
	    /* fallthrough */
	default:
	    arg_error();
	}
    }
    myArgv[myArgc] = NULL;	/* Just to make it NULL terminated */

    if (keyflag && keychar == '\0')
	key_usage();

    signon_msg();
    check_expiration_date();	/* hobble any experimental version */

    /*
     * Write to stdout if explicitly asked to, or in filter mode and
     * no explicit file name was given.
     */
    output_stdout = outputfile ? strcmp(outputfile, "-")  == 0 : filter_mode;

#if 1
    /* At request of Peter Simons, use stderr always. Sounds reasonable. */
    /* JIS: Put this code back in... removing it broke too many things */
    if (!output_stdout)
	pgpout = stdout;
#endif


#if defined(UNIX) || defined(VMS)
    umask(077);			/* Make files default to private */
#endif

    initsigs();			/* Catch signals */
    noise();			/* Start random number generation */

    if (keyflag) {
	status = do_keyopt(keychar);
	if (status < 0)
	    user_error();
	exitPGP(status);
    }
    /* -db means break off signature certificate into separate file */
    if (decrypt_only_flag && strip_sig_flag)
	decrypt_only_flag = FALSE;

    if (decrypt_only_flag && armor_flag)
	decrypt_mode = de_armor_only = TRUE;

    if (outputfile != NULL)
	preserve_filename = FALSE;

    if (!sign_flag && !encrypt_flag && !conventional_flag && !armor_flag) {
	if (wipeflag) {		/* wipe only */
	    if (myArgc != 3)
		arg_error();	/* need one argument */
	    if (wipefile(myArgv[2]) == 0 && remove(myArgv[2]) == 0) {
		fprintf(pgpout,
			LANG("\nFile %s wiped and deleted. "), myArgv[2]);
		fprintf(pgpout, "\n");
		exitPGP(EXIT_OK);
	    } else if (file_exists(myArgv[2]))
	        fprintf(pgpout,
LANG("\n\007Error: Can't wipe out file '%s' - read only, maybe?\n"),
                        myArgv[2]);
            else {
	        fprintf(pgpout,
		        LANG("\n\007File '%s' does not exist.\n"), myArgv[2]);
	    }
	    exitPGP(UNKNOWN_FILE_ERROR);
	}
	/* decrypt if none of the -s -e -c -a -w options are specified */
	decrypt_mode = TRUE;
    }
    if (myArgc == 2) {		/* no arguments */
#ifdef UNIX
	if (!filter_mode && !isatty(fileno(stdin))) {
	    /* piping to pgp without arguments and no -f:
	     * switch to filter mode but don't write output to stdout
	     * if it's a tty, use the preserved filename */
	    if (!moreflag)
		pgpout = stderr;
	    filter_mode = TRUE;
	    if (isatty(fileno(stdout)) && !moreflag)
		preserve_filename = TRUE;
	}
#endif
	if (!filter_mode) {
	    if (quietmode) {
		quietmode = FALSE;
		signon_msg();
	    }
	    fprintf(pgpout,
LANG("\nFor details on licensing and distribution, see the PGP User's Guide.\
\nFor other cryptography products and custom development services, contact:\
\nPhilip Zimmermann, 3021 11th St, Boulder CO 80304 USA, \
phone +1 303 541-0140\n"));
	    if (strcmp((p = LANG("@translator@")), "@translator@"))
		fprintf(pgpout, p);
	    fprintf(pgpout, LANG("\nFor a usage summary, type:  pgp -h\n"));
#ifdef MACTC5
		exitPGP(BAD_ARG_ERROR);
#else
	    exit(BAD_ARG_ERROR);	/* error exit */
#endif
	}
    } else {
	if (filter_mode) {
	    recipient = &myArgv[2];
	} else {
	    inputfile = myArgv[2];
	    recipient = &myArgv[3];
	}
	recipient = ParseRecipients(recipient);
    }


    if (filter_mode) {
	inputfile = "stdin";
    } else if (makerandom > 0) {	/* Create the input file */
	/*
	 * +makerandom=<bytes>: Create an input file consisting of <bytes>
	 * cryptographically strong random bytes, before applying the
	 * encryption options of PGP.  This is an advanced option, so
	 * assume the user knows what he's doing and don't bother about
	 * overwriting questions.  E.g.
	 * pgp +makerandom=24 foofile
	 *	Create "foofile" with 24 random bytes in it.
	 * pgp +makerandom=24 -ea foofile recipient
	 *	The same, but also encrypt it to "recipient", creating
	 *	foofile.asc as well.
	 * This feature was created to allow PGP to create and send keys
	 * around for other applications to use.
	 */
	status = cryptRandWriteFile(inputfile, (struct IdeaCfbContext *)0,
	                       (unsigned)makerandom);
	if (status < 0) {
		fprintf(stderr,"Error writing file \"%s\"\n",inputfile);
		exitPGP(INVALID_FILE_ERROR);
	}
	fprintf(pgpout, LANG("File %s created containing %d random bytes.\n"),
		inputfile, makerandom);
	/* If we aren't encrypting, don't bother trying to decrypt this! */
	if (decrypt_mode)
		exitPGP(EXIT_OK);

	/* This is obviously NOT a text file */
	literal_mode = MODE_BINARY;
    } else {
	if (decrypt_mode && no_extension(inputfile)) {
	    strcpy(cipherfile, inputfile);
	    force_extension(cipherfile, ASC_EXTENSION);
	    if (file_exists(cipherfile)) {
		inputfile = cipherfile;
	    } else {
		force_extension(cipherfile, PGP_EXTENSION);
		if (file_exists(cipherfile)) {
		    inputfile = cipherfile;
		} else {
		    force_extension(cipherfile, SIG_EXTENSION);
		    if (file_exists(cipherfile))
			inputfile = cipherfile;
		}
	    }
	}
	if (!file_exists(inputfile)) {
	    fprintf(pgpout,
		    LANG("\n\007File '%s' does not exist.\n"), inputfile);
	    errorLvl = FILE_NOT_FOUND_ERROR;
	    user_error();
	}
    }

    if (strlen(inputfile) >= (unsigned) MAX_PATH - 4) {
	fprintf(pgpout, 
		LANG("\007Invalid filename: '%s' too long\n"), inputfile);
	errorLvl = INVALID_FILE_ERROR;
	user_error();
    }
    strcpy(plainfile, inputfile);

    if (filter_mode) {
	setoutdir(NULL);	/* NULL means use tmpdir */
    } else {
	if (outputfile)
	    setoutdir(outputfile);
	else
	    setoutdir(inputfile);
    }

    if (filter_mode) {
	workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	readPhantomInput(workfile);
    } else {
	workfile = inputfile;
    }

    get_header_info_from_file(workfile, &ctb, 1);
    if (decrypt_mode) {
	strip_spaces = FALSE;
	if (!is_ctb(ctb) && is_armor_file(workfile, 0L))
	    do_armorfile(workfile);
	else if (do_decrypt(workfile) < 0)
	    user_error();
#ifdef MACTC5
	if (verbose) fprintf(stderr, "Final file = %s.\n", plainfile);
	/* Allow for overide of auto-unmacbin : 205b */
	if( (macbin_flag == FALSE) && is_macbin(plainfile) ) 
		bin2mac(plainfile,TRUE);
	else {
		AddOutputFiles(plainfile);
		PGPSetFinfo(plainfile,FType,FCreator);
	}
	if (use_clipboard) File2Scrap(plainfile);
#endif
	if (batchmode && !signature_checked)
	    exitPGP(1);		/* alternate success, file did not have sig. */
	else
	    exitPGP(EXIT_OK);
    }
    /*
     * See if plaintext input file was actually created by PGP earlier--
     * If it was, maybe we should NOT encapsulate it in a literal packet.
     * (nestflag = TRUE).  Otherwise, always encapsulate it (default).
     * (Why test for filter_mode???)
     */
    if (!batchmode && !filter_mode && legal_ctb(ctb)) {
	/*      Special case--may be a PGP-created packet, so
	   do we inhibit encapsulation in literal packet? */
	fprintf(pgpout,
LANG("\n\007Input file '%s' looks like it may have been created by PGP. "),
		inputfile);
	fprintf(pgpout,
LANG("\nIs it safe to assume that it was created by PGP (y/N)? "));
	nestflag = getyesno('n');
    } else if (force_flag && makerandom == 0 && legal_ctb(ctb)) {
	nestflag = TRUE;
    }

    if (moreflag && makerandom == 0) {
	/* special name to cause printout on decrypt */
	strcpy(literal_file_name, CONSOLE_FILENAME);
	literal_mode = MODE_TEXT;	/* will check for text file later */
    } else {
	strcpy(literal_file_name, file_tail(inputfile));
#ifdef MSDOS
	strlwr(literal_file_name);
#endif
    }
    literal_file = literal_file_name;

    /*      Make sure non-text files are not accidentally converted 
       to canonical text.  This precaution should only be followed 
       for US ASCII text files, since European text files may have 
       8-bit character codes and still be legitimate text files 
       suitable for conversion to canonical (CR/LF-terminated) 
       text format. */
    if (literal_mode == MODE_TEXT && !is_text_file(workfile)) {
	fprintf(pgpout,
LANG("\nNote: '%s' is not a pure text file.\n\
File will be treated as binary data.\n"),
		workfile);
	literal_mode = MODE_BINARY;	/* now expect straight binary */
    }
    if (moreflag && literal_mode == MODE_BINARY) {
	/* For eyes only?  Can't display binary file. */
	fprintf(pgpout,
LANG("\n\007Error: Only text files may be sent as display-only.\n"));
	errorLvl = INVALID_FILE_ERROR;
	user_error();
    }

    /*  
     * See if plainfile looks like it might be incompressible, 
     * by examining its contents for compression headers for 
     * commonly-used compressed file formats like PKZIP, etc.
     * Remember this information for later, when we are deciding
     * whether to attempt compression before encryption.
     *
     * Naturally, don't bother if we are making a separate signature or
     * clear-signed message.  Also, don't bother trying to compress a
     * PGP message, as it's probably already compressed.
     */
    attempt_compression = compress_enabled && !separate_signature &&
                          !nestflag && !clearfile && makerandom == 0 &&
                          file_compressible(plainfile);

#ifdef MACTC5
	if(( macbin_flag == TRUE ) && (nestflag==FALSE)) {
		char *saveworkfile;
		nestflag = false;
		saveworkfile = workfile;
		workfile = tempfile(TMP_WIPE|TMP_TMPDIR);
		if (mac2bin(saveworkfile, workfile)!=0) {
			fprintf(pgpout, LANG("\n\007Error: MacBinary failed!\n"));
			errorLvl = INVALID_FILE_ERROR;
			rmtemp(workfile);
			exitPGP(errorLvl);
		}
	}
#endif
    if (sign_flag) {
	if (!filter_mode && !quietmode)
	    fprintf(pgpout,
LANG("\nA secret key is required to make a signature. "));
	if (!quietmode && my_name[0] == '\0') {
	    fprintf(pgpout,
LANG("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
	}
	strip_spaces = FALSE;
	clearfile = NULL;
	if (literal_mode == MODE_TEXT) {
	    /* Text mode requires becoming canonical */
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	    /* +clear means output file with signature in the clear,
	       only in combination with -t and -a, not with -e or -b */
	    if (!encrypt_flag && !separate_signature &&
		emit_radix_64 && clear_signatures) {
		clearfile = workfile;
		strip_spaces = TRUE;
	    }
	    make_canonical(workfile, tempf);
	    if (!clearfile)
		rmtemp(workfile);
	    workfile = tempf;
	}
	if (attempt_compression || encrypt_flag || emit_radix_64 ||
	    output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	/* for clear signatures we create a separate signature */
	status = signfile(nestflag, separate_signature || (clearfile != NULL),
		   my_name, workfile, tempf, literal_mode, literal_file);
	rmtemp(workfile);
	workfile = tempf;

	if (status < 0) {	/* signfile failed */
	    fprintf(pgpout, LANG("\007Signature error\n"));
	    errorLvl = SIGNATURE_ERROR;
	    user_error();
	}
    } else if (!nestflag) {	/* !sign_file */
	/*      Prepend CTB_LITERAL byte to plaintext file.
	   --sure wish this pass could be optimized away. */
	if (attempt_compression || encrypt_flag || emit_radix_64 ||
	    output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	/* for clear signatures we create a separate signature */
	status = make_literal(workfile, tempf, literal_mode, literal_file);
	rmtemp(workfile);
	workfile = tempf;
    }

    if (encrypt_flag) {
        if (emit_radix_64 || output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	if (!conventional_flag) {
	    if (!filter_mode && !quietmode)
		fprintf(pgpout,
LANG("\n\nRecipients' public key(s) will be used to encrypt. "));
	    if (recipient == NULL || *recipient == NULL ||
		**recipient == '\0') {
		/* no recipient specified on command line */
		fprintf(pgpout,
LANG("\nA user ID is required to select the recipient's public key. "));
		fprintf(pgpout, LANG("\nEnter the recipient's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the recipient's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
		if ((mcguffins = (char **) malloc(2 * sizeof(char *))) == NULL)
		{
		    fprintf(stderr, LANG("\n\007Out of memory.\n"));
		    exitPGP(7);
		}
		mcguffins[0] = mcguffin;
		mcguffins[1] = "";
	    } else {
		/* recipient specified on command line */
		mcguffins = recipient;
	    }
	    for (recipient = mcguffins; *recipient != NULL &&
		 **recipient != '\0'; recipient++) {
		CONVERT_TO_CANONICAL_CHARSET(*recipient);
	    }
	    status = encryptfile(mcguffins, workfile, tempf,
				 attempt_compression);
	} else {
	    status = idea_encryptfile(workfile, tempf, attempt_compression);
	}

	rmtemp(workfile);
	workfile = tempf;

	if (status < 0) {
	    fprintf(pgpout, LANG("\007Encryption error\n"));
	    errorLvl = (conventional_flag ? ENCR_ERROR : RSA_ENCR_ERROR);
	    user_error();
	} 
    } else if (attempt_compression && !separate_signature && !clearfile) {
	/*
	 * PGP used to be parsimonious about compression; originally, it only
	 * did it for files that were being encrypted (to reduce the
	 * redundancy in the plaintext), but it should really do it for
	 * anything where it's not a bad idea.
	 */
        if (emit_radix_64 || output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	squish_file(workfile, tempf);
	rmtemp(workfile);
	workfile = tempf;
    }

    /*
     * Write to stdout if explicitly asked to, or in filter mode and
     * no explicit file name was given.
     */
    if (output_stdout) {
	if (emit_radix_64) {
	    /* NULL for outputfile means write to stdout */
	    if (armor_file(workfile, NULL, inputfile, clearfile, FALSE) != 0) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (clearfile)
		rmtemp(clearfile);
	} else {
	    if (writePhantomOutput(workfile) < 0) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	}
	rmtemp(workfile);
    } else {
	char name[MAX_PATH];
        char *t;
	if (outputfile) {
	    strcpy(name, outputfile);
	} else {
	    strcpy(name, inputfile);
	    drop_extension(name);
	}
        do {
	    if (!outputfile && no_extension(name)) {
	        if (emit_radix_64)
		    force_extension(name, ASC_EXTENSION);
	        else if (sign_flag && separate_signature)
		    force_extension(name, SIG_EXTENSION);
	        else
		    force_extension(name, PGP_EXTENSION);
#ifdef MACTC5
			if (addresfork) {
				drop_extension(name);
				force_extension(name, ".sdf");
			}
#endif
	    }
            if (!file_exists(name)) break;
            t=ck_dup_output(name, TRUE, !clearfile);
            if (t==NULL) user_error();
            if (clearfile && !strcmp(t,name)) break;
            strcpy(name,t);
        } while (TRUE);
	if (emit_radix_64) {
	    if (armor_file(workfile, name, inputfile, clearfile, FALSE) != 0) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (clearfile)
		rmtemp(clearfile);
	} else {
	    if ((outputfile = savetemp(workfile, name)) == NULL) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (!quietmode) {
		if (encrypt_flag)
		    fprintf(pgpout,
			    LANG("\nCiphertext file: %s\n"), outputfile);
		else if (sign_flag)
		    fprintf(pgpout,
			    LANG("\nSignature file: %s\n"), outputfile);
	    }
	}
#ifdef MACTC5
		AddOutputFiles(name);
		if (addresfork) {
			if(!AddResourceFork(name)) {
				short frefnum,len,i;
				char *p,*q;
				Handle h;
				c2pstr(name);
				q=file_tail(argv[2]);
				len=strlen(q);
				frefnum=OpenResFile((uchar *)name);
				h=NewHandle(len+1);
				HLock(h);
				p=*h;
				*p++=len;
				for (i=0; i<len; i++) *p++=*q++;
				AddResource(h,'STR ',500,(uchar *)"");
				ChangedResource(h);
				WriteResource(h);
				UpdateResFile(frefnum);
				CloseResFile(frefnum);
				p2cstr((uchar *)name);
			} else {
				BailoutAlert("AddResFork failed!");
				exitPGP(-1);
			}
		} 	
		if (binhex_flag) {
			if (binhex(name)) {
				BailoutAlert("BinHex failed!");
				exitPGP(-1);
			}
			remove(name);
		}
		if (use_clipboard) File2Scrap(name);
#endif /* MACTC5 */
    }

    if (wipeflag) {
	/* destroy every trace of plaintext */
	if (wipefile(inputfile) == 0) {
	    remove(inputfile);
	    fprintf(pgpout, LANG("\nFile %s wiped and deleted. "), inputfile);
	    fprintf(pgpout, "\n");
	}
    }

#ifdef MACTC5
	if(!addresfork && !use_clipboard)
		if (!emit_radix_64) PGPSetFinfo(outputfile,'Cryp','MPGP');
#endif

    exitPGP(EXIT_OK);
    return 0;			/* to shut up lint and some compilers */
#ifdef MACTC5
}				/* pgp_dispatch */
#else
}				/* main */
#endif

#ifdef MSDOS
#include <dos.h>
static char *dos_errlst[] =
{
    "Write protect error",	/* LANG ("Write protect error") */
    "Unknown unit",
    "Drive not ready",		/* LANG ("Drive not ready") */
    "3", "4", "5", "6", "7", "8", "9",
    "Write error",		/* LANG ("Write error") */
    "Read error",		/* LANG ("Read error") */
    "General failure",
};

/* handler for msdos 'harderrors' */
#ifndef OS2
#ifdef __TURBOC__		/* Turbo C 2.0 */
static int dostrap(int errval)
#else
static void dostrap(unsigned deverr, unsigned errval)
#endif
{
    char errbuf[64];
    int i;
    sprintf(errbuf, "\r\nDOS error: %s\r\n", dos_errlst[errval]);
    i = 0;
    do
	bdos(2, (unsigned int) errbuf[i], 0);
    while (errbuf[++i]);
#ifdef __TURBOC__
    return 0;			/* ignore (fopen will return NULL) */
#else
    return;
#endif
}
#endif				/* MSDOS */
#endif

static void initsigs()
{
#ifdef MSDOS
#ifndef OS2
#ifdef __TURBOC__
    harderr(dostrap);
#else				/* MSC */
#ifndef __GNUC__		/* DJGPP's not MSC */
    _harderr(dostrap);
#endif
#endif
#endif
#endif				/* MSDOS */
#ifdef SIGINT
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, breakHandler);
#if defined(UNIX) || defined(VMS) || defined(ATARI)
#ifndef __PUREC__ /* PureC doesn't recognise all signals */
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	signal(SIGHUP, breakHandler);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, breakHandler);
#endif
#ifdef UNIX
    signal(SIGPIPE, breakHandler);
#endif
    signal(SIGTERM, breakHandler);
#ifdef MACTC5
	signal(SIGABRT,breakHandler);
	signal(SIGTERM,breakHandler);
#ifndef DEBUG
    signal(SIGTRAP, breakHandler);
    signal(SIGSEGV, breakHandler);
    signal(SIGILL, breakHandler);
#ifdef SIGBUS
    signal(SIGBUS, breakHandler);
#endif
#endif				/* DEBUG */
#endif				/* MACTC5 */
#endif				/* UNIX */
#endif				/* SIGINT */
}				/* initsigs */


static void do_armorfile(char *armorfile)
{
    char *tempf;
    char cipherfile[MAX_PATH];
    long lastpos, linepos = 0;
    int status;
    int success = 0;

    for (;;) {
	/* Handle transport armor stripping */
	tempf = tempfile(0);
	strip_spaces = FALSE;	/* de_armor_file() sets
				   this for clear signature files */
        use_charset_header = TRUE;
        lastpos = linepos;
	status = de_armor_file(armorfile, tempf, &linepos);
	if (status) {
	    fprintf(pgpout,
LANG("\n\007Error: Transport armor stripping failed for file %s\n"),
		    armorfile);
	    errorLvl = INVALID_FILE_ERROR;
	    user_error();	/* Bad file */
	}
	if (keepctx || de_armor_only) {
	    if (outputfile && de_armor_only) {
		if (strcmp(outputfile, "-") == 0) {
		    writePhantomOutput(tempf);
		    rmtemp(tempf);
		    return;
		}
		strcpy(cipherfile, outputfile);
	    } else {
		strcpy(cipherfile, file_tail(armorfile));
		force_extension(cipherfile, PGP_EXTENSION);
	    }
	    if ((tempf = savetemp(tempf, cipherfile)) == NULL) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (!quietmode)
		fprintf(pgpout,
LANG("Stripped transport armor from '%s', producing '%s'.\n"),
			armorfile, tempf);
	    /* -da flag: don't decrypt */
	    if (de_armor_only || do_decrypt(tempf) >= 0)
		++success;
	} else {
	    if (charset_header[0])        /* Check signature with charset from Charset: header */
		checksig_pass = 1;
	    if (do_decrypt(tempf) >= 0)
		++success;
	    rmtemp(tempf);
	    if (charset_header[0]) {
		if (checksig_pass == 2) { /* Sigcheck failed: try again with local charset */
		    tempf = tempfile(0);
		    use_charset_header = FALSE;
		    linepos = lastpos;
		    de_armor_file(armorfile, tempf, &linepos);
		    if (do_decrypt(tempf) >= 0)
		        ++success;
		    rmtemp(tempf);
		}
		checksig_pass = 0;
	    }
	}

	if (!is_armor_file(armorfile, linepos)) {
	    if (!success)	/* print error msg if we didn't
				   decrypt anything */
		user_error();
	    return;
	}
	fprintf(pgpout,
		LANG("\nLooking for next packet in '%s'...\n"), armorfile);
    }
}				/* do_armorfile */


static int do_decrypt(char *cipherfile)
{
    char *outfile = NULL;
    int status, i;
    boolean nested_info = FALSE;
    char ringfile[MAX_PATH];
    byte ctb;
    byte header[8];		/* used to classify file type at the end. */
    char preserved_name[MAX_PATH];
    char *newname;

    /* will be set to the original file name after processing a
       literal packet */
    preserved_name[0] = '\0';

    do {			/* while nested parsable info present */
	if (nested_info) {
	    rmtemp(cipherfile);	/* never executed on first pass */
	    cipherfile = outfile;
	}
	if (get_header_info_from_file(cipherfile, &ctb, 1) < 0) {
	    fprintf(pgpout,
LANG("\n\007Can't open ciphertext file '%s'\n"), cipherfile);
	    errorLvl = FILE_NOT_FOUND_ERROR;
	    return -1;
	}
	if (!is_ctb(ctb))	/* not a real CTB -- complain */
	    break;

	if (moreflag)
	    outfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    outfile = tempfile(TMP_WIPE);

	/* PKE is Public Key Encryption */
	if (is_ctb_type(ctb, CTB_PKE_TYPE)) {

	    if (!quietmode)
		fprintf(pgpout,
LANG("\nFile is encrypted.  Secret key is required to read it. "));

	    /* Decrypt to scratch file since we may have a LITERAL2 */
	    status = decryptfile(cipherfile, outfile);

	    if (status < 0) {	/* error return */
		errorLvl = RSA_DECR_ERROR;
		return -1;
	    }
	    nested_info = (status > 0);

	} else if (is_ctb_type(ctb, CTB_SKE_TYPE)) {

	    if (decrypt_only_flag) {
		/* swap file names instead of just copying the file */
		rmtemp(outfile);
		outfile = cipherfile;
		cipherfile = NULL;
		if (!quietmode)
		    fprintf(pgpout,
LANG("\nThis file has a signature, which will be left in place.\n"));
		nested_info = FALSE;
		break;		/* Do no more */
	    }
	    if (!quietmode && checksig_pass<=1)
		fprintf(pgpout,
LANG("\nFile has signature.  Public key is required to check signature.\n"));

	    status = check_signaturefile(cipherfile, outfile,
					 strip_sig_flag, preserved_name);

	    if (status < 0) {	/* error return */
		errorLvl = SIGNATURE_CHECK_ERROR;
		return -1;
	    }
	    nested_info = (status > 0);

	    if (strcmp(preserved_name, "/dev/null") == 0) {
		rmtemp(outfile);
		fprintf(pgpout, "\n");
		return 0;
	    }
	} else if (is_ctb_type(ctb, CTB_CKE_TYPE)) {

	    /* Conventional Key Encrypted ciphertext. */
	    /* Tell user it's encrypted here, and prompt for
	       password in subroutine. */
	    if (!quietmode)
		fprintf(pgpout, LANG("\nFile is conventionally encrypted.  "));
	    /* Decrypt to scratch file since it may be a LITERAL2 */
	    status = idea_decryptfile(cipherfile, outfile);
	    if (status < 0) {	/* error return */
		errorLvl = DECR_ERROR;
		return -1;	/* error exit status */
	    }
	    nested_info = (status > 0);

	} else if (is_ctb_type(ctb, CTB_COMPRESSED_TYPE)) {

	    /* Compressed text. */
	    status = decompress_file(cipherfile, outfile);
	    if (status < 0) {	/* error return */
		errorLvl = DECOMPRESS_ERROR;
		return -1;
	    }
	    /* Always assume nested information... */
	    nested_info = TRUE;

	} else if (is_ctb_type(ctb, CTB_LITERAL_TYPE) ||
		   is_ctb_type(ctb, CTB_LITERAL2_TYPE)) { /* Raw plaintext.
							     Just copy it.
							     No more nesting.
							   */

	    /* Strip off CTB_LITERAL prefix byte from file: */
	    /* strip_literal may alter plainfile; will set mode */
	    status = strip_literal(cipherfile, outfile,
				   preserved_name, &literal_mode);
	    if (status < 0) {	/* error return */
		errorLvl = UNKNOWN_FILE_ERROR;
		return -1;
	    }
	    nested_info = FALSE;
	} else if (ctb == CTB_CERT_SECKEY || ctb == CTB_CERT_PUBKEY) {

	    rmtemp(outfile);
	    if (decrypt_only_flag) {
		/* swap file names instead of just copying the file */
		outfile = cipherfile;
		cipherfile = NULL;
		nested_info = FALSE;	/* No error */
		break;		/* no further processing */
	    }
	    /* Key ring.  View it. */
	    fprintf(pgpout,
LANG("\nFile contains key(s).  Contents follow..."));
	    if (view_keyring(NULL, cipherfile, TRUE, FALSE) < 0) {
		errorLvl = KEYRING_VIEW_ERROR;
		return -1;
	    }
	    /* filter mode explicit requested with -f */
	    if (filter_mode && !preserve_filename)
		return 0;	/*    No output file */
	    if (batchmode)
		return 0;
	    if (ctb == CTB_CERT_SECKEY)
		strcpy(ringfile, globalSecringName);
	    else
		strcpy(ringfile, globalPubringName);
	    /*      Ask if it should be put on key ring */
	    fprintf(pgpout,
LANG("\nDo you want to add this keyfile to keyring '%s' (y/N)? "), ringfile);
	    if (!getyesno('n'))
		return 0;
	    status = addto_keyring(cipherfile, ringfile);
	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keyring add error. "));
		errorLvl = KEYRING_ADD_ERROR;
		return -1;
	    }
	    return 0;		/*    No output file */

	} else {		/* Unrecognized CTB */
	    break;
	}

    } while (nested_info);
    /* No more nested parsable information */

    /* Stopped early due to error */
    if (nested_info) {
	fprintf(pgpout,
"\7\nERROR: Nested data has unexpected format.  CTB=0x%02X\n", ctb);
	if (outfile)
	    rmtemp(outfile);
	if (cipherfile)
	    rmtemp(cipherfile);
	errorLvl = UNKNOWN_FILE_ERROR;
	return -1;
    }
    if (outfile == NULL) {	/* file was not encrypted */
	if (!filter_mode && !moreflag) {
	    fprintf(pgpout,
LANG("\007\nError: '%s' is not a ciphertext, signature, or key file.\n"),
		    cipherfile);
	    errorLvl = UNKNOWN_FILE_ERROR;
	    return -1;
	}
	outfile = cipherfile;
    } else {
	if (cipherfile)
	    rmtemp(cipherfile);
    }

    if (moreflag || (strcmp(preserved_name, CONSOLE_FILENAME) == 0)) {
	/* blort to screen */
	if (strcmp(preserved_name, CONSOLE_FILENAME) == 0) {
	    fprintf(pgpout,
LANG("\n\nThis message is marked \"For your eyes only\".  Display now \
(Y/n)? "));
	    if (batchmode
#ifdef UNIX
            || !isatty(fileno(stdout))	/* stdout is redirected! */
#endif
            || filter_mode || !getyesno('y')) {
		/* no -- abort display, and clean up */
                fprintf(pgpout, "\n");
		rmtemp(outfile);
		return 0;
	    }
	}
	if (!quietmode)
	    fprintf(pgpout, LANG("\n\nPlaintext message follows...\n"));
	else
	    putc('\n', pgpout);
	fprintf(pgpout, "------------------------------\n");
	more_file(outfile, strcmp(preserved_name, CONSOLE_FILENAME) == 0);
	/* Disallow saving to disk if outfile is console-only: */
	if (strcmp(preserved_name, CONSOLE_FILENAME) == 0) {
	    clearscreen();	/* remove all evidence */
	} else if (!quietmode && !batchmode) {
	    fprintf(pgpout, LANG("Save this file permanently (y/N)? "));
	    if (getyesno('n')) {
		char moreFilename[256];
		fprintf(pgpout, LANG("Enter filename to save file as: "));
#ifdef AMIGA
                requesterdesc=LANG("Enter filename to save file as: ");
#endif
                if (preserved_name[0]) {
		    fprintf(pgpout, "[%s]: ", file_tail(preserved_name));
#ifdef AMIGA
                    strcat(requesterdesc, "[");
                    strcat(requesterdesc, file_tail(preserved_name));
                    strcat(requesterdesc, "]:");
#endif
                }
#ifdef MACTC5
		if(!GetFilePath(LANG("Enter filename to save file as:"),moreFilename,PUTFILE))
			strcpy(moreFilename,"");
		else
			fprintf(pgpout, "%s\n",moreFilename);
#else
		getstring(moreFilename, 255, TRUE);
#endif
		if (*moreFilename == '\0') {
		    if (*preserved_name != '\0')
			savetemp(outfile, file_tail(preserved_name));
		    else
			rmtemp(outfile);
		} else
		    savetemp(outfile, moreFilename);
		return 0;
	    }
	}
	rmtemp(outfile);
	return 0;
    }				/* blort to screen */
    if (outputfile) {
	if (!strcmp(outputfile, "/dev/null")) {
	    rmtemp(outfile);
	    return 0;
	}
	filter_mode = (strcmp(outputfile, "-") == 0);
	strcpy(plainfile, outputfile);
    } else {
#ifdef VMS
	/* VMS null extension has to be ".", not "" */
	force_extension(plainfile, ".");
#else				/* not VMS */
	drop_extension(plainfile);
#endif				/* not VMS */
    }

    if (!preserve_filename && filter_mode) {
	if (writePhantomOutput(outfile) < 0) {
	    errorLvl = UNKNOWN_FILE_ERROR;
	    return -1;
	}
	rmtemp(outfile);
	return 0;
    }
    if (preserve_filename && preserved_name[0] != '\0')
	strcpy(plainfile, file_tail(preserved_name));

    if (quietmode) {
	if (savetemp(outfile, plainfile) == NULL) {
	    errorLvl = UNKNOWN_FILE_ERROR;
	    return -1;
	}
	return 0;
    }
    if (!verbose)	       /* if other filename messages were suppressed */
	fprintf(pgpout, LANG("\nPlaintext filename: %s"), plainfile);


/*---------------------------------------------------------*/

    /*      One last thing-- let's attempt to classify some of the more
       frequently occurring cases of plaintext output files, as an
       aid to the user.

       For example, if output file is a public key, it should have
       the right extension on the filename.

       Also, it will likely be common to encrypt files created by
       various archivers, so they should be renamed with the archiver
       extension.
     */
    get_header_info_from_file(outfile, header, 8);

    newname = NULL;
#ifdef MACTC5
	if (header[0] == CTB_CERT_SECKEY)
		PGPSetFinfo(plainfile,'SKey','MPGP');
#endif
    if (header[0] == CTB_CERT_PUBKEY) {
	/* Special case--may be public key, worth renaming */
#ifdef MACTC5
		PGPSetFinfo(plainfile,'PKey','MPGP');
#endif
	fprintf(pgpout,
LANG("\nPlaintext file '%s' looks like it contains a public key."),
		plainfile);
	newname = maybe_force_extension(plainfile, PGP_EXTENSION);
    }
    /* Possible public key output file */ 
    else if ((i = compressSignature(header)) >= 0) {
	/*      Special case--may be an archived/compressed file,
		worth renaming
	*/
	fprintf(pgpout, LANG("\nPlaintext file '%s' looks like a %s file."),
		plainfile, compressName[i]);
	newname = maybe_force_extension(plainfile, compressExt[i]);
    } else if (is_ctb(header[0]) &&
	       (is_ctb_type(header[0], CTB_PKE_TYPE)
		|| is_ctb_type(header[0], CTB_SKE_TYPE)
		|| is_ctb_type(header[0], CTB_CKE_TYPE))) {
	/* Special case--may be another ciphertext file, worth renaming */
	fprintf(pgpout,
LANG("\n\007Output file '%s' may contain more ciphertext or signature."),
		plainfile);
	newname = maybe_force_extension(plainfile, PGP_EXTENSION);
    }				/* Possible ciphertext output file */
#ifdef MACTC5
	if( (newname = savetemp(outfile, (newname ? newname : plainfile))) == NULL) {
#else
    if (savetemp(outfile, (newname ? newname : plainfile)) == NULL) {
#endif
	errorLvl = UNKNOWN_FILE_ERROR;
	return -1;
    }
#ifdef MACTC5
	else if( strcmp(newname, plainfile) != 0 )	/* 203a */
		strcpy(plainfile, newname);
#endif
    fprintf(pgpout, "\n");
    return 0;
}				/* do_decrypt */

static int do_keyopt(char keychar)
{
    char keyfile[MAX_PATH];
    char ringfile[MAX_PATH];
    char *workfile;
    int status;

    if ((filter_mode || batchmode)
	&& (keychar == 'g' || keychar == 'e' || keychar == 'd'
	    || (keychar == 'r' && sign_flag))) {
	errorLvl = NO_BATCH;
	arg_error();		/* interactive process, no go in batch mode */
    }
    /*
     * If we're not doing anything that uses stdout, produce output there,
     * in case user wants to redirect it.
     */
    if (!filter_mode)
	pgpout = stdout;

    switch (keychar) {

/*-------------------------------------------------------*/
    case 'g':
	{		/*      Key generation
			   Arguments: bitcount, bitcount
			 */
	    char keybits[6], ebits[6], *username = NULL;

	    /* 
	     * Why all this code?
	     * 
	     * Some people may object to PGP insisting on finding the
	     * manual somewhere in the neighborhood to generate a key.
	     * They bristle against this seemingly authoritarian
	     * attitude.  Some people have even modified PGP to defeat
	     * this feature, and redistributed their hotwired version to
	     * others.  That creates problems for me (PRZ).
	     * 
	     * Here is the problem.  Before I added this feature, there
	     * were maimed versions of the PGP distribution package
	     * floating around that lacked the manual.  One of them was
	     * uploaded to Compuserve, and was distributed to countless
	     * users who called me on the phone to ask me why such a
	     * complicated program had no manual.  It spread out to BBS
	     * systems around the country.  And a freeware distributor got
	     * hold of the package from Compuserve and enshrined it on
	     * CD-ROM, distributing thousands of copies without the
	     * manual.  What a mess.
	     * 
	     * Please don't make my life harder by modifying PGP to
	     * disable this feature so that others may redistribute PGP
	     * without the manual.  If you run PGP on a palmtop with no
	     * memory for the manual, is it too much to ask that you type
	     * one little extra word on the command line to do a key
	     * generation, a command that is seldom used by people who
	     * already know how to use PGP?  If you can't stand even this
	     * trivial inconvenience, can you suggest a better method of
	     * reducing PGP's distribution without the manual?
	     * 
	     * PLEASE DO NOT DISABLE THIS CHECK IN THE SOURCE CODE
	     * WITHOUT AT LEAST CALLING PHILIP ZIMMERMANN 
	     * (+1 303 541-0140, or prz@acm.org) TO DISCUSS IT. 
	     */
	    if (!nomanual && manuals_missing()) {
		char const *const *dir;
		fputs(LANG("\a\nError: PGP User's Guide not found.\n\
PGP looked for it in the following directories:\n"), pgpout);
#ifdef MACTC5
		fprintf(pgpout, "\t\"%s\"\n", appPathName);
#else
		for (dir = manual_dirs; *dir; dir++)
		    fprintf(pgpout, "\t\"%s\"\n", *dir);
#endif	/* MACTC5 */
		fputs(
LANG("and the doc subdirectory of each of the above.  Please put a copy of\n\
both volumes of the User's Guide in one of these directories.\n\
\n\
Under NO CIRCUMSTANCES should PGP ever be distributed without the PGP\n\
User's Guide, which is included in the standard distribution package.\n\
If you got a copy of PGP without the manual, please inform whomever you\n\
got it from that this is an incomplete package that should not be\n\
distributed further.\n\
\n\
PGP will not generate a key without finding the User's Guide.\n\
There is a simple way to override this restriction.  See the\n\
PGP User's Guide for details on how to do it.\n\
\n"), pgpout);
		return KEYGEN_ERROR;
	    }
	    if (myArgc > 2)
		strncpy(keybits, myArgv[2], sizeof(keybits) - 1);
	    else
		keybits[0] = '\0';

	    if (myArgc > 3)
		strncpy(ebits, myArgv[3], sizeof(ebits) - 1);
	    else
		ebits[0] = '\0';

	    /* If the -u option is given, use that username */
	    if (u_flag && my_name != NULL && *my_name != '\0')
		username = my_name;

	    /* dokeygen writes the keys out to the key rings... */
	    status = dokeygen(keybits, ebits, username);

	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keygen error. "));
		errorLvl = KEYGEN_ERROR;
	    }
#ifdef MACTC5
		else  {
			strcpy(ringfile, globalPubringName );
			PGPSetFinfo(ringfile,'PKey','MPGP');
			strcpy(ringfile, globalSecringName  );
			PGPSetFinfo(ringfile,'SKey','MPGP');                
		}
#endif              
	    return status;
	}			/* Key generation */

/*-------------------------------------------------------*/
    case 'c':
	{			/*      Key checking
				   Arguments: userid, ringfile
				 */

	    if (myArgc < 3) {	/* Default to all user ID's */
		mcguffin[0] = '\0';
	    } else {
		strcpy(mcguffin, myArgv[2]);
		if (strcmp(mcguffin, "*") == 0)
		    mcguffin[0] = '\0';
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    if (myArgc < 4)	/* default key ring filename */
		strcpy(ringfile, globalPubringName);
	    else
		strncpy(ringfile, myArgv[3], sizeof(ringfile) - 1);

	    if ((myArgc < 4 && myArgc > 2)     /* Allow just key file as arg */
		&&has_extension(myArgv[2], PGP_EXTENSION)) {
		strcpy(ringfile, myArgv[2]);
		mcguffin[0] = '\0';
	    }
	    status = dokeycheck(mcguffin, ringfile, CHECK_ALL);

	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keyring check error.\n"));
		errorLvl = KEYRING_CHECK_ERROR;
	    }
	    if (status >= 0 && mcguffin[0] != '\0')
		return status;	/* just checking a single user,
				   dont do maintenance */

	    if ((status = maint_check(ringfile, 0)) < 0 && status != -7) {
		fprintf(pgpout, LANG("\007Maintenance pass error. "));
		errorLvl = KEYRING_CHECK_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			PGPSetFinfo(ringfile,'SKey','MPGP');
		else if (ctb == CTB_CERT_PUBKEY)
		PGPSetFinfo(ringfile,'PKey','MPGP');
		}
#endif
	    return status == -7 ? 0 : status;
	}			/* Key check */

/*-------------------------------------------------------*/
    case 'm':
	{			/*      Maintenance pass
				   Arguments: ringfile
				 */

	    if (myArgc < 3)	/* default key ring filename */
		strcpy(ringfile, globalPubringName);
	    else
		strcpy(ringfile, myArgv[2]);

#ifdef MSDOS
	    strlwr(ringfile);
#endif
	    if (!file_exists(ringfile))
		default_extension(ringfile, PGP_EXTENSION);

	    if ((status = maint_check(ringfile,
		      MAINT_VERBOSE | (c_flag ? MAINT_CHECK : 0))) < 0) {
		if (status == -7)
		    fprintf(pgpout,
			    LANG("File '%s' is not a public keyring\n"),
			    ringfile);
		fprintf(pgpout, LANG("\007Maintenance pass error. "));
		errorLvl = KEYRING_CHECK_ERROR;
	    }
#ifdef MACTC5
		PGPSetFinfo(ringfile,'PKey','MPGP');
#endif
	    return status;
	}			/* Maintenance pass */

/*-------------------------------------------------------*/
    case 's':
	{			/*      Key signing
				   Arguments: her_id, keyfile
				 */

	    if (myArgc >= 4)
		strncpy(keyfile, myArgv[3], sizeof(keyfile) - 1);
	    else
		strcpy(keyfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to sign */
	    } else {
		fprintf(pgpout,
LANG("\nA user ID is required to select the public key you want to sign. "));
		if (batchmode)	/* not interactive, userid
				   must be on command line */
		    return -1;
		fprintf(pgpout, LANG("\nEnter the public key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the public key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    if (my_name[0] == '\0') {
		fprintf(pgpout,
LANG("\nA secret key is required to make a signature. "));
		fprintf(pgpout,
LANG("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
	    }
	    status = signkey(mcguffin, my_name, keyfile);

	    if (status >= 0) {
		status = maint_update(keyfile, 0);
		if (status == -7) { /* ringfile is a keyfile or
				       secret keyring */
		    fprintf(pgpout,
			    "Warning: '%s' is not a public keyring\n",
			    keyfile);
		    return 0;
		}
		if (status < 0)
		    fprintf(pgpout, LANG("\007Maintenance pass error. "));
	    }
	    if (status < 0) {
		fprintf(pgpout, LANG("\007Key signature error. "));
		errorLvl = KEY_SIGNATURE_ERROR;
	    }
#ifdef MACTC5
		PGPSetFinfo(keyfile,'PKey','MPGP');
#endif
	    return status;
	}			/* Key signing */


/*-------------------------------------------------------*/
    case 'd':
	{			/*      disable/revoke key
				   Arguments: userid, keyfile
				 */

	    if (myArgc >= 4)
		strncpy(keyfile, myArgv[3], sizeof(keyfile) - 1);
	    else
		strcpy(keyfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to sign */
	    } else {
		fprintf(pgpout,
LANG("\nA user ID is required to select the key you want to revoke or \
disable. "));
		fprintf(pgpout, LANG("\nEnter user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    status = disable_key(mcguffin, keyfile);

	    if (status >= 0) {
		status = maint_update(keyfile, 0);
		if (status == -7) { /* ringfile is a keyfile or
				       secret keyring */
		    fprintf(pgpout, "Warning: '%s' is not a public keyring\n",
			    keyfile);
		    return 0;
		}
		if (status < 0)
		    fprintf(pgpout, LANG("\007Maintenance pass error. "));
	    }
	    if (status < 0)
		errorLvl = KEY_SIGNATURE_ERROR;
#ifdef MACTC5
		PGPSetFinfo(keyfile,'PKey','MPGP');
#endif
	    return status;
	}			/* Key compromise */

/*-------------------------------------------------------*/
    case 'e':
	{			/*      Key editing
				   Arguments: userid, ringfile
				 */

	    if (myArgc >= 4)
		strncpy(ringfile, myArgv[3], sizeof(ringfile) - 1);
	    else		/* default key ring filename */
		strcpy(ringfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to edit */
	    } else {
		fprintf(pgpout,
LANG("\nA user ID is required to select the key you want to edit. "));
		fprintf(pgpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    status = dokeyedit(mcguffin, ringfile);

	    if (status >= 0) {
		status = maint_update(ringfile, 0);
		if (status == -7)
		    status = 0;	/* ignore "not a public keyring" error */
		if (status < 0)
		    fprintf(pgpout, LANG("\007Maintenance pass error. "));
	    }
	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keyring edit error. "));
		errorLvl = KEYRING_EDIT_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			PGPSetFinfo(ringfile,'SKey','MPGP');
		else if (ctb == CTB_CERT_PUBKEY)
		PGPSetFinfo(ringfile,'PKey','MPGP');
		}
#endif
	    return status;
	}			/* Key edit */

/*-------------------------------------------------------*/
    case 'a':
	{			/*      Add key to key ring
				   Arguments: keyfile, ringfile
				 */

	    if (myArgc < 3 && !filter_mode)
		arg_error();

	    if (!filter_mode) {	/* Get the keyfile from args */
		strncpy(keyfile, myArgv[2], sizeof(keyfile) - 1);

#ifdef MSDOS
		strlwr(keyfile);
#endif
		if (!file_exists(keyfile))
		    default_extension(keyfile, PGP_EXTENSION);

		if (!file_exists(keyfile)) {
		    fprintf(pgpout,
			    LANG("\n\007Key file '%s' does not exist.\n"),
			    keyfile);
		    errorLvl = NONEXIST_KEY_ERROR;
		    return -1;
		}
		workfile = keyfile;

	    } else {
		workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
		readPhantomInput(workfile);
	    }

	    if (myArgc < (filter_mode ? 3 : 4)) { /* default key ring
						     filename */
		byte ctb;
		get_header_info_from_file(workfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
		    strcpy(ringfile, globalSecringName);
		else
		    strcpy(ringfile, globalPubringName);
	    } else {
		strncpy(ringfile, myArgv[(filter_mode ? 2 : 3)],
			sizeof(ringfile) - 1);
		default_extension(ringfile, PGP_EXTENSION);
	    }
#ifdef MSDOS
	    strlwr(ringfile);
#endif

	    status = addto_keyring(workfile, ringfile);

	    if (filter_mode)
		rmtemp(workfile);

	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keyring add error. "));
		errorLvl = KEYRING_ADD_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			PGPSetFinfo(ringfile,'SKey','MPGP');
		else if (ctb == CTB_CERT_PUBKEY)
		PGPSetFinfo(ringfile,'PKey','MPGP');
		}
#endif
	    return status;
	}			/* Add key to key ring */

/*-------------------------------------------------------*/
    case 'x':
	{			/*      Extract key from key ring
				   Arguments: mcguffin, keyfile, ringfile
				 */

	    if (myArgc >= (filter_mode ? 4 : 5)) /* default key ring
						    filename */
		strncpy(ringfile, myArgv[(filter_mode ? 3 : 4)],
			sizeof(ringfile) - 1);
	    else
		strcpy(ringfile, globalPubringName);

	    if (myArgc >= (filter_mode ? 2 : 3)) {
		if (myArgv[2])
		    /* Userid to extract */
		    strcpy(mcguffin, myArgv[2]);
		else
		    strcpy(mcguffin, "");
	    } else {
		fprintf(pgpout,
LANG("\nA user ID is required to select the key you want to extract. "));
		if (batchmode)	/* not interactive, userid
				   must be on command line */
		    return -1;
		fprintf(pgpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    if (!filter_mode) {
		if (myArgc >= 4)
		    strncpy(keyfile, myArgv[3], sizeof(keyfile) - 1);
		else
		    keyfile[0] = '\0';

		workfile = keyfile;
	    } else {
		workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	    }

#ifdef MSDOS
	    strlwr(workfile);
	    strlwr(ringfile);
#endif

	    default_extension(ringfile, PGP_EXTENSION);

	    status = extract_from_keyring(mcguffin, workfile,
					  ringfile, (filter_mode ? FALSE :
						     emit_radix_64));

	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keyring extract error. "));
		errorLvl = KEYRING_EXTRACT_ERROR;
		if (filter_mode)
		    rmtemp(workfile);
		return status;
	    }
	    if (filter_mode && !status) {
		if (emit_radix_64) {
		    /* NULL for outputfile means write to stdout */
		    if (armor_file(workfile, NULL, NULL, NULL, FALSE) != 0) {
			errorLvl = UNKNOWN_FILE_ERROR;
			return -1;
		    }
		} else {
		    if (writePhantomOutput(workfile) < 0) {
			errorLvl = UNKNOWN_FILE_ERROR;
			return -1;
		    }
		}
		rmtemp(workfile);
	    }
#ifdef MACTC5
		if (status)
			return KEYRING_EXTRACT_ERROR;
		if ((!emit_radix_64)&&(strlen(keyfile)>0)) {
		byte ctb;
		get_header_info_from_file(keyfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			PGPSetFinfo(ringfile,'SKey','MPGP');
		else if (ctb == CTB_CERT_PUBKEY)
			PGPSetFinfo(ringfile,'PKey','MPGP');
		}
#endif
	    return 0;
	}			/* Extract key from key ring */

/*-------------------------------------------------------*/
    case 'r':
	{	/*      Remove keys or selected key signatures from userid keys
			Arguments: userid, ringfile
		 */

	    if (myArgc >= 4)
		strcpy(ringfile, myArgv[3]);
	    else		/* default key ring filename */
		strcpy(ringfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to work on */
	    } else {
		if (sign_flag) {
		    fprintf(pgpout,
LANG("\nA user ID is required to select the public key you want to\n\
remove certifying signatures from. "));
		} else {
		    fprintf(pgpout,
LANG("\nA user ID is required to select the key you want to remove. "));
		}
		if (batchmode)	/* not interactive, userid must be on
				   command line */
		    return -1;
		fprintf(pgpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

#ifdef MSDOS
	    strlwr(ringfile);
#endif
	    if (!file_exists(ringfile))
		default_extension(ringfile, PGP_EXTENSION);

	    if (sign_flag) {	/* Remove signatures */
		if (remove_sigs(mcguffin, ringfile) < 0) {
		    fprintf(pgpout, LANG("\007Key signature remove error. "));
		    errorLvl = KEYSIG_REMOVE_ERROR;
		    return -1;
		}
	    } else {		/* Remove keyring */
#ifdef MACTC5
			if (remove_from_keyring( NULL, mcguffin, ringfile,
					(boolean)!strcmp(ringfile, globalPubringName))) {
#else
		if (remove_from_keyring(NULL, mcguffin, ringfile,
					(boolean) (myArgc < 4)) < 0) {
#endif
		    fprintf(pgpout, LANG("\007Keyring remove error. "));
		    errorLvl = KEYRING_REMOVE_ERROR;
		    return -1;
		}
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			PGPSetFinfo(ringfile,'SKey','MPGP');
		else if (ctb == CTB_CERT_PUBKEY)
		PGPSetFinfo(ringfile,'PKey','MPGP');
		PGPSetFinfo(globalPubringName,'PKey','MPGP');
		}
#endif
	    return 0;
	}			/* remove key signatures from userid */

/*-------------------------------------------------------*/
    case 'v':
    case 'V':			/* -kvv */
	{			/* View or remove key ring entries,
				   with userid match
				   Arguments: userid, ringfile
				 */

	    if (myArgc < 4)	/* default key ring filename */
		strcpy(ringfile, globalPubringName);
	    else
		strcpy(ringfile, myArgv[3]);

	    if (myArgc > 2) {
		strcpy(mcguffin, myArgv[2]);
		if (strcmp(mcguffin, "*") == 0)
		    mcguffin[0] = '\0';
	    } else {
		*mcguffin = '\0';
	    }

	    if ((myArgc == 3) && has_extension(myArgv[2], PGP_EXTENSION)) {
		strcpy(ringfile, myArgv[2]);
		mcguffin[0] = '\0';
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

#ifdef MSDOS
	    strlwr(ringfile);
#endif
	    if (!file_exists(ringfile))
		default_extension(ringfile, PGP_EXTENSION);

	    /* If a second 'v' (keychar = V), show signatures too */
	    status = view_keyring(mcguffin, ringfile,
				  (boolean) (keychar == 'V'), c_flag);
	    if (status < 0) {
		fprintf(pgpout, LANG("\007Keyring view error. "));
		errorLvl = KEYRING_VIEW_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			PGPSetFinfo(ringfile,'SKey','MPGP');
		else if (ctb == CTB_CERT_PUBKEY)
		PGPSetFinfo(ringfile,'PKey','MPGP');
		}
#endif
	    return status;
	}			/* view key ring entries, with userid match */

    default:
	arg_error();
    }
    return 0;
}				/* do_keyopt */

/* comes here if user made a boo-boo. */
void user_error()
{
    fprintf(pgpout, LANG("\nFor a usage summary, type:  pgp -h\n"));
    fprintf(pgpout,
	    LANG("For more detailed help, consult the PGP User's Guide.\n"));
    exitPGP(errorLvl ? errorLvl : 127);		/* error exit */
}

#if defined(DEBUG) && defined(linux)
#include <malloc.h>
#endif

/*
 * exitPGP: wipes and removes temporary files, also tries to wipe
 * the stack.
 */
void exitPGP(int returnval)
{
    char buf[STACK_WIPE];
    struct hashedpw *hpw;

    if (verbose)
	fprintf(pgpout, "exitPGP: exitcode = %d\n", returnval);
    for (hpw = passwds; hpw; hpw = hpw->next)
	memset(hpw->hash, 0, sizeof(hpw->hash));
    for (hpw = keypasswds; hpw; hpw = hpw->next)
	memset(hpw->hash, 0, sizeof(hpw->hash));
#ifdef MACTC5
	mac_cleanup_tmpf();
#else
    cleanup_tmpf();
#endif
    /* Merge any entropy we collected into the randseed.bin file */
    if (cryptRandOpen((struct IdeaCfbContext *)0) >= 0)
	    cryptRandSave((struct IdeaCfbContext *)0);
#if defined(DEBUG) && defined(linux)
    if (verbose) {
	struct mstats mstat;
	mstat = mstats();
	printf("%d chunks used (%d bytes)  %d bytes total\n",
	       mstat.chunks_used, mstat.bytes_used, mstat.bytes_total);
    }
#endif
    memset(buf, 0, sizeof(buf));	/* wipe stack */
#ifdef VMS
/*
 * Fake VMS style error returns with severity in bottom 3 bits
 */
    if (returnval)
	returnval = (returnval << 3) | 0x10000002;
    else
	returnval = 0x10000001;
#endif				/* VMS */
    exit(returnval);
}

static void arg_error()
{
    signon_msg();
    fprintf(pgpout, LANG("\nInvalid arguments.\n"));
    errorLvl = BAD_ARG_ERROR;
    user_error();
}

/*
 * Check for language specific help files in PGPPATH, then the system
 * directory.  If that fails, check for the default pgp.hlp, again
 * first a private copy, then the system-wide one.
 *
 * System-wide copies currently only exist on Unix.
 */
static void build_helpfile(char *helpfile, char const *extra)
{
    if (strcmp(language, "en")) {
	buildfilename(helpfile, language);
	strcat(helpfile, extra);
	force_extension(helpfile, HLP_EXTENSION);
	if (file_exists(helpfile))
	    return;
#ifdef PGP_SYSTEM_DIR
	strcpy(helpfile, PGP_SYSTEM_DIR);
	strcat(helpfile, language);
	strcat(helpfile, extra);
	force_extension(helpfile, HLP_EXTENSION);
	if (file_exists(helpfile))
	    return;
#endif
    }
    buildfilename(helpfile, "pgp");
    strcat(helpfile, extra);
    force_extension(helpfile, HLP_EXTENSION);
#ifdef PGP_SYSTEM_DIR
    if (file_exists(helpfile))
	return;
    strcpy(helpfile, PGP_SYSTEM_DIR);
    strcat(helpfile, "pgp");
    strcat(helpfile, extra);
    force_extension(helpfile, HLP_EXTENSION);
#endif
}

static void usage()
{
    char helpfile[MAX_PATH];
    char *tmphelp = helpfile;
    extern unsigned char *ext_c_ptr;

    signon_msg();
    build_helpfile(helpfile, "");

    if (ext_c_ptr) {
	/* conversion to external format necessary */
	tmphelp = tempfile(TMP_TMPDIR);
	CONVERSION = EXT_CONV;
	if (copyfiles_by_name(helpfile, tmphelp) < 0) {
	    rmtemp(tmphelp);
	    tmphelp = helpfile;
	}
	CONVERSION = NO_CONV;
    }
    /* built-in help if pgp.hlp is not available */
    if (more_file(tmphelp, FALSE) < 0)
	fprintf(pgpout, LANG("\nUsage summary:\
\nTo encrypt a plaintext file with recipent's public key, type:\
\n   pgp -e textfile her_userid [other userids] (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key:\
\n   pgp -s textfile [-u your_userid]           (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key, and then encrypt it\
\n   with recipent's public key, producing a .pgp file:\
\n   pgp -es textfile her_userid [other userids] [-u your_userid]\
\nTo encrypt with conventional encryption only:\
\n   pgp -c textfile\
\nTo decrypt or check a signature for a ciphertext (.pgp) file:\
\n   pgp ciphertextfile [-o plaintextfile]\
\nTo produce output in ASCII for email, add the -a option to other options.\
\nTo generate your own unique public/secret key pair:  pgp -kg\
\nFor help on other key management functions, type:   pgp -k\n"));
    if (ext_c_ptr)
	rmtemp(tmphelp);
    exit(BAD_ARG_ERROR);	/* error exit */
}

static void key_usage()
{
    char helpfile[MAX_PATH];
    char *tmphelp = helpfile;
    extern unsigned char *ext_c_ptr;

    signon_msg();
    build_helpfile(helpfile, "key");

    if (ext_c_ptr) {
	/* conversion to external format necessary */
	tmphelp = tempfile(TMP_TMPDIR);
	CONVERSION = EXT_CONV;
	if (copyfiles_by_name(helpfile, tmphelp) < 0) {
	    rmtemp(tmphelp);
	    tmphelp = helpfile;
	}
	CONVERSION = NO_CONV;
    }
    /* built-in help if key.hlp is not available */
    if (more_file(tmphelp, FALSE) < 0)
	/* only use built-in help if there is no helpfile */
	fprintf(pgpout, LANG("\nKey management functions:\
\nTo generate your own unique public/secret key pair:\
\n   pgp -kg\
\nTo add a key file's contents to your public or secret key ring:\
\n   pgp -ka keyfile [keyring]\
\nTo remove a key or a user ID from your public or secret key ring:\
\n   pgp -kr userid [keyring]\
\nTo edit your user ID or pass phrase:\
\n   pgp -ke your_userid [keyring]\
\nTo extract (copy) a key from your public or secret key ring:\
\n   pgp -kx userid keyfile [keyring]\
\nTo view the contents of your public key ring:\
\n   pgp -kv[v] [userid] [keyring]\
\nTo check signatures on your public key ring:\
\n   pgp -kc [userid] [keyring]\
\nTo sign someone else's public key on your public key ring:\
\n   pgp -ks her_userid [-u your_userid] [keyring]\
\nTo remove selected signatures from a userid on a keyring:\
\n   pgp -krs userid [keyring]\
\n"));
    if (ext_c_ptr)
	rmtemp(tmphelp);
    exit(BAD_ARG_ERROR);	/* error exit */
}

char **ParseRecipients(char **recipients)
{
	/*
	 * ParseRecipients() expects an array of pointers to
	 * characters, usually the array returned by the C startup
	 * code. Then it will look for entries beginning with the
  	 * string "-@" followed by a filename, which may be appended
 	 * directly or seperated by a blank.
 	 *
 	 * If the file exists and is readable, the routine will load
 	 * the contents and insert it into the command line as if the
 	 * names had been specified there.
 	 *
 	 * Each entry in the file consists of one line. The file line
 	 * will be treated as one argument, no matter whether it
 	 * contains spaces or not. Lines beginning with "#" will be
 	 * ignored and treated as comments. Empty lines will be ignored
 	 * also. Trailing white spaces will be removed.
 	 *
 	 * Currently, ParseRecipients() uses one fixed buffer, meaning,
 	 * that one single line must not be longer than 255 characters.
 	 * The number of included lines is unlimited.
 	 *
 	 * When any kind of problem occurs, PGP will terminate and do
 	 * nothing. No need to test for an error, the result is always
 	 * correct.
 	 *
  	 *             21-Sep-95, Peter Simons <simons@peti.rhein.de>
 	 */

	char **backup = recipients, **new;
	int entrynum;
 	int MAX_RECIPIENTS = 128;   /* The name is somewhat wrong. of
 				     * course the memory handling is
 				     * dynamic.
 				     */

 	/* Check whether we need to do something or not. */
 	while(*recipients) {
 		if (!strncmp(*recipients, INCLUDE_MARK, INCLUDE_MARK_LEN))
 		    break;
 		recipients++;
 	}
 	if (!*recipients)
 	  return backup;	/* nothin' happened */

 	recipients=backup;
 	if (!(new = malloc(MAX_RECIPIENTS * sizeof(char *))))
 	  exitPGP(OUT_OF_MEM);
 	entrynum = 0;

 	while(*recipients) {
 		if (strncmp(*recipients, INCLUDE_MARK, INCLUDE_MARK_LEN))
                {
 			new[entrynum++] = *recipients++;
 			if (entrynum == MAX_RECIPIENTS) {
 				/* Current buffer is too small.
 				 * Use realloc() to largen itt.
 				 */
 				MAX_RECIPIENTS += 128;
 				if (!(new = realloc(new,
                                 MAX_RECIPIENTS * sizeof(char *))))
 				  exitPGP(OUT_OF_MEM);
 			}
 		}
 		else {
 			/* We got a hit! Load the file and parse it. */
 			FILE *fh;
 			char *filename, tempbuf[256];

 			if (strlen(*recipients) == INCLUDE_MARK_LEN)
 			  filename = *++recipients;
 			else
 			  filename = *recipients+INCLUDE_MARK_LEN;
 			fprintf(pgpout, LANG("\nIncluding \"%s\"...\n"), filename);
 			if (!(fh = fopen(filename, "r"))) {
 				perror("PGP");
 				exitPGP(UNKNOWN_FILE_ERROR);
 			}
 			while(fgets(tempbuf, sizeof(tempbuf)-1, fh)) {
 				int i = strlen(tempbuf);

 				/* Test for comments or empty lines. */
 				if (!i || *tempbuf == '#')
 				  continue;

 				/* Remove trailing blanks. */
 				while (isspace(tempbuf[i-1]))
 				  i--;
 				tempbuf[i] = '\0';

 				/* Copy new entry to new */
 				if (!(new[entrynum++] = store_str(tempbuf)))
 				  exitPGP(OUT_OF_MEM);
 				if (entrynum == MAX_RECIPIENTS) {
 					/* Current buffer is too small.
 					 * Use realloc() to largen itt.
 					 */
 					MAX_RECIPIENTS += 128;
 					if (!(new = realloc(new,
                                         MAX_RECIPIENTS * sizeof(char *))))
 					  exitPGP(OUT_OF_MEM);
 				}
 			}
 			if (ferror(fh)) {
 				perror("PGP");
 				exitPGP(UNKNOWN_FILE_ERROR);
 			}
 			fclose(fh);
 			recipients++;
 		}
 	}

 	/*
 	 * We have to write one trailing NULL pointer.
 	 * Check array size first.
 	 */
 	if (entrynum == MAX_RECIPIENTS) {
 		if (!(new = realloc(new, (MAX_RECIPIENTS+1) * sizeof(char *))))
 		  exitPGP(OUT_OF_MEM);
 	}
 	new[entrynum] = NULL;
 	return new;
}
