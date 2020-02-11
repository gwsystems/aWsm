/*      keymaint.c  - Keyring maintenance pass routines for PGP.
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

   keymaint.c implemented by Branko Lankester.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "keymaint.h"
#include "mpiio.h"
#include "charset.h"
#include "language.h"
#include "pgp.h"
#ifdef MACTC5
#include "Macutil3.h"
#include "PGPDialogs.h"
#include "MyBufferedStdio.h"
#include "ReplaceStdio.h"
#endif

#if 1				/* def DEBUG */
#include <assert.h>
#else
#define assert(x)
#endif

/* Helper functions to work on newkey lists */
void free_newkeys(struct newkey *nkeys)
{
    struct newkey *nkey;

    while (nkeys) {
	nkey = nkeys;
	nkeys = nkeys->next;
	free(nkey);
    }
}

int ismember_newkeys(byte const keyid[KEYFRAGSIZE], struct newkey const *nkeys)
{
    while (nkeys) {
	if (memcmp(keyid, nkeys->keyID, KEYFRAGSIZE) == 0)
	    return 1;
	nkeys = nkeys->next;
    }
    return 0;
}

/* The main checking code... */

struct userid;
struct signature;

struct pubkey {
    struct pubkey *pk_next;
    struct pubkey *pk_hash;	/* hash list for keyID */
    struct userid *pk_userids;
    struct signature *pk_signed;	/* signatures this key made */
    byte pk_keyid[KEYFRAGSIZE];
    byte pk_owntrust;
    byte pk_depth;		/* shortest cert. path to buckstop key */
};

struct userid {
    struct userid *uid_next;
    struct pubkey *uid_key;	/* backlink to key */
    struct signature *uid_signatures;
    char *uid_userid;
    byte uid_legit;
};

struct signature {
    struct signature *sig_next;	/* list of signatures on a userid */
    struct userid *sig_uid;	/* the userid it signs */
    struct pubkey *sig_from;	/* key that made this signature */
    /* list of sigs made by the same key (sig_from) */
    struct signature *sig_nextfrom;
    byte sig_trust;
};


int maint_list(char *ringfile);
void init_trust_lst(void);
long lookup_by_keyID(FILE * f, byte * srch_keyID);
void show_userid(FILE * f, byte * keyID);

static int maintenance(char *ringfile, struct newkey const *nkeys);
static int maint_read_data(char *ringfile, struct newkey const *nkeys);
static int maint_trace_chain(void);
static int trace_sig_chain(struct pubkey *pk, int depth);
static int maint_final(char *ringfile);
static struct pubkey *getpubkey(byte * keyID);
static void setup_trust(void);
static int check_secretkey(FILE * f, long keypos, byte keyctrl);
static void maint_init_mem(void);
static void maint_release_mem(void);
static VOID *allocn(int size);
static VOID *allocbuf(int size);
static void freebufpool(void);
static void compute_legit(struct userid *id);


#define	ALLOC_UNIT 4000	/* memory will be allocated in chunks of this size */

#define	MAX_DEPTH  8		/* max. value of max_cert_depth */

/* returned when trying to do a maintenance pass on a
   secret keyring or keyfile */
#define	ERR_NOTRUST	-7

#define TRUST_MASK	7	/* mask for userid/signature trust bytes */
#define SET_TRUST(b,v)	(*(b) = (*(b) & ~TRUST_MASK) | (v))
#define TRUST_LEV(b)	((b) & TRUST_MASK)

#define	TRUST_FAC(x)	(trust_tbl[TRUST_LEV(x)])

#define ctb_type(c)	((c&CTB_TYPE_MASK)>>2)
/*
 * table for tuning user paranoia index.
 * values represent contribution of one signature indexed by the
 * SIGTRUST of a signature
 */
static int trust_tbl[8];

static int marginal_min;
static int complete_min;	/* total count needed for a fully legit key */

int marg_min = 2;		/* number of marginally trusted signatures
				   needed for a fully legit key
				   (can be set in config.pgp). */
int compl_min = 1;		/* number of fully trusted signatures needed */

char trust_lst[8][16] =
{
    "undefined",		/* LANG("undefined") */
    "unknown",			/* LANG("unknown") */
    "untrusted",		/* LANG("untrusted") */
    "<3>",			/* unused */
    "<4>",			/* unused */
    "marginal",			/* LANG("marginal") */
    "complete",			/* LANG("complete") */
    "ultimate",			/* LANG("ultimate") */
};

char legit_lst[4][16] =
{
    "undefined",
    "untrusted",
    "marginal",
    "complete"
};

static int trustlst_len = 9;	/* length of longest trust word */
static int legitlst_len = 9;	/* length of longest legit word */

char floppyring[MAX_PATH] = "";
int max_cert_depth = 4;		/* maximum nesting of signatures */

static boolean check_only = FALSE;
static boolean mverbose;
static FILE *sec_fp;
static FILE *floppy_fp = NULL;
static int undefined_trust;	/* number of complete keys with undef. trust */

/*
 * Update trust parameters in a keyring, should be called after all
 * key management functions which can affect the trust parameters.
 * Changes are done "inplace", the file must be writable.
 *
 * nkeys is a list of new keys.  Any key on this list is checked to
 * see if it on the secret keyring.  If it is, and the BUCKSTOP bit
 * is not set, the user is prompted to set it.
 */
int maint_update(char *ringfile, struct newkey const *nkeys)
{
    check_only = mverbose = FALSE;
    return maintenance(ringfile, nkeys);
}

/*
 * Check trust parameters in ringfile
 * options can be:
 *      MAINT_CHECK     check only, don't ask if keyring should be updated
 *      MAINT_VERBOSE   verbose output, shows signature chains
 */
