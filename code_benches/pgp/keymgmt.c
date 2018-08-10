/*      keymgmt.c  - Key management routines for PGP.
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
#include <stdlib.h>
#ifdef UNIX
#include <sys/types.h>
#endif
#include <time.h>
#include <ctype.h>
#include "system.h"
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "rsagen.h"
#include "mpiio.h"
#include "language.h"
#include "pgp.h"
#include "md5.h"
#include "charset.h"
#include "keymaint.h"
#include "idea.h"
#ifdef MACTC5
#include "Aestuff.h"
#include "MacPGP.h"
#include "Macutil2.h"
#include "Macutil3.h"
#include "PGPDialogs.h"
#include "password.h"
#include "exitpgp.h"
#include "MyBufferedStdio.h"
#include "ReplaceStdio.h"
boolean userid_match(char *userid, char *substr,unitptr n);
void showKeyHash( unitptr n, unitptr e );
int backup_rename(char *scratchfile, char *destfile);
#endif

/*
   **   Convert to or from external byte order.
   **   Note that convert_byteorder does nothing if the external byteorder
   **  is the same as the internal byteorder.
 */
#define convert2(x,lx)	convert_byteorder( (byteptr)&(x), (lx) )
#define convert(x)		convert2( (x), sizeof(x) )


/*
 * check if userid matches the substring, magic characters ^ and $
 * can be used to match start and end of userid.
 * if n is NULL, only return TRUE if substr is an exact match of
 * userid, a substring does not match in this case.
 * the comparison is always case insensitive
 */
#ifdef MACTC5
boolean userid_match(char *userid, char *substr, unitptr n)
#else
static boolean userid_match(char *userid, char *substr, unitptr n)
#endif
{
    boolean match_end = FALSE;
    int id_len, sub_len, i;
    char buf[256], sub[256], *p;
#ifdef MACTC5
	int j;
	char argKeyID[10],curKeyID[10];
	unsigned long tempKeyID[2];
#endif

    if (substr == NULL || *substr == '\0')
	return TRUE;
    if (userid == NULL || *userid == '\0')
	return FALSE;

    /* Check whether we have an ASCII or hex userID to check for */
#ifdef EBCDIC
    /* EBCDIC assertion: to_lower works on EBCDIC (not internal) charset */
    if (n != NULL && EXT_C(substr[0]) == '0' && to_lower(EXT_C(substr[1])) == 'x') {
	userid = key2IDstring(n);
	CONVERT_TO_CANONICAL_CHARSET(userid);
	substr += 2;
    }
    id_len = strlen(userid);
    for (i = 0; i <= id_len; ++i)
	buf[i] = INT_C(to_lower(EXT_C(userid[i])));

    sub_len = strlen(substr);
    for (i = 0; i <= sub_len; ++i)
	sub[i] = INT_C(to_lower(EXT_C(substr[i])));
#else /* !EBCDIC */
    if (n != NULL && substr[0] == '0' && to_lower(substr[1]) == 'x') {
	userid = key2IDstring(n);
	substr += 2;
    }
    id_len = strlen(userid);
    for (i = 0; i <= id_len; ++i)
	buf[i] = to_lower(userid[i]);

    sub_len = strlen(substr);
    for (i = 0; i <= sub_len; ++i)
	sub[i] = to_lower(substr[i]);
#endif /* !EBCDIC */

    if (n == NULL) {
	return !strcmp(buf, sub);
    }
#ifdef MAGIC_MATCH
    if (sub_len > 1 && sub[sub_len - 1] == '$') {
	match_end = TRUE;
	sub[--sub_len] = '\0';
    }
    if (*sub == '^') {
	if (match_end)
	    return !strcmp(buf, sub + 1);
	else
	    return !strncmp(buf, sub + 1, sub_len - 1);
    }
#endif
    if (sub_len > id_len)
	return FALSE;

    if (match_end)
	return !strcmp(buf + id_len - sub_len, sub);

    p = buf;
    while ((p = strchr(p, *sub)) != NULL) {
	if (strncmp(p, sub, sub_len) == 0)
#ifdef MACTC5
		{	if (!argc)
				return true;
			for(j=1; j<100; j++) {
				if (argv[j]==nil) {
					j=100;
					break;
					}
				if (!strcmp(substr,argv[j])) break;
			}
			if (j==100) return TRUE;
			if (arg_keyid[j]==0) return TRUE;
			tempKeyID[0]=0;
			tempKeyID[1]=arg_keyid[j];
			strcpy(argKeyID,keyIDstring((byte *)tempKeyID));
			strcpy(curKeyID,key2IDstring( n ));
			if (strcmp(argKeyID,curKeyID))
				return FALSE;
			else {
				arg_keyid[j]=0;
				return TRUE;
			}
	}
#else
	    return TRUE;
#endif
	++p;
    }
    return FALSE;
}

int is_key_ctb(byte ctb)
{
    return ctb == CTB_CERT_PUBKEY || ctb == CTB_CERT_SECKEY;
}


/*
   **   keyIDstring
   **
   **   Return printable key fragment, which is an abbreviation of the public
   **   key.  Show LEAST significant 32 bits (KEYFRAGSIZE bytes) of modulus,
   **   LSB last.  Yes, that's LSB LAST.
 */

char const blankkeyID[] = "        ";

char *keyIDstring(byte * keyID)
{
    short i;
    char *bufptr;		/* ptr to Key ID string */
    static char keyIDbuf[9];

    /* only show bottom 4 bytes of keyID */

    bufptr = keyIDbuf;

#ifdef XLOWFIRST
    /* LSB-first keyID format */

    for (i = 3; i >= 0; i--) {
	sprintf(bufptr, "%02X", keyID[i]);
	bufptr += 2;
    }
#else
    /* MSB-first keyID format */

    for (i = KEYFRAGSIZE - 4; i < KEYFRAGSIZE; i++) {
	sprintf(bufptr, "%02X", keyID[i]);
	bufptr += 2;
    }
#endif
    *bufptr = '\0';
    return keyIDbuf;
}				/* keyIDstring */



void extract_keyID(byteptr keyID, unitptr n)
/*
 * Extract key fragment from modulus n.  keyID byte array must be
 * at least KEYFRAGSIZE bytes long.
 */
{
    byte buf[MAX_BYTE_PRECISION + 2];
    short i, j;

    fill0(buf, KEYFRAGSIZE + 2);	/* in case n is too short */
    reg2mpi(buf, n);		/* MUST be at least KEYFRAGSIZE long */
#ifdef XLOWFIRST
    i = reg2mpi(buf, n);	/* MUST be at least KEYFRAGSIZE long */
    /* For LSB-first keyID format, start of keyID is: */
    i = 2;			/* skip over the 2 bytes of bitcount */
    for (j = 0; j < KEYFRAGSIZE;)
	keyID[j++] = buf[i++];
#else
    i = reg2mpi(buf, n);	/* MUST be at least KEYFRAGSIZE long */
    /* For MSB-first keyID format, start of keyID is: */
    i = i + 2 - KEYFRAGSIZE;
    for (j = 0; j < KEYFRAGSIZE;)
	keyID[j++] = buf[i++];
#endif

}				/* extract_keyID */



char *key2IDstring(unitptr n)
/*      Derive the key abbreviation fragment from the modulus n,
   and return printable string of key ID.
   n is key modulus from which to extract keyID.
 */
{
    byte keyID[KEYFRAGSIZE];
    extract_keyID(keyID, n);
    return keyIDstring(keyID);
}				/* key2IDstring */



static void showkeyID(byteptr keyID, FILE *pgpout)
/*      Print key fragment, which is an abbreviation of the public key. */
{
    fprintf(pgpout, "%s", keyIDstring(keyID));
}				/* showkeyID */



void writekeyID(unitptr n, FILE * f)
/*      Write message prefix keyID to a file.
   n is key modulus from which to extract keyID.
 */
{
    byte keyID[KEYFRAGSIZE];
    extract_keyID(keyID, n);
    fwrite(keyID, 1, KEYFRAGSIZE, f);
}				/* writekeyID */



static boolean checkkeyID(byte * keyID, unitptr n)
/*      Compare specified keyID with one derived from actual key modulus n. */
{
    byte keyID0[KEYFRAGSIZE];
    if (keyID == NULL)		/* no key ID -- assume a good match */
	return TRUE;
    extract_keyID(keyID0, n);
    return equal_buffers(keyID, keyID0, KEYFRAGSIZE);
}				/* checkkeyID */



/* external function prototype, from mpiio.c */
void dump_unit_array(string s, unitptr r);

void write_trust(FILE * f, byte trustbyte)
/*      Write a key control packet to f, with the specified trustbyte data.
 */
{
    putc(CTB_KEYCTRL, f);	/* Key control header byte */
    putc(1, f);			/* Key control length */
    putc(trustbyte, f);		/* Key control byte */
}

static
short writekeyfile(char *fname, struct IdeaCfbContext *cfb, word32 timestamp,
    byte * userid, unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
		   unitptr u)
/*      Write key components p, q, n, e, d, and u to specified file.
   hidekey is TRUE iff key should be encrypted.
   userid is a length-prefixed Pascal-type character string. 
   We write three packets: a key packet, a key control packet, and
   a userid packet.  We assume the key being written is our own,
   so we set the control bits for full trust.
 */
{
    FILE *f;
    byte ctb;
    byte alg, version;
    word16 validity;
    word16 cert_length;
    extern word16 mpi_checksum;
    byte iv[8];
    int i;

    /* open file f for write, in binary (not text) mode... */
    if ((f = fopen(fname, FOPWBIN)) == NULL) {
	fprintf(pgpout,
		LANG("\n\007Unable to create key file '%s'.\n"), fname);
	return -1;
    }
/*** Begin key certificate header fields ***/
    if (d == NULL) {
	/* public key certificate */
	ctb = CTB_CERT_PUBKEY;
	cert_length = 1 + SIZEOF_TIMESTAMP + 2 + 1 + (countbytes(n) + 2)
	    + (countbytes(e) + 2);
    } else {
	/* secret key certificate */
	ctb = CTB_CERT_SECKEY;
	cert_length = 1 + SIZEOF_TIMESTAMP + 2 + 1
	    + (countbytes(n) + 2)
	    + (countbytes(e) + 2)
	    + 1 + (cfb ? 8 : 0)	/* IDEA algorithm byte and IV */
	    +(countbytes(d) + 2)
	    + (countbytes(p) + 2) + (countbytes(q) + 2)
	    + (countbytes(u) + 2) + 2;

    }

    fwrite(&ctb, 1, 1, f);	/* write key certificate header byte */
    convert(cert_length);	/* convert to external byteorder */
    fwrite(&cert_length, 1, sizeof(cert_length), f);
    version = version_byte;
    fwrite(&version, 1, 1, f);	/* set version number */
    memcpy(iv, &timestamp, 4);
    convert_byteorder(iv, 4);	/* convert to external form */
    fwrite(iv, 1, 4, f);	/* write certificate timestamp */
    validity = 0;
    fwrite(&validity, 1, sizeof(validity), f);	/* validity period */
    alg = RSA_ALGORITHM_BYTE;
    fwrite(&alg, 1, 1, f);
    write_mpi(n, f, FALSE);
    write_mpi(e, f, FALSE);

    if (is_secret_key(ctb)) {	/* secret key */
	/* Write byte for following algorithm */
	alg = cfb ? IDEA_ALGORITHM_BYTE : 0;
	putc(alg, f);

	if (cfb) {		/* store encrypted IV */
	    for (i = 0; i < 8; i++)
		iv[i] = trueRandByte();
	    ideaCfbEncrypt(cfb, iv, iv, 8);
	    fwrite(iv, 1, 8, f);	/* write out the IV */
	}
	mpi_checksum = 0;
	write_mpi(d, f, cfb);
	write_mpi(p, f, cfb);
	write_mpi(q, f, cfb);
	write_mpi(u, f, cfb);
	/* Write checksum here - based on plaintext values */
	convert(mpi_checksum);
	fwrite(&mpi_checksum, 1, sizeof(mpi_checksum), f);
    } else {
	/* Keyring control packet, public keys only */
	write_trust(f, KC_OWNERTRUST_ULTIMATE | KC_BUCKSTOP);
    }
    /* User ID packet */
    ctb = CTB_USERID;
    fwrite(&ctb, 1, 1, f);	/* write userid header byte */
    fwrite(userid, 1, userid[0] + 1, f);	/* write user ID */
    if (d == NULL)		/* only on public keyring */
	write_trust(f, KC_LEGIT_COMPLETE);
    if (write_error(f)) {
	fclose(f);
	return -1;
    }
    fclose(f);
    if (verbose)
	fprintf(pgpout, "%d-bit %s key written to file '%s'.\n",
		countbits(n),
		is_secret_key(ctb) ? "secret" : "public",
		fname);
    return 0;
}				/* writekeyfile */

#ifdef EBCDIC
/* in RECFM=FB datasets fread() != 0 when eof() cause of padding 0x00 */
#define CTB_EOF 0x00
#else
#define CTB_EOF 0x1A
#endif

/* Return -1 on EOF, else read next key packet, return its ctb, and
 * advance pointer to beyond the packet.
 * This is short of a "short form" of readkeypacket
 */
