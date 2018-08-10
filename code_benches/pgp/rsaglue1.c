/*
 * rsaglue1.c - These functions wrap and unwrap message digests (MDs) and
 * data encryption keys (DEKs) in padding and RSA-encrypt them into
 * multi-precision integers.  This layer of abstraction was introduced
 * to allow the transparent use of either the RSAREF Cryptographic
 * Toolkit from RSA Data Security Inc for the RSA calculations (where
 * the RSA patent applies), or, Philip Zimmermann's mpi library for the
 * RSA calculations.  The rsaglue.c module from PGP version 2.3a performs
 * the same functions as this module, but can be compiled to select the
 * use of mpilib functions instead of RSAREF as the underlying math engine.
 * That version of rsaglue.c would be suitable where the RSA patent does
 * not apply, such as Canada.
 *
 * This file uses MPILIB to perform the actual encryption and decryption.
 * It uses the same PKCS format as RSAREF, although it also accepts an older
 * format used in PGP 2.1.
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
 */

#include <string.h> 	/* for mem*() */
#include "mpilib.h"
#include "mpiio.h"
#include "pgp.h"
#include "rsaglue.h"
#include "random.h"	/* for cryptRandByte() */

/* No RSADSI credit for MPI version */
char signon_legalese[] = "";

/* These functions hide all the internal details of RSA-encrypted
 * keys and digests.  They owe a lot of their heritage to
 * the preblock() and postunblock() routines in mpiio.c.
 */

/* Abstract Syntax Notation One (ASN.1) Distinguished Encoding Rules (DER)
   encoding for RSA/MD5, used in PKCS-format signatures. */
static byte asn_array[] = {	/* PKCS 01 block type 01 data */
	0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
	0x02,0x05,0x05,0x00,0x04,0x10 };
/* This many bytes from the end, there's a zero byte */
#define ASN_ZERO_END 3

int
rsa_public_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr N)
/* Encrypt a DEK with a public key.  Returns 0 on success.
 * <0 means there was an error.
 * -1: Generic error
 * -3: Key too big
 * -4: Key too small
 */
{
	unit temp[MAX_UNIT_PRECISION];
	unsigned int blocksize;
	int i;	/* Temporary, and holds error codes */
	byte *p = (byte *)temp;

	/*
	 * We are building the mpi in place, except for a possible
	 * byte-order swap to little-endian at the end.  Thus, we
	 * need to fill the buffer with leading 0's in the unused
	 * most significant byte positions.
	 */
	blocksize = countbytes(N) - 1;	/* Bytes available for user data */
	for (i = units2bytes(global_precision) - blocksize; i > 0; --i)
		*p++ = 0;
	/*
	 * Both the PKCS and PGP 2.0 key formats add a type byte, and a
	 * a framing byte of 0 to the user data.  The remaining space
	 * is filled with random padding.  (PKCS requires that there be
	 * at least 1 byte of padding.)
	 */
	i = blocksize - 2 - bytes;

	if (i < 1)		/* Less than minimum padding? */
		return -4;
	*p++ = CK_ENCRYPTED_BYTE;	/* Type byte */
	while (i)			/* Non-zero random padding */
		if ((*p = cryptRandByte()))
			++p, --i;
	*p++ = 0;			/* Framing byte */
	memcpy(p, inbuf, bytes);	/* User data */

	mp_convert_order((byte *)temp);		/* Convert buffer to MPI */
	i = mp_modexp(outbuf, temp, E, N);	/* Do the encryption */
	if (i < 0)
		i = -1;

Cleanup:
	mp_burn(temp);
	return i < 0 ? i : 0;
} /* rsa_public_encrypt */

int
rsa_private_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N)
/* Encrypt a message digest with a private key.
 * Returns <0 on error:
 * -1: generic error
 * -4: Key too big
 * -5: Key too small
 */
{
	unit temp[MAX_UNIT_PRECISION];
	unit DP[MAX_UNIT_PRECISION], DQ[MAX_UNIT_PRECISION];
	byte *p;
	int i;
	unsigned int blocksize;

	/* PGP doesn't store these coefficents, so we need to compute them. */
	mp_move(temp,P);
	mp_dec(temp);
	mp_mod(DP,D,temp);
	mp_move(temp,Q);
	mp_dec(temp);
	mp_mod(DQ,D,temp);

	p = (byte *)temp;


	/* We are building the mpi in place, except for a possible
	 * byte-order swap to little-endian at the end.  Thus, we
	 * need to fill the buffer with leading 0's in the unused
	 * most significant byte positions.
	 */
	blocksize = countbytes(N) - 1;	/* Space available for data */
	for (i = units2bytes(global_precision) - blocksize; i > 0; --i)
		*p++ = 0;

	i = blocksize - 2 - bytes;		/* Padding needed */
	i -= sizeof(asn_array);		/* Space for type encoding */
	if (i < 0) {
		i = -4;			/* Error code */
		goto Cleanup;
	}
	*p++ = MD_ENCRYPTED_BYTE;	/* Type byte */
	memset(p, ~0, i);		/* All 1's padding */
	p += i;
	*p++ = 0;			/* Zero framing byte */
	memcpy(p, asn_array, sizeof(asn_array)); /* ASN data */
	p += sizeof(asn_array);
	memcpy(p, inbuf, bytes);	/* User data */

	mp_convert_order((byte *)temp);
	i = mp_modexp_crt(outbuf, temp, P, Q, DP, DQ, U);	/* Encrypt */
	if (i < 0)
		i = -1;

Cleanup:
	burn(temp);

	return i;
} /* rsa_private_encrypt */

/* Remove a signature packet from an MPI */
/* Thus, we expect constant padding and the MIC ASN sequence */
int
rsa_public_decrypt(byteptr outbuf, unitptr inbuf,
	unitptr E, unitptr N)
