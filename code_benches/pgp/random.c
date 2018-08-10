/*
 * True and cryptographic random number generation.
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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>	/* For SIGINT */
#include <time.h>
#include <unistd.h>

#ifdef AMIGA      /* Includes for timer -- RKNOP */
#include <devices/timer.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <proto/exec.h>
#endif /* AMIGA */

#ifdef __PUREC__
#include <ext.h>
#endif

#include "system.h"	/* For ttycbreak, getch, etc. */
#include "idea.h"
#include "md5.h"
#include "noise.h"
#include "language.h"
#include "random.h"
#include "fileio.h"	/* For FOPRBIN */
#include "pgp.h"	/* For globalRandseedName */
#include "randpool.h"
#ifdef MACTC5
#include "Macutil2.h"
#include "Macutil3.h"
#include "TimeManager.h"
#include <unix.h>
#endif

/*
 * As of PGP 2.6.2, the randseed.bin file has been expanded.  An explanation
 * of how the whole thing works in in order, as people are always suspiscious
 * of the random number generator.  (After the xorbytes bug in 2.6, perhaps
 * you should be.)  There are two random number generators in PGP.  One
 * is the "cryptRand" family, which is based on X9.17, but uses IDEA instead
 * of 2-key EDE triple-DES.  This is the generator with a lot of peer review.
 * The implementation is in idea.c.
 * The second is the "trueRand" family, which attempt to accurately measure
 * the entropy available from keyboard I/O.  It keeps a lot more state.
 * The implementation of this is in randpool.c.
 * Originally, the trueRand generator was only used for generating primes,
 * and the cryptRand for generating IDEA session keys.  But things have
 * become a bit more complex.  In particular, the X9.17 specification
 * wants a source of high-resolution time-of-day information, as a source
 * of some "true" randomness to throw in.  So we use the trueRand pool
 * for that.
 * The cryptRand functions keep a state file around, usually named
 * randseed.bin, for a seed, as the X9.17 generator requires 24 bytes of
 * known initial information.
 * This data in this file is carefully "washed" before and after use to
 * help ensure that if the file is captured or altered, the keys will
 * not be too vulnerable.  A washing consists of an encryption in PGP's
 * usual CFB mode of the material coming from or being written to the
 * randseed.bin file on disk.  Assuming the cipher is strong, the effects
 * of the wash are as difficult to predict as the key that is used is
 * difficult to guess.
 * Beforehand, we use the MD5 of the file being encrypted as an additional
 * source of randomness (on the theory that an attacker trying to break
 * a session key probably doesn't have the plaintext, or he wouldn't need
 * to bother), and use that as an IDEA key (with a fixed IV of zero)
 * to encrypt the randseed.bin data.
 * After generating an IDEA key and IV, some more random bytes are generated
 * to reinitialize randseed.bin, and these are encrypted in the same manner
 * as the PGP message before being written to disk, on the assumption that
 * if an attacker can decrypt that, they can decrypt the message directly
 * and not bother attacking the randseed.bin file.
 * The previous code only saved the 24 bytes needed by the X9.17 algorithm.
 * But in 2.6.2, we decided to make the randseed.bin file substantially
 * larger to hold more information that a would-be attacker must guess.
 * There are two reasons for this:
 * - Every time you run PGP, especially when responding to one of PGP's
 *   prompts, PGP samples the keystrokes for use as random numbers.
 *   It is a shame to throw this entropy (randomness) away just because
 *   there is no need for it in the current invocation of PGP.
 * - A feature was added to 2.6.2 to generate files full of random bytes
 *   for other programs to use as key material.  In this case, we haven't
 *   got a message we're encrypting to take some entropy from, and we may
 *   be asked to generate more than 24 random bytes, so there should be
 *   more than 24 bytes of seed material to work from.
 * The implementation is added on to the previous one, to offer assurance
 * that it is no weaker.
 * When the cryptRand generator is opened, the file is washed (if possible)
 * and the first 24 bytes are fed to the cryptographic RNG, while the
 * remainder is added to the trueRand random number pool.
 * When saving, the randseed.bin file is refilled with newly generated
 * bytes, again washed if possible.  It turns out (if you study the
 * X9.17 RNG) that each of the 2^64 possible timestamp information
 * values used in generating each 8 bytes of output generates a output
 * value, so the entropy in the trueRand pool is put to good use; this
 * is not just generating more data from 24 bytes of seed.
 * The random pool is opened and saved with a washing key when
 * generating a session key (see make_random_ideakey in crypto.c),
 * but it is also opened (harmless if alreasy open) and saved
 * (harmless if already saved) without a washing key in the exitPGP routine,
 * to mix in any entropy collected in this invocation of PGP even if
 * a session key was not generated.
 */