int maint_check(char *ringfile, int options)
{
    int status;
    char *fixfile;

    mverbose = ((options & MAINT_VERBOSE) != 0);

    if (moreflag)
	open_more();
    if (*floppyring != '\0' && (floppy_fp = fopen(floppyring, FOPRBIN))
	== NULL)
	fprintf(pgpout, LANG("\nCan't open backup key ring file '%s'\n"),
		floppyring);
    check_only = TRUE;
    status = maintenance(ringfile, NULL);
    if (floppy_fp) {
	fclose(floppy_fp);
	floppy_fp = NULL;
    }
    if (status <= 0) {
	if (status == 0)
	    maint_list(ringfile);
	close_more();
	return status;
    }
#ifdef xDEBUG
    if (status > 0 && (options & MAINT_CHECK)) {
	FILE *sav = pgpout;
	if (pgpout = fopen("before.lst", "w")) {
	    maint_list(ringfile);
	    fclose(pgpout);
	}
	pgpout = sav;
    }
#endif
    /* Inform user of trust parameters to be changed... */
    if (undefined_trust) {

	/* If we are just going to check, then exit now... */
	if (options & MAINT_CHECK) {
	    maint_list(ringfile);
	}
	fprintf(pgpout,
		LANG("\n%d \"trust parameter(s)\" need to be changed.\n"),
		undefined_trust);

	if (options & MAINT_CHECK) {
	    close_more();
	    return status;
	}
	fprintf(pgpout, LANG("Continue with '%s' (Y/n)? "),
		ringfile);
	if (!getyesno('y')) {
	    close_more();
	    return status;
	}
    }
    /* do the fixes in a scratch file */
    fixfile = tempfile(0);
    if (copyfiles_by_name(ringfile, fixfile) < 0) {
	close_more();
	return -1;
    }
    check_only = mverbose = FALSE;
    if ((status = maintenance(fixfile, NULL)) >= 0) {
	maint_list(fixfile);
	fprintf(pgpout,
		LANG("\n%d \"trust parameter(s)\" changed.\n"), status);
    }
    close_more();
    if (status > 0 && !(options & MAINT_CHECK)) {
	fprintf(pgpout, LANG("Update public keyring '%s' (Y/n)? "), ringfile);
	if (getyesno('y'))
	    return savetempbak(fixfile, ringfile);
    }
    rmtemp(fixfile);
    return status;
}				/* maint_check */


static int maintenance(char *ringfile, struct newkey const *nkeys)
{
    int status;
    undefined_trust = 0;	/* None so far... */

    if (max_cert_depth > MAX_DEPTH)
	max_cert_depth = MAX_DEPTH;
    if ((sec_fp = fopen(globalSecringName, FOPRBIN)) == NULL)
	fprintf(pgpout, LANG("\nCan't open secret key ring file '%s'\n"),
		globalSecringName);

    setkrent(ringfile);
    setup_trust();
    maint_init_mem();
    if (mverbose || verbose)
	fprintf(pgpout,
	 LANG("\nPass 1: Looking for the \"ultimately-trusted\" keys...\n"));
    status = maint_read_data(ringfile, nkeys);
    if (sec_fp) {
	fclose(sec_fp);
	sec_fp = NULL;
    }
    if (status < 0)
	goto failed;

    if (mverbose || verbose)
	fprintf(pgpout, LANG("\nPass 2: Tracing signature chains...\n"));
    if ((status = maint_trace_chain()) < 0)
	goto failed;

    if (verbose)
	fprintf(pgpout, "\nPass 3: %s keyring...\n",
		(check_only ? "Checking with" : "Updating"));
    if ((status = maint_final(ringfile)) < 0)
	goto failed;

    endkrent();
    maint_release_mem();
    return status + undefined_trust;

  failed:
    if (verbose)
	fprintf(pgpout, "maintenance pass: error exit = %d\n", status);
    endkrent();
    maint_release_mem();
    return status;
}				/* maintenance */


static struct pubkey *pklist, **pkhash = NULL;

#define	PK_HASHSIZE	256	/* must be power of 2 */
#define	PK_HASH(x)		(*(byte *) (x) & (PK_HASHSIZE - 1))

/*
 * get the pubkey struct for keyID from hash table, allocate a new
 * node and insert in hash table if necessary.
 */
static struct pubkey *
 getpubkey(byte * keyID)
{
    struct pubkey *pk;
    for (pk = pkhash[PK_HASH(keyID)]; pk; pk = pk->pk_hash)
	if (memcmp(pk->pk_keyid, keyID, KEYFRAGSIZE) == 0)
	    return pk;
    pk = allocn(sizeof(struct pubkey));
    memset(pk, 0, sizeof(struct pubkey));
    memcpy(pk->pk_keyid, keyID, KEYFRAGSIZE);
    pk->pk_hash = pkhash[PK_HASH(keyID)];
    pkhash[PK_HASH(keyID)] = pk;
    return pk;
}

/*
 * Read in keyring, a graph of keys, userids and signatures is built.
 * Also check if axiomatic keys are present in the secret keyring and
 * compare them with the floppy ring if this is requested.
 */
static int maint_read_data(char *ringfile, struct newkey const *nkeys)
{
    FILE *f;
    int status;
    char userid[256];
    byte keyID[KEYFRAGSIZE];
    byte sigkeyID[KEYFRAGSIZE];
    byte ctb;
    byte keyctrl;
    boolean buckstop = FALSE, show_user = FALSE;
    int buckstopcount = 0;
    long keypos = 0;
    int skip = 0;
    struct pubkey *pk = NULL;
    struct userid *id = NULL;
    struct signature *sig = NULL;

    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout,
		LANG("\n\007Can't open key ring file '%s'\n"), ringfile);
	return -1;
    }
    while ((status = readkpacket(f, &ctb, userid, keyID, sigkeyID)) != -1) {
	if (status == -3 || status == -2) {
	    fclose(f);
	    return status;
	}
	if (status < 0 || is_ctb_type(ctb, CTB_CERT_SECKEY_TYPE)) {
	    skip = 1;		/* version error or bad key */
	    continue;
	}
	if (skip) {
	    if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
		skip = 0;
	    else
		continue;
	}
	if (is_ctb_type(ctb, CTB_COMMENT_TYPE) || ctb == CTB_KEYCTRL)
	    continue;

	if (pk && is_ctb_type(ctb, CTB_SKE_TYPE) && !pk->pk_userids) {
	    /* sig. cert before userids can only be compromise cert. */
	    pk->pk_owntrust = KC_OWNERTRUST_NEVER;
	    continue;
	}
	/* other packets should have trust byte */
	if (read_trust(f, &keyctrl) < 0) {
	    fclose(f);
	    return ERR_NOTRUST;	/* not a public keyring */
	}
	switch (ctb_type(ctb)) {
	case CTB_CERT_PUBKEY_TYPE:
	    if (pk)
		pk = pk->pk_next = getpubkey(keyID);
	    else
		pk = pklist = getpubkey(keyID);

	    if (pk->pk_next) {
		fprintf(pgpout,
			LANG("Keyring contains duplicate key: %s\n"),
			keyIDstring(keyID));
		fclose(f);
		return -1;
	    }
	    if (keyctrl & KC_BUCKSTOP ||
		ismember_newkeys(keyID, nkeys)) {
		if (check_secretkey(f, keypos, keyctrl) == 0) {
		    ++buckstopcount;
		    keyctrl |= KC_BUCKSTOP;
		    SET_TRUST(&keyctrl, KC_OWNERTRUST_ULTIMATE);
		    buckstop = TRUE;
		    if (mverbose)
			fprintf(pgpout, "* %s", keyIDstring(keyID));
		} else {	/* not in secret keyring */
		    keyctrl &= ~KC_BUCKSTOP;
		    if (TRUST_LEV(keyctrl) == KC_OWNERTRUST_ULTIMATE)
			keyctrl = KC_OWNERTRUST_ALWAYS;
		    if (mverbose)
			fprintf(pgpout, ". %s", keyIDstring(keyID));
		}
		show_user = mverbose;
	    } else {
		buckstop = FALSE;
		show_user = FALSE;
	    }
	    pk->pk_owntrust = keyctrl;
	    pk->pk_userids = id = NULL;
	    break;
	case CTB_USERID_TYPE:
#ifdef MACTC5
		mac_poll_for_break();
#endif
	    if (!pk)
		break;
	    if (show_user) {
		if (pk->pk_userids)	/* more than one user ID */
		    fprintf(pgpout, "        ");
		fprintf(pgpout, "  %s\n", LOCAL_CHARSET(userid));
	    }
	    if (id)
		id = id->uid_next = allocn(sizeof(struct userid));
	    else
		id = pk->pk_userids = allocn(sizeof(struct userid));

	    if (mverbose)
		id->uid_userid = store_str(userid);
	    keyctrl &= ~KC_LEGIT_MASK;
	    if (buckstop)
		keyctrl |= KC_LEGIT_COMPLETE;
	    else
		keyctrl |= KC_LEGIT_UNKNOWN;
	    id->uid_next = NULL;
	    id->uid_key = pk;
	    id->uid_legit = keyctrl;
	    id->uid_signatures = sig = NULL;
	    break;
	case CTB_SKE_TYPE:
	    if (!pk || !id)
		break;
	    if (sig)
		sig = sig->sig_next = allocn(sizeof(struct signature));
	    else
		sig = id->uid_signatures = allocn(sizeof(struct signature));
	    sig->sig_next = NULL;
	    sig->sig_uid = id;
	    sig->sig_from = getpubkey(sigkeyID);
	    sig->sig_nextfrom = sig->sig_from->pk_signed;
	    sig->sig_from->pk_signed = sig;
	    sig->sig_trust = keyctrl & KC_SIG_CHECKED;
	    break;
	}			/* switch ctb_type */
	keypos = ftell(f);
    }
    if (buckstopcount == 0 && mverbose)
	fprintf(pgpout, LANG("No ultimately-trusted keys.\n"));
    fclose(f);
    return 0;
}				/* maint_read_data */

