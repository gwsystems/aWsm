#include <config.h>
/* $Id: def_pars.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
 */
char *def_pars_id = "$Id: def_pars.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $";
#include <stdio.h>
#include <useconfig.h>
#include <math.h>
#include "proto.h"
#include "getargs.h"
#include "nsynth.h"
#include "hplay.h"

klatt_global_t klatt_global;

/* default values for pars array from .doc file */
klatt_frame_t def_pars =
{
#include "pars.def"
};

#ifdef USE_RC_FILE

#define RC_FILE "/.sayrc";

/* Based on an idea by 

   John Cartmill   -  cartmill@wisconsin.nrlssc.navy.mil 

   The ability to read the user's  .sayrc file 
   ( which has the same format as  the pars.def file) 

   Not enabled by default as I don't have one other than 
   pars.def! to test it on and it does not handle comments  

 */

static void
read_rc_file(void)
{
 char *home = getenv("HOME");
 if (home)
  {
   char *path = malloc(strlen(home) + strlen(RC_FILE) + 1);
   if (path)
    {
     FILE *f = fopen(strcat(strcpy(path, home), RC_FILE), "r");
     if (f)
      {
       char string[256];
       int i;
       union
        {
         klatt_t def_pars;
         long array[NPAR];
        }
       pars;
       for (i = 0; i < NPAR; ++i)
        {
         fgets(string, sizeof(string), infp);
         value = atoi(string);
         pars.array[i] = value;
        }
       def_pars = pars.def_pars;
       fclose(f);
      }
     free(path);
    }
  }
}

#endif

int
init_synth(argc, argv)
int argc;
char *argv[];
{
 double mSec_per_frame = 10;
 int impulse = 0;
 int casc = 0;
 klatt_global.samrate = samp_rate;
 klatt_global.quiet_flag = TRUE;
 klatt_global.glsource = NATURAL;
 klatt_global.f0_flutter = 0;

#ifdef USE_RC_FILE
 read_rc_file();
#endif

 argc = getargs("Synth paramters",argc, argv,
                "q", NULL, &klatt_global.quiet_flag, "Quiet - minimal messages",
                "I", NULL, &impulse,                 "Impulse glottal source",
                "c", "%d", &casc,                    "Number cascade formants",
                "F", "%d", &klatt_global.f0_flutter, "F0 flutter",
                "f", "%lg", &mSec_per_frame,         "mSec per frame",
                "t", "%d", &def_pars.TLTdb,          "Tilt dB",
                "x", "%d", &def_pars.F0hz10,         "Base F0 in 0.1Hz",
                NULL);

 if (casc > 0)
  {
   klatt_global.synthesis_model = CASCADE_PARALLEL;
   klatt_global.nfcascade = casc;
  }
 else
  klatt_global.synthesis_model = ALL_PARALLEL;

 if (impulse)
  klatt_global.glsource = IMPULSIVE;

 klatt_global.nspfr = (klatt_global.samrate * mSec_per_frame) / 1000;
#ifdef HAVE_NONSTDARITH
 /* turn off strict IEEE compliance in favour of speed */
 nonstandard_arithmetic();
#endif
 return argc;
}