/* Decrypt a message digest using a public key.  Returns the number of bytes
 * extracted, or <0 on error.
 * -1: Corrupted packet.
 * -3: Key too big
 * -4: Key too small
 * -5: Maybe malformed RSA packet
 * -7: Unknown conventional algorithm
 * -9: Malformed RSA packet
 */
{
	unit temp[MAX_UNIT_PRECISION];
	unsigned int blocksize;
	int i;
	byte *front, *back;

	i = mp_modexp(temp, inbuf, E, N);
	if (i < 0) {
		mp_burn(temp);
		return -1;
	}
	mp_convert_order((byte *)temp);
	blocksize = countbytes(N) - 1;
	front = (byte *)temp;			/* Points to start of block */
	i = units2bytes(global_precision);
	back = front + i;			/* Points to end of block */
	i -= countbytes(N) - 1;			/* Expected leading 0's */

	/*
	 * Strip off the padding.  This handles both PKCS and PGP 2.0
	 * formats.  If we're using RSAREF2, we use the padding-removal
	 * code in RSAPublicDecrypt, which accepts only PKCS style.
	 * Oh, well.
	 */

	if (i < 0)				/* This shouldn't happen */
		goto ErrorReturn;
	while (i--)				/* Extra bytes should be 0 */
		if (*front++)
			goto ErrorReturn;

	/* How to distinguish old PGP from PKCS formats.
	 * The old PGP format ends in a trailing type byte, with
	 * all 1's padding before that.  The PKCS format ends in
	 * 16 bytes of message digest, preceded by an ASN string
	 * which is not all 1's.
	 */
	if (back[-1] == MD_ENCRYPTED_BYTE &&
	    back[-17] == 0xff && back[-18] == 0xff) {
		/* Old PGP format: Padding is at the end */
		if (*--back != MD_ENCRYPTED_BYTE)
			goto ErrorReturn;
		if (*front++ != MD5_ALGORITHM_BYTE) {
			mp_burn(temp);
			return -7;
		}
		while (*--back == 0xff)	/* Skip constant padding */
			;
		if (*back)		/* It should end with a zero */
			goto ErrorReturn;
	} else {
		/* PKCS format: padding at the beginning */
		if (*front++ != MD_ENCRYPTED_BYTE)
			goto ErrorReturn;
		while (*front++ == 0xff) /* Skip constant padding */
			;
		if (front[-1])	/* First non-FF byte should be 0 */
			goto ErrorReturn;
		/* Then comes the ASN header */
		if (memcmp(front, asn_array, sizeof(asn_array))) {
			mp_burn(temp);
			return -7;
		}
		front += sizeof(asn_array);
	}

/* We're done - copy user data to outbuf */
	if (back < front)
		goto ErrorReturn;
	blocksize = back-front;
	memcpy(outbuf, front, blocksize);
	mp_burn(temp);
	return blocksize;
ErrorReturn:
	mp_burn(temp);
	return -9;
} /* rsa_public_decrypt */

/* We expect to find random padding and an encryption key */
int
rsa_private_decrypt(byteptr outbuf, unitptr inbuf,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N)
/* Decrypt an encryption key using a private key.  Returns the number of bytes
 * extracted, or <0 on error.
 * -1: Generic error
 * -3: Key too big
 * -4: Key too small
 * -5: Maybe malformed RSA
 * -7: Unknown conventional algorithm
 * -9: Malformed RSA packet
 */
{
	byte *back;
	byte *front;
	unsigned int blocksize;
	unit temp[MAX_UNIT_PRECISION];
	unit DP[MAX_UNIT_PRECISION], DQ[MAX_UNIT_PRECISION];
	int i;

	/* PGP doesn't store (d mod p-1) and (d mod q-1), so compute 'em */
	mp_move(temp,P);
	mp_dec(temp);
	mp_mod(DP,D,temp);
	mp_move(temp,Q);
	mp_dec(temp);
	mp_mod(DQ,D,temp);

	i = mp_modexp_crt(temp, inbuf, P, Q, DP, DQ, U);
	mp_burn(DP);
	mp_burn(DQ);
	if (i < 0) {
		mp_burn(temp);
		return -1;
	}
	mp_convert_order((byte *)temp);
	front = (byte *)temp;			/* Start of block */
	i = units2bytes(global_precision);
	back = (byte *)front + i;		/* End of block */
	blocksize = countbytes(N) - 1;
	i -= blocksize;				/* Expected # of leading 0's */

	if (i < 0)				/* This shouldn't happen */
		goto Corrupted;
	while (i--)				/* Extra bytes should be 0 */
		if (*front++)
			goto Corrupted;

	/* How to distinguish old PGP from PKCS formats.
	 * PGP packets have a trailing type byte (CK_ENCRYPTED_BYTE),
	 * while PKCS formats have it leading.
	 */
	if (front[0] != CK_ENCRYPTED_BYTE && back[-1] == CK_ENCRYPTED_BYTE) {
		/* PGP 2.0 format  - padding at the end */
		if (back[-1] != CK_ENCRYPTED_BYTE)
			goto Corrupted;
		while (*--back)	/* Skip non-zero random padding */
			;
	} else {
		/* PKCS format - padding at the beginning */
		if (*front++ != CK_ENCRYPTED_BYTE)
			goto Corrupted;
		while (*front++)	/* Skip non-zero random padding */
			;
	}
	if (back <= front)
		goto Corrupted;
	blocksize = back-front;

	memcpy(outbuf, front, blocksize);
	mp_burn(temp);
	return blocksize;

Corrupted:
	mp_burn(temp);
	return -9;
} /* rsa_private_decrypt */