/*
 * scan keyring for buckstop keys and start the recursive trace_sig_chain()
 * on them
 */
static int maint_trace_chain(void)
{
    char *userid;
    struct pubkey *pk;

    for (pk = pklist; pk; pk = pk->pk_next) {
	if (!(pk->pk_owntrust & KC_BUCKSTOP))
	    continue;
	if (mverbose)
	    fprintf(pgpout,
		    "* %s\n", LOCAL_CHARSET(pk->pk_userids->uid_userid));
	if (TRUST_LEV(pk->pk_owntrust) == KC_OWNERTRUST_UNDEFINED) {
	    userid = user_from_keyID(pk->pk_keyid);
	    SET_TRUST(&pk->pk_owntrust, ask_owntrust(userid, pk->pk_owntrust));
	}
	trace_sig_chain(pk, 0);
    }
    return 0;
}				/* maint_trace_chain */


/*
 * Find all signatures made with the key pk.
 * If a trusted signature makes a key fully legit then signatures made
 * with this key are also recursively traced on down the tree.
 *
 * depth is the level of recursion, it is used to indent the userIDs
 * and to check if we don't exceed the limit "max_cert_depth"
 *
 * NOTE: a signature made with a key with pk_depth == max_cert_depth will
 * not be counted here to limit the maximum chain length, but will be
 * counted when the validity of a key is computed in maint_final()
 */
static int trace_sig_chain(struct pubkey *pk, int depth)
{
    int d, trust_count = 0;
    int counts[MAX_DEPTH];
    struct signature *sig, *s;
    struct pubkey *p;
    struct userid *id;

    assert(depth <= max_cert_depth);
    if (pk->pk_depth && pk->pk_depth <= depth)
	return 0;
    pk->pk_depth = depth;

    /* Should we ask for trust.  If this key is legit, then go for
     * it!  Ask the user....
     */
    if (TRUST_LEV(pk->pk_owntrust) == KC_OWNERTRUST_UNDEFINED)
	for (id = pk->pk_userids; id; id = id->uid_next) {
	    compute_legit(id);
	    if ((id->uid_legit & KC_LEGIT_MASK) ==
		KC_LEGIT_COMPLETE) {
		SET_TRUST(&pk->pk_owntrust,
			  ask_owntrust(user_from_keyID(pk->pk_keyid),
				       pk->pk_owntrust));
		break;
	    }
	}
    /* Return if I haven't signed anyone's keys, since I
     * don't need to check any further..  -warlord 93-04-11
     */
    if (!pk->pk_signed)
	return 0;

#ifdef DEBUG
    if (mverbose)
	fprintf(pgpout, "%*s%d-v  %s\n", 2 * depth, "",
		depth, pk->pk_userids->uid_userid);
#endif

    /* all keys signed by pk */
    for (sig = pk->pk_signed; sig; sig = sig->sig_nextfrom) {

	/* If signature is good, copy trust from signator */
	/* CONTIG bit currently unused */
	if (sig->sig_trust & KC_SIG_CHECKED) {
	    SET_TRUST(&sig->sig_trust, TRUST_LEV(pk->pk_owntrust));
	    sig->sig_trust |= KC_CONTIG; /* CONTIG bit currently unused */
	    if (mverbose)
		fprintf(pgpout, "%*s  > %s\n", 2 * depth, "",
			LOCAL_CHARSET(sig->sig_uid->uid_userid));
	} else {
	    SET_TRUST(&sig->sig_trust, KC_SIGTRUST_UNTRUSTED);
	    sig->sig_trust &= ~KC_CONTIG;
	    if (mverbose)
		fprintf(pgpout, "%*s  X %s\n", 2 * depth, "",
			LOCAL_CHARSET(sig->sig_uid->uid_userid));
	}

	if (TRUST_FAC(sig->sig_trust) == 0)
	    continue;
	p = sig->sig_uid->uid_key;	/* this key signed by pk */
	if (p->pk_owntrust & KC_BUCKSTOP)
	    continue;		/* will be handled from main loop */
	if (p->pk_depth && p->pk_depth <= depth + 1)
	    continue;		/* already handled this key at a lower level */

	for (d = 0; d < max_cert_depth; ++d)
	    counts[d] = 0;
	for (s = sig->sig_uid->uid_signatures; s; s = s->sig_next) {
	    d = s->sig_from->pk_depth;
	    if (d < max_cert_depth)
		counts[d] += TRUST_FAC(s->sig_trust);
	}
	/*
	 * find a combination of signatures that will make the key
	 * valid through the shortest cert. path.
	 */
	trust_count = 0;
	for (d = 0; d < max_cert_depth; ++d) {
	    trust_count += counts[d];
	    if (trust_count >= complete_min) {
		trace_sig_chain(p, d + 1);
		break;
	    }
	}
    }

#ifdef DEBUG
    if (mverbose)
	fprintf(pgpout, "%*s%d-^  %s\n", 2 * depth, "",
		depth, pk->pk_userids->uid_userid);
#endif
    return 0;
}				/* trace_sig_chain */

