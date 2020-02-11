#include <config.h>


/* $Id: nsynth.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
 */
char *nsynth_id = "$Id: nsynth.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $";

/* Copyright            1982                    by Dennis H. Klatt

 *      Klatt synthesizer
 *         Modified version of synthesizer described in
 *         J. Acoust. Soc. Am., Mar. 1980. -- new voicing
 *         source.
 *
 * Edit history
 * 000001 10-Mar-83 DK  Initial creation.
 * 000002  5-May-83 DK  Fix bug in operation of parallel F1
 * 000003  7-Jul-83 DK  Allow parallel B1 to vary, and if ALL_PARALLEL,
 *                      also allow B2 and B3 to vary
 * 000004 26-Jul-83 DK  Get rid of mulsh, use short for VAX
 * 000005 24-Oct-83 DK  Split off parwavtab.c, change short to int
 * 000006 16-Nov-83 DK  Make samrate a variable, use exp(), cos() rand()
 * 000007 17-Nov-83 DK  Convert to float, remove  cpsw, add set outsl
 * 000008 28-Nov-83 DK  Add simple impulsive glottal source option
 * 000009  7-Dec-83 DK  Use spkrdef[7] to select impulse or natural voicing
 *                       and update cascade F1,..,F6 at update times
 * 000010 19-Dec-83 DK  Add subroutine no_rad_char() to get rid of rad char
 * 000011 28-Jan-84 DK  Allow up to 8 formants in cascade branch F7 fixed
 *                       at 6.5 kHz, F8 fixed at 7.5 kHz
 * 000012 14-Feb-84 DK  Fix bug in 'os' options so os>12 works
 * 000013 17-May-84 DK  Add G0 code
 * 000014 12-Mar-85 DHW modify for Haskins environment
 * 000015 11-Jul-87 LG  modificiations for PC
 * 000016 20-Apr-91 ATS Modified for SPARCSTATION
 */

#include <useconfig.h>
#include <stdio.h>
#include <math.h>
#include "proto.h"
#include "nsynth.h"
#ifndef PI
#ifndef M_PI                      /* <math.h> */
#define PI               3.1415927
#else /* M_PI */
#define PI               M_PI
#endif /* M_PI */
#endif

#ifdef __STDC__
#define ONE 1.0F
#else
#define ONE 1.0
#endif

typedef struct
 {
  char *name;
  float a;
  float b;
  float c;
  float p1;
  float p2;
 }
resonator_t, *resonator_ptr;

/* Various global variables */

int time_count = 0;
static warnsw;                    /* JPI added for f0 flutter */

/* COUNTERS */

static long nper;                 /* Current loc in voicing period   40000 samp/s */

/* COUNTER LIMITS */

static long T0;                   /* Fundamental period in output samples times 4 */
static long nopen;                /* Number of samples in open phase of period  */
static long nmod;                 /* Position in period to begin noise amp. modul */

/* Variables that have to stick around for awhile, and thus locals
   are not appropriate 
 */

/* Incoming parameter Variables which need to be updated synchronously  */

static long F0hz10;               /* Voicing fund freq in Hz  */
static long AVdb;                 /* Amp of voicing in dB,    0 to   70  */
static long Kskew;                /* Skewness of alternate periods,0 to   40  */

/* Various amplitude variables used in main loop */

static float amp_voice;           /* AVdb converted to linear gain  */
static float amp_bypas;           /* AB converted to linear gain  */
static float par_amp_voice;       /* AVpdb converted to linear gain  */
static float amp_aspir;           /* AP converted to linear gain  */
static float amp_frica;           /* AF converted to linear gain  */
static float amp_breth;           /* ATURB converted to linear gain  */

/* State variables of sound sources */

static long skew;                 /* Alternating jitter, in half-period units  */

static float natglot_a;           /* Makes waveshape of glottal pulse when open  */
static float natglot_b;           /* Makes waveshape of glottal pulse when open  */
static float vwave;               /* Ditto, but before multiplication by AVdb  */
static float vlast;               /* Previous output of voice  */
static float nlast;               /* Previous output of random number generator  */
static float glotlast;            /* Previous value of glotout  */
static float decay;               /* TLTdb converted to exponential time const  */
static float onemd;               /* in voicing one-pole low-pass filter  */
static float minus_pi_t;          /* func. of sample rate */
static float two_pi_t;            /* func. of sample rate */


