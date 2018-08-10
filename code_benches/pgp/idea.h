#ifndef IDEA_H
#define IDEA_H

/*
 *	idea.h - header file for idea.c
 */

#include "usuals.h"  /* typedefs for byte, word16, boolean, etc. */

#define IDEAKEYSIZE 16
#define IDEABLOCKSIZE 8

#define IDEAROUNDS 8
#define IDEAKEYLEN (6*IDEAROUNDS+4)

/*
 * iv[] is used as a circular buffer.  bufleft is the number of
 * bytes at the end which have to be filled in before we crank
 * the block cipher again.  We do the block cipher operation
 * lazily: bufleft may be 0.  When we need one more byte, we
 * crank the block cipher and set bufleft to 7.
 *
 * oldcipher[] holds the previous 8 bytes of ciphertext, for use
 * by ideaCfbSync() and Phil's, ahem, unique (not insecure, just
 * unusual) way of doing CFB encryption.
 */
struct IdeaCfbContext {
	byte oldcipher[8];
	byte iv[8];
	word16 key[IDEAKEYLEN];
	int bufleft;
};

struct IdeaRandContext {
	byte outbuf[8];
	word16 key[IDEAKEYLEN];
	int bufleft;
	byte internalbuf[8];
};

void ideaCfbReinit(struct IdeaCfbContext *context, byte const *iv);
void ideaCfbInit(struct IdeaCfbContext *context, byte const (key[16]));
void ideaCfbSync(struct IdeaCfbContext *context);
void ideaCfbDestroy(struct IdeaCfbContext *context);
void ideaCfbEncrypt(struct IdeaCfbContext *context,
		    byte const *src, byte *dest, int count);
void ideaCfbDecrypt(struct IdeaCfbContext *context,
		    byte const *src, byte *dest, int count);
void ideaRandInit(struct IdeaRandContext *context, byte const (key[16]),
		  byte const (seed[8]));
byte ideaRandByte(struct IdeaRandContext *c);
void ideaRandWash(struct IdeaRandContext *c, struct IdeaCfbContext *cfb);
void ideaRandState(struct IdeaRandContext *c, byte key[16], byte seed[8]);

#endif /* !IDEA_H */