short nextkeypacket(FILE * f, byte * pctb)
{
    word32 cert_length;
    int count;
    byte ctb;

    *pctb = 0;			/* assume no ctb for caller at first */
    count = fread(&ctb, 1, 1, f);	/* read key certificate CTB byte */
    if (count == 0)
	return -1;		/* premature eof */
    *pctb = ctb;		/* returns type to caller */
    if ((ctb != CTB_CERT_PUBKEY) && (ctb != CTB_CERT_SECKEY) &&
	(ctb != CTB_USERID) && (ctb != CTB_KEYCTRL) &&
	!is_ctb_type(ctb, CTB_SKE_TYPE) &&
	!is_ctb_type(ctb, CTB_COMMENT_TYPE))
	/* Either bad key packet or X/Ymodem padding detected */
	return (ctb == CTB_EOF) ? -1 : -2;

    cert_length = getpastlength(ctb, f);	/* read certificate length */

    if (cert_length > MAX_KEYCERT_LENGTH - 3)
	return -3;		/* bad length */

    fseek(f, cert_length, SEEK_CUR);
    return 0;
}				/* nextkeypacket */

/*
 * Reads a key certificate from the current file position of file f.
 * Depending on the certificate type, it will set the proper fields
 * of the return arguments.  Other fields will not be set.
 * pctb is always set.
 * If the packet is CTB_CERT_PUBKEY or CTB_CERT_SECKEY, it will
 * return timestamp, n, e, and if the secret key components are
 * present and d is not NULL, it will read, decrypt if hidekey is
 * true, and return d, p, q, and u.
 * If the packet is CTB_KEYCTRL, it will return keyctrl as that byte.
 * If the packet is CTB_USERID, it will return userid.
 * If the packet is CTB_COMMENT_TYPE, it won't return anything extra.
 * The file pointer is left positioned after the certificate.
 *
 * If the key could not be read because of a version error or bad
 * data, the return value is -6 or -4, the file pointer will be
 * positioned after the certificate, only the arguments pctb and
 * userid will valid in this case, other arguments are undefined.
 * Return value -3 means the error is unrecoverable.
 */
short readkeypacket(FILE * f, struct IdeaCfbContext *cfb, byte * pctb,
		    byte * timestamp, char *userid,
	unitptr n, unitptr e, unitptr d, unitptr p, unitptr q, unitptr u,
		    byte * sigkeyID, byte * keyctrl)
{
    byte ctb;
    word16 cert_length;
    int count;
    byte version, alg, mdlen;
    word16 validity;
    word16 chksum;
    extern word16 mpi_checksum;
    long next_packet;
    byte iv[8];

/*** Begin certificate header fields ***/
    *pctb = 0;			/* assume no ctb for caller at first */
    count = fread(&ctb, 1, 1, f);	/* read key certificate CTB byte */
    if (count == 0)
	return -1;		/* premature eof */
    *pctb = ctb;		/* returns type to caller */
    if ((ctb != CTB_CERT_PUBKEY) && (ctb != CTB_CERT_SECKEY) &&
	(ctb != CTB_USERID) && (ctb != CTB_KEYCTRL) &&
	!is_ctb_type(ctb, CTB_SKE_TYPE) &&
	!is_ctb_type(ctb, CTB_COMMENT_TYPE))
	/* Either bad key packet or X/Ymodem padding detected */
	return (ctb == CTB_EOF) ? -1 : -2;

    cert_length = getpastlength(ctb, f);	/* read certificate length */

    if (cert_length > MAX_KEYCERT_LENGTH - 3)
	return -3;		/* bad length */

    next_packet = ftell(f) + cert_length;

    /*
     * skip packet and return, keeps us in sync when we hit a
     * version error or bad data.  Implemented oddly to make it
     * only one statement.
     */
#define SKIP_RETURN(x) return fseek(f, next_packet, SEEK_SET), x

    if (ctb == CTB_USERID) {
	if (cert_length > 255)
	    return -3;		/* Bad length error */
	if (userid) {
	    userid[0] = cert_length;	/* Save user ID length */
	    fread(userid + 1, 1, cert_length, f);    /* read rest of user ID */
	} else
	    fseek(f, (long) cert_length, SEEK_CUR);
	return 0;		/* normal return */

    } else if (is_ctb_type(ctb, CTB_SKE_TYPE)) {

	if (sigkeyID) {
	    fread(&version, 1, 1, f);	/* Read version of sig packet */
	    if (version_byte_error(version))
		SKIP_RETURN(-6);	/* Need a later version */
	    /* Skip timestamp, validity period, and type byte */
	    fread(&mdlen, 1, 1, f);
	    fseek(f, (long) mdlen, SEEK_CUR);
	    /* Read and return KEY ID */
	    fread(sigkeyID, 1, KEYFRAGSIZE, f);
	}
	SKIP_RETURN(0);		/* normal return */

    } else if (ctb == CTB_KEYCTRL) {

	if (cert_length != 1)
	    return -3;		/* Bad length error */
	if (keyctrl)
	    fread(keyctrl, 1, cert_length, f);	/* Read key control byte */
	else
	    fseek(f, (long) cert_length, SEEK_CUR);
	return 0;		/* normal return */

    } else if (!is_key_ctb(ctb))	/* comment or other packet */
	SKIP_RETURN(0);		/* normal return */

    /* Here we have a key packet */
    if (n != NULL)
	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */
    fread(&version, 1, 1, f);	/* read and check version */
    if (version_byte_error(version))
	SKIP_RETURN(-6);	/* Need a later version */
    if (timestamp) {
	fread(timestamp, 1, SIZEOF_TIMESTAMP, f); /* read certificate
						     timestamp */
	convert_byteorder(timestamp, SIZEOF_TIMESTAMP);	/* convert from
							   external form */
    } else {
	fseek(f, (long) SIZEOF_TIMESTAMP, SEEK_CUR);
    }
    fread(&validity, 1, sizeof(validity), f);	/* Read validity period */
    convert(validity);		/* convert from external byteorder */
    /* We don't use validity period yet */
    fread(&alg, 1, 1, f);
    if (version_error(alg, RSA_ALGORITHM_BYTE))
	SKIP_RETURN(-6);	/* Need a later version */
/*** End certificate header fields ***/

    /* We're past certificate headers, now look at some key material... */

    cert_length -= 1 + SIZEOF_TIMESTAMP + 2 + 1;

    if (n == NULL)		/* Skip key certificate data */
	SKIP_RETURN(0);

    if (read_mpi(n, f, TRUE, FALSE) < 0)
	SKIP_RETURN(-4);	/* data corrupted, return error */

    /* Note that precision was adjusted for n */

    if (read_mpi(e, f, FALSE, FALSE) < 0)
	SKIP_RETURN(-4);	/* data corrupted, error return */

    cert_length -= (countbytes(n) + 2) + (countbytes(e) + 2);

    if (d == NULL) {		/* skip rest of this key certificate */
        if (cert_length && !is_secret_key(ctb))
	    SKIP_RETURN(-4);	/* key w/o userID */
        else
	    SKIP_RETURN(0);	/* Normal return */
    }

    if (is_secret_key(ctb)) {
	fread(&alg, 1, 1, f);
	if (alg && version_error(alg, IDEA_ALGORITHM_BYTE))
	    SKIP_RETURN(-6);	/* Unknown version */

	if (!cfb && alg)
	    /* Don't bother trying if hidekey is false and alg is true */
	    SKIP_RETURN(-5);

	if (alg) {		/* if secret components are encrypted... */
	    /* process encrypted CFB IV before reading secret components */
	    count = fread(iv, 1, 8, f);
	    if (count < 8)
		return -4;	/* data corrupted, error return */

	    ideaCfbDecrypt(cfb, iv, iv, 8);
	    cert_length -= 8;	/* take IV length into account */
	}
	/* Reset checksum before these reads */
	mpi_checksum = 0;

	if (read_mpi(d, f, FALSE, cfb) < 0)
	    return -4;		/* data corrupted, error return */
	if (read_mpi(p, f, FALSE, cfb) < 0)
	    return -4;		/* data corrupted, error return */
	if (read_mpi(q, f, FALSE, cfb) < 0)
	    return -4;		/* data corrupted, error return */

	/* use register 'u' briefly as scratchpad */
	mp_mult(u, p, q);	/* compare p*q against n */
	if (mp_compare(n, u) != 0)	/* bad pass phrase? */
	    return -5;		/* possible bad pass phrase, error return */
	/* now read in real u */
	if (read_mpi(u, f, FALSE, cfb) < 0)
	    return -4;		/* data corrupted, error return */

	/* Read checksum, compare with mpi_checksum */
	fread(&chksum, 1, sizeof(chksum), f);
	convert(chksum);
	if (chksum != mpi_checksum)
	    return -5;		/* possible bad pass phrase */

	cert_length -= 1 + (countbytes(d) + 2) + (countbytes(p) + 2)
	    + (countbytes(q) + 2) + (countbytes(u) + 2) + 2;

    } else {			/* not a secret key */

	mp_init(d, 0);
	mp_init(p, 0);
	mp_init(q, 0);
	mp_init(u, 0);
    }

    if (cert_length != 0) {
	fprintf(pgpout, "\n\007Corrupted key.  Bad length, off by %d bytes.\n",
		(int) cert_length);
	SKIP_RETURN(-4);	/* data corrupted, error return */
    }
    return 0;			/* normal return */

}				/* readkeypacket */

/*
 * keyID contains key fragment we expect to find in keyfile.
 * If keyID is NULL, then userid contains a C string search target of
 * userid to find in keyfile.
 * keyfile is the file to begin search in, and it may be modified
 * to indicate true filename of where the key was found.  It can be
 * either a public key file or a secret key file.
 * file_position is returned as the byte offset within the keyfile
 * that the key was found at.  pktlen is the length of the key packet.
 * These values are for the key packet itself, not including any
 * following userid, control, signature, or comment packets.
 *
 * possible flags:
 * GPK_GIVEUP: we are just going to do a single file search only.
 * GPK_SHOW: show the key if found.
 * GPK_NORVK: skip revoked keys.
 * GPK_DISABLED: don't ignore disabled keys (when doing userid lookup)
 * GPK_SECRET: looking for a secret key
 *
 * Returns -6 if the key was found but the key was not read because of a
 * version error or bad data.  The arguments timestamp, n and e are
 * undefined in this case.
 */
int getpublickey(int flags, char *keyfile, long *_file_position,
	     int *_pktlen, byte * keyID, byte * timestamp, byte * userid,
		 unitptr n, unitptr e)
{
    byte ctb;			/* returned by readkeypacket */
    FILE *f;
    int status, keystatus = -1;
    boolean keyfound = FALSE;
    char matchid[256];		/* C string format */
    long fpos;
    long file_position = 0;
    int pktlen = 0;
    boolean skip = FALSE;	/* if TRUE: skip until next key packet */
    byte keyctrl;
#ifdef MACTC5
	boolean use_pubring2;
	use_pubring2 = (globalPubringName2[0] != '\0');
#endif

    if (keyID == NULL)		/* then userid has search target */
	strcpy(matchid, (char *) userid);
    else
	matchid[0] = '\0';

  top:
    if (strlen(keyfile) == 0)	/* null filename */
	return -1;		/* give up, error return */

    if (!file_exists(keyfile))
        default_extension(keyfile, PGP_EXTENSION);

    if (!file_exists(keyfile)) {
	if (flags & GPK_GIVEUP)
	    return -1;		/* give up, error return */
	fprintf(pgpout, LANG("\n\007Keyring file '%s' does not exist. "),
		keyfile);
        fprintf(pgpout, "\n");
	goto nogood;
    }
    if (verbose) {
	fprintf(pgpout, "searching key ring file '%s' ", keyfile);
	if (keyID)
	    fprintf(pgpout, "for keyID %s\n", keyIDstring(keyID));
	else
	    fprintf(pgpout, "for userid \"%s\"\n", LOCAL_CHARSET(userid));
    }
    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(keyfile, FOPRBIN)) == NULL)
	return -1;		/* error return */

    keyfound = FALSE;
    for (;;) {
	fpos = ftell(f);
	status = readkeypacket(f, FALSE, &ctb, timestamp, (char *) userid,
			       n, e, NULL, NULL, NULL, NULL, NULL, NULL);
	/* Note that readkeypacket has called set_precision */

	if (status == -1)	/* end of file */
	    break;

	if (status < -1 && status != -4 && status != -6) {
	    fprintf(pgpout, LANG("\n\007Could not read key from file '%s'.\n"),
		    keyfile);
	    fclose(f);		/* close key file */
	    return status;
	}
	/* Remember packet position and size for last key packet */
	if (is_key_ctb(ctb)) {
	    file_position = fpos;
	    pktlen = (int) (ftell(f) - fpos);
	    keystatus = status;
	    if (!keyID && !(flags & GPK_DISABLED) &&
		(is_ctb_type(ctb, CTB_CERT_SECKEY_TYPE) ||
		 is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE)) &&
		read_trust(f, &keyctrl) == 0 &&
		(keyctrl & KC_DISABLED))
		skip = TRUE;
	    else
		skip = FALSE;
	}
	/* Only check for matches when we find a USERID packet */
	if (!skip && ctb == CTB_USERID) {
#ifdef MACTC5
		mac_poll_for_break();
#endif
  /* keyID contains key fragment.  Check it against n from keyfile. */
	    if (keyID != NULL) {
		if (keystatus == 0)
		    keyfound = checkkeyID(keyID, n);
	    } else {
		/* matchid is already a C string */
		PascalToC((char *) userid);	/* for C string functions */
		/* Accept any matching subset */
		keyfound = userid_match((char *) userid, matchid, n);
		CToPascal((char *) userid);
	    }
	}
	if (keyfound) {
	    if (flags & GPK_SHOW)
		show_key(f, file_position, 0);
	    fseek(f, file_position, SEEK_SET);
	    if ((flags & GPK_NORVK) && keystatus == 0 && is_compromised(f)) {
		if (flags & GPK_SHOW) {		/* already printed user ID */
		    fprintf(pgpout,
	       LANG("\n\007Sorry, this key has been revoked by its owner.\n"));
		} else {
		    PascalToC((char *) userid);
		    fprintf(pgpout, LANG("\nKey for user ID \"%s\"\n\
has been revoked.  You cannot use this key.\n"),
			    LOCAL_CHARSET((char *) userid));
		}
		keyfound = FALSE;
		skip = TRUE;
		/* we're positioned at the key packet, skip it */
		nextkeypacket(f, &ctb);
	    } else {
		/* found key, normal return */
		if (_pktlen)
		    *_pktlen = pktlen;
		if (_file_position)
		    *_file_position = file_position;
		fclose(f);
		return keystatus;
	    }
	}
    }				/* while TRUE */

    fclose(f);			/* close key file */

    if (flags & GPK_GIVEUP)
	return -1;		/* give up, error return */

    if (keyID != NULL) {
	fprintf(pgpout,
       LANG("\n\007Key matching expected Key ID %s not found in file '%s'.\n"),
		keyIDstring(keyID), keyfile);
    } else {
	fprintf(pgpout,
	    LANG("\n\007Key matching userid '%s' not found in file '%s'.\n"),
		LOCAL_CHARSET(matchid), keyfile);
    }

  nogood:
    if (filter_mode || batchmode)
	return -1;		/* give up, error return */

