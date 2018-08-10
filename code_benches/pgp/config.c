/*	config.c  - config file parser by Peter Gutmann
	Parses config file for PGP

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

	Modified 24 Jun 92 - HAJK
	Misc fixes for VAX C restrictions

	Updated by Peter Gutmann to only warn about unrecognized options,
	so future additions to the config file will give old versions a
	chance to still run.  A number of code cleanups, too.  */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "usuals.h"
#include "fileio.h"
#include "language.h"	/*added by Naoki */
#include "pgp.h"
#include "config.h"
#include "charset.h"

/* Various maximum/minimum allowable settings for config options */

#define MIN_MARGINALS	1
#define MIN_COMPLETE	1
#define MAX_COMPLETE	4
#define MIN_CERT_DEPTH	0
#define MAX_CERT_DEPTH	8

/* Prototypes for local functions */

static int lookup( char *key, int keyLength, char *keyWords[], int range );
static int extractToken( char *buffer, int *endIndex, int *length );
static int getaString( char *buffer, int *endIndex );
static int getAssignment( char *buffer, int *endIndex, INPUT_TYPE settingType );
static void processAssignment( int intrinsicIndex );

/* The external config variables we can set here are referenced in pgp.h */

/* Return values */

#define ERROR	-1
#define OK		0

/* The types of error we check for */

enum { NO_ERROR, ILLEGAL_CHAR_ERROR, LINELENGTH_ERROR };

#define CPM_EOF		0x1A	/* ^Z = CPM EOF char */

#define MAX_ERRORS	3	/* Max.no.errors before we give up */

#define LINEBUF_SIZE	100	/* Size of input buffer */

static int line;		/* The line on which an error occurred */
static int errCount;		/* Total error count */
static boolean hasError;	/* Whether this line has an error in it */

/* The settings parsed out by getAssignment() */

static char str[ LINEBUF_SIZE ];
static int value;
static char *errtag;		/* Prefix for printing error messages */
static char optstr[ 100 ];	/* Option being processed */
#ifdef MACTC5
extern boolean use_ftypes, wipe_warning, recycle_passwd;
#endif

/* A .CFG file roughly follows the format used in the world-famous HPACK
   archiver and is as follows:

	- Leading spaces/tabs (whitespace) are ignored.

	- Lines with a '#' as the first non-whitespace character are treated
	  as comment lines.

	- All other lines are treated as config options for the program.

	- Lines may be terminated by either linefeeds, carriage returns, or
	  carriage return/linefeed pairs (the latter being the DOS default
	  method of storing text files).

	- Config options have the form:

	  <option> '=' <setting>

	  where <setting> may be 'on', 'off', a numeric value, or a string
	  value.

	- If strings have spaces or the '#' character inside them they must be
	  surrounded by quote marks '"' */

/* Intrinsic variables */

#define NO_INTRINSICS		(sizeof(intrinsics) / sizeof(intrinsics[0]))
#define CONFIG_INTRINSICS	BATCHMODE

enum {
	ARMOR, COMPRESS, SHOWPASS, KEEPBINARY, LANGUAGE,
	MYNAME, TEXTMODE, TMP, TZFIX, VERBOSE, BAKRING,
	ARMORLINES, COMPLETES_NEEDED, MARGINALS_NEEDED, PAGER,
	CERT_DEPTH, CHARSET, CLEARSIG, SELF_ENCRYPT,
	INTERACTIVE, PUBRING, SECRING, RANDSEED,
	COMMENT, AUTOSIGN,
        LEGAL_KLUDGE,
#ifdef MACTC5
	FILE_TYPES, WIPE_WARNING, RECYCLE_PASSWD, MULTIPLE_RECIPIENTS,
#endif
	/* options below this line can only be used as command line
	 * "long" options */
	BATCHMODE, FORCE, NOMANUAL, MAKERANDOM
	};

