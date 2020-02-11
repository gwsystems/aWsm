/* $Id: dict.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
extern void *dict;
extern unsigned char *dict_find PROTO((char *s, unsigned n));
extern int dict_init PROTO((int argc,char *argv[]));
extern void dict_term PROTO((void));
extern char **dialect;

