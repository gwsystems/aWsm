#ifndef CONFIGP_H
#define CONFIGP_H

/* The types of input we can expect */

typedef enum { BOOL, NUMERIC, STRING } INPUT_TYPE;

extern int processConfigLine( char *option );
extern int processConfigFile( char *configFileName );

#endif	/* ifndef CONFIGP_H */