/*
 * compute validity of userid/key pair, the number of signatures and the
 * trust level of these signatures determines the validity.
 */
static void compute_legit(struct userid *id)
{
    struct signature *s;
    int trust_count, legit;

    if (id->uid_key->pk_owntrust & KC_BUCKSTOP)
	legit = KC_LEGIT_COMPLETE;
    else {
	trust_count = 0;
	for (s = id->uid_signatures; s; s = s->sig_next)
	    trust_count += TRUST_FAC(s->sig_trust);

	if (trust_count == 0)
	    legit = KC_LEGIT_UNKNOWN;
	else if (trust_count < marginal_min)
	    legit = KC_LEGIT_UNTRUSTED;
	else if (trust_count < complete_min)
	    legit = KC_LEGIT_MARGINAL;
	else
	    legit = KC_LEGIT_COMPLETE;
    }
    id->uid_legit = (id->uid_legit & ~KC_LEGIT_MASK) | legit;
}				/* compute_legit */

/* 
 * check if the maintenance pass changed anything
 * returns 0 if files f and g are equal and the number of changed
 * trust bytes if the files are different or a negative value on error
 */
static int maint_final(char *ringfile)
{
    int status;
    FILE *f;
    long trust_pos = 0;
    char userid[256];
    byte keyID[KEYFRAGSIZE];
    byte sigkeyID[KEYFRAGSIZE];
    byte ctb;
    byte kc_orig, kc_new = 0, mask;
    int changed = 0;
    int skip = 0;
    struct pubkey *pk;
    struct userid *id = NULL;
    struct signature *sig = NULL;

    if (check_only)
	f = fopen(ringfile, FOPRBIN);
    else
	f = fopen(ringfile, FOPRWBIN);
    if (f == NULL) {
	fprintf(pgpout,
		LANG("\n\007Can't open key ring file '%s'\n"), ringfile);
	return -1;
    }
    pk = pklist;
    while ((status = readkpacket(f, &ctb, userid, keyID, sigkeyID)) != -1) {
	if (status == -3 || status == -2)
	    break;
	if (status < 0 || is_ctb_type(ctb, CTB_CERT_SECKEY_TYPE)) {
	    skip = 1;
	    continue;
	}
	if (skip) {
	    if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
		skip = 0;
	    else
		continue;
	}
	if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE) ||
	    is_ctb_type(ctb, CTB_SKE_TYPE) || ctb == CTB_USERID) {
	    trust_pos = ftell(f);
	    if (read_trust(f, &kc_orig) < 0) {
		status = ERR_NOTRUST;
		if (is_ctb_type(ctb, CTB_SKE_TYPE))
		    continue;	/* skip compr. cert. */
		else
		    break;
	    }
	}
	switch (ctb_type(ctb)) {
	case CTB_CERT_PUBKEY_TYPE:
	    assert(pk && !memcmp(pk->pk_keyid, keyID, KEYFRAGSIZE));
	    assert(!sig && !id);
	    id = pk->pk_userids;
	    kc_new = pk->pk_owntrust;
#ifdef DEBUG
	    if (mverbose)
		fprintf(pgpout, "  ------ %d\n", pk->pk_depth);
#endif
	    pk = pk->pk_next;
	    mask = KC_OWNERTRUST_MASK | KC_BUCKSTOP;
	    break;
	case CTB_USERID_TYPE:
	    assert(id && !sig);
#ifdef MACTC5
	    mac_poll_for_break();
#endif
	    sig = id->uid_signatures;
	    compute_legit(id);
	    kc_new = id->uid_legit;
#ifdef DEBUG
	    if (mverbose)
		fprintf(pgpout, "%c %02x  %02x  %s\n",
			' ' + (kc_new != kc_orig),
			kc_orig, kc_new, id->uid_userid);
#endif
	    id = id->uid_next;
	    mask = KC_LEGIT_MASK;
	    break;
	case CTB_SKE_TYPE:
	    assert(sig);
	    assert(!memcmp(sig->sig_from->pk_keyid, sigkeyID, KEYFRAGSIZE));
	    kc_new = sig->sig_trust;
#ifdef DEBUG
	    if (mverbose && sig->sig_from->pk_userids)
		fprintf(pgpout, "%c %02x  %02x    %s\n",
			' ' + (kc_new != kc_orig),
		 kc_orig, kc_new, sig->sig_from->pk_userids->uid_userid);
#endif
	    sig = sig->sig_next;
	    mask = KC_SIGTRUST_MASK | KC_CONTIG;
	    break;
	default:
	    mask = 0;
	}
	if ((kc_new & mask) != (kc_orig & mask)) {
	    if (!check_only)
		write_trust_pos(f, kc_new, trust_pos);
	    ++changed;
	}
    }
    fclose(f);
    if (status < -1)		/* -1 is OK, EOF */
	return status;
    if (pk || sig || id) {
	fprintf(pgpout, "maint_final: internal error\n");
	return -1;
    }
    return changed;
}				/* maint_final */

int maint_list(char *ringfile)
{
    int status;
    FILE *f;
    char userid[256];
    byte keyID[KEYFRAGSIZE];
    byte sigkeyID[KEYFRAGSIZE];
    char *signator;
    char tchar = 0;
    byte ctb, kc;
    int owntrust = 0;
    int usercount = 0;

    if ((f = fopen(ringfile, FOPRBIN)) == NULL) {
	fprintf(pgpout,
		LANG("\n\007Can't open key ring file '%s'\n"), ringfile);
	return -1;
    }
    init_trust_lst();
    setkrent(ringfile);
    init_userhash();

    fprintf(pgpout, LANG("  KeyID    Trust     Validity  User ID\n"));
    while ((status = readkpacket(f, &ctb, userid, keyID, sigkeyID)) != -1) {
	if (status == -3 || status == -2)
	    break;
	if (status < 0)
	    continue;

	if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE) ||
	    is_ctb_type(ctb, CTB_SKE_TYPE) || ctb == CTB_USERID) {
	    if (read_trust(f, &kc) < 0) {
		status = ERR_NOTRUST;
		/* compromise cert. don't have trust byte */
		if (!is_ctb_type(ctb, CTB_SKE_TYPE))
		    break;
	    }
	}
#ifdef MACTC5
	{
		extern int BreakCntl;
		if (!BreakCntl) BreakCntl = 35;
		mac_poll_for_break();
	}