/* INTERNAL MEMORY FOR DIGITAL RESONATORS AND ANTIRESONATOR  */

static resonator_t rnpp =
{"parallel nasal pole"};
static resonator_t r1p =
{"parallel 1st formant"};
static resonator_t r2p =
{"parallel 2nd formant"};
static resonator_t r3p =
{"parallel 3rd formant"};
static resonator_t r4p =
{"parallel 4th formant"};
static resonator_t r5p =
{"parallel 5th formant"};
static resonator_t r6p =
{"parallel 6th formant"};
static resonator_t r1c =
{"cascade 1st formant"};
static resonator_t r2c =
{"cascade 2nd formant"};
static resonator_t r3c =
{"cascade 3rd formant"};
static resonator_t r4c =
{"cascade 4th formant"};
static resonator_t r5c =
{"cascade 5th formant"};
static resonator_t r6c =
{"cascade 6th formant"};
static resonator_t r7c =
{"cascade 7th formant"};
static resonator_t r8c =
{"cascade 8th formant"};
static resonator_t rnpc =
{"cascade nasal pole"};
static resonator_t rnz =
{"cascade nasal zero"};
static resonator_t rgl =
{"crit-damped glot low-pass filter"};
static resonator_t rlp =
{"downsamp low-pass filter"};
static resonator_t rout =
{"output low-pass"};

/*
 * Constant natglot[] controls shape of glottal pulse as a function
 * of desired duration of open phase N0
 * (Note that N0 is specified in terms of 40,000 samples/sec of speech)
 *
 *    Assume voicing waveform V(t) has form: k1 t**2 - k2 t**3
 *
 *    If the radiation characterivative, a temporal derivative
 *      is folded in, and we go from continuous time to discrete
 *      integers n:  dV/dt = vwave[n]
 *                         = sum over i=1,2,...,n of { a - (i * b) }
 *                         = a n  -  b/2 n**2
 *
 *      where the  constants a and b control the detailed shape
 *      and amplitude of the voicing waveform over the open
 *      potion of the voicing cycle "nopen".
 *
 *    Let integral of dV/dt have no net dc flow --> a = (b * nopen) / 3
 *
 *    Let maximum of dUg(n)/dn be constant --> b = gain / (nopen * nopen)
 *      meaning as nopen gets bigger, V has bigger peak proportional to n
 *
 *    Thus, to generate the table below for 40 <= nopen <= 263:
 *
 *      natglot[nopen - 40] = 1920000 / (nopen * nopen)
 */
static const short natglot[224] =
{
 1200, 1142, 1088, 1038, 991, 948, 907, 869, 833, 799,
 768, 738, 710, 683, 658, 634, 612, 590, 570, 551,
 533, 515, 499, 483, 468, 454, 440, 427, 415, 403,
 391, 380, 370, 360, 350, 341, 332, 323, 315, 307,
 300, 292, 285, 278, 272, 265, 259, 253, 247, 242,
 237, 231, 226, 221, 217, 212, 208, 204, 199, 195,
 192, 188, 184, 180, 177, 174, 170, 167, 164, 161,
 158, 155, 153, 150, 147, 145, 142, 140, 137, 135,
 133, 131, 128, 126, 124, 122, 120, 119, 117, 115,
 113, 111, 110, 108, 106, 105, 103, 102, 100, 99,
 97, 96, 95, 93, 92, 91, 90, 88, 87, 86,
 85, 84, 83, 82, 80, 79, 78, 77, 76, 75,
 75, 74, 73, 72, 71, 70, 69, 68, 68, 67,
 66, 65, 64, 64, 63, 62, 61, 61, 60, 59,
 59, 58, 57, 57, 56, 56, 55, 55, 54, 54,
 53, 53, 52, 52, 51, 51, 50, 50, 49, 49,
 48, 48, 47, 47, 46, 46, 45, 45, 44, 44,
 43, 43, 42, 42, 41, 41, 41, 41, 40, 40,
 39, 39, 38, 38, 38, 38, 37, 37, 36, 36,
 36, 36, 35, 35, 35, 35, 34, 34, 33, 33,
 33, 33, 32, 32, 32, 32, 31, 31, 31, 31,
 30, 30, 30, 30, 29, 29, 29, 29, 28, 28,
 28, 28, 27, 27
};

/*
 * Convertion table, db to linear, 87 dB --> 32767
 *                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
 *                                 ...
 *                                 81 dB --> 16384 (6 dB down = 0.5)
 *                                 ...
 *                                  0 dB -->     0
 *
 * The just noticeable difference for a change in intensity of a vowel
 *   is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
 *   steps.
 */