static char *intrinsics[] = {
	"ARMOR", "COMPRESS", "SHOWPASS", "KEEPBINARY", "LANGUAGE",
	"MYNAME", "TEXTMODE", "TMP", "TZFIX", "VERBOSE", "BAKRING",
	"ARMORLINES", "COMPLETES_NEEDED", "MARGINALS_NEEDED", "PAGER",
	"CERT_DEPTH", "CHARSET", "CLEARSIG", "ENCRYPTTOSELF", 
	"INTERACTIVE", "PUBRING", "SECRING", "RANDSEED",
	"COMMENT", "AUTOSIGN", 
        "LEGAL_KLUDGE",
#ifdef MACTC5
	"FILE_TYPES", "WIPE_WARNING", "RECYCLE_PASSWORDS", "MULTIPLE_RECIPIENTS",
#endif
	/* command line only */
	"BATCHMODE", "FORCE", "NOMANUAL", "MAKERANDOM"
	};

static INPUT_TYPE intrinsicType[] = {
	BOOL, BOOL, BOOL, BOOL, STRING,
	STRING, BOOL, STRING, NUMERIC, NUMERIC, STRING,
	NUMERIC, NUMERIC, NUMERIC, STRING,
	NUMERIC, STRING, BOOL, BOOL,
	BOOL, STRING, STRING, STRING,
	STRING, BOOL,
        BOOL,
#ifdef MACTC5
	BOOL, BOOL, BOOL, BOOL,
#endif
	/* command line only */
	BOOL, BOOL, BOOL, NUMERIC
	};

/* Possible settings for variables */

#define NO_SETTINGS			2

static char *settings[] = { "OFF", "ON" };

/* Search a list of keywords for a match */

static int lookup( char *key, int keyLength, char *keyWords[], int range )
{
	int index, position = 0, noMatches = 0;

	strncpy( optstr, key, keyLength );
	optstr[ keyLength ] = '\0';

	/* Make the search case insensitive */
	for( index = 0; index < keyLength; index++ )
		key[ index ] = to_upper( key[ index ] );

	for( index = 0; index < range; index++ )
		if( !strncmp( key, keyWords[ index ], keyLength ) )
			{
			if( strlen( keyWords[ index ] ) == keyLength )
				return index;	/* exact match */
			position = index;
			noMatches++;
			}

	switch( noMatches )
		{
		case 0:
			fprintf( stderr, "%s: unknown keyword: \"%s\"\n",
					 errtag, optstr );
			break;
		case 1:
			return( position );	/* Match succeeded */
		default:
			fprintf( stderr, "%s: \"%s\" is ambiguous\n",
					 errtag, optstr );
		}
	return ERROR;
}

/* Extract a token from a buffer */

static int extractToken( char *buffer, int *endIndex, int *length )
{
	int index = 0, tokenStart;
	char ch;

	/* Skip whitespace */
	for( ch = buffer[ index ]; ch && ( ch == ' ' || ch == '\t' );
		 ch = buffer[ index ] )
		index++;
	tokenStart = index;

	/* Find end of setting */
	while( index < LINEBUF_SIZE && ( ch = buffer[ index ] ) != '\0'
		   && ch != ' ' && ch != '\t' )
		index++;
	*endIndex += index;
	*length = index - tokenStart;

	/* Return start position of token in buffer */
	return tokenStart;
}

/* Get a string constant */

static int getaString( char *buffer, int *endIndex )
	{
	boolean noQuote = FALSE;
	int stringIndex = 0, bufferIndex = 1;
	char ch = *buffer;

	/* Skip whitespace */
	while( ch && ( ch == ' ' || ch == '\t' ) )
		ch = buffer[ bufferIndex++ ];

	/* Check for non-string */
	if( ch != '\"' )
		{
		*endIndex += bufferIndex;

		/* Check for special case of null string */
		if( !ch )
			{
			*str = '\0';
			return OK;
			}

		/* Use nasty non-rigorous string format */
		noQuote = TRUE;
		}

	/* Get first char of string */
	if( !noQuote )
		ch = buffer[ bufferIndex++ ];

	/* Get string into string */
	while( ch && ch != '\"' )
		{
		/* Exit on '#' if using non-rigorous format */
		if( noQuote && ch == '#' )
			break;

		str[ stringIndex++ ] = ch;
		ch = buffer[ bufferIndex++ ];
		}

	/* If using the non-rigorous format, stomp trailing spaces */
	if( noQuote )
		while( stringIndex > 0 && str[ stringIndex - 1 ] == ' ' )
			stringIndex--;

	str[ stringIndex++ ] = '\0';
	*endIndex += bufferIndex;

	/* Check for missing string terminator */
	if( ch != '\"' && !noQuote )
		{
		if( line )
			fprintf( stderr, "%s: unterminated string in line %d\n",
					 errtag, line );
		else
			fprintf( stderr, "unterminated string: '\"%s'\n", str );
		hasError = TRUE;
		errCount++;
		return ERROR;
		}

	return OK;
}

