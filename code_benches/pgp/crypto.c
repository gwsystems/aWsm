/*	crypto.c  - Cryptographic routines for PGP.
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
	
 	Modified: 12-Nov-92 HAJK
 	Add FDL stuff for VAX/VMS local mode. 
	Reopen temporary files rather than create new version.
	
	Modified: 13-Dec-92 Derek Atkins <warlord@MIT.EDU)
	Added Multiple Recipients
	
	Modified 25-Feb-93 Colin Plumb
	Improved security of randseed.bin in strong_pseudorandom.
	Thoroughly revamped make_random_ideakey.
	
	Modified  6-May-93 Colin Plumb
	Changed to use the entry points in rsaglue.c.
	*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mpilib.h"
#include "mpiio.h"
#include "random.h"
#include "crypto.h"
#include "keymgmt.h"
#include "keymaint.h"
#include "mdfile.h"
#include "fileio.h"
#include "charset.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
#include "zipup.h"
#include "rsaglue.h"
#include "idea.h"
#ifdef MACTC5
#include "Macutil.h"
#include "Macutil2.h"
#include "Macutil3.h"
#include "Aestuff.h"
#include "MyBufferedStdio.h"
#include "ReplaceStdio.h"
#undef fopen
extern long GMTTimeshift(void);
extern unsigned char passhash[];
#endif

#define ENCRYPT_IT FALSE	/* to pass to idea_file */
#define DECRYPT_IT TRUE		/* to pass to idea_file */

#define	USE_LITERAL2


/* This variable stores the md5 hash of the current file, if it is
   available.  It is used in make_random_ideakey. */
static unsigned char md5buf[16];

/* This flag is set if the buffer above has been filled. */
static char already_have_md5 = 0;


/* Used by encryptfile */
static int encryptkeyintofile(FILE *g, char *mcguffin, byte *keybuf,
	char *keyfile, int ckp_length, int keys_used);

#ifdef  M_XENIX
long time();
#endif

/*--------------------------------------------------------------------------*/

#ifdef MACTC5
void CToPascal(char *s)
{
	/* "xyz\0" --> "\3xyz" ... converts C string to Pascal string */
	int i,j;
	j = string_length(s);
	for (i=j; i!=0; i--)
		s[i] = s[i-1];	/* move everything 1 byte to the right */
	s[0] = j;		/* Pascal length byte at beginning */
}	/* CToPascal */

void PascalToC( char *s )
{
	/* "\3xyz" --> "xyz\0" ... converts Pascal string to C string */
	int i,j;
	for (i=0,j=((byte *) s)[0]; i<j; i++)
		s[i] = s[i+1];	/* move everything 1 byte to the left */
	s[i] = '\0';		/* append C string terminator */
}	/* PascalToC */

#else
void CToPascal(char *s)
{
	/* "xyz\0" --> "\3xyz" ... converts C string to Pascal string */
	int i,j;
	j = string_length(s);
	for (i=j; i!=0; i--)
		s[i] = s[i-1];	/* move everything 1 byte to the right */
	s[0] = j;		/* Pascal length byte at beginning */
}	/* CToPascal */


void PascalToC( char *s )
{
	/* "\3xyz" --> "xyz\0" ... converts Pascal string to C string */
	int i,j;
	for (i=0,j=((byte *) s)[0]; i<j; i++)
		s[i] = s[i+1];	/* move everything 1 byte to the left */
	s[i] = '\0';		/* append C string terminator */
}	/* PascalToC */

#endif
/* 
	Note:  On MSDOS, the time() function calculates GMT as the local
	system time plus a built-in timezone correction, which defaults to
	adding 7 hours (PDT) in the summer, or 8 hours (PST) in the winter,
	assuming the center of the universe is on the US west coast. Really--
	I'm not making this up!  The only way to change this is by setting 
	the MSDOS environmental variable TZ to reflect your local time zone,
	for example "set TZ=MST7MDT".  This means add 7 hours during standard
	time season, or 6 hours during daylight time season, and use MST and 
	MDT for the two names of the time zone.  If you live in a place like 
	Arizona with no daylight savings time, use "set TZ=MST7".  See the
	Microsoft C function tzset().  Just in case your local software
	environment is too weird to predict how to set environmental
	variables for this, PGP also uses its own TZFIX variable in
	config.pgp to optionally correct this problem further.  For example,
	set TZFIX=-1 in config.pgp if you live in Colorado and the TZ
	variable is undefined.
*/
word32 get_timestamp(byte *timestamp)
/*	Return current timestamp as a byte array in internal byteorder,
	and as a 32-bit word */
{
	word32 t;
	t = 0xdeadbeef/* time(NULL) */;    /* returns seconds since GMT 00:00 1 Jan 1970 */

#ifdef _MSC_VER
#if (_MSC_VER == 700)
	/*  Under MSDOS and MSC 7.0, time() returns elapsed time since
	 *  GMT 00:00 31 Dec 1899, instead of Unix's base date of 1 Jan 1970.
	 *  So we must subtract 70 years worth of seconds to fix this.
	 *  6/19/92  rgb 
	*/
#define	LEAP_DAYS	(((unsigned long)70L/4)+1)
#define CALENDAR_KLUDGE ((unsigned long)86400L * (((unsigned long)365L \
						   * 70L) + LEAP_DAYS))
   	t -= CALENDAR_KLUDGE;
#endif
#endif
#ifdef MACTC5
	t -= GMTTimeshift();
#endif

	t += timeshift; /* timeshift derived from TZFIX in config.pgp */

	if (timestamp != NULL) {
		/* first, fill array in external byte order: */
		put_word32(t, timestamp);
		convert_byteorder(timestamp,4);
				/* convert to internal byteorder */
	}

	return t;	/* return 32-bit timestamp integer */
}	/* get_timestamp */


/*	Given timestamp as seconds elapsed since 1970 Jan 1 00:00:00,
	returns year (1970-2106), month (1-12), day (1-31).
	Not valid for dates after 2100 Feb 28 (no leap day that year).
	Also returns day of week (0-6) as functional return.
*/
static int date_ymd(word32 *tstamp, int *year, int *month, int *day)
{
	word32 days,y;
	int m,d,i;
	static short mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	days = (*tstamp)/(unsigned long)86400L;	/* day 0 is 1970/1/1 */
	days -= 730L;	/* align days relative to 1st leap year, 1972 */
	y = ((days*4)/(unsigned long)1461L);	/* 1972 is year 0 */
	/* reduce to days elapsed since 1/1 last leap year: */
	d = (int) (days - ((y/4)*1461L));
	*year = (int)(y+1972);
	for (i=0; i<48; i++) {	/* count months 0-47 */
		m = i % 12;
		d -= mdays[m] + (i==1);	/* i==1 is the only leap month */
		if (d < 0) {
			d += mdays[m] + (i==1);
			break;
		}
	}
	*month = m+1;
	*day = d+1;
	i = (int)((days-2) % (unsigned long)7L); /* compute day of week 0-6 */
	return i;	/* returns weekday 0-6; 0=Sunday, 6=Saturday */
}	/* date_ymd */


/*	Return date string, given pointer to 32-bit timestamp */
char *cdate(word32 *tstamp)
{
	int month,day,year;
	static char datebuf[20];
	if (*tstamp == 0)
		return "          ";
	(void) date_ymd(tstamp,&year,&month,&day);
	sprintf(datebuf,"%4d/%02d/%02d", year, month, day);
	return (datebuf);
}	/* cdate */


/*	Return date and time string, given pointer to 32-bit timestamp */
char *ctdate(word32 *tstamp)
{
	int hours,minutes;
	static char tdatebuf[40];
	long seconds;
	seconds = (*tstamp) % (unsigned long)86400L;
				/* seconds past midnight today */
	minutes = (int)((seconds+30L) / 60L);
				/* round off to minutes past midnight */
	hours = minutes / 60;	/* hours past midnight */
	minutes = minutes % 60;	/* minutes past the hour */
	sprintf(tdatebuf,"%s %02d:%02d GMT", cdate(tstamp), hours, minutes);
	return (tdatebuf);
}	/* ctdate */



/* Warn user he if key in keyfile at position fp of length pktlen, belonging
 * to userid, is untrusted.  Return -1 if the user doesn't want to proceed.
 */
static int warn_signatures(char *keyfile, long fp,
			   char *userid, boolean warn_only)
{
     FILE		*f;
     long		fpusr;
     int			usrpktlen;
     byte		keyctrl;
     int			trust_status = -1;

     keyctrl = KC_LEGIT_UNKNOWN; /* Assume the worst */
     if (getpubuserid (keyfile, fp, (byte *) userid, &fpusr,
		       &usrpktlen, FALSE) >= 0)
       {
	    f = fopen(keyfile, FOPRBIN);
	    fseek (f, fpusr+usrpktlen, SEEK_SET);
	    /* Read trust byte */
	    trust_status = read_trust(f, &keyctrl);
	    fseek(f, fp, SEEK_SET);
	    if (is_compromised(f)) {
		 CToPascal(userid);
		 fprintf(pgpout, "\n");
		 show_key(f, fp, 0);
		 fclose (f);
		 fprintf(pgpout,
   LANG("\007\nWARNING:  This key has been revoked by its owner,\n\
possibly because the secret key was compromised.\n"));
		 if (warn_only) {
		      /* this is only for checking signatures */
		      fprintf(pgpout,
   LANG("This could mean that this signature is a forgery.\n"));
		      return 1;
		 } else {	/* don't use it for encryption */
		      fprintf(pgpout,
   LANG("You cannot use this revoked key.\n"));
		      return -1;
		 }
	    }
	    fclose (f);
       }
     CToPascal(userid);
     if ((keyctrl & KC_LEGIT_MASK) != KC_LEGIT_COMPLETE) {
	  byte userid0[256];
	  PascalToC(userid);
	  strcpy ((char *) userid0, userid);
	  CToPascal(userid);

	  if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_UNKNOWN)
	    fprintf(pgpout,
   LANG("\007\nWARNING:  Because this public key is not certified with \
a trusted\nsignature, it is not known with high confidence that this \
public key\nactually belongs to: \"%s\".\n"),
		    LOCAL_CHARSET((char *)userid0));

	  if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_UNTRUSTED)
	    fprintf(pgpout,
   LANG("\007\nWARNING:  This public key is not trusted to actually belong \
to:\n\"%s\".\n"), LOCAL_CHARSET((char *)userid0));

		if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_MARGINAL)
			fprintf(pgpout,
   LANG("\007\nWARNING:  Because this public key is not certified with enough \
trusted\nsignatures, it is not known with high confidence that this \
public key\nactually belongs to: \"%s\".\n"),
				LOCAL_CHARSET((char *)userid0));

		if (keyctrl & KC_WARNONLY) {
				/* KC_WARNONLY bit already set,
				   user must have approved before. */
			fprintf(pgpout,
   LANG("But you previously approved using this public key anyway.\n"));
		}

		if (!filter_mode && !batchmode && !warn_only
		    && !(keyctrl & KC_WARNONLY))
		{
			fprintf(pgpout,
   LANG("\nAre you sure you want to use this public key (y/N)? "));
			if (!getyesno('n'))
				return -1;
			if (trust_status == 0
			    && (f = fopen(keyfile, FOPRWBIN)) != NULL)
			{
				fseek (f, fpusr+usrpktlen, SEEK_SET);
				keyctrl |= KC_WARNONLY;
				write_trust(f, keyctrl);
				fclose(f);
			}
		}
	}
	return 0;
}	/* warn_signatures */


/* Used to determine if nesting should be allowed. */
boolean legal_ctb(byte ctb)
{
	boolean legal;
	byte ctbtype;
	if (!is_ctb(ctb))		/* not even a bonafide CTB */
		return FALSE;
	/* Sure hope CTB internal bit definitions don't change... */
	ctbtype = (ctb & CTB_TYPE_MASK) >> 2;
	/* Only allow these CTB types to be nested... */
	legal = ( (ctbtype==CTB_PKE_TYPE)
		|| (ctbtype==CTB_SKE_TYPE)
		|| (ctbtype==CTB_CERT_SECKEY_TYPE)
		|| (ctbtype==CTB_CERT_PUBKEY_TYPE)
		|| (ctbtype==CTB_LITERAL_TYPE)
		|| (ctbtype==CTB_LITERAL2_TYPE)
		|| (ctbtype==CTB_COMPRESSED_TYPE)
		|| (ctbtype==CTB_CKE_TYPE)
		 );
	return legal;
}	/* legal_ctb */


/* Return nonzero if val doesn't match checkval, after printing a
 * warning.
 */
int
version_error(int val, int checkval)
{
	if (val != checkval) {
		fprintf (pgpout,
LANG("\n\007Unsupported packet format - you need a newer version of PGP \
for this file.\n"));
		return 1;
	}
	return 0;
}

int
version_byte_error(int val)
{
	if (val != VERSION_BYTE_OLD && val != VERSION_BYTE_NEW) {
		fprintf (pgpout,
LANG("\n\007Unsupported packet format - you need a newer version of PGP \
for this file.\n"));
		return 1;
	}
	return 0;
}
		

/*-------------------------------------------------------------------------*/

#define RAND_PREFIX_LENGTH 8	/* Length of IV for IDEA encryption */

/*
 * Make a random IDEA key.  Returns its length (the constant 16).
 * It also generates a random IV, which is placed in the key array
 * after the key proper, but is not counted in the length.
 * Reads IDEA random key and random number seed from file, cranks the
 * seed through the IDEA strong pseudorandom number generator,
 * and writes them back out.  This is used for generation of
 * cryptographically strong pseudorandom numbers.  This is mainly to
 * save the user the trouble of having to type in lengthy keyboard
 * sequences for generation of truly random numbers every time we want
 * to make a random session key.  This pseudorandom generator will only
 * work if the file containing the random seed exists and is not empty.
 * If this is not the case, it will be automatically created.
 *
 * The MD5 of the current file is used to "prewash" the random numbers,
 * to make it more difficult for an attacker to predict the output.
 *
 * The "skip" parameter says to skip that many bytes at the beginning,
 * used to generate a random IV only for conventional encryption.
 */
