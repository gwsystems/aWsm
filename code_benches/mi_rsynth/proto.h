/* $Id: proto.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
#ifndef PROTO
#if defined (USE_PROTOTYPES) ? USE_PROTOTYPES : defined (__STDC__)
#define PROTO(ARGS) ARGS
#ifndef NOPROTOFLOAT
#define Float float
#else
#define Float double
#endif
#else
#define PROTO(ARGS) ()
#define Float double
#endif

#ifndef __GNUC__
#define inline
#endif

#endif /* PROTO */