/* Get an assignment to an intrinsic */

static int getAssignment( char *buffer, int *endIndex, INPUT_TYPE settingType )
{
	int settingIndex = 0, length;
	long longval;
	char *p;

	buffer += extractToken( buffer, endIndex, &length );

	/* Check for an assignment operator */
	if( *buffer != '=' )
		{
		if( line )
			fprintf( stderr, "%s: expected '=' in line %d\n",
					 errtag, line );
		else
			fprintf( stderr, "%s: expected '=' after \"%s\"\n",
					 errtag, optstr);
		hasError = TRUE;
		errCount++;
		return ERROR;
		}
	buffer++;	/* Skip '=' */

	buffer += extractToken( buffer, endIndex, &length );

	switch( settingType )
		{
		case BOOL:
			/* Check for known intrinsic - really more general
			   than just checking for TRUE or FALSE */
			settingIndex = lookup( buffer, length, settings,
			                       NO_SETTINGS );
			if( settingIndex == ERROR )
				{
				hasError = TRUE;
				errCount++;
				return ERROR;
				}

			value = ( settingIndex == 0 ) ? FALSE : TRUE;
			break;

		case STRING:
			/* Get a string */
			getaString( buffer, &length );
			break;

		case NUMERIC:
			longval = strtol(buffer, &p, 0);
			if (p == buffer+length &&
			    longval <= INT_MAX && longval >= INT_MIN) {
				value = (int)longval;
				break;
			}
			if( line )
				fprintf( stderr,
				  "%s: numeric argument expected in line %d\n",
						 errtag, line );
			else
				fprintf( stderr,
				   "%s: numeric argument required for \"%s\"\n",
						 errtag, optstr);
			hasError = TRUE;
			errCount++;
			return ERROR;
		}

	return settingIndex;
}

/* Process an assignment */