static int make_random_ideakey(byte key[IDEAKEYSIZE+RAND_PREFIX_LENGTH],
			       int skip)
{
	int count;
	struct IdeaCfbContext cfb;
	byte buf[10];

	ideaCfbInit(&cfb, md5buf);
	burn(md5buf);

	if (cryptRandOpen(&cfb) < 0) {
		fprintf(pgpout,LANG("Preparing random session key..."));

		 /* get some random key bits */
		trueRandAccum((IDEAKEYSIZE+RAND_PREFIX_LENGTH)*8);

		cryptRandInit(&cfb);
	}

	/*
	 * Generate a good random IDEA key and initial vector.  If we have
	 * no random bytes, the trueRandByte() part will be useless
	 */
	count = IDEAKEYSIZE+RAND_PREFIX_LENGTH;
	for (count = skip; count < IDEAKEYSIZE+RAND_PREFIX_LENGTH; count++)
		key[count] = cryptRandByte() ^ trueRandByte();

	/*
	 * Write out a new randseed.bin.  It is encrypted in precisely the
	 * same manner as the message itself, although the leading
	 * IV and check bytes are discarded.  This "postwash" is to
	 * ensure that it's easier to decrypt the message directly than to
	 * try to figure out what the key was by examining the entrails
	 * of the random number generator state in randseed.bin.
	 */
	ideaCfbInit(&cfb, key);
	memcpy(buf, key, 8);
	buf[8] = buf[6];
	buf[9] = buf[7];
	ideaCfbEncrypt(&cfb, buf, buf, 10);
	ideaCfbSync(&cfb);

	/* Save out the washed session key */
	cryptRandSave(&cfb);

	ideaCfbDestroy(&cfb);

	return IDEAKEYSIZE;
}


/*	Returns the length of a packet according to the CTB and
	the length field. */
word32 getpastlength(byte ctb, FILE *f)
{
	word32 length;
	unsigned int llength;	/* length of length */
	byte buf[8];

	fill0(buf,sizeof(buf));
	length = 0L;
	/* Use ctb length-of-length field... */
	llength = ctb_llength(ctb);	/* either 1, 2, 4, or 8 */
	if (llength==8) /* 8 means no length field, assume huge length */
		return -1L;	/* return huge length */

	/* now read in the actual length field... */
	if (fread((byteptr) buf,1,llength,f) < llength)
		return (-2L); /* error -- read failure or premature eof */
	/* convert length from external byteorder... */
	if (llength==1)
		length = (word32) buf[0];
	if (llength==2)
		length = (word32) fetch_word16(buf);
	if (llength==4)
		length = fetch_word32(buf);
	return length;
} /* getpastlength */


/* Write a CTB with the appropriate length field.  If big is true,
 * always use a four-byte length field.
 */
void write_ctb_len (FILE *f, byte ctb_type, word32 length, boolean big)
{
	int	llength, llenb;
	byte	ctb;
	byte	buf[4];

	if (big || (length > 0xFFFFL)) {
		llength = 4;
		llenb = 2;
	} else if ((word16)length > 0xFF) {
		llength = 2;
		llenb = 1;
	} else {
		llength = 1;
		llenb = 0;
	}
	
	putc(CTB_BYTE(ctb_type, llenb), f);
	/* convert length to external byteorder... */
	if (llength==1)
		buf[0] = length;
	if (llength==2)
		put_word16((word16) length, buf);
	if (llength==4)
		put_word32(length, buf);
	fwrite( buf, 1, llength, f );
} /* write_ctb_len */

/*
 * Use IDEA in cipher feedback (CFB) mode to encrypt or decrypt a file. 
 * The encrypted material starts out with a 64-bit random prefix, which
 * serves as an encrypted random CFB initialization vector, and
 * following that is 16 bits of "key check" material, which is a
 * duplicate of the last 2 bytes of the random prefix.  Encrypted key
 * check bytes detect if correct IDEA key was used to decrypt ciphertext.
 */
static
int idea_file(byte *ideakey, boolean decryp, FILE *f, FILE *g, word32 lenfile)
{
	int count, status = 0;
	extern byte textbuf[DISKBUFSIZE];
	struct IdeaCfbContext cfb;
#define RAND_PREFIX_LENGTH 8

	/* init CFB key */
	ideaCfbInit(&cfb, ideakey);

	if (!decryp) {	/* encrypt-- insert key check bytes */
		/* There is a random prefix followed by 2 key check bytes */

		memcpy(textbuf, ideakey+IDEAKEYSIZE, RAND_PREFIX_LENGTH);
    /* key check bytes are simply duplicates of final 2 random bytes */
		textbuf[RAND_PREFIX_LENGTH] = textbuf[RAND_PREFIX_LENGTH-2];
		textbuf[RAND_PREFIX_LENGTH+1] = textbuf[RAND_PREFIX_LENGTH-1];

		ideaCfbEncrypt(&cfb, textbuf, textbuf, RAND_PREFIX_LENGTH+2);
		fwrite(textbuf,1,RAND_PREFIX_LENGTH+2,g);
	} else { /* decrypt-- check for key check bytes */
		/* See if the redundancy is present after the random prefix */
		count = fread(textbuf,1,RAND_PREFIX_LENGTH+2,f);
		lenfile -= count;
		if (count==(RAND_PREFIX_LENGTH+2)) {
			ideaCfbDecrypt(&cfb, textbuf, textbuf,
				       RAND_PREFIX_LENGTH+2);
			if ((textbuf[RAND_PREFIX_LENGTH] !=
			     textbuf[RAND_PREFIX_LENGTH-2])
				|| (textbuf[RAND_PREFIX_LENGTH+1] !=
				    textbuf[RAND_PREFIX_LENGTH-1]))
			{
				status = -2;		/* bad key error */
			}
		} else	/* file too short for key check bytes */
			status = -3;		/* error of the weird kind */
	}

	ideaCfbSync(&cfb);

	/* read and write the whole file in CFB mode... */
	count = (lenfile < DISKBUFSIZE) ? (int)lenfile : DISKBUFSIZE;
	while (count && status == 0) {
		if ((count = fread(textbuf,1,count,f)) <= 0) {
			status = -3;
			break;
		}
		lenfile -= count;
		if (decryp)
			ideaCfbDecrypt(&cfb, textbuf, textbuf, count);
		else
			ideaCfbEncrypt(&cfb, textbuf, textbuf, count);
		if (fwrite(textbuf,1,count,g) != count)
			status = -3;
		count = (lenfile < DISKBUFSIZE) ? (int)lenfile : DISKBUFSIZE;
#ifdef MACTC5
		mac_poll_for_break();
#endif
	}

	ideaCfbDestroy(&cfb);	/* Clean up data structures */
	burn(textbuf);	/* burn sensitive data on stack */
	return status;	/* should always take normal return */
}	/* idea_file */


/* Checksum maintained as a running sum by read_mpi and write_mpi.
 * The checksum is maintained based on the plaintext values being
 * read and written.  To use it, store a 0 to it before doing a set
 * of read_mpi's or write_mpi's.  Then read it aftwerwards.
 */
word16	mpi_checksum;

/*
 * Read a mutiprecision integer from a file.
 * adjust_precision is TRUE iff we should call set_precision to the 
 * size of the number read in.
 * scrambled is TRUE iff field is encrypted (protects secret key fields).
 * Returns the bitcount of the number read in, or returns a negative
 * number if an error is detected.
 */
int read_mpi(unitptr r, FILE *f, boolean adjust_precision,
             struct IdeaCfbContext *cfb)
{
	byte buf[MAX_BYTE_PRECISION+2];
	unsigned int count;
	word16 bytecount,bitcount;

	mp_init(r,0);

	if ((count = fread(buf,1,2,f)) < 2)
		return (-1); /* error -- read failure or premature eof */

	bitcount = fetch_word16(buf);
	if (bits2units(bitcount) > global_precision)
		return -1;	/* error -- possible corrupted bitcount */

	bytecount = bits2bytes(bitcount);

	count = fread(buf+2,1,bytecount,f);
	if (count < bytecount)
		return -1;	/* error -- premature eof */

	if (cfb) {	/* decrypt the field */
		ideaCfbSync(cfb);
		ideaCfbDecrypt(cfb, buf+2, buf+2, bytecount);
	}

	/* Update running checksum, in case anyone cares... */
	mpi_checksum += checksum (buf, bytecount+2);

	/*	We assume that the bitcount prefix we read is an exact
		bitcount, not rounded up to the next byte boundary.
		Otherwise we would have to call mpi2reg, then call
		countbits, then call set_precision, then recall mpi2reg
		again.
	*/
	if (adjust_precision && bytecount) {
		/* set the precision to that specified by the number read. */
		if (bitcount > MAX_BIT_PRECISION-SLOP_BITS)
			return -1;
		set_precision(bits2units(bitcount+SLOP_BITS));
		/* Now that precision is optimally set, call mpi2reg */
	}

	if (mpi2reg(r,buf) == -1)	/* convert to internal format */
		return -1;
	burn(buf);	/* burn sensitive data on stack */
	return (bitcount);
}	/* read_mpi */



/*
 * Write a multiprecision integer to a file.
 * scrambled is TRUE iff we should scramble field on the way out,
 * which is used to protect secret key fields.
 */
void write_mpi(unitptr n, FILE *f, struct IdeaCfbContext *cfb)
{
	byte buf[MAX_BYTE_PRECISION+2];
	short bytecount;
	bytecount = reg2mpi(buf,n);
	mpi_checksum += checksum (buf, bytecount+2);
	if (cfb) { /* encrypt the field, skipping over the bitcount */
		ideaCfbSync(cfb);
		ideaCfbEncrypt(cfb, buf+2, buf+2, bytecount);
	}
	fwrite(buf,1,bytecount+2,f); 
	burn(buf);	/* burn sensitive data on stack */
}	/* write_mpi */

/*======================================================================*/

/*	Reads the first count bytes from infile into header. */
int get_header_info_from_file(char *infile,  byte *header, int count)
{
	FILE *f;
	fill0(header,count);
	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
		return -1;
	/* read Cipher Type Byte, and maybe more */
	count = fread(header,1,count,f);
	fclose(f);
	return count;	/* normal return */
}	/* get_header_info_from_file */


/* System clock must be broken if it isn't past this date: */
#define REASONABLE_DATE ((unsigned long) 0x27804180L)  /* 91 Jan 01 00:00:00 */


/*	Constructs a signed message digest in a signature certificate.
	Returns total certificate length in bytes, or returns negative
	error status.
*/
static
int make_signature_certificate(byte *certificate, struct MD5Context *MD,
   byte class, unitptr e, unitptr d, unitptr p, unitptr q, unitptr u,
			       unitptr n)
{
	byte inbuf[MAX_BYTE_PRECISION], outbuf[MAX_BYTE_PRECISION+2];
	int i, j, certificate_length, blocksize,bytecount;
	word16 ske_length;
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte keyID[KEYFRAGSIZE];
	byte val;
	int mdlen = 5;	/* length of class plus timestamp, for adding to MD */

	/*	Note that RSA key must be at least big enough to encipher a
		complete message digest packet in a single RSA block. */

		blocksize = countbytes(n)-1;	/* size of a plaintext block */
		if (blocksize < 31) {
			fprintf(pgpout,
   "\n\007Error: RSA key length must be at least 256 bits.\n");
			return -1;
		}

		get_timestamp(timestamp); /* Timestamp when signature was
					     made */
		if (tstamp < REASONABLE_DATE) {
			/* complain about bad time/date setting */
			fprintf(pgpout,
   LANG("\n\007Error: System clock/calendar is set wrong.\n"));
			return -1;
		}
		convert_byteorder(timestamp,4); /* convert to external form */

	/* Finish off message digest calculation with this information */
	MD_addbuffer (MD, &class, 1, 0);
	MD_addbuffer (MD, timestamp, 4, md5buf);
/* We wrote the digest to a static variable because we want to keep it around
   for random number generation later.   Also make a note of that fact. */
	already_have_md5 = 1;

	if (!quietmode) {
		fprintf(pgpout,LANG("Just a moment...")); /* RSA will take
							     a while. */
		fflush(pgpout);
	}

	/* do RSA signature calculation: */
	i = rsa_private_encrypt((unitptr)outbuf, md5buf, sizeof(md5buf),
	                        e, d, p, q, u, n);
	if (i < 0) {
		if (i == -4) {
			fprintf(pgpout,
   "\n\007Error: RSA key length must be at least 256 bits.\n");
		} else if (i == -3) {
			fputs(
"\a\nError: key is too large.  RSA keys may be no longer than 1024 bits\
,\ndue to limitations imposed by software provided by RSADSI.\n", pgpout);
		} else {
			fprintf(pgpout,"\a\nUnexpected error %d signing\n", i);
		}
		return i;
	}

	/* bytecount does not include the 2 prefix bytes */
	bytecount = reg2mpi(outbuf,(unitptr)outbuf); /* convert to external
							format */
	/*	outbuf now contains a message digest in external byteorder 
		form.  Now make a complete signature certificate from this.
		(Note that the first two bytes of md5buf are used below as
		part of the certificate.)
	*/

	certificate_length = 0;

   /* SKE is Secret Key Encryption (signed).  Append CTB for signed msg. */
	certificate[certificate_length++] = CTB_SKE;

   /* SKE packet length does not include itself or CTB prefix: */
	ske_length = 1 + 1 	  /* version and mdlen byte */
	  + mdlen		  /* class, timestamp and validation period */ 
	    + KEYFRAGSIZE + 1 + 1 /* Key ID and 2 algorithm bytes */
	      + 2 + bytecount+2;  /* 2 MD bytes and RSA MPI w/bitcount */
	put_word16((word16) ske_length, certificate+certificate_length);
	certificate_length+=2;	/* advance past word */

	certificate[certificate_length++] = version_byte;

	/* Begin fields that are included in MD calculation... */

	certificate[certificate_length++] =  mdlen; /* mdlen is length
						       of MD-extras */

	certificate[certificate_length++] =  class & 0xff;

	/* timestamp already in external format */
	for (j=0; j<SIZEOF_TIMESTAMP; j++)
		certificate[certificate_length++] =  timestamp[j];
 
	/* ...end of fields that are included in MD calculation */

	/* Now append keyID... */
	extract_keyID(keyID, n);	/* gets keyID */
	for (i=0; i<KEYFRAGSIZE; i++)
		certificate[certificate_length++] = keyID[i];

	certificate[certificate_length++] = RSA_ALGORITHM_BYTE;
	certificate[certificate_length++] = MD5_ALGORITHM_BYTE;

	/* Now append first two bytes of message digest */
	certificate[certificate_length++] = md5buf[0];
	certificate[certificate_length++] = md5buf[1];;

	/* Now append the RSA-signed message digest packet: */
	for (i=0; i<bytecount+2; i++)
		certificate[certificate_length++] = outbuf[i];

	if (!quietmode)
		fputc('.',pgpout);	/* Signal RSA signature completion. */

	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */

	return certificate_length; /* return length of certificate in bytes */

}	/* make_signature_certificate */