#ifdef MACTC5
	{
	Boolean result;
	if (flags & GPK_SECRET)
		result=GetFilePath(LANG("Enter secret key filename: "), keyfile, GETFILE);
	else if (use_pubring2) {
		strcpy(keyfile,globalPubringName2);
		use_pubring2 = false;
		result = true;
	} else
		result=GetFilePath(LANG("Enter public key filename: "), keyfile, GETFILE);
	if (!result) strcpy(keyfile,"");
	if (flags & GPK_SECRET)
		fprintf(pgpout,LANG("Enter secret key filename: "));
	else
		fprintf(pgpout,LANG("Enter public key filename: "));
	fprintf(pgpout, "%s\n",keyfile);
	}
#else  
    if (flags & GPK_SECRET)
	fprintf(pgpout, LANG("Enter secret key filename: "));
    else
	fprintf(pgpout, LANG("Enter public key filename: "));

    getstring(keyfile, 59, TRUE);	/* echo keyboard input */
#endif
    goto top;

}				/* getpublickey */

/*  Start at key_position in keyfile, and scan for the key packet
   that contains userid.  Return userid_position and userid_len.
   Return 0 if OK, -1 on error.  Userid should be a C string.
   If exact_match is TRUE, the userid must match for full length,
   a substring is not enough.
 */
int getpubuserid(char *keyfile, long key_position, byte * userid,
	     long *userid_position, int *userid_len, boolean exact_match)
{
    unit n[MAX_UNIT_PRECISION];
    unit e[MAX_UNIT_PRECISION];
    byte ctb;			/* returned by readkeypacket */
    FILE *f;
    int status;
    char userid0[256];		/* C string format */
    long fpos;

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(keyfile, FOPRBIN)) == NULL)
	return -1;		/* error return */

    /* Start off at correct location */
    fseek(f, key_position, SEEK_SET);
    (void) nextkeypacket(f, &ctb);	/* Skip key */
    for (;;) {
	fpos = ftell(f);
	status = readkeypacket(f, FALSE, &ctb, NULL, (char *) userid0, n, e,
			       NULL, NULL, NULL, NULL, NULL, NULL);

	if (status < 0 || is_key_ctb(ctb)) {
	    fclose(f);		/* close key file */
	    return status ? status : -1;	/* give up, error return */
	}
	/* Only check for matches when we find a USERID packet */
	if (ctb == CTB_USERID) {
	    if (userid[0] == '0' && userid[1] == 'x')
		break;	       /* use first userid if user specified a keyID */
	    /* userid is already a C string */
	    PascalToC((char *) userid0);	/* for C string functions */
	    /* Accept any matching subset if exact_match is FALSE */
	    if (userid_match((char *) userid0, (char *) userid,
			     (exact_match ? NULL : n)))
		break;
	}
    }				/* for(;;) */
    *userid_position = fpos;
    *userid_len = (int) (ftell(f) - fpos);
    fclose(f);
    return 0;			/* normal return */
}				/* getpubuserid */

/*
 * Start at user_position in keyfile, and scan for the signature packet
 * that matches sigkeyID.  Return the signature timestamp, sig_position
 * and sig_len.
 *
 * Return 0 if OK, -1 on error.
 */
int getpubusersig(char *keyfile, long user_position, byte * sigkeyID,
		  byte * timestamp, long *sig_position, int *sig_len)
{
    byte ctb;			/* returned by readkeypacket */
    FILE *f;
    int status;
    byte keyID0[KEYFRAGSIZE];
    long fpos;

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(keyfile, FOPRBIN)) == NULL)
	return -1;		/* error return */

    /* Start off at correct location */
    fseek(f, user_position, SEEK_SET);
    (void) nextkeypacket(f, &ctb);	/* Skip userid packet */
    for (;;) {
	fpos = ftell(f);
	status = readkeypacket(f, FALSE, &ctb, NULL, NULL, NULL, NULL,
			       NULL, NULL, NULL, NULL, keyID0, NULL);

	if (status < 0 || is_key_ctb(ctb) || ctb == CTB_USERID)
	    break;

	/* Only check for matches when we find a signature packet */
	if (is_ctb_type(ctb, CTB_SKE_TYPE)) {
	    if (equal_buffers(sigkeyID, keyID0, KEYFRAGSIZE)) {
		*sig_position = fpos;
		*sig_len = (int) (ftell(f) - fpos);
		fseek(f, fpos + 6, SEEK_SET);
		fread(timestamp, 1, SIZEOF_TIMESTAMP, f); /* read certificate
							     timestamp */
		convert_byteorder(timestamp, SIZEOF_TIMESTAMP);	/* convert
							       from external 
							       orm */
		fclose(f);
		return 0;	/* normal return */
	    }
	}
    }				/* for (;;) */

    fclose(f);			/* close key file */
    return status ? status : -1;	/* give up, error return */
}				/* getpubusersig */

#ifdef MACTC5
/* Truncated version of getsecretkey used to get default userid during
   initialization.  Does not annoy user by asking for password. */
int getfirstsecretkey(boolean giveup, boolean showkey, char *keyfile, byte *keyID,
	byte *timestamp, char *passp, boolean *hkey,
	byte *userid, unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
	unitptr u)
{
	char keyfilename[MAX_PATH];	/* for getpublickey */
	long file_position;
	int pktlen;	/* unused, just to satisfy getpublickey */

	if (keyfile == NULL)
	{	/* use default pathname */
		buildfilename(keyfilename,globalSecringName);
		keyfile = keyfilename;
	}

	return(getpublickey(GPK_GIVEUP, keyfile, &file_position, &pktlen,
			keyID, timestamp, userid, n, e));
}
#endif

/*
 * keyID contains key fragment we expect to find in keyfile.
 * If keyID is NULL, then userid contains search target of
 * userid to find in keyfile.
 * giveup controls whether we ask the user for the name of the
 * secret key file on failure.  showkey controls whether we print
 * out the key information when we find it.  keyfile, if non-NULL,
 * is the name of the secret key file; if NULL, we use the
 * default.  hpass and hkey, if non-NULL, get returned with a copy
 * of the hashed password buffer and hidekey variable.
 */
int getsecretkey(int flags, char *keyfile, byte * keyID,
	   byte * timestamp, byte * hpass, boolean * hkey, byte * userid,
		 unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
		 unitptr u)
{
    byte ctb;			/* returned by readkeypacket */
    FILE *f;
    char keyfilename[MAX_PATH];	/* for getpublickey */
    long file_position;
    int status;
    boolean hidekey;		/* TRUE iff secret key is encrypted */
    word16 iv[4];		/* initialization vector for encryption */
    byte ideakey[16];
    int guesses;
    struct hashedpw *hpw, **hpwp;
    struct IdeaCfbContext cfb;

    if (keyfile == NULL) {
	/* use default pathname */
	strcpy(keyfilename, globalSecringName);
	keyfile = keyfilename;
    }
    status = getpublickey(flags | GPK_SECRET, keyfile, &file_position,
			  NULL, keyID, timestamp, userid, n, e);
    if (status < 0)
	return status;		/* error return */

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(keyfile, FOPRBIN)) == NULL)
	return -1;		/* error return */

    /* First guess is no password */
    hidekey = FALSE;
    fseek(f, file_position, SEEK_SET);	/* reposition file to key */
    status = readkeypacket(f, 0, &ctb, timestamp, (char *) userid,
			   n, e, d, p, q, u, NULL, NULL);
    if (status != -5)		/* Anything except bad password */
	goto done;

    /* If we're not signing a key (when we force asking the user),
     * check the prevosuly known passwords.
     */
    if (!(flags & GPK_ASKPASS)) {
	hidekey = TRUE;
	/* Then come existing key passwords */
	hpw = keypasswds;
	while (hpw) {
	    ideaCfbInit(&cfb, hpw->hash);
	    fseek(f, file_position, SEEK_SET);
	    status = readkeypacket(f, &cfb, &ctb, timestamp,
			  (char *) userid, n, e, d, p, q, u, NULL, NULL);
	    ideaCfbDestroy(&cfb);
	    if (status != -5) {
		memcpy(ideakey, hpw->hash, sizeof(ideakey));
		goto done;
	    }
	    hpw = hpw->next;
	}
	/* Then try "other" passwords" */
	hpwp = &passwds;
	hpw = *hpwp;
	while (hpw) {
	    ideaCfbInit(&cfb, hpw->hash);
	    fseek(f, file_position, SEEK_SET);
	    status = readkeypacket(f, &cfb, &ctb, timestamp,
			  (char *) userid, n, e, d, p, q, u, NULL, NULL);
	    ideaCfbDestroy(&cfb);
	    if (status >= 0) {
		/* Success - move to key password list */
		memcpy(ideakey, hpw->hash, sizeof(ideakey));
		*hpwp = hpw->next;
		hpw->next = keypasswds;
		keypasswds = hpw;
	    }
	    if (status != -5)
		goto done;
	    hpwp = &hpw->next;
	    hpw = *hpwp;
	}
    }
    /* If batchmode, we don't ask the user. */
    if (batchmode) {
	/* PGPPASS (or -z) wrong or not set */
	fprintf(pgpout, LANG("\n\007Error:  Bad pass phrase.\n"));
	fclose(f);		/* close key file */
	return -1;
    }
    /* Finally, prompt the user. */
    fprintf(pgpout,
	    LANG("\nYou need a pass phrase to unlock your RSA secret key. "));
    if (!(flags & GPK_SHOW)) {
	/* let user know for which key he should type his password */
	PascalToC((char *) userid);
	fprintf(pgpout, LANG("\nKey for user ID: %s\n"),
		LOCAL_CHARSET((char *) userid));
	fprintf(pgpout, LANG("%d-bit key, key ID %s, created %s\n"),
		countbits(n), key2IDstring(n), cdate((word32 *) timestamp));
	CToPascal((char *) userid);
    }
    guesses = 0;
    for (;;) {
	if (++guesses > 3)
	    hidekey = 0;
	else
	    hidekey = (GetHashedPassPhrase((char *)ideakey, 1) > 0);
	/*
	 * We've already tried the null password - interpret
	 * a null string as "I dunno".
	 */
	if (!hidekey) {
	    status = -5;	/* Bad passphrase */
	    fputs(LANG("No passphrase; secret key unavailable.\n"),
		  pgpout);
	    break;
	}
	ideaCfbInit(&cfb, ideakey);
	fseek(f, file_position, SEEK_SET);
	status = readkeypacket(f, &cfb, &ctb, timestamp,
			  (char *) userid, n, e, d, p, q, u, NULL, NULL);
	ideaCfbDestroy(&cfb);
	if (status >= 0) {
#ifdef MACTC5
		;
	}
		if (Abort) guesses=1;
#else
	    /* Success - remember this key for later use */
	    if (flags & GPK_ASKPASS) {
		/*
		 * This may be a duplicate because we didn't
		 * search the lists before - check.
		 */
		hpw = passwds;
		while (hpw) {
		    if (memcmp(hpw->hash, ideakey,
			       sizeof(ideakey)) == 0)
			goto done;
		    hpw = hpw->next;
		}
		hpw = keypasswds;
		while (hpw) {
		    if (memcmp(hpw->hash, ideakey,
			       sizeof(ideakey)) == 0)
			goto done;
		    hpw = hpw->next;
		}
	    }
	    /* Insert new key into remember lists. */
	    hpw = (struct hashedpw *) malloc(sizeof(struct hashedpw));
	    if (hpw) {
		/* If malloc fails, just don't remember the phrase */
		memcpy(hpw->hash, ideakey, sizeof(hpw->hash));
		hpw->next = keypasswds;
		keypasswds = hpw;
	    }
	}
#endif /* MACTC5 */
	if (status != -5)
	    goto done;
#ifdef MACTC5
	passhash[0] = '\0';
