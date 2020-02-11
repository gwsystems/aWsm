/*      armor.c  - ASCII/binary encoding/decoding based partly on
   PEM RFC1113 and MIME standards.
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
#include <string.h>
#include "mpilib.h"
#include "fileio.h"
#include "mpiio.h"
#include "language.h"
#include "pgp.h"
#include "charset.h"
#include "crypto.h"
#include "armor.h"
#include "keymgmt.h"
#ifdef MACTC5
#include "Macutil2.h"
#include "Macutil3.h"
#endif

static int darmor_file(char *infile, char *outfile);
static crcword crchware(byte ch, crcword poly, crcword accum);
static int armordecode(FILE * in, FILE * out, int *warned);
static void mk_crctbl(crcword poly);
static boolean is_armorfile(char *infile);

/*      Begin ASCII armor routines.
   This converts a binary file into printable ASCII characters, in a
   radix-64 form mostly compatible with the MIME format.
   This makes it easier to send encrypted files over a 7-bit channel.
 */

/* Index this array by a 6 bit value to get the character corresponding
 * to that value.
 */
static
unsigned char bintoasc[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Index this array by a 7 bit value to get the 6-bit binary field
 * corresponding to that value.  Any illegal characters return high bit set.
 */
static
unsigned char asctobin[] =
{
#ifdef EBCDIC
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128, 62,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128, 63,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128, 26, 27, 28, 29, 30, 31, 32,  33, 34,128,128,128,128,128,128,
    128, 35, 36, 37, 38, 39, 40, 41,  42, 43,128,128,128,128,128,128,
    128,128, 44, 45, 46, 47, 48, 49,  50, 51,128,128,128,128,128,128,
    128,128,128,128,128,128,128,128, 128,128,128,128,128,128,128,128,
    128,  0,  1,  2,  3,  4,  5,  6,   7,  8,128,128,128,128,128,128,
    128,  9, 10, 11, 12, 13, 14, 15,  16, 17,128,128,128,128,128,128,
    128,128, 18, 19, 20, 21, 22, 23,  24, 25,128,128,128,128,128,128,
     52, 53, 54, 55, 56, 57, 58, 59,  60, 61,128,128,128,128,128,128
#else
    0200, 0200, 0200, 0200, 0200, 0200, 0200, 0200,
    0200, 0200, 0200, 0200, 0200, 0200, 0200, 0200,
    0200, 0200, 0200, 0200, 0200, 0200, 0200, 0200,
    0200, 0200, 0200, 0200, 0200, 0200, 0200, 0200,
    0200, 0200, 0200, 0200, 0200, 0200, 0200, 0200,
    0200, 0200, 0200, 0076, 0200, 0200, 0200, 0077,
    0064, 0065, 0066, 0067, 0070, 0071, 0072, 0073,
    0074, 0075, 0200, 0200, 0200, 0200, 0200, 0200,
    0200, 0000, 0001, 0002, 0003, 0004, 0005, 0006,
    0007, 0010, 0011, 0012, 0013, 0014, 0015, 0016,
    0017, 0020, 0021, 0022, 0023, 0024, 0025, 0026,
    0027, 0030, 0031, 0200, 0200, 0200, 0200, 0200,
    0200, 0032, 0033, 0034, 0035, 0036, 0037, 0040,
    0041, 0042, 0043, 0044, 0045, 0046, 0047, 0050,
    0051, 0052, 0053, 0054, 0055, 0056, 0057, 0060,
    0061, 0062, 0063, 0200, 0200, 0200, 0200, 0200
#endif
};

/* Current line number for mult decodes */
#ifdef MACTC5
long	infile_line;		/* Current line number for mult decodes */
#else
static long infile_line;
#endif

/************************************************************************/

/* CRC Routines. */
/*      These CRC functions are derived from code in chapter 19 of the book 
 *    "C Programmer's Guide to Serial Communications", by Joe Campbell.
 *      Generalized to any CRC width by Philip Zimmermann.
 */

#define byte unsigned char

#define CRCBITS 24		/* may be 16, 24, or 32 */
/* #define maskcrc(crc) ((crcword)(crc)) *//* if CRCBITS is 16 or 32 */
#define maskcrc(crc) ((crc) & 0xffffffL)	/* if CRCBITS is 24 */
#define CRCHIBIT ((crcword) (1L<<(CRCBITS-1)))	/* 0x8000 if CRCBITS is 16 */
#define CRCSHIFTS (CRCBITS-8)

/*
 * Notes on making a good 24-bit CRC--
 * The primitive irreducible polynomial of degree 23 over GF(2),
 * 040435651 (octal), comes from Appendix C of "Error Correcting Codes,
 * 2nd edition" by Peterson and Weldon, page 490.  This polynomial was
 * chosen for its uniform density of ones and zeros, which has better
 * error detection properties than polynomials with a minimal number of
 * nonzero terms.  Multiplying this primitive degree-23 polynomial by
 * the polynomial x+1 yields the additional property of detecting any
 * odd number of bits in error, which means it adds parity.  This 
 * approach was recommended by Neal Glover.
 *
 * To multiply the polynomial 040435651 by x+1, shift it left 1 bit and
 * bitwise add (xor) the unshifted version back in.  Dropping the unused 
 * upper bit (bit 24) produces a CRC-24 generator bitmask of 041446373 
 * octal, or 0x864cfb hex.  
 *
 * You can detect spurious leading zeros or framing errors in the 
 * message by initializing the CRC accumulator to some agreed-upon 
 * nonzero value, but the value used is a bit nonstandard.  
 */

#define CCITTCRC 0x1021		/* CCITT's 16-bit CRC generator polynomial */
#define PRZCRC 0x864cfbL	/* PRZ's 24-bit CRC generator polynomial */
#define CRCINIT 0xB704CEL	/* Init value for CRC accumulator */

static crcword crctable[256];	/* Table for speeding up CRC's */

/*
 * mk_crctbl derives a CRC lookup table from the CRC polynomial.
 * The table is used later by the crcbytes function given below.
 * mk_crctbl only needs to be called once at the dawn of time.
 *
 * The theory behind mk_crctbl is that table[i] is initialized
 * with the CRC of i, and this is related to the CRC of i>>1,
 * so the CRC of i>>1 (pointed to by p) can be used to derive
 * the CRC of i (pointed to by q).
 */
static void
mk_crctbl(crcword poly)
{
    int i;
    crcword t, *p, *q;
    p = q = crctable;
    *q++ = 0;
    *q++ = poly;
    for (i = 1; i < 128; i++) {
	t = *++p;
	if (t & CRCHIBIT) {
	    t <<= 1;
	    *q++ = t ^ poly;
	    *q++ = t;
	} else {
	    t <<= 1;
	    *q++ = t;
	    *q++ = t ^ poly;
	}
    }
}

/*
 * Accumulate a buffer's worth of bytes into a CRC accumulator,
 * returning the new CRC value.
 */
crcword
crcbytes(byte * buf, unsigned len, register crcword accum)
{
    do {
	accum = accum << 8 ^ crctable[(byte) (accum >> CRCSHIFTS) ^ *buf++];
    } while (--len);
    return maskcrc(accum);
}				/* crcbytes */

/* Initialize the CRC table using our codes */
void
init_crc(void)
{
    mk_crctbl(PRZCRC);
}


/************************************************************************/


/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ((int)bintoasc[((c) & 077)])
#define PAD		'='

/*
 * output one group of up to 3 bytes, pointed at by p, on file f.
 * if fewer than 3 are present, the 1 or two extras must be zeros.
 */
static void
outdec(char *p, FILE *f, int count)
{
    int c1, c2, c3, c4;

    c1 = *p >> 2;
    c2 = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
    c3 = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
    c4 = p[2] & 077;
    putc(ENC(c1), f);
    putc(ENC(c2), f);
    if (count == 1) {
	putc(PAD, f);
	putc(PAD, f);
    } else {
	putc(ENC(c3), f);
	if (count == 2)
	    putc(PAD, f);
	else
	    putc(ENC(c4), f);
    }
}				/* outdec */


/* Output the CRC value, MSB first per normal CRC conventions */
static void
outcrc(word32 crc, FILE *outFile)
{
    char crcbuf[4];
    crcbuf[0] = (crc >> 16) & 0xff;
    crcbuf[1] = (crc >> 8) & 0xff;
    crcbuf[2] = (crc >> 0) & 0xff;
    putc(PAD, outFile);
    outdec(crcbuf, outFile, 3);
    putc('\n', outFile);
}				/* outcrc */

/* Return filename for output (text mode), but replace last letter(s) of
 * filename with the ascii for num.  It will use the appropriate number
 * of digits for ofnum when converting num, so if ofnum < 10, use 1 digit,
 * >= 10 and < 100 use 2 digits, >= 100 and < 1000 use 3 digits.  If its
 * >= 1000, then we have other problems to worry about, and this might do
 * weird things.
 */
static char *
numFilename(char *fname, int num, int ofnum)
{
    static char fnamenum[MAX_PATH];
    int len;
    int offset = 1;

    strcpy(fnamenum, fname);
    len = strlen(fnamenum);
    do {
	fnamenum[len - offset] = '0' + (num % 10);
	num /= 10;
	ofnum /= 10;
	offset++;
    } while (ofnum >= 1 && offset < 4);
    return fnamenum;
}

/*
 * Reads and discards a line from the given file.  Returns -1 on error or
 * EOF, 0 if the line is blank, and 1 if the line is not blank.
 */
static int
skipline(FILE * f)
{
    int state, flag, c;

    state = 0;
    flag = 0;
    for (;;) {
	c = getc(f);
	if (c == '\n')
	    return flag;
	if (state) {
	    ungetc(c, f);
	    return flag;
	}
	if (c == EOF)
	    return -1;
	if (c == '\r')
	    state = 1;
	else if (c != ' ')
	    flag = 1;
    }
}				/* skipline */


/*
 * Copies a line from the input file to the output.  Does NOT copy the
 * trailing newline.  Returns -1 on EOF or error, 0 if the line was terminated
 * by EOF, and 1 if the line was terminated with a newline sequence.
 */
static int
copyline(FILE * in, FILE * out)
{
    int state, flag, c;

    state = 0;
    for (;;) {
	c = getc(in);
	if (c == '\n')
	    return 1;
	if (state) {
	    ungetc(c, in);
	    return 1;
	}
	if (c == EOF)
	    return 0;
	if (c == '\r')
	    state = 1;
	else
	    putc(c, out);
    }
}				/* copyline */

/*
 * Reads a line from file f, up to the size of the buffer.  The line in the
 * buffer will NOT include line termination, although any of (CR, LF, CRLF)
 * is accepted on input.  The return value is -1 on error, 0 if the line
 * was terminated abnormally (EOF, error, or out of buffer space), and
 * 1 if the line was terminated normally.
 *
 * Passing in a buffer less than 2 characters long is not a terribly bright
 * idea.
 */
static int
pgp_getline(char *buf, int n, FILE * f)
{
    int state;
    char *p;
    int c;

    state = 0;
    p = buf;
    for (;;) {
	c = getc(f);
	if (c == '\n') {
	    *p = 0;
	    return 1;		/* Line terminated with \n or \r\n */
	}
	if (state) {
	    ungetc(c, f);
	    *p = 0;
	    return 1;		/* Line terminated with \r */
	}
	if (c == EOF) {
	    *p = 0;
	    return (p == buf) ? -1 : 0;		/* Error */
	}
	if (c == '\r')
	    state = 1;
	else if (--n > 0) {
	    *p++ = c;
	} else {
	    ungetc(c, f);
	    *p = 0;
	    return 0;		/* Out of buffer space */
	}
    }				/* for (;;) */
}				/* pgp_getline */

#if 1
/* This limit is advisory only; longer lines are handled properly.
 * The only requirement is that this be at least as long as the longest
 * delimiter string used by PGP
 * (e.g. "-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n")
 */
#define MAX_LINE_SIZE 80
#else
#ifdef MSDOS			/* limited stack space */
#define MAX_LINE_SIZE	256
#else
#define MAX_LINE_SIZE	1024
#endif
#endif

/*
 * Read a line from file f, buf must be able to hold at least MAX_LINE_SIZE
 * characters.  Anything after that is ignored.  Strips trailing spaces and
 * line terminator, can read LF, CRLF and CR textfiles.  It can't be ASCII
 * armor anyway.
 */
static char *
get_armor_line(char *buf, FILE * f)
{
    int c, n = MAX_LINE_SIZE-1;
    char *p = buf;

    do {
	c = getc(f);
	if (c == '\n' || c == '\r' || c == EOF)
	    break;
	*p++ = c;
    } while (--n > 0);
    if (p == buf && c == EOF) {
	*buf = '\0';
	return NULL;
    }
    /*
     * Skip to end of line, setting n to non-zero if anything trailing is
     * not a space (meaning that any trailing whitespace in the buffer is
     * not trailing whitespace on the line and should not be stripped).
     */
    n = 0;
    while (c != '\n' && c != '\r' && c != EOF) {
        n |= c ^ ' ';
	c = getc(f);
    }
    if (c == '\r' && (c = getc(f)) != '\n')
	ungetc(c, f);
    if (!n) {	/* Skip trailing whitespace, as described above */
	while (p > buf && p[-1] == ' ')
	    --p;
    }
    *p = '\0';
    return buf;
}


/*
 * Encode a file in sections.  64 ASCII bytes * 720 lines = 46K, 
 * recommended max.  Usenet message size is 50K so this leaves a nice 
 * margin for .signature.  In the interests of orthogonality and 
 * programmer laziness no check is made for a message containing only 
 * a few lines (or even just an 'end')  after a section break. 
 */
#define LINE_LEN	48L
int pem_lines = 720;
#define BYTES_PER_SECTION	(LINE_LEN * pem_lines)

#if defined(VMS) || defined(C370)
#define FOPRARMOR	FOPRTXT
#else
/* armored files are opened in binary mode so that CRLF/LF/CR files
   can be handled by all systems */
#define	FOPRARMOR	FOPRBIN
#endif

extern boolean verbose;		/* Undocumented command mode in PGP.C */
extern boolean filter_mode;

/*
 * Copy from infilename to outfilename, ASCII armoring as you go along,
 * and with breaks every pem_lines lines.
 * If clearfilename is non-NULL, first output that file preceded by a
 * special delimiter line.  filename is the original filename, used
 * only for debugging.
 */
int
armor_file(char *infilename, char *outfilename, char *filename,
	char *clearfilename, boolean kv_label)
{
    char buffer[MAX_LINE_SIZE];
    int i, rc, bytesRead, lines = 0;
    int noSections, currentSection = 1;
    long fileLen;
    crcword crc;
    FILE *inFile, *outFile, *clearFile;
    char *tempf;
    char *blocktype = "MESSAGE";
#ifdef MACTC5
    char curOutFile[256]="";
#endif

    if (verbose)
	fprintf(pgpout,
"armor_file: infile = %s, outfile = %s, filename = %s, clearname = %s\n",
		infilename, outfilename, filename,
		clearfilename == NULL ? "" : clearfilename);

    /* open input file as binary */
    if ((inFile = fopen(infilename, FOPRBIN)) == NULL)
	return 1;

    if (!outfilename || pem_lines == 0) {
	noSections = 1;
    } else {
	/* Evaluate how many parts this file will comprise */
	fseek(inFile, 0L, SEEK_END);
	fileLen = ftell(inFile);
	rewind(inFile);
	noSections = (fileLen + BYTES_PER_SECTION - 1) /
	    BYTES_PER_SECTION;
	if (noSections > 99) {
	    pem_lines = ((fileLen + LINE_LEN - 1) / LINE_LEN + 98) / 99;
	    noSections = (fileLen + BYTES_PER_SECTION - 1) /
		BYTES_PER_SECTION;
	    fprintf(pgpout,
	    "value for \"armorlines\" is too low, using %d\n", pem_lines);
	}
    }

    if (clearfilename)
      tempf = tempfile(TMP_WIPE);
    else
      tempf = outfilename;

    if (outfilename == NULL) {
	outFile = stdout;
    } else {
	if (noSections > 1) {
            do {
                char *t;
                force_extension(outfilename, ASC_EXTENSION);
                strcpy(outfilename, numFilename(outfilename, 1, noSections));
                if (!file_exists(outfilename)) break;
                t = ck_dup_output(outfilename, TRUE, TRUE);
                if (t==NULL) user_error();
                strcpy(outfilename,t);
            } while (TRUE);
            outFile = fopen(tempf, FOPWTXT);
	} else
	    outFile = fopen(tempf, FOPWTXT);
#ifdef MACTC5
		strcpy(curOutFile,outfilename);
#endif
    }

    if (outFile == NULL) {
	fclose(inFile);
	return 1;
    }
    if (clearfilename) {
	if ((clearFile = fopen(clearfilename, FOPRTXT)) == NULL) {
	    fclose(inFile);
	    if (outFile != stdout)
		fclose(outFile);
	    return 1;
	}
	fprintf(outFile, "-----BEGIN PGP SIGNED MESSAGE-----\n\n");
	while ((i = pgp_getline(buffer, sizeof buffer, clearFile)) >= 0) {
	    /* Quote lines beginning with '-' as per RFC1113;
	     * Also quote lines beginning with "From "; this is
	     * for Unix mailers which add ">" to such lines.
	     */
	    if (buffer[0] == '-' || strncmp(buffer, "From ", 5) == 0)
		fputs("- ", outFile);
	    fputs(buffer, outFile);
	    /* If there is more on this line, copy it */
	    if (i == 0)
		if (copyline(clearFile, outFile) <= 0)
		    break;
	    fputc('\n', outFile);
	}
	fclose(clearFile);
	putc('\n', outFile);
	blocktype = "SIGNATURE";
    }
    if (noSections == 1) {
	byte ctb = 0;
        int keycounter = 0;
        int status;
	ctb = getc(inFile);
	if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE)) {
	    blocktype = "PUBLIC KEY BLOCK";
            if (kv_label) {
                kv_title(outFile);     /* Title line */
                rewind(inFile);        /* Back over CTB */
                status = kvformat_keypacket(inFile, outFile, TRUE, "", infilename,
                                            FALSE, FALSE, &keycounter);
	        fprintf(outFile, "\n");
            }
	} else if (is_ctb_type(ctb, CTB_CERT_SECKEY_TYPE)) {
            blocktype = "SECRET KEY BLOCK";
            if (kv_label) {
                kv_title(outFile);     /* Title line */
                rewind(inFile);        /* Back over CTB */
                status = kvformat_keypacket(inFile, outFile, TRUE, "", infilename,
                                            FALSE, FALSE, &keycounter);
	        fprintf(outFile, "\n");
            }
	}
	fprintf(outFile, "-----BEGIN PGP %s-----\n", blocktype);
	rewind(inFile);
    } else {
	fprintf(outFile,
		"-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n",
		1, noSections);
    }
    fprintf(outFile, "Version: %s\n", LANG(rel_version));
    if (clearfilename)
	fprintf(outFile, "Charset: %s\n", charset);
    if (globalCommentString[0])
	fprintf(outFile, "Comment: %s\n", globalCommentString);
    fprintf(outFile, "\n");

    init_crc();
    crc = CRCINIT;

    while ((bytesRead = fread(buffer, 1, LINE_LEN, inFile)) > 0) {
	/* Munge up LINE_LEN characters */
	if (bytesRead < LINE_LEN)
	    fill0(buffer + bytesRead, LINE_LEN - bytesRead);

	crc = crcbytes((byte *) buffer, bytesRead, crc);
	for (i = 0; i < bytesRead - 3; i += 3)
	    outdec(buffer + i, outFile, 3);
	outdec(buffer + i, outFile, bytesRead - i);
	putc('\n', outFile);
#ifdef MACTC5
		mac_poll_for_break();
#endif

	if (++lines == pem_lines && currentSection < noSections) {
	    lines = 0;
	    outcrc(crc, outFile);
	    fprintf(outFile,
		    "-----END PGP MESSAGE, PART %02d/%02d-----\n\n",
		    currentSection, noSections);
	    if (write_error(outFile)) {
		fclose(outFile);
		return -1;
 	    }
	    fclose(outFile);
#ifdef MACTC5
		PGPSetFinfo(curOutFile,'TEXT','MPGP');
#endif
	    outFile = fopen(numFilename(outfilename,
					++currentSection,
					noSections), FOPWTXT);
#ifdef MACTC5
		strcpy(curOutFile,numFilename (outfilename,currentSection,noSections));
#endif
	    if (outFile == NULL) {
		fclose(inFile);
		return -1;
	    }
	    fprintf(outFile,
		    "-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n",
		    currentSection, noSections);
	    fprintf(outFile, "\n");
	    crc = CRCINIT;
	}
    }
    outcrc(crc, outFile);

    if (noSections == 1)
	fprintf(outFile, "-----END PGP %s-----\n", blocktype);
    else
	fprintf(outFile, "-----END PGP MESSAGE, PART %02d/%02d-----\n",
		noSections, noSections);

    /* Done */
    fclose(inFile);
    rc = write_error(outFile);
    if (outFile == stdout)
	return rc;