static const float amptable[88] =
{
 0.0, 0.0, 0.0, 0.0, 0.0,
 0.0, 0.0, 0.0, 0.0, 0.0,
 0.0, 0.0, 0.0, 6.0, 7.0,
 8.0, 9.0, 10.0, 11.0, 13.0,
 14.0, 16.0, 18.0, 20.0, 22.0,
 25.0, 28.0, 32.0, 35.0, 40.0,
 45.0, 51.0, 57.0, 64.0, 71.0,
 80.0, 90.0, 101.0, 114.0, 128.0,
 142.0, 159.0, 179.0, 202.0, 227.0,
 256.0, 284.0, 318.0, 359.0, 405.0,
 455.0, 512.0, 568.0, 638.0, 719.0,
 811.0, 911.0, 1024.0, 1137.0, 1276.0,
 1438.0, 1622.0, 1823.0, 2048.0, 2273.0,
 2552.0, 2875.0, 3244.0, 3645.0, 4096.0,
 4547.0, 5104.0, 5751.0, 6488.0, 7291.0,
 8192.0, 9093.0, 10207.0, 11502.0, 12976.0,
 14582.0, 16384.0, 18350.0, 20644.0, 23429.0,
 26214.0, 29491.0, 32767.0
};

const char *par_name[] =
{
 "F0hz10",
 "AVdb",
 "F1hz", "B1hz",
 "F2hz", "B2hz",
 "F3hz", "B3hz",
 "F4hz", "B4hz",
 "F5hz", "B5hz",
 "F6hz", "B6hz",
 "FNZhz", "BNZhz",
 "FNPhz", "BNPhz",
 "AP",
 "Kopen",
 "Aturb",
 "TLTdb",
 "AF",
 "Kskew",
 "A1", "B1phz",
 "A2", "B2phz",
 "A3", "B3phz",
 "A4", "B4phz",
 "A5", "B5phz",
 "A6", "B6phz",
 "ANP",
 "AB",
 "AVpdb",
 "Gain0"
};

static void flutter PROTO((klatt_global_ptr globals, klatt_frame_ptr pars));
static float resonator PROTO((resonator_ptr r, Float input));
static float antiresonator PROTO((resonator_ptr r, Float input));
static float impulsive_source PROTO((long nper));
static float natural_source PROTO((long nper));
static void setabc PROTO((long int f, long int bw, resonator_ptr rp));
static void setabcg PROTO((long int f, long int bw, resonator_ptr rp, Float gain));
static void setzeroabc PROTO((long int f, long int bw, resonator_ptr rp));
static float DBtoLIN PROTO((klatt_global_ptr globals, long int dB));
static float dBconvert PROTO((long int arg));
static void overload_warning PROTO((klatt_global_ptr globals, long int arg));
static short clip PROTO((klatt_global_ptr globals, Float input));
static void pitch_synch_par_reset PROTO((klatt_global_ptr globals,
                                         klatt_frame_ptr frame, long ns));
static void frame_init PROTO((klatt_global_ptr globals, klatt_frame_ptr frame));
void show_parms PROTO((klatt_global_ptr globals, int *pars));


void
show_parms(globals, pars)
klatt_global_ptr globals;
int *pars;
{
 int i;
 static int names;
 if ((names++ % 64) == 0)
  {
   for (i = 0; i < NPAR; i++)
    printf("%s ", par_name[i]);
   printf("\n");
  }
 for (i = 0; i < NPAR; i++)
  {
   printf("%*d ", (int) strlen(par_name[i]), pars[i]);
  }
 printf("\n");
}

/*
   function FLUTTER

   This function adds F0 flutter, as specified in:

   "Analysis, synthesis and perception of voice quality variations among
   female and male talkers" D.H. Klatt and L.C. Klatt JASA 87(2) February 1990.
   Flutter is added by applying a quasi-random element constructed from three
   slowly varying sine waves.
 */
static void
flutter(globals, pars)
klatt_global_ptr globals;
klatt_frame_ptr pars;
{
 long original_f0 = pars->F0hz10 / 10;
 double fla = (double) globals->f0_flutter / 50;
 double flb = (double) original_f0 / 100;
 double flc = sin(2 * PI * 12.7 * time_count);
 double fld = sin(2 * PI * 7.1 * time_count);
 double fle = sin(2 * PI * 4.7 * time_count);
 double delta_f0 = fla * flb * (flc + fld + fle) * 10;
 F0hz10 += (long) delta_f0;
}