#endif
	fprintf(pgpout, LANG("\n\007Error:  Bad pass phrase.\n"));
    }
    while (--guesses);
    /* Failed - fall through to done */

  done:
    fclose(f);
    if (hkey)
	*hkey = hidekey;
    if (status == -5)
	return status;
    if (status < 0) {
	fprintf(pgpout, LANG("\n\007Could not read key from file '%s'.\n"),
		keyfile);
	fclose(f);		/* close key file */
	return -1;
    }
    if (hpass)
	memcpy(hpass, ideakey, sizeof(ideakey));
    burn(ideakey);

    /* Note that readkeypacket has called set_precision */

    if (d != NULL) {	/* No effective check of pass phrase if d is NULL */
	if (!quietmode) {
	    if (!hidekey)
		fprintf(pgpout,
LANG("\nAdvisory warning: This RSA secret key is not protected by a \
passphrase.\n"));
	    else
		fprintf(pgpout, LANG("Pass phrase is good.  "));
	}
	if (testeq(d, 0)) {	/* didn't get secret key components */
	    fprintf(pgpout,
		    LANG("\n\007Key file '%s' is not a secret key file.\n"),
		    keyfile);
	    return -1;
	}
    }
    return 0;			/* normal return */

}				/* getsecretkey */

/*
 * check if a key has a compromise certificate, file pointer must
 * be positioned at or right after the key packet.
 */
int is_compromised(FILE * f)
{
    long pos, savepos;
    byte class, ctb;
    int cert_len;
    int status = 0;

    pos = savepos = ftell(f);

    nextkeypacket(f, &ctb);
    if (is_key_ctb(ctb)) {
	pos = ftell(f);
	nextkeypacket(f, &ctb);
    }
    if (ctb != CTB_KEYCTRL)
	fseek(f, pos, SEEK_SET);

    /* file pointer now positioned where compromise cert. should be */
    if (fread(&ctb, 1, 1, f) != 1) {
	status = -1;
	goto ex;
    }
    if (is_ctb_type(ctb, CTB_SKE_TYPE)) {
	cert_len = (int) getpastlength(ctb, f);
	if (cert_len > MAX_SIGCERT_LENGTH) {	/* Huge packet length */
	    status = -1;
	    goto ex;
	}
	/* skip version and mdlen byte */
	fseek(f, 2L, SEEK_CUR);
	if (fread(&class, 1, 1, f) != 1) {
	    status = -1;
	    goto ex;
	}
	status = (class == KC_SIGNATURE_BYTE);
    }
  ex:
    fseek(f, savepos, SEEK_SET);
    return status;
}


/*      Alfred Hitchcock coined the term "mcguffin" for the generic object 
   being sought in his films-- the diamond, the microfilm, etc. 
 */


/*
 * Calculate and display a hash for the public components of the key.
 * The components are converted to their external (big-endian) 
 * representation, concatenated, and an MD5 on the bit values 
 * (i.e. excluding the length value) calculated and displayed in hex.
 *
 * The hash, or "fingerprint", of the key is useful mainly for quickly
 * and easily verifying over the phone that you have a good copy of 
 * someone's public key.  Just read the hash over the phone and have
 * them check it against theirs.
 */
void getKeyHash(byte * hash, unitptr n, unitptr e)
{
    struct MD5Context mdContext;
    byte buffer[MAX_BYTE_PRECISION + 2];
    byte mdBuffer[MAX_BYTE_PRECISION * 2];
    int i, mdIndex = 0, bufIndex;

/* Convert n and e to external (big-endian) byte order and move to mdBuffer */
    i = reg2mpi(buffer, n);
    for (bufIndex = 2; bufIndex < i + 2; bufIndex++)	/* +2 skips count */
	mdBuffer[mdIndex++] = buffer[bufIndex];
    i = reg2mpi(buffer, e);
    for (bufIndex = 2; bufIndex < i + 2; bufIndex++)	/* +2 skips count */
	mdBuffer[mdIndex++] = buffer[bufIndex];

    /* Now evaluate the MD5 for the two MPI's */
    MD5Init(&mdContext);
    MD5Update(&mdContext, mdBuffer, mdIndex);
    MD5Final(hash, &mdContext);

}				/* getKeyHash */


void printKeyHash(byteptr hash, boolean indent)
{
    int i;
#ifdef MACTC5
	char	str[256],*start=str;
#endif

/*      Display the hash.  The format is:
   pub  1024/xxxxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
   Key fingerprint = xx xx xx xx xx xx xx xx  xx xx xx xx xx xx xx xx 
 */
    fprintf(pgpout, "%*s", indent ? 29 : 1, LANG("Key fingerprint ="));
    for (i = 0; i < 8; i++)
	fprintf(pgpout, " %02X", hash[i]);
    putc(' ', pgpout);
    for (i = 8; i < 16; i++)
	fprintf(pgpout, " %02X", hash[i]);
    putc('\n', pgpout);

#ifdef MACTC5
	start+=sprintf(start, "%s", LANG("Key fingerprint =" ) );	/* CP */
	for( i = 0; i < 8; i++ )
		start+=sprintf(start, "%02X ", hash[ i ] );
	*(start++)=' ';
	for( i = 8; i < 16; i++ )
		start+=sprintf(start, "%02X ", hash[ i ] );
	*(--start)=0;					/* Remove trailing space */
	AddResult(str);
#endif
}				/* printKeyHash */


void showKeyHash(unitptr n, unitptr e)
{
    byte hash[16];

    getKeyHash(hash, n, e);	/* compute hash of (n,e) */

    printKeyHash(hash, TRUE);
}				/* showKeyHash */

/*
 * Lists all entries in keyring that have mcguffin string in userid.
 * mcguffin is a null-terminated C string.
 */
int view_keyring(char *mcguffin, char *ringfile, boolean show_signatures,
		 boolean show_hashes)
{
    FILE *f;
    int status;
    char dfltring[MAX_PATH];
    int keycounter = 0;
    FILE *savepgpout;

    /* Default keyring to check signature ID's */
    strcpy(dfltring, globalPubringName);

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout,
		LANG("\n\007Can't open key ring file '%s'\n"), ringfile);
	return -1;
    }
    if (show_signatures) {
	setkrent(ringfile);
	setkrent(dfltring);
	init_userhash();
    }
/*      Here's a good format for display of key or signature certificates:
   Type bits/keyID    Date       User ID
   pub  1024/xxxxxxxx yyyy-mm-dd aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
   sec   512/xxxxxxxx yyyy-mm-dd aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
   sig   384/xxxxxxxx yyyy-mm-dd aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 */

    /* XXX Send this to stdout.  Do we always want to do this?
     * Why couldn't we have a PGPOUT and PGPERR, and then we wouldn't
     * have this problem?  -warlord
     */
    savepgpout = pgpout;
    pgpout = stdout;

    if (moreflag)
	open_more();
    if (!quietmode) {
	fprintf(pgpout, LANG("\nKey ring: '%s'"), ringfile);
	if (mcguffin && strlen(mcguffin) > 0)
	    fprintf(pgpout,
		    LANG(", looking for user ID \"%s\"."),
		    LOCAL_CHARSET(mcguffin));
    }

    fprintf(pgpout, "\n");
    kv_title(pgpout);
    status = kvformat_keypacket(f, pgpout, FALSE, mcguffin, ringfile,
                                show_signatures, show_hashes, &keycounter);

    fclose(f);			/* close key file */
    if (show_signatures)
	endkrent();
    if (keycounter == 1)
	fprintf(pgpout, LANG("1 matching key found.\n"));
    else
	fprintf(pgpout, LANG("%d matching keys found.\n"), keycounter);
    close_more();
    pgpout = savepgpout;

    if (status < 0)
	return status;
    if (mcguffin != NULL && *mcguffin != '\0') {
	/* user specified substring */
	if (keycounter == 0)
	    return 67;		/* user not found */
	else if (keycounter > 1)
	    return 1;		/* more than one match */
    }
    return 0;			/* normal return */

}				/* view_keyring */

/*      Lists all entries in keyring that have mcguffin string in userid.
   mcguffin is a null-terminated C string.
   If options is CHECK_NEW, only new signatures are checked and are
   marked as being checked in the trustbyte (called from addto_keyring).
 */