static void processAssignment( int intrinsicIndex )
	{
	if( !hasError )
		switch( intrinsicIndex )
			{
			case ARMOR:
				emit_radix_64 = value;
				break;

			case ARMORLINES:
				pem_lines = value;
				break;

			case AUTOSIGN:
				sign_new_userids = value;
				break;

			case BAKRING:
				strcpy( floppyring, str );
				break;

			case BATCHMODE:
				batchmode = value;
				break;

			case CERT_DEPTH:
				max_cert_depth = value;
				if( max_cert_depth < MIN_CERT_DEPTH )
					max_cert_depth = MIN_CERT_DEPTH;
				if( max_cert_depth > MAX_CERT_DEPTH )
					max_cert_depth = MAX_CERT_DEPTH;
				break;

			case CHARSET:
				strncpy( charset, str, 16 );
				break;

			case CLEARSIG:
				clear_signatures = value;
				break;

			case COMMENT:
				strcpy( globalCommentString, str );
				break;

			case COMPLETES_NEEDED:
				compl_min = value;
				/* Keep within range */
				if( compl_min < MIN_COMPLETE )
					compl_min = MIN_COMPLETE;
				if( compl_min > MAX_COMPLETE )
					compl_min = MAX_COMPLETE;
				break;

			case COMPRESS:
				compress_enabled = value;
				break;

			case FORCE:
				force_flag = value;
				break;

			case INTERACTIVE:
				interactive_add = value;
				break;

			case KEEPBINARY:
				keepctx = value;
				break;

			case LANGUAGE:
				strncpy( language, str, 15 );
				break;

			case LEGAL_KLUDGE:
				if (!value)
#ifdef USA
                                        fprintf(stdout,
LANG("The legal_kludge cannot be disabled in US version.\n"));
#else
					version_byte = VERSION_BYTE_OLD;
#endif
				break;

			case MAKERANDOM:
				makerandom = value;
				break;
#ifdef MACTC5				
			case FILE_TYPES:
				use_ftypes = value;
				break;
			
			case WIPE_WARNING:
				wipe_warning = value;
				break;
			
			case RECYCLE_PASSWD:
				recycle_passwd = value;
				break;

			case MULTIPLE_RECIPIENTS:
				fprintf(stdout, LANG("The multiple_recipients flag is unnecessary in this \
version of MacPGP.\
\nPlease remove this entry from your configuration file.\n"));
				break;
#endif

			case MARGINALS_NEEDED:
				marg_min = value;
				/* Keep within range */
				if( marg_min < MIN_MARGINALS )
					marg_min = MIN_MARGINALS;
				break;

			case MYNAME:
				strcpy( my_name, str );
#ifdef EBCDIC
    CONVERT_TO_CANONICAL_CHARSET(my_name);
#endif
				break;

			case NOMANUAL:
				nomanual = value;
				break;

			case PAGER:
				strcpy( pager, str );
				break;

			case PUBRING:
				strcpy( globalPubringName, str );
				break;

			case RANDSEED:
				strcpy( globalRandseedName, str );
				break;

			case SECRING:
				strcpy( globalSecringName, str );
				break;

			case SELF_ENCRYPT:
				encrypt_to_self = value;
				break;

			case SHOWPASS:
				showpass = value;
				break;

			case TEXTMODE:
				if( value )
					literal_mode = MODE_TEXT;
				else
					literal_mode = MODE_BINARY;
				break;

			case TMP:
				/* directory pathname to store temp files */
				settmpdir( str );
				break;

			case TZFIX:
				/* How many hours to add to time() to get GMT.
				   We just compute the seconds from hours to
				   get the GMT shift */
				timeshift = 3600L * ( long ) value;
				break;

			case VERBOSE:
				if( value < 1 )
					{
					quietmode = TRUE;
					verbose = FALSE;
					}
				else
					if( value == 1 )
						{
						quietmode = FALSE;
						verbose = FALSE;
						}
					else
						{
						/* Value > 1 */
						quietmode = FALSE;
						verbose = TRUE;
						}
				break;

			}
}

/* Process an option on a line by itself.  This expects options which are
   taken from the command-line, and is less finicky about errors than the
   config-file version */

int processConfigLine( char *option )
{
	int index, intrinsicIndex;
	char ch;

	/* Give it a pseudo-linenumber of 0 */
	line = 0;

	errtag = "pgp";
	errCount = 0;
	for( index = 0;
		 index < LINEBUF_SIZE && ( ch = option[ index ] ) != '\0' &&
				ch != ' ' && ch != '\t' && ch != '=';
		 index++ );
	if( ( intrinsicIndex = lookup( ( char * ) option, index, intrinsics,
				      NO_INTRINSICS ) ) == ERROR )
		return -1;
	if( option[ index ] == '\0' && intrinsicType[ intrinsicIndex ] == BOOL)
		{
		/* Boolean option, no '=' means TRUE */
		value = TRUE;
		processAssignment( intrinsicIndex );
		}
	else
		/* Get the value to set to, either as a string, a numeric
		   value, or a boolean flag */
		if( getAssignment( ( char * ) option + index,
			   &index, intrinsicType[ intrinsicIndex ] ) != ERROR )
			processAssignment( intrinsicIndex );

	return errCount ? -1 : 0;
}

/* Process a configuration file */