static float
impulsive_source(nper)
long nper;
{
 static float doublet[] =
 {0., 13000000., -13000000.};
 if (nper < 3)
  {
   vwave = doublet[nper];
  }
 else
  {
   vwave = 0.0;
  }
 /* Low-pass filter the differenciated impulse with a critically-damped
    second-order filter, time constant proportional to Kopen */
 return resonator(&rgl, vwave);
}


/* Vwave is the differentiated glottal flow waveform, there is a weak
   spectral zero around 800 Hz, magic constants a,b reset pitch-synch
 */

static float
natural_source(nper)
long nper;
{
 float lgtemp;
 /* See if glottis open */
 if (nper < nopen)
  {
   natglot_a -= natglot_b;
   vwave += natglot_a;
   lgtemp = vwave * 0.028;        /* function of samp_rate ? */
   return (lgtemp);
  }
 else
  {
   /* Glottis closed */
   vwave = 0.0;
   return (0.0);
  }
}

/*----------------------------------------------------------------------------*/
/* Convert formant freqencies and bandwidth into
   resonator difference equation coefficents
 */
static void
setabc(f, bw, rp)
long int f;                       /* Frequency of resonator in Hz  */
long int bw;                      /* Bandwidth of resonator in Hz  */
resonator_ptr rp;                 /* Are output coefficients  */
{
 double arg = minus_pi_t * bw;
 float r = exp(arg);              /* Let r  =  exp(-pi bw t) */
 rp->c = -(r * r);                /* Let c  =  -r**2 */
 arg = two_pi_t * f;
 rp->b = r * cos(arg) * 2.0;      /* Let b = r * 2*cos(2 pi f t) */
 rp->a = 1.0 - rp->b - rp->c;     /* Let a = 1.0 - b - c */
}

/* Convienience function for setting parallel resonators with gain */
static void
setabcg(f, bw, rp, gain)
long int f;                       /* Frequency of resonator in Hz  */
long int bw;                      /* Bandwidth of resonator in Hz  */
resonator_ptr rp;                 /* Are output coefficients  */
Float gain;
{
 setabc(f, bw, rp);
 rp->a *= gain;
}

/* Convert formant freqencies and bandwidth into
 *      anti-resonator difference equation constants
 */
static void
setzeroabc(f, bw, rp)
long int f;                       /* Frequency of resonator in Hz  */
long int bw;                      /* Bandwidth of resonator in Hz  */
resonator_ptr rp;                 /* Are output coefficients  */
{
 setabc(f, bw, rp);               /* First compute ordinary resonator coefficients */
 /* Now convert to antiresonator coefficients */
 rp->a = 1.0 / rp->a;             /* a'=  1/a */
 rp->b *= -rp->a;                 /* b'= -b/a */
 rp->c *= -rp->a;                 /* c'= -c/a */
}

/*----------------------------------------------------------------------------*/


/* Convert from decibels to a linear scale factor */
static float
DBtoLIN(globals, dB)
klatt_global_ptr globals;
long int dB;
{
 /* Check limits or argument (can be removed in final product) */
 if (dB < 0)
  dB = 0;
 else if (dB >= 88)
  {
   if (!globals->quiet_flag)
    printf("Try to compute amptable[%ld]\n", dB);
   dB = 87;
  }
 return amptable[dB] * 0.001;
}

/* WHAT WERE THESE FOR ? */
#define ACOEF           0.005
#define BCOEF           (1.0 - ACOEF)	/* Slight decay to remove dc */

static float
dBconvert(arg)
long int arg;
{
 return 20.0 * log10((double) arg / 32767.0);
}

static void
overload_warning(globals, arg)
klatt_global_ptr globals;
long int arg;
{
 if (warnsw == 0)
  {
   warnsw++;
   if (!globals->quiet_flag)
    {
     printf("\n* * * WARNING: ");
     printf(" Signal at output of synthesizer (+%3.1f dB) exceeds 0 dB\n",
            dBconvert(arg));
    }
  }
}

/* Reset selected parameters pitch-synchronously */

