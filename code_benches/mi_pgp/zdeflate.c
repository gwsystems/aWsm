/*

 Copyright (C) 1990-1992 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included
 unmodified, that it is not sold for profit, and that this copyright notice
 is retained.

*/

/*
 *  deflate.c by Jean-loup Gailly.
 *
 *  PURPOSE
 *
 *      Identify new text as repetitions of old text within a fixed-
 *      length sliding window trailing behind the new text.
 *
 *  DISCUSSION
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature is of this algorithm is that insertion and deletions
 *      from the string dictionary are very simple and thus fast. Insertions
 *      and deletions are performed at each input character, whereas string
 *      matches are performed only when the previous match ends. So it is
 *      preferable to spend more time in matches to allow very fast string
 *      insertions and deletions. The matching algorithm for small strings
 *      is inspired from that of Rabin & Karp. A brute force approach is
 *      used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost and uses more memory. However
 *      the F&G algorithm may be faster for some highly redundant files if
 *      the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many info-zippers for bug reports and testing.
 *
 *  REFERENCES
 *
 *      APPNOTE.TXT documentation file in PKZIP 2.0 distribution.
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 *  INTERFACE
 *
 *      void lm_init (int pack_level, ush *flags)
 *          Initialize the "longest match" routines for a new file
 *
 *      ulg deflate (void)
 *          Processes a new input file and return its compressed length. Sets
 *          the compressed length, crc, deflate flags and internal file
 *          attributes.
 */

#include "zunzip.h"
#include "zip.h"
#ifdef MACTC5
#include "Macutil3.h"
#endif

/* ===========================================================================
 * Configuration parameters
 */

/* Compile with MEDIUM_MEM to reduce the memory requirements or
 * with SMALL_MEM to use as little memory as possible.
 * Warning: defining these symbols affects MATCH_BUFSIZE and HASH_BITS
 * (see below) and thus affects the compression ratio. The compressed output
 * is still correct, and might even be smaller in some cases.
 */

#ifdef SMALL_MEM
#   define HASH_BITS  13  /* Number of bits used to hash strings */
#else
#ifdef MEDIUM_MEM
#   define HASH_BITS  14
#else
#   define HASH_BITS  15
   /* For portability to 16 bit machines, do not use values above 15. */
#endif
#endif

#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK     (WSIZE-1)
/* HASH_SIZE and WSIZE must be powers of two */

#define NIL 0
/* Tail of hash chains */

#define FAST 4
#define SLOW 2
/* speed options for the general purpose bit flag */

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* ===========================================================================
 * Local data used by the "longest match" routines.
 */

typedef ush Pos;
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

#ifndef DYN_ALLOC
  uch    far slide[2L*WSIZE];
  /* Sliding window. Input bytes are read into the second half of the window,
   * and move to the first half later to keep a dictionary of at least WSIZE
   * bytes. With this organization, matches are limited to a distance of
   * WSIZE-MAX_MATCH bytes, but this ensures that IO is always
   * performed with a length multiple of the block size. Also, it limits
   * the window size to 64K, which is quite useful on MSDOS.
   * To do: limit the window size to WSIZE+BSZ if SMALL_MEM (the code would
   * be less efficient since the data would have to be copied WSIZE/BSZ times)
   */
  Pos    far prev[WSIZE];
  /* Link to older string with same hash index. To limit the size of this
   * array to 64K, this link is maintained only for the last 32K strings.
   * An index in this array is thus a window index modulo 32K.
   */
  Pos    far head[HASH_SIZE];
  /* Heads of the hash chains or NIL */
#else
  uch    far * near slide = NULL;
  Pos    far * near prev   = NULL;
  static void far *__slide, *__prev;
  static Pos    far * near head;
#endif

long block_start;
/* window position at the beginning of the current output block. Gets
 * negative when the window is moved backwards.
 */

local unsigned near ins_h;  /* hash index of string to be inserted */

#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS
 */

unsigned int near prev_length;
/* Length of the best match at previous step. Matches not greater than this
 * are discarded. This is used in the lazy match evaluation.
 */

      unsigned near strstart;      /* start of string to insert */
unsigned near match_start;         /* start of matching string */
local int      near eofile;        /* flag set at end of input file */
local unsigned near lookahead;     /* number of valid bytes ahead in window */

unsigned near max_chain_length;
/* To speed up deflation, hash chains are never searched beyond this length.
 * A higher limit improves compression ratio but degrades the speed.
 */

local unsigned int max_lazy_match;
/* Attempt to find a better match only when the current match is strictly
 * smaller than this value.
 */

int near good_match;
/* Use a faster search when the previous match is longer than this */