#ifdef MACTC5
	PGPSetFinfo(curOutFile,'TEXT','MPGP');
#endif
    fclose(outFile);
    if (clearfilename) {
        remove(outfilename);
        savetemp(tempf,outfilename);
    }

    if (rc)
	return -1;

    if (clearfilename) {
	fprintf(pgpout,
		LANG("\nClear signature file: %s\n"), outfilename);
    } else if (noSections == 1) {
	fprintf(pgpout,
		LANG("\nTransport armor file: %s\n"), outfilename);
    } else {
	fprintf(pgpout, LANG("\nTransport armor files: "));
	for (i = 1; i <= noSections; ++i)
	    fprintf(pgpout, "%s%s",
		    numFilename(outfilename, i, noSections),
		    i == noSections ? "\n" : ", ");
    }
    return 0;
}				/* armor_file */

/*      End ASCII armor encode routines. */


/*
 * ASCII armor decode routines.
 */
static int
darmor_buffer(char *inbuf, char *outbuf, int *outlength)
{
    unsigned char *bp;
    int length;
    unsigned int c1, c2, c3, c4;
    register int j;

    length = 0;
    bp = (unsigned char *) inbuf;

    /* FOUR input characters go into each THREE output charcters */

    while (*bp != '\0') {
#ifdef EBCDIC
	if ((c1 = asctobin[*bp]) & 0x80)
	    return -1;
	++bp;
	if ((c2 = asctobin[*bp]) & 0x80)
	    return -1;
#else
	if (*bp & 0x80 || (c1 = asctobin[*bp]) & 0x80)
	    return -1;
	++bp;
	if (*bp & 0x80 || (c2 = asctobin[*bp]) & 0x80)
	    return -1;
#endif
	if (*++bp == PAD) {
	    c3 = c4 = 0;
	    length += 1;
	    if (c2 & 15)
		return -1;
	    if (strcmp((char *) bp, "==") == 0)
		bp += 1;
	    else if (strcmp((char *) bp, "=3D=3D") == 0)
		bp += 5;
	    else
		return -1;
#ifdef EBCDIC
	} else if ((c3 = asctobin[*bp]) & 0x80) {
#else
	} else if (*bp & 0x80 || (c3 = asctobin[*bp]) & 0x80) {
#endif
	    return -1;
	} else {
	    if (*++bp == PAD) {
		c4 = 0;
		length += 2;
		if (c3 & 3)
		    return -1;
		if (strcmp((char *) bp, "=") == 0);	/* All is well */
		else if (strcmp((char *) bp, "=3D") == 0)
		    bp += 2;
		else
		    return -1;
#ifdef EBCDIC
	    } else if ((c4 = asctobin[*bp]) & 0x80) {
#else
	    } else if (*bp & 0x80 || (c4 = asctobin[*bp]) & 0x80) {
#endif
		return -1;
	    } else {
		length += 3;
	    }
	}
	++bp;
	j = (c1 << 2) | (c2 >> 4);
	*outbuf++ = j;
	j = (c2 << 4) | (c3 >> 2);
	*outbuf++ = j;
	j = (c3 << 6) | c4;
	*outbuf++ = j;
    }

    *outlength = length;
    return 0;			/* normal return */

}				/* darmor_buffer */

