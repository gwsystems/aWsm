/*      mpiio.c - C source code for multiprecision integer I/O routines.
   Implemented Nov 86 by Philip Zimmermann
   Last revised 13 Sep 91 by PRZ

   Boulder Software Engineering
   3021 Eleventh Street
   Boulder, CO 80304
   (303) 541-0140

   (c) Copyright 1986-1996 by Philip Zimmermann.  All rights reserved.
   The author assumes no liability for damages resulting from the use
   of this software, even if the damage results from defects in this
   software.  No warranty is expressed or implied.

   These routines are for multiprecision arithmetic I/O functions for
   number-theoretic cryptographic algorithms such as ElGamal,
   Diffie-Hellman, Rabin, or factoring studies for large composite
   numbers, as well as Rivest-Shamir-Adleman (RSA) public key
   cryptography.

   The external data representation for RSA messages and keys that
   some of these library routines assume is outlined in a paper by 
   Philip Zimmermann, "A Proposed Standard Format for RSA Cryptosystems",
   IEEE Computer, September 1986, Vol. 19 No. 9, pages 21-34.
   Some revisions to this data format have occurred since the paper
   was published.
 */

/* #define DEBUG */


#ifndef EMBEDDED		/* not EMBEDDED - not compiling for
				   embedded target */
#include <stdio.h>		/* for printf, etc. */
#else				/* EMBEDDED - compiling for embedded target */
#define NULL (VOID *)0
#endif

#include "mpilib.h"
#include "mpiio.h"
#include "pgp.h"
#ifdef MACTC5
extern int  Putchar(int c);
#undef putchar
#define putchar Putchar
#endif

static void puthexbyte(byte b);	/* Put out byte in ASCII hex via putchar. */
static
void puthexw16(word16 w);	/* Put out 16-bit word in hex,
				   high byte first. */
static
void putstr(string s);		/* Put out null-terminated ASCII
				   string via putchar. */

/*----------------- Following procedures relate to I/O ------------------*/

/* Returns string length, just like strlen() from <string.h> */
int string_length(char *s)
{
    int i;
    i = 0;
    if (s != NULL)
	while (*s++)
	    i++;
    return (i);
}				/* string_length */

#ifdef DEBUG
/* Returns integer 0-15 if c is an ASCII hex digit, -1 otherwise. */
static int ctox(int c)
{
    if ((c >= '0') && (c <= '9'))
	return (c - '0');
    if ((c >= 'a') && (c <= 'f'))
	return ((c - 'a') + 10);
    if ((c >= 'A') && (c <= 'F'))
	return ((c - 'A') + 10);
    return (-1);		/* error -- not a hex digit */
}				/* ctox */

/* Converts a possibly-signed digit string into a large binary number.
   Returns assumed radix, derived from suffix 'h','o',b','.' */
int str2reg(unitptr reg, string digitstr)
{
    unit temp[MAX_UNIT_PRECISION], base[MAX_UNIT_PRECISION];
    int c, i;
    boolean minus = FALSE;
    short radix;		/* base 2-16 */

    mp_init(reg, 0);

    i = string_length(digitstr);
    if (i == 0)
	return (10);		/* empty string, assume radix 10 */
    c = digitstr[i - 1];	/* get last char in string */

    switch (c) {		/* classify radix select suffix character */
    case '.':
	radix = 10;
	break;
    case 'H':
    case 'h':
	radix = 16;
	break;
    case 'O':
    case 'o':
	radix = 8;
	break;
    case 'B':			/* caution! 'b' is a hex digit! */
    case 'b':
	radix = 2;
	break;
    default:
	radix = 10;
	break;
    }

    mp_init(base, radix);
    if ((minus = (*digitstr == '-')) != 0)
	digitstr++;
    while ((c = *digitstr++) != 0) {
	if (c == ',')
	    continue;		/* allow commas in number */
	c = ctox(c);
	if ((c < 0) || (c >= radix))
	    break;		/* scan terminated by any non-digit */
	mp_mult(temp, reg, base);
	mp_move(reg, temp);
	mp_init(temp, c);
	mp_add(reg, temp);
    }
    if (minus)
	mp_neg(reg);
    return (radix);
}				/* str2reg */

#endif				/* DEBUG */