/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
typedef struct config {
   int good_length;
   int max_lazy;
   unsigned max_chain;
   uch flag;
} config;

local config configuration_table[10] = {
/*      good lazy chain flag */
/* 0 */ {0,    0,    0,  0},     /* store only */
/* 1 */ {4,    4,   16,  FAST},  /* maximum speed  */
/* 2 */ {6,    8,   16,  0},
/* 3 */ {8,   16,   32,  0},
/* 4 */ {8,   32,   64,  0},
/* 5 */ {8,   64,  128,  0},
/* 6 */ {8,  128,  256,  0},
/* 7 */ {8,  128,  512,  0},
/* 8 */ {32, 258, 1024,  0},
/* 9 */ {32, 258, 4096,  SLOW}}; /* maximum compression */

/* Note: the current code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * but these restrictions can easily be removed at a small cost.
 */

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 *  Prototypes for local functions. Use asm version by default for
 *  MSDOS but not Unix. However the asm version version is recommended
 *  for 386 Unix.
 */
#ifdef ATARI_ST
#  undef MSDOS /* avoid the processor specific parts */
#endif
#if defined(MSDOS) && !defined(NO_ASM) && !defined(ASM)
#  define ASM
#endif

local void fill_window   OF((void));
      int  longest_match OF((IPos cur_match));
#ifdef ASM
      void match_init OF((void)); /* asm code initialization */
#endif

#ifdef DEBUG
local  void check_match OF((IPos start, IPos match, int length));
#endif

#undef MIN
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
/* The arguments must not have side effects. */

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

/* ===========================================================================
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#define INSERT_STRING(s, match_head) \
   (UPDATE_HASH(ins_h, slide[(s) + MIN_MATCH-1]), \
    prev[(s) & WMASK] = match_head = head[ins_h], \
    head[ins_h] = (s))

/* ===========================================================================
 * Initialize the "longest match" routines for a new file
 */
void lm_init (pack_level, flags)
    int pack_level; /* 0: store, 1: best speed, 9: best compression */
    ush *flags;     /* general purpose bit flag */
{
    register unsigned j;

    if (pack_level < 1 || pack_level > 9) error("bad pack level");

    /* Use dynamic allocation if compiler does not like big static arrays: */
#ifdef DYN_ALLOC
	__slide = slide = (uch far*) fcalloc(WSIZE*2*sizeof(uch)+16, 1);
	__prev = prev = (Pos far*) fcalloc(WSIZE*sizeof(Pos)+16, 1);
	head   = (Pos far*) fcalloc(HASH_SIZE, sizeof(Pos));

	if (slide == NULL || prev == NULL || head == NULL) {
		err(ZE_MEM, "window allocation");
	}
#endif /* DYN_ALLOC */
#ifdef ASM
    match_init(); /* initialize the asm code */
#endif
    /* Initialize the hash table. */
    for (j = 0;  j < HASH_SIZE; j++) head[j] = NIL;
    /* prev will be initialized on the fly */

    /* Set the default configuration parameters:
     */
    max_lazy_match   = configuration_table[pack_level].max_lazy;
    good_match       = configuration_table[pack_level].good_length;
    max_chain_length = configuration_table[pack_level].max_chain;
    *flags          |= configuration_table[pack_level].flag;
    /* ??? reduce max_chain_length for binary files */

    strstart = 0;
    block_start = 0L;

#if defined(MSDOS) && !defined(__32BIT__) && !defined(__GO32__)
    /* Can't read a 64K block under MSDOS */
    lookahead = read_buf((char*)slide, (unsigned)WSIZE);
#else
    lookahead = read_buf((char*)slide, 2*WSIZE);
#endif
    if (lookahead == 0 || lookahead == (unsigned)EOF) {
       eofile = 1, lookahead = 0;
       return;
    }
    eofile = 0;
    /* Make sure that we always have enough lookahead. This is important
     * if input comes from a device such as a tty.
     */
    while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window();

    ins_h = 0;
    for (j=0; j<MIN_MATCH-1; j++) UPDATE_HASH(ins_h, slide[j]);
    /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
     * not important since only literal bytes will be emitted.
     */
}

void lm_free()
{
#ifdef DYN_ALLOC
#ifndef __TURBOC__          /*EWS*/
	fcfree(__slide);
	fcfree(__prev);
	fcfree(head);
#else
        free(__slide);
        free(__prev);
        free(head);
#endif
	slide = NULL;
	prev = head = NULL;
#endif
}

/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */
#ifndef ASM
/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm. The code
 * is functionally equivalent, so you can use the C version if desired.
 */
