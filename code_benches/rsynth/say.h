/* $Id: say.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
extern unsigned xlate_string PROTO((char *string,darray_ptr phone));
extern void say_string PROTO((char *s));
extern char *concat_args PROTO((int argc,char *argv[]));
extern int suspect_word PROTO((char *s,int n));
extern void say_phones PROTO((char *phone,int len, int verbose));
extern unsigned spell_out PROTO((char *word,int n,darray_ptr phone));
extern unsigned xlate_ordinal PROTO((long int value,darray_ptr phone));
extern unsigned xlate_cardinal PROTO((long int value,darray_ptr phone));

