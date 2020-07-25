/*
 * charset.c
 *
 * Conversion tables and routines to support different character sets.
 * The PGP internal format is latin-1.
 *
 * (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
 * The author assumes no liability for damages resulting from the use
 * of this software, even if the damage results from defects in this
 * software.  No warranty is expressed or implied.
 *
 * Code that has been incorporated into PGP from other sources was
 * either originally published in the public domain or is used with
 * permission from the various authors.
 *
 * PGP is available for free to the public under certain restrictions.
 * See the PGP User's Guide (included in the release package) for
 * important information about licensing, patent restrictions on
 * certain algorithms, trademarks, copyrights, and export controls.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "usuals.h"
#include "language.h"
#include "charset.h"
#include "system.h"

#ifndef NULL
#define	NULL	0
#endif

#define UNK	'?'

static unsigned char
intern2ascii[] = { /* ISO 8859-1 Latin Alphabet 1 to US ASCII */
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
 32,  33,  99,  35,  36,  89, 124,  80,  34,  67,  97,  34, 126,  45,  82,  95,
111, UNK,  50,  51,  39, 117,  45,  45,  44,  49, 111,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
 68,  78,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89,  84, 115,
 97,  97,  97,  97,  97,  97,  97,  99, 101, 101, 101, 101, 105, 105, 105, 105,
100, 110, 111, 111, 111, 111, 111,  47, 111, 117, 117, 117, 117, 121, 116, 121
};

static unsigned char
intern2cp850[] = { /* ISO 8859-1 Latin Alphabet 1 (Latin-1)
		      to IBM Code Page 850 (International) */
186, 205, 201, 187, 200, 188, 204, 185, 203, 202, 206, 223, 220, 219, 254, 242,
179, 196, 218, 191, 192, 217, 195, 180, 194, 193, 197, 176, 177, 178, 213, 159,
255, 173, 189, 156, 207, 190, 221, 245, 249, 184, 166, 174, 170, 240, 169, 238,
248, 241, 253, 252, 239, 230, 244, 250, 247, 251, 167, 175, 172, 171, 243, 168,
183, 181, 182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222, 214, 215, 216,
209, 165, 227, 224, 226, 229, 153, 158, 157, 235, 233, 234, 154, 237, 232, 225,
133, 160, 131, 198, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139,
208, 164, 149, 162, 147, 228, 148, 246, 155, 151, 163, 150, 129, 236, 231, 152
};

static unsigned char
cp8502intern[] = { /* IBM Code Page 850 to Latin-1 */
199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 248, 163, 216, 215, 159,
225, 237, 243, 250, 241, 209, 170, 186, 191, 174, 172, 189, 188, 161, 171, 187,
155, 156, 157, 144, 151, 193, 194, 192, 169, 135, 128, 131, 133, 162, 165, 147,
148, 153, 152, 150, 145, 154, 227, 195, 132, 130, 137, 136, 134, 129, 138, 164,
240, 208, 202, 203, 200, 158, 205, 206, 207, 149, 146, 141, 140, 166, 204, 139,
211, 223, 212, 210, 245, 213, 181, 254, 222, 218, 219, 217, 253, 221, 175, 180,
173, 177, 143, 190, 182, 167, 247, 184, 176, 168, 183, 185, 179, 178, 142, 160
};

static unsigned char
intern2cp852[] = { /* ISO 8859-2 Latin Alphabet 2 (Latin-2)
		      to IBM Code Page 852 (Eastern Europe) */
186, 205, 201, 187, 200, 188, 204, 185, 203, 202, 206, 223, 220, 219, 254, UNK,
179, 196, 218, 191, 192, 217, 195, 180, 194, 193, 197, 176, 177, 178, UNK, UNK,
255, 164, 244, 157, 207, 149, 151, 245, 249, 230, 184, 155, 141, 240, 166, 189,
248, 165, 247, 136, 239, 150, 151, 243, 242, 231, 173, 156, 171, 241, 167, 190,
232, 181, 182, 198, 142, 145, 143, 128, 172, 144, 168, 211, 183, 214,  73, 210,
209, 227, 213, 224, 226, 138, 153, 158, 252, 222, 233, 235, 154, 237, 221, 225,
234, 160, 131, 199, 132, 146, 134, 135, 159, 130, 169, 137, 216, 161, 140, 212,
208, 228, 229, 162, 147, 139, 148, 246, 253, 133, 163, 251, 129, 236, 238, 250
};