/*
 * The new randseed size, big enough to hold the full context of the cryptRand
 * and trueRand generators.  With the current RANDPOOLBITS of 3072 (384 bytes),
 * that's 408 bytes.  It's useless to make it any larger, although if
 * RANDPOOLBITS is increased, it might be an idea to keep this smaller than
 * one disk block on all systems (512 bytes is a good figure to use)
 * so we don't change the space requirements for randseed.bin.
 */
#define RANDSEED_BYTES	(RANDPOOLBITS/8 + 24)
/* Have we read in the randseed.bin file? */
static boolean randSeedOpen = 0;
static struct IdeaRandContext randContext;

/*
 * Load the RNG state from the randseed.bin file on disk.
 * Returns 0 on success, <0 on error.
 *
 * If cfb is non-zero, prewashes the data by encrypting with it.
 */
int
cryptRandOpen(struct IdeaCfbContext *cfb)
{
	byte buf[256];
	int len;
	FILE *f;

	if (randSeedOpen)
		return 0;	/* Already open */

	f = fopen(globalRandseedName, FOPRBIN);
	if (!f)
		return -1;

	/* First get the bare minimum 24 bytes we need for the IDEA RNG */
	len = fread((char *)buf, 1, 24, f);
	if (cfb)
		ideaCfbEncrypt(cfb, buf, buf, 24);
	ideaRandInit(&randContext, buf, buf+16);
	randSeedOpen = TRUE;
	if (len != 24) { /* Error */
		fclose(f);
		return -1;
	}

	/* Read any extra into the random pool */
	for (;;) {
		len = fread((char *)buf, 1, sizeof(buf), f);
		if (len <= 0)
			break;
		if (cfb)
			ideaCfbEncrypt(cfb, buf, buf, len);
		randPoolAddBytes(buf, len);
	}

	fclose(f);
	burn(buf);
#ifdef MACTC5
	PGPSetFinfo(globalRandseedName,'RSed','MPGP');
#endif
	return 0;
}

/* Create a new state from the output of trueRandByte */
void
cryptRandInit(struct IdeaCfbContext *cfb)
{
	byte buf[24];
	int i;

	for (i = 0; i < sizeof(buf); i++)
		buf[i] = trueRandByte();
	if (cfb)
		ideaCfbEncrypt(cfb, buf, buf, sizeof(buf));

	ideaRandInit(&randContext, buf, buf+16);
	randSeedOpen = TRUE;
	burn(buf);
}

byte
cryptRandByte(void)
{
	if (!randSeedOpen)
		cryptRandOpen((struct IdeaCfbContext *)0);
	return ideaRandByte(&randContext);
}

/*
 * Write out a file of random bytes.  If cfb is defined, wash it with the
 * cipher.
 */
int
cryptRandWriteFile(char const *name, struct IdeaCfbContext *cfb, unsigned bytes)
{
	byte buf[256];
	FILE *f;
	int i, len;

	f = fopen(name, FOPWBIN);
	if (!f)
		return -1;

	while (bytes) {
		len = (bytes < sizeof(buf)) ? bytes : sizeof(buf);
		for (i = 0; i < len; i++)
			buf[i] = ideaRandByte(&randContext);
		if (cfb)
			ideaCfbEncrypt(cfb, buf, buf, len);
		i = fwrite(buf, 1, len, f);
		if (i < len)
			break;
		bytes -= len;
	}

#ifdef MACTC5
	PGPSetFinfo((char *)name,'RSed','MPGP');
#endif

	return (fclose(f) != 0 || bytes != 0) ? -1 : 0;
}

