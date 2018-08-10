/* keymgmt.h - headers for keymgmt.c 
*/

#include "idea.h" /* Declaration of IdeaCfbContext */

/*	Return printable public key fragment. */
char *keyIDstring(byte *keyID);
char *key2IDstring(unitptr n);
extern char const blankkeyID[];

/* Do an RSA key pair generation, and write them out to the keyring files. */
int dokeygen(char *numstr, char *numstr2, char *username);

/* Edit the userid and/or pass phrase for an RSA key pair, and put them	back
   into the ring files */
int dokeyedit(char *mcguffin, char *ringfile);

/* Copy the first entry in key ring that has mcguffin string in userid and
   put it into keyfile */
int extract_from_keyring (char *mcguffin, char *keyfile, char *ringfile,
						  boolean transflag);

/* Lists all entries in keyring that have mcguffin string in userid */
int view_keyring(char *mcguffin, char *ringfile,
		 boolean show_signatures, boolean show_hashes);

/* Signature-check all entries in keyring that have mcguffin string
   in userid */
int dokeycheck(char *mcguffin, char *ringfile, int options);
/* options: */
#define	CHECK_ALL	0	/* Check all signatures */
#define CHECK_NEW	1	/* Only check new signatures */

/* Allow user to remove signatures from keys in keyring that have mcguffin */
int remove_sigs(char *mcguffin, char *ringfile);

/* Remove the first entry in key ring that has mcguffin string in userid */
int remove_from_keyring(byte *keyID, char *mcguffin,
			char *ringfile, boolean secring_too);

/* Extract key fragment from modulus n */
void extract_keyID(byteptr keyID, unitptr n);

/* Write message prefix keyID to a file */
void writekeyID(unitptr n, FILE *f);

/* Extract public key corresponding to keyID or userid from keyfile */
int getpublickey(int flags, char *keyfile,
		 long *file_position, int *pktlen, byte *keyID,
		 byte *timestamp, byte *userid, unitptr n,
		 unitptr e);
/* flags: */
#define	GPK_GIVEUP	1
#define	GPK_SHOW	2
#define	GPK_NORVK	4
#define	GPK_DISABLED	8
/* Flag used in getsecretkey() only - should it be GSK_? */
/* Prevents use of existing password list. */
#define GPK_ASKPASS	16
#define GPK_SECRET	32	/* We are actually getting a secret key */

#ifdef MACTC5
int getfirstsecretkey(boolean giveup, boolean showkey, char *keyfile, byte *keyID,
	byte *timestamp, char *passp, boolean *hkey,
	byte *userid, unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
	unitptr u);
#endif

/* Extract private key corresponding to keyID or userid from keyfile */
int getsecretkey(int flags, char *keyfile, byte *keyID, byte *timestamp,
			byte *hpass, boolean *hkey, byte *userid,
			unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
			unitptr u);

/* Return true if ctb is one for a key in a keyring */
int is_key_ctb (byte ctb);

/* Read next key packet from file f, return its ctb in *pctb, and advance
 * the file pointer to point beyond the key packet.
 */
short nextkeypacket(FILE *f, byte *pctb);

/* Read the next key packet from file f, return info about it in the various
 * pointers.  Most pointers can be NULL without breaking it.
 */
short readkeypacket(FILE *f, struct IdeaCfbContext *cfb, byte *pctb,
	byte *timestamp, char *userid,
	unitptr n ,unitptr e, unitptr d, unitptr p, unitptr q, unitptr u,
	byte *sigkeyID, byte *keyctrl);

/* Starting at key_position in keyfile, scan for the userid packet which
 * matches C string userid.  Return the packet position and size.
 */
int getpubuserid(char *keyfile, long key_position, byte *userid,
	long *userid_position, int *userid_len, boolean exact_match);

int getpubusersig(char *keyfile, long user_position, byte *sigkeyID,
	byte *timestamp, long *sig_position, int *sig_len);

void getKeyHash( byte *hash, unitptr n, unitptr e );
void printKeyHash( byteptr hash, boolean indent );

extern int is_compromised(FILE *f);

int disable_key(char *, char *);

void kv_title(FILE *fo);

int  kvformat_keypacket(FILE *f, FILE *pgpout, boolean one_key,
                        char *mcguffin, char *ringfile,
                        boolean show_signatures, boolean show_hashes,
                        int *keycounter);
