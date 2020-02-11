/*      mdfile.c  - Message Digest routines for PGP.
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

#include <stdio.h>
#include "mpilib.h"
#include "mdfile.h"
#include "fileio.h"
#include "language.h"
#include "pgp.h"
#ifdef MACTC5
#include "Macutil3.h"
#endif

/* Begin MD5 routines */

/* Note - the routines in this module, except for MD_addbuffer,
 * do not "finish" the MD5 calculation.  MD_addbuffer finishes the
 * calculation in each case, usually to append the timestamp and class info.
 */

/* Computes the message digest for a file from current position for
   longcount bytes.
   Uses the RSA Data Security Inc. MD5 Message Digest Algorithm */
int MDfile0_len(struct MD5Context *mdContext, FILE * f, word32 longcount)
{
    int bytecount;
    unsigned char buffer[1024];

    MD5Init(mdContext);
    /* Process 1024 bytes at a time... */
    do {
	if (longcount < (word32) 1024)
	    bytecount = (int) longcount;
	else
	    bytecount = 1024;
	bytecount = fread(buffer, 1, bytecount, f);
	if (bytecount > 0) {
	    MD5Update(mdContext, buffer, bytecount);
	    longcount -= bytecount;
#ifdef MACTC5
		mac_poll_for_break();
#endif
	}
	/* if text block was short, exit loop */
    } while (bytecount == 1024);
    return 0;
}				/* MDfile0_len */


/* Computes the message digest for a file from current position to EOF.
   Uses the RSA Data Security Inc. MD5 Message Digest Algorithm */

static int MDfile0(struct MD5Context *mdContext, FILE * inFile)
{
    int bytes;
    unsigned char buffer[1024];

    MD5Init(mdContext);
    while ((bytes = fread(buffer, 1, 1024, inFile)) != 0)
#ifdef MACTC5
		{
		mac_poll_for_break();
		MD5Update(mdContext,buffer,bytes);
		}
#else
	MD5Update(mdContext, buffer, bytes);
#endif
    return 0;
}

/* Computes the message digest for a specified file */

int MDfile(struct MD5Context *mdContext, char *filename)
{
    FILE *inFile;
    inFile = fopen(filename, FOPRBIN);

    if (inFile == NULL) {
	fprintf(pgpout, LANG("\n\007Can't open file '%s'\n"), filename);
	return -1;
    }
    MDfile0(mdContext, inFile);
    fclose(inFile);
    return 0;
}

/* Add a buffer's worth of data to the MD5 computation.  If a digest
 * pointer is supplied, complete the computation and write the digest.
 */
void MD_addbuffer(struct MD5Context *mdContext, byte * buf, int buflen,
		  byte digest[16])
{
    MD5Update(mdContext, buf, buflen);
    if (digest) {
	MD5Final(digest, mdContext);
	burn(*mdContext);	/* Paranoia */
    }
}

/* End MD5 routines */