static char armorfilename[MAX_PATH];
/*
 * try to open the next file of a multi-part armored file
 * the sequence number is expected at the end of the file name
 */
static FILE *
open_next(void)
{
    char *p, *s, c;
    FILE *fp;

    p = armorfilename + strlen(armorfilename);
    while (--p >= armorfilename && isdigit(*p)) {
	if (*p != '9') {
	    ++*p;
	    return fopen(armorfilename, FOPRARMOR);
	}
	*p = '0';
    }

    /* need an extra digit */
    if (p >= armorfilename) {
	/* try replacing character ( .as0 -> .a10 ) */
	c = *p;
	*p = '1';
	if ((fp = fopen(armorfilename, FOPRARMOR)) != NULL)
	    return fp;
	*p = c;			/* restore original character */
    }
    ++p;
    for (s = p + strlen(p); s >= p; --s)
	s[1] = *s;
    *p = '1';			/* insert digit ( fn0 -> fn10 ) */

#if defined(MSDOS) && !defined(BUG)
    /* if the resulting filename has more than three
       characters after the first dot, don't even try to open it */
    s = strchr(armorfilename, '.');
    if (s != NULL)
       if (strlen(s) > 3)
          return NULL;
#endif /* MSDOS */

    return fopen(armorfilename, FOPRARMOR);
}