int longest_match(cur_match)
    IPos cur_match;                             /* current match */
{
    unsigned chain_length = max_chain_length;   /* max hash chain length */
    register uch far *scan = slide + strstart;  /* current string */
    register uch far *match = scan;             /* matched string */
    register int len;                           /* length of current match */
    int best_len = prev_length;                 /* best match length so far */
    IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of slide index 0.
     */
#ifdef UNALIGNED_OK
    register ush scan_start = *(ush*)scan;
    register ush scan_end   = *(ush*)(scan+best_len-1);
#else
    register uch scan_start = *scan;
    register uch scan_end1  = scan[best_len-1];
    register uch scan_end   = scan[best_len];
#endif

    /* Do not waste too much time if we already have a good match: */
    if (prev_length >= good_match) {
        chain_length >>= 2;
    }

    do {
        Assert(cur_match < strstart, "no future");
        match = slide + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */
#if (defined(UNALIGNED_OK) && HASH_BITS >= 8)
        /* This code assumes sizeof(unsigned short) == 2 and
         * sizeof(unsigned long) == 4. Do not use UNALIGNED_OK if your
         * compiler uses different sizes.
         */
        if (*(ush*)(match+best_len-1) != scan_end ||
            *(ush*)match != scan_start) continue;

        len = MIN_MATCH - 4;
        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8.
         */
        do {} while ((len+=4) < MAX_MATCH-3 &&
                     *(ulg*)(scan+len) == *(ulg*)(match+len));
        /* The funny do {} generates better code for most compilers */

        if (*(ush*)(scan+len) == *(ush*)(match+len)) len += 2;
        if (scan[len] == match[len]) len++;

#else /* UNALIGNED_OK */
        if (match[best_len] != scan_end ||
            match[best_len-1] != scan_end1 || *match != scan_start)
           continue;
        /* It is not necessary to compare scan[1] and match[1] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that h_shift+8 <= HASH_BITS,
         * that is, when the last byte is entirely included in the hash key.
         * The condition is equivalent to
         *       (HASH_BITS+2)/3 + 8 <= HASH_BITS
         * or: HASH_BITS >= 13
         * Also, we check for a match at best_len-1 to get rid quickly of
         * the match with the suffix of the match made at the previous step,
         * which is known to fail.
         */
#if HASH_BITS >= 13
        len = 1;
#else
        len = 0;
#endif
        do {} while (++len < MAX_MATCH && scan[len] == match[len]);

#endif /* UNALIGNED_OK */

        if (len > best_len) {
            match_start = cur_match;
            best_len = len;
            if (len == MAX_MATCH) break;
#ifdef UNALIGNED_OK
            scan_end = *(ush*)(scan+best_len-1);
#else
            scan_end1  = scan[best_len-1];
            scan_end   = scan[best_len];
#endif
        }
    } while (--chain_length != 0 &&
             (cur_match = prev[cur_match & WMASK]) > limit);

    return best_len;
}
#endif /* NO_ASM */

#ifdef DEBUG
/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local void check_match(start, match, length)
    IPos start, match;
    int length;
{
    /* check that the match is indeed a match */
    if (memcmp((char*)slide + match,
                (char*)slide + start, length) != EQUAL) {
        fprintf(stderr,
            " start %d, match %d, length %d\n",
            start, match, length);
        error("invalid match");
    }
    if (verbose) {
        fprintf(stderr,"\\[%d,%d]", start-match, length);
        /* putc a macro, not safe to modify args!! */
        do { putc(slide[start], stderr); start++; } while (--length!=0);
    }
}
#else
#  define check_match(start, match, length)
#endif

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead, and sets eofile if end of input file.
 * IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
 * OUT assertions: at least one byte has been read, or eofile is set;
 *    file reads are performed for at least two bytes (required for the
 *    translate_eol option).
 */