static unsigned char
cp8522intern[] = { /* IBM Code Page 852 to Latin-2 */
199, 252, 233, 226, 228, 249, 230, 231, 179, 235, 213, 245, 238, 172, 196, 198,
201, 197, 229, 244, 246, 165, 181, 166, 182, 214, 220, 171, 187, 163, 215, 232,
225, 237, 243, 250, 161, 177, 174, 190, 202, 234, UNK, 188, 200, 186,  60,  62,
155, 156, 157, 144, 151, 193, 194, 204, 170, 135, 128, 131, 133, 175, 191, 147,
148, 153, 152, 150, 145, 154, 195, 227, 132, 130, 137, 136, 134, 129, 138, 164,
240, 208, 207, 203, 239, 210, 205,  85, 236, 149, 146, 141, 140, 222, 217, 139,
211, 223, 212, 209, 241, 242, 169, 185, 192, 218, 224, 219, 253, 221, 254, 180,
173, 189, 184, 183, 162, 167, 247, 178, 176, 168, 255, 251, 216, 248, 142, 160
};

static unsigned char
intern2cp860[] = { /* ISO 8859-1 Latin Alphabet 1 (Latin-1)
                      to IBM Code Page 860 (Portuguese) */
186, 205, 201, 187, 200, 188, 204, 185, 203, 202, 206, 223, 220, 219, 254,  95,
179, 196, 218, 191, 192, 217, 195, 180, 194, 193, 197, 176, 177, 178, UNK, UNK,
255, 173, 155, 156,  36,  89, 124,  80,  34,  67, 166, 174, 170,  45,  82,  95,
248, 241, 253,  51,  39, 230,  45, 250,  44,  49, 167, 175, 172, 171, UNK, 168,
145, 134, 143, 142,  65,  65,  65, 128, 146, 144, 137,  69, 152, 139,  73,  73,
 68, 165, 169, 159, 140, 153,  79, 120,  79, 157, 150,  85, 154,  89,  84, 225,
133, 160, 131, 132,  97,  97,  97, 135, 138, 130, 136, 101, 141, 161, 105, 105,
100, 164, 149, 162, 147, 148, 111, 246, 111, 151, 163, 117, 129, 121, 116, 121
};

static unsigned char
cp8602intern[] = { /* IBM Code Page 860 to Latin-1 */
199, 252, 233, 226, 227, 224, 193, 231, 234, 202, 232, 205, 212, 236, 195, 194,
201, 192, 200, 244, 245, 242, 218, 249, 204, 213, 220, 162, 163, 217, 164, 211,
225, 237, 243, 250, 241, 209, 170, 186, 191, 210, 172, 189, 188, 161, 171, 187,
155, 156, 157, 144, 151, 151, 135, 147, 131, 135, 128, 131, 133, 149, 133, 147,
148, 153, 152, 150, 145, 154, 150, 134, 132, 130, 137, 136, 134, 129, 138, 137,
153, 136, 152, 148, 132, 130, 146, 154, 138, 149, 146, 141, 140, 141, 141, 139,
UNK, 223, UNK, UNK, UNK, UNK, 181, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
 61, UNK, UNK, UNK, UNK, UNK, 247, 126, 176, 183, 183, UNK, 110, 178, 142, 160
};

static unsigned char
intern2keybcs[] = { /* ISO 8859-2 Latin Alphabet 2 (Latin-2)
 		       to KEYBCS2 (Eastern Europe) */
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK,  65, UNK,  76, UNK, 156,  83, 173, UNK, 155,  83, 134,  90, UNK, 146,  90,
248, UNK, UNK, 108, UNK, 140, 115, UNK, UNK, 168, 115, 159, 122, UNK, 145, 122,
171, 143,  65,  65, 142, 138,  67,  67, 128, 144,  69,  69, 137, 139,  73, 133,
 68,  78, 165, 149, 167,  79, 153, UNK, 158, 166, 151,  85, 154, 157,  84, 225,
170, 160,  97,  97, 132, 141,  99,  99, 135, 130, 101, UNK, 136, 161, 105, 131,
100, 110, 164, 162, 147, 111, 148, UNK, 169, 150, 163, 117, 129, 152, 116, UNK
};

static unsigned char
keybcs2intern[] = { /* KEYBCS2 to Latin-2 */
200, 252, 233, 239, 228, 207, 171, 232, 236, 204, 197, 205, 181, 229, 196, 193,
201, 190, 174, 244, 246, 211, 249, 218, 253, 214, 220, 169, 165, 221, 216, 187,
225, 237, 243, 250, 242, 210, 217, 212, 185, 248, 224, 192, UNK, 167, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, 223, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, 176, UNK, UNK, UNK, UNK, UNK, UNK, UNK
};

