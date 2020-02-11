#include <config.h>
/* $Id: holmes.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
 */
char *holmes_id = "$Id: holmes.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $";
#include <stdio.h>
#include <ctype.h>
#include <useconfig.h>
#include <math.h>
#include "proto.h"
#include "nsynth.h"
#include "elements.h"
#include "darray.h"
#include "holmes.h"
#include "phfeat.h"
#include "getargs.h"

#if 1
#define AMP_ADJ 14
#else
#define AMP_ADJ 0
#endif

FILE *par_file;
FILE *jsru_file;
int speed = 1;

double frac = 1.0;

typedef struct
 {
  float v;                        /* boundary value */
  int t;                          /* transition time */
 }
slope_t;

typedef struct
 {
  slope_t p[nEparm];
 }
trans_t;

typedef struct
 {
  float a;
  float b;
  float v;
 }
filter_t, *filter_ptr;

static float filter PROTO((filter_ptr p, Float v));

static void jsru_save PROTO((double f0, float *tp));

static float
filter(p, v)
filter_ptr p;
Float v;
{
 return p->v = (p->a * v + p->b * p->v);
}

/* 'a' is dominant element, 'b' is dominated
   ext is flag to say to use external times from 'a' rather
   than internal i.e. ext != 0 if 'a' is NOT current element.

 */

static void set_trans PROTO((slope_t * t, Elm_ptr a, Elm_ptr b, int ext, int e));

static void
set_trans(t, a, b, ext, e)
slope_t *t;
Elm_ptr a;
Elm_ptr b;
int ext;
int e;
{
 int i;
 for (i = 0; i < nEparm; i++)
  {
   t[i].t = ((ext) ? a->p[i].ed : a->p[i].id) * speed;
   if (t[i].t)
    t[i].v = a->p[i].fixd + (a->p[i].prop * b->p[i].stdy) * (float) 0.01;
   else
    t[i].v = b->p[i].stdy;
  }
}

static float linear PROTO((Float a, Float b, int t, int d));

/*              
   ______________ b
   /
   /
   /
   a____________/                 
   0   d
   ---------------t---------------
 */

static float
linear(a, b, t, d)
Float a;
Float b;
int t;
int d;
{
 if (t <= 0)
  return a;
 else if (t >= d)
  return b;
 else
  {
   float f = (float) t / (float) d;
   return a + (b - a) * f;
  }
}

static float interpolate PROTO((char *w, char *p, slope_t * s, slope_t * e, Float mid, int t, int d));

static float
interpolate(w, p, s, e, mid, t, d)
char *w;
char *p;
slope_t *s;
slope_t *e;
Float mid;
int t;
int d;
{
 float steady = d - (s->t + e->t);
#ifdef DEBUG
 fprintf(stdout, "%4s %s s=%g,%d e=%g,%d m=%g,%g\n",
         w, p, s->v, s->t, e->v, e->t, mid, steady);
#endif
 if (steady >= 0)
  {
   /* Value reaches stready state somewhere ... */
   if (t < s->t)
    return linear(s->v, mid, t, s->t);	/* initial transition */
   else
    {
     t -= s->t;
     if (t <= steady)
      return mid;                 /* steady state */
     else
      return linear(mid, e->v, (int) (t - steady), e->t);
     /* final transition */
    }
  }
 else
  {
   float f = (float) 1.0 - ((float) t / (float) d);
   float sp = linear(s->v, mid, t, s->t);
   float ep = linear(e->v, mid, d - t, e->t);
   return f * sp + ((float) 1.0 - f) * ep;
  }
}