/*
 * Returns -1 if the line given is does not begin as a valid ASCII
 * armor header line (something of the form "Label: ", where "Label"
 * must begin with a letter followed by letters, numbers, or hyphens,
 * followed immediately by a colon and a space), 0 if it is a familiar
 * label, and the length of the label if it is an unfamiliar label
 * (E.g. not "Version" or "Comment");
 */
static int
isheaderline(char const *buf)
{
	int i;

	if (!isalpha(*buf))
		return -1;	/* Not a label */

	for (i = 1; isalnum(buf[i]) || i == '-'; i++)
		;
	if (buf[i] != ':' || buf[i+1] != ' ')
		return -1;	/* Not a label */

	if (memcmp(buf, "Charset", i) == 0) {
		if (use_charset_header) strcpy(charset_header,buf+9);
		return 0;
	}
	if (memcmp(buf, "Version", i) == 0 ||
	    memcmp(buf, "Comment", i) == 0)
		return 0;	/* Familiar label */
	return i;	/* Unfamiliar label */
}

/* 
 * Skips a bunch of headers, either returning 0, or printing
 * an error message and returning -1.
 * If it encounters an unfamiliar label and *warned is not set,
 * prints a warning and sets *warned.
 * NOTE that file read errors are NOT printed or reported in the
 * return code.  It is assumed that the following read will
 * notice the error and do something appropriate.
 */