int dokeycheck(char *mcguffin, char *ringfile, int options)
{
    FILE *f, *fixedf = NULL;
    byte ctb, keyctb = 0;
    long fpsig = 0, fpkey = 0, fixpos = 0, trustpos = -1;
    int status, sigstatus;
    int keypktlen = 0, sigpktlen = 0;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    byte keyID[KEYFRAGSIZE];
    byte sigkeyID[KEYFRAGSIZE];
    byte keyuserid[256];	/* key certificate userid */
    byte siguserid[256];	/* sig certificate userid */
    char dfltring[MAX_PATH];
    char *tempring = NULL;
    word32 tstamp;
    byte *timestamp = (byte *) & tstamp;	/* key certificate timestamp */
    word32 sigtstamp;
    byte *sigtimestamp = (byte *) & sigtstamp;
    byte sigclass;
    int firstuser = 0;
    int compromised = 0;
    boolean invalid_key = FALSE;	/* unsupported version or bad data */
    boolean failed = FALSE;
    boolean print_userid = FALSE;
    byte sigtrust, newtrust;
    FILE *savepgpout;

    /* Default keyring to check signature ID's */
    strcpy(dfltring, globalPubringName);

    /* open file f, in binary (not text) mode... */
    f = fopen(ringfile, FOPRWBIN);
    if (f == NULL) {
	fprintf(pgpout,
		LANG("\n\007Can't open key ring file '%s'\n"), ringfile);
	return -1;
    }
/*      Here's a good format for display of key or signature certificates:
   Type bits/keyID   Date       User ID
   pub  1024/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
   sec   512/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
   sig   384/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 */

    /* XXX Send this to stdout.  Do we always want to do this?
     * Why couldn't we have a PGPOUT and PGPERR, and then we wouldn't
     * have this problem?  -warlord
     */
    savepgpout = pgpout;
    pgpout = stdout;

    if (options & CHECK_NEW) {
	fprintf(pgpout, LANG("\nChecking signatures...\n"));
    } else {
	if (moreflag)
	    open_more();
	if (!quietmode) {
	    fprintf(pgpout, LANG("\nKey ring: '%s'"), ringfile);
	    if (mcguffin && strlen(mcguffin) > 0)
		fprintf(pgpout, LANG(", looking for user ID \"%s\"."),
			LOCAL_CHARSET(mcguffin));
	}
        fprintf(pgpout, "\n");
        kv_title(pgpout);
    }
    for (;;) {
	long fpos = ftell(f);
	status = readkeypacket(f, FALSE, &ctb, timestamp, (char *) keyuserid,
			       n, e,
			       NULL, NULL, NULL, NULL, sigkeyID, NULL);
	/* Note that readkeypacket has called set_precision */
	if (status == -1)
	    break;		/* eof reached */
	if (status == -4 || status == -6) {
	    /* only ctb and userid are valid */
	    memset(sigkeyID, 0, KEYFRAGSIZE);
	    tstamp = 0;
	} else if (status < 0) {
	    fprintf(pgpout, LANG("\n\007Could not read key from file '%s'.\n"),
		    ringfile);
	    fclose(f);		/* close key file */
	    return -1;
	}
	if (is_key_ctb(ctb)) {
	    firstuser = 1;
	    keyctb = ctb;
	    fpkey = fpos;
	    keypktlen = (int) (ftell(f) - fpkey);
	    compromised = is_compromised(f);
	    if (status < 0) {
		invalid_key = TRUE;
		memset(keyID, 0, KEYFRAGSIZE);
	    } else {
		invalid_key = FALSE;
		extract_keyID(keyID, n);
	    }
	    if (options & CHECK_NEW)
		print_userid = TRUE;
	}
	if (ctb == CTB_USERID) {
#ifdef MACTC5
		mac_poll_for_break();
#endif
	    PascalToC((char *) keyuserid);
	} else if (is_ctb_type(ctb, CTB_SKE_TYPE)) {
	    fpsig = fpos;
	    sigpktlen = (int) (ftell(f) - fpsig);
	} else {
	    continue;
	}

	trustpos = ftell(f);
	status = read_trust(f, &sigtrust);
	if (status == -1)
	    break;		/* EOF */
	if (status == -7) {
	    trustpos = -1;
	    continue;	    /* not a keyring or this was a compromise cert. */
	}
	if (status < 0) {
	    fclose(f);
	    return status;
	}
	if (options & CHECK_NEW) {
	    if (!is_ctb_type(ctb, CTB_SKE_TYPE))
		continue;
	    if (sigtrust & KC_SIG_CHECKED)
		continue;
	    /* addto_keyring has called setkrent() */
	    if (user_from_keyID(sigkeyID) == NULL)
		continue;	/* unknown signator */
	}
	/* If we don't list the signatures, continue */
	if (!(options & CHECK_NEW) &&
	    !userid_match((char *) keyuserid, mcguffin, n))
	    continue;

	if (ctb == CTB_USERID || print_userid) {
	    /* CHECK_NEW: only print userid if it has new signature */
	    print_userid = FALSE;
	    if (firstuser) {
		if (is_ctb_type(keyctb, CTB_CERT_PUBKEY_TYPE))
		    fprintf(pgpout, LANG("pub"));
		else if (is_ctb_type(keyctb, CTB_CERT_SECKEY_TYPE))
		    fprintf(pgpout, LANG("sec"));
		else
		    fprintf(pgpout, "???");
		if (invalid_key)
		    fprintf(pgpout, "? ");
		else
		    fprintf(pgpout, "  ");
		fprintf(pgpout, "%4d/%s %s ",
			countbits(n), keyIDstring(keyID), cdate(&tstamp));
	    } else {
		fprintf(pgpout, "          %s            ",
			blankkeyID);
	    }
	    if (compromised && firstuser) {
		fprintf(pgpout, LANG("*** KEY REVOKED ***\n"));
		fprintf(pgpout, "          %s              ",
			blankkeyID);
	    }
	    firstuser = 0;
	    fprintf(pgpout, "%s\n", LOCAL_CHARSET((char *) keyuserid));
	}
	/* Ignore comments and anything else */
	if (!is_ctb_type(ctb, CTB_SKE_TYPE))
	    continue;

	/* So now we're checking a signature... */
	/* Try checking signature on either this ring or dflt ring */

	CToPascal((char *) keyuserid);
	sigstatus = check_key_sig(f, fpkey, keypktlen,
				  (char *) keyuserid, f, fpsig,
				  ringfile, (char *) siguserid,
				  sigtimestamp, &sigclass);
	if (sigstatus == -2 && strcmp(ringfile, dfltring) != 0) {
	    sigstatus = check_key_sig(f, fpkey, keypktlen,
				      (char *) keyuserid, f, fpsig,
				      dfltring, (char *) siguserid,
				      sigtimestamp, &sigclass);
	}
	/*
	 * Note: sigstatus has the following values:
	 *   0 Good signature
	 *  -1 Generic error
	 *  -2 Can't find key
	 *  -3 Key too big
	 *  -4 Key too small
	 *  -5 Maybe malformed RSA (RSAREF)
	 *  -6 Unknown PK algorithm
	 *  -7 Unknown conventional algorithm
	 *  -8 Unknown version
	 *  -9 Malformed RSA packet
	 * -10 Malformed packet
	 * -20 BAD SIGNATURE
	 */
	PascalToC((char *) keyuserid);
	fseek(f, fpsig + sigpktlen, SEEK_SET);
	if (sigclass == KC_SIGNATURE_BYTE)
	    fprintf(pgpout, LANG("com"));
	else
	    fprintf(pgpout, LANG("sig"));
	if (sigstatus >= 0)
	    fputs("!      ", pgpout);	/* Good */
	else if (status < 0 || sigstatus == -2 || sigstatus == -3)
	    fputs("?      ", pgpout);	/* Uncheckable */
	else if (sigstatus != -20)
	    fputs("%      ", pgpout);	/* Malformed */
	else
	    fputs("*      ", pgpout);	/* BAD! */

	showkeyID(sigkeyID, pgpout);

	/* If we got a keyID, show it */
	if (sigstatus >= 0 || sigstatus == -3 ||
	    (sigstatus <= -5 && sigstatus >= -9) ||
	    sigstatus == -20) {
	    PascalToC((char *) siguserid);
	    fprintf(pgpout, " %s ", cdate(&sigtstamp));
	    if (sigclass != KC_SIGNATURE_BYTE)
		putc(' ', pgpout);
	    fputs(LOCAL_CHARSET((char *) siguserid), pgpout);
	    putc('\n', pgpout);
	    /* If an error, prepare next line for message */
	    if (sigstatus < 0)
		fprintf(pgpout, "          %s             ",
			blankkeyID);
	} else {
	    /* Indent error messages past date field */
	    fprintf(pgpout, "             ");
	}

	/* Compute new trust */
	newtrust = sigtrust;
	if (sigstatus >= 0) {
	    newtrust |= KC_SIG_CHECKED;
	} else if (sigstatus == -2) {
	    newtrust |= KC_SIG_CHECKED;
	    newtrust &= ~KC_SIGTRUST_MASK;
	} else {
	    newtrust &= ~KC_SIGTRUST_MASK & ~KC_SIG_CHECKED;
	    newtrust |= KC_SIGTRUST_UNTRUSTED;
	}

	/* If it changed, write it out */
	if (trustpos > 0 && newtrust != sigtrust)
	    write_trust_pos(f, newtrust, trustpos);
	if (sigstatus >= 0)
	    continue;		/* Skip error code */

	/* An error: print an appropriate message */
	if (sigstatus == -2)
	    fprintf(pgpout, LANG("(Unknown signator, can't be checked)"));
	else if (sigstatus == -3)
	    fprintf(pgpout, LANG("(Key too long, can't be checked)"));
	else if (sigstatus == -5)
	    fprintf(pgpout, LANG("(Malformed or obsolete signature format)"));
	else if (sigstatus == -6)
	    fprintf(pgpout, LANG("(Unknown public-key algorithm)"));
	else if (sigstatus == -7)
	    fprintf(pgpout, LANG("(Unknown hash algorithm)"));
	else if (sigstatus == -8)
	    fprintf(pgpout, LANG("(Unknown signature packet version)"));
	else if (sigstatus == -9)
	    fprintf(pgpout, LANG("(Malformed signature)"));
	else if (sigstatus == -10)
	    fprintf(pgpout, LANG("(Corrupted signature packet)"));
	else if (sigstatus == -20)
	    fprintf(pgpout, LANG("\007**** BAD SIGNATURE! ****"));
	else
	    fprintf(pgpout, "(Unexpected signature error %d)", sigstatus);
	putc('\n', pgpout);

	/*
	 * If the signature was not too bad, leave it on the key
	 * ring.
	 */
	if (sigstatus == -2 || sigstatus == -3)
	    continue;
	/*
	 * The signature was unacceptable, and
	 * likely to remain that way, so remove it
	 * from the keyring.
	 */
	if (!failed) {
	    /* first bad signature: create scratch file */
	    tempring = tempfile(TMP_TMPDIR);
	    fixedf = fopen(tempring, FOPWBIN);
	    failed = TRUE;
	}
	if (fixedf != NULL) {
	    copyfilepos(f, fixedf, fpsig - fixpos, fixpos);
	    fseek(f, fpsig + sigpktlen, SEEK_SET);
	    if (nextkeypacket(f, &ctb) < 0 || ctb != CTB_KEYCTRL)
		fseek(f, fpsig + sigpktlen, SEEK_SET);
	    fixpos = ftell(f);
	}
    }				/* loop for all packets */

    close_more();
    pgpout = savepgpout;

    if (status < -1) {
	fclose(f);
	return status;
    }
    fputc('\n', pgpout);

    if (failed && fixedf) {
	copyfilepos(f, fixedf, -1L, fixpos);
	fclose(f);
	if (write_error(fixedf)) {
	    fclose(fixedf);
	    return -1;
	}
	fclose(fixedf);
	if (!batchmode)
	    fprintf(pgpout, LANG("Remove bad signatures (Y/n)? "));
	if (batchmode || getyesno('y')) {
	    savetempbak(tempring, ringfile);
	    failed = 0;
	}
    } else {
	fclose(f);
    }

    return 0;			/* normal return */

}				/* dokeycheck */

int backup_rename(char *scratchfile, char *destfile)
{
    /* rename scratchfile to destfile after making a backup file */
    char bakfile[MAX_PATH];

    if (is_tempfile(destfile)) {
	remove(destfile);
    } else {
	if (file_exists(destfile)) {
	    strcpy(bakfile, destfile);
	    force_extension(bakfile, BAK_EXTENSION);
	    remove(bakfile);
	    rename(destfile, bakfile);
	}
    }
    return rename2(scratchfile, destfile);
}

/* Lists all signatures for keys with specified mcguffin string, and asks
 * if they should be removed.
 */
int remove_sigs(char *mcguffin, char *ringfile)
{
    FILE *f, *g;
    byte ctb;
    long fp, fpuser;
    int packetlength;
    int status;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    byte sigkeyID[KEYFRAGSIZE];
    byte userid[256];		/* key certificate userid */
    char dfltring[MAX_PATH];
    word32 tstamp;
    byte *timestamp = (byte *) & tstamp;	/* key certificate timestamp */
    int nsigs = 0, nremoved = 0;
    int keeping;
    char *scratchf;

    /* Default keyring to check signature ID's */
    strcpy(dfltring, globalPubringName);

    if (!mcguffin || strlen(mcguffin) == 0)
	return -1;

    setoutdir(ringfile);
    scratchf = tempfile(0);

    strcpy((char *) userid, mcguffin);

    fprintf(pgpout,
	    LANG("\nRemoving signatures from userid '%s' in key ring '%s'\n"),
	    LOCAL_CHARSET(mcguffin), ringfile);

    status = getpublickey(GPK_GIVEUP | GPK_SHOW, ringfile, &fp,
			  &packetlength, NULL, timestamp, userid, n, e);
    if (status < 0) {
	fprintf(pgpout, LANG("\n\007Key not found in key ring '%s'.\n"),
		ringfile);
	return 0;		/* normal return */
    }
    strcpy((char *) userid, mcguffin);
    getpubuserid(ringfile, fp, userid, &fpuser, &packetlength, FALSE);
    packetlength += (int) (fpuser - fp);

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout, LANG("\n\007Can't open key ring file '%s'\n"),
		ringfile);
	return -1;
    }
    /* Count signatures */
    fseek(f, fp + packetlength, SEEK_SET);
    for (;;) {
	status = nextkeypacket(f, &ctb);
	if (status < 0 || is_key_ctb(ctb) || ctb == CTB_USERID)
	    break;
	if (is_ctb_type(ctb, CTB_SKE_TYPE))
	    ++nsigs;
    }

    rewind(f);

    if (nsigs == 0) {
	fprintf(pgpout, LANG("\nKey has no signatures to remove.\n"));
	fclose(f);
	return 0;		/* Normal return */
    }
    fprintf(pgpout, LANG("\nKey has %d signature(s):\n"), nsigs);

    /* open file g for writing, in binary (not text) mode... */
    if ((g = fopen(scratchf, FOPWBIN)) == NULL) {
	fclose(f);
	return -1;
    }
    copyfile(f, g, fp + packetlength);	/* copy file f to g up through key */

    /* Now print out any following sig certs */
    keeping = 1;
    for (;;) {
	fp = ftell(f);
	status = readkeypacket(f, FALSE, &ctb, NULL, NULL, NULL, NULL,
			       NULL, NULL, NULL, NULL, sigkeyID, NULL);
	packetlength = (int) (ftell(f) - fp);
	if ((status < 0 && status != -6 && status != -4) ||
	    is_key_ctb(ctb) || ctb == CTB_USERID)
	    break;
	if (is_ctb_type(ctb, CTB_SKE_TYPE)) {
	    fprintf(pgpout, LANG("sig"));
	    fprintf(pgpout, "%c     ", status < 0 ? '?' : ' ');
	    if (status < 0)
		memset(sigkeyID, 0, KEYFRAGSIZE);
	    showkeyID(sigkeyID, pgpout);
	    fprintf(pgpout, "               ");	/* Indent signator userid */
	    if (getpublickey(GPK_GIVEUP, ringfile, NULL, NULL,
			     sigkeyID, timestamp, userid, n, e) >= 0 ||
		getpublickey(GPK_GIVEUP, dfltring, NULL, NULL,
			     sigkeyID, timestamp, userid, n, e) >= 0) {
		PascalToC((char *) userid);
		fprintf(pgpout, "%s\n", LOCAL_CHARSET((char *) userid));
	    } else {
		fprintf(pgpout,
			LANG("(Unknown signator, can't be checked)\n"));
	    }
	    fprintf(pgpout, LANG("Remove this signature (y/N)? "));
	    if (!(keeping = !getyesno('n')))
		++nremoved;
	}
	if (keeping)
	    copyfilepos(f, g, (long) packetlength, fp);
    }				/* scanning sig certs */
    copyfilepos(f, g, -1L, fp);	/* Copy rest of file */

    fclose(f);			/* close key file */
    if (write_error(g)) {
	fclose(g);
	return -1;
    }
    fclose(g);			/* close scratch file */
    savetempbak(scratchf, ringfile);
    if (nremoved == 0)
	fprintf(pgpout, LANG("\nNo key signatures removed.\n"));
    else
	fprintf(pgpout, LANG("\n%d key signature(s) removed.\n"), nremoved);

    return 0;			/* normal return */

}				/* remove_sigs */

/*
 * Remove the first entry in key ring that has mcguffin string in userid.
 * Or it removes the first matching keyID from the ring.
 * A non-NULL keyID takes precedence over a mcguffin specifier.
 * mcguffin is a null-terminated C string.
 * If secring_too is TRUE, the secret keyring is also checked.
 */