#endif

	switch (ctb_type(ctb)) {
	case CTB_CERT_PUBKEY_TYPE:
	    tchar = (kc & KC_BUCKSTOP ? '*' : ' ');
	    owntrust = TRUST_LEV(kc);
	    usercount = 0;
	    userid[0] = '\0';
	    break;
	case CTB_USERID_TYPE:
	    if (!usercount) {	/* first userid */
		fprintf(pgpout, "%c %s ", tchar, keyIDstring(keyID));
		fprintf(pgpout, "%-*s ", trustlst_len, trust_lst[owntrust]);
	    } else
		fprintf(pgpout, "  %s %*s ", blankkeyID, trustlst_len, "");
	    fprintf(pgpout, "%-*s ", legitlst_len,
		    legit_lst[kc & KC_LEGIT_MASK]);
	    if (usercount)
		putc(' ', pgpout);
	    ++usercount;
	    fprintf(pgpout, "%s\n", LOCAL_CHARSET(userid));
	    break;
	case CTB_SKE_TYPE:
	    if (!usercount) {	/* sig before userid: compromise cert. */
		tchar = '#';
		break;
	    }
	    fprintf(pgpout, "%c %s ",
		    (kc & KC_CONTIG) ? 'c' : ' ', blankkeyID);
	    fprintf(pgpout, "%-*s ", trustlst_len, trust_lst[TRUST_LEV(kc)]);
	    fprintf(pgpout, "%*s  ", legitlst_len, "");
	    if ((signator = user_from_keyID(sigkeyID)) == NULL)
		fprintf(pgpout, LANG("(KeyID: %s)\n"), keyIDstring(sigkeyID));
	    else
		fprintf(pgpout, "%s\n", LOCAL_CHARSET(signator));
	    break;
	}
    }
    endkrent();
    fclose(f);
    if (status < -1)		/* -1 is OK, EOF */
	return status;
    return 0;
}				/* maint_list */

/*
 * translate the messages in the arrays trust_lst and legit_lst.
 * trustlst_len and legitlst_len will be set to the length of
 * the longest translated string.
 */
void init_trust_lst(void)
{
    static int initialized = 0;
    int i, len;
    char *s;

    if (initialized)
	return;
    for (i = 0; i < 8; ++i) {
	if (trust_lst[i][0]) {
	    s = LANG(trust_lst[i]);
	    if (s != trust_lst[i])
		strncpy(trust_lst[i], s, sizeof(trust_lst[0]) - 1);
	    len = strlen(s);
	    if (len > trustlst_len)
		trustlst_len = len;
	}
    }
    for (i = 0; i < 4; ++i) {
	s = LANG(legit_lst[i]);
	if (s != legit_lst[i])
	    strncpy(legit_lst[i], s, sizeof(legit_lst[0]) - 1);
	len = strlen(s);
	if (len > legitlst_len)
	    legitlst_len = len;
    }
    initialized = 1;
}				/* init_trust_lst */



/*
 * compare the key in file f at keypos with the matching key in the
 * secret keyring, the global variable sec_fp must contain the file pointer
 * for of the secret keyring.
 *
 * returns 1 if the key was not found, -2 if the keys were different
 * and 0 if the keys compared OK
 */
static int check_secretkey(FILE * f, long keypos, byte keyctrl)
{
    int status = -1;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    unit nsec[MAX_UNIT_PRECISION], esec[MAX_UNIT_PRECISION];
    char userid[256];
    byte keyID[KEYFRAGSIZE];
    long savepos, pktlen;
    byte ctb;

    if (sec_fp == NULL)
	return -1;

    savepos = ftell(f);
    fseek(f, keypos, SEEK_SET);
    if (readkeypacket(f, FALSE, &ctb, NULL, NULL, n, e,
		      NULL, NULL, NULL, NULL, NULL, NULL) < 0)
	goto ex;
    extract_keyID(keyID, n);

    do {			/* get userid */
	status = readkpacket(f, &ctb, userid, NULL, NULL);
	if (status == -1 || status == -3)
	    goto ex;
    } while (ctb != CTB_USERID);

    if (lookup_by_keyID(sec_fp, keyID) < 0) {
#if 0
	if (!check_only) {
	    fprintf(pgpout,
LANG("\nAn \"axiomatic\" key is one which does not need certifying by\n\
anyone else.  Usually this special status is reserved only for your\n\
own keys, which should also appear on your secret keyring.  The owner\n\
of an axiomatic key (who is typically yourself) is \"ultimately trusted\"\n\
by you to certify any or all other keys.\n"));
	    fprintf(pgpout, LANG("\nKey for user ID \"%s\"\n\
is designated as an \"ultimately-trusted\" introducer, but the key\n\
does not appear in the secret keyring.\n\
Use this key as an ultimately-trusted introducer (y/N)? "),
		    LOCAL_CHARSET(userid));
	    status = (getyesno('n') ? 0 : 1);
	}
#else
	status = 1;
#endif
    } else {
	long kpos = ftell(sec_fp);
	if (readkeypacket(sec_fp, FALSE, &ctb, NULL, NULL, nsec, esec,
			  NULL, NULL, NULL, NULL, NULL, NULL) < 0) {
	    fprintf(pgpout, LANG("\n\007Cannot read from secret keyring.\n"));
	    status = -3;
	    goto ex;
	}
	if (mp_compare(n, nsec) || mp_compare(e, esec)) {
	    /* Red Alert! */
	    fprintf(pgpout,
		    LANG("\n\007WARNING: Public key for user ID: \"%s\"\n\
does not match the corresponding key in the secret keyring.\n"),
		    LOCAL_CHARSET(userid));
	    fprintf(pgpout,
LANG("This is a serious condition, indicating possible keyring tampering.\n"));
	    status = -2;
	} else {
	    status = 0;
	}

	/* Okay, key is in secret key ring, and it matches. */
	if (!(keyctrl & KC_BUCKSTOP)) {
	    if (batchmode) {
		status = -1;
	    } else {
		fprintf(pgpout, LANG("\nKey for user ID \"%s\"\n\
also appears in the secret key ring."), LOCAL_CHARSET(userid));
		fputs(
	  LANG("\nUse this key as an ultimately-trusted introducer (y/N)? "),
		      pgpout);
		status = getyesno('n') ? 0 : -1;
	    }
	}
	if (status == 0 && floppy_fp) {
	    if (lookup_by_keyID(floppy_fp, keyID) < 0) {
		fprintf(pgpout, LANG("Public key for: \"%s\"\n\
is not present in the backup keyring '%s'.\n"),
			LOCAL_CHARSET(userid), floppyring);
	    } else {
		pktlen = ftell(sec_fp) - kpos;
		fseek(sec_fp, kpos, SEEK_SET);
		while (--pktlen >= 0 && getc(sec_fp) == getc(floppy_fp));
		if (pktlen != -1) {
		    fprintf(pgpout,
			    LANG("\n\007WARNING: Secret key for: \"%s\"\n\
does not match the key in the backup keyring '%s'.\n"),
			    LOCAL_CHARSET(userid), floppyring);
		    fprintf(pgpout,
LANG("This is a serious condition, indicating possible keyring tampering.\n"));
		    status = -2;
		}
	    }
	}
    }
  ex:
    fseek(f, savepos, SEEK_SET);
    return status;
}				/* check_secretkey */