unsigned
holmes(nelm, elm, nsamp, samp_base)
unsigned nelm;
unsigned char *elm;
unsigned nsamp;
short *samp_base;
{
 filter_t flt[nEparm];
 klatt_frame_t pars;
 short *samp = samp_base;
 Elm_ptr le = &Elements[0];
 unsigned i = 0;
 unsigned tstress = 0;
 unsigned ntstress = 0;
 slope_t stress_s;
 slope_t stress_e;
 float top = 1.1 * def_pars.F0hz10;
 int j;
 pars = def_pars;
 pars.FNPhz = le->p[fn].stdy;
 pars.B1phz = pars.B1hz = 60;
 pars.B2phz = pars.B2hz = 90;
 pars.B3phz = pars.B3hz = 150;
#if 0
 pars.F4hz = 3500;
#endif
 pars.B4phz = def_pars.B4phz;

 /* flag new utterance */
 parwave_init(&klatt_global);

 /* Set stress attack/decay slope */
 stress_s.t = 40;
 stress_e.t = 40;
 stress_e.v = 0.0;

 for (j = 0; j < nEparm; j++)
  {
   flt[j].v = le->p[j].stdy;
   flt[j].a = frac;
   flt[j].b = (float) 1.0 - (float) frac;
  }
 while (i < nelm)
  {
   Elm_ptr ce = &Elements[elm[i++]];
   unsigned dur = elm[i++];
   i++; /* skip stress */
   /* Skip zero length elements which are only there to affect
      boundary values of adjacent elements
    */
   if (dur > 0)
    {
     Elm_ptr ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
     slope_t start[nEparm];
     slope_t end[nEparm];
     unsigned t;

     if (ce->rk > le->rk)
      {
       if (par_file)
        fprintf(par_file, "# %s < %s\n", le->name, ce->name);
       set_trans(start, ce, le, 0, 's');
       /* we dominate last */
      }
     else
      {
       if (par_file)
        fprintf(par_file, "# %s >= %s\n", le->name, ce->name);
       set_trans(start, le, ce, 1, 's');
       /* last dominates us */
      }

     if (ne->rk > ce->rk)
      {
       if (par_file)
        fprintf(par_file, "# %s < %s\n", ce->name, ne->name);
       set_trans(end, ne, ce, 1, 'e');
       /* next dominates us */
      }
     else
      {
       if (par_file)
        fprintf(par_file, "# %s >= %s\n", ce->name, ne->name);
       set_trans(end, ce, ne, 0, 'e');
       /* we dominate next */
      }

     if (par_file)
      {
       int j;
       fprintf(par_file, "# %s\n", ce->name);
       for (j = 0; j < nEparm; j++)
        fprintf(par_file, "%c%6s", (j) ? ' ' : '#', Ep_name[j]);
       fprintf(par_file, "\n");
       for (j = 0; j < nEparm; j++)
        fprintf(par_file, "%c%6.4g", (j) ? ' ' : '#', start[j].v);
       fprintf(par_file, "\n");
       for (j = 0; j < nEparm; j++)
        fprintf(par_file, "%c%6d", (j) ? ' ' : '#', start[j].t);
       fprintf(par_file, "\n");
      }

     for (t = 0; t < dur; t++, tstress++)
      {
       float base = top * 0.8 /* 3 * top / 5 */;
       float tp[nEparm];
       int j;

       if (tstress == ntstress)
        {
         unsigned j = i;
         stress_s = stress_e;
         tstress = 0;
         ntstress = dur;
#ifdef DEBUG_STRESS
         printf("Stress %g -> ", stress_s.v);
#endif
         while (j <= nelm)
          {
           Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
           unsigned du = (j < nelm) ? elm[j++] : 0;
           unsigned s  = (j < nelm) ? elm[j++] : 3;
           if (s || e->feat & vwl)
            {
             unsigned d = 0;
             if (s)
              stress_e.v = (float) s / 3;
             else
              stress_e.v = (float) 0.1;
             do
              {
               d += du;
#ifdef DEBUG_STRESS
               printf("%s", (e && e->dict) ? e->dict : "");
#endif
               e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
               du = elm[j++];
              }
             while ((e->feat & vwl) && elm[j++] == s);
             ntstress += d / 2;
             break;
            }
           ntstress += du;
          }
#ifdef DEBUG_STRESS
         printf(" %g @ %d\n", stress_e.v, ntstress);
#endif
        }

       for (j = 0; j < nEparm; j++)
        tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &start[j], &end[j], (float) ce->p[j].stdy, t, dur));

       /* Now call the synth for each frame */

       pars.F0hz10 = base + (top - base) *
        interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

       pars.AVdb = pars.AVpdb = tp[av];
       pars.AF = tp[af];
       pars.FNZhz = tp[fn];
       pars.ASP = tp[asp];
       pars.Aturb = tp[avc];
       pars.B1phz = pars.B1hz = tp[b1];
       pars.B2phz = pars.B2hz = tp[b2];
       pars.B3phz = pars.B3hz = tp[b3];
       pars.F1hz = tp[f1];
       pars.F2hz = tp[f2];
       pars.F3hz = tp[f3];
       /* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
          Needs to be fixed properly in tables
        */
/*
   pars.ANP  = AMP_ADJ + tp[an];
 */
       pars.AB = AMP_ADJ + tp[ab];
       pars.A5 = AMP_ADJ + tp[a5];
       pars.A6 = AMP_ADJ + tp[a6];
       pars.A1 = AMP_ADJ + tp[a1];
       pars.A2 = AMP_ADJ + tp[a2];
       pars.A3 = AMP_ADJ + tp[a3];
       pars.A4 = AMP_ADJ + tp[a4];

       parwave(&klatt_global, &pars, samp);

       samp += klatt_global.nspfr;
       if (par_file)
        {
         for (j = 0; j < nEparm; j++)
          fprintf(par_file, " %6.4g", tp[j]);
         fprintf(par_file, "\n");
        }
       if (jsru_file)
        jsru_save(pars.F0hz10 * 0.1, tp);
       /* Declination of f0 envelope 0.25Hz / cS */
       top -= 0.5;
      }
     if (par_file)
      {
       int j;
       for (j = 0; j < nEparm; j++)
        fprintf(par_file, "%c%6.4g", (j) ? ' ' : '#', end[j].v);
       fprintf(par_file, "\n");
       for (j = 0; j < nEparm; j++)
        fprintf(par_file, "%c%6d", (j) ? ' ' : '#', end[j].t);
       fprintf(par_file, "\n");
      }
    }
   le = ce;
  }
 return (samp - samp_base);
}

