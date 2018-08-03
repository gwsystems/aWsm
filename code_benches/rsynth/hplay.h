/* $Id: hplay.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
extern char *program;
extern long samp_rate;
extern int audio_init PROTO((int argc, char *argv[]));
extern void audio_term PROTO((void));
extern void audio_play PROTO((int n, short *data));