static int
skipheaders(FILE *in, char *buf, int *warned, int armorfollows)
{
    int label_seen = 0;
    int i;
#ifndef STRICT_ARMOR	/* Allow no space */
    long fpos;
    char outbuf[(MAX_LINE_SIZE*3+3)/4];
    int n;
#endif

    for (;;) {
	++infile_line;
#ifndef STRICT_ARMOR
	fpos = ftell(in);
#endif
	if (get_armor_line(buf, in) == NULL)	/* Error */
	    return 0;	/* See comment above */
	if (buf[0] == '\0')	/* Blank line */
	    return 0;	/* Success */
	if (label_seen && (buf[0] == ' ' || buf[0] == '\t'))
	    continue;	/* RFC-822-style continuation line */
	i = isheaderline(buf);
	if (i < 0) {	/* Not a legal header line */
#ifndef STRICT_ARMOR	/* If it's as ASCII armor line, accept it */
	    if (armorfollows && darmor_buffer(buf, outbuf, &n) == 0 && n == 48)
	    {
		fseek(in, fpos, SEEK_SET);
		--infile_line;
		return 0;	/* Consider this acceptable */
	    }
#else
	    (void)armorfollows;	/* Stop compiler complaints */
#endif
	    fprintf(pgpout,
LANG("Invalid ASCII armor header line: \"%.40s\"\n\
ASCII armor corrupted.\n"), buf);
	    return -1;
	}
	if (i > 0 && !*warned) {
		fprintf(pgpout,
LANG("Warning: Unrecognized ASCII armor header label \"%.*s:\" ignored.\n"),
		        i, buf);
		*warned = 1;
	}
	label_seen = 1;	/* Continuation lines are now legal */
    }
}