int remove_from_keyring(byte * keyID, char *mcguffin,
			char *ringfile, boolean secring_too)
{
    FILE *f;
    FILE *g;
    long fp, nfp;
    int packetlength;
    byte ctb;
    int status;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    byte userid[256];		/* key certificate userid */
    word32 tstamp;
    byte *timestamp = (byte *) & tstamp;	/* key certificate timestamp */
    int userids;
    boolean rmuserid = FALSE;
    char *scratchf;
    unsigned secflag = 0;

    default_extension(ringfile, PGP_EXTENSION);

    if ((keyID == NULL) && (!mcguffin || strlen(mcguffin) == 0))
	return -1;	/* error, null mcguffin will match everything */

  top:
    if (mcguffin)
	strcpy((char *) userid, mcguffin);

    fprintf(pgpout, LANG("\nRemoving from key ring: '%s'"), ringfile);
    if (mcguffin && strlen(mcguffin) > 0)
	fprintf(pgpout, LANG(", userid \"%s\".\n"),
		LOCAL_CHARSET(mcguffin));

    status = getpublickey(secflag | GPK_GIVEUP | GPK_SHOW, ringfile, &fp,
			  &packetlength, NULL, timestamp, userid, n, e);
    if (status < 0 && status != -4 && status != -6) {
	fprintf(pgpout, LANG("\n\007Key not found in key ring '%s'.\n"),
		ringfile);
	return 0;		/* normal return */
    }
    /* Now add to packetlength the subordinate following certificates */
    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout, LANG("\n\007Can't open key ring file '%s'\n"),
		ringfile);
	return -1;
    }
    fseek(f, fp + packetlength, SEEK_SET);
    userids = 0;
    do {			/* count user ID's, position nfp at next key */
	nfp = ftell(f);
	status = nextkeypacket(f, &ctb);
	if (status == 0 && ctb == CTB_USERID)
	    ++userids;
    } while (status == 0 && !is_key_ctb(ctb));
    if (status < -1) {
	fclose(f);
	return -1;
    }
    if (keyID == NULL) {	/* Human confirmation is required. */
	/* Supposedly the key was fully displayed by getpublickey */
	if (userids > 1) {
	    fprintf(pgpout, LANG("\nKey has more than one user ID.\n\
Do you want to remove the whole key (y/N)? "));
	    if (!getyesno('n')) {
		/* find out which userid should be removed */
		rmuserid = TRUE;
		fseek(f, fp + packetlength, SEEK_SET);
		for (;;) {
		    fp = ftell(f);
		    status = readkpacket(f, &ctb, (char *) userid, NULL, NULL);
		    if (status < 0 && status != -4 && status != -6
			|| is_key_ctb(ctb)) {
			fclose(f);
			fprintf(pgpout, LANG("\nNo more user ID's\n"));
			return -1;
		    }
		    if (ctb == CTB_USERID) {
			fprintf(pgpout, LANG("Remove \"%s\" (y/N)? "), userid);
			if (getyesno('n'))
			    break;
		    }
		}
		do {		/* also remove signatures and trust bytes */
		    nfp = ftell(f);
		    status = nextkeypacket(f, &ctb);
		} while ((status == 0 || status == -4 || status == -6) &&
			 !is_key_ctb(ctb) && ctb != CTB_USERID);
		if (status < -1 && status != -4 && status != -6) {
		    fclose(f);
		    return -1;
		}
	    }
	} else if (!force_flag) {	/* only one user ID */
	    fprintf(pgpout,
	       LANG("\nAre you sure you want this key removed (y/N)? "));
	    if (!getyesno('n')) {
		fclose(f);
		return -1;	/* user said "no" */
	    }
	}
    }
    fclose(f);
    packetlength = (int) (nfp - fp);

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout, LANG("\n\007Can't open key ring file '%s'\n"),
		ringfile);
	return -1;
    }
    setoutdir(ringfile);
    scratchf = tempfile(0);
    /* open file g for writing, in binary (not text) mode... */
    if ((g = fopen(scratchf, FOPWBIN)) == NULL) {
	fclose(f);
	return -1;
    }
    copyfilepos(f, g, fp, 0L);	/* copy file f to g up to position fp */
    copyfilepos(f, g, -1L, fp + packetlength);	/* copy rest of file f */
    fclose(f);			/* close key file */
    if (write_error(g)) {
	fclose(g);
	return -1;
    }
    fclose(g);			/* close scratch file */
    if (secring_too)		/* TRUE if this is the public keyring */
	maint_update(scratchf, 0);
    savetempbak(scratchf, ringfile);
    if (rmuserid)
	fprintf(pgpout, LANG("\nUser ID removed from key ring.\n"));
    else
	fprintf(pgpout, LANG("\nKey removed from key ring.\n"));

    if (secring_too) {
	secring_too = FALSE;
	strcpy(ringfile, globalSecringName);
	strcpy((char *) userid, mcguffin);
	if (getpublickey(GPK_GIVEUP | GPK_SECRET, ringfile, NULL,
			 NULL, NULL, timestamp, userid, n, e) == 0) {
	    fprintf(pgpout,
LANG("\nKey or user ID is also present in secret keyring.\n\
Do you also want to remove it from the secret keyring (y/N)? "));
	    if (getyesno('n')) {
		secflag = GPK_SECRET;
		goto top;
	    }
	}
    }
    return 0;			/* normal return */

}				/* remove_from_keyring */

/*
 * Copy the first entry in key ring that has mcguffin string in
 * userid and put it into keyfile.
 * mcguffin is a null-terminated C string.
 */
int extract_from_keyring(char *mcguffin, char *keyfile, char *ringfile,
			 boolean transflag)
{
    FILE *f;
    FILE *g;
    long fp;
    int packetlength = 0;
    byte ctb;
    byte keyctrl;
    int status;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    byte keyID[KEYFRAGSIZE];
    byte userid[256];		/* key certificate userid */
    char fname[MAX_PATH], transfile[MAX_PATH], transname[MAX_PATH];
    char *tempf = NULL;
    word32 tstamp;
    byte *timestamp = (byte *) & tstamp;	/* key cert tstamp */
    boolean append = FALSE;
    boolean whole_ring = FALSE;

    default_extension(ringfile, PGP_EXTENSION);

    if (!mcguffin || strlen(mcguffin) == 0 || strcmp(mcguffin, "*") == 0)
	whole_ring = TRUE;

    /* open file f for read, in binary (not text) mode... */
    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout, LANG("\n\007Can't open key ring file '%s'\n"),
		ringfile);
	return -1;
    }
    if (!whole_ring) {
	strcpy((char *) userid, mcguffin);
	fprintf(pgpout, LANG("\nExtracting from key ring: '%s'"), ringfile);
	fprintf(pgpout, LANG(", userid \"%s\".\n"), LOCAL_CHARSET(mcguffin));

	status = getpublickey(GPK_GIVEUP | GPK_SHOW,
			      ringfile, &fp, &packetlength, NULL,
			      timestamp, userid, n, e);
	if (status < 0 && status != -4 && status != -6) {
	    fprintf(pgpout, LANG("\n\007Key not found in key ring '%s'.\n"),
		    ringfile);
	    fclose(f);
	    return 1;		/* non-normal return */
	}
	extract_keyID(keyID, n);
    } else {
	do			/* set fp to first key packet */
	    fp = ftell(f);
	while ((status = nextkeypacket(f, &ctb)) >= 0 && !is_key_ctb(ctb));
	if (status < 0) {
	    fclose(f);
	    return -1;
	}
	packetlength = (int) (ftell(f) - fp);
    }

    if (!keyfile || strlen(keyfile) == 0) {
	fprintf(pgpout, "\n");
	fprintf(pgpout, LANG("Extract the above key into which file?"));
	fprintf(pgpout, " ");
	if (batchmode)
	    return -1;
#ifdef MACTC5
	if(!GetFilePath(LANG("Extract the above key into which file?"), fname, PUTFILE)) {
		Putchar('\n');
		fname[0]='\0';
		return -1;
		}
	fprintf(pgpout,fname);
#else
	getstring(fname, sizeof(fname) - 4, TRUE);
#endif
	if (*fname == '\0')
	    return -1;
    } else {
	strcpy(fname, keyfile);
    }
    default_extension(fname, PGP_EXTENSION);

    /* If transport armoring, use a dummy file for keyfile */
    if (transflag) {
	strcpy(transname, fname);
	strcpy(transfile, fname);
	force_extension(transfile, ASC_EXTENSION);
	tempf = tempfile(TMP_TMPDIR | TMP_WIPE);
	strcpy(fname, tempf);
    }
    if (file_exists(transflag ? transfile : fname)) {
	if (!transflag && !whole_ring) {
	    /* see if the key is already present in fname */
	    status = getpublickey(GPK_GIVEUP, fname, NULL, NULL, keyID,
				  timestamp, userid, n, e);
	    if (status >= 0 || status == -4 || status == -6) {
		fclose(f);
		fprintf(pgpout,
		  LANG("Key ID %s is already included in key ring '%s'.\n"),
			keyIDstring(keyID), fname);
		return -1;
	    }
	}
	if (whole_ring || transflag || status < -1) {
	    if (!is_tempfile(fname) && !force_flag)
		/* Don't ask this for mailmode or for 
		 * a tempfile, since its ok.
		 */
	    {
/* if status < -1 then fname is not a keyfile,
   ask if it should be overwritten */
		fprintf(pgpout,
	     LANG("\n\007Output file '%s' already exists.  Overwrite (y/N)? "),
			transflag ? transfile : fname);
		if (!getyesno('n')) {
		    fclose(f);
		    return -1;	/* user chose to abort */
		}
	    }
	} else {
	    append = TRUE;
	}
    }
    if (append)
	g = fopen(fname, FOPRWBIN);
    else
	g = fopen(fname, FOPWBIN);
    if (g == NULL) {
	if (append)
	    fprintf(pgpout,
		    LANG("\n\007Can't open key ring file '%s'\n"), ringfile);
	else
	    fprintf(pgpout,
		    LANG("\n\007Unable to create key file '%s'.\n"), fname);
	fclose(f);
	return -1;
    }
    if (append)
	fseek(g, 0L, SEEK_END);
    do {
	/* file f is positioned right after key packet */
	if (whole_ring && read_trust(f, &keyctrl) == 0
	    && (keyctrl & KC_DISABLED)) {
	    do {		/* skip this key */
		fp = ftell(f);
		status = nextkeypacket(f, &ctb);
		packetlength = (int) (ftell(f) - fp);
	    }
	    while (!is_key_ctb(ctb) && status >= 0);
	    continue;
	}
	if (copyfilepos(f, g, (long) packetlength, fp) < 0) {
	    /* Copy key out */
	    status = -2;
	    break;
	}
	/* Copy any following signature or userid packets */
	for (;;) {
	    fp = ftell(f);
	    status = nextkeypacket(f, &ctb);
	    packetlength = (int) (ftell(f) - fp);
	    if (status < 0 || is_key_ctb(ctb))
		break;
	    if (ctb == CTB_USERID || is_ctb_type(ctb, CTB_SKE_TYPE))
		if (copyfilepos(f, g, (long) packetlength, fp) < 0) {
		    status = -2;
		    break;
		}
	}
    }
    while (whole_ring && status >= 0);

    fclose(f);
    if (status < -1 || write_error(g)) {
	fclose(g);
	return -1;
    }
    fclose(g);

    if (transflag) {
        do {
            char *t;
            force_extension(transfile, ASC_EXTENSION);
            if (!file_exists(transfile)) break;
            t=ck_dup_output(transfile, TRUE, TRUE);
            if (t==NULL) user_error();
            strcpy(transfile,t);
        } while (TRUE);
	status = armor_file(fname, transfile, transname, NULL, !whole_ring);
	rmtemp(tempf);
	if (status)
	    return -1;
    }
    fprintf(pgpout, LANG("\nKey extracted to file '%s'.\n"),
	    transflag ? transfile : fname);

    return 0;			/* normal return */
}				/* extract_from_keyring */


/*======================================================================*/

/* Copy the key data in keyfile into ringfile, replacing the data that
   is in ringfile starting at fp and for length packetlength.
   keylen is the number of bytes to copy from keyfile
 */
static int merge_key_to_ringfile(char *keyfile, char *ringfile, long fp,
				 int packetlength, long keylen)
{
    FILE *f, *g, *h;
    char *tempf;
    int rc;

    setoutdir(ringfile);
    tempf = tempfile(TMP_WIPE);
    /* open file f for reading, binary, as keyring file */
    if ((f = fopen(ringfile, FOPRBIN)) == NULL)
	return -1;
    /* open file g for writing, binary, as scratch keyring file */
    if ((g = fopen(tempf, FOPWBIN)) == NULL) {
	fclose(f);
	return -1;
    }
    /* open file h for reading, binary, as key file to be inserted */
    if ((h = fopen(keyfile, FOPRBIN)) == NULL) {
	fclose(f);
	fclose(g);
	return -1;
    }
    /* Copy pre-key keyring data from f to g */
    copyfile(f, g, fp);
    /* Copy temp key data from h to g */
    copyfile(h, g, keylen);
    /* Copy post-key keyring data from f to g */
    copyfilepos(f, g, -1L, fp + packetlength);
    fclose(f);
    rc = write_error(g);
    fclose(g);
    fclose(h);

    if (!rc)
	savetempbak(tempf, ringfile);

    return rc ? -1 : 0;
}				/* merge_key_to_ringfile */

static int insert_userid(char *keyfile, byte * userid, long fpos)
{
    /* insert userid and trust byte at position fpos in file keyfile */
    char *tmpf;
    FILE *f, *g;

    tmpf = tempfile(TMP_TMPDIR);
    if ((f = fopen(keyfile, FOPRBIN)) == NULL)
	return -1;
    if ((g = fopen(tmpf, FOPWBIN)) == NULL) {
	fclose(f);
	return -1;
    }
    copyfile(f, g, fpos);
    putc(CTB_USERID, g);
    fwrite(userid, 1, userid[0] + 1, g);
    write_trust(g, KC_LEGIT_COMPLETE);
    copyfile(f, g, -1L);
    fclose(f);
    if (write_error(g)) {
	fclose(g);
	return -1;
    }
    fclose(g);
    return savetempbak(tmpf, keyfile);
}