/* These I/O functions, such as putstr, puthexbyte, and puthexw16, 
   are provided here to avoid the need to link in printf from the 
   C I/O library.  This is handy in an embedded application.
   For embedded applications, use a customized putchar function, 
   separately compiled.
 */

/* Put out null-terminated ASCII string via putchar. */
static void putstr(string s)
{
    while (*s)
	putchar(*s++);
}				/* putstr */

/* Put out byte in ASCII hex via putchar. */
static void puthexbyte(byte b)
{
    static char const nibs[] = "0123456789ABCDEF";

    putchar(nibs[b >> 4]);
    putchar(nibs[b & 0x0F]);
}				/* puthexbyte */

/* Put out 16-bit word in hex, high byte first. */
static void puthexw16(word16 w)
{
    puthexbyte((byte) (w >> 8));
    puthexbyte((byte) (w & 0xFF));
}				/* puthexw16 */

#ifdef UNIT32

/* Puts out 32-bit word in hex, high byte first. */
static void puthexw32(word32 lw)
{
    puthexw16((word16) (lw >> 16));
    puthexw16((word16) (lw & 0xFFFFL));
}				/* puthexw32 */

#endif				/* UNIT32 */


#ifdef UNIT8
#define puthexunit(u) puthexbyte(u)
#endif
#ifdef UNIT16
#define puthexunit(u) puthexw16(u)
#endif
#ifdef UNIT32
#define puthexunit(u) puthexw32(u)
#endif

#ifdef DEBUG
int display_in_base(string s, unitptr n, short radix)
/*
 * Display n in any base, such as base 10.  Returns number of digits.
 * s is string to label the displayed register.
 * n is multiprecision integer.
 * radix is base, 2-16. 
 */
{
    char buf[MAX_BIT_PRECISION + (MAX_BIT_PRECISION / 8) + 2];
    unit r[MAX_UNIT_PRECISION], quotient[MAX_UNIT_PRECISION];
    word16 remainder;
    char *bp = buf;
    char minus = FALSE;
    int places = 0;
    int commaplaces;		/* put commas this many digits apart */
    int i;

    /*      If string s is just an ESC char, don't print it.
       It's just to inhibit the \n at the end of the number.
     */
#ifdef EBCDIC
    if ((s[0] != ESC) || (s[1] != '\0'))
#else
    if ((s[0] != '\033') || (s[1] != '\0'))
#endif
	putstr(s);

    if ((radix < 2) || (radix > 16)) {
	putstr("****\n");	/* radix out of range -- show error */
	return (-1);
    }
    commaplaces = (radix == 10 ? 3 : (radix == 16 ? 4 :
			       (radix == 2 ? 8 : (radix == 8 ? 8 : 1))));
    mp_move(r, n);
    if ((radix == 10) && mp_tstminus(r)) {
	minus = TRUE;
	mp_neg(r);		/* make r positive */
    }
    *bp = '\0';
    do {			/* build backwards number string */
	if (++places > 1)
	    if ((places % commaplaces) == 1)
		*++bp = ',';	/* 000,000,000,000 */
	remainder = mp_shortdiv(quotient, r, radix);
	*++bp = "0123456789ABCDEF"[remainder];	/* Isn't C wonderful? */
	mp_move(r, quotient);
    } while (testne(r, 0));
    if (minus)
	*++bp = '-';

    if (commaplaces != 1)
	while ((++places % commaplaces) != 1)
	    *++bp = ' ';	/* pad to line up commas */

    i = string_length(s);
    while (*bp) {
	putchar(*bp);
	++i;
	if ((*bp == ',') || commaplaces == 1)
	    if (i > (72 - commaplaces)) {
		putchar('\n');
		i = string_length(s);
		while (i--)
		    putchar(' ');
		i = string_length(s);
	    }
	bp--;
    }

    /* show suffix character to designate radix */
    switch (radix) {
    case 10:			/* decimal */
	putchar('.');
	break;
    case 16:			/* hex */
	putchar('h');
	break;
    case 8:			/* octal */
	putchar('o');
	break;
    case 2:			/* binary */
	putchar('b');
	break;
    default:			/* nonstandard radix */
	/* printf("(%d)",radix); */ ;
    }

    if ((s[0] == '\033') && (s[1] == '\0'))
	putchar(' ');		/* supress newline */
    else
	putchar('\n');

    fill0((byteptr) buf, sizeof(buf));	/* burn the evidence on the stack... */
    /* Note that local stack arrays r and quotient are now 0 */
    return (places);
}				/* display_in_base */