static void
pitch_synch_par_reset(globals, frame, ns)
klatt_global_ptr globals;
klatt_frame_ptr frame;
long ns;
{
 long temp;
 float temp1;
 if (F0hz10 > 0)
  {
   T0 = (40 * globals->samrate) / F0hz10;
   /* Period in samp*4 */
   amp_voice = DBtoLIN(globals, AVdb);

   /* Duration of period before amplitude modulation */
   nmod = T0;
   if (AVdb > 0)
    {
     nmod >>= 1;
    }

   /* Breathiness of voicing waveform */

   amp_breth = DBtoLIN(globals, frame->Aturb) * 0.1;

   /* Set open phase of glottal period */
   /* where  40 <= open phase <= 263 */

   nopen = 4 * frame->Kopen;
   if ((globals->glsource == IMPULSIVE) && (nopen > 263))
    {
     nopen = 263;
    }

   if (nopen >= (T0 - 1))
    {
     nopen = T0 - 2;
     if (!globals->quiet_flag)
      {
       printf("Warning: glottal open period cannot exceed T0, truncated\n");
      }
    }

   if (nopen < 40)
    {
     nopen = 40;                  /* F0 max = 1000 Hz */
     if (!globals->quiet_flag)
      {
       printf("Warning: minimum glottal open period is 10 samples.\n");
       printf("truncated, nopen = %ld\n", nopen);
      }
    }

   /* Reset a & b, which determine shape of "natural" glottal waveform */

   natglot_b = natglot[nopen - 40];
   natglot_a = (natglot_b * nopen) * .333;

   /* Reset width of "impulsive" glottal pulse */

   temp = globals->samrate / nopen;
   setabc(0L, temp, &rgl);

   /* Make gain at F1 about constant */

   temp1 = nopen * .00833;
   rgl.a *= (temp1 * temp1);

   /* Truncate skewness so as not to exceed duration of closed phase
      of glottal period */

   temp = T0 - nopen;
   if (Kskew > temp)
    {
     if (!globals->quiet_flag)
      {
       printf("Kskew duration=%ld > glottal closed period=%ld, truncate\n",
              Kskew, T0 - nopen);
      }
     Kskew = temp;
    }
   if (skew >= 0)
    {
     skew = Kskew;                /* Reset skew to requested Kskew */
    }
   else
    {
     skew = -Kskew;
    }

   /* Add skewness to closed portion of voicing period */

   T0 = T0 + skew;
   skew = -skew;
  }
 else
  {
   T0 = 4;                        /* Default for f0 undefined */
   amp_voice = 0.0;
   nmod = T0;
   amp_breth = 0.0;
   natglot_a = 0.0;
   natglot_b = 0.0;
  }

 /* Reset these pars pitch synchronously or at update rate if f0=0 */

 if ((T0 != 4) || (ns == 0))
  {
   /* Set one-pole low-pass filter that tilts glottal source */
   decay = (0.033 * frame->TLTdb);	/* Function of samp_rate ? */
   if (decay > 0.0)
    {
     onemd = 1.0 - decay;
    }
   else
    {
     onemd = 1.0;
    }
  }
}

/* Get variable parameters from host computer,
   initially also get definition of fixed pars
 */

