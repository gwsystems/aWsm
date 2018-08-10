/* usuals.h - The usual typedefs, etc.
*/
#ifndef USUALS /* Assures no redefinitions of usual types...*/
#define USUALS

typedef unsigned char boolean;	/* values are TRUE or FALSE */
typedef unsigned char byte;	/* values are 0-255 */
typedef byte *byteptr;	/* pointer to byte */
typedef char *string;	/* pointer to ASCII character string */
typedef unsigned short word16;	/* values are 0-65535 */
#ifdef __alpha
typedef unsigned int word32;	/* values are 0-4294967295 */
#else
typedef unsigned long word32;	/* values are 0-4294967295 */
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif	/* if TRUE not already defined */

#ifndef min	/* if min macro not already defined */
#define min(a,b) (((a)<(b)) ? (a) : (b) )
#define max(a,b) (((a)>(b)) ? (a) : (b) )
#endif	/* if min macro not already defined */

/* void for use in pointers */
#ifndef NO_VOID_STAR
#define	VOID	void
#else
#define	VOID	char
#endif

	/* Zero-fill the byte buffer. */
#define fill0(buffer,count)	memset( buffer, 0, count )

	/* This macro is for burning sensitive data.  Many of the
	   file I/O routines use it for zapping buffers */
#define burn(x) fill0((VOID *)&(x),sizeof(x))

#ifdef C370
#define EBCDIC
#include "c370.h"
#endif /* C370 */

#ifdef LE370
#define EBCDIC
#endif /* LE370 */

#ifdef EBCDIC

#ifndef PSTR
#define PSTR(x) x
#endif
#define PORTABLE
#define HIGHFIRST
#define IDEA32
#define UNIT32
#define USE_NBIO
#define NOTERMIO
#define BEL  '\a'
#define BS   '\b'
#define FF   '\f'
#define LF   '\n'
#define CR   '\r'
#define HT   '\t'
#define VT   '\v'
#define TAB   HT
#define DEL   0x07              /* ASCII 0x7f */
#define ESC   0x27              /* ASCII 0x1b */
#define CtrlZ EOF               /* ASCII 0x1a */

#pragma map( mp_shortdiv,          "MPILIB01" )
#pragma map( mp_shortmod,          "MPILIB02" )
#pragma map( mp_modexp,            "MPILIB03" )
#pragma map( mp_modexp_crt,        "MPILIB04" )
#pragma map( cryptRandWriteFile,   "RANDOM00" )
#pragma map( cryptRandOpen,        "RANDOM01" )
#pragma map( cryptRandWash,        "RANDOM02" )
#pragma map( cryptRandByte,        "RANDOM03" )
#pragma map( cryptRandSave,        "RANDOM04" )
#pragma map( cryptRandCreate,      "RANDOM05" )
#pragma map( trueRandEvent,        "RANDOM06" )
#pragma map( trueRandFlush,        "RANDOM07" )
#pragma map( trueRandConsume,      "RANDOM08" )
#pragma map( trueRandAccumLater,   "RANDOM09" )
#pragma map( trueRandAccum,        "RANDOM10" )
#pragma map( trueRandByte,         "RANDOM11" )
#pragma map( version_error,        "CRYPTO01" )
#pragma map( version_byte_error,   "CRYPTO02" )
#pragma map( copyfile,             "FILEIO01" )
#pragma map( copyfilepos,          "FILEIO02" )
#pragma map( copyfile_to_canon,    "FILEIO03" )
#pragma map( copyfile_from_canon,  "FILEIO04" )
#pragma map( copyfiles_by_name,    "FILEIO05" )
#pragma map( savetemp,             "FILEIO06" )
#pragma map( savetempbak,          "FILEIO07" )
#pragma map( extract_from_keyring, "KEYMGM01" )
#pragma map( extract_keyID,        "KEYMGM02" )
#pragma map( getpubuserid,         "KEYMGM03" )
#pragma map( getpubusersig,        "KEYMGM04" )
#pragma map( version_byte,         "PGP00001" )
#pragma map( compress_enabled,     "PGP00002" )
#pragma map( compressSignature,    "PGP00003" )
#pragma map( processConfigLine,    "CONFIG01" )
#pragma map( processConfigFile,    "CONFIG02" )
#pragma map( write_trust,          "KEYMAI01" )
#pragma map( write_trust_pos,      "KEYMAI02" )
#pragma map( rsa_public_encrypt,   "RSAGLU01" )
#pragma map( rsa_public_decrypt,   "RSAGLU02" )
#pragma map( rsa_private_encrypt,  "RSAGLU03" )
#pragma map( rsa_private_decrypt,  "RSAGLU04" )
#pragma map( fetch_word16,         "MPIIO001" )
#pragma map( fetch_word32,         "MPIIO002" )
#pragma map( put_word16,           "MPIIO003" )
#pragma map( put_word32,           "MPIIO004" )
#pragma map( ideaCfbDestroy,       "IDEA0001" )
#pragma map( ideaCfbDecrypt,       "IDEA0002" )
#pragma map( ideaRandInit,         "IDEA0003" )
#pragma map( ideaRandByte,         "IDEA0004" )
#pragma map( ideaRandWash,         "IDEA0005" )
#pragma map( ideaRandState,        "IDEA0006" )
#pragma map( randPoolStir,         "RANDPO01" )
#pragma map( randPoolAddBytes,     "RANDPO02" )
#pragma map( randPoolGetBytes,     "RANDPO03" )
#pragma map( randPoolGetByte,      "RANDPO04" )
#pragma map( inflate_codes,        "ZINFLA01" )
#pragma map( inflate_stored,       "ZINFLA02" )
#pragma map( inflate_fixed,        "ZINFLA03" )
#pragma map( inflate_dynamic,      "ZINFLA04" )
#pragma map( inflate_block,        "ZINFLA05" )
#pragma map( inflate_entry,        "ZINFLA06" )

#endif /* EBCDIC */

#endif	/* if USUALS not already defined */