/*
 * Create a new RNG state, encrypt it with the supplied key, and save it
 * out to disk.
 *
 * When we encrypt a file, the saved data is "postwashed" using the
 * same key and initial vector (including the repeated check bytes and
 * everything) that is used to encrypt the user's message.
 * The hope is that this "postwash" renders it is at least as hard to
 * derive old session keys from randseed.bin as it is to crack the the
 * message directly.
 *
 * The purpose of using EXACTLY the same encryption is to make sure that
 * there isn't related, but different data floating around that can be
 * used for cryptanalysis.
 *
 * This function is always called by exitPGP, without a washing encryption,
 * so this function ignores that call if it has previously been called
 * to save washed bytes.
 */
#ifdef MACTC5
	boolean savedwashed = FALSE;
#endif

void
cryptRandSave(struct IdeaCfbContext *cfb)
{
#ifndef MACTC5
	static boolean savedwashed = FALSE;
#endif

	if (!randSeedOpen)
		return;	/* Do nothing */

	if (cfb)
		savedwashed = TRUE;
	else if (savedwashed)
		return;	/* Don't re-save if it's already been saved washed. */

	(void)cryptRandWriteFile(globalRandseedName, cfb, RANDSEED_BYTES);
#ifdef MACTC5
	PGPSetFinfo(globalRandseedName,'RSed','MPGP');
#endif
}

/*
 * True random bit handling
 */

/*
 * Because these truly random bytes are so unwieldy to accumulate,
 * they can be regarded as a precious resource.  Unfortunately,
 * cryptographic key generation algorithms may require a great many
 * random bytes while searching about for large random prime numbers.
 * Fortunately, they need not all be truly random.  We only need as
 * many truly random bits as there are bits in the large prime we
 * are searching for.  These random bytes can be recycled and modified
 * via pseudorandom numbers until the key is generated, without losing
 * any of the integrity of randomness of the final key.
 *
 * The technique used is a pool of random numbers, which bytes are
 * taken from successively and, when the end is reached, the pool
 * is stirred using an irreversible hash function.  Some (64 bytes)
 * of the pool is not returned to ensure the sequence is not predictible
 * from the values retriefed from trueRandByte().  To be precise,
 * MD5Transform is used as a block cipher in CBC mode, and then the
 * "key" (i.e. what is usually the material to be hashed) is overwritten
 * with some of the just-generated random bytes.
 *
 * This is implemented in randpool.c; see that file for details.
 *
 * An estimate of the number of bits of true (Shannon) entropy in the
 * pool is kept in trueRandBits.  This is incremented when timed
 * keystrokes are available, and decremented when bits are explicitly
 * consumed for some purpose (such as prime generation) or another.
 *
 * trueRandFlush is called to obliterate traces of old random bits after
 * prime generation is completed.  (Primes are the most carefully-guarded
 * values in PGP.)
 */

static unsigned trueRandBits = 0;	/* Bits of entropy in pool */

#ifdef MACTC5
#define CBITS 5
#define TRByt 3
#define TREvt 1

static void perturb(rbits)
int rbits;
{	
	spinner();
	randPoolAddBytes((byte *) seedBuffer, 8);
	trueRandBits +=rbits;
	if (trueRandBits > RANDPOOLBITS)
		trueRandBits = RANDPOOLBITS;
	return;
}
#endif

/* trueRandPending is bits to add to next accumulation request */
static unsigned trueRandPending = 0;

/*
 * Destroys already-used random numbers.  Ensures no sensitive data
 * remains in memory that can be recovered later.  This is called
 * after RSA key generation, so speed is not critical, but security is.
 * RSA key generation takes long enough that interrupts and other
 * tasks are likely to have used a measurable and difficult-to-predict
 * amount of real time, so there is some virtue in sampling the clocks
 * with noise().
 */
void
trueRandFlush(void)
{
	noise();
	randPoolStir();	/* Destroy evidence of what primes were generated */
	randPoolStir();
	randPoolStir();
	randPoolStir();	/* Make *really* certain */
}

/*
 * "Consumes" count bits worth of entropy from the true random pool for some
 * purpose, such as prime generation.
 *
 * Note that something like prime generation can end up calling trueRandByte
 * more often than is implied by the count passed to trueRandClaim; this
 * may happen if the random bit consumer is not perfectly efficient in its
 * use of random bits.  For example, if a search for a suitable prime fails,
 * the easiest thing to do is to get another load of random bits and try
 * again.  It is perfectly acceptable if these bits are correlated with the
 * bits used in the failed attempt, since they are discarded.
 */