#ifdef VMS
/*
 * Local mode VMS, we write out the word VMS to say who owns the data then
 * we follow that with the file's FDL generated earlier by fdl_generate().
 * This FDL is preceded by a sixteen bit size. The file follows.
 */
void write_litlocal(FILE *g, char *fdl, short fdl_len)
{
    fputc('\0', g); /* Kludge for null literal file name (supplied by FDL) */
    fputs("VMS ", g);
    fwrite(&fdl_len, 2, 1, g); /* Byte order *not* important,
				  only VMS reads this!*/
    fwrite(fdl, 1, fdl_len, g);
}
#endif /* VMS */

/*======================================================================*/


/*	Write an RSA-signed message digest of input file to specified
	output file, and append input file to output file.
	separate_signature is TRUE iff we should not append the 
	plaintext to the output signature certificate.
	If lit_mode is MODE_TEXT, we know the infile is in canonical form.
	We create a CTB_LITERAL packet for the plaintext data.
*/
int signfile(boolean nested, boolean separate_signature,
		char *mcguffin, char *infile, char *outfile,
		char lit_mode, char *literalfile)
{
	FILE *f;
	FILE *g;
	int certificate_length;	/* signature certificate length */
	byte certificate[MAX_SIGCERT_LENGTH];
	char lfile[MAX_PATH];
	byte signature_class;
#ifdef VMS
	char *fdl;
	short fdl_len;
#endif /* VMS */


	{	/* temporary scope for some buffers */
		word32 tstamp; byte *timestamp = (byte *) &tstamp;
                  /* key certificate timestamp */
		byte userid[256];
		char keyfile[MAX_PATH];
		int status;
		struct MD5Context MD;
		byte keyID[KEYFRAGSIZE];
		unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
		unit d[MAX_UNIT_PRECISION];
		unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION];
		unit u[MAX_UNIT_PRECISION];

		set_precision(MAX_UNIT_PRECISION);/* safest opening
						     assumption */

		if (verbose)
			fprintf(pgpout,
 "signfile: infile = '%s', outfile = '%s', mode = '%c', literalfile = '%s'\n",
			infile,outfile,EXT_C(lit_mode),literalfile);

		if (MDfile(&MD, infile) < 0)
			return -1; /* problem with input file.  error return */

		userid[0] = '\0';
		if (mcguffin)
			strcpy((char *) userid,mcguffin); /* Who we are
							     looking for */

		if (getsecretkey(0, NULL, NULL, timestamp, NULL, NULL,
						 userid, n, e, d, p, q, u) < 0)
			return -1; /* problem with secret key file.
				      error return. */
		extract_keyID(keyID, n);
		strcpy(keyfile, globalPubringName); /* use default pathname */
		if ((status = getpublickey(GPK_NORVK, keyfile,
					   NULL, NULL, keyID,
				timestamp, userid, n, e)) < 0)
			return -1;	/* problem with public key file.
					   error return. */

		if (lit_mode==MODE_TEXT) signature_class = SM_SIGNATURE_BYTE;
		else signature_class = SB_SIGNATURE_BYTE;

		certificate_length = make_signature_certificate(certificate,
								&MD,
			signature_class, e, d, p, q, u, n);
		if (certificate_length < 0)
			return -1;	/* error return from
					   make_signature_certificate() */
	}	/* end of scope for some buffers */

	/* open file f for read, in binary (not text) mode...*/
#ifdef VMS
	if (lit_mode == MODE_LOCAL) {
	    if (!(fdl_generate(infile, &fdl, &fdl_len ) & 01)) {
		fprintf(pgpout,
   LANG("\n\007Can't open input plaintext file '%s'\n"),infile);
		return -1;
	    }
	}
#endif /* VMS */
	if ((f = fopen(infile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
   LANG("\n\007Can't open plaintext file '%s'\n"),infile);
		return -1;
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen(outfile,FOPWBIN)) == NULL) {
		fprintf(pgpout,
   LANG("\n\007Can't create signature file '%s'\n"),outfile);
		fclose(f);
		return -1;
	}

	/* write out certificate record to outfile ... */
	fwrite(certificate,1,certificate_length,g);

	if (literalfile == NULL) {
		/* Put in a zero byte to indicate no filename */
		lfile[0] = '\0';
	} else {
		strcpy( lfile, literalfile );
		file_to_canon( lfile );
		CToPascal( lfile );
	}

	if (!separate_signature) {
		if (!nested) {
			word32 flen = fsize(f);
			word32 dummystamp = 0;
			if (lit_mode == MODE_LOCAL)
#ifdef VMS
				write_ctb_len(g, CTB_LITERAL2_TYPE,
				    flen + fdl_len + sizeof(fdl_len) + 6,
					      TRUE);
#else
				/* debug check: should never get here */
			    fprintf(pgpout, "signfile: invalid mode\n");
#endif
			else {
#ifdef USE_LITERAL2
			    write_ctb_len (g, CTB_LITERAL2_TYPE,
					   flen + (unsigned char) lfile[0]
					   + 6, FALSE);
#else
			    write_ctb_len (g, CTB_LITERAL_TYPE, flen, FALSE);
#endif /* USE_LITERAL2 */
			}
			putc(lit_mode, g); 	/*	write lit_mode */
			if (lit_mode == MODE_LOCAL) {
#ifdef VMS
			    write_litlocal( g, fdl, fdl_len);
			    free(fdl);
#endif /* VMS */
			} else {
			    /* write literalfile name */
				fwrite (lfile, 1, (unsigned char) lfile[0]+1,
					g);
			    /* Dummy file creation timestamp */
			    fwrite ( &dummystamp, 1, sizeof(dummystamp), g);
			}
		}
		copyfile(f,g,-1L); /* copy rest of file from file f to g */
	}

	fclose(f);
	if (write_error(g)) {
		fclose(g);
		return -1;
	}
	fclose(g);
	return 0;	/* normal return */

}	/* signfile */

/*======================================================================*/

int compromise(byte *keyID, char *keyfile)
{
	FILE *f, *g;
	byte ctb;	/* Cipher Type Byte */
	int certificate_length;	/* signature certificate length */
	byte certificate[MAX_SIGCERT_LENGTH];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte userid[256];
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	struct MD5Context MD;
	unit d[MAX_UNIT_PRECISION];
	unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION];
	unit u[MAX_UNIT_PRECISION];
	long fp, insertpos;
	int pktlen;
	int prec;
	char *scratchf;

	setoutdir(keyfile);
	scratchf = tempfile(0);

	if (getsecretkey(0, NULL, keyID, timestamp, NULL, NULL,
					 userid, n, e, d, p, q, u) < 0)
		return -1;	/* problem with secret key file.
				   error return. */

	if (getpublickey(0, keyfile, &fp, &pktlen, keyID,
			timestamp, userid, n, e) < 0)
		return -1;

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
   LANG("\n\007Can't open key ring file '%s'\n"),keyfile);
		return -1;
	}

	fseek (f, fp+pktlen, SEEK_SET);
	nextkeypacket(f, &ctb);
	if (ctb == CTB_KEYCTRL) {
		insertpos = ftell(f);
		nextkeypacket(f, &ctb);
	} else {
		insertpos = fp + pktlen;
	}

	if (is_ctb_type(ctb, CTB_SKE_TYPE)) {
		fprintf(pgpout, LANG("This key has already been revoked.\n"));
		fclose(f);
		return -1;
	}

	prec = global_precision;
	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	fseek(f, fp, SEEK_SET);
	/* Calculate signature */
	if (MDfile0_len(&MD, f, pktlen) < 0) {
		fclose(f);
		return -1;	/* problem with input file.  error return */
	}
	set_precision(prec);

	certificate_length = make_signature_certificate(certificate, &MD,
		KC_SIGNATURE_BYTE, e, d, p, q, u, n);
	if (certificate_length < 0) {
		fclose(f);
		return -1;	/* error return from
				   make_signature_certificate() */
	}


	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen(scratchf,FOPWBIN)) == NULL) {
		fprintf(pgpout,
   LANG("\n\007Can't create output file to update key ring.\n"));
		fclose(f);
		return -1;
	}

	/* Copy pre-key and key to file g */
	rewind(f);
	copyfile (f, g, insertpos);

	/* write out certificate record to outfile ... */
	fwrite(certificate,1,certificate_length,g);

	/* Copy the remainder from file f to file g */
	copyfile (f, g, -1L);

	fclose(f);
	
	if (write_error(g)) {
		fclose(g);
		return -1;
	}
	fclose(g);

	savetempbak(scratchf,keyfile);

	fprintf(pgpout, LANG("\nKey compromise certificate created.\n"));
	return 0;	/* normal return */
}	/* compromise */

/*======================================================================*/

int do_sign(char *keyfile, long fp, int pktlen, byte *userid, byte *keyID,
            char *sigguffin, boolean batchmode)
{
	FILE *f;
	FILE *g;
	byte ctb;	/* Cipher Type Byte */
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte keyID2[KEYFRAGSIZE];
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	int certificate_length;	/* signature certificate length */
	byte certificate[MAX_SIGCERT_LENGTH];
	long fpusr;
	int usrpktlen, usrctrllen;
	char *tempring;
	int status;

	status = getpubuserid(keyfile, fp, userid, &fpusr,
	                      &usrpktlen, FALSE);
	if (status < 0)
		return -1;

	/* open file f for read, in binary (not text) mode...*/
	f = fopen(keyfile,FOPRBIN);
	if (f == NULL) {
		fprintf(pgpout,
   LANG("\n\007Can't open key ring file '%s'\n"),keyfile);
		return -1;
	}

	/* See if there is another signature with this keyID already */
	fseek (f, fpusr+usrpktlen, SEEK_SET);
	nextkeypacket(f, &ctb);		/* Add key control packet to len */
	usrctrllen = 0;
	if (ctb != CTB_KEYCTRL)
		fseek(f,fpusr+usrpktlen,SEEK_SET);
	else
		usrctrllen = (int) (ftell(f) - (fpusr+usrpktlen));
	for (;;) {
		status = readkeypacket(f,FALSE,&ctb,NULL,NULL,NULL,NULL,
					NULL,NULL,NULL,NULL,keyID2,NULL);
		if (status < 0  ||  is_key_ctb (ctb)  ||  ctb==CTB_USERID)
			break;
		if (equal_buffers(keyID, keyID2, KEYFRAGSIZE)) {
			fprintf(pgpout,
   LANG("\n\007Key is already signed by user '%s'.\n"),
				LOCAL_CHARSET(sigguffin));
			fclose(f);
			return -1;
		}
	}
	rewind(f);

	if (!batchmode) {
		fprintf(pgpout,
LANG("\n\nREAD CAREFULLY:  Based on your own direct first-hand knowledge, \
are\nyou absolutely certain that you are prepared to solemnly certify \
that\nthe above public key actually belongs to the user specified by \
the\nabove user ID (y/N)? "));
		if (!getyesno('n')) {
			fclose(f);
			return -1;
		}
	}

	{	/* temporary scope for some buffers */
		struct MD5Context MD;
		unit d[MAX_UNIT_PRECISION], p[MAX_UNIT_PRECISION];
		unit q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];

		set_precision(MAX_UNIT_PRECISION); /* safest opening
						      assumption */

		if ((g = fopen(keyfile,FOPRBIN)) == NULL) {
			fclose(f);
			fprintf(pgpout,
   LANG("\n\007Can't open key ring file '%s'\n"),keyfile);
			return -1;
		}
		fseek(g, fp, SEEK_SET);
		/* Calculate signature */
		if (MDfile0_len(&MD, g, pktlen) < 0) {
			fclose(f);
			fclose(g);
			return -1; /* problem with input file.
				      error return */
		}
		fclose(g);

		/* Add data from user id */
		CToPascal((char *)userid);
		MD5Update(&MD, userid+1, (int)(unsigned char)userid[0]);

		strcpy((char *)userid,sigguffin); /* Who we are looking for */

		/* Make sure that we DONT use the internal password to
		 * get the secret key!  This way you need to type your
		 * pass phrase every time you come to this point!
		 * Derek Atkins		<warlord@MIT.EDU>	93-02-25
		 *
		 * If batchmode, then let it use the passed-in password,
		 * for signing agents.
		 * Derek Atkins		<warlord@MIT.EDU>	94-06-20
		 */
#ifdef MACTC5
		passhash[0]='\0';
#endif
		if (getsecretkey((batchmode ? 0 : GPK_ASKPASS), NULL, NULL, 
				 timestamp, NULL, NULL,
		                 userid, n, e, d, p, q, u) < 0)
		{
			fclose(f);
			return -1; /* problem with secret key file.
				      error return. */
		}

		certificate_length =
		  make_signature_certificate(certificate, &MD,
					     K0_SIGNATURE_BYTE, e, d, p, q,
					     u, n);
		if (certificate_length < 0) {
			fclose(f);
			return -1; /* error return from
				      make_signature_certificate() */
		}

	}	/* end of scope for some buffers */

	/* open file g for write, in binary (not text) mode...*/
	tempring = tempfile(TMP_TMPDIR);
	if ((g = fopen(tempring,FOPWBIN)) == NULL) {
		fprintf(pgpout,
   LANG("\n\007Can't create output file to update key ring.\n"));
		fclose(f);
		return -1;
	}

	/* Copy pre-key and key to file g */
	copyfile (f, g, fpusr+usrpktlen+usrctrllen);

	/* write out certificate record to outfile ... */
	fwrite(certificate,1,certificate_length,g);

	/* Add "trusty" control packet */
	write_trust (g, KC_SIGTRUST_ULTIMATE|KC_CONTIG|KC_SIG_CHECKED);

	/* Copy the remainder from file f to file g */
	copyfile (f, g, -1L);
	
	fclose(f);
	if (write_error(g)) {
		fclose(g);
		return -1;
	}
	fclose(g);

	savetempbak(tempring,keyfile);

	fprintf(pgpout, LANG("\nKey signature certificate added.\n"));

        return 0;  /* normal return */

}       /* do_sign */


