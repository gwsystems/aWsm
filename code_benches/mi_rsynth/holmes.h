/* $Id: holmes.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
extern unsigned holmes PROTO((unsigned nelm,unsigned char *elm,unsigned nsamp, short *samp_base));
extern int init_holmes PROTO((int argc,char *argv[]));
extern void term_holmes PROTO((void));
extern int speed;