/*
 * setup tables for trust scoring.
 */
static void setup_trust(void)
{
    /* initialize trust table */
    if (marg_min == 0) {	/* marginally trusted signatures are ignored */
	trust_tbl[5] = 0;
	trust_tbl[6] = 1;
	complete_min = compl_min;
    } else {
	if (marg_min < compl_min)
	    marg_min = compl_min;
	trust_tbl[5] = compl_min;
	trust_tbl[6] = marg_min;
	complete_min = compl_min * marg_min;
    }
    trust_tbl[7] = complete_min;	/* ultimate trust */
    marginal_min = complete_min / 2;
}				/* setup_trust */

/* Ask for a wetware decision from the human on how much to trust 
   this key's owner to certify other keys.  Returns trust value. */
int ask_owntrust(char *userid, byte cur_trust)
{
    char buf[8];

    if (check_only || filter_mode || batchmode) {
	/* not interactive */
	++undefined_trust;	/* We complete/undefined. Why?  */
	return KC_OWNERTRUST_UNDEFINED;
    }
#ifdef MACTC5
	get_trust(buf,userid);
#endif
    fprintf(pgpout,
LANG("\nMake a determination in your own mind whether this key actually\n\
belongs to the person whom you think it belongs to, based on available\n\
evidence.  If you think it does, then based on your estimate of\n\
that person's integrity and competence in key management, answer\n\
the following question:\n"));
    fprintf(pgpout, LANG("\nWould you trust \"%s\"\n\
to act as an introducer and certify other people's public keys to you?\n\
(1=I don't know. 2=No. 3=Usually. 4=Yes, always.) ? "),
	    LOCAL_CHARSET(userid));
    fflush(pgpout);
#ifdef MACTC5
	Putchar(buf[0]);
	Putchar('\n');
#else
    getstring(buf, sizeof(buf) - 1, TRUE);
#endif
    switch (buf[0]) {
    case '1':
	return KC_OWNERTRUST_UNKNOWN;
    case '2':
	return KC_OWNERTRUST_NEVER;
    case '3':
	return KC_OWNERTRUST_USUALLY;
    case '4':
	return KC_OWNERTRUST_ALWAYS;
    default:
	return TRUST_LEV(cur_trust);
    }
}				/* ask_owntrust */

/*
 * scan keyfile f for keyID srch_keyID.
 * returns the file position of the key if it is found, and sets the
 * file pointer to the start of the key packet.
 * returns -1 if the key was not found or < -1 if there was an error
 */
long lookup_by_keyID(FILE * f, byte * srch_keyID)
{
    int status;
    long keypos = 0;
    byte keyID[KEYFRAGSIZE];
    byte ctb;

    rewind(f);
    while ((status = readkpacket(f, &ctb, NULL, keyID, NULL)) != -1) {
	if (status == -3 || status == -2)
	    break;
	if (status < 0)
	    continue;
	if (is_key_ctb(ctb) && memcmp(keyID, srch_keyID, KEYFRAGSIZE) == 0) {
	    fseek(f, keypos, SEEK_SET);
	    return keypos;
	}
	keypos = ftell(f);
    }
    return status;
}				/* lookup_by_keyID */

/*
 * look up the key matching "keyID" and print the first userID
 * of this key.  File position of f is saved.
 */
void show_userid(FILE * f, byte * keyID)
{
    int status;
    long filepos;
    char userid[256];
    byte ctb;

    filepos = ftell(f);
    if (lookup_by_keyID(f, keyID) >= 0)
	while ((status = readkpacket(f, &ctb, userid, NULL, NULL)) != -1
	       && status != -3)
	    if (ctb == CTB_USERID) {
		fprintf(pgpout, "%s\n", LOCAL_CHARSET(userid));
		fseek(f, filepos, SEEK_SET);
		return;
	    }
    fprintf(pgpout, LANG("(KeyID: %s)\n"), keyIDstring(keyID));
    fseek(f, filepos, SEEK_SET);
}				/* show_userid */

/*
 * messages printed by show_key()
 */
static char *owntrust_msg[] =
{
    "",				/* Just don't say anything in this case */
    "",
    _LANG("This user is untrusted to certify other keys.\n"),
    "",				/* reserved */
    "",				/* reserved */
    _LANG("This user is generally trusted to certify other keys.\n"),
    _LANG("This user is completely trusted to certify other keys.\n"),
    _LANG("This axiomatic key is ultimately trusted to certify other keys.\n"),
};
static char *keylegit_msg[] =
{
    _LANG("This key/userID association is not certified.\n"),
    _LANG("This key/userID association is not certified.\n"),
    _LANG("This key/userID association is marginally certified.\n"),
    _LANG("This key/userID association is fully certified.\n"),
};
static char *sigtrust_msg[] =
{
    _LANG("  Questionable certification from:\n  "),
    _LANG("  Questionable certification from:\n  "),
    _LANG("  Untrusted certification from:\n  "),
    "",				/* reserved */
    "",				/* reserved */
    _LANG("  Generally trusted certification from:\n  "),
    _LANG("  Completely trusted certification from:\n  "),
    _LANG("  Axiomatically trusted certification from:\n  "),
};

/*
 * show the key in file f at file position keypos.
 * 'what' controls the info that will be shown:
 *   SHOW_TRUST: show trust byte info
 *   SHOW_SIGS:  show signatures
 *   SHOW_HASH:  show key fingerprint
 * these constants can be or'ed
 *
 * 'what' can also be SHOW_LISTFMT to get the same format as for pgp -kv
 * no signatures or extra userids will be printed in this case.
 *
 * 'what' can be SHOW_CHANGE, in which case it will take the keyID and
 * call show_update();
 */