/*
 * Copy from in to out, decoding as you go, with handling for multiple
 * 500-line blocks of encoded data.  This function also knows how to
 * go past the end of one part to the beginning of the next in a multi-part
 * file.  (As you can see from some ugliness below, this is not the best
 * place to do it, since the caller is responsible for closing the
 * "original_in" file.)
 */
static int
armordecode(FILE *original_in, FILE *out, int *warned)
{
    char inbuf[MAX_LINE_SIZE];
    char outbuf[MAX_LINE_SIZE];

    int i, n, status;
    int line;
    int section, currentSection = 1;
    int noSections = 0;
    int gotcrc = 0;
    long crc = CRCINIT, chkcrc = -1;
    char crcbuf[4];
    int ret_code = 0;
    int end_of_message;
    FILE *in = original_in;

    init_crc();

    for (line = 1;; line++) {	/* for each input line */
	if (get_armor_line(inbuf, in) == NULL) {
	    end_of_message = 1;
	} else {
	    end_of_message =
		(strncmp(inbuf, "-----END PGP MESSAGE,", 21) == 0);
	    ++infile_line;
	}

	if (currentSection != noSections && end_of_message) {
	    /* End of this section */
	    if (gotcrc) {
		if (chkcrc != crc) {
		    fprintf(pgpout,
 LANG("ERROR: Bad ASCII armor checksum in section %d.\n"), currentSection);
/* continue with decoding to see if there are other bad parts */
		    ret_code = -1;
		}
	    }
	    gotcrc = 0;
	    crc = CRCINIT;
	    section = 0;

	    /* Try and find start of next section */
	    do {
		if (get_armor_line(inbuf, in) == NULL) {
		    if (in != original_in)
			fclose(in);
		    if ((in = open_next()) != NULL)
			continue;	/* Keep working on new in */
		    fprintf(pgpout,
		    LANG("Can't find section %d.\n"), currentSection + 1);
		    return -1;
		}
		++infile_line;
	    }
	    while (strncmp(inbuf, "-----BEGIN PGP MESSAGE", 22));

	    /* Make sure this section is the correct one */
	    if (2 != sscanf(inbuf,
			    "-----BEGIN PGP MESSAGE, PART %d/%d",
			    &section, &noSections)) {
		fprintf(pgpout,
			LANG("Badly formed section delimiter, part %d.\n"),
			currentSection + 1);
		goto error;
	    }
	    if (section != ++currentSection) {
		fprintf(pgpout,
LANG("Sections out of order, expected part %d"), currentSection);
		if (section)
		    fprintf(pgpout,
			    LANG(", got part %d\n"), section);
		else
		    fputc('\n', pgpout);
		goto error;
	    }
	    /* Skip header after BEGIN line */
	    if (skipheaders(in, inbuf, warned, 1) < 0)
		goto error;
	    if (feof(in)) {
		fprintf(pgpout,
		   LANG("ERROR: Hit EOF in header of section %d.\n"),
			currentSection);
		goto error;
	    }
		
	    /* Continue decoding */
	    continue;
	}
#ifdef MACTC5
	mac_poll_for_break();
#endif

/* Quit when hit the -----END PGP MESSAGE----- line or a blank,
 * or handle checksum
 */
	if (inbuf[0] == PAD) {	/* Checksum lines start
				   with PAD char */
	    /* If the already-armored file is sent through MIME
	     * and gets armored again, '=' will become '=3D'.
	     * To make life easier, we detect and work around this
	     * idiosyncracy.
	     */
	    if (strlen(inbuf) == 7 &&
		inbuf[1] == '3' && inbuf[2] == 'D')
		status = darmor_buffer(inbuf + 3, crcbuf, &n);
	    else
		status = darmor_buffer(inbuf + 1, crcbuf, &n);
	    if (status < 0 || n != 3) {
		fprintf(pgpout,
LANG("ERROR: Badly formed ASCII armor checksum, line %d.\n"), line);
                goto error;
	    }
	    chkcrc = (((long) crcbuf[0] << 16) & 0xff0000L) +
		((crcbuf[1] << 8) & 0xff00L) + (crcbuf[2] & 0xffL);
	    gotcrc = 1;
	    continue;
	}
	if (inbuf[0] == '\0') {
	    fprintf(pgpout,
		    LANG("WARNING: No ASCII armor `END' line.\n"));
	    break;
	}
	if (strncmp(inbuf, "-----END PGP ", 13) == 0)
	    break;

	status = darmor_buffer(inbuf, outbuf, &n);

	if (status == -1) {
	    fprintf(pgpout,
	     LANG("ERROR: Bad ASCII armor character, line %d.\n"), line);
	    gotcrc = 1;		/* this will print part number,
				   continue with next part */
	    ret_code = -1;
	}
	if (n > sizeof outbuf) {
	    fprintf(pgpout,
	     LANG("ERROR: Bad ASCII armor line length %d on line %d.\n"),
		    n, line);
	    goto error;
	}
	crc = crcbytes((byte *) outbuf, n, crc);
	if (fwrite(outbuf, 1, n, out) != n) {
	    ret_code = -1;
	    break;
	}
    }				/* line */

    if (gotcrc) {
	if (chkcrc != crc) {
	    fprintf(pgpout,
		    LANG("ERROR: Bad ASCII armor checksum"));
	    if (noSections > 0)
		fprintf(pgpout,
			LANG(" in section %d"), noSections);
	    fputc('\n', pgpout);
	    goto error;
	}
    } else {
	fprintf(pgpout,
		LANG("Warning: Transport armor lacks a checksum.\n"));
    }

    if (in != original_in)
	fclose(in);
    return ret_code;		/* normal return */
error:
    if (in != original_in)
	fclose(in);
    return -1;			/* error return */
}				/* armordecode */