static unsigned char
intern2next[] = { /* ISO 8859-1 Latin Alphabet 1 (Latin-1)
                     to NeXTSTEP char set */
 UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,  UNK,
0300, 0301, 0302, 0303, 0304, 0305, 0306, 0307, 0310,  UNK, 0312, 0313,  UNK, 0272, 0316, 0247,
 UNK, 0241, 0242, 0243, 0250, 0245, 0265,  UNK, 0310, 0240, 0343, 0253, 0276, 0261, 0260, 0320,
0312, 0321, 0311, 0314, 0270, 0235, 0266, 0267, 0313, 0300,  UNK, 0273, 0322, 0323, 0324, 0277,
0201, 0202, 0203, 0204, 0205, 0206, 0341, 0207, 0210, 0211, 0212, 0213, 0214, 0215, 0216, 0217,
0220, 0221, 0222, 0223, 0224, 0225, 0226, 0236, 0351, 0227, 0230, 0231, 0232, 0233, 0234, 0373,
0325, 0326, 0327, 0330, 0331, 0332, 0361, 0333, 0334, 0335, 0336, 0337, 0340, 0342, 0344, 0345,
0346, 0347, 0354, 0355, 0356, 0357, 0360, 0237, 0371, 0362, 0363, 0364, 0366, 0367, 0374, 0375
};

static unsigned char
next2intern[] = { /* NeXTSTEP char set to Latin-1 */
 UNK, 0300, 0301, 0302, 0303, 0304, 0305, 0307, 0310, 0311, 0312, 0313, 0314, 0315, 0316, 0317,
0320, 0321, 0322, 0323, 0324, 0325, 0326, 0331, 0332, 0333, 0334, 0335, 0336, 0265, 0337, 0267,
0251, 0241, 0242, 0243, 0057, 0245, 0146, 0247, 0244, 0140, 0042, 0253, 0074, 0076,  UNK,  UNK,
0256, 0255,  UNK,  UNK, 0056, 0246, 0266, 0267, 0054, 0042, 0235, 0273,  UNK,  UNK, 0254, 0277,
0220, 0221, 0222, 0223, 0224, 0225, 0226, 0232, 0230, 0262, 0227, 0270, 0263, 0042, 0236, 0226,
0257, 0261, 0274, 0275, 0276, 0340, 0341, 0342, 0343, 0344, 0345, 0347, 0350, 0351, 0352, 0353,
0354, 0306, 0355, 0252, 0356, 0357, 0365, 0361, 0243, 0330,  UNK, 0272, 0362, 0363, 0364, 0365,
0366, 0346, 0371, 0372, 0373, 0151, 0374, 0375, 0154, 0370,  UNK, 0337, 0376, 0377, UNK, UNK
};

#ifdef MACTC5
Boolean iso_latin1 = false;
#endif

static unsigned char
intern2mac[] = { /* ISO 8859-1 Latin Alphabet 1 (Latin1)
                    to Macintosh Geneva/Monaco */
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
202, 193, 162, 163, 217, 180, 124, 164, 172, 169, 187, 199, 194, 209, 168,  95,
188, 177,  50,  51, 171, 181, 166, 165,  44,  49, 161, 200, UNK, UNK, UNK, 192,
203, 135, 129, 204, 128, 129, 174, 130, 143, 131, 144, 145, 147, 146, 148, 149,
 68, 132, 152, 151, 153, 205, 133, 120, 175, 157, 156, 158, 134,  89,  84, 167,
136, 135, 137, 139, 138, 140,  97, 141, 143, 142, 144, 145, 147, 146, 148, 149,
100, 150, 152, 151, 153, 155, 154, 214, 191, 157, 156, 158, 159, 121, 116, 216
};

static unsigned char
mac2intern[] = { /* Macintosh Geneva/Monaco to Latin-1 */
196, 197, 199, 201, 209, 214, 220, 225, 224, 226, 228, 227, 229, 231, 233, 232,
234, 235, 237, 236, 238, 239, 241, 243, 242, 244, 246, 245, 250, 249, 251, 252,
UNK, 186, 162, 163, 167, 183, 182, 223, 174, 169, UNK, 180, 168, UNK, 198, 216,
UNK, 177, UNK, UNK, 165, 181, 100,  83,  80, 112,  83, 170, 176,  79, 230, 248,
191, 161, 172, UNK, 102, 126,  68, 171, 187, UNK, 160, 192, 195, 213,  79, 111,
 45, 173,  34,  34,  96,  39, 247, UNK, 255, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK
};

