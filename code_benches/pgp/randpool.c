/*
 * True random number computation and storage
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
 * Written by Colin Plumb.
 */
#include <stdlib.h>
#include <string.h>

#include "randpool.h"
#include "usuals.h"
#include "md5.h"
#ifdef MACTC5
#include "TimeManager.h"
#endif

/* The pool must be a multiple of the 16-byte (128-bit) MD5 block size */
#define RANDPOOLWORDS ((RANDPOOLBITS+127 & ~127) >> 5)
#if RANDPOOLWORDS <= 16
/* #error is not portable, this has the same effect */
#include "Random pool too small - please increase RANDPOOLBITS in randpool.h"
#endif

/* Must be word-aligned, so make it words.  Cast to bytes as needed. */
static word32 randPool[RANDPOOLWORDS];	/* Random pool */
static unsigned randPoolGetPos = sizeof(randPool); /* Position to get from */
static unsigned randPoolAddPos = 0;	/* Position to add to */

static void
xorbytes(byte *dest, byte const *src, unsigned len)
{
	while (len--)
		*dest++ ^= *src++;
}

/*
 * Destroys already-used random numbers.  Ensures no sensitive data
 * remains in memory that can be recovered later.  This is also
 * called to "stir in" newly acquired environmental noise bits before
 * removing any random bytes.
 *
 * The transformation is carried out by "encrypting" the data in CFB
 * mode with MD5 as the block cipher.  Then, to make certain the stirring
 * operation is strictly one-way, we destroy the key, getting 64 bytes
 * from the beginning of the pool and using them to reinitialize the
 * key.  These bytes are not returned by randPoolGetBytes().
 *
 * The stirring operation is done twice, to ensure that each bit in the
 * pool depends on each bit of entropy XORed in after each call to
 * randPoolStir().
 *
 * To make this useful for pseudo-random (that is, repeatable) operations,
 * the MD5 transformation is always done with a consistent byte order.
 * MD5Transform itself works with 32-bit words, not bytes, so the pool,
 * usually an array of bytes, is transformed into an array of 32-bit words,
 * taking each group of 4 bytes in big-endian order.  At the end of the
 * stirring, the transformation is reversed.
 */
void
randPoolStir(void)
{
	int i;
	byte *p;
	word32 t;
	word32 iv[4];
	static word32 randPoolKey[16] = {0};

	/* Convert to word32s for stirring operation */
	p = (byte *)randPool;
	for (i = 0; i < RANDPOOLWORDS; i++) {
		t = (word32)((unsigned)p[3]<<8 | p[2]) << 16 |
		             (unsigned)p[1]<<8 | p[0];
		randPool[i] = t;
		p += 4;
	}

	/* Start IV from last block of randPool */
	memcpy(iv, randPool+RANDPOOLWORDS-4, sizeof(iv));

	/* First CFB pass */
	for (i = 0; i < RANDPOOLWORDS; i += 4) {
		MD5Transform(iv, randPoolKey);
		iv[0] = randPool[i  ] ^= iv[0];
		iv[1] = randPool[i+1] ^= iv[1];
		iv[2] = randPool[i+2] ^= iv[2];
		iv[3] = randPool[i+3] ^= iv[3];
	}

	/* Get new key */
	memcpy(randPoolKey, randPool, sizeof(randPoolKey));

	/* Second CFB pass */
	for (i = 0; i < RANDPOOLWORDS; i += 4) {
		MD5Transform(iv, randPoolKey);
		iv[0] = randPool[i  ] ^= iv[0];
		iv[1] = randPool[i+1] ^= iv[1];
		iv[2] = randPool[i+2] ^= iv[2];
		iv[3] = randPool[i+3] ^= iv[3];
	}

	/* Get new key */
	memcpy(randPoolKey, randPool, sizeof(randPoolKey));

	/* Wipe iv from memory */
	memset(iv, 0, sizeof(iv));

	/* Convert randPool back to bytes for further use */
	p = (byte *)randPool;
	for (i = 0; i < RANDPOOLWORDS; i++) {
		t = randPool[i];
		p[0] = t>>24;
		p[1] = t>>16;
		p[2] = t>>8;
		p[3] = t;
		p += 4;
	}

	/* Set up pointers for future addition or removal of random bytes */
	randPoolAddPos = 0;
	randPoolGetPos = sizeof(randPoolKey);
#ifdef MACTC5
	spinner();
#endif
}

/*
 * Make a deposit of information (entropy) into the pool.  The bits
 * deposited need not have any particular distribution; the stirring
 * operation transformes them to uniformly-distributed bits.
 */
void
randPoolAddBytes(byte const *buf, unsigned len)
{
	unsigned t;

	while (len > (t = sizeof(randPool) - randPoolAddPos)) {
		xorbytes((byte *)randPool+randPoolAddPos, buf, t);
		buf += t;
		len -= t;
		randPoolStir();
	}

	if (len) {
		xorbytes((byte *)randPool+randPoolAddPos, buf, len);
		randPoolAddPos += len;
		randPoolGetPos = sizeof(randPool); /* Force stir on get */
	}
}

/*
 * Withdraw some bits from the pool.  Regardless of the distribution of the
 * input bits, the bits returned are uniformly distributed, although they
 * cannot, of course, contain more Shannon entropy than the input bits.
 */
void
randPoolGetBytes(byte *buf, unsigned len)
{
	unsigned t;

	while (len > (t = sizeof(randPool) - randPoolGetPos)) {
		memcpy(buf, (byte *)randPool+randPoolGetPos, t);
		buf += t;
		len -= t;
		randPoolStir();
	}

#ifdef MACTC5
	spinner();
#endif

	if (len) {
		memcpy(buf, (byte *)randPool+randPoolGetPos, len);
		randPoolGetPos += len;
	}
}

byte
randPoolGetByte(void)
{
	if (randPoolGetPos == sizeof(randPool))
		randPoolStir();

#ifdef MACTC5
	spinner();
#endif

	return (((byte *)randPool)[randPoolGetPos++]);
}
