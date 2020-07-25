/*
 *	language.h
 *	Include file for PGP foreign language translation facility
 */

/*
 * Strings with LANG() around them are found by automatic tools and put
 * into the special text file to be translated into foreign languages.
 * LANG () (note the space between 'G' and '(') should be used if there
 * is no string to be extracted (eg. prototype).
 */

extern char	*LANG (char *s);

/*
 * Use the dummy macro _LANG for strings that should be extracted, but
 * shouldn't be processed by the LANG function (eg. array initializers).
 */
#define _LANG(x)	x

extern char language[]; /* language selector prefix for string file */