int show_key(FILE * f, long keypos, int what)
{
    int status, keystatus = -1;
    long filepos;
    char userid[256];
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
    byte sigkeyID[KEYFRAGSIZE];
    word32 timestamp;
    byte ctb, keyctb = 0, keyctrl;
    int userids = 0;
    int keyids = 0;
    byte savekeyID[KEYFRAGSIZE];
    boolean print_trust = FALSE;
    byte hash[16];
    int precision = global_precision;
    int compromised = 0;
    int disabled = 0;

    filepos = ftell(f);
    fseek(f, keypos, SEEK_SET);
    while ((status = readkeypacket(f, FALSE, &ctb, (byte *) & timestamp,
				   userid,
	      n, e, NULL, NULL, NULL, NULL, sigkeyID, &keyctrl)) != -1) {
	if (status == -2 || status == -3)
	    break;
	if (is_key_ctb(ctb)) {

	    if (keyids)
		break;
	    extract_keyID(savekeyID, n);
	    keyids++;
	    if (what & SHOW_HASH)
		getKeyHash(hash, n, e);
	    keyctb = ctb;
	    keystatus = status;	/* remember status, could be version error */

	} else if (ctb == CTB_KEYCTRL) {

	    /* trust bytes only in public keyrings */
	    if (keystatus >= 0 && !userids)	/* key packet trust byte */
		if (keyctrl & KC_DISABLED)
		    disabled = 1;
	    if (what & SHOW_TRUST)
		print_trust = TRUE;

	} else if (ctb == CTB_USERID) {

	    if (userids == 0) {
		PascalToC(userid);	/* for display */
		++userids;
		if (what & SHOW_CHANGE) {
		    show_update(key2IDstring(n));
		    break;
		}
		if (what & SHOW_LISTFMT) {
		    if (is_ctb_type(keyctb, CTB_CERT_PUBKEY_TYPE))
			fprintf(pgpout, LANG("pub"));
		    else if (is_ctb_type(keyctb, CTB_CERT_SECKEY_TYPE))
			fprintf(pgpout, LANG("sec"));
		    else
			fprintf(pgpout, "???");
		    if (keystatus < 0)
			fprintf(pgpout, "? ");
		    else if (compromised)
			fprintf(pgpout, "# ");
		    else if (disabled)
			fprintf(pgpout, "- ");
		    else
			fprintf(pgpout, "  ");
		    fprintf(pgpout, "%4d/%s %s  ",
		       countbits(n), key2IDstring(n), cdate(&timestamp));
		    fprintf(pgpout, "%s\n", LOCAL_CHARSET(userid));
		    break;	/* only print default userid */
		}
		fprintf(pgpout, LANG("\nKey for user ID: %s\n"),
			LOCAL_CHARSET(userid));
		fprintf(pgpout, LANG("%d-bit key, key ID %s, created %s\n"),
			countbits(n), key2IDstring(n), cdate(&timestamp));
		if (keystatus == -4)
		    fprintf(pgpout, LANG("Bad key format.\n"));
		else if (keystatus == -6)
		    fprintf(pgpout, LANG("Unrecognized version.\n"));
		else if (what & SHOW_HASH)
		    printKeyHash(hash, FALSE);
		if (compromised)
		    fprintf(pgpout, LANG("Key has been revoked.\n"));
		if (disabled)
		    fprintf(pgpout, LANG("Key is disabled.\n"));
		if (print_trust && *owntrust_msg[TRUST_LEV(keyctrl)] != '\0')
		    fprintf(pgpout, LANG(owntrust_msg[TRUST_LEV(keyctrl)]));
	    } else {
		PascalToC(userid);
		if (what != 0)
		    fprintf(pgpout, "\n");
		fprintf(pgpout, LANG("Also known as: %s\n"),
			LOCAL_CHARSET(userid));
	    }
	    if (print_trust) {
		read_trust(f, &keyctrl);
		fprintf(pgpout, LANG(keylegit_msg[keyctrl & KC_LEGIT_MASK]));
	    }			/* print_trust */
	} else if (is_ctb_type(ctb, CTB_SKE_TYPE)) {

	    if (userids == 0)
		compromised = 1;
	    if (what & SHOW_CHANGE) {
		show_update(key2IDstring(n));
		break;
	    }
	    if (what & SHOW_SIGS) {
		if (print_trust) {
		    read_trust(f, &keyctrl);
		    fprintf(pgpout, LANG(sigtrust_msg[TRUST_LEV(keyctrl)]));
		} else {
		    fprintf(pgpout, LANG("  Certified by: "));
		}
		show_userid(f, sigkeyID);
	    }
	}
    }
    if (status == -1 && userids)
	status = 0;
    if (!userids && !compromised && (what != SHOW_CHANGE)) {
    	status = -1;
	fprintf(pgpout, LANG("\nWarning: keyid %4d/%s %s  has no user id!\n"),
		countbits(n), keyIDstring(savekeyID), cdate(&timestamp));
    }
    set_precision(precision);
    fseek(f, filepos, SEEK_SET);
    return status;
}				/* show_key */

/* show_update -- this function just prints an update message to
 * pgpout to inform the user that an update happened.
 */
void show_update(char *s)
{
    fprintf(pgpout, LANG("Updated keyID: 0x%s\n"), s);
}

/*
 * stripped down version of readkeypacket(), the output userid
 * is a null terminated string.
 */
int readkpacket(FILE * f, byte * ctb, char *userid,
		byte * keyID, byte * sigkeyID)
{
    int status;
    unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];

    status = readkeypacket(f, FALSE, ctb, NULL, userid, n, e,
			   NULL, NULL, NULL, NULL, sigkeyID, NULL);

    if (status < 0) {
#ifdef DEBUG
	if (status < -1)
	    fprintf(stderr, "readkeypacket returned %d\n", status);
#endif
	return status;
    }
    if (keyID && is_key_ctb(*ctb))
	extract_keyID(keyID, n);

    if (userid && *ctb == CTB_USERID)
	PascalToC(userid);

    return 0;
}				/* readkpacket */

/*
 * write trust byte "keyctrl" to file f at file position "pos"
 */
void write_trust_pos(FILE * f, byte keyctrl, long pos)
{
    long fpos;

    fpos = ftell(f);
    fseek(f, pos, SEEK_SET);
    write_trust(f, keyctrl);
    fseek(f, fpos, SEEK_SET);
}				/* write_trust_pos */

/*
 * read a trust byte packet from file f, the trust byte will be
 * stored in "keyctrl".
 * returns -1 on EOF, -3 on corrupt input, and ERR_NOTRUST if
 * the packet was not a trust byte (this can be used to check if
 * a file is a keyring (with trust bytes) or a keyfile).
 * The current file position is left unchanged in this case.
 */
int read_trust(FILE * f, byte * keyctrl)
{
    unsigned char buf[3];

    if (fread(buf, 1, 3, f) != 3)
	return -1;
    if (buf[0] != CTB_KEYCTRL) {
	if (is_ctb(buf[0])) {
	    fseek(f, -3L, SEEK_CUR);
	    return ERR_NOTRUST;
	} else
	    return -3;		/* bad data */
    }
    if (buf[1] != 1)		/* length must be 1 */
	return -3;
    if (keyctrl)
	*keyctrl = buf[2];
    return 0;
}				/* read_trust */



/****** userid lookup ******/

#define	HASH_ALLOC	(ALLOC_UNIT / sizeof(struct hashent))