int
init_holmes(argc, argv)
int argc;
char *argv[];
{
 char *par_name = NULL;
 char *jsru_name = NULL;
 argc = getargs("Holmes",argc, argv,
                "p", "", &par_name,  "Parameter file for plot",
                "j", "", &jsru_name, "Data for alternate synth (JSRU)",
                "S", "%d", &speed,   "Speed (1.0 is 'normal')",
                "K", "%lg", &frac,   "Parameter filter 'fraction'",
                NULL);
 if (help_only)
  return argc;
 if (par_name)
  {
   par_file = fopen(par_name, "w");
   if (!par_file)
    perror(par_name);
  }
 if (jsru_name)
  {
   jsru_file = fopen(jsru_name, "w");
   if (!jsru_file)
    perror(jsru_name);
  }
 return argc;
}

void
term_holmes()
{
 if (par_file)
  fclose(par_file);
 if (jsru_file)
  fclose(jsru_file);
}

static int jsru_freq PROTO((Float f, Float base, Float inc));
static int
jsru_freq(f, base, inc)
Float f;
Float base;
Float inc;
{
 int i;
 f = (f - base) / inc;
 i = (int) f;
 if (i >= 64)
  i = 63;
 return i;
}

static int jsru_amp PROTO((Float a));
static int
jsru_amp(a)
Float a;
{
 int i = a;
 if (i <= 0)
  i = 1;
 if (i >= 64)
  i = 63;
 return i;
}


/*          0     1      2      3      4    5
   F1    F2     F3                 FN
   flflim 125.0 550.0 1350.0 3500.0 3500.0 95.0
   fincrm  25.0  50.0   50.0    0.0    0.0  5.0
 */

static void
jsru_save(f0, tp)
double f0;
float *tp;
{
 f0 = 16 * (log(f0 / 25.0) / log(2.0)) - 1;

 /* fn, alf, f1, a1, f2, a2, f3, a3, ahf, v, f0, m */
 fputc(jsru_freq(tp[fn], 95.0, 5.0), jsru_file);
 fputc(jsru_amp(tp[an]), jsru_file);
 fputc(jsru_freq(tp[f1], 125.0, 25.0), jsru_file);
 fputc(jsru_amp(tp[a1]), jsru_file);
 fputc(jsru_freq(tp[f2], 550.0, 50.0), jsru_file);
 fputc(jsru_amp(tp[a2]), jsru_file);
 fputc(jsru_freq(tp[f3], 1350.0, 50.0), jsru_file);
 fputc(jsru_amp(tp[a3]), jsru_file);
 fputc(jsru_amp(tp[a4]), jsru_file);
 fputc(jsru_amp(tp[av]), jsru_file);
 fputc((int) f0, jsru_file);
 fputc((int) 32, jsru_file);
}