#endif				/* DEBUG */

/* Display register r in hex, with prefix string s. */
void mp_display(string s, unitptr r)
{
    short precision;
    int i, j;
    putstr(s);
    normalize(r, precision);	/* strip off leading zeros */
    if (precision == 0) {
	putstr(" 0\n");
	return;
    }
    make_msbptr(r, precision);
    i = 0;
    while (precision--) {
	if (!(i++ % (16 / BYTES_PER_UNIT))) {
	    if (i > 1) {
		putchar('\n');
		j = string_length(s);
		while (j--)
		    putchar(' ');
	    }
	}
	puthexunit(*r);
	putchar(' ');
	post_lowerunit(r);
    }
    putchar('\n');
}				/* mp_display */

/* Returns checksum of buffer. */
word16 checksum(register byteptr buf, register word16 count)
{
    word16 cs;
    cs = 0;
    while (count--)
	cs += *buf++;
    return (cs);
}				/* checksum */

/*
 * Performs the XOR necessary for RSA Cipher Block Chaining.
 * The dst buffer ought to have 1 less byte of significance than 
 * the src buffer.  Only the least significant part of the src 
 * buffer is used.  bytecount is the size of a plaintext block.
 */
void cbc_xor(register unitptr dst, register unitptr src, word16 bytecount)
{
    short nunits;		/* units of precision */
    nunits = bytes2units(bytecount) - 1;
    make_lsbptr(dst, global_precision);
    while (nunits--) {
	*dst ^= *post_higherunit(src);
	post_higherunit(dst);
	bytecount -= units2bytes(1);
    }
    /* on the last unit, don't xor the excess top byte... */
    *dst ^= (*src & (power_of_2(bytecount << 3) - 1));
}				/* cbc_xor */

/* Reverses the order of bytes in an array of bytes. */
void hiloswap(byteptr r1, short numbytes)
{
    byteptr r2;
    byte b;
    r2 = &(r1[numbytes - 1]);
    while (r1 < r2) {
	b = *r1;
	*r1++ = *r2;
	*r2-- = b;
    }
}				/* hiloswap */

#define byteglue(lo,hi) ((((word16) hi) << 8) + (word16) lo)

/****	The following functions must be changed if the external byteorder
	changes for integers in PGP packet data.
****/

/*      Fetches a 16-bit word from where byte pointer is pointing.
   buf points to external-format byteorder array.
 */
word16 fetch_word16(byte * buf)
{
    word16 w0, w1;
/* Assume MSB external byte ordering */
    w1 = *buf++;
    w0 = *buf++;
    return (w0 + (w1 << 8));
}				/* fetch_word16 */

/*
 * Puts a 16-bit word to where byte pointer is pointing, and 
 * returns updated byte pointer.
 * buf points to external-format byteorder array.
 */
byte *put_word16(word16 w, byte * buf)
{
/* Assume MSB external byte ordering */
    buf[1] = w & 0xff;
    w = w >> 8;
    buf[0] = w & 0xff;
    return (buf + 2);
}				/* put_word16 */

/*      Fetches a 32-bit word from where byte pointer is pointing.
   buf points to external-format byteorder array.
 */
word32 fetch_word32(byte * buf)
{
    word32 w0, w1, w2, w3;
/* Assume MSB external byte ordering */
    w3 = *buf++;
    w2 = *buf++;
    w1 = *buf++;
    w0 = *buf++;
    return (w0 + (w1 << 8) + (w2 << 16) + (w3 << 24));
}				/* fetch_word32 */

/*      Puts a 32-bit word to where byte pointer is pointing, and 
   returns updated byte pointer.
   buf points to external-format byteorder array.
 */
byte *put_word32(word32 w, byte * buf)
{
/* Assume MSB external byte ordering */
    buf[3] = w & 0xff;
    w = w >> 8;
    buf[2] = w & 0xff;
    w = w >> 8;
    buf[1] = w & 0xff;
    w = w >> 8;
    buf[0] = w & 0xff;
    return (buf + 4);
}				/* put_word32 */