/*
 * Write an RSA-signed message digest of key for user keyguffin in
 * keyfile, using signature from user sigguffin.  Append
 * the signature right after the key.
 */
int signkey(char *keyguffin, char *sigguffin, char *keyfile)
{
	byte keyID[KEYFRAGSIZE];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte userid[256];
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	long fp;
	int pktlen;
	int status;

	/* Get signature key ID */
	strcpy((char *)userid,sigguffin);	/* Who we are looking for */
	status = getsecretkey(0, NULL, NULL, timestamp, NULL, NULL,
	                      userid, n, e, NULL, NULL, NULL, NULL);
	if (status < 0)
		return -1; /* problem with secret key file. error return. */
	extract_keyID(keyID, n);	/* Remember signer key ID */

	/* Check that the public key exists in the destination keyring */
	status = getpublickey(GPK_NORVK|GPK_GIVEUP, keyfile, &fp, &pktlen,
				 keyID, timestamp, userid, n, e);
	if (status < 0) {
		PascalToC((char *)userid);
		fprintf(pgpout, LANG("\nError: Key for signing userid '%s'\n\
does not appear in public keyring '%s'.\n\
Thus, a signature made with this key cannot be checked on this keyring.\n"), 
		LOCAL_CHARSET((char *)userid), keyfile);
		return -1;	/* problem with public key file.
				   error return. */
	}

	strcpy((char *)userid, keyguffin);
	fprintf(pgpout, LANG("\nLooking for key for user '%s':\n"), 
		LOCAL_CHARSET((char *)userid));

	status = getpublickey(GPK_SHOW|GPK_NORVK, keyfile, &fp, &pktlen, NULL,
	                      timestamp, userid, n, e);
	if (status < 0)
	    return -1;
	showKeyHash(n, e);

	PascalToC((char *) userid);
        if (do_sign(keyfile, fp, pktlen, userid, keyID, sigguffin, batchmode) < 0)
            return -1;

	return 0;	/* normal return */

}	/* signkey */

/*======================================================================*/

/* Check signature in infile for validity.  Strip off the signature
 * and write the remaining packet to outfile.  If strip_signature,
 * also write the signature to outfile.sig.
 * the original filename is stored in preserved_name
 */
int check_signaturefile(char *infile, char *outfile, boolean strip_signature,
			char *preserved_name)
{
	byte ctb,ctb2=0;	/* Cipher Type Bytes */
	char keyfile[MAX_PATH];	/* for getpublickey */
	char sigfile[MAX_PATH]; /* .sig file if strip_signature */
	char plainfile[MAX_PATH]; /* buffer for getstring() */
	char *tempFileName;	/* Name for temporary uncanonicalized file */
	FILE *tempFile;
	long fp;
	FILE *f;
	FILE *g;
	long start_text;	/* marks file position */
	int i,count;
	word16 cert_length;
	byte certbuf[MAX_SIGCERT_LENGTH];
	byteptr certificate; /* for parsing certificate buffer */
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	byte keyID[KEYFRAGSIZE];
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp; /* key certificate timestamp */
	word32 dummystamp;
	byte userid[256];
	struct MD5Context MD;
	byte digest[16];
	boolean separate_signature;
	boolean fixedLiteral = FALSE; /* Whether it's a fixed literal2
					 packet */
	extern char **myArgv;
	extern int myArgc;
	char lit_mode = MODE_BINARY;
	unsigned char litfile[MAX_PATH];
	word32 text_len = -1;
	int	status;
	byte	*mdextras;
	byte	mdlensave;
	byte	version;
	byte	mdlen;	/* length of material to be added to MD calculation */
	byte	class;
	byte	algorithm;
	byte	mdlow2[2];
	char	org_sys[5];	    /* Name of originating system */
        boolean retry = TRUE;
#ifdef VMS
	char	*fdl;
	short	fdl_len;
#endif
#ifdef MACTC5
	extern Boolean bad_separate_signature;
#endif
	int		outbufoffset;

	fill0( keyID, KEYFRAGSIZE );

	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	strcpy(keyfile, globalPubringName); /* use default pathname */

	if (verbose)
		fprintf(pgpout,
			"check_signaturefile: infile = '%s', outfile = '%s'\n",
		infile,outfile);

	if (preserved_name)
		*preserved_name = '\0';

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
		      LANG("\n\007Can't open ciphertext file '%s'\n"),infile);
		return -1;
	}

   /******************** Read header CTB and length field ******************/

	fread(&ctb,1,1,f);	/* read certificate CTB byte */
	certificate = certbuf;
	*certificate++ = ctb;	/* copy ctb into certificate */

	if (!is_ctb(ctb) || !is_ctb_type(ctb,CTB_SKE_TYPE))
		goto badcert;	/* complain and return bad status */

	cert_length = getpastlength(ctb, f); /* read certificate length */
	certificate += ctb_llength(ctb);	/* either 1, 2, 4, or 8 */
	if (cert_length > MAX_SIGCERT_LENGTH-3)	/* Huge packet length */
		goto badcert;	/* complain and return bad status */

	/* read whole certificate: */
	if (fread((byteptr) certificate, 1, cert_length, f) < cert_length)
		/* bad packet length field */
		goto badcert;	/* complain and return bad status */

	version = *certificate++;
	if (version_byte_error(version))
		goto err1;

	mdlensave = mdlen = *certificate++; /* length of material to
					       be added to MD */
	mdextras = certificate;	/* pointer to extra material for
				   MD calculation */

	class = *certificate++;
	if (class != SM_SIGNATURE_BYTE  &&  class != SB_SIGNATURE_BYTE) {
		(void) version_error(class, SM_SIGNATURE_BYTE);
		goto err1;
	}
	mdlen--;

	if (mdlen>0) {	/* if more MD material is included... */
		for (i=0; i<SIZEOF_TIMESTAMP; ++i) {
			timestamp[i] = *certificate++;
			mdlen--;
		}
	}

	if (mdlen>0) {	/* if more MD material is included... */
		certificate+=2;	/* skip past unused validity period field */
		mdlen-=2;
	}

	for (i=0; i<KEYFRAGSIZE; i++)
		keyID[i] = *certificate++; /* copy rest of key fragment */

	algorithm = *certificate++;
	if (version_error(algorithm, RSA_ALGORITHM_BYTE))
		goto err1;

	algorithm = *certificate++;
	if (version_error(algorithm, MD5_ALGORITHM_BYTE))
		goto err1;

	mdlow2[0] = *certificate++;
	mdlow2[1] = *certificate++;

	/* getpublickey() sets precision for mpi2reg, if key not found: use
	   maximum precision to avoid error return from mpi2reg() */
	if (getpublickey(0, keyfile, &fp, NULL, keyID,
			(byte *)&dummystamp, userid, n, e) < 0) {
		set_precision(MAX_UNIT_PRECISION); /* safest opening
						      assumption */
		if (filter_mode || batchmode) retry = FALSE;
	}

	if (mpi2reg((unitptr)inbuf,certificate) == -1) /* get signed message
							  digest */
		goto err1;
	certificate += countbytes((unitptr)inbuf)+2;

	if ((certificate-certbuf) != cert_length+3)
		/*	Bad length in signature certificate.  Off by 
			((certificate-certbuf) - (cert_length+3)) */
		goto badcert;	/* complain and return bad status */

	start_text = ftell(f);	/* mark position of text for later */

	if (fread(outbuf,1,1,f) < 1) {	/* see if any plaintext is there */
		/*	Signature certificate has no plaintext following it.
			Must be in another file.  Go look. */
		separate_signature = TRUE;
		if (preserved_name) /* let caller know there is
				       no output file */
			strcpy(preserved_name, "/dev/null");
		fclose(f);
		fprintf(pgpout,
   LANG("\nFile '%s' has signature, but with no text."),infile);
		if (myArgc > 3 && file_exists(myArgv[3])) {
			outfile = myArgv[3];
			fprintf(pgpout,
   LANG("\nText is assumed to be in file '%s'.\n"),outfile);
		} else {
			strcpy(plainfile, outfile);
			outfile = plainfile;
			drop_extension(outfile);

			if (file_exists(outfile)) {
				fprintf(pgpout,
   LANG("\nText is assumed to be in file '%s'.\n"),outfile);
			} else {
				if (batchmode)
					return -1;
				fprintf(pgpout,
   LANG("\nPlease enter filename of material that signature applies to: "));
#ifdef MACTC5
				if (!GetFilePath(LANG("File signature applies to?"),outfile,GETFILE))
					strcpy(outfile, "");
				else
					fprintf(pgpout, "%s\n",outfile);
#else
				getstring(outfile,59,TRUE); /* echo keyboard */
#endif
				if ((int)strlen(outfile) == 0)
					return -1;
			}
		}
		/* open file f for read, in binary (not text) mode...*/
		if ((f = fopen(outfile,FOPRBIN)) == NULL) {
			fprintf(pgpout,
   LANG("\n\007Can't open file '%s'\n"),outfile);
			return -1;
		}
		start_text = ftell(f);	/* mark position of text for later */
 		text_len = fsize(f);	/* remember length of text */
	} else {
		separate_signature = FALSE;
		/* We just read 1 byte, so outbuf[0] should contain a ctb, 
		   maybe a CTB_LITERAL byte. */
		ctb2 = outbuf[0];
		fixedLiteral = is_ctb_type(ctb2,CTB_LITERAL2_TYPE);
		if (is_ctb(ctb2) && (is_ctb_type(ctb2,CTB_LITERAL_TYPE)
				     ||fixedLiteral))
		{	/* Read literal data */
			text_len = getpastlength(ctb2, f); /* read packet
							      length */
			lit_mode = '\0';
			fread (&lit_mode,1,1,f); /* get literal packet
						    mode byte */
			if (lit_mode != MODE_TEXT
			    && lit_mode != MODE_BINARY &&
				lit_mode != MODE_LOCAL)
			{
				fprintf(pgpout,
   "\n\007Error: Illegal mode byte %02x in literal packet.\n",
					lit_mode); /* English-only diagnostic
						      for debugging */
				(void) version_error(lit_mode, MODE_BINARY);
				goto err1;
			}
			if (verbose)
				fprintf(pgpout,
   LANG("File type: '%c'\n"), EXT_C(lit_mode));
			/* Read literal file name, use it if possible */
			litfile[0] = 0;
			fread (litfile,1,1,f);
			if( fixedLiteral )
				/* Get corrected text_len value by subtracting
				   the length of the filename and the
				   timestamp and mode byte and litfile
				   length byte */
				text_len -= litfile[0]
				  + sizeof(dummystamp) + 2;
			if (litfile[0] > 0)
			{
				if ((int)litfile[0] >= MAX_PATH) {
					fseek(f, litfile[0], SEEK_CUR);
					litfile[0] = 0;
				} else {
					 fread (litfile+1,1,litfile[0],f);
				}
			}
				/* Use litfile if it's writeable and he
				   didn't say an outfile */
			if (litfile[0]) {
				PascalToC( (char *)litfile );
#ifdef EBCDIC
				file_from_canon( (char *)litfile );
#endif
				if (verbose)
					fprintf(pgpout,
   LANG("Original plaintext file name was: '%s'\n"), litfile);
				if (preserved_name)
					strcpy(preserved_name,
					       (char *) litfile);
			}
			if (lit_mode == MODE_LOCAL) {
			    fread(org_sys, 1, 4, f); org_sys[4] = '\0';
#ifdef VMS
#define LOCAL_TEST !strncmp("VMS ",org_sys,4)
#else
#define LOCAL_TEST FALSE
#endif
			    if (LOCAL_TEST) {
#ifdef VMS
					fread(&fdl_len, 2, 1, f);
					fdl = (char *) malloc(fdl_len);
					fread(fdl, 1, fdl_len, f);
					if ((g = 
					     fdl_create( fdl, fdl_len,
							outfile,
							(char *) litfile))
					    == NULL)
					{
						fprintf(pgpout,
   "\n\007Unable to create file %s\n", outfile);
						return -1;
					}
					free(fdl);
					if (preserved_name)
						strcpy(preserved_name,
						       (char *) litfile);
					text_len -= (fdl_len
						     + sizeof(fdl_len));
#endif /* VMS */
			    } else {
					fprintf(pgpout,
   "\n\007Unrecognised local binary type %s\n",org_sys);
					return -1;
			    }
			} else {
			    /* Discard file creation timestamp for now */
			    fread (&dummystamp, 1, sizeof(dummystamp), f);
			}
			start_text = ftell(f);	/* mark position of
						   text for later */
		}	/* packet is CTB_LITERAL_TYPE */
	}

	/* Use keyID prefix to look up key... */

	/*	Get and validate public key from a key file: */
	if (!retry || getpublickey(0, keyfile, &fp, NULL, keyID,
			(byte *)&dummystamp, userid, n, e) < 0)
	{			/* Can't get public key.  Complain and
				   process file copy anyway. */
		fprintf(pgpout,
   LANG("\nWARNING: Can't find the right public key-- can't check signature \
integrity.\n"));
		goto outsig;
	}	/* Can't find public key */

	count = rsa_public_decrypt(outbuf, (unitptr)inbuf, e, n);

	if (!quietmode)
		fputc('.',pgpout);	/* Signal RSA completion. */

	/* outbuf should contain message digest packet */
	/*==================================================================*/
	/* Look at nested stuff within RSA block... */

	if (count == -7 || (count > 0 && count != sizeof(digest)))
	{
		fputs(LANG("\007\nUnrecognized message digest algorithm.\n\
This may require a newer version of PGP.\n\
Can't check signature integrity.\n"), pgpout);
		goto outsig;	/* Output data anyway */
	}
	if (count == -5) {	/* RSAREF returned malformed */
		fputs(
LANG("\a\nMalformed or obsolete signature.  Can't check signature \
integrity.\n"),
			pgpout);
		goto outsig;
	}
	if (count == -3) {	/* Key too big */
		fputs(
LANG("\a\nSigning key is too large.  Can't check signature integrity.\n"), pgpout);
		goto outsig;
	}
	if (count < 0) {	/* Catch-all */
		fprintf(pgpout,
LANG("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n\
"));
		goto outsig;	/* Output data anyway */
	}

	/* Distinguish PKCS-compatible from pre-3.3 which has an extra byte */
	outbufoffset = (count==sizeof(digest)) ? 0 : 1;

	if (outbuf[outbufoffset] != mdlow2[0]  ||
		outbuf[outbufoffset+1] != mdlow2[1])
	{
		fprintf(pgpout,
   LANG("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n\
"));
		goto outsig;	/* Output data anyway */
	}

	/* Reposition file to where that plaintext begins... */
	fseek(f,start_text,SEEK_SET); /* reposition file from last ftell */

	MDfile0_len(&MD,f,text_len); /* compute a message digest from
					rest of file */

	MD_addbuffer (&MD, mdextras, mdlensave, digest); /* Finish message
							    digest */

	convert_byteorder(timestamp,4); /* convert timestamp from external
					   form */
	PascalToC((char *)userid);	/* for display */

	/* now compare computed MD with claimed MD */