void
trueRandConsume(unsigned count)
{
#ifdef MACTC5
	if( trueRandBits >= count )
		trueRandBits -= count;
	else
		trueRandBits = 0;
#else
	assert (trueRandBits >= count);
	trueRandBits -= count;
#endif
}

/*
 * Returns a truly random byte if any are available.  It degenerates to
 * a pseudorandom value if there are none.  It is not an error to call
 * this if none are available.  For example, it is called when generating
 * session keys to add to other sources of cryptographic random numbers.
 *
 * This forces an accumulation if any extra random bytes are pending.
 */
int
trueRandByte(void)
{
	if (trueRandPending)
		trueRandAccum(0);
#ifdef MACTC5
	while( trueRandBits < 8 ) {
		perturb(TRByt);
		spinner();
		}
#endif

	return randPoolGetByte();
}

/*
 * Given an event (typically a keystroke) coded by "event"
 * at a random time, add all randomness to the random pool,
 * compute a (conservative) estimate of the amount, add it
 * to the pool, and return the amount of randomness.
 * (The return value is just for informational purposes.)
 *
 * Double events are okay, but three in a row is considered
 * suspiscous and the randomness is counted as 0.
 */
unsigned
trueRandEvent(int event)
{
	static int event1 = 0, event2 = 0;
	word32 delta;
	unsigned cbits;

	delta = noise();
	randPoolAddBytes((byte *)&event, sizeof(event));

#ifdef MACTC5
	perturb(TRByt);
#endif

	if (event == event1 && event == event2) {
		cbits = 0;
	} else {
		event2 = event1;
		event1 = event;

		for (cbits = 0; delta; cbits++)
			delta >>= 1;

		/* Excessive paranoia? */
#ifdef MACTC5
		if (cbits > CBITS)
			cbits = CBITS;
#else
		if (cbits > 8)
			cbits = 8;
#endif
	}

	trueRandBits += cbits;
	if (trueRandBits > RANDPOOLBITS)
		trueRandBits = RANDPOOLBITS;

	return cbits;
}


/*
 * Since getting random bits from the keyboard requires user attention,
 * we buffer up requests for them until we can do one big request.
 */
void
trueRandAccumLater(unsigned bitcount)
{
	trueRandPending += bitcount;	/* Wow, that was easy! :-) */
#ifdef MACTC5
	spinner();
#endif
}

static void flush_input(void);

#ifdef AMIGA  /* Globals used for timing here and in noise.c - RKNOP 940613 */
struct Library *TimerBase=NULL;
struct timerequest *TimerIO=NULL;
union { struct timeval t;
        struct EClockVal e;
      } time0,time1;
unsigned short use_eclock=0;
#endif /* AMIGA */

/*
 * Performs an accumulation of random bits.  As long as there are fewer bits
 * in the buffer than are needed (the number passed, plus pending bits),
 * prompt for more.
 */