/* Russian language specific conversation section */
/* Two point-to-point charset decode tables       */
/* produced by Andrew A. Chernov                  */
/* Decode single char from KOI8-R to ALT-CODES, if present */
static unsigned char intern2alt[] = {
	0xc4, 0xb3, 0xda, 0xbf, 0xc0, 0xd9, 0xc3, 0xb4,
	0xc2, 0xc1, 0xc5, 0xdf, 0xdc, 0xdb, 0xdd, 0xde,
	0xb0, 0xb1, 0xb2, 0xf4, 0xfe, 0xf9, 0xfb, 0xf7,
	0xf3, 0xf2, 0xff, 0xf5, 0xf8, 0xfd, 0xfa, 0xf6,
	0xcd, 0xba, 0xd5, 0xf1, 0xd6, 0xc9, 0xb8, 0xb7,
	0xbb, 0xd4, 0xd3, 0xc8, 0xbe, 0xbd, 0xbc, 0xc6,
	0xc7, 0xcc, 0xb5, 0xf0, 0xb6, 0xb9, 0xd1, 0xd2,
	0xcb, 0xcf, 0xd0, 0xca, 0xd8, 0xd7, 0xce, 0xfc,
	0xee, 0xa0, 0xa1, 0xe6, 0xa4, 0xa5, 0xe4, 0xa3,
	0xe5, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae,
	0xaf, 0xef, 0xe0, 0xe1, 0xe2, 0xe3, 0xa6, 0xa2,
	0xec, 0xeb, 0xa7, 0xe8, 0xed, 0xe9, 0xe7, 0xea,
	0x9e, 0x80, 0x81, 0x96, 0x84, 0x85, 0x94, 0x83,
	0x95, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e,
	0x8f, 0x9f, 0x90, 0x91, 0x92, 0x93, 0x86, 0x82,
	0x9c, 0x9b, 0x87, 0x98, 0x9d, 0x99, 0x97, 0x9a
};

/* Decode single char from ALT-CODES, if present, to KOI8-R */
static unsigned char alt2intern[] = {
	0xe1, 0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa,
	0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
	0xf2, 0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe,
	0xfb, 0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1,
	0xc1, 0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda,
	0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
	0x90, 0x91, 0x92, 0x81, 0x87, 0xb2, 0xb4, 0xa7,
	0xa6, 0xb5, 0xa1, 0xa8, 0xae, 0xad, 0xac, 0x83,
	0x84, 0x89, 0x88, 0x86, 0x80, 0x8a, 0xaf, 0xb0,
	0xab, 0xa5, 0xbb, 0xb8, 0xb1, 0xa0, 0xbe, 0xb9,
	0xba, 0xb6, 0xb7, 0xaa, 0xa9, 0xa2, 0xa4, 0xbd,
	0xbc, 0x85, 0x82, 0x8d, 0x8c, 0x8e, 0x8f, 0x8b,
	0xd2, 0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde,
	0xdb, 0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1,
	0xb3, 0xa3, 0x99, 0x98, 0x93, 0x9b, 0x9f, 0x97,
	0x9c, 0x95, 0x9e, 0x96, 0xbf, 0x9d, 0x94, 0x9a
};

/*
 * Most Unixes has KOI8, and DOS has ALT_CODES
 * If your Unix is non-standard, set CHARSET to "alt_codes"
 * in config.txt
 */

#ifndef	DEFAULT_RU_CSET
#ifdef MSDOS
#define DEFAULT_RU_CSET "alt_codes"
#else
#define DEFAULT_RU_CSET "koi8"
#endif
#endif

/* End of Russian section */

#ifndef	DEFAULT_CSET
#if defined(MSDOS) || defined(OS2)
#define DEFAULT_CSET    "cp850"
#elif defined(NEXT)
#define	DEFAULT_CSET	"next"
#elif defined(MACTC5)
#define	DEFAULT_CSET	"mac"
#else
#define	DEFAULT_CSET	"noconv"
#endif
#endif

#ifdef EBCDIC
/* ebcdic-ascii converting, accustom to your local MVS-settings */
/* in this case it's taken from c370.c                          */
#define ebcdic__ascii ebcdic_ascii
#define ascii__ebcdic ascii_ebcdic
#endif /* EBCDIC */

