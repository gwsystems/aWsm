/*
	Pretty Good(tm) Privacy - RSA public key cryptography for the masses
	Written by Philip Zimmermann, Phil's Pretty Good(tm) Software.
	Version 1.0 - 5 Jun 91, last revised 6 Jul 91 by PRZ

	This file defines the various formats, filenames, and general control
	methods used by PGP, as well as a few global switches which control
	the functioning of the driver code.

*/

#ifndef PGP_H
#define PGP_H

#include "usuals.h"
#include "more.h"
#include "armor.h"

#define KEYFRAGSIZE 8	/* # of bytes in key ID modulus fragment */
#define SIZEOF_TIMESTAMP 4 /* 32-bit timestamp */

/* The maximum length of the file path for this system.  Varies on UNIX
   systems */

#ifndef	MAX_PATH
#ifdef MSDOS
#define MAX_PATH	64
#else
#define MAX_PATH	256
#endif
#endif

#ifdef __PUREC__
#define sizeof(x) (int)sizeof(x)
#endif

/*
**********************************************************************
*/

/* Cipher Type Byte (CTB) definitions follow...*/
#define CTB_DESIGNATOR 0x80
#define is_ctb(c) (((c) & CTB_DESIGNATOR)==CTB_DESIGNATOR)
#define CTB_TYPE_MASK 0x7c
#define CTB_LLEN_MASK 0x03

/* "length of length" field of packet, in bytes (1, 2, 4, 8 bytes): */
#define ctb_llength(ctb) ((int) 1 << (int) ((ctb) & CTB_LLEN_MASK))

#define is_ctb_type(ctb,type) (((ctb) & CTB_TYPE_MASK)==(4*type))
#define CTB_BYTE(type,llen) (CTB_DESIGNATOR + (4*type) + llen)

#define CTB_PKE_TYPE 1			/* packet encrypted with RSA public
					   key */
#define CTB_SKE_TYPE 2			/* packet signed with RSA secret key */
#define CTB_MD_TYPE 3			/* message digest packet */
#define CTB_CERT_SECKEY_TYPE 5  /* secret key certificate */
#define CTB_CERT_PUBKEY_TYPE 6  /* public key certificate */
#define CTB_COMPRESSED_TYPE 8	/* compressed data packet */
#define CTB_CKE_TYPE 9			/* conventional-key-encrypted data */
#define	CTB_LITERAL_TYPE 10		/* raw data with filename and mode */
#define CTB_LITERAL2_TYPE 11	/* Fixed literal packet */
#define CTB_KEYCTRL_TYPE 12		/* key control packet */
#define CTB_USERID_TYPE 13		/* user id packet */
#define CTB_COMMENT_TYPE 14		/* comment packet */

/* Unimplemented CTB packet types follow... */
/* #define CTB_EXTENDED_TYPE 15 */ /* 2-byte CTB, 256 extra CTB types */

#define CTB_PKE CTB_BYTE(CTB_PKE_TYPE,1)
	/* CTB_PKE len16 keyID mpi(RSA(CONKEYPKT)) */
	/*	  1		 2	 SIZE  countbytes()+2 */
#define CTB_SKE CTB_BYTE(CTB_SKE_TYPE,1)
	/* CTB_SKE len16 keyID mpi(RSA(MDPKT)) */
	/*	  1		 2	 SIZE  countbytes()+2 */
#define CTB_MD CTB_BYTE(CTB_MD_TYPE,0)
	/* CTB_MD len8 algorithm MD timestamp */
#define CTB_CERT_SECKEY CTB_BYTE(CTB_CERT_SECKEY_TYPE,1)
	/* CTB_CERT_SECKEY len16 timestamp userID mpi(n) mpi(e) mpi(d)
	   mpi(p) mpi(q) mpi(u) crc16 */
#define CTB_CERT_PUBKEY CTB_BYTE(CTB_CERT_PUBKEY_TYPE,1)
	/* CTB_CERT_PUBKEY len16 timestamp userID mpi(n) mpi(e) crc16 */