/* Assume MSB external byte ordering */
	if (!equal_buffers(digest, outbuf+outbufoffset, 16)) {
		/* IF the signature is bad, AND this machine does not use
		   MSDOS-stype canonical text as its native text format, AND
		   this is a detached signature certificate, AND this file
		   appears to contain non-canonical ASCII text, THEN we
		   convert the file to canonical text form and check the
		   signature again.  This is because a detached signature
		   certificate probably means the file is not currently in
		   a canonical text packet, but it was in canonical text form
		   when the signature was created, so by re-canonicalizing
		   it we can check the signature. */
		if (class == SM_SIGNATURE_BYTE && separate_signature
		    && is_text_file(outfile))
		{	/* Reposition file to where the plaintext begins
			   and canonicalize it */
			rewind( f );
			tempFileName = tempfile( TMP_WIPE | TMP_TMPDIR );
			if (verbose)
				fprintf(stderr,
   "signature checking failed, trying in canonical mode\n");
			make_canonical(outfile,tempFileName);
			if( ( tempFile = fopen( tempFileName, FOPRBIN ) )
			   != NULL )
			{
			    	/* Now check the signature */
				MDfile0_len(&MD, tempFile, -1L );
				MD_addbuffer(&MD, mdextras, mdlensave,
					     digest);

				/* Clean up behind us */
				fclose( tempFile );
				rmtemp( tempFileName );

				/* Check if the signature is OK this time
				   round */
/* Assume MSB external byte ordering */
				if(equal_buffers(digest, outbuf+outbufoffset,
						 16))
					goto goodsig;
			}
		}

		if (checksig_pass == 1) { /* Bad signature - try one more pass with other charset */
			checksig_pass++;
			return -1;
		}
#ifdef MACTC5
		if (separate_signature) bad_separate_signature = true;
#endif
		fprintf(pgpout,"\007\n");
		fprintf(pgpout,
LANG("WARNING: Bad signature, doesn't match file contents!"));
		fprintf(pgpout,"\007\n");
		fprintf(pgpout,LANG("\nBad signature from user \"%s\".\n"),
			LOCAL_CHARSET((char *)userid));
		fprintf(pgpout,
LANG("Signature made %s using %d-bit key, key ID %s\n"),
                ctdate((word32 *)timestamp), countbits(n), key2IDstring(n));
		if (moreflag && !batchmode) {
			/* more will scroll the message off the screen */
			fprintf(pgpout, LANG("\nPress ENTER to continue..."));
			fflush(pgpout);
#ifdef MACTC5
			BailoutAlert(LANG("WARNING: Bad signature, doesn't match file contents!"));
#else
			getyesno('n');
#endif
		}
		goto warnsig;	/* Output data anyway */
	}

goodsig:
	signature_checked = TRUE;	/* set flag for batch processing */
	fprintf(pgpout,LANG("\nGood signature from user \"%s\".\n"),
		LOCAL_CHARSET((char *)userid));
	fprintf(pgpout,
LANG("Signature made %s using %d-bit key, key ID %s\n"),
                ctdate((word32 *)timestamp), countbits(n), key2IDstring(n));
#ifdef MACTC5
	AddResult((char *)userid);
#endif

warnsig:
	/* warn only, don't ask if user wants to use the key */
	warn_signatures(keyfile, fp, (char *)userid, TRUE);

outsig:
	/* Reposition file to where that plaintext begins... */
	fseek(f,start_text,SEEK_SET); /* reposition file from last ftell */

	if (separate_signature)
	{
		if (!quietmode)
			fprintf(pgpout,
LANG("\nSignature and text are separate.  No output file produced. "));
	} else {
		/* signature precedes plaintext in file... */
		/* produce a plaintext output file from signature file */
		/* open file g for write, in binary or text mode...*/
		if (lit_mode==MODE_LOCAL) {
#ifdef VMS
			if (status = fdl_copyfile2bin( f, g, text_len)) {
			     /*  Copy ok? */
				if (status > 0)
					fprintf(stderr,
					    "\n...copying to literal file\n");
				else
					perror(
					    "\nError copying from work file");
				fdl_close(g);
				goto err1;
			}
			fdl_close(g);
#endif /*VMS */
		} else {
			if (lit_mode == MODE_BINARY)
				g = fopen(outfile, FOPWBIN);
			else
				g = fopen(outfile, FOPWTXT);
			if (g == NULL) {
				fprintf(pgpout,
LANG("\n\007Can't create plaintext file '%s'\n"),outfile);
				goto err1;
			}
			CONVERSION = (lit_mode==MODE_TEXT)?EXT_CONV:NO_CONV;
			if (lit_mode == MODE_BINARY)
			    status = copyfile(f, g, text_len);
			else
			    status = copyfile_from_canon(f, g, text_len);
			CONVERSION = NO_CONV;
			if (write_error(g) || status < 0) {
				fclose(g);
				goto err1;
			}
			fclose(g);
		}

		if (strip_signature) {
			/* Copy signature to a .sig file */
			strcpy (sigfile, outfile);
			force_extension(sigfile,SIG_EXTENSION);
			if (!force_flag && file_exists(sigfile)) {
			fprintf(pgpout,
LANG("\n\007Signature file '%s' already exists.  Overwrite (y/N)? "),
					sigfile);
				if (!getyesno('n'))
					goto err1;
			}
			if ((g = fopen(sigfile,FOPWBIN)) == NULL) {
				fprintf(pgpout,
LANG("\n\007Can't create signature file '%s'\n"),sigfile);
				goto err1;
			}
			fseek (f,0L,SEEK_SET);
			copyfile (f,g,(unsigned long)(cert_length
						      +ctb_llength(ctb)+1));
			if (write_error(g)) {
				fclose(g);
				goto err1;
			}
			fclose(g);
			if (!quietmode)
				fprintf(pgpout,
LANG("\nWriting signature certificate to '%s'\n"),sigfile);
		}
	}

	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	fclose(f);
	if (separate_signature)
		return 0;	/* normal return, no nested info */
	if (is_ctb(ctb2) && (is_ctb_type(ctb2,CTB_LITERAL_TYPE)
			     || fixedLiteral))
				/* we already stripped away the CTB_LITERAL */
		return 0;	/* normal return, no nested info */
				/* Otherwise, it's best to assume a
				   nested CTB */
	return 1;		/* nested information return */

badcert:	/* Bad packet.  Complain. */
	fprintf(pgpout,
LANG("\n\007Error: Badly-formed or corrupted signature certificate.\n"));
	fprintf(pgpout,
LANG("File \"%s\" does not have a properly-formed signature.\n"),infile);
	/* Now just drop through to error exit... */

err1:
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	fclose(f);
	return -1;	/* error return */

}	/* check_signaturefile */

/*
 * Check signature of key in file fkey at position fpkey, using signature
 * in file fsig and position fpsig.  keyfile tells the file to use to
 * look for the public key in to check the sig.  Return 0 if OK,
 * -1 generic error
 * -2 Can't find key
 * -3 Key too big
 * -4 Key too small
 * -5 Maybe malformed RSA
 * -6 Unknown PK algorithm
 * -7 Unknown conventional algorithm
 * -8 Unknown version
 * -9 Malformed RSA packet
 * -10 Malformed packet
 * -20 BAD SIGNATURE
 */
int check_key_sig(FILE *fkey, long fpkey, int keypktlen, char *keyuserid,
	 FILE *fsig, long fpsig, char *keyfile, char *siguserid,
		  byte *xtimestamp, byte *sigclass)
{
	byte ctb;	/* Cipher Type Bytes */
	long fp;
	word16 cert_length;
	int i, count;
	byte certbuf[MAX_SIGCERT_LENGTH];
	byteptr certificate; /* for parsing certificate buffer */
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	byte keyID[KEYFRAGSIZE];
	struct MD5Context MD;
	byte digest[16];
	byte *mdextras;
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp; /* key certificate timestamp */
	byte	version;
	byte	mdlen;	/* length of material to be added to MD calculation */
	byte	class;
	byte	algorithm;
	byte	mdlow2[2];

	fill0( keyID, KEYFRAGSIZE );

	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

   /******************** Read header CTB and length field ******************/

	fseek(fsig, fpsig, SEEK_SET);
	fread(&ctb,1,1,fsig);	/* read certificate CTB byte */
	certificate = certbuf;
	*certificate++ = ctb;	/* copy ctb into certificate */

	if (!is_ctb(ctb) || !is_ctb_type(ctb,CTB_SKE_TYPE))
		goto badcert;	/* complain and return bad status */

	cert_length = getpastlength(ctb, fsig); /* read certificate length */
	certificate += ctb_llength(ctb);	/* either 1, 2, 4, or 8 */
	if (cert_length > MAX_SIGCERT_LENGTH-3)	/* Huge packet length */
		goto badcert;	/* complain and return bad status */

	/* read whole certificate: */
	if (fread((byteptr) certificate, 1, cert_length, fsig) < cert_length)
		/* bad packet length field */
		goto badcert;	/* complain and return bad status */

	version = *certificate++;
	if (version_byte_error(version))
		return -8;

	mdlen = *certificate++;	/* length of material to be added to MD */
	if (version_error(mdlen, 5))
		return -8;

	mdextras = certificate;	/* pointer to extra material for MD
				   calculation */

	*sigclass = class = *certificate++;
	if (class != K0_SIGNATURE_BYTE  &&  class != K1_SIGNATURE_BYTE &&
		class != K2_SIGNATURE_BYTE  &&  class != K3_SIGNATURE_BYTE &&
		class != KC_SIGNATURE_BYTE)
	{
		(void)version_error(class, K0_SIGNATURE_BYTE);
		return -8;
	}

	for (i=0; i<SIZEOF_TIMESTAMP; ++i)
		timestamp[i] = *certificate++;

	for (i=0; i<KEYFRAGSIZE; i++)
		keyID[i] = *certificate++; /* copy rest of key fragment */

	algorithm = *certificate++;
	if (version_error(algorithm, RSA_ALGORITHM_BYTE))
		return -6;

	algorithm = *certificate++;
	if (version_error(algorithm, MD5_ALGORITHM_BYTE))
		return -7;

	/* Grab 1st 2 bytes of message digest */
	mdlow2[0] = *certificate++;
	mdlow2[1] = *certificate++;

	/* We used to set precision here based on certificate value,
	 * but it was sometimes less than that based on n.  Read public
	 * key here to set precision, before we go on.
	 */
	/* This sets precision, too, based on n. */
	if (getpublickey(GPK_GIVEUP, keyfile, &fp, NULL, keyID,
			xtimestamp, (unsigned char *)siguserid, n, e) < 0)
		return -2;

	if (mpi2reg((unitptr)inbuf,certificate) == -1) /* get signed message
							  digest */
		return -10;
	certificate += countbytes((unitptr)inbuf)+2;

	if ((certificate-certbuf) != cert_length+3)
		/*	Bad length in signature certificate.  Off by 
			((certificate-certbuf) - (cert_length+3)) */
		return -10;	/* complain and return bad status */

	count = rsa_public_decrypt(outbuf, (unitptr)inbuf, e, n);

	if (count < 0)
		return count;

	if (count != sizeof(digest))
		return -9;	/* Bad RSA decrypt.  Corruption,
				   or wrong key. */

	/* outbuf should contain message digest packet */
	/*==================================================================*/
	/* Look at nested stuff within RSA block... */

/* Assume MSB external byte ordering */
	if (outbuf[0] != mdlow2[0]  || outbuf[1] != mdlow2[1])
		return -9;	/* Bad RSA decrypt.  Corruption,
				   or wrong key. */

	/* Position file to where that plaintext begins... */
	fseek(fkey,fpkey,SEEK_SET);

	/* compute a message digest from key packet */
	MDfile0_len(&MD,fkey,keypktlen);
	/* Add data from user id */
	if (class != KC_SIGNATURE_BYTE)
		MD5Update(&MD, (unsigned char *) keyuserid+1,
			  (int)(unsigned char)keyuserid[0]);
	/* Add time and class data */
	MD_addbuffer (&MD, mdextras, mdlen, digest); /* Finish message
							digest */

	/* now compare computed MD with claimed MD */
/* Assume MSB external byte ordering */
	if (!equal_buffers(digest, outbuf, 16))
		return -20;	/* BAD SIGNATURE */

	convert_byteorder(timestamp,4); /* convert timestamp from external
					   form */
	memcpy (xtimestamp, timestamp, 4); /* Return signature timestamp */

	return 0;	/* normal return */

badcert:	/* Bad packet.  Complain. */
	fprintf(pgpout,
LANG("\n\007Error: Badly-formed or corrupted signature certificate.\n"));
	return -10;
} /* check_key_sig */