/***	End of functions that must be changed if the external byteorder
	changes for integer fields in PGP packets.
***/

/*
 * Converts a multiprecision integer from the externally-represented 
 * form of a byte array with a 16-bit bitcount in a leading length 
 * word to the internally-used representation as a unit array.
 * Converts to INTERNAL byte order.
 * The same buffer address may be used for both r and buf.
 * Returns number of units in result, or returns -1 on error.
 */
short mpi2reg(register unitptr r, register byteptr buf)
{
    byte buf2[MAX_BYTE_PRECISION];
    word16 bitcount, bytecount, unitcount, zero_bytes, i;

    /* First, extract 16-bit bitcount prefix from first 2 bytes... */
    bitcount = fetch_word16(buf);
    buf += 2;

    /* Convert bitcount to bytecount and unitcount... */
    bytecount = bits2bytes(bitcount);
    unitcount = bytes2units(bytecount);
    if (unitcount > global_precision) {
	/* precision overflow during conversion. */
	return (-1);		/* precision overflow -- error return */
    }
    zero_bytes = units2bytes(global_precision) - bytecount;
/* Assume MSB external byte ordering */
    fill0(buf2, zero_bytes);	/* fill leading zero bytes */
    i = zero_bytes;		/* assumes MSB first */
    while (bytecount--)
	buf2[i++] = *buf++;

    mp_convert_order(buf2);	/* convert to INTERNAL byte order */
    mp_move(r, (unitptr) buf2);
    mp_burn((unitptr) buf2);	/* burn the evidence on the stack */
    return (unitcount);		/* returns unitcount of reg */
}				/* mpi2reg */

/*
 * Converts the multiprecision integer r from the internal form of 
 * a unit array to the normalized externally-represented form of a
 * byte array with a leading 16-bit bitcount word in buf[0] and buf[1].
 * This bitcount length prefix is exact count, not rounded up.
 * Converts to EXTERNAL byte order.
 * The same buffer address may be used for both r and buf.
 * Returns the number of bytes of the result, not counting length prefix.
 */
short reg2mpi(register byteptr buf, register unitptr r)
{
    byte buf1[MAX_BYTE_PRECISION];
    byteptr buf2;
    short bytecount, bc;
    word16 bitcount;
    bitcount = countbits(r);
#ifdef DEBUG
    if (bitcount > MAX_BIT_PRECISION) {
	fprintf(stderr, "reg2mpi: bitcount out of range (%d)\n", bitcount);
	return 0;
    }
#endif
    bytecount = bits2bytes(bitcount);
    bc = bytecount;		/* save bytecount for return */
    buf2 = buf1;
    mp_move((unitptr) buf2, r);
    mp_convert_order(buf2);	/* convert to EXTERNAL byteorder */
/* Assume MSB external byte ordering */
    buf2 += units2bytes(global_precision) - bytecount;
    buf = put_word16(bitcount, buf);	/* store bitcount in external
					   byteorder */

    while (bytecount--)
	*buf++ = *buf2++;

    mp_burn((unitptr) buf1);	/* burn the evidence on the stack */
    return (bc);		/* returns bytecount of mpi, not counting
				   prefix */
}				/* reg2mpi */


#ifdef DEBUG

/* Dump buffer in hex, with string label prefix. */
void dumpbuf(string s, byteptr buf, int bytecount)
{
    putstr(s);
    while (bytecount--) {
	puthexbyte(*buf++);
	putchar(' ');
	if ((bytecount & 0x0f) == 0)
	    putchar('\n');
    }
}				/* dumpbuf */

/*
 * Dump unit array r as a C array initializer, with string label prefix. 
 * Array is dumped in native unit order.
 */
void dump_unit_array(string s, unitptr r)
{
    int unitcount;
    unitcount = global_precision;
    putstr(s);
    putstr("\n{ ");
    while (unitcount--) {
	putstr("0x");
	puthexunit(*r++);
	putchar(',');
	if (unitcount && ((unitcount & 0x07) == 0))
	    putstr("\n  ");
    }
    putstr(" 0};\n");
}				/* dump_unit_array */

#endif				/* ifdef DEBUG */

/************ end of multiprecision integer I/O library *****************/