static void
frame_init(globals, frame)
klatt_global_ptr globals;
klatt_frame_ptr frame;
{
 long Gain0;                      /* Overall gain, 60 dB is unity  0 to   60  */
 float amp_parF1;                 /* A1 converted to linear gain  */
 float amp_parFN;                 /* ANP converted to linear gain  */
 float amp_parF2;                 /* A2 converted to linear gain  */
 float amp_parF3;                 /* A3 converted to linear gain  */
 float amp_parF4;                 /* A4 converted to linear gain  */
 float amp_parF5;                 /* A5 converted to linear gain  */
 float amp_parF6;                 /* A6 converted to linear gain  */

#if 0
 show_parms((int *) frame);
#endif

 /*
    Read  speech frame definition into temp store
    and move some parameters into active use immediately
    (voice-excited ones are updated pitch synchronously
    to avoid waveform glitches).
  */

 F0hz10 = frame->F0hz10;
 AVdb = frame->AVdb - 7;
 if (AVdb < 0)
  AVdb = 0;

 amp_aspir = DBtoLIN(globals, frame->ASP) * .05;
 amp_frica = DBtoLIN(globals, frame->AF) * 0.25;

 Kskew = frame->Kskew;
 par_amp_voice = DBtoLIN(globals, frame->AVpdb);

 /* Fudge factors (which comprehend affects of formants on each other?)
    with these in place ALL_PARALLEL should sound as close as 
    possible to CASCADE_PARALLEL.
    Possible problem feeding in Holmes's amplitudes given this.
  */
 amp_parF1 = DBtoLIN(globals, frame->A1) * 0.4;	/* -7.96 dB */
 amp_parF2 = DBtoLIN(globals, frame->A2) * 0.15;	/* -16.5 dB */
 amp_parF3 = DBtoLIN(globals, frame->A3) * 0.06;	/* -24.4 dB */
 amp_parF4 = DBtoLIN(globals, frame->A4) * 0.04;	/* -28.0 dB */
 amp_parF5 = DBtoLIN(globals, frame->A5) * 0.022;	/* -33.2 dB */
 amp_parF6 = DBtoLIN(globals, frame->A6) * 0.03;	/* -30.5 dB */
 amp_parFN = DBtoLIN(globals, frame->ANP) * 0.6;	/* -4.44 dB */
 amp_bypas = DBtoLIN(globals, frame->AB) * 0.05;	/* -26.0 db */

 if (globals->nfcascade >= 8)
  {
   /* Inside Nyquist rate ? */
   if (globals->samrate >= 16000)
    setabc(7500, 600, &r8c);
   else
    globals->nfcascade = 6;
  }

 if (globals->nfcascade >= 7)
  {
   /* Inside Nyquist rate ? */
   if (globals->samrate >= 16000)
    setabc(6500, 500, &r7c);
   else
    globals->nfcascade = 6;
  }

 /* Set coefficients of variable cascade resonators */

 if (globals->nfcascade >= 6)
  setabc(frame->F6hz, frame->B6hz, &r6c);

 if (globals->nfcascade >= 5)
  setabc(frame->F5hz, frame->B5hz, &r5c);

 setabc(frame->F4hz, frame->B4hz, &r4c);
 setabc(frame->F3hz, frame->B3hz, &r3c);
 setabc(frame->F2hz, frame->B2hz, &r2c);
 setabc(frame->F1hz, frame->B1hz, &r1c);

 /* Set coeficients of nasal resonator and zero antiresonator */
 setabc(frame->FNPhz, frame->BNPhz, &rnpc);
 setzeroabc(frame->FNZhz, frame->BNZhz, &rnz);

 /* Set coefficients of parallel resonators, and amplitude of outputs */
 setabcg(frame->F1hz, frame->B1phz, &r1p, amp_parF1);
 setabcg(frame->FNPhz, frame->BNPhz, &rnpp, amp_parFN);
 setabcg(frame->F2hz, frame->B2phz, &r2p, amp_parF2);
 setabcg(frame->F3hz, frame->B3phz, &r3p, amp_parF3);
 setabcg(frame->F4hz, frame->B4phz, &r4p, amp_parF4);
 setabcg(frame->F5hz, frame->B5phz, &r5p, amp_parF5);
 setabcg(frame->F6hz, frame->B6phz, &r6p, amp_parF6);


 /* fold overall gain into output resonator */
 Gain0 = frame->Gain0 - 3;
 if (Gain0 <= 0)
  Gain0 = 57;
 /* output low-pass filter - resonator with freq 0 and BW = globals->samrate
    Thus 3db point is globals->samrate/2 i.e. Nyquist limit.
    Only 3db down seems rather mild...
  */
 setabcg(0L, (long) globals->samrate, &rout, DBtoLIN(globals, Gain0));
}

static short
clip(globals, input)
klatt_global_ptr globals;
Float input;
{
 long temp = input;
 /* clip on boundaries of 16-bit word */
 if (temp < -32767)
  {
   overload_warning(globals, -temp);
   temp = -32767;
  }
 else if (temp > 32767)
  {
   overload_warning(globals, temp);
   temp = 32767;
  }
 return (temp);
}

/* Generic resonator function */
static float
resonator(r, input)
resonator_ptr r;
Float input;
{
 register float x = r->a * input + r->b * r->p1 + r->c * r->p2;
 r->p2 = r->p1;
 r->p1 = x;
 return x;
}

/* Generic anti-resonator function
   Same as resonator except that a,b,c need to be set with setzeroabc()
   and we save inputs in p1/p2 rather than outputs.
   There is currently only one of these - "rnz"
 */
/*  Output = (rnz.a * input) + (rnz.b * oldin1) + (rnz.c * oldin2) */