int CONVERSION = NO_CONV;      /* None text file conversion at start time */

unsigned char *ext_c_ptr;
static unsigned char *int_c_ptr;

char charset[16] = "";

void
init_charset(void)
{
	ext_c_ptr = NULL;	/* NULL means latin1 or KOI8
				   (internal format) */
	int_c_ptr = NULL;

	if (charset[0] == '\0') {
		/* use default character set for this system */
#ifdef MACTC5
		if (iso_latin1)
			strcpy(charset, DEFAULT_CSET);
		else
			strcpy(charset, "noconv");
#else
		if (strcmp(language, "ru") == 0)
			strcpy(charset, DEFAULT_RU_CSET);
		else
			strcpy(charset, DEFAULT_CSET);
#endif
	} else {
		strlwr(charset);
	}

	/* latin-1 and KOI8 are in internal format: no conversion needed */
	if (!strcmp(charset, "latin1") || !strcmp(charset, "koi8") ||
		!strcmp(charset, "noconv"))
		return;

	if (!strcmp(charset, "cp850")) {
		ext_c_ptr = intern2cp850;
		int_c_ptr = cp8502intern;
	} else if (!strcmp(charset, "cp852")) {
		ext_c_ptr = intern2cp852;
		int_c_ptr = cp8522intern;
	} else if (!strcmp(charset, "cp860")) {
		ext_c_ptr = intern2cp860;
		int_c_ptr = cp8602intern;
 	} else if (!strcmp(charset, "cp866")) {
 		ext_c_ptr = intern2alt;
 		int_c_ptr = alt2intern;
	} else if (!strcmp(charset, "alt_codes")) {
		ext_c_ptr = intern2alt;
		int_c_ptr = alt2intern;
	} else if (!strcmp(charset, "keybcs2"))  {
	        ext_c_ptr = intern2keybcs;
		int_c_ptr = keybcs2intern;
	} else if (!strcmp(charset, "next"))  {
	        ext_c_ptr = intern2next;
		int_c_ptr = next2intern;
	} else if (!strcmp(charset, "mac"))  {
	        ext_c_ptr = intern2mac;
		int_c_ptr = mac2intern;
	} else if (!strcmp(charset, "ascii")) {
		ext_c_ptr = intern2ascii;
	} else {
		fprintf(stderr, LANG("Unsupported character set: '%s'\n"),
			charset);
		strcpy(charset, "noconv");
	}
}

#ifdef EBCDIC
char EXT_C(char c)  { return ascii__ebcdic[c]; }
char INT_C(char c)  { return ebcdic__ascii[c]; }
#else /* !EBCDIC */
char
EXT_C(char c)
{
 	if (!(c & 0x80) || !ext_c_ptr)
		return c;
	return ext_c_ptr[c & 0x7f];
}

char
INT_C(char c)
{
 	if (!(c & 0x80) || !int_c_ptr)
		return c;
	return int_c_ptr[c & 0x7f];
}
#endif /* !EBCDIC */

/*
 * to_upper() and to_lower(), replacement for toupper() and tolower(),
 * calling to_upper() on uppercase or to_lower on lowercase characters
 * is handled correctly.
 * 
 * XXX: should handle local characterset when 8-bit userID's are allowed
 */
#ifdef EBCDIC
/* With EBCDIC-charset things like (c >= 'a' && c <= 'z') do not work!!!
 * Therefor use the appropriate ctype-functions
 */
#include <ctype.h>
int to_upper(int c) { return toupper(c); }
int to_lower(int c) { return tolower(c); }
#else /* !EBCDIC */
int
to_upper(int c)
{
 	c &= 0xFF;
 	if (islower(c))
 		return (toupper(c));
 	return c;
}

int
to_lower(int c)
{
 	c &= 0xFF;
 	if (isupper(c))
 		return (tolower(c));
 	return c;
}
#endif /* !EBCDIC */

#ifdef EBCDIC
void CONVERT_TO_CANONICAL_CHARSET(char *s) /* String to internal string (at same place) */
{
	for (; *s; s++) *s = INT_C(*s);
}

static char buf[128];
char * LOCAL_CHARSET( char *s)             /* String to external string (at extra place) */
{
	strcpy( buf, s );
	for (s=buf; *s; s++) *s = EXT_C(*s);
	return buf;
}
#endif /* EBCDIC */