static boolean
is_armorfile(char *infile)
{
    FILE *in;
    char inbuf[MAX_LINE_SIZE];
    char outbuf[MAX_LINE_SIZE];
    int n;
    long il;

    in = fopen(infile, FOPRARMOR);
    if (in == NULL)
	return FALSE;	/* can't open file */
   
    /* Read to infile_line before we begin looking */
    for (il = 0; il < infile_line; ++il) {
	if (get_armor_line(inbuf, in) == NULL) {
	    fclose(in);
	    return FALSE;
	}
    }

    /* search file for delimiter line */
    for (;;) {
	if (get_armor_line(inbuf, in) == NULL)
	    break;
	if (strncmp(inbuf, "-----BEGIN PGP ", 15) == 0) {
	    if (strncmp(inbuf,
		    "-----BEGIN PGP SIGNED MESSAGE-----", 34) == 0) {
		fclose(in);
		return TRUE;
	    }
	    n = 1;	/* Don't print warnings yet */
	    if (skipheaders(in, inbuf, &n, 1) < 0 ||
	        get_armor_line(inbuf, in) == NULL ||
	        darmor_buffer(inbuf, outbuf, &n) < 0)
		break;
	    fclose(in);
	    return TRUE;
	}
    }

    fclose(in);
    return FALSE;
}				/* is_armorfile */