static float
antiresonator(r, input)
resonator_ptr r;
Float input;
{
 register float x = r->a * input + r->b * r->p1 + r->c * r->p2;
 r->p2 = r->p1;
 r->p1 = input;
 return x;
}

/*
   function PARWAV

   CONVERT FRAME OF PARAMETER DATA TO A WAVEFORM CHUNK
   Synthesize globals->nspfr samples of waveform and store in jwave[].
 */

void
parwave(globals, frame, jwave)
klatt_global_ptr globals;
klatt_frame_ptr frame;
short int *jwave;
{
 long ns;
 float out = 0.0;
 /* Output of cascade branch, also final output  */

 /* Initialize synthesizer and get specification for current speech
    frame from host microcomputer */

 frame_init(globals, frame);

 if (globals->f0_flutter != 0)
  {
   time_count++;                  /* used for f0 flutter */
   flutter(globals, frame);       /* add f0 flutter */
  }

 /* MAIN LOOP, for each output sample of current frame: */

 for (ns = 0; ns < globals->nspfr; ns++)
  {
   static unsigned long seed = 5; /* Fixed staring value */
   float noise;
   int n4;
   float sourc;                   /* Sound source if all-parallel config used  */
   float glotout;                 /* Output of glottal sound source  */
   float par_glotout;             /* Output of parallelglottal sound sourc  */
   float voice;                   /* Current sample of voicing waveform  */
   float frics;                   /* Frication sound source  */
   float aspiration;              /* Aspiration sound source  */
   long nrand;                    /* Varible used by random number generator  */

   /* Our own code like rand(), but portable
      whole upper 31 bits of seed random 
      assumes 32-bit unsigned arithmetic
      with untested code to handle larger.
    */
   seed = seed * 1664525 + 1;
   if (8 * sizeof(unsigned long) > 32)
         seed &= 0xFFFFFFFF;

   /* Shift top bits of seed up to top of long then back down to LS 14 bits */
   /* Assumes 8 bits per sizeof unit i.e. a "byte" */
   nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

   /* Tilt down noise spectrum by soft low-pass filter having
    *    a pole near the origin in the z-plane, i.e.
    *    output = input + (0.75 * lastoutput) */

   noise = nrand + (0.75 * nlast);	/* Function of samp_rate ? */
   nlast = noise;

   /* Amplitude modulate noise (reduce noise amplitude during
      second half of glottal period) if voicing simultaneously present
    */

   if (nper > nmod)
    {
     noise *= 0.5;
    }

   /* Compute frication noise */
   sourc = frics = amp_frica * noise;

   /* Compute voicing waveform : (run glottal source simulation at
      4 times normal sample rate to minimize quantization noise in 
      period of female voice)
    */

   for (n4 = 0; n4 < 4; n4++)
    {
     if (globals->glsource == IMPULSIVE)
      {
       /* Use impulsive glottal source */
       voice = impulsive_source(nper);
      }
     else
      {
       /* Or use a more-natural-shaped source waveform with excitation
          occurring both upon opening and upon closure, stronest at closure */
       voice = natural_source(nper);
      }

     /* Reset period when counter 'nper' reaches T0 */
     if (nper >= T0)
      {
       nper = 0;
       pitch_synch_par_reset(globals, frame, ns);
      }

     /* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
     /* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

     voice = resonator(&rlp, voice);	/* in=voice, out=voice */

     /* Increment counter that keeps track of 4*globals->samrate samples/sec */
     nper++;
    }

   /* Tilt spectrum of voicing source down by soft low-pass filtering, amount
      of tilt determined by TLTdb
    */
   voice = (voice * onemd) + (vlast * decay);
   vlast = voice;

   /* Add breathiness during glottal open phase */
   if (nper < nopen)
    {
     /* Amount of breathiness determined by parameter Aturb */
     /* Use nrand rather than noise because noise is low-passed */
     voice += amp_breth * nrand;
    }

   /* Set amplitude of voicing */
   glotout = amp_voice * voice;

   /* Compute aspiration amplitude and add to voicing source */
   aspiration = amp_aspir * noise;
   glotout += aspiration;

   par_glotout = glotout;

   if (globals->synthesis_model != ALL_PARALLEL)
    {
     /* Cascade vocal tract, excited by laryngeal sources.
        Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
      */
     float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
     float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

     /* Recoded from sequence of if's to use C's fall through switch
        semantics. May allow compiler to optimize
      */
     switch (globals->nfcascade)
      {
       case 8:
        casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
       case 7:
        casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
       case 6:
        casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
       case 5:
        casc_next_in = resonator(&r5c, casc_next_in);
       case 4:
        casc_next_in = resonator(&r4c, casc_next_in);
       case 3:
        casc_next_in = resonator(&r3c, casc_next_in);
       case 2:
        casc_next_in = resonator(&r2c, casc_next_in);
       case 1:
        out = resonator(&r1c, casc_next_in);
        break;
       default:
        out = 0.0;
      }
#if 0
     /* Excite parallel F1 and FNP by voicing waveform */
     /* Source is voicing plus aspiration */
     /* Add in phase, boost lows for nasalized */
     out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
#endif
    }
   else
    {
     /* Is ALL_PARALLEL */
     /* NIS - rsynth "hack"
        As Holmes' scheme is weak at nasals and (physically) nasal cavity
        is "back near glottis" feed glottal source through nasal resonators
        Don't think this is quite right, but improves things a bit
      */
     par_glotout = antiresonator(&rnz, par_glotout);
     par_glotout = resonator(&rnpc, par_glotout);
     /* And just use r1p NOT rnpp */
     out = resonator(&r1p, par_glotout);
     /* Sound sourc for other parallel resonators is frication
        plus first difference of voicing waveform.
      */
     sourc += (par_glotout - glotlast);
     glotlast = par_glotout;
    }

   /* Standard parallel vocal tract
      Formants F6,F5,F4,F3,F2, outputs added with alternating sign
    */
   out = resonator(&r6p, sourc) - out;
   out = resonator(&r5p, sourc) - out;
   out = resonator(&r4p, sourc) - out;
   out = resonator(&r3p, sourc) - out;
   out = resonator(&r2p, sourc) - out;

   out = amp_bypas * sourc - out;

   out = resonator(&rout, out);
   *jwave++ = clip(globals, out); /* Convert back to integer */
  }
}

