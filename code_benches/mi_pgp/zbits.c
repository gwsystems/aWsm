/*

 Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup Gailly.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included
 unmodified, that it is not sold for profit, and that this copyright notice
 is retained.

*/

/*
 *  bits.c by Jean-loup Gailly.
 *
 *  This is a new version of im_bits.c originally written by Richard B. Wales
 *
 *  PURPOSE
 *
 *      Output variable-length bit strings.
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflate" file format interprets compressed file data
 *      as a sequence of bits.  Multi-bit strings in the file may cross
 *      byte boundaries without restriction.
 *
 *      The first bit of each byte is the low-order bit.
 *
 *      The routines in this file allow a variable-length bit value to
 *      be output right-to-left (useful for literal values). For
 *      left-to-right output (useful for code strings from the tree routines),
 *      the bits must have been reversed first with bi_reverse().
 *
 *  INTERFACE
 *
 *      void bi_init (FILE *zipfile)
 *          Initialize the bit string routines.
 *
 *      void send_bits (int value, int length)
 *          Write out a bit string, taking the source bits right to
 *          left.
 *
 *      int bi_reverse (int value, int length)
 *          Reverse the bits of a bit string, taking the source bits left to
 *          right and emitting them right to left.
 *
 *      void bi_windup (void)
 *          Write out any remaining bits in an incomplete byte.
 *
 *      void copy_block(char far *buf, unsigned len, int header)
 *          Copy a stored block to the zip file, storing first the length and
 *          its one's complement if requested.
 *
 */

#include "zip.h"

/* ===========================================================================
 * Local data used by the "bit string" routines.
 */

local FILE *zfile; /* output zip file */

local unsigned short bi_buf;
/* Output buffer. bits are inserted starting at the bottom (least significant
 * bits).
 */

#define Buf_size (8 * 2*sizeof(char))
/* Number of bits used within bi_buf. (bi_buf might be implemented on
 * more than 16 bits on some systems.)
 */

local int bi_valid;                  /* number of valid bits in bi_buf */
/* All bits above the last valid bit are always zero.
 */

#ifdef DEBUG
ulg bits_sent;   /* bit length of the compressed data */
#endif

/* Output a 16 bit value to the bit stream, lower (oldest) byte first */
#define PUTSHORT(w) \
{  (void) zputc ((char)((w) & 0xff), zfile); \
   (void) zputc ((char)((ush)(w) >> 8), zfile); \
}

/* Output an 8 bit value to the bit stream, bits right to left */
#define PUTBYTE(w) \
{  (void) zputc ((char)((w) & 0xff), zfile); \
}

/* ===========================================================================
 * Initialize the bit string routines.
 */
void bi_init (zipfile)

    FILE *zipfile;  /* output zip file */
{
    zfile  = zipfile;
    bi_buf = 0;
    bi_valid = 0;
#ifdef DEBUG
    bits_sent = 0L;
#endif
}

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
void send_bits(value, length)
    int value;  /* value to send */
    int length; /* number of bits */
{
#ifdef DEBUG
    Tracevv((stderr," l %2d v %4x ", length, value));
    Assert(length > 0 && length <= 15, "invalid length");
    bits_sent += (ulg)length;
#endif
    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
     * unused bits in value.
     */
    if (bi_valid > (int)Buf_size - length) {
        bi_buf |= (value << bi_valid);
        PUTSHORT(bi_buf);
        bi_buf = (ush)value >> (Buf_size - bi_valid);
        bi_valid += length - Buf_size;
    } else {
        bi_buf |= value << bi_valid;
        bi_valid += length;
    }
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
unsigned bi_reverse(code, len)
    unsigned code; /* the value to invert */
    int len;       /* its bit length */
{
    register unsigned res = 0;
    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

/* ===========================================================================
 * Write out any remaining bits in an incomplete byte.
 */
void bi_windup()
{
    if (bi_valid > 8) {
        PUTSHORT(bi_buf);
    } else if (bi_valid > 0) {
        PUTBYTE(bi_buf);
    }
    bi_buf = 0;
    bi_valid = 0;
    if (ferror (zfile)) error ("write error on zip file");
#ifdef DEBUG
    bits_sent = (bits_sent+7) & ~7;
#endif
}

/* ===========================================================================
 * Copy a stored block to the zip file, storing first the length and its
 * one's complement if requested.
 */
void copy_block(buf, len, header)
    char far *buf; /* the input data */
    unsigned len;  /* its length */
    int header;    /* true if block header must be written */
{
    bi_windup();              /* align on byte boundary */

    if (header) {
	PUTSHORT((ush)len);   
	PUTSHORT((ush)~len);
#ifdef DEBUG
        bits_sent += 2*16;
#endif
    }
    zfwrite(buf, 1, len, zfile); /* ??? far */
#ifdef DEBUG
    bits_sent += (ulg)len<<3;
#endif
	fflush(zfile);
    if (ferror(zfile)) error ("write error on zip file");
}