static int
darmor_file(char *infile, char *outfile)
{
    FILE *in, *out;
    char buf[MAX_LINE_SIZE];
    char outbuf[(MAX_LINE_SIZE*3+3)/4];
    int status, n;
    long il, fpos;
    char *litfile = NULL;
    int header_warned = 0;	/* Complained about unknown header */

    if ((in = fopen(infile, FOPRARMOR)) == NULL) {
	fprintf(pgpout, LANG("ERROR: Can't find file %s\n"), infile);
	return 10;
    }
    strcpy(armorfilename, infile);	/* store filename for multi-parts */

    /* Skip to infile_line */
    for (il = 0; il < infile_line; ++il) {
	if (get_armor_line(buf, in) == NULL) {
	    fclose(in);
	    return -1;
	}
    }

    /* Loop through file, searching for delimiter.  Decode anything with a
       delimiter, complain if there were no delimiter. */

    /* search file for delimiter line */
    for (;;) {
	++infile_line;
	if (get_armor_line(buf, in) == NULL) {
	    fprintf(pgpout,
		    LANG("ERROR: No ASCII armor `BEGIN' line!\n"));
	    fclose(in);
	    return 12;
	}
	if (strncmp(buf, "-----BEGIN PGP ", 15) == 0)
	    break;
    }
    if (strncmp(buf, "-----BEGIN PGP SIGNED MESSAGE-----", 34) == 0) {
	FILE *litout;
	char *p;
	int nline;

	/*
	 * It would be nice to allow headers here, as we could add
	 * additional information to PGP messages, but it appears to
	 * be too easy to spoof, given standard text viewers.  So,
	 * forbid it outright until we sit down and figure out how to
	 * thwart all the ways of faking an end-of-headers.  The
	 * possibilities are:
	 * - Enough trailing whitespace on a valid-looking line to force a
	 *   line wrap.  The 80 column case is tricky, as the classical
	 *   Big Iron IBM mainframe pads to 80 columns, and some terminal
	 *   and text viewer combinations cause a blank line, while others
	 *   don't.  A line that is exactly 80 columns wide but ends in
	 *   a non-blank would do, too.
	 * - A big pile of whitespace within a line, enough to 
	 *   produce something that looks like a blank line between
	 *   the beginning and end parts.
	 * - Various cursor-control sequences.
	 * Basically, it's a nasty problem.  A very strong case can be made
	 * for the argument that it's the text viewer's problem, and outside
	 * PGP's jurisdiction, but that has a few conflicts with reality.
	 */
	charset_header[0] = '\0';
	if (get_armor_line(buf, in) == NULL) {
		fprintf(pgpout,
LANG("ERROR: ASCII armor decode input ended unexpectedly!\n"));
		fclose(in);
		return 12;
	}
	if (buf[0] != '\0') {
		fprintf(pgpout,
LANG("ERROR: Header line added to ASCII armor: \"%s\"\n\
ASCII armor corrupted.\n"), buf);
		fclose(in);
		return -1;
		
	}
		
	litfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	if ((litout = fopen(litfile, FOPWTXT)) == NULL) {
	    fprintf(pgpout,
LANG("\n\007Unable to write ciphertext output file '%s'.\n"), litfile);
	    fclose(in);
	    return -1;
	}

	status = 0;
	for (;;) {
	    ++infile_line;
	    nline = status;
	    status = pgp_getline(buf, sizeof buf, in);
	    if (status < 0) {
		fprintf(pgpout,
LANG("ERROR: ASCII armor decode input ended unexpectedly!\n"));
		fclose(in);
		fclose(litout);
		rmtemp(litfile);
		return 12;
	    }
	    if (strncmp(buf, "-----BEGIN PGP ", 15) == 0)
		break;
	    if (nline)
		putc('\n', litout);
	    /* De-quote lines starting with '- ' */
	    fputs(buf + ((buf[0] == '-' && buf[1] == ' ') ? 2 : 0), litout);
	    /* Copy trailing part of line, if any. */
	    if (!status)
		status = copyline(in, litout);
	    /* Ignore error; pgp_getline will discover it again */
	}
	fflush(litout);
	if (ferror(litout)) {
	    fclose(litout);
	    fclose(in);
	    rmtemp(litfile);
	    return -1;
	}
	fclose(litout);
    }
    /* Skip header after BEGIN line */
    if (skipheaders(in, buf, &header_warned, 1) < 0) {
	fclose(in);
	return -1;
    }
    if (feof(in)) {
	fprintf(pgpout, LANG("ERROR: Hit EOF in header.\n"));
	fclose(in);
	return 13;
    }

    if ((out = fopen(outfile, FOPWBIN)) == NULL) {
	fprintf(pgpout,
LANG("\n\007Unable to write ciphertext output file '%s'.\n"), outfile);
	fclose(in);
	return -1;
    }
    status = armordecode(in, out, &header_warned);

    if (litfile) {
	char *canonfile, hold_charset[16];
	char lit_mode = MODE_TEXT;
	word32 dummystamp = 0;
	FILE *f;

	/* Convert clearsigned message to internal character set */
	canonfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	strip_spaces = TRUE;
        if (charset_header[0]) {
            strcpy(hold_charset, charset);
            strcpy(charset, charset_header);
            init_charset();
        }
	make_canonical(litfile, canonfile);
	rmtemp(litfile);
	litfile = canonfile;
        if (charset_header[0]) {
            strcpy(charset, hold_charset);
            init_charset();
	}
	/* Glue the literal file read above to the signature */
        f = fopen(litfile, FOPRBIN);

	write_ctb_len(out, CTB_LITERAL2_TYPE, fsize(f) + 6, FALSE);
	fwrite(&lit_mode, 1, 1, out);	/* write lit_mode */
	fputc('\0', out);	/* No filename */
	fwrite(&dummystamp, 1, sizeof(dummystamp), out);
	/* dummy timestamp */
	copyfile(f, out, -1L);	/* Append literal file */
	fclose(f);
	rmtemp(litfile);
    }
    if (write_error(out))
	status = -1;
    fclose(out);
    fclose(in);
    return status;
}				/* darmor_file */

/* Entry points for generic interface names */

int de_armor_file(char *infile, char *outfile, long *curline)
{
    int status;

    if (verbose)
	fprintf(pgpout,
	     "de_armor_file: infile = %s, outfile = %s, curline = %ld\n",
		infile, outfile, *curline);
    infile_line = (curline ? *curline : 0);
    status = darmor_file(infile, outfile);
    if (curline)
	*curline = infile_line;
    return status;
}

boolean
is_armor_file(char *infile, long startline)
{
    infile_line = startline;
    return is_armorfile(infile);
}