local void fill_window()
{
    register unsigned n, m;
    unsigned more = (unsigned)((ulg)2*WSIZE - (ulg)lookahead - (ulg)strstart);
    /* Amount of free space at the end of the window. */

    /* If the window is full, move the upper half to the lower one to make
     * room in the upper half.
     */
    if (more == (unsigned)EOF) {
        /* Very unlikely, but possible on 16 bit machine if strstart == 0
         * and lookahead == 1 (input done one byte at time)
         */
        more--;
    } else if (more <= 1) {
        /* By the IN assertion, the window is not empty so we can't confuse
         * more == 0 with more == 64K on a 16 bit machine.
         */
        memcpy((char*)slide, (char*)slide+WSIZE, (unsigned)WSIZE);
        match_start -= WSIZE;
        strstart    -= WSIZE;
        /* strstart - WSIZE >= WSIZE - 1 - lookahead >= WSIZE - MIN_LOOKAHEAD
         * so we now have strstart >= MAX_DIST:
         */
        Assert (strstart >= MAX_DIST, "window slide too early");
        block_start -= (long) WSIZE;

        for (n = 0; n < HASH_SIZE; n++) {
            m = head[n];
            head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
        }
        for (n = 0; n < WSIZE; n++) {
            m = prev[n];
            prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
            /* If n is not on any hash chain, prev[n] is garbage but
             * its value will never be used.
             */
        }
        more += WSIZE;
#ifdef ZIP
        if (verbose) putc('.', stderr);
#endif
    }
    /* At this point, more >= 2 */
    n = read_buf((char*)slide+strstart+lookahead, more);
    if (n == 0 || n == (unsigned)EOF) {
        eofile = 1;
    } else {
        lookahead += n;
    }
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK(eof) \
   flush_block(block_start >= 0L ? (char*)&slide[block_start] : (char*)NULL,\
               (long)strstart - block_start, (eof))

/* ===========================================================================
 * Processes a new input file and return its compressed length.
 */
#ifdef NO_LAZY
ulg deflate()
{
    IPos hash_head; /* head of the hash chain */
    int flush;      /* set if current block must be flushed */
    unsigned match_length = 0;  /* length of best match */

    prev_length = MIN_MATCH-1;
    while (lookahead != 0) {
        /* Insert the string slide[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
#ifdef MACTC5
		mac_poll_for_break();
#endif
        INSERT_STRING(strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        if (hash_head != NIL && strstart - hash_head <= MAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of slide index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            match_length = longest_match (hash_head);
            /* longest_match() sets match_start */
            if (match_length > lookahead) match_length = lookahead;
        }
        if (match_length >= MIN_MATCH) {
            check_match(strstart, match_start, match_length);

            flush = ct_tally(strstart-match_start, match_length - MIN_MATCH);

            lookahead -= match_length;
            match_length--; /* string at strstart already in hash table */
            do {
                strstart++;
                INSERT_STRING(strstart, hash_head);
                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
                 * next lookahead bytes will always be emitted as literals.
                 */
            } while (--match_length != 0);
        } else {
            /* No match, output a literal byte */
            flush = ct_tally (0, slide[strstart]);
            lookahead--;
        }
        strstart++; 
        if (flush) FLUSH_BLOCK(0), block_start = strstart;

        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window();

    }
    return FLUSH_BLOCK(1); /* eof */
}
#else /* LAZY */
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
ulg deflate()
{
    IPos hash_head;          /* head of hash chain */
    IPos prev_match;         /* previous match */
    int flush;               /* set if current block must be flushed */
    int match_available = 0; /* set if previous match exists */
    register unsigned match_length = MIN_MATCH-1; /* length of best match */
#ifdef DEBUG
    extern ulg isize;        /* byte length of input file, for debug only */
#endif

    /* Process the input block. */
    while (lookahead != 0) {
        /* Insert the string slide[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
#ifdef MACTC5
		mac_poll_for_break();
#endif
        INSERT_STRING(strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
         */
        prev_length = match_length, prev_match = match_start;
        match_length = MIN_MATCH-1;

        if (hash_head != NIL && prev_length < max_lazy_match &&
            strstart - hash_head <= MAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of slide index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            match_length = longest_match (hash_head);
            /* longest_match() sets match_start */
            if (match_length > lookahead) match_length = lookahead;
            /* Ignore a length 3 match if it is too distant: */
            if (match_length == MIN_MATCH && strstart-match_start > TOO_FAR){
                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                match_length--;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (prev_length >= MIN_MATCH && match_length <= prev_length) {

            check_match(strstart-1, prev_match, prev_length);

            flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
             */
            lookahead -= prev_length-1;
            prev_length -= 2;
            do {
                strstart++;
                INSERT_STRING(strstart, hash_head);
                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
                 * next lookahead bytes will always be emitted as literals.
                 */
            } while (--prev_length != 0);
            match_available = 0;
            match_length = MIN_MATCH-1;
            strstart++;
            if (flush) FLUSH_BLOCK(0), block_start = strstart;

        } else if (match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c",slide[strstart-1]));
            if (ct_tally (0, slide[strstart-1])) {
                FLUSH_BLOCK(0), block_start = strstart;
            }
            strstart++;
            lookahead--;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            match_available = 1;
            strstart++;
            lookahead--;
        }
#if 0	/* for pgp: disabled to allow compiling with -DDEBUG */
        Assert (strstart <= isize && lookahead <= isize, "a bit too far");
#endif

        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window();
    }
    if (match_available) ct_tally (0, slide[strstart-1]);

    return FLUSH_BLOCK(1); /* eof */
}
#endif /* LAZY */