int processConfigFile( char *configFileName )
{
	FILE *configFilePtr;
	int ch = 0, theChar;
	int errType, errPos = 0, lineBufCount, intrinsicIndex;
	int index;
	char inBuffer[ LINEBUF_SIZE ];

	line = 1;
	errCount = 0;
	errtag = file_tail( configFileName );

	if( ( configFilePtr = fopen( configFileName, FOPRTXT ) ) == NULL )
		{
		fprintf( stderr, "Cannot open configuration file %s\n",
				 configFileName );
		return OK;	/* Treat it as if it were an empty file */
		}

	/* Process each line in the configFile */
	while( ch != EOF )
		{
		/* Skip whitespace */
		while( ( ( ch = getc( configFilePtr ) ) == ' ' || ch == '\t' )
		      && ch != EOF )
			;

		/* Get a line into the inBuffer */
		hasError = FALSE;
		lineBufCount = 0;
		errType = NO_ERROR;
		while( ch != '\r' && ch != '\n' && ch != CPM_EOF && ch != EOF )
			{
			/* Check for an illegal char in the data */
#ifdef EBCDIC
			if( iscntrl(ch) && !isspace(ch) && ch != EOF )
#else
			if( ( ch < ' ' || ch > '~' ) &&
				  ch != '\r' && ch != '\n' &&
				  ch != ' ' && ch != '\t' && ch != CPM_EOF &&
				  ch != EOF )
#endif
				{
				if( errType == NO_ERROR )
					/* Save pos of first illegal char */
					errPos = lineBufCount;
				errType = ILLEGAL_CHAR_ERROR;
				}

			/* Make sure the path is of the correct length.  Note
			   that the code is ordered so that a LINELENGTH_ERROR
			   takes precedence over an ILLEGAL_CHAR_ERROR */
			if( lineBufCount > LINEBUF_SIZE )
				errType = LINELENGTH_ERROR;
			else
				inBuffer[ lineBufCount++ ] = ch;

			if( ( ch = getc( configFilePtr ) ) == '#' )
				{
				/* Skip comment section and trailing
				   whitespace */
				while( ch != '\r' && ch != '\n' &&
					   ch != CPM_EOF && ch != EOF )
				  ch = getc( configFilePtr );
				break;
				}
			}

		/* Skip trailing whitespace and add der terminador */
		while( lineBufCount &&
		       ( ( theChar = inBuffer[ lineBufCount - 1 ] ) == ' ' ||
			   theChar == '\t' ) )
		  lineBufCount--;
		inBuffer[ lineBufCount ] = '\0';

		/* Process the line unless its a blank or comment line */
		if( lineBufCount && *inBuffer != '#' )
			{
			switch( errType )
				{
				case LINELENGTH_ERROR:
					fprintf( stderr,
					    "%s: line '%.30s...' too long\n",
							 errtag, inBuffer );
					errCount++;
					break;

				case ILLEGAL_CHAR_ERROR:
					fprintf( stderr, "> %s\n  ", inBuffer );
					fprintf( stderr, "%*s^\n", errPos, "" );
					fprintf( stderr,
				    "%s: bad character in command on line %d\n",
							 errtag, line );
					errCount++;
					break;

				default:
					for( index = 0;
					     index < LINEBUF_SIZE &&
					     ( ch = inBuffer[ index ] ) != '\0'
					     && ch != ' ' && ch != '\t'
					     && ch != '=';
					     index++ )
						/*Do nothing*/ ;

					/* Try and find the intrinsic.  We
					   don't treat unknown intrinsics as
					   an error to allow older versions to
					   be used with new config files */
					intrinsicIndex = lookup(inBuffer,
						index, intrinsics,
						CONFIG_INTRINSICS );
				
					if( intrinsicIndex == ERROR )
						break;

					/* Get the value to set to, either as
					   a string, a numeric value, or a
					   boolean flag */
					getAssignment( inBuffer + index, &index,
					     intrinsicType[ intrinsicIndex ] );
					processAssignment( intrinsicIndex );
					break;
				}
			}

		/* Handle special-case of ^Z if configFile came off an
		   MSDOS system */
		if( ch == CPM_EOF )
			ch = EOF;

		/* Exit if there are too many errors */
		if( errCount >= MAX_ERRORS )
			break;

		line++;
		}

	fclose( configFilePtr );

	/* Exit if there were errors */
	if( errCount )
		{
		fprintf( stderr, "%s: %s%d error(s) detected\n\n",
				 configFileName, ( errCount >= MAX_ERRORS ) ?
				 "Maximum level of " : "", errCount );
		return ERROR;
		}

	return OK;
}
