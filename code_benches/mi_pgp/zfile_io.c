/*---------------------------------------------------------------------------

  file_io.c

  This file contains routines for doing direct input/output, file-related
  sorts of things.  Most of the system-specific code for unzip is contained
  here, including the non-echoing password code for decryption (bottom).

  Modified: 24 Jun 92 - HAJK
  Fix VMS support
  ---------------------------------------------------------------------------*/


#include "zunzip.h"

#if 0
/****************************/
/* Function FillBitBuffer() */
/****************************/

int FillBitBuffer()
{
    /*
     * Fill bitbuf, which is 32 bits.  This function is only used by the
     * READBIT and PEEKBIT macros (which are used by all of the uncompression
     * routines).
     */
    UWORD temp;

    zipeof = 1;
    while (bits_left < 25 && ReadByte(&temp) == 8)
    {
      bitbuf |= (ULONG)temp << bits_left;
      bits_left += 8;
      zipeof = 0;
    }
    return 0;
}

/***********************/
/* Function ReadByte() */
/***********************/

int ReadByte(x)
UWORD *x;
{
    /*
     * read a byte; return 8 if byte available, 0 if not
     */


    if (csize-- <= 0)
	return 0;

    if (incnt == 0) {
	if ((incnt = read(zipfd, (char *) inbuf, INBUFSIZ)) <= 0)
	    return 0;
	/* buffer ALWAYS starts on a block boundary:  */
	inptr = inbuf;
    }
    *x = *inptr++;
    --incnt;
    return 8;
}

#else
/*
 * This function is used only by the NEEDBYTE macro in inflate.c.
 * It fills the buffer and resets the count and pointers for the
 * macro to resume processing.  The count is set to the number of bytes
 * read in minus one, while the pointer is set to the beginning of the
 * buffer.  This is all to make the macro more efficient.
 *
 * In exceptional circumstances, zinflate can read a byte or two past the
 * end of input, so we allow this for a little bit before returning
 * an error.  zinflate doesn't actually use the bytes, so the value
 * is irrelevant.
 *
 * Returns 0 on success, non-zero on error.
 */
#ifdef MACTC5
int eofonce = 0;
#endif

int FillInBuf()
{
#ifndef MACTC5
	static int eofonce = 0;
#endif

	incnt = read(zipfd, (char *)inbuf, INBUFSIZ);

	if (incnt > 0) {	/* Read went okay */
		inptr = inbuf;
		--incnt;
		return 0;
	} else if (incnt == 0 && !eofonce) {	/* Special fudge case */
		eofonce++;
		incnt = 2;
		inptr = inbuf;
		return 0;
	} else {		/* Error */
		return 1;
	}
}
#endif

/**************************/
/* Function FlushOutput() */
/**************************/

int FlushOutput()
{
    /*
     * flush contents of output buffer; return PK-type error code
     */
    int len;

    if (outcnt) {
			len = outcnt;
	    if (write(outfd, (char *) outout, len) != len)
#ifdef MINIX
                if (errno == EFBIG)
                    if (write(fd, outout, len/2) != len/2  ||
                        write(fd, outout+len/2, len/2) != len/2)
#endif /* MINIX */
                {
                    return 50;    /* 50:  disk full */
                }
        outpos += outcnt;
        outcnt = 0;
        outptr = outbuf;
    }
    return (0);                 /* 0:  no error */
}