/*======================================================================*/
static int squish_and_idea_file(byte *ideakey, FILE *f, FILE *g, 
	boolean attempt_compression)
{
	FILE *t;
	char *tempf = NULL;
	word32 fpos, fpos0;
	extern char plainfile[];

	/*
	**  If the caller specified that we should attempt compression, we
	**  create a temporary file 't' and compress our input file 'f' into
	**  't'.  Ideally, we would see if we get a good compression ratio 
	**  and if we did, then use file 't' for input and write a 
	**  CTB_COMPRESSED prefix.  But in this implementation we just always
	**  use the compressed output, even if it didn't compress well.
	*/

	rewind( f );

	if (!attempt_compression)
		t = f;	/* skip compression attempt */
	else	/* attempt compression-- get a tempfile */ 
		if ((tempf = tempfile(TMP_TMPDIR|TMP_WIPE)) == NULL ||
			(t = fopen(tempf, FOPWPBIN)) == NULL)
		  /* error: no tempfile */
			t = f;	/* skip compression attempt */
		else	/* attempt compression */ 
		{
			extern int zipup( FILE *, FILE * );


			if (verbose) fprintf(pgpout,
"\nCompressing [%s] ", plainfile);

			/* We don't put a length field on CTB_COMPRESSED yet */
			
			putc(CTB_COMPRESSED, t); /* write CTB_COMPRESSED */
				/* No CTB packet length specified
				   means indefinite length. */
			putc(ZIP2_ALGORITHM_BYTE, t); /* write ZIP algorithm
							 byte */

			/* Compression the file */
			zipup( f, t);
			if (write_error(t)) {
				fclose(t);
				if (tempf)
					rmtemp(tempf);
				return -1;
			}
			if (verbose) fprintf(pgpout, LANG("compressed.  ") );
			else if (!quietmode)
				fputc('.',pgpout);	/* show progress */
			rewind( t );
	  	}

	/*	Now write out file thru IDEA cipher... */

	/* Write CTB prefix, leave 4 bytes for later length */
	fpos0 = ftell(g);
	write_ctb_len (g, CTB_CKE_TYPE, 0L, TRUE);
	fpos = ftell(g) - fpos0;

	idea_file( ideakey, ENCRYPT_IT, t, g, fsize(t) );

	/* Now re-write CTB prefix, this time with length */
	fseek(g,fpos0,SEEK_SET);
	write_ctb_len (g, CTB_CKE_TYPE, fsize(g)-fpos, TRUE);

	if (t != f) {
		fclose( t );  /* close and remove the temporary file */
		if (tempf)
			rmtemp(tempf);
	}

	return 0;	/* normal return */

}	/* squish_and_idea_file */

int squish_file(char *infile, char *outfile)
{
	FILE *f, *g;
	extern int zip( FILE *, FILE * );

	if (verbose)
		fprintf(pgpout,"squish_file: infile = '%s', outfile = '%s'\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen( infile, FOPRBIN )) == NULL) {
		fprintf(pgpout,LANG("\n\007Can't open file '%s'\n"), infile );
		return -1;
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen( outfile, FOPWBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create compressed file '%s'\n"), outfile );
		fclose(f);
		return -1;
	}


	if (verbose) fprintf(pgpout, LANG("Compressing file..."));

	/* We don't put a length field on CTB_COMPRESSED yet */
	putc(CTB_COMPRESSED, g);	/* use compression prefix CTB */
	/* No CTB packet length specified means indefinite length. */
	putc(ZIP2_ALGORITHM_BYTE, g); 	/* use ZIP compression */

	/* Compress/store the file */
	zipup( f, g );
	if (verbose) fprintf(pgpout, LANG("compressed.  ") );

	fclose (f);
	if (write_error(g)) {
		fclose(g);
		return -1;
	}
	fclose (g);
	return 0;
}   /* squish_file */

#define NOECHO1 1	/* Disable password from being displayed on screen */
#define NOECHO2 2	/* Disable password from being displayed on screen */

int idea_encryptfile(char *infile, char *outfile,
	boolean attempt_compression)
{
	FILE *f;	/* input file */
	FILE *g;	/* output file */
	byte ideakey[24];
	struct hashedpw *hpw;

	if (verbose)
		fprintf(pgpout,
"idea_encryptfile: infile = '%s', outfile = '%s'\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen( infile, FOPRBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't open plaintext file '%s'\n"), infile );
		return -1;
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen( outfile, FOPWBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create ciphertext file '%s'\n"), outfile );
		fclose(f);
		return -1;
	}

	/* Get IDEA password, hashed to a key */
	if (passwds) {
		memcpy(ideakey, passwds->hash, sizeof(ideakey));
		memset(passwds->hash, 0, sizeof(passwds->hash));
		hpw = passwds;
		passwds = passwds->next;
		free(hpw);
	} else {
#ifdef MACTC5
 		byte savepass[20];
 		memcpy(savepass, passhash, 20);
		passhash[0] = '\0';
#endif
		if (!quietmode)
			fprintf(pgpout,
LANG("\nYou need a pass phrase to encrypt the file. "));
		if (batchmode
		    || GetHashedPassPhrase((char *)ideakey,NOECHO2) <= 0)
		{
			fclose(f);
			fclose(g);
#ifdef MACTC5
			memcpy(passhash, savepass, 20);
#endif
			return -1;
		}
#ifdef MACTC5
			memcpy(passhash, savepass, 20);
#endif
	}
	/*
	 * Get an initial vector, and write out a new randseed.bin.
	 * We do this now so that we can use the random bytes from the
	 * user's keystroke timings.
	 */
	make_random_ideakey(ideakey, 16);

	if (!quietmode) {
		fprintf(pgpout,
LANG("Just a moment..."));  /* this may take a while */
		fflush(pgpout);
	}

	/* Now compress the plaintext and encrypt it with IDEA... */
	squish_and_idea_file( ideakey, f, g, attempt_compression );

	burn(ideakey);	/* burn sensitive data on stack */

	fclose(f);
	if (write_error(g)) {
		fclose(g);
		return -1;
	}
	fclose(g);

	return 0;

}	/* idea_encryptfile */

/*======================================================================*/

static byte (*keyID_list)[KEYFRAGSIZE] = NULL;

static int getmyname(char *userid) {
        char keyfile[MAX_PATH];
        unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
        word32 tstamp; byte *timestamp = (byte *) &tstamp;
        long fp;
        int pktlen;

	strcpy(keyfile, globalSecringName);

	getpublickey(GPK_SECRET, keyfile, &fp,
		     NULL, NULL, timestamp, (unsigned char *)userid, n, e);

        PascalToC((char *)userid);

        return(0);
}