#define CTB_KEYCTRL CTB_BYTE(CTB_KEYCTRL_TYPE,0)
#define	CTB_USERID	CTB_BYTE(CTB_USERID_TYPE,0)

#define CTB_CKE CTB_BYTE(CTB_CKE_TYPE,3)
	/*	CTB_CKE ciphertext */

#define CTB_LITERAL CTB_BYTE(CTB_LITERAL_TYPE,3)
#define CTB_LITERAL2 CTB_BYTE(CTB_LITERAL_TYPE,3)
	/*	CTB_LITERAL data */

#define CTB_COMPRESSED CTB_BYTE(CTB_COMPRESSED_TYPE,3)
	/*	CTB_COMPRESSED compressedtext */

/*	Public key encryption algorithm selector bytes. */
#define RSA_ALGORITHM_BYTE	1	/*	use RSA	*/

/*	Conventional encryption algorithm selector bytes. */
#define IDEA_ALGORITHM_BYTE	1	/*	use the IDEA cipher */

/*	Message digest algorithm selector bytes. */
#define MD5_ALGORITHM_BYTE 1	/* MD5 message digest algorithm */

/*	Data compression algorithm selector bytes. */
#define ZIP2_ALGORITHM_BYTE  1	/* Zip-based deflate compression algorithm */

/* Signature classification bytes. */
#define SB_SIGNATURE_BYTE	0x00	/* Signature of a binary msg or doc */
#define SM_SIGNATURE_BYTE	0x01	/* Signature of canonical msg or doc */
#define	K0_SIGNATURE_BYTE	0x10	/* Key certification, generic */
#define	K1_SIGNATURE_BYTE	0x11	/* Key certification, persona */
#define	K2_SIGNATURE_BYTE	0x12	/* Key certification, casual ID */
#define	K3_SIGNATURE_BYTE	0x13	/* Key certification, positive ID */
#define KC_SIGNATURE_BYTE	0x20	/* Key compromise */
#define KR_SIGNATURE_BYTE	0x30	/* Key revocation */
#define	TS_SIGNATURE_BYTE	0x40	/* Timestamp someone else's
					   signature */

/* Public key encrypted data classification bytes. */
#define MD_ENCRYPTED_BYTE	1	/* Message digest is encrypted */
#define CK_ENCRYPTED_BYTE	2	/* Conventional key is encrypted */

/* Version byte for data structures created by this version of PGP */
#define	VERSION_BYTE_OLD	2	/* PGP2 */
#define	VERSION_BYTE_NEW	3

/* Values for trust bits in keycntrl packet after key packet */
#define	KC_OWNERTRUST_MASK	0x07	/* Trust bits for key owner */
#define	KC_OWNERTRUST_UNDEFINED	0x00
#define	KC_OWNERTRUST_UNKNOWN	0x01
#define	KC_OWNERTRUST_NEVER	0x02
/* 2 levels reserved */
#define	KC_OWNERTRUST_USUALLY	0x05
#define	KC_OWNERTRUST_ALWAYS	0x06
#define	KC_OWNERTRUST_ULTIMATE	0x07	/* Only for keys in secret ring */
#define	KC_BUCKSTOP		0x80	/* This key is in secret ring */
#define	KC_DISABLED		0x20	/* key is disabled */

/* Values for trust bits in keycntrl packet after userid packet */
#define	KC_LEGIT_MASK		0x03	/* Key legit bits for key */
#define	KC_LEGIT_UNKNOWN	0x00
#define KC_LEGIT_UNTRUSTED	0x01
#define KC_LEGIT_MARGINAL	0x02
#define	KC_LEGIT_COMPLETE	0x03
#define	KC_WARNONLY		0x80

/* Values for trust bits in keycntrl packet after signature packet */
#define	KC_SIGTRUST_MASK	0x07	/* Trust bits for key owner */
#define	KC_SIGTRUST_UNDEFINED	0x00
#define	KC_SIGTRUST_UNKNOWN	0x01
#define	KC_SIGTRUST_UNTRUSTED	0x02
/* 2 levels reserved */
#define	KC_SIGTRUST_MARGINAL	0x05
#define	KC_SIGTRUST_COMPLETE	0x06
#define	KC_SIGTRUST_ULTIMATE	0x07
#define	KC_SIG_CHECKED		0x40	/* This sig has been checked */
#define	KC_CONTIG		0x80	/* This sig is on a cert. path */