int dokeyedit(char *mcguffin, char *ringfile)
/*
 * Edit the userid and/or pass phrase for an RSA key pair, and
 * put them back into the ring files.
 */
{
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION], d[MAX_UNIT_PRECISION],
         p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];
    char *fname, secring[MAX_PATH];
    FILE *f;
    byte userid[256];
    byte userid1[256];
    word32 timestamp;		/* key certificate timestamp */
    byte keyID[KEYFRAGSIZE];
    boolean hidekey;		/* TRUE iff secret key is encrypted */
    boolean changed = FALSE, changeID = FALSE;
    byte ctb;
    int status;
    long fpp, fps, fpu, trust_pos, keylen;
    int pplength = 0, pslength = 0;
    byte ideakey[16];
    byte keyctrl;
    struct IdeaCfbContext cfb;

    if (!ringfile || strlen(ringfile) == 0 || !mcguffin
	|| strlen(mcguffin) == 0)
	return -1;		/* Need ringfile name, user name */

    if (!file_exists(ringfile))
        force_extension(ringfile, PGP_EXTENSION);

    /*
     * Although the name of a secret keyring may change in the future, it
     * is a safe bet that anything named "secring.pgp" will be a secret
     * key ring for the indefinite future.  
     */
    if (!strcmp(file_tail(ringfile), "secring.pgp") ||
	!strcmp(file_tail(ringfile), file_tail(globalSecringName))) {
	fprintf(pgpout,
LANG("\nThis operation may not be performed on a secret keyring.\n\
Defaulting to public keyring."));
	strcpy(ringfile, globalPubringName);
    }
    strcpy((char *) userid, mcguffin);
    fprintf(pgpout, LANG("\nEditing userid \"%s\" in key ring: '%s'.\n"),
	    LOCAL_CHARSET((char *) userid), ringfile);

    if (!file_exists(ringfile)) {
	fprintf(pgpout, LANG("\nCan't open public key ring file '%s'\n"),
		ringfile);
	return -1;
    }
    status = getpublickey(GPK_GIVEUP | GPK_SHOW, ringfile, &fpp, &pplength,
			  NULL, (byte *) & timestamp, userid, n, e);
    if (status < 0) {
	fprintf(pgpout, LANG("\n\007Key not found in key ring '%s'.\n"),
		ringfile);
	return -1;
    }
    /* Now add to pplength any following key control certificate */
    if ((f = fopen(ringfile, FOPRWBIN)) == NULL) {
	fprintf(pgpout, LANG("\n\007Can't open key ring file '%s'\n"),
		ringfile);
	return -1;
    }
    if (fread(&ctb, 1, 1, f) != 1 || !is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE)) {
	fprintf(pgpout, LANG("\n\007File '%s' is not a public keyring.\n"),
		ringfile);
	fclose(f);
	return -1;
    }
    fseek(f, fpp, SEEK_SET);
    if (is_compromised(f)) {
	fprintf(pgpout,
		LANG("\n\007This key has been revoked by its owner.\n"));
	fclose(f);
	return -1;
    }
    trust_pos = fpp + pplength;
    fseek(f, trust_pos, SEEK_SET);
    if (read_trust(f, &keyctrl) < 0)
	trust_pos = -1;		/* keyfile: no trust byte */

    extract_keyID(keyID, n);

#if 0
    /*
     * Old code: looks in the same directory as the given keyring, but
     * with the secret keyring filename.
     */
    strcpy(secring, ringfile);
    strcpy(file_tail(secring), file_tail(globalSecringName));
    if (!file_exists(secring) && strcmp(file_tail(secring), "secring.pgp")) {
	strcpy(file_tail(secring), "secring.pgp");
    }
#else
    /*
     * What it should do: use the secret keyring, always.
     * Now that the path can be set, this is The Right Thing.
     * It used to be impossible to put the secret and public keyring in
     * different directories, so forcing the same directory name was The
     * Right Thing.  It is no longer.
     */
    strcpy(secring, globalSecringName);
#endif
    if (!file_exists(secring)) {
	fprintf(pgpout, LANG("\nCan't open secret key ring file '%s'\n"),
		secring);
	fclose(f);
	return -1;
    }
    /* Get position of key in secret key file */
    (void) getpublickey(GPK_GIVEUP | GPK_SECRET, secring, &fps, &pslength,
			keyID, (byte *) & timestamp, userid1, n, e);
    /* This was done to get us fps and pslength */
    status = getsecretkey(GPK_GIVEUP, secring, keyID, (byte *) & timestamp,
			  ideakey, &hidekey, userid1, n, e, d, p, q, u);

    if (status < 0) {	/* key not in secret keyring: edit owner trust */
	int i;

	fprintf(pgpout,
LANG("\nNo secret key available.  Editing public key trust parameter.\n"));
	if (trust_pos < 0) {
	    fprintf(pgpout,
		    LANG("\n\007File '%s' is not a public keyring.\n"),
		    ringfile);
	    fclose(f);
	    return -1;
	}
	show_key(f, fpp, SHOW_ALL);

	init_trust_lst();
	fprintf(pgpout, LANG("Current trust for this key's owner is: %s\n"),
		trust_lst[keyctrl & KC_OWNERTRUST_MASK]);

	PascalToC((char *) userid);	/* convert to C string for display */
	i = ask_owntrust((char *) userid, keyctrl);
	if (i == (keyctrl & KC_OWNERTRUST_MASK)) {
	    fclose(f);
	    return 0;		/* unchanged */
	}
	if (i < 0 || i > KC_OWNERTRUST_ALWAYS) {
	    fclose(f);
	    return -1;
	}
	keyctrl = (keyctrl & ~KC_OWNERTRUST_MASK) | i;

	fseek(f, trust_pos, SEEK_SET);
	write_trust(f, keyctrl);
	fclose(f);
	fprintf(pgpout, LANG("Public key ring updated.\n"));
	return 0;
    }
    if (trust_pos > 0 && (keyctrl & (KC_BUCKSTOP | KC_OWNERTRUST_MASK)) !=
	(KC_OWNERTRUST_ULTIMATE | KC_BUCKSTOP)) {
	/* key is in secret keyring but buckstop is not set */
	fprintf(pgpout,
LANG("\nUse this key as an ultimately-trusted introducer (y/N)? "), userid);
	if (getyesno('n')) {
	    fseek(f, trust_pos, SEEK_SET);
	    keyctrl = KC_OWNERTRUST_ULTIMATE | KC_BUCKSTOP;
	    write_trust(f, keyctrl);
	}
    }
    /* Show user her ID again to be clear */
    PascalToC((char *) userid);
    fprintf(pgpout, LANG("\nCurrent user ID: %s"),
	    LOCAL_CHARSET((char *) userid));
    CToPascal((char *) userid);

    fprintf(pgpout, LANG("\nDo you want to add a new user ID (y/N)? "));
    if (getyesno('n')) {	/* user said yes */
	fprintf(pgpout, LANG("\nEnter the new user ID: "));
#ifdef MACTC5
		GetNewUID((char *)userid);
		fprintf(pgpout, "%s\n",userid);
#else
	getstring((char *) userid, 255, TRUE);	/* echo keyboard input */
#endif
	if (userid[0] == '\0') {
	    fclose(f);
	    return -1;
	}
	CONVERT_TO_CANONICAL_CHARSET((char *) userid);
	fprintf(pgpout,
LANG("\nMake this user ID the primary user ID for this key (y/N)? "));
	if (!getyesno('n')) {
	    /* position file pointer at selected user id */
	    int pktlen;
	    long fpuser;

	    strcpy((char *) userid1, mcguffin);
	    if (getpubuserid(ringfile, fpp, userid1, &fpuser, &pktlen,
			     FALSE) < 0) {
		fclose(f);
		return -1;
	    }
	    fseek(f, fpuser, SEEK_SET);
	} else {		/* position file pointer at key packet */
	    fseek(f, fpp, SEEK_SET);
	}
	nextkeypacket(f, &ctb);	/* skip userid or key packet */
	do {			/* new user id will be inserted before next
				   userid or key packet */
	    fpu = ftell(f);
	    if (nextkeypacket(f, &ctb) < 0)
		break;
	} while (ctb != CTB_USERID && !is_key_ctb(ctb));
	CToPascal((char *) userid);	/* convert to length-prefixed string */
	changeID = TRUE;
	changed = TRUE;
    }
    fclose(f);

    fprintf(pgpout, LANG("\nDo you want to change your pass phrase (y/N)? "));
    if (getyesno('n')) {	/* user said yes */
	hidekey = (GetHashedPassPhrase((char *)ideakey, 2) > 0);
	changed = TRUE;
    }
    if (!changed) {
	fprintf(pgpout, LANG("(No changes will be made.)\n"));
	if (hidekey)
	    burn(ideakey);
	goto done;
    }
    /* init CFB IDEA key */
    if (hidekey) {
	ideaCfbInit(&cfb, ideakey);
	burn(ideakey);
    }
    /* First write secret key data to a file */
    fname = tempfile(TMP_TMPDIR | TMP_WIPE);
    writekeyfile(fname, hidekey ? &cfb : 0, timestamp,
		 userid, n, e, d, p, q, u);

    if (hidekey)		/* done with IDEA to protect RSA secret key */
	ideaCfbDestroy(&cfb);

    if (changeID) {
	keylen = -1;
    } else {
	/* don't copy userid */
	f = fopen(fname, FOPRBIN);
	if (f == NULL)
	    goto err;
	nextkeypacket(f, &ctb);	/* skip key packet */
	keylen = ftell(f);
	fclose(f);
    }
    if (merge_key_to_ringfile(fname, secring, fps, pslength, keylen) < 0) {
	fprintf(pgpout, LANG("\n\007Unable to update secret key ring.\n"));
	goto err;
    }
    fprintf(pgpout, LANG("\nSecret key ring updated...\n"));
#ifdef MACTC5
	PGPSetFinfo(secring,'SKey','MPGP');
#endif

    /* Now write public key data to file */
    if (changeID) {
	if (insert_userid(ringfile, userid, fpu) < 0) {
	    fprintf(pgpout, LANG("\n\007Unable to update public key ring.\n"));
	    goto err;
	}

        /* Automatically sign new userid? */
        if (sign_new_userids) {
     	    PascalToC((char *) userid);
            strcpy((char *)userid1, (char *)userid);
            if (do_sign(ringfile, fpp, pplength, userid, keyID, (char *)userid1, TRUE) < 0)
                return -1;
	}

        fprintf(pgpout, LANG("Public key ring updated.\n"));
#ifdef MACTC5
	PGPSetFinfo(ringfile,'PKey','MPGP');
#endif
    } else {
	fprintf(pgpout, LANG("(No need to update public key ring)\n"));
    }

    rmtemp(fname);

  done:
    mp_burn(d);			/* burn sensitive data on stack */
    mp_burn(p);
    mp_burn(q);
    mp_burn(u);
    mp_burn(e);
    mp_burn(n);

    return 0;			/* normal return */
  err:
    mp_burn(d);			/* burn sensitive data on stack */
    mp_burn(p);
    mp_burn(q);
    mp_burn(u);
    mp_burn(e);
    mp_burn(n);

    rmtemp(fname);

    return -1;			/* error return */

}				/* dokeyedit */

int disable_key(char *keyguffin, char *keyfile)
{
    FILE *f;
    byte keyctrl;
    byte keyID[KEYFRAGSIZE];
    byte userid[256];
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    long fp;
    int pktlen;

    strcpy((char *) userid, keyguffin);
    if (getpublickey(GPK_SHOW | GPK_DISABLED, keyfile, &fp, &pktlen, NULL,
		     NULL, userid, n, e) < 0)
	return -1;

    extract_keyID(keyID, n);
    if (getsecretkey(GPK_GIVEUP, NULL, keyID, NULL, NULL, NULL,
		     userid, n, e, NULL, NULL, NULL, NULL) >= 0) {
	/* can only compromise if key also in secring */
	PascalToC((char *) userid);
	fprintf(pgpout,
		LANG("\nDo you want to permanently revoke your public key\n\
by issuing a secret key compromise certificate\n\
for \"%s\" (y/N)? "), LOCAL_CHARSET((char *) userid));
	if (getyesno('n'))
	    return compromise(keyID, keyfile);
    }
    if ((f = fopen(keyfile, FOPRWBIN)) == NULL) {
	fprintf(pgpout,
		LANG("\n\007Can't open key ring file '%s'\n"), keyfile);
	return -1;
    }
    fseek(f, fp + pktlen, SEEK_SET);
    if (read_trust(f, &keyctrl) < 0) {
	fprintf(pgpout,
		LANG("\n\007File '%s' is not a public keyring.\n"), keyfile);
	fprintf(pgpout,
		LANG("You can only disable keys on your public keyring.\n"));
	fclose(f);
	return -1;
    }
    if (keyctrl & KC_DISABLED) {
	fprintf(pgpout, LANG("\nKey is already disabled.\n\
Do you want to enable this key again (y/N)? "));
	keyctrl &= ~KC_DISABLED;
    } else {
	fprintf(pgpout, LANG("\nDisable this key (y/N)? "));
	keyctrl |= KC_DISABLED;
    }
    if (!getyesno('n')) {
	fclose(f);
	return -1;
    }
    write_trust_pos(f, keyctrl, fp + pktlen);
    fclose(f);
    return 0;
}				/* disable_key */


/*======================================================================*/

/*
 * Do an RSA key pair generation, and write them out to the keyring files.
 * numstr is a decimal string, the desired bitcount for the modulus n.
 * numstr2 is a decimal string, the desired bitcount for the exponent e.
 * username is the desired name for the key.
 */
int dokeygen(char *numstr, char *numstr2, char *username)
{
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    unit d[MAX_UNIT_PRECISION], p[MAX_UNIT_PRECISION];
    unit q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];
    char *fname;
#ifdef MACTC5
    char message[256];