int encryptfile(char **mcguffins, char *infile, char *outfile, 
	boolean attempt_compression)
{
	int i,ckp_length;
	FILE *f;
	FILE *g;
	byte keybuf[MAX_BYTE_PRECISION]; /* This keeps our IDEA to encrypt */
	byte ideakey[24]; /* must be big enough for make_random_ideakey */
	word32 chksum;
	char keyfile[MAX_PATH];
	int keys_used = 0;

	if (mcguffins == NULL || *mcguffins == NULL || **mcguffins == '\0') {
		/* Well, we haven't gotten a user, lets die here */
		return -1;	
	}

	if (verbose)
		fprintf(pgpout,"encryptfile: infile = %s, outfile = %s\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen( infile, FOPRBIN )) == NULL)
	{
		fprintf(pgpout,
LANG("\n\007Can't open plaintext file '%s'\n"), infile );
		return -1;
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{
		fprintf(pgpout,
LANG("\n\007Can't create ciphertext file '%s'\n"), outfile );
		fclose(f);
		return -1;
	}

	/*	Now we have to generate a random session key and IV.
		As part of this computation, we use the MD5 hash of the
		current file, if it has previously been obtained due to a
		signing operation.  If it has not been obtained, we hash
		the first 2K (for efficiency reasons) for input into
		the key generatrion process.  This is to ensure that
		capturing a randseed.bin file will not allow reconstruction
		of subsequent session keys without knowing the message
		that was encrypted.  (A session key only protects a
		single message, so it is reasonable to assume that an
		opponent trying to obtain a session key is trying to
		obtain, and thus is ignorant of, the message it encrypts.)

		This is not perfect, but it's an improvement on how session
		keys used to be generated, and can be changed in future
		without compatibility worries.
	*/

	if (!already_have_md5) {
		/* Obtain some random bits from the input file */
		struct MD5Context MD;

		MD5Init(&MD);
		MDfile0_len(&MD, f, 4096); /* Ignore errors - what could be
					      done? */
		MD5Final(md5buf, &MD);
		already_have_md5 = 1;

		fseek(f, 0, SEEK_SET); /* Get back to the beginning for
					  encryption */
	}

	ckp_length = make_random_ideakey(ideakey, 0);
	/* Returns a 24 byte random IDEA key */

/* Assume MSB external byte ordering */
	/* Prepend identifier byte to key */
	keybuf[0] = IDEA_ALGORITHM_BYTE;
	for (i=0; i<ckp_length; ++i)
		keybuf[i+1] = ideakey[i];
	/* Compute and append checksum to the key */
	chksum = checksum (keybuf+1, ckp_length);
	ckp_length++;
	put_word16((word16) chksum, keybuf+ckp_length);
	ckp_length += 2;

	/* Ok, we now have our IDEA key which we are going to use
	 * to encrypt our packet.  We have stuffed it into a packet
	 * which we can now encrypt in the Public Key of EACH USER
	 * which we want to be able to decrypt this message.  Now we
	 * will walk down mcguffins until we hit a NULL or NULL string,
	 * and we will encrypt for each user in the list, and write
	 * that out to the output file.
	 *
	 * -derek	<warlord@MIT.EDU>	13 Dec 1992
	 */

	for (i = 0; mcguffins[i] != NULL; ++i)
		;
	if (encrypt_to_self)
		++i;
	keyID_list = xmalloc(i * KEYFRAGSIZE);
	/* Iterate through users */
	for (; *mcguffins && **mcguffins ; ++mcguffins) {
		strcpy(keyfile, globalPubringName);
		/* use default pathname */

		keys_used =
			encryptkeyintofile(g, *mcguffins, keybuf,
					   keyfile, ckp_length, keys_used);
	} /* for */

	if (!keys_used) {
		fclose(f);
		fclose(g);
		free(keyID_list);
		return -1;
	}

	/* encrypt to myself if need be */
	if (encrypt_to_self) {
	        if (!*my_name)
		        /* Find our name from our keyring */
		       getmyname(my_name);
		if (*my_name)
			/* If we were successful */
		  keys_used = 
		    encryptkeyintofile(g, my_name, keybuf,
				       keyfile, ckp_length, keys_used);
	}
	free(keyID_list);

	/* Finished with RSA block containing IDEA key. */

	/* Now compress the plaintext and encrypt it with IDEA... */
	squish_and_idea_file( ideakey, f, g, attempt_compression );

	burn(keybuf);	/* burn the Idea Key Packet */
	burn(ideakey);	/* burn sensitive data on stack */

	fclose(f);
	if (write_error(g)) {
		fclose(g);
		return -1;
	}
	fclose(g);

	return 0;
}	/* encryptfile */

static int
encryptkeyintofile(FILE *g, char *mcguffin, byte *keybuf,
		   char *keyfile, int ckp_length, int keys_used) {
	int i;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte keyID[KEYFRAGSIZE];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	word32 tstamp; byte *timestamp = (byte *) &tstamp; /* key certificate
							      timestamp */
	byte userid[256];
	long fp;
	int blocksize;
	byte (*keyp)[KEYFRAGSIZE];
	

	/* This "loop" is so we can break out at opportune moments */
	do {
		userid[0] = '\0';
		
		strcpy((char *)userid,mcguffin);
		/* Who we are looking for (C string) */
		
		/* Get and validate public key from a key file: 
		* We will be nice and ask the user ONCE (and ONLY once)
		* for a keyfile if its not in the default. 
		*/
		
		if (getpublickey((quietmode?0:GPK_SHOW)|GPK_NORVK,
				 keyfile, &fp, NULL, NULL,
				timestamp, userid, n, e) < 0)
		{
			fprintf(pgpout,
LANG("\n\007Cannot find the public key matching userid '%s'\n\
This user will not be able to decrypt this message.\n"), 
			LOCAL_CHARSET(mcguffin));
			continue;
		}
		
		/* Make sure we haven't already used this key */
		extract_keyID(keyID, n);
		for (keyp = keyID_list; keyp < keyID_list+keys_used; ++keyp) {
			if (!memcmp(keyp, keyID, KEYFRAGSIZE)) 
				break;
		}

		if (keyp < keyID_list + keys_used) {
				/* This key was already specified.
				   Quietly ignore it. */
			continue;
		}
		
		/* Add this keyID to the list of keys used so far */
		memcpy(keyp, keyID, KEYFRAGSIZE);
		
		PascalToC((char *)userid);
		if (warn_signatures(keyfile, fp, (char *)userid, 
				    FALSE) < 0) {
			fprintf(pgpout, LANG("Skipping userid %s\n"), mcguffin);
			continue;
		}
		
		/* set_precision has been properly called by getpublickey */
		
		/*	Note that RSA key must be at least big enough
			to encipher a complete conventional key packet 
			in a single RSA block.
		*/
		
		blocksize = countbytes(n)-1;	
		/* size of a plaintext block */
		
		if (blocksize < 31) {
			fprintf(pgpout,
"\n\007Error: RSA key length must be at least 256 bits.\n");
			fprintf(pgpout, "Skipping userid %s\n", mcguffin);
			continue;
		}
		
#ifdef MR_DEBUG
		/* XXX This is dangerous... This will print out the
		 * IDEA Key, which is a breach of security!
		 */
		fprintf(pgpout, "Idea Key: ");
		for (i = 0; i < ckp_length; i++)
			fprintf(pgpout, "%02X ", keybuf[i]);
		fprintf(pgpout, "\n");
#endif
		i = rsa_public_encrypt((unitptr)outbuf, keybuf,
				       ckp_length, e, n);
		if (i < 0) {
			if (i == -4) {
				fprintf(pgpout,
"\n\007Error: RSA key length must be at least 256 bits.\n");
			} else if (i == -3) {
				fputs(
"\a\nError: key is too large.  RSA keys may be no longer than 1024 bits\
,\ndue to limitations imposed by software provided by RSADSI.\n", pgpout);
			} else {
				fprintf(pgpout,
"\a\nUnexpected error %d encrypting\n", i);
			}
			fprintf(pgpout,
LANG("Skipping userid %s\n"), mcguffin);
			continue;
		}
		
		/* write out header record to outfile ... */
		
		/* PKE is Public Key Encryption */
		write_ctb_len (g, CTB_PKE_TYPE,
			       1+KEYFRAGSIZE+1+2+countbytes((unitptr)outbuf), 
			       FALSE);
		
		/* Write version byte */
		putc(version_byte, g);
		
		writekeyID( n, g );	
		/* write msg prefix fragment of modulus n */
		
		/* Write algorithm byte */
		putc(RSA_ALGORITHM_BYTE, g);
		
		/* convert RSA ciphertext block via reg2mpi and 
		* write to file
		*/
		
		write_mpi( (unitptr)outbuf, g, FALSE );
		
		burn(inbuf);	/* burn sensitive data on stack */
		burn(outbuf);	/* burn sensitive data on stack */
		++keys_used;

	} while (0);

	return keys_used;
}		/* encryptkeyintofile */

/*======================================================================*/

/*
 * Prepend a CTB_LITERAL prefix to a file.  Convert to canonical form if
 * lit_mode is MODE_TEXT.
 */
int make_literal(char *infile, char *outfile, char lit_mode, char *literalfile)
{
	char lfile[MAX_PATH];
	FILE *f;
	FILE *g;
	int status = 0;
#ifdef VMS
	char *fdl;
	short fdl_len;
#endif /* VMS */

	word32 flen, fpos;
	word32 dummystamp = 0;

	if (verbose)
		fprintf(pgpout,
"make_literal: infile = %s, outfile = %s, mode = '%c', literalfile = '%s'\n",
		infile,outfile,EXT_C(lit_mode),literalfile);

	/* open file f for read, in binary or text mode...*/

#ifdef VMS
	if (lit_mode == MODE_LOCAL) {
	    if (!(fdl_generate(infile, &fdl, &fdl_len ) & 01)) {
		fprintf(pgpout,
LANG("\n\007Can't open input plaintext file '%s'\n"),infile);
		return -1;
	    }
	}
#endif /*VMS*/
	if (lit_mode == MODE_TEXT)
		f = fopen(infile, FOPRTXT);
	else
		f = fopen(infile, FOPRBIN);
	if (f == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't open input plaintext file '%s'\n"),infile);
		return -1;
	}
	flen = fsize(f);

	/* 	open file g for write, in binary (not text) mode... */
	if ((g = fopen( outfile,FOPWBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create plaintext file '%s'\n"), outfile );
		goto err1;
	}

	if (literalfile == NULL) {
		/* Put in a zero byte to indicate no filename */
		lfile[0] = '\0';
	} else {
		strcpy( lfile, literalfile );
		file_to_canon( lfile );
		CToPascal( lfile );
	}

#ifdef USE_LITERAL2
#define	LENGTH_FIELD		(flen + (unsigned char) lfile[0] + 6)
#define	LIT_TYPE	CTB_LITERAL2_TYPE
#else
#define	LENGTH_FIELD	flen
#define	LIT_TYPE	CTB_LITERAL_TYPE
#endif
	if (lit_mode == MODE_BINARY)
		write_ctb_len (g, LIT_TYPE, LENGTH_FIELD, FALSE);
#ifdef VMS
	else if (lit_mode == MODE_LOCAL)
		write_ctb_len (g, CTB_LITERAL2_TYPE, flen
			       + fdl_len + sizeof(fdl_len) + 6, TRUE);
#endif /* VMS */
	else /* Will put in size field later for text mode */
		write_ctb_len (g, LIT_TYPE, 0L, TRUE);
#ifdef USE_LITERAL2
	fpos = ftell(g);
#endif
	putc(lit_mode, g);

	if (lit_mode == MODE_LOCAL) {
#ifdef VMS
	    write_litlocal( g, fdl, fdl_len);
	    free(fdl);
#else
	    ;   /*  Null statement if we don't have anything to do! */
#endif /* VMS */
	} else {
	    /* write literalfile name */
		fwrite (lfile, 1, (unsigned char) lfile[0]+1, g);
	    /* Dummy file creation timestamp */
	    fwrite ( &dummystamp, 1, sizeof(dummystamp), g);
	}
#ifndef USE_LITERAL2
	fpos = ftell(g);
#endif

	if ((lit_mode == MODE_BINARY) || (lit_mode == MODE_LOCAL)) {
		if (copyfile( f, g, -1L )) {
		    fprintf(pgpout,
"\n\007Unable to append to literal plaintext file");
		    perror("\n");
		    fclose(g);
		    goto err1;
		}
	} else {
		CONVERSION = (lit_mode == MODE_TEXT) ? INT_CONV : NO_CONV;
		status = copyfile_to_canon( f, g, -1L );
		CONVERSION = NO_CONV;
		/* Re-write CTB with correct length info */
		rewind (g);
		write_ctb_len (g, LIT_TYPE, fsize(g)-fpos, TRUE);
	}
	if (write_error(g) || status < 0) {
		fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);
	return 0;	/* normal return */

err1:
	fclose(f);
	return -1;	/* error return */

}	/* make_literal */
#undef LENGTH_FIELD
#undef LIT_TYPE

/*======================================================================*/

/*
 * Strip off literal prefix from infile, copying to outfile.
 * Get lit_mode and literalfile info from
 * the prefix.  Replace outfile with literalfile unless
 * literalfile is illegal
 * the original filename is stored in preserved_name
 * If lit_mode is MODE_TEXT, convert from canonical form as we
 * copy the data.
 */
int strip_literal(char *infile, char *outfile, char *preserved_name,
		char *lit_mode)
{
	byte ctb;	/* Cipher Type Byte */
	FILE *f;
	FILE *g;
	word32 LITlength = 0;
	unsigned char litfile[MAX_PATH];
	word32 dummystamp;
	char	org_sys[5];	    /* Name of originating system */
	int	status;
#ifdef VMS
	char	*fdl;
	short	fdl_len;
#endif
	*lit_mode = MODE_BINARY;
	if (verbose)
		fprintf(pgpout,"strip_literal: infile = %s, outfile = %s\n",
		infile,outfile);

	if (preserved_name)
		*preserved_name = '\0';

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't open input plaintext file '%s'\n"),infile);
		return -1;
	}

	fread(&ctb,1,1,f);	/* read Cipher Type Byte */

	if (!is_ctb(ctb) || !(is_ctb_type(ctb,CTB_LITERAL_TYPE) ||
	    is_ctb_type(ctb,CTB_LITERAL2_TYPE)))
	{
		/* debug message in English only -- something got corrupted */
		fprintf(pgpout,
"\n\007'%s' is not a literal plaintext file.\n",infile);
		fclose(f);
		return -1;
	}

	LITlength = getpastlength(ctb, f); /* read packet length */

	/* Read literal data */
	*lit_mode = '\0';
	fread (lit_mode,1,1,f);
	if ((*lit_mode != MODE_BINARY) && (*lit_mode != MODE_TEXT)
	    && (*lit_mode != MODE_LOCAL))
	{
		(void) version_error(*lit_mode, MODE_TEXT);
		fclose(f);
		return -1;
	}
	if (verbose)
		fprintf(pgpout, LANG("File type: '%c'\n"), EXT_C(*lit_mode));
	/* Read literal file name, use it if possible */
	litfile[0] = 0;
	fread (litfile,1,1,f); 
	if (is_ctb_type(ctb, CTB_LITERAL2_TYPE)) {
				/* subtract header length: namelength
				   + lengthbyte + modebyte + stamp */
		LITlength -= litfile[0] + 2 + sizeof(dummystamp);
	}
	/* Use litfile if it's writeable and he didn't say an outfile */
	if (litfile[0] > 0) {
		if ((int)litfile[0] >= MAX_PATH) {
			fseek(f, litfile[0], SEEK_CUR);
			litfile[0] = 0;
		} else {
			fread (litfile+1,1,litfile[0],f);
		}
	}
	if (litfile[0]) {
		PascalToC( (char *)litfile );
#ifdef EBCDIC
	       	file_from_canon( (char *)litfile );
#endif
		if (verbose)
			fprintf(pgpout,
LANG("Original plaintext file name was: '%s'\n"), litfile);
		if (preserved_name)
			strcpy(preserved_name, (char *) litfile);
	}
	if (*lit_mode == MODE_LOCAL) {
		fread(org_sys, 1, 4, f); org_sys[4] = '\0';
#ifdef VMS
#define LOCAL_TEST !strncmp("VMS ",org_sys,4)
#else
#define LOCAL_TEST FALSE
#endif
		if (LOCAL_TEST) {
#ifdef VMS
			remove(outfile); /*  Prevent litter, we recreate
					     the file with correct chars. */
			fread(&fdl_len, 2, 1, f);
			fdl = (char *) malloc(fdl_len);
			fread(fdl, 1, fdl_len, f);
			if ((g = fdl_create( fdl, fdl_len, outfile,
					    (char *) litfile)) == NULL) {
				fprintf(pgpout,
"\n\007Unable to create file %s\n", outfile);
				return -1;
			}
			free(fdl);
			if (preserved_name)
				strcpy(preserved_name, (char *) litfile);
			LITlength -= (fdl_len + sizeof(fdl_len));
#endif /* VMS */
		} else {
			fprintf(pgpout,
"\n\007Unrecognised local binary type %s\n",org_sys);
			return -1;
		}
	} else {
	    /* Discard file creation timestamp for now */
	    fread (&dummystamp, 1, sizeof(dummystamp), f);
	}

	if (*lit_mode==MODE_LOCAL) {
#ifdef VMS
		if (status = fdl_copyfile2bin( f, g, LITlength)) {
				/*  Copy ok? */
			if (status > 0)
				fprintf(stderr,
"\n...copying to literal file\n");
			else
				perror("\nError copying from work file");
			fdl_close(g);
			goto err1;
		}
		fdl_close(g);
#endif /*VMS */
	} else {
	    if (*lit_mode == MODE_TEXT)
			g = fopen(outfile, FOPWTXT);
	    else
			g = fopen(outfile, FOPWBIN);
	    if (g == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create plaintext file '%s'\n"), outfile );
		    goto err1;
	    }
	    /* copy rest of literal plaintext file */
	    CONVERSION = (*lit_mode == MODE_TEXT) ? EXT_CONV : NO_CONV;
	    if (*lit_mode == MODE_BINARY)
		    status = copyfile(f, g, LITlength);
		else
		    status = copyfile_from_canon(f, g, LITlength);
	    CONVERSION = NO_CONV;
		if (write_error(g) || status < 0) {
			fclose(g);
			goto err1;
		}
		fclose(g);
	}

	fclose(f);
	return 0;	/* normal return */

err1:
	fclose(f);
	return -1;	/* error return */

}	/* strip_literal */

/*======================================================================*/

int decryptfile(char *infile, char *outfile)
{
	byte ctb;	/* Cipher Type Byte */
	byte ctbCKE; /* Cipher Type Byte */
	FILE *f;
	FILE *g;
	int count = 0, status, thiskey, gotkey, end_of_pkes;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	unit d[MAX_UNIT_PRECISION];
	unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION];
	unit u[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	byte keyID[KEYFRAGSIZE];
	word32 tstamp; byte *timestamp = (byte *) &tstamp; /* key certificate
							      timestamp */
	byte userid[256];
	word32 flen;
	word32 fpos = 0;
	byte ver, alg;
	short realprecision = 0;
	word16 chksum;
	struct nkey {
		byte keyID[KEYFRAGSIZE];
		struct nkey *next;
	} *nkey, *nkeys = NULL;

	if (verbose)
		fprintf(pgpout,"decryptfile: infile = %s, outfile = %s\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't open ciphertext file '%s'\n"),infile);
		return -1;
	}

	/* Now we have to keep reading in packets until we either get
	 * to a non PKE-type packet or we find our own...  Once we find
	 * our own, we're gonna have to get our private key, and then
	 * keep going until we find the end of the PKE packets
	 *
	 * -derek	<warlord@MIT.EDU>	13 Dec 1992
	 */

	gotkey = end_of_pkes = 0; /* Set this flag now. */
	do {
		thiskey = 0;

		set_precision(MAX_UNIT_PRECISION);
		/* Need to set this EACH TIME...   Sigh.  This is because
		 * read_mpi needs to have a global_precision which is
		 * >= the size of the key.  Therefore once we find the
		 * real key, we save off the precision and then we'll
		 * reset it later.	-derek
		 */

		fread(&ctb,1,1,f);	/* read Cipher Type Byte */
		if (!is_ctb(ctb)) {
			fprintf(pgpout,
LANG("\n\007'%s' is not a cipher file.\n"),infile);
			fclose(f);
			return -1;
		}

		/* PKE is Public Key Encryption */
		if (!is_ctb_type(ctb,CTB_PKE_TYPE)) {
			end_of_pkes = 1;
			continue;
		}

		getpastlength(ctb, f); /* read packet length */

		/* Read and check version */
		ver = getc(f);
		if (version_byte_error(ver)) {
			fclose (f);
			return (-1);
		}

		fread(keyID,1,KEYFRAGSIZE,f); /* read key ID */
		/* Use keyID prefix to look up key. */

		/* Add this keyID to the list of keys read in */
		nkey = (struct nkey *) malloc(sizeof(struct nkey));
		if (nkey == NULL) {
			fprintf(stderr, LANG("\n\007Out of memory.\n"));
			exitPGP(7);
		}
		memcpy(nkey->keyID, keyID, KEYFRAGSIZE);
		nkey->next = nkeys;
		nkeys = nkey;

		/* Read and check algorithm */
		alg = getc(f);
		if (version_error(alg, RSA_ALGORITHM_BYTE)) {
			fclose (f);
			return (-1);
		}

		if (!gotkey)		/* Only do this if we havent already */
			/*	Get and validate secret key from a key file: */
			if (getsecretkey(GPK_GIVEUP|(quietmode?0:GPK_SHOW),
					 NULL, keyID, timestamp, NULL, NULL,
					 userid, n, e, d, p, q, u) == 0)
				{	
					thiskey = gotkey = 1;
					realprecision = global_precision;
				} else {
					set_precision(MAX_UNIT_PRECISION);
				}					
		/* DAMN this... This is REALLY frustrating, that I have to
		 * do this...  Basically, if I go to getsecretkey, it will
		 * set the precision, and the precision might NOT be correct
		 * if the key I get is not correct, so I have to set the
		 * precision NUMEROUS times in this loop..  This sucks, 
		 * but its the only way.  Sigh.
		 *
		 * -derek	<warlord@MIT.EDU>	13 Dec 1992
		 */

		/*	Note that RSA key must be at least big enough 
			to encipher a complete conventional key packet in 
			a single RSA block. */

		/*========================================================*/
		/* read ciphertext block, converting to internal format: */
		read_mpi((unitptr)inbuf, f, FALSE, FALSE);

		if (thiskey) {
			if (!quietmode) {
				fprintf(pgpout,LANG("Just a moment...")); 
				/* RSA will take a while. */
				fflush(pgpout);
			}
			count = rsa_private_decrypt(outbuf, (unitptr)inbuf,
			                            e, d, p, q, u, n);
			if (count < 0) {
				if (count == -3) {
					fputs(
"\a\nError: key is too large.  RSA keys may be no longer than 1024 bits\
,\ndue to limitations imposed by software provided by RSADSI.\n", pgpout);
				} else if (count == -9 || count == -7) {
					fprintf(pgpout,
LANG("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n\
"));
				} else if (count == -5) {
				        fprintf(pgpout,
LANG("\n\007Error: RSA block is possibly malformed.  Old format, maybe?\n"));
				} else {
					fprintf(pgpout,
"\a\nUnexpected error %d decrypting\n", count);
				}
				fclose(f);
				return count;
			}
		
			if (!quietmode)
				fputc('.',pgpout);	
					/* Signal RSA completion. */
		}

		fpos = ftell(f);	/* Save this position */

	} while (!end_of_pkes);		/* Loop until end of PKE packets */

	/* Should we list the recipients? */
	if (!gotkey || verbose) {
		char *user;

		setkrent(NULL);
		init_userhash();
		if (gotkey)	/* verbose flag */
			fprintf(pgpout,"\nRecipients:\n");
		else
			fprintf(pgpout,
LANG("\nThis message can only be read by:\n"));

		for (nkey = nkeys; nkey; nkey = nkey->next) {
			if ((user = user_from_keyID(nkey->keyID)) == NULL)
				fprintf(pgpout,
LANG("  keyID: %s\n"), keyIDstring(nkey->keyID));
			else
				fprintf(pgpout, "  %s\n", LOCAL_CHARSET(user));
		}
		endkrent();
	}
	for (nkey = nkeys; nkey; ) {
		nkey = nkey->next;
		free(nkeys);
		nkeys = nkey;
	}

	/* Ok, Now lets clean up, and continue on to the rest of the file so
	 * that it can be decrypted properly.  Things should be ok once I
	 * reset some stuff here...	-derek
	 */
	if (gotkey) {
		fseek(f, fpos, SEEK_SET); /* Get back to the Real McCoy! */
		set_precision(realprecision); /* reset the precision */
	} else {
		/* No secret key, exit gracefully (NOT!) */
		fprintf(pgpout,
LANG("\n\007You do not have the secret key needed to decrypt this file.\n"));
		fclose(f);
		return -1;
	}
	/* Verify that top of buffer has correct algorithm byte */
	--count;	/* one less byte to drop algorithm byte */
/* Assume MSB external byte ordering */
	if (version_error(outbuf[0], IDEA_ALGORITHM_BYTE)) {
		fclose(f);
		return -1;
	}

	/* Verify checksum */
	count -= 2;	/* back up before checksum */
/* Assume MSB external byte ordering */
	chksum = fetch_word16(outbuf+1+count);
	if (chksum != checksum(outbuf+1, count)) {
		fprintf(pgpout,
LANG("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n\
"));
		fclose(f);
		return -1;
	}

	/* outbuf should contain random IDEA key packet */
	/*==================================================================*/

	/* 	open file g for write, in binary (not text) mode... */

	if ((g = fopen( outfile, FOPWBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create plaintext file '%s'\n"), outfile );
		goto err1;
	}

	fread(&ctbCKE,1,1,f);	/* read Cipher Type Byte, should be CTB_CKE */
	if (!is_ctb(ctbCKE) || !is_ctb_type(ctbCKE,CTB_CKE_TYPE)) {
		/* Should never get here. */
		fprintf(pgpout,"\007\nBad or missing CTB_CKE byte.\n");
		goto err1;	/* Abandon ship! */
	}

	flen = getpastlength(ctbCKE, f); /* read packet length */

	/* Decrypt ciphertext file */
/* Assume MSB external byte ordering */
	status = idea_file( outbuf+1, DECRYPT_IT, f, g, flen );
	if (status < 0) {
		fprintf(pgpout,
LANG("\n\007Error: Decrypted plaintext is corrupted.\n"));
	}
	if (!quietmode)
		fputc('.',pgpout);	/* show progress */

	if (write_error(g)) {
		fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	mp_burn(d);	/* burn sensitive data on stack */
	mp_burn(p);	/* burn sensitive data on stack */
	mp_burn(q);	/* burn sensitive data on stack */
	mp_burn(u);	/* burn sensitive data on stack */
	if (status < 0)	/* if idea_file failed, then error return */
		return status;
	return 1;	/* always indicate output file has
			   nested stuff in it. */

err1:
	fclose(f);
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	mp_burn(d);	/* burn sensitive data on stack */
	mp_burn(p);	/* burn sensitive data on stack */
	mp_burn(q);	/* burn sensitive data on stack */
	mp_burn(u);	/* burn sensitive data on stack */
	return -1;	/* error return */

}	/* decryptfile */

int idea_decryptfile(char *infile, char *outfile)
{
	byte ctb;	/* Cipher Type Byte */
	FILE *f;
	FILE *g;
	byte ideakey[16];
	int status, retries = 0;
	struct hashedpw *hpw, **hpwp;
	word32 flen;

	if (verbose)
		fprintf(pgpout,"idea_decryptfile: infile = %s, outfile = %s\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't open ciphertext file '%s'\n"),infile);
		return -1;
	}

	/* 	open file g for write, in binary (not text) mode... */
	if ((g = fopen( outfile, FOPWBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create plaintext file '%s'\n"), outfile );
		goto err1;
	}

	/* First, try all pre-specified passwords */
	hpwp = &passwds;
	hpw = *hpwp;

	do /* while pass phrase is bad */
	{
		fread(&ctb,1,1,f); /* read Cipher Type Byte,
				      should be CTB_CKE */

		if (!is_ctb(ctb) || !is_ctb_type(ctb,CTB_CKE_TYPE)) {
			/* Should never get here. */
			fprintf(pgpout,"\007\nBad or missing CTB_CKE byte.\n");
			fclose(g);
			goto err1;	/* Abandon ship! */
		}
		flen = getpastlength(ctb, f); /* read packet length */

		/* Get IDEA password, hashed */
		if (hpw) {
			/* first try environment passwords */
			memcpy(ideakey, hpw->hash, sizeof(ideakey));
		} else {
#ifdef MACTC5
 			byte savepass[20];
 			memcpy(savepass, passhash, 20);
			passhash[0] = '\0';
#endif
			fprintf(pgpout,
LANG("\nYou need a pass phrase to decrypt this file. "));
			if (batchmode
			    || GetHashedPassPhrase((char *)ideakey,NOECHO1)
			    <= 0)
			{
				fclose(f);
				fclose(g);
#ifdef MACTC5
	 			memcpy(savepass, passhash, 20);
#endif
				return -1;
			}
#ifdef MACTC5
	 			memcpy(savepass, passhash, 20);
#endif
		}

		if (!quietmode) {
			fprintf(pgpout,LANG("Just a moment..."));
				/* this may take a while */
			fflush(pgpout);
		}

		status = idea_file( ideakey, DECRYPT_IT, f, g, flen );
		if (status == 0) {
			if (hpw) {
				/* "Use up" password. */
				*hpwp = hpw->next;
				memset(hpw->hash, 0, sizeof(hpw->hash));
				free(hpw);
			}
			break;
		}
		if (hpw) {
			/* Go to next available password */
			hpwp = &hpw->next;
			hpw = *hpwp;
		} else {
			++retries;
			fprintf(pgpout,
LANG("\n\007Error:  Bad pass phrase.\n"));
#ifdef MACTC5
			passhash[0] = '\0';
#endif
		}

		rewind(f);
		rewind(g);
	} while (status == -2 && retries < 2);

	burn(ideakey);	/* burn sensitive data on stack */

	if (status == 0 && !quietmode)
		fputc('.',pgpout);	/* show progress */

	if (write_error(g)) {
		fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);

	if (status < 0) {	/* if idea_file failed, then complain */
		remove(outfile);	/* throw away our mistake */
		return status;		/* error return */
	}
	if (!quietmode)
		fprintf(pgpout,LANG("Pass phrase appears good. "));
	return 1;		/* always indicate output file has
				   nested stuff in it. */

err1:
	fclose(f);
	return -1;	/* error return */

}	/* idea_decryptfile */

int decompress_file(char *infile, char *outfile)
{
	byte ctb;
	FILE *f;
	FILE *g;
	extern void lzhDecode( FILE *, FILE * );
	extern int unzip( FILE *, FILE * );
	if (verbose) fprintf(pgpout, LANG("Decompressing plaintext...") );

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't open compressed file '%s'\n"),infile);
		return -1;
	}

	fread(&ctb,1,1,f);	/* read and skip over Cipher Type Byte */
	if (!is_ctb_type( ctb, CTB_COMPRESSED_TYPE )) {
		/* Shouldn't get here, or why were we called to begin with? */
		fprintf(pgpout,"\007\nBad or missing CTB_COMPRESSED byte.\n");
		goto err1;	/* Abandon ship! */
	}

	getpastlength(ctb, f); /* read packet length */
	/* The packet length is ignored.  Assume it's huge. */

	fread(&ctb,1,1,f);	/* read and skip over compression
				   algorithm byte */
	if (ctb != ZIP2_ALGORITHM_BYTE) {
		/* We only know how to uncompress deflate-compressed data.  We
		   may hit imploded or Lharc'ed data but treat it as an error
		   just the same */
		fprintf(pgpout,
LANG("\007\nUnrecognized compression algorithm.\n\
This may require a newer version of PGP.\n"));
		goto err1;	/* Abandon ship! */
	}

	/* 	open file g for write, in binary (not text) mode... */
	if ((g = fopen( outfile, FOPWBIN )) == NULL) {
		fprintf(pgpout,
LANG("\n\007Can't create decompressed file '%s'\n"), outfile );
		goto err1;
	}

	if (unzip( f, g )) {
		fprintf(pgpout,
LANG("\n\007Decompression error.  Probable corrupted input.\n"));
		goto err2;
	}

	if (verbose)
		fprintf(pgpout, LANG("done.  ") );
	else if (!quietmode)
		fputc('.',pgpout);	/* show progress */

	if (write_error(g))
		goto err2;

	fclose(g);
	fclose(f);
	return 1;		/* always indicate output file
				   has nested stuff in it. */

err2:
	fclose(g);
err1:
	fclose(f);
	return -1;	/* error return */

} /* decompress_file */