#define is_secret_key(ctb) is_ctb_type(ctb,CTB_CERT_SECKEY_TYPE)

#define MPILEN (2+MAX_BYTE_PRECISION)
#define MAX_SIGCERT_LENGTH (1+2+1 +1+7 +KEYFRAGSIZE+2+2+MPILEN)
#define MAX_KEYCERT_LENGTH (1+2+1+4+2+1 +(2*MPILEN) +1+8 +(4*MPILEN) +2)

/* Modes for CTB_LITERAL2 packet */
#ifdef EBCDIC
#define	MODE_BINARY	0x62
#define	MODE_TEXT	0x74
#define MODE_LOCAL	0x6c
#else
#define	MODE_BINARY	'b'
#define	MODE_TEXT	't'
#define MODE_LOCAL	'l'
#endif

void user_error(void);

/* Global filenames and system-wide file extensions... */
extern char PGP_EXTENSION[];
extern char ASC_EXTENSION[];
extern char SIG_EXTENSION[];
extern char BAK_EXTENSION[];
extern char CONSOLE_FILENAME[];
extern char rel_version[];

/* These files use the environmental variable PGPPATH as a default path: */
extern char globalPubringName[MAX_PATH];
extern char globalSecringName[MAX_PATH];
extern char globalRandseedName[MAX_PATH];
extern char globalCommentString[128];

/* Variables which are global across the driver code */
extern boolean	filter_mode;
extern boolean	moreflag;
extern FILE	*pgpout;	/* FILE structure for routine output */

/* Variables settable by config.pgp and referenced in config.c ... */
extern char language[];	/* foreign language prefix code for language.pgp
			   file */
extern char charset[];
extern char charset_header[];
/* my_name is substring of default userid for secret key to make signatures */
extern char my_name[];
extern char floppyring[]; /* for comparing secret keys with backup on floppy */
extern char literal_mode;	/* text or binary mode for literal packet */
extern boolean emit_radix_64;
extern boolean showpass;
extern boolean keepctx;
extern boolean verbose;	/* display maximum information */
extern boolean compress_enabled; /* attempt compression before encryption */
extern boolean clear_signatures;
extern boolean encrypt_to_self; /* Should I encrypt to myself? */
extern boolean sign_new_userids;
extern boolean batchmode;	/* for batch processing */
extern boolean quietmode;	/* less verbose */
extern boolean force_flag;	/* overwrite existing file without asking */
/* Ask for each key separately if it should be added to the keyring */
extern boolean interactive_add;
extern long timeshift;	/* seconds from GMT timezone */
extern boolean signature_checked;
extern int checksig_pass;
extern int pem_lines;
extern int marg_min;	/* number of marginally trusted signatures needed to
						   make a key fully-legit */
extern int compl_min;	/* number of fully trusted signatures needed */
extern int max_cert_depth;
extern char pager[];	/* file lister command */
extern int version_byte;
extern boolean nomanual;
extern int makerandom;	/* Fill in file with this many random bytes */

/* These lists store hashed passwords for future use. */
/* passwds are passwords of as-yet-unknown purpose; keypasswds
   are passwords used to decrypt keys. */
struct hashedpw {
	struct hashedpw *next;
	byte hash[16];
};
extern struct hashedpw *keypasswds, *passwds;

extern boolean strip_spaces;
extern boolean use_charset_header;

#ifdef VMS
/*
 * FDL Support Prototypes, Currently Used Only In SYSTEM.C and CRYPTO.C
 */

int fdl_generate(char *in_file, char **fdl, short *len);
VOID *fdl_create( char *fdl, short len, char *outfile, char *preserved_name);
int fdl_copyfile2bin(FILE *f, VOID *rab, word32 longcount); 
void fdl_close( VOID *rab);
#endif /* VMS */

extern int compressSignature(byte *header);

#endif /* PGP_H */