static VOID *allocbuf(int size);
static void freebufpool();

static struct hashent {
    struct hashent *next;
    byte keyID[KEYFRAGSIZE];
    char *userid;
} **hashtbl = NULL, *hashptr;

static char *strptr;
static int strleft = 0;
static int hashleft = 0;
static int nleft = 0;

#define MAXKR	8	/* max. number of keyrings for user_from_keyID() */
static char *krnames[MAXKR];
static int nkr = 0;

/*
 * Lookup userid by keyID without using the in-memory hash table.
 */
static char *
 _user_from_keyID(byte * srch_keyID)
{
    FILE *f;
    int i, status, found = 0;
    byte keyID[KEYFRAGSIZE];
    static char userid[256];
    byte ctb;

    /* search all keyfiles set with setkrent() */
    for (i = 0; !found && i < nkr; ++i) {
	if ((f = fopen(krnames[i], FOPRBIN)) == NULL)
	    continue;
	while ((status = readkpacket(f, &ctb, userid, keyID, NULL)) != -1) {
	    if (status == -2 || status == -3)
		break;
	    if (is_key_ctb(ctb) && memcmp(keyID, srch_keyID, KEYFRAGSIZE) == 0)
		found = 1;
	    if (found && ctb == CTB_USERID)
		break;
	}
	fclose(f);
    }
    return found ? userid : NULL;
}				/* _user_from_keyID */

/*
 * Lookup userid by keyID, use hash table if initialized.
 */
char *
 user_from_keyID(byte * keyID)
{
    struct hashent *p;

    if (!hashtbl)
	return _user_from_keyID(keyID);
    for (p = hashtbl[PK_HASH(keyID)]; p; p = p->next)
	if (memcmp(keyID, p->keyID, KEYFRAGSIZE) == 0)
	    return p->userid;
    return NULL;
}				/* user_from_keyID */

/*
 * add keyfile to userid hash table, userids are added, endkrent() clears
 * the hash table.
 */
int setkrent(char *keyring)
{
    int i;

    assert(nkr < MAXKR);
    if (keyring == NULL)
	keyring = globalPubringName;
    for (i = 0; i < nkr; ++i)
	if (strcmp(keyring, krnames[i]) == 0)
	    return 0;		/* duplicate name */
    krnames[nkr++] = store_str(keyring);
    return 0;
}				/* setkrent */

void endkrent(void)
{
    hashleft = strleft = 0;
    hashtbl = NULL;
    nkr = 0;
    freebufpool();
}				/* endkrent */

/*
 * create userid hash table, read all files set with setkrent()
 */
int init_userhash(void)
{
    FILE *f;
    int status, i;
    byte keyID[KEYFRAGSIZE];
    char userid[256];
    byte ctb;
    int keyflag;

    if (!hashtbl) {
	hashtbl = allocbuf(PK_HASHSIZE * sizeof(struct hashent *));
	memset(hashtbl, 0, PK_HASHSIZE * sizeof(struct hashent *));
    }
    for (i = 0; i < nkr; ++i) {
	if ((f = fopen(krnames[i], FOPRBIN)) == NULL)
	    continue;
	keyflag = 0;
	while ((status = readkpacket(f, &ctb, userid, keyID, NULL)) != -1) {
	    if (is_key_ctb(ctb) && user_from_keyID(keyID) == NULL)
		keyflag = 1;
	    if (keyflag && ctb == CTB_USERID) {
		if (!hashleft) {
		    hashptr = allocbuf(HASH_ALLOC * sizeof(struct hashent));
		    hashleft = HASH_ALLOC;
		}
		memcpy(hashptr->keyID, keyID, KEYFRAGSIZE);
		hashptr->userid = store_str(userid);
		hashptr->next = hashtbl[PK_HASH(keyID)];
		hashtbl[PK_HASH(keyID)] = hashptr;
		++hashptr;
		--hashleft;
		keyflag = 0;
	    }
	}
	fclose(f);
    }
    return 0;
}				/* init_userhash */

/*
 * memory management routines
 */

static void maint_init_mem(void)
{
    pkhash = allocbuf(PK_HASHSIZE * sizeof(struct pubkey *));
    memset(pkhash, 0, PK_HASHSIZE * sizeof(struct pubkey *));
}

static void maint_release_mem(void)
{
    nleft = 0;
    strleft = 0;
    pkhash = NULL;
    freebufpool();
}

/*
 * allocn() does the same as malloc().  Memory is allocated in chunks
 * of ALLOC_UNIT bytes, all memory can be freed by calling freebufpool().
 */
static VOID *
 allocn(int size)
{
    static char *ptr;
#ifndef MSDOS			/* don't align on MSDOS to save memory */
    size = (size + 3) & ~3;
#endif
    assert(size < ALLOC_UNIT);
    if (size > nleft) {
	ptr = allocbuf(ALLOC_UNIT);
	nleft = ALLOC_UNIT;
    }
    nleft -= size;
    ptr += size;
    return ptr - size;
}				/* allocn */

/*
 * store_str does the same as strdup(), but allocates memory with allocbuf()
 */
char *
 store_str(char *str)
{
    int size = strlen(str) + 1;
    if (size > ALLOC_UNIT) {
	fprintf(stderr, "store_str: string too long\n");
	return NULL;
    }
    if (size > strleft) {
	strptr = allocbuf(ALLOC_UNIT);
	strleft = ALLOC_UNIT;
    }
    strcpy(strptr, str);
    strptr += size;
    strleft -= size;
    return strptr - size;
}				/* store_str */


static struct bufpool {
    struct bufpool *next;
    char buf[1];		/* variable size */
} *bufpool = NULL;

long totalsize = 0;

/*
 * allocate buffer, all buffers allocated with this function can be
 * freed with one call to freebufpool()
 */
static VOID *
 allocbuf(int size)
{
    struct bufpool *p;

    p = xmalloc(size + sizeof(struct bufpool *));
    totalsize += size;
    p->next = bufpool;
    bufpool = p;
    return p->buf;
}				/* allocbuf */

/*
 * free all memory obtained with allocbuf()
 */
static void freebufpool(void)
{
    struct bufpool *p;

    if (verbose)
	fprintf(pgpout, "\nMemory used: %ldk\n", totalsize / 1024);
    totalsize = 0;
    while (bufpool) {
	p = bufpool;
	bufpool = bufpool->next;
	free(p);
    }
    nleft = strleft = hashleft = 0;
}				/* freebufpool */

#ifdef MACTC5
void ReInitKeyMaint(void);

void ReInitKeyMaint(void) {
	trustlst_len = 9;
	legitlst_len = 9;
	strcpy(floppyring,"");
	check_only = FALSE;
	floppy_fp = NULL;
	pkhash = NULL;
	pklist = NULL;
	hashtbl = NULL;
	totalsize = 0;
}
#endif