void
parwave_init(globals)
klatt_global_ptr globals;
{
 long FLPhz = (950 * globals->samrate) / 10000;
 long BLPhz = (630 * globals->samrate) / 10000;

 minus_pi_t = -PI / globals->samrate;
 two_pi_t = -2.0 * minus_pi_t;

 setabc(FLPhz, BLPhz, &rlp);
 nper = 0;                        /* LG */
 T0 = 0;                          /* LG */

 rnpp.p1 = 0;                     /* parallel nasal pole  */
 rnpp.p2 = 0;

 r1p.p1 = 0;                      /* parallel 1st formant */
 r1p.p2 = 0;

 r2p.p1 = 0;                      /* parallel 2nd formant */
 r2p.p2 = 0;

 r3p.p1 = 0;                      /* parallel 3rd formant */
 r3p.p2 = 0;

 r4p.p1 = 0;                      /* parallel 4th formant */
 r4p.p2 = 0;

 r5p.p1 = 0;                      /* parallel 5th formant */
 r5p.p2 = 0;

 r6p.p1 = 0;                      /* parallel 6th formant */
 r6p.p2 = 0;

 r1c.p1 = 0;                      /* cascade 1st formant  */
 r1c.p2 = 0;

 r2c.p1 = 0;                      /* cascade 2nd formant  */
 r2c.p2 = 0;

 r3c.p1 = 0;                      /* cascade 3rd formant  */
 r3c.p2 = 0;

 r4c.p1 = 0;                      /* cascade 4th formant  */
 r4c.p2 = 0;

 r5c.p1 = 0;                      /* cascade 5th formant  */
 r5c.p2 = 0;

 r6c.p1 = 0;                      /* cascade 6th formant  */
 r6c.p2 = 0;

 r7c.p1 = 0;
 r7c.p2 = 0;

 r8c.p1 = 0;
 r8c.p2 = 0;

 rnpc.p1 = 0;                     /* cascade nasal pole  */
 rnpc.p2 = 0;

 rnz.p1 = 0;                      /* cascade nasal zero  */
 rnz.p2 = 0;

 rgl.p1 = 0;                      /* crit-damped glot low-pass filter */
 rgl.p2 = 0;

 rlp.p1 = 0;                      /* downsamp low-pass filter  */
 rlp.p2 = 0;

 vlast = 0;                       /* Previous output of voice  */
 nlast = 0;                       /* Previous output of random number generator  */
 glotlast = 0;                    /* Previous value of glotout  */
 warnsw = 0;
}
