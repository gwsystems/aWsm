/* $Id: text.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
extern int isvowel PROTO((int chr));
extern int isconsonant PROTO((int chr));

typedef void (*out_p) PROTO((void *arg,char *s));
extern int NRL PROTO((char *s,unsigned n,darray_ptr phone));