void
trueRandAccum(unsigned count)	/* Get this many random bits ready */
{
	int c;
#if defined(MSDOS) || defined(__MSDOS__)
	time_t timer;
#endif

	count += trueRandPending;	/* Do deferred accumulation now */
	trueRandPending = 0;

	if (count > RANDPOOLBITS)
		count = RANDPOOLBITS;

	if (trueRandBits >= count)
		return;

	fprintf(stderr,
LANG("\nWe need to generate %u random bits.  This is done by measuring the\
\ntime intervals between your keystrokes.  Please enter some random text\
\non your keyboard until you hear the beep:\n"), count-trueRandBits);

	ttycbreak();

#ifdef AMIGA  /* RKNOP 940613 */
        TimerIO=(struct timerequest *)AllocMem(sizeof(struct timerequest),
                                               MEMF_PUBLIC|MEMF_CLEAR);
        if (TimerIO)
        {  if (OpenDevice(TIMERNAME,UNIT_MICROHZ,(struct IORequest *)TimerIO,0))
              TimerBase=NULL;
           else
           {  TimerBase=(struct Library *)TimerIO->tr_node.io_Device;
              if (TimerBase->lib_Version>=36) /* Use E-clock instead */
              {  use_eclock=1;
                 CloseDevice((struct IORequest *)TimerIO);
                 if (!OpenDevice(TIMERNAME,UNIT_ECLOCK,
                                 (struct IORequest *)TimerIO,0))
                    TimerBase=(struct Library *)TimerIO->tr_node.io_Device;
                 else
                    TimerBase=NULL;
              }
              else use_eclock=0;
           }
        }
        else TimerBase=NULL;
#endif /* AMIGA */

	do {
		/* display counter to show progress */
		fprintf(stderr,"\r%4d ", count-trueRandBits);
		fflush(stderr);	/* assure screen update */

		flush_input();	/* If there's no delay, we can't use it */
#ifdef MACTC5
		StartTMCounter();
#endif
		c = getch();	/* always wait for input */
#ifdef MSDOS
		if (c == 3)
			breakHandler(SIGINT);
		if (c == 0)
			c = getch() + 256;
#endif
		/* Print flag indicating acceptance (or not) */
                /* putc a macro, not safe to have function as an arg!! */
                fputc(trueRandEvent(c) ? '.' : '?' , stderr);
#ifdef MACTC5
		StopTMCounter();
#endif
	} while (trueRandBits < count);

	fputs("\r   0 *", stderr);
	fputs(LANG("\007 -Enough, thank you.\n"), stderr);

#if defined(MSDOS) || defined(__MSDOS__)
	/* Wait until one full second has passed without keyboard input */
	do {
		flush_input();
		sleep(1);
	} while (kbhit());
#else
#ifdef AMIGA       /* Added RKNOP 940608 */
        Delay(50*1);    /* dos.library function, wait 1 second */
        if (TimerBase) CloseDevice((struct IORequest *)TimerIO);
        TimerBase=NULL;
        if (TimerIO) FreeMem(TimerIO,sizeof(struct timerequest));
        TimerIO=NULL;
#else
	sleep(1);
	flush_input();
#endif
#endif

	ttynorm();
}

#ifndef EBCDIC                   /* already defined in usuals.h */
#define BS 8
#define LF 10
#define CR 13
#define DEL 127
#endif

#ifdef VMS
int putch(int);
#else
#define putch(c) putc(c, stderr)
#endif

int
getstring(char *strbuf, unsigned maxlen, int echo)
/* Gets string from user, with no control characters allowed.
 * Also accumulates random numbers.
 * maxlen is max length allowed for string.
 * echo is TRUE iff we should echo keyboard to screen.
 * Returns null-terminated string in strbuf.
 */
{
	unsigned i;
	char c;

	ttycbreak();

#ifdef AMIGA     /* In case of -f (use stdio for plaintext input),
                    use ReqTools for input from the user */
        if (!IsInteractive(Input()))
           return AmigaRequestString(strbuf,maxlen,echo);
#endif

	fflush(stdout);
	i=0;
	for (;;) {
#ifndef VMS
		fflush(stderr);
#endif /* VMS */
		c = getch();
		trueRandEvent(c);
#ifdef VMS
		if (c == 25) {  /*  Control-Y detected */
		    ttynorm();
		    breakHandler(SIGINT);
		}
#endif /* VMS */
#if defined(MSDOS) || defined (__MSDOS__)
		if (c == 3)
			breakHandler(SIGINT);
#endif
		if (c==BS || c==DEL) {
			if (i) {
				if (echo) {
					putch(BS);
					putch(' ');
					putch(BS);
				}
				i--;
			} else {
				putch('\007');
			}
			continue;
		}
		if (c < ' ' && c != LF && c != CR) {
			putch('\007');
#if defined(MSDOS) || defined (__MSDOS__)
			if (c == 3)
				breakHandler(SIGINT);	
			if (c == 0)
				getch(); /* Skip extended key codes */
#endif
			continue;
		}
		if (echo)
			putch(c);
		if (c==CR) {
			if (echo)
				putch(LF);
			break;
		}
		if (c==LF)
			break;
		if (c=='\n')
			break;
		strbuf[i++] = c;
		if (i >= maxlen) {
			fputs("\007*\n", stderr);	/* -Enough! */
#if 0
			while (kbhit())
				getch();	/* clean up any typeahead */
#endif
			break;
		}
	}
	strbuf[i] = '\0';	/* null termination of string */

	ttynorm();

	return(i);		/* returns string length */
} /* getstring */


static void
flush_input(void)
{	/* on unix ttycbreak() will flush the input queue */
#if defined(MSDOS) || defined (__MSDOS__) || defined(MACTC5)
	while (kbhit())	/* flush typahead buffer */
		getch();
#endif
}
