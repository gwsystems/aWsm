#ifndef ARMORP_H
#define ARMORP_H

#include <stdio.h>

/* #define crcword unsigned short */	/* if CRCBITS is 16 */
/* #define crcword unsigned short */	/* if CRCBITS is 16 */

#ifdef __alpha
#define crcword unsigned int		/* if CRCBITS is 24 or 32 */
#else
#define crcword unsigned long		/* if CRCBITS is 24 or 32 */
#endif

extern crcword crcbytes(byte *buf, unsigned len, register crcword accum);
extern void init_crc(void);
extern int armor_file (char *infile, char *outfile, char *filename,
	char *clearname, boolean kv_label);
extern int de_armor_file(char *infile, char *outfile, long *curline);
extern boolean is_armor_file (char *infile, long startline);

#endif /* #ifdef ARMORP_H */
