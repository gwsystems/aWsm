/*	C include file for MPI library I/O routines

	(c) Copyright 1986 by Philip Zimmermann.  All rights reserved.
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

	NOTE:  This assumes previous inclusion of "mpilib.h"
*/

/*--------------------- Byte ordering stuff -------------------*/

/* XLOWFIRST is defined iff external file format is LSB-first byteorder */
/* #define XLOWFIRST */ /* defined if external byteorder is LSB-first */

#ifdef NEEDSWAP
#undef NEEDSWAP	/* make sure NEEDSWAP is initially undefined */
#endif

/* Assume MSB external byte ordering */
#ifndef HIGHFIRST
#define NEEDSWAP /* External/internal byteorder differs, need byte swap */
#endif


word16 fetch_word16(byte *buf);
/*	Fetches a 16-bit word from where byte pointer is pointing.
	buf points to external-format byteorder array. */

byte *put_word16(word16 w, byte *buf);
/*	Puts a 16-bit word to where byte pointer is pointing, and 
	returns updated byte pointer.
	buf points to external-format byteorder array. */

word32 fetch_word32(byte *buf);
/*	Fetches a 32-bit word from where byte pointer is pointing.
	buf points to external-format byteorder array. */

byte *put_word32(word32 w, byte *buf);
/*	Puts a 32-bit word to where byte pointer is pointing, and 
	returns updated byte pointer.
	buf points to external-format byteorder array. */

/*	Note that convert_byteorder does nothing if internal native 
	byteorder is already the same as external byteorder. */

#ifdef NEEDSWAP /* External/internal byteorder differs, need byte swap */
#define convert_byteorder(buf,bytecount) hiloswap(buf,bytecount)
#define mp_convert_order(r) hiloswap(r,units2bytes(global_precision))
#else
#define convert_byteorder(buf,bytecount)	/* nil statement */
#define mp_convert_order(r)	/* nil statement */
#endif	/* not NEEDSWAP */

/*------------------ End byte ordering stuff -------------------*/

#include <string.h>

#ifdef EMBEDDED
int putchar(int c);		/* standard C library function from <stdio.h> */
#endif	/* EMBEDDED */

int string_length(char *s);
	/* Returns string length */

int str2reg(unitptr reg,string digitstr);
	/* Converts a possibly-signed digit string into a large binary number.
	   Returns assumed radix, derived from suffix 'h','o',b','.' */

int display_in_base(string s,unitptr n,short radix);
	/* Display n in any base, such as base 10.  Returns number
	   of digits. */

void mp_display(string s,unitptr r);
	/* Display register r in hex, with prefix string s. */

word16 checksum(register byteptr buf, register word16 count);
	/* Returns checksum of buffer. */

void cbc_xor(register unitptr dst, register unitptr src, word16 bytecount);
	/* Performs the XOR necessary for RSA Cipher Block Chaining. */

void hiloswap(byteptr r1,short numbytes);
	/* Reverses the order of bytes in an array of bytes. */

short mpi2reg(register unitptr r, register byteptr buf);
	/* Converts to unit array from byte array with bit length prefix
	   word. */

short reg2mpi(register byteptr buf, register unitptr r);
	/* Converts from unit array to byte array with bit length prefix
	   word. */

/****************** end of MPI I/O library ************************/