#endif
    word16 iv[4];		/* for IDEA CFB mode, to protect
				   RSA secret key */
    byte userid[256];
    short keybits, ebits;
    word32 tstamp;
    boolean hidekey;		/* TRUE iff secret key is encrypted */
    boolean cryptrandflag;
    byte ideakey[16];
    struct IdeaCfbContext cfb;
    struct hashedpw *hpw;
    byte keyID[KEYFRAGSIZE];
    boolean keygen_OK;

    if (!numstr || strlen(numstr) == 0) {
#ifdef MACTC5
		numstr = (char *)userid;
		if (argc < 5) getRSAkeySize(numstr,numstr2);
#endif
        fputs("\n", pgpout);
	fputs(LANG("Pick your RSA key size:\n\
    1)   512 bits- Low commercial grade, fast but less secure\n\
    2)   768 bits- High commercial grade, medium speed, good security\n\
    3)  1024 bits- \"Military\" grade, slow, highest security\n\
Choose 1, 2, or 3, or enter desired number of bits: "), pgpout);
#ifdef MACTC5
		fprintf(pgpout, "%s\n",numstr);
#else
	numstr = (char *) userid;	/* use userid buffer as scratchpad */
	getstring(numstr, 5, TRUE);	/* echo keyboard */
#endif
    }
#ifdef MACTC5
	else							/* CP: argv[4] contains the new_userid */
		if(argc>4)
			strcpy(new_uid,argv[4]);
#endif
    keybits = 0;
    while ((*numstr >= '0') && (*numstr <= '9'))
	keybits = keybits * 10 + (*numstr++ - '0');

    if (keybits == 0)		/* user entered null response */
	return -1;		/* error return */

    /* Standard default key sizes: */
    if (keybits == 1)
	keybits = 512;		/* Low commercial grade */
    if (keybits == 2)
	keybits = 768;		/* High commercial grade */
    if (keybits == 3)
	keybits = 1024;		/* Military grade */

#ifndef DEBUG
    if (keybits < MIN_KEY_BITS)
	keybits = MIN_KEY_BITS;
    if (keybits > MAX_KEY_BITS)
        keybits = MAX_KEY_BITS;
#else
    if (keybits > MAX_BIT_PRECISION)
	keybits = MAX_BIT_PRECISION;
#endif

    ebits = 0;			/* number of bits in e */
    while ((*numstr2 >= '0') && (*numstr2 <= '9'))
	ebits = ebits * 10 + (*numstr2++ - '0');

    fprintf(pgpout, "\n");
    fprintf(pgpout,
	    LANG("Generating an RSA key with a %d-bit modulus.\n"), keybits);

    if (username == NULL || *username == '\0') {
        /* We need to ask for a username */
        fputs(
LANG("\nYou need a user ID for your public key.  The desired form for this\n\
user ID is your name, followed by your E-mail address enclosed in\n\
<angle brackets>, if you have an E-mail address.\n\
For example:  John Q. Smith <12345.6789@compuserve.com>\n\
Enter a user ID for your public key: \n"), pgpout);
#ifdef VMS
	putch('\n'); /* That last newline was just a return, do a real one */
#endif
#ifdef MACTC5
	strcpy((char *)userid,new_uid);
	fprintf(stdout, "%s\n",new_uid);
#else
	getstring((char *) userid, 255, TRUE);	/* echo keyboard input */
#endif
	if (userid[0] == '\0')	/* user entered null response */
	    return -1;		/* error return */

    } else {
        /* Copy in passed-in username */
        memcpy(userid, username, 255);
	fprintf(pgpout,
	    LANG("Generating RSA key-pair with UserID \"%s\".\n"), userid);
    }
    CONVERT_TO_CANONICAL_CHARSET((char *) userid);
    CToPascal((char *) userid);	/* convert to length-prefixed string */

    fputs(LANG("\nYou need a pass phrase to protect your RSA secret key.\n\
Your pass phrase can be any sentence or phrase and may have many\n\
words, spaces, punctuation, or any other printable characters.\n"), pgpout);
    hidekey = (GetHashedPassPhrase((char *)ideakey, 2) > 0);
    /* init CFB IDEA key */
#ifdef MACTC5
    if (Abort)
    	exitPGP(-2);
#endif
    if (hidekey) {
        /* Remember password - we need it later when we sign the key */
	hpw = (struct hashedpw *) malloc(sizeof(struct hashedpw));
	if (hpw) {
	    /* If malloc fails, just don't remember the phrase */
	    memcpy(hpw->hash, ideakey, sizeof(hpw->hash));
	    hpw->next = keypasswds;
	    keypasswds = hpw;
	}
	ideaCfbInit(&cfb, ideakey);
	trueRandAccumLater(64);	/* IV for encryption */
    }
/* As rsa_keygen does a major accumulation of random bits, if we need
 * any others for a seed file, let's get them at the same time.
 */
    cryptrandflag = (cryptRandOpen((struct IdeaCfbContext *)0) < 0);
    if (cryptrandflag)
	trueRandAccumLater(192);

    fputs(LANG("\nNote that key generation is a lengthy process.\n"), pgpout);

#ifdef MACTC5
	InitCursor();
	strcpy(message,"Now generating RSA key pair.\rType command-period to abort.");
	c2pstr(message);
	ParamText((uchar *)message,(uchar *)"",(uchar *)"",(uchar *)"");
	ProgressDialog=GetNewDialog(161,nil,(WindowPtr)-1);
#endif

    if (rsa_keygen(n, e, d, p, q, u, keybits, ebits) < 0) {
#ifdef MACTC5
		if (Abort)
			fprintf(pgpout, LANG("Key generation stopped at user request.\n"));
		else
			fprintf(pgpout, LANG("\n\007Keygen failed!\n"));
		DisposDialog(ProgressDialog);
		ProgressDialog=NULL;
		return -1;	/* error return */
	}
	DisposDialog(ProgressDialog);
	ProgressDialog=NULL;
#else
	fputs(LANG("\n\007Keygen failed!\n"), pgpout);
	return -1;		/* error return */
    }
#endif
    putc('\n', pgpout);

    if (verbose) {
	fprintf(pgpout, LANG("Key ID %s\n"), key2IDstring(n));

	mp_display(" modulus n = ", n);
	mp_display("exponent e = ", e);

	fputs(LANG("Display secret components (y/N)?"), pgpout);
	if (getyesno('n')) {
	    mp_display("exponent d = ", d);
	    mp_display("   prime p = ", p);
	    mp_display("   prime q = ", q);
	    mp_display(" inverse u = ", u);
	}
    }
    tstamp = get_timestamp(NULL);	/* Timestamp when key was generated */

    fputc('\007', pgpout); /* sound the bell when done with lengthy process */
    fflush(pgpout);

    /* First, write out the secret key... */
    fname = tempfile(TMP_TMPDIR | TMP_WIPE);
    writekeyfile(fname, hidekey ? &cfb : 0, tstamp, userid, n, e, d, p, q, u);

    mp_burn(d);
    mp_burn(p);
    mp_burn(q);
    mp_burn(u);

    if (hidekey)		/* done with IDEA to protect RSA secret key */
	ideaCfbDestroy(&cfb);

    if (file_exists(globalSecringName)) {
	if (!(keygen_OK = (merge_key_to_ringfile(fname, globalSecringName, 0L, 0, -1L) == 0)))
	    fprintf(pgpout, LANG("\n\007Unable to update secret key ring.\n"));
	rmtemp(fname);
    } else {
	keygen_OK = (savetemp(fname, globalSecringName) != NULL);
    }

    /* Second, write out the public key... */
    if (keygen_OK) {
        fname = tempfile(TMP_TMPDIR | TMP_WIPE);
        writekeyfile(fname, NULL, tstamp, userid, n, e, NULL, NULL, NULL, NULL);
        if (file_exists(globalPubringName)) {
	    if (!(keygen_OK = (merge_key_to_ringfile(fname, globalPubringName, 0L, 0, -1L) == 0)))
	        fprintf(pgpout, LANG("\n\007Unable to update public key ring.\n"));
	    rmtemp(fname);
        } else {
	    keygen_OK = (savetemp(fname, globalPubringName) != NULL);
	}
    }

    /* Finally, sign the newly created userid on the public key... */
    if (keygen_OK && sign_new_userids) {
	long fp;
	int pktlen;
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
        byte sigguffin[256];

	PascalToC((char *) userid);
        extract_keyID(keyID, n);
	getpublickey(GPK_GIVEUP, globalPubringName, &fp, &pktlen, NULL,
	             timestamp, userid, n, e);
	PascalToC((char *) userid);
        strcpy((char *)sigguffin, (char *)userid);
        do_sign(globalPubringName, fp, pktlen, userid, keyID, (char *)sigguffin, TRUE);
    }

    mp_burn(e);
    mp_burn(n);

    if (keygen_OK)
        fputs(LANG("\007Key generation completed.\n"), pgpout);
    else
	return -1;		/* error return */

    /*
     *    If we need a seed file, create it now.
     */
    if (cryptrandflag) {
	trueRandConsume(192);
	cryptRandInit((struct IdeaCfbContext *)0);
	/* It will get saved by exitPGP */
    }
    return 0;			/* normal return */
}				/* dokeygen */

/* Does double duty for both -kv[v] and -kxa  */
void kv_title(FILE *fo)
{
    fprintf(fo, LANG("Type Bits/KeyID    Date       User ID\n"));
    return;
} /* kv_title */

/* Does double duty for both -kv[v] and -kxa  */
/* returns status & keycounter*/
int kvformat_keypacket(FILE *f, FILE *pgpout, boolean one_key,
                       char *mcguffin, char *ringfile,
                       boolean show_signatures, boolean show_hashes,
                       int *keycounter)
{
    byte ctb, keyctb=0;
    int status;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    byte keyID[KEYFRAGSIZE];
    byte sigkeyID[KEYFRAGSIZE];
    byte userid[256];               /* key certificate userid */
    char *siguserid;        /* signator userid */
    word32 tstamp;
    byte *timestamp = (byte *) &tstamp;             /* key certificate timestamp */
    int firstuser = 0;
    int compromised = 0;
    boolean shownKeyHash=FALSE;
    boolean invalid_key=FALSE;      /* unsupported version or bad data */
    boolean match = FALSE;
    boolean disabled = FALSE;
    boolean first_key= FALSE;

    for (;;) {
	status = readkeypacket(f, FALSE, &ctb, timestamp, (char *) userid,
			       n, e,
			       NULL, NULL, NULL, NULL, sigkeyID, NULL);
	/* Note that readkeypacket has called set_precision */
	if (status == -1) {
	    status = 0;
	    break;		/* eof reached */
	}
	if (status == -4 || status == -6) {
	    /* only ctb and userid are valid */
	    memset(sigkeyID, 0, KEYFRAGSIZE);
	    tstamp = 0;
	} else if (status < 0) {
	    fprintf(pgpout, LANG("\n\007Could not read key from file '%s'.\n"),
		    ringfile);
	    break;
	}
	if (is_key_ctb(ctb)) {
	    byte keyctrl;

	    firstuser = 1;
	    keyctb = ctb;
	    compromised = is_compromised(f);
	    shownKeyHash = FALSE;
	    if (status < 0) {
		invalid_key = TRUE;
		memset(keyID, 0, KEYFRAGSIZE);
	    } else {
		invalid_key = FALSE;
		extract_keyID(keyID, n);
		if (read_trust(f, &keyctrl) == 0 && (keyctrl & KC_DISABLED))
		    disabled = TRUE;
		else
		    disabled = FALSE;
	    }
	}
	if (ctb != CTB_USERID && !is_ctb_type(ctb, CTB_SKE_TYPE))
	    continue;
	if (ctb == CTB_USERID) {
	    PascalToC((char *) userid);
	    match = userid_match((char *) userid, mcguffin, n);
	}
	if (match) {
	    if (ctb == CTB_USERID) {
		if (firstuser) {
		    (*keycounter)++;
		    if (is_ctb_type(keyctb, CTB_CERT_PUBKEY_TYPE))
			fprintf(pgpout, LANG("pub"));
		    else if (is_ctb_type(keyctb, CTB_CERT_SECKEY_TYPE))
			fprintf(pgpout, LANG("sec"));
		    else
			fprintf(pgpout, "???");
		    if (invalid_key)
			fprintf(pgpout, "? ");
		    else if (disabled)
			fprintf(pgpout, "- ");
		    else
			fprintf(pgpout, "  ");
		    fprintf(pgpout, "%4d/%s %s ",
		       countbits(n), keyIDstring(keyID), cdate(&tstamp));
		} else {
		    fprintf(pgpout, "     %s                 ", blankkeyID);
		}
		if (compromised && firstuser) {
		    fprintf(pgpout, LANG("*** KEY REVOKED ***\n"));
		    fprintf(pgpout, "     %s                 ", blankkeyID);
		}
		firstuser = 0;
		fprintf(pgpout, "%s\n", LOCAL_CHARSET((char *) userid));

		/* Display the hashes for n and e if required */
		if (show_hashes && !shownKeyHash) {
		    showKeyHash(n, e);
		    shownKeyHash = TRUE;
		}
	    } else if (show_signatures &&
		       !(firstuser && compromised)) {
		/* Must be sig cert */
		fprintf(pgpout, LANG("sig"));
		fprintf(pgpout, "%c      ", status < 0 ? '?' : ' ');
		showkeyID(sigkeyID, pgpout);
		fprintf(pgpout, "             "); /* Indent signator userid */
		if ((siguserid = user_from_keyID(sigkeyID)) == NULL)
		    fprintf(pgpout,
			    LANG("(Unknown signator, can't be checked)\n"));
		else
		    fprintf(pgpout, "%s\n", LOCAL_CHARSET(siguserid));
	    }			/* printing a sig cert */
	}			/* if it has mcguffin */
    }				/* loop for all packets */
    return(status);
} /* kvformat_keypacket */
